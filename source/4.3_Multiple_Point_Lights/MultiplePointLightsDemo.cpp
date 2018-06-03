#include "pch.h"
#include "MultiplePointLightsDemo.h"
#include "FirstPersonCamera.h"
#include "VertexDeclarations.h"
#include "Game.h"
#include "GameException.h"
#include "..\Library.Shared\Model.h"
#include "..\Library.Shared\Mesh.h"
#include "ProxyModel.h"
#include "MultiplePointLightsMaterial.h"
#include "Texture2D.h"
#include "PointLight.h"

using namespace std;
using namespace std::string_literals;
using namespace gsl;
using namespace Library;
using namespace DirectX;

namespace Rendering
{
	MultiplePointLightsDemo::MultiplePointLightsDemo(Game & game, const shared_ptr<Camera>& camera) :
		DrawableGameComponent(game, camera)
	{
	}

	MultiplePointLightsDemo::~MultiplePointLightsDemo()
	{
	}

	bool MultiplePointLightsDemo::AnimationEnabled() const
	{
		return mAnimationEnabled;
	}

	void MultiplePointLightsDemo::SetAnimationEnabled(bool enabled)
	{
		mAnimationEnabled = enabled;
	}

	void MultiplePointLightsDemo::ToggleAnimation()
	{
		mAnimationEnabled = !mAnimationEnabled;
	}

	float MultiplePointLightsDemo::AmbientLightIntensity() const
	{
		return mMaterial->AmbientColor().x;
	}

	void MultiplePointLightsDemo::SetAmbientLightIntensity(float intensity)
	{
		mMaterial->SetAmbientColor(XMFLOAT4(intensity, intensity, intensity, 1.0f));
	}

	const std::array<Library::PointLight, 4>& MultiplePointLightsDemo::PointLights() const
	{
		return mMaterial->PointLights();
	}

	void MultiplePointLightsDemo::SetPointLight(const Library::PointLight& light, std::uint32_t index)
	{
		mMaterial->SetPointLight(light, index);
		
		assert(index < mProxyModels.size());
		mProxyModels[index]->SetPosition(light.Position());
	}

	float MultiplePointLightsDemo::SpecularIntensity() const
	{
		return mMaterial->SpecularColor().x;
	}

	void MultiplePointLightsDemo::SetSpecularIntensity(float intensity)
	{
		mMaterial->SetSpecularColor(XMFLOAT3(intensity, intensity, intensity));
	}

	float MultiplePointLightsDemo::SpecularPower() const
	{
		return mMaterial->SpecularPower();
	}

	void MultiplePointLightsDemo::SetSpecularPower(float power)
	{
		mMaterial->SetSpecularPower(power);
	}

	void MultiplePointLightsDemo::Initialize()
	{
		const auto model = mGame->Content().Load<Model>(L"Models\\Sphere.obj.bin"s);
		Mesh* mesh = model->Meshes().at(0).get();
		CreateVertexBuffer(mGame->Direct3DDevice(), *mesh, mVertexBuffer.ReleaseAndGetAddressOf());
		mesh->CreateIndexBuffer(*mGame->Direct3DDevice(), mIndexBuffer.ReleaseAndGetAddressOf());
		mIndexCount = narrow<uint32_t>(mesh->Indices().size());

		auto colorMap = mGame->Content().Load<Texture2D>(L"Textures\\EarthComposite.dds"s);
		auto specularMap = mGame->Content().Load<Texture2D>(L"Textures\\EarthSpecularMap.png"s);
		mMaterial = make_shared<MultiplePointLightsMaterial>(*mGame, colorMap, specularMap);
		mMaterial->Initialize();

		for (uint32_t i = 0; i < mProxyModels.size(); ++i)
		{
			auto& proxyModel = mProxyModels[i];

			proxyModel = make_unique<ProxyModel>(*mGame, mCamera, "Models\\PointLightProxy.obj.bin"s, 0.5f);
			proxyModel->Initialize();
		}
				
		SetPointLight(PointLight(XMFLOAT3(10.0f, 0.0, 0.0f)), 0);
		SetPointLight(PointLight(XMFLOAT3(-10.0f, 0.0, 0.0f)), 1);
		SetPointLight(PointLight(XMFLOAT3(0.0f, 10.0, 0.0f)), 2);
		SetPointLight(PointLight(XMFLOAT3(0.0f, -10.0, 0.0f)), 3);

		auto firstPersonCamera = mCamera->As<FirstPersonCamera>();
		if (firstPersonCamera != nullptr)
		{
			firstPersonCamera->SetPositionUpdatedCallback([this]() {
				mMaterial->UpdateCameraPosition(mCamera->Position());
			});
		}

		auto updateMaterialFunc = [this]() { mUpdateMaterial = true; };
		mCamera->SetViewMatrixUpdatedCallback(updateMaterialFunc);
		mCamera->SetProjectionMatrixUpdatedCallback(updateMaterialFunc);
	}

	void MultiplePointLightsDemo::Update(const GameTime& gameTime)
	{
		if (mAnimationEnabled)
		{
			mModelRotationAngle += gameTime.ElapsedGameTimeSeconds().count() * RotationRate;
			XMStoreFloat4x4(&mWorldMatrix, XMMatrixRotationY(mModelRotationAngle));			
			mUpdateMaterial = true;
		}

		for (auto& proxyModel : mProxyModels)
		{
			proxyModel->Update(gameTime);
		}
	}

	void MultiplePointLightsDemo::Draw(const GameTime& gameTime)
	{
		if (mUpdateMaterial)
		{
			const XMMATRIX worldMatrix = XMLoadFloat4x4(&mWorldMatrix);
			const XMMATRIX wvp = XMMatrixTranspose(worldMatrix * mCamera->ViewProjectionMatrix());
			mMaterial->UpdateTransforms(wvp, XMMatrixTranspose(worldMatrix));
			mUpdateMaterial = false;
		}

		mMaterial->DrawIndexed(mVertexBuffer.Get(), mIndexBuffer.Get(), mIndexCount);

		for (auto& proxyModel : mProxyModels)
		{
			proxyModel->Draw(gameTime);
		}		
	}

	void MultiplePointLightsDemo::CreateVertexBuffer(not_null<ID3D11Device*> device, const Mesh& mesh, not_null<ID3D11Buffer**> vertexBuffer) const
	{
		const vector<XMFLOAT3>& sourceVertices = mesh.Vertices();
		const auto& sourceUVs = mesh.TextureCoordinates().at(0);
		assert(sourceUVs.size() == sourceVertices.size());
		const auto& sourceNormals = mesh.Normals();
		assert(sourceNormals.size() == sourceVertices.size());

		vector<VertexPositionTextureNormal> vertices;
		vertices.reserve(sourceVertices.size());
		for (size_t i = 0; i < sourceVertices.size(); i++)
		{
			const XMFLOAT3& position = sourceVertices.at(i);
			const XMFLOAT3& uv = sourceUVs.at(i);
			const XMFLOAT3& normal = sourceNormals.at(i);
			vertices.emplace_back(XMFLOAT4(position.x, position.y, position.z, 1.0f), XMFLOAT2(uv.x, uv.y), normal);
		}

		D3D11_BUFFER_DESC vertexBufferDesc{ 0 };
		vertexBufferDesc.ByteWidth = narrow<uint32_t>(sizeof(VertexPositionTextureNormal) * vertices.size());
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA vertexSubResourceData{ 0 };
		vertexSubResourceData.pSysMem = &vertices[0];
		ThrowIfFailed(device->CreateBuffer(&vertexBufferDesc, &vertexSubResourceData, vertexBuffer), "ID3D11Device::CreateBuffer() failed.");
	}
}