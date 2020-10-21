#pragma once
#include "effect.h"
#include <DirectXMath.h>
#include "effects.h"

namespace mini
{
	namespace gk2
	{
		class EnvironmentMapper : public BasicEffect, public VSConstantBuffers, public PSConstantBuffers, public PSSamplers, private PSShaderResources
		{
		public:
			enum VSConstantBufferSlots
			{
				WorldMtxSlot,
				ViewMtxSlot,
				ProjMtxSlot
			};

			enum PSConstantBufferSlots
			{
				SurfaceColorSlot
			};

			enum PSSamplerSlots
			{
				TextureSamplerSlot
			};

			static const int TEXTURE_SIZE;

			EnvironmentMapper() = default;

			EnvironmentMapper(const DxDevice& device,
				dx_ptr<ID3D11VertexShader>&& vs,
				dx_ptr<ID3D11PixelShader>&& ps,
				const ConstantBuffer<DirectX::XMFLOAT4X4>& cbWorld,
				const ConstantBuffer<DirectX::XMFLOAT4X4, 2>& cbView,
				const ConstantBuffer<DirectX::XMFLOAT4X4>& cbProj,
				const ConstantBuffer<DirectX::XMFLOAT4>& cbSurfaceColor,
				const dx_ptr<ID3D11SamplerState>& sampler,
				float nearPlane, float farPlane, DirectX::XMFLOAT3 position);

			void Begin(const dx_ptr<ID3D11DeviceContext>& context) const override;
			
			void SetWorldMatrixBuffer(const ConstantBuffer<DirectX::XMFLOAT4X4>& buffer)
			{
				SetVSConstantBuffer(WorldMtxSlot, buffer);
			}
			void SetViewMatrixBuffer(const ConstantBuffer<DirectX::XMFLOAT4X4, 2>& buffer)
			{
				SetVSConstantBuffer(ViewMtxSlot, buffer);
			}
			void SetProjMatrixBuffer(const ConstantBuffer<DirectX::XMFLOAT4X4>& buffer)
			{
				SetVSConstantBuffer(ProjMtxSlot, buffer);
			}

			void SetTextureSampler(const dx_ptr<ID3D11SamplerState>& sampler)
			{
				SetPSSampler(TextureSamplerSlot, sampler);
			}
			void SetSurfaceColorBuffer(const ConstantBuffer<DirectX::XMFLOAT4, 2>& buffer)
			{
				SetPSConstantBuffer(SurfaceColorSlot, buffer);
			}

			void BeginFace(const dx_ptr<ID3D11DeviceContext>& context,
				ConstantBuffer<DirectX::XMFLOAT4X4, 2>& cbView,
				ConstantBuffer<DirectX::XMFLOAT4X4>& cbProj,
				D3D11_TEXTURECUBE_FACE face);
			void EndFace(const dx_ptr<ID3D11DeviceContext>& context);

		private:
			float m_nearPlane;
			float m_farPlane;
			DirectX::XMFLOAT4 m_position;
			D3D11_TEXTURECUBE_FACE m_face;

			dx_ptr<ID3D11Texture2D> m_envTexture, m_faceTexture;
			dx_ptr<ID3D11RenderTargetView> m_renderTarget;
			dx_ptr<ID3D11DepthStencilView> m_depthBuffer;
		};
	}
}