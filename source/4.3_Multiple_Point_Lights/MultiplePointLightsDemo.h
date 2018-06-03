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
#include <array>
#include "DrawableGameComponent.h"
#include "MatrixHelper.h"

namespace Library
{
	class Camera;
	class PointLight;
	class Mesh;
	class ProxyModel;
	class VertexShader;
	class PixelShader;
}

namespace Rendering
{
	class MultiplePointLightsMaterial;

	class MultiplePointLightsDemo final : public Library::DrawableGameComponent
	{
	public:
		MultiplePointLightsDemo(Library::Game& game, const std::shared_ptr<Library::Camera>& camera);
		MultiplePointLightsDemo(const MultiplePointLightsDemo&) = delete;
		MultiplePointLightsDemo(MultiplePointLightsDemo&&) = default;
		MultiplePointLightsDemo& operator=(const MultiplePointLightsDemo&) = default;		
		MultiplePointLightsDemo& operator=(MultiplePointLightsDemo&&) = default;
		~MultiplePointLightsDemo();

		bool AnimationEnabled() const;
		void SetAnimationEnabled(bool enabled);
		void ToggleAnimation();

		float AmbientLightIntensity() const;
		void SetAmbientLightIntensity(float intensity);

		const std::array<Library::PointLight, 4>& PointLights() const;
		void SetPointLight(const Library::PointLight& light, std::uint32_t index);
		
		float SpecularIntensity() const;
		void SetSpecularIntensity(float intensity);

		float SpecularPower() const;
		void SetSpecularPower(float power);

		virtual void Initialize() override;
		virtual void Update(const Library::GameTime& gameTime) override;
		virtual void Draw(const Library::GameTime& gameTime) override;

	private:
		void CreateVertexBuffer(gsl::not_null<ID3D11Device*> device, const Library::Mesh& mesh, gsl::not_null<ID3D11Buffer**> vertexBuffer) const;

		inline static const float RotationRate{ DirectX::XM_PI };

		std::shared_ptr<MultiplePointLightsMaterial> mMaterial;
		DirectX::XMFLOAT4X4 mWorldMatrix{ Library::MatrixHelper::Identity };
		Microsoft::WRL::ComPtr<ID3D11Buffer> mVertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mIndexBuffer;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mColorTexture;
		std::uint32_t mIndexCount{ 0 };
		std::array<std::unique_ptr<Library::ProxyModel>, 4> mProxyModels;
		float mModelRotationAngle{ 0.0f };
		bool mAnimationEnabled{ true };
		bool mUpdateMaterial{ true };
	};
}