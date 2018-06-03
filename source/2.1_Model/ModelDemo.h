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

namespace Library
{
	class Camera;
	class Mesh;
}

namespace Rendering
{
	class ModelDemo final : public Library::DrawableGameComponent
	{
	public:
		ModelDemo(Library::Game& game, const std::shared_ptr<Library::Camera>& camera);

		bool AnimationEnabled() const;
		void SetAnimationEnabled(bool enabled);

		virtual void Initialize() override;
		virtual void Update(const Library::GameTime& gameTime) override;
		virtual void Draw(const Library::GameTime& gameTime) override;

	private:
		struct CBufferPerObject
		{
			DirectX::XMFLOAT4X4 WorldViewProjection;
		};

		void CreateVertexBuffer(const Library::Mesh& mesh, gsl::not_null<ID3D11Buffer**> vertexBuffer) const;

		inline static const float RotationRate{ DirectX::XM_PI };

		DirectX::XMFLOAT4X4 mWorldMatrix{ Library::MatrixHelper::Identity };
		CBufferPerObject mCBufferPerObject;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> mVertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> mPixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> mInputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mVertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mIndexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mConstantBuffer;		
		std::uint32_t mIndexCount{ 0 };
		float mRotationAngle{ 0.0f };
		bool mAnimationEnabled{ true };
		bool mUpdateConstantBuffer{ true };
	};
}