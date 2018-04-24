#include "pch.h"
#include "RenderTarget.h"

using namespace std;
using namespace gsl;

namespace Library
{
    RTTI_DEFINITIONS(RenderTarget)

    stack<RenderTarget::RenderTargetData> RenderTarget::sRenderTargetStack;

	void RenderTarget::Begin(not_null<ID3D11DeviceContext*> deviceContext, uint32_t viewCount, not_null<ID3D11RenderTargetView**> renderTargetViews, not_null<ID3D11DepthStencilView*> depthStencilView, const D3D11_VIEWPORT& viewport)
	{
		sRenderTargetStack.push(RenderTargetData(viewCount, renderTargetViews, depthStencilView, viewport));
		deviceContext->OMSetRenderTargets(viewCount, renderTargetViews, depthStencilView);
		deviceContext->RSSetViewports(1, &viewport);
	}

	void RenderTarget::End(not_null<ID3D11DeviceContext*> deviceContext)
	{
		sRenderTargetStack.pop();

		RenderTargetData renderTargetData = sRenderTargetStack.top();
		deviceContext->OMSetRenderTargets(renderTargetData.ViewCount, renderTargetData.RenderTargetViews, renderTargetData.DepthStencilView);
		deviceContext->RSSetViewports(1, &renderTargetData.Viewport);
	}

	void RenderTarget::RebindCurrentRenderTargets(not_null<ID3D11DeviceContext*> deviceContext)
	{
		RenderTargetData renderTargetData = sRenderTargetStack.top();
		deviceContext->OMSetRenderTargets(renderTargetData.ViewCount, renderTargetData.RenderTargetViews, renderTargetData.DepthStencilView);
		deviceContext->RSSetViewports(1, &renderTargetData.Viewport);
	}
}