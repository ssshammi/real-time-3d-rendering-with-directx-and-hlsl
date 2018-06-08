#pragma once

#include <gsl\gsl>
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <wrl.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <cstdint>
#include <memory>
#include "DrawableGameComponent.h"
#include "MatrixHelper.h"
#include "SpotLight.h"

namespace Library
{
	class Camera;
	class Mesh;
	class ProxyModel;
	class VertexShader;
	class PixelShader;
	struct VertexPositionTextureNormal;
}

namespace Rendering
{
	class SpotLightMaterial;

	class SpotLightDemo final : public Library::DrawableGameComponent
	{
	public:
		SpotLightDemo(Library::Game& game, const std::shared_ptr<Library::Camera>& camera);
		SpotLightDemo(const SpotLightDemo&) = delete;
		SpotLightDemo(SpotLightDemo&&) = default;
		SpotLightDemo& operator=(const SpotLightDemo&) = default;		
		SpotLightDemo& operator=(SpotLightDemo&&) = default;
		~SpotLightDemo();

		float AmbientLightIntensity() const;
		void SetAmbientLightIntensity(float intensity);

		float SpotLightIntensity() const;
		void SetSpotLightIntensity(float intensity);

		const DirectX::XMFLOAT3& LightPosition() const;
		const DirectX::XMVECTOR LightPositionVector() const;
		void SetLightPosition(const DirectX::XMFLOAT3& position);
		void SetLightPosition(DirectX::FXMVECTOR position);

		const DirectX::XMFLOAT3& LightLookAt() const;
		void RotateSpotLight(const DirectX::XMFLOAT2& amount);

		float LightRadius() const;
		void SetLightRadius(float radius);

		float SpecularIntensity() const;
		void SetSpecularIntensity(float intensity);

		float SpecularPower() const;
		void SetSpecularPower(float power);

		float SpotLightInnerAngle() const;
		void SetSpotLightInnerAngle(float angle);

		float SpotLightOuterAngle() const;
		void SetSpotLightOuterAngle(float angle);

		virtual void Initialize() override;
		virtual void Update(const Library::GameTime& gameTime) override;
		virtual void Draw(const Library::GameTime& gameTime) override;

	private:
		void CreateVertexBuffer(const gsl::span<const Library::VertexPositionTextureNormal>& vertices, gsl::not_null<ID3D11Buffer**> vertexBuffer) const;

		std::shared_ptr<SpotLightMaterial> mMaterial;
		DirectX::XMFLOAT4X4 mWorldMatrix{ Library::MatrixHelper::Identity };
		Microsoft::WRL::ComPtr<ID3D11Buffer> mVertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mColorTexture;
		std::uint32_t mVertexCount{ 0 };
		Library::SpotLight mSpotLight;
		std::unique_ptr<Library::ProxyModel> mProxyModel;
		float mModelRotationAngle{ 0.0f };
		bool mUpdateMaterial{ true };
	};
}