#include "pch.h"
#include "DepthMapMaterial.h"
#include "Game.h"
#include "GameException.h"
#include "VertexDeclarations.h"
#include "VertexShader.h"
#include "PixelShader.h"

using namespace std;
using namespace gsl;
using namespace DirectX;

namespace Library
{
	RTTI_DEFINITIONS(DepthMapMaterial)

	DepthMapMaterial::DepthMapMaterial(Game& game) :
		Material(game)
	{
	}	

	uint32_t DepthMapMaterial::VertexSize() const
	{
		return sizeof(VertexPosition);
	}

	void DepthMapMaterial::Initialize()
	{
		Material::Initialize();

		auto vertexShader = mGame->Content().Load<VertexShader>(L"Shaders\\DepthMapVS.cso");
		SetShader(vertexShader);
		SetShader<PixelShader>(nullptr);
		
		auto direct3DDevice = mGame->Direct3DDevice();
		vertexShader->CreateInputLayout<VertexPosition>(direct3DDevice);
		SetInputLayout(vertexShader->InputLayout());

		D3D11_BUFFER_DESC constantBufferDesc{ 0 };
		constantBufferDesc.ByteWidth = sizeof(XMFLOAT4X4);
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		ThrowIfFailed(direct3DDevice->CreateBuffer(&constantBufferDesc, nullptr, mVSConstantBuffer.put()), "ID3D11Device::CreateBuffer() failed.");
		AddConstantBuffer(ShaderStages::VS, mVSConstantBuffer.get());
	}

	void DepthMapMaterial::UpdateTransform(CXMMATRIX worldLightViewProjectionMatrix)
	{
		mGame->Direct3DDeviceContext()->UpdateSubresource(mVSConstantBuffer.get(), 0, nullptr, worldLightViewProjectionMatrix.r, 0, 0);
	}
}