#pragma once
#include "PhongEffect.h"
#include <DirectXMath.h>

namespace mini
{
	namespace gk2
	{
		class LightAndShadowMap : public StaticEffect<PhongEffect, PSShaderResources, PSSamplers>
		{
		public:
			static constexpr unsigned int TEXTURE_SIZE = 1024;
			static constexpr float LIGHT_NEAR = 0.35f;
			static constexpr float LIGHT_FAR = 5.5f;

			//can't have in-class initializer since XM_PI is not constexpr
			static const float LIGHT_FOV_ANGLE;

			LightAndShadowMap() = default;

			LightAndShadowMap(const DxDevice& device, const ConstantBuffer<DirectX::XMFLOAT4X4>& cbWorld,
				const ConstantBuffer<DirectX::XMFLOAT4X4, 2> cbView, const ConstantBuffer<DirectX::XMFLOAT4X4>& cbProj,
				const ConstantBuffer<DirectX::XMFLOAT4> cbLightPos, const ConstantBuffer<DirectX::XMFLOAT4> cbSurfaceColor);

			DirectX::XMFLOAT4 UpdateLightPosition(const dx_ptr<ID3D11DeviceContext>& context, DirectX::XMMATRIX lightMtx);

			void BeginShadowRender(const dx_ptr<ID3D11DeviceContext>& context, ConstantBuffer<DirectX::XMFLOAT4X4, 2>& cbView,
				ConstantBuffer<DirectX::XMFLOAT4X4>& cbProj) const;

		private:
			dx_ptr<ID3D11RenderTargetView> m_shadowRenderTarget;
			dx_ptr<ID3D11DepthStencilView> m_shadowDepthBuffer;
			ConstantBuffer<DirectX::XMFLOAT4X4> m_cbMapMtx;
			DirectX::XMFLOAT4X4 m_lightViewMtx[2], m_lightProjMtx;
		};
	}
}
