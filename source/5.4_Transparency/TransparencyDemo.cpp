#include "pch.h"
#include "TransparencyDemo.h"
#include "FirstPersonCamera.h"
#include "VertexDeclarations.h"
#include "Game.h"
#include "GameException.h"
#include "Model.h"
#include "Mesh.h"
#include "TransparencyMaterial.h"
#include "Texture2D.h"
#include "TextureCube.h"
#include "ProxyModel.h"

using namespace std;
using namespace std::string_literals;
using namespace gsl;
using namespace Library;
using namespace DirectX;

namespace Rendering
{
	TransparencyDemo::TransparencyDemo(Game & game, const shared_ptr<Camera>& camera) :
		DrawableGameComponent(game, camera)
	{
	}

	TransparencyDemo::~TransparencyDemo()
	{
	}

	float TransparencyDemo::AmbientLightIntensity() const
	{
		return mMaterial->AmbientColor().x;
	}

	void TransparencyDemo::SetAmbientLightIntensity(float intensity)
	{
		mMaterial->SetAmbientColor(XMFLOAT4(intensity, intensity, intensity, 1.0f));
	}

	float TransparencyDemo::DirectionalLightIntensity() const
	{
		return mMaterial->LightColor().x;
	}

	void TransparencyDemo::SetDirectionalLightIntensity(float intensity)
	{
		mMaterial->SetLightColor(XMFLOAT4(intensity, intensity, intensity, 1.0f));
	}

	const XMFLOAT3& TransparencyDemo::LightDirection() const
	{
		return mDirectionalLight.Direction();
	}

	void TransparencyDemo::RotateDirectionalLight(XMFLOAT2 amount)
	{
		XMMATRIX lightRotationMatrix = XMMatrixRotationY(amount.x) * XMMatrixRotationAxis(mDirectionalLight.RightVector(), amount.y);
		mDirectionalLight.ApplyRotation(lightRotationMatrix);
		mProxyModel->ApplyRotation(lightRotationMatrix);
		mMaterial->SetLightDirection(mDirectionalLight.DirectionToLight());
	}

	float TransparencyDemo::SpecularIntensity() const
	{
		return mMaterial->SpecularColor().x;
	}

	void TransparencyDemo::SetSpecularIntensity(float intensity)
	{
		mMaterial->SetSpecularColor(XMFLOAT3(intensity, intensity, intensity));
	}

	float TransparencyDemo::SpecularPower() const
	{
		return mMaterial->SpecularPower();
	}

	void TransparencyDemo::SetSpecularPower(float power)
	{
		mMaterial->SetSpecularPower(power);
	}

	float TransparencyDemo::FogStart() const
	{
		return mMaterial->FogStart();
	}

	void TransparencyDemo::SetFogStart(float fogStart)
	{
		mMaterial->SetFogStart(fogStart);
	}

	float TransparencyDemo::FogRange() const
	{
		return mMaterial->FogRange();
	}

	void TransparencyDemo::SetFogRange(float fogRange)
	{
		mMaterial->SetFogRange(fogRange);
	}

	void TransparencyDemo::Initialize()
	{
		auto direct3DDevice = mGame->Direct3DDevice();
		
		const VertexPositionTextureNormal sourceVertices[] =
		{
			VertexPositionTextureNormal(XMFLOAT4(-0.5f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), Vector3Helper::Backward),
			VertexPositionTextureNormal(XMFLOAT4(-0.5f, 1.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f), Vector3Helper::Backward),
			VertexPositionTextureNormal(XMFLOAT4(0.5f, 1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), Vector3Helper::Backward),

			VertexPositionTextureNormal(XMFLOAT4(-0.5f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), Vector3Helper::Backward),
			VertexPositionTextureNormal(XMFLOAT4(0.5f, 1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), Vector3Helper::Backward),
			VertexPositionTextureNormal(XMFLOAT4(0.5f, 0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 1.0f), Vector3Helper::Backward)
		};

		const span<const VertexPositionTextureNormal> vertices{ sourceVertices };
		mVertexCount = narrow_cast<uint32_t>(vertices.size());
		VertexPositionTextureNormal::CreateVertexBuffer(mGame->Direct3DDevice(), vertices, not_null<ID3D11Buffer**>(mVertexBuffer.put()));

		auto colorMap = mGame->Content().Load<Texture2D>(L"Textures\\Checkerboard.png"s);
		auto specularMap = mGame->Content().Load<Texture2D>(L"Textures\\CheckerboardSpecularMap.png"s);
		auto transparencyMap = mGame->Content().Load<Texture2D>(L"Textures\\AlphaMask_32bpp.png"s);
		mMaterial = make_shared<TransparencyMaterial>(*mGame, colorMap, specularMap, transparencyMap);
		mMaterial->Initialize();

		mProxyModel = make_unique<ProxyModel>(*mGame, mCamera, "Models\\DirectionalLightProxy.obj.bin"s, 0.5f);
		mProxyModel->Initialize();
		mProxyModel->SetPosition(10.0f, 0.0, 0.0f);
		mProxyModel->ApplyRotation(XMMatrixRotationY(XM_PIDIV2));

		mMaterial->SetLightDirection(mDirectionalLight.DirectionToLight());

		auto updateMaterialFunc = [this]() { mUpdateMaterial = true; };
		mCamera->AddViewMatrixUpdatedCallback(updateMaterialFunc);
		mCamera->AddProjectionMatrixUpdatedCallback(updateMaterialFunc);

		auto firstPersonCamera = mCamera->As<FirstPersonCamera>();
		if (firstPersonCamera != nullptr)
		{
			firstPersonCamera->AddPositionUpdatedCallback([this]() {
				mMaterial->UpdateCameraPosition(mCamera->Position());
			});
		}

		XMStoreFloat4x4(&mWorldMatrix, XMMatrixScaling(10.0f, 10.0f, 10.0f));
	}

	void TransparencyDemo::Update(const GameTime& gameTime)
	{
		mProxyModel->Update(gameTime);
	}

	void TransparencyDemo::Draw(const GameTime& gameTime)
	{
		if (mUpdateMaterial)
		{
			const XMMATRIX worldMatrix = XMLoadFloat4x4(&mWorldMatrix);
			const XMMATRIX wvp = XMMatrixTranspose(worldMatrix * mCamera->ViewProjectionMatrix());
			mMaterial->UpdateTransforms(wvp, XMMatrixTranspose(worldMatrix));
			mUpdateMaterial = false;
		}

		mMaterial->Draw(not_null<ID3D11Buffer*>(mVertexBuffer.get()), mVertexCount);
		mProxyModel->Draw(gameTime);
	}
}