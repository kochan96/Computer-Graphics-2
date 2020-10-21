#pragma once
#include "TexturedEffect.h"

namespace mini
{
	namespace gk2
	{
		class ColoredTextureEffect : public StaticEffect<TexturedEffect, PSConstantBuffers>
		{
		public:
			enum PSConstantBufferSlots
			{
				SurfaceColorSlot
			};

			ColoredTextureEffect() = default;

			ColoredTextureEffect(dx_ptr<ID3D11VertexShader>&& vs, dx_ptr<ID3D11PixelShader>&& ps, const ConstantBuffer<DirectX::XMFLOAT4X4>& cbWorld,
				const ConstantBuffer<DirectX::XMFLOAT4X4, 2>& cbView, const ConstantBuffer<DirectX::XMFLOAT4X4>& cbProj,
				const ConstantBuffer<DirectX::XMFLOAT4X4>& cbTextureMtx, const ConstantBuffer<DirectX::XMFLOAT4>& cbSurfaceColor,
				const dx_ptr<ID3D11SamplerState>& sampler, const dx_ptr<ID3D11ShaderResourceView>& texture)
				: StaticEffect(TexturedEffect(move(vs), move(ps), cbWorld, cbView, cbProj, cbTextureMtx, sampler, texture),
					PSConstantBuffers{ cbSurfaceColor })
			{ }

			void SetSurfaceColorBuffer(const ConstantBuffer<DirectX::XMFLOAT4, 2>& buffer) { SetPSConstantBuffer(SurfaceColorSlot, buffer); }
		};
	}
}
