#pragma once

#include <DirectXColors.h>
#include "Material.h"
#include "VectorHelper.h"
#include "MatrixHelper.h"
#include "SamplerStates.h"
#include "RenderStateHelper.h"

namespace Library
{
	class Texture2D;
}

namespace Rendering
{
	class TransparencyMaterial : public Library::Material
	{
		RTTI_DECLARATIONS(TransparencyMaterial, Library::Material)

	public:
		TransparencyMaterial(Library::Game& game, std::shared_ptr<Library::Texture2D> colormap, std::shared_ptr<Library::Texture2D> specularMap, std::shared_ptr<Library::Texture2D> transparencyMap);
		TransparencyMaterial(const TransparencyMaterial&) = default;
		TransparencyMaterial& operator=(const TransparencyMaterial&) = default;
		TransparencyMaterial(TransparencyMaterial&&) = default;
		TransparencyMaterial& operator=(TransparencyMaterial&&) = default;
		virtual ~TransparencyMaterial() = default;

		winrt::com_ptr<ID3D11SamplerState> SamplerState() const;
		void SetSamplerState(winrt::com_ptr<ID3D11SamplerState> samplerState);

		std::shared_ptr<Library::Texture2D> ColorMap() const;
		void SetColorMap(std::shared_ptr<Library::Texture2D> texture);

		std::shared_ptr<Library::Texture2D> SpecularMap() const;
		void SetSpecularMap(std::shared_ptr<Library::Texture2D> texture);

		std::shared_ptr<Library::Texture2D> TransparencyMap() const;
		void SetTransparencyMap(std::shared_ptr<Library::Texture2D> texture);

		const DirectX::XMFLOAT4& AmbientColor() const;
		void SetAmbientColor(const DirectX::XMFLOAT4& color);

		const DirectX::XMFLOAT3& LightDirection() const;
		void SetLightDirection(const DirectX::XMFLOAT3& direction);

		const DirectX::XMFLOAT4& LightColor() const;
		void SetLightColor(const DirectX::XMFLOAT4& color);

		const DirectX::XMFLOAT3& SpecularColor() const;
		void SetSpecularColor(const DirectX::XMFLOAT3& color);

		const float SpecularPower() const;
		void SetSpecularPower(float power);

		const float FogStart() const;
		void SetFogStart(const float fogStart);

		const float FogRange() const;
		void SetFogRange(const float fogRange);

		virtual std::uint32_t VertexSize() const override;
		virtual void Initialize() override;

		void UpdateCameraPosition(const DirectX::XMFLOAT3& position);
		void UpdateTransforms(DirectX::FXMMATRIX worldViewProjectionMatrix, DirectX::CXMMATRIX worldMatrix);
		
	private:
		struct VertexCBufferPerFrame
		{	
			DirectX::XMFLOAT3 CameraPosition{ Library::Vector3Helper::Zero };
			float FogStart{ 20.0f };			
			float FogRange{ 40.0f };
			float Padding[3]{ 0.0f, 0.0f, 0.0f };
		};

		struct VertexCBufferPerObject
		{
			DirectX::XMFLOAT4X4 WorldViewProjection{ Library::MatrixHelper::Identity };
			DirectX::XMFLOAT4X4 World{ Library::MatrixHelper::Identity };
		};

		struct PixelCBufferPerFrame
		{
			DirectX::XMFLOAT4 AmbientColor{ DirectX::Colors::Black };
			DirectX::XMFLOAT3 LightDirection{ 0.0f, 0.0f, 1.0f };
			float Padding;
			DirectX::XMFLOAT4 LightColor{ DirectX::Colors::White };
			DirectX::XMFLOAT4 FogColor{ DirectX::Colors::CornflowerBlue };
			DirectX::XMFLOAT3 CameraPosition{ Library::Vector3Helper::Zero };
			float Padding2{ 0.0f };
		};

		struct PixelCBufferPerObject
		{
			DirectX::XMFLOAT3 SpecularColor{ 1.0f, 1.0f, 1.0f };
			float SpecularPower{ 128.0f };
		};

		virtual void BeginDraw() override;
		virtual void EndDraw() override;

		void ResetPixelShaderResources();

		winrt::com_ptr<ID3D11Buffer> mVertexCBufferPerFrame;
		winrt::com_ptr<ID3D11Buffer> mVertexCBufferPerObject;
		winrt::com_ptr<ID3D11Buffer> mPixelCBufferPerFrame;
		winrt::com_ptr<ID3D11Buffer> mPixelCBufferPerObject;
		VertexCBufferPerFrame mVertexCBufferPerFrameData;
		VertexCBufferPerObject mVertexCBufferPerObjectData;
		PixelCBufferPerFrame mPixelCBufferPerFrameData;
		PixelCBufferPerObject mPixelCBufferPerObjectData;		
		std::shared_ptr<Library::Texture2D> mColorMap;
		std::shared_ptr<Library::Texture2D> mSpecularMap;
		std::shared_ptr<Library::Texture2D> mTransparencyMap;
		winrt::com_ptr<ID3D11SamplerState> mSamplerState{ Library::SamplerStates::TrilinearClamp };
		Library::RenderStateHelper mRenderStateHelper;
		bool mVertexCBufferPerFrameDataDirty{ true };
		bool mPixelCBufferPerFrameDataDirty{ true };
		bool mPixelCBufferPerObjectDataDirty{ true };
	};
}