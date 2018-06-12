#pragma once

#include <cstdint>
#include <d3d11.h>
#include <DirectXMath.h>
#include <gsl\gsl>

namespace Library
{
	void CreateIndexBuffer(gsl::not_null<ID3D11Device*> device, const gsl::span<const std::uint16_t>& indices, gsl::not_null<ID3D11Buffer**> indexBuffer);
	void CreateIndexBuffer(gsl::not_null<ID3D11Device*> device, const gsl::span<const std::uint32_t>& indices, gsl::not_null<ID3D11Buffer**> indexBuffer);

	inline float ConvertDipsToPixels(float dips, float dpi)
	{
		static const float dipsPerInch = 96.0f;
		return floorf(dips * dpi / dipsPerInch + 0.5f); // Round to nearest integer.
	}

#if defined(_DEBUG)
	// Check for SDK Layer support.
	inline bool SdkLayersAvailable()
	{
		HRESULT hr = D3D11CreateDevice(
			nullptr,
			D3D_DRIVER_TYPE_NULL,       // There is no need to create a real hardware device.
			0,
			D3D11_CREATE_DEVICE_DEBUG,  // Check for the SDK layers.
			nullptr,                    // Any feature level will do.
			0,
			D3D11_SDK_VERSION,          // Always set this to D3D11_SDK_VERSION for Windows Store apps.
			nullptr,                    // No need to keep the D3D device reference.
			nullptr,                    // No need to know the feature level.
			nullptr                     // No need to keep the D3D device context reference.
			);

		return SUCCEEDED(hr);
	}
#endif
}