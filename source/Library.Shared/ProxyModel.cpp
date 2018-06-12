#include "pch.h"
#include "ProxyModel.h"
#include "Game.h"
#include "GameException.h"
#include "Utility.h"
#include "Camera.h"
#include "VertexDeclarations.h"
#include "RasterizerStates.h"
#include "..\Library.Shared\Model.h"
#include "..\Library.Shared\Mesh.h"
#include "BasicMaterial.h"
#include <gsl\gsl>

using namespace std;
using namespace gsl;
using namespace DirectX;

namespace Library
{
	RTTI_DEFINITIONS(ProxyModel)

	ProxyModel::ProxyModel(Game& game, const shared_ptr<Camera>& camera, const std::string& modelFileName, float scale) :
		DrawableGameComponent(game, camera),
		mModelFileName(modelFileName), mScale(scale),
		mMaterial(make_shared<BasicMaterial>(*mGame))
	{
	}

	const XMFLOAT3& ProxyModel::Position() const
	{
		return mPosition;
	}

	const XMFLOAT3& ProxyModel::Direction() const
	{
		return mDirection;
	}

	const XMFLOAT3& ProxyModel::Up() const
	{
		return mUp;
	}

	const XMFLOAT3& ProxyModel::Right() const
	{
		return mRight;
	}

	XMVECTOR ProxyModel::PositionVector() const
	{
		return XMLoadFloat3(&mPosition);
	}

	XMVECTOR ProxyModel::DirectionVector() const
	{
		return XMLoadFloat3(&mDirection);
	}

	XMVECTOR ProxyModel::UpVector() const
	{
		return XMLoadFloat3(&mUp);
	}

	XMVECTOR ProxyModel::RightVector() const
	{
		return XMLoadFloat3(&mRight);
	}

	bool& ProxyModel::DisplayWireframe()
	{
		return mDisplayWireframe;
	}

	void ProxyModel::SetPosition(float x, float y, float z)
	{
		XMVECTOR position = XMVectorSet(x, y, z, 1.0f);
		SetPosition(position);
	}

	void ProxyModel::SetPosition(FXMVECTOR position)
	{
		XMStoreFloat3(&mPosition, position);
	}

	void ProxyModel::SetPosition(const XMFLOAT3& position)
	{
		mPosition = position;
	}

	void ProxyModel::ApplyRotation(CXMMATRIX transform)
	{
		XMVECTOR direction = XMLoadFloat3(&mDirection);
		XMVECTOR up = XMLoadFloat3(&mUp);

		direction = XMVector3TransformNormal(direction, transform);
		direction = XMVector3Normalize(direction);

		up = XMVector3TransformNormal(up, transform);
		up = XMVector3Normalize(up);

		XMVECTOR right = XMVector3Cross(direction, up);
		up = XMVector3Cross(right, direction);

		XMStoreFloat3(&mDirection, direction);
		XMStoreFloat3(&mUp, up);
		XMStoreFloat3(&mRight, right);
	}

	void ProxyModel::ApplyRotation(const XMFLOAT4X4& transform)
	{
		XMMATRIX transformMatrix = XMLoadFloat4x4(&transform);
		ApplyRotation(transformMatrix);
	}

	void ProxyModel::SetColor(const DirectX::XMFLOAT4& color)
	{
		mMaterial->SetSurfaceColor(color);
	}

	void ProxyModel::Initialize()
	{
		const auto model = mGame->Content().Load<Model>(Utility::ToWideString(mModelFileName));
		Mesh* mesh = model->Meshes().at(0).get();
		VertexPosition::CreateVertexBuffer(mGame->Direct3DDevice(), *mesh, not_null<ID3D11Buffer**>(mVertexBuffer.ReleaseAndGetAddressOf()));
		mesh->CreateIndexBuffer(*mGame->Direct3DDevice(), not_null<ID3D11Buffer**>(mIndexBuffer.ReleaseAndGetAddressOf()));
		mIndexCount = narrow<uint32_t>(mesh->Indices().size());

		mMaterial->Initialize();

		using namespace std::placeholders;
		mMaterial->SetUpdateMaterialCallback(bind(&ProxyModel::UpdateMaterial, this));	}

	void ProxyModel::Update(const GameTime&)
	{
		XMMATRIX worldMatrix = XMMatrixIdentity();
		MatrixHelper::SetForward(worldMatrix, mDirection);
		MatrixHelper::SetUp(worldMatrix, mUp);
		MatrixHelper::SetRight(worldMatrix, mRight);
		MatrixHelper::SetTranslation(worldMatrix, mPosition);

		XMStoreFloat4x4(&mWorldMatrix, XMMatrixScaling(mScale, mScale, mScale) * worldMatrix);
	}

	void ProxyModel::Draw(const GameTime&)
	{
		if (mDisplayWireframe)
		{
			mGame->Direct3DDeviceContext()->RSSetState(RasterizerStates::Wireframe.Get());
			mMaterial->DrawIndexed(not_null<ID3D11Buffer*>(mVertexBuffer.Get()), not_null<ID3D11Buffer*>(mIndexBuffer.Get()), mIndexCount);
			mGame->Direct3DDeviceContext()->RSSetState(nullptr);
		}
		else
		{
			mMaterial->DrawIndexed(not_null<ID3D11Buffer*>(mVertexBuffer.Get()), not_null<ID3D11Buffer*>(mIndexBuffer.Get()), mIndexCount);
		}
	}

	void ProxyModel::UpdateMaterial()
	{
		const XMMATRIX worldMatrix = XMLoadFloat4x4(&mWorldMatrix);
		const XMMATRIX wvp = XMMatrixTranspose(worldMatrix * mCamera->ViewProjectionMatrix());
		mMaterial->UpdateTransform(wvp);
	}
}