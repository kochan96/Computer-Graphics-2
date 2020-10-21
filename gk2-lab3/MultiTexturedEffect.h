#pragma once
#include "effect.h"
#include "constantBuffer.h"
#include <DirectXMath.h>
#include "TexturedEffect.h"

namespace mini
{
	namespace gk2
	{
		class MultiTexturedEffect : public TexturedEffect
		{
		public:
			enum VSConstantBufferSlots
			{
				WorldMtxSlot,
				ViewMtxSlot,
				ProjMtxSlot,
				TextureMtxSlot,
				TextureMtxSlot1
			};

			enum PSSamplerSlots
			{
				TextureSamplerSlot
			};

			enum PSTextureSlots
			{
				TextureSlot,
				TextureSlot1
			};

			MultiTexturedEffect() = default;

			MultiTexturedEffect(dx_ptr<ID3D11VertexShader>&& vs, dx_ptr<ID3D11PixelShader>&& ps,
				const ConstantBuffer<DirectX::XMFLOAT4X4>& cbWorld,
				const ConstantBuffer<DirectX::XMFLOAT4X4, 2>& cbView, 
				const ConstantBuffer<DirectX::XMFLOAT4X4>& cbProj,
				const ConstantBuffer<DirectX::XMFLOAT4X4>& cbTextureMtx,
				const ConstantBuffer<DirectX::XMFLOAT4X4>& cbTextureMtx1,
				const dx_ptr<ID3D11SamplerState>& textureSampler,
				const dx_ptr<ID3D11ShaderResourceView>& texture,
				const dx_ptr<ID3D11ShaderResourceView>& texture1)
				: TexturedEffect(move(vs), move(ps), 
					cbWorld, cbView, cbProj, cbTextureMtx,
					textureSampler, texture)
			{ 
				SetTextureMatrix1Buffer(cbTextureMtx1);
				SetTexture1(texture1);
			}
			
			void SetTextureMatrix1Buffer(const ConstantBuffer<DirectX::XMFLOAT4X4>& buffer)
			{
				SetVSConstantBuffer(TextureMtxSlot1, buffer);
			}
			void SetTexture1(const dx_ptr<ID3D11ShaderResourceView>& texture)
			{
				SetPSShaderResource(TextureSlot1, texture);
			}
		};
	}
}
