#include "pch.h"
#include "EnvironmentMappingMaterial.h"
#include "Game.h"
#include "GameException.h"
#include "VertexDeclarations.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "Texture2D.h"
#include "TextureCube.h"

using namespace std;
using namespace std::string_literals;
using namespace gsl;
using namespace winrt;
using namespace DirectX;
using namespace Library;

namespace Rendering
{
	RTTI_DEFINITIONS(EnvironmentMappingMaterial)

	EnvironmentMappingMaterial::EnvironmentMappingMaterial(Game& game, shared_ptr<Texture2D> colorMap, shared_ptr<TextureCube> environmentMap) :
		Material(game), mColorMap(move(colorMap)), mEnvironmentMap(move(environmentMap))
	{
	}

	com_ptr<ID3D11SamplerState> EnvironmentMappingMaterial::SamplerState() const
	{
		return mSamplerState;
	}

	void EnvironmentMappingMaterial::SetSamplerState(com_ptr<ID3D11SamplerState> samplerState)
	{
		mSamplerState = move(samplerState);
	}

	shared_ptr<Texture2D> EnvironmentMappingMaterial::ColorMap() const
	{
		return mColorMap;
	}

	void EnvironmentMappingMaterial::SetColorMap(shared_ptr<Texture2D> texture)
	{
		mColorMap = move(texture);
	}

	shared_ptr<TextureCube> EnvironmentMappingMaterial::EnvironmentMap() const
	{
		return mEnvironmentMap;
	}

	void EnvironmentMappingMaterial::SetEnvironmentMap(shared_ptr<TextureCube> texture)
	{
		mEnvironmentMap = move(texture);
	}

	const XMFLOAT4& EnvironmentMappingMaterial::AmbientColor() const
	{
		return mPixelCBufferPerFrameData.AmbientColor;
	}

	void EnvironmentMappingMaterial::SetAmbientColor(const XMFLOAT4& color)
	{
		mPixelCBufferPerFrameData.AmbientColor = color;
		mPixelCBufferPerFrameDataDirty = true;
	}

	const XMFLOAT4& EnvironmentMappingMaterial::EnvironmentColor() const
	{
		return mPixelCBufferPerFrameData.EnvironmentColor;
	}

	void EnvironmentMappingMaterial::SetEnvironmentColor(const XMFLOAT4& color)
	{
		mPixelCBufferPerFrameData.EnvironmentColor = color;
		mPixelCBufferPerFrameDataDirty = true;
	}

	const float EnvironmentMappingMaterial::ReflectionAmount() const
	{
		return mPixelCBufferPerObjectData.ReflectionAmount;
	}

	void EnvironmentMappingMaterial::SetReflectionAmount(float amount)
	{
		mPixelCBufferPerObjectData.ReflectionAmount = amount;
		mGame->Direct3DDeviceContext()->UpdateSubresource(mPixelCBufferPerObject.get(), 0, nullptr, &mPixelCBufferPerObjectData, 0, 0);
	}

	uint32_t EnvironmentMappingMaterial::VertexSize() const
	{
		return sizeof(VertexPositionTextureNormal);
	}

	void EnvironmentMappingMaterial::Initialize()
	{
		Material::Initialize();

		auto direct3DDevice = mGame->Direct3DDevice();
		mVertexShader = mGame->Content().Load<VertexShader>(L"Shaders\\EnvironmentMappingDemoVS.cso"s);
		mVertexShader->CreateInputLayout<VertexPositionTextureNormal>(direct3DDevice);
		mPixelShader = mGame->Content().Load<PixelShader>(L"Shaders\\EnvironmentMappingDemoPS.cso");

		D3D11_BUFFER_DESC constantBufferDesc{ 0 };
		constantBufferDesc.ByteWidth = sizeof(VertexCBufferPerFrame);
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		ThrowIfFailed(direct3DDevice->CreateBuffer(&constantBufferDesc, nullptr, mVertexCBufferPerFrame.put()), "ID3D11Device::CreateBuffer() failed.");

		constantBufferDesc.ByteWidth = sizeof(VertexCBufferPerObject);
		ThrowIfFailed(direct3DDevice->CreateBuffer(&constantBufferDesc, nullptr, mVertexCBufferPerObject.put()), "ID3D11Device::CreateBuffer() failed.");

		constantBufferDesc.ByteWidth = sizeof(PixelCBufferPerFrame);
		ThrowIfFailed(direct3DDevice->CreateBuffer(&constantBufferDesc, nullptr, mPixelCBufferPerFrame.put()), "ID3D11Device::CreateBuffer() failed.");

		constantBufferDesc.ByteWidth = sizeof(PixelCBufferPerObject);
		ThrowIfFailed(direct3DDevice->CreateBuffer(&constantBufferDesc, nullptr, mPixelCBufferPerObject.put()), "ID3D11Device::CreateBuffer() failed.");

		auto direct3DDeviceContext = mGame->Direct3DDeviceContext();
		direct3DDeviceContext->UpdateSubresource(mVertexCBufferPerFrame.get(), 0, nullptr, &mVertexCBufferPerFrameData, 0, 0);
		direct3DDeviceContext->UpdateSubresource(mVertexCBufferPerObject.get(), 0, nullptr, &mVertexCBufferPerObjectData, 0, 0);
		direct3DDeviceContext->UpdateSubresource(mPixelCBufferPerFrame.get(), 0, nullptr, &mPixelCBufferPerFrameData, 0, 0);
		direct3DDeviceContext->UpdateSubresource(mPixelCBufferPerObject.get(), 0, nullptr, &mPixelCBufferPerObjectData, 0, 0);
	}

	void EnvironmentMappingMaterial::UpdateCameraPosition(const DirectX::XMFLOAT3& position)
	{
		mVertexCBufferPerFrameData.CameraPosition = position;
		mGame->Direct3DDeviceContext()->UpdateSubresource(mVertexCBufferPerFrame.get(), 0, nullptr, &mVertexCBufferPerFrameData, 0, 0);
	}

	void EnvironmentMappingMaterial::UpdateTransforms(FXMMATRIX worldViewProjectionMatrix, CXMMATRIX worldMatrix)
	{
		XMStoreFloat4x4(&mVertexCBufferPerObjectData.WorldViewProjection, worldViewProjectionMatrix);
		XMStoreFloat4x4(&mVertexCBufferPerObjectData.World, worldMatrix);
		mGame->Direct3DDeviceContext()->UpdateSubresource(mVertexCBufferPerObject.get(), 0, nullptr, &mVertexCBufferPerObjectData, 0, 0);
	}

	void EnvironmentMappingMaterial::BeginDraw()
	{
		Material::BeginDraw();

		auto direct3DDeviceContext = mGame->Direct3DDeviceContext();

		if (mPixelCBufferPerFrameDataDirty)
		{
			direct3DDeviceContext->UpdateSubresource(mPixelCBufferPerFrame.get(), 0, nullptr, &mPixelCBufferPerFrameData, 0, 0);
			mPixelCBufferPerFrameDataDirty = false;
		}

		ID3D11Buffer* const vsConstantBuffers[]{ mVertexCBufferPerFrame.get(), mVertexCBufferPerObject.get() };
		direct3DDeviceContext->VSSetConstantBuffers(0, narrow_cast<uint32_t>(size(vsConstantBuffers)), vsConstantBuffers);

		ID3D11Buffer* const psConstantBuffers[]{ mPixelCBufferPerFrame.get(), mPixelCBufferPerObject.get() };
		direct3DDeviceContext->PSSetConstantBuffers(0, narrow_cast<uint32_t>(size(psConstantBuffers)), psConstantBuffers);

		ID3D11ShaderResourceView* const psShaderResources[] = { mColorMap->ShaderResourceView().get(), mEnvironmentMap->ShaderResourceView().get() };
		direct3DDeviceContext->PSSetShaderResources(0, narrow_cast<uint32_t>(size(psShaderResources)), psShaderResources);

		const auto psSamplers = mSamplerState.get();
		direct3DDeviceContext->PSSetSamplers(0, 1, &psSamplers);
	}
}