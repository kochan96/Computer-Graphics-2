#pragma once
#include "gk2ExampleBase.h"
#include "constantBuffer.h"
#include "mesh.h"
#include "PhongEffect.h"
#include "TexturedEffect.h"
#include "MultiTexturedEffect.h"
#include "ColorTextureEffect.h"
#include "particleSystem.h"
#include "environmentMapper.h"

namespace mini::gk2
{
	class RoomDemo : public Gk2ExampleBase
	{
	public:
		explicit RoomDemo(HINSTANCE appInstance);

	protected:
		void Update(const Clock& dt) override;
		void Render() override;

	private:
#pragma region CONSTANTS
		static constexpr float TABLE_H = 1.0f;
		static constexpr float TABLE_TOP_H = 0.1f;
		static constexpr float TABLE_R = 1.5f;

		//can't have in-class initializer since XMFLOAT4 constructors are not constexpr
		static const DirectX::XMFLOAT4 TABLE_POS;
		static const DirectX::XMFLOAT4 LIGHT_POS[2];
#pragma endregion
		ConstantBuffer<DirectX::XMFLOAT4X4> m_cbWorldMtx, //vertex shader constant buffer slot 0
			m_cbProjMtx,	//vertex shader constant buffer slot 2 & geometry shader constant buffer slot 0
			m_cbTex1Mtx,	//vertex shader constant buffer slot 3
			m_cbTex2Mtx;	//vertex shader constant buffer slot 4
		ConstantBuffer<DirectX::XMFLOAT4X4, 2> m_cbViewMtx; //vertex shader constant buffer slot 1
		ConstantBuffer<DirectX::XMFLOAT4> m_cbSurfaceColor;	//pixel shader constant buffer slot 0
		ConstantBuffer<DirectX::XMFLOAT4, 2> m_cbLightPos; //pixel shader constant buffer slot 1

		Mesh m_wall; //uses m_wallsMtx[6]
		Mesh m_sphere; //uses m_sphereMtx
		Mesh m_teapot; //uses m_tepotMtx
		Mesh m_box; //uses m_boxMtx
		Mesh m_lamp; //uses m_lampMtx
		Mesh m_chairSeat; //uses m_chairMtx
		Mesh m_chairBack; //uses m_chairMtx
		Mesh m_tableLeg; //uses m_tableLegsMtx[4]
		Mesh m_tableTop; //uses m_tableTopMtx
		Mesh m_tableSide; //uses m_tableSideMtx
		Mesh m_monitor; //uses m_monitorMtx
		Mesh m_screen; //uses m_monitorMtx

		DirectX::XMFLOAT4X4 m_projMtx, m_wallsMtx[6], m_sphereMtx, m_teapotMtx, m_boxMtx, m_lampMtx,
			m_chairMtx, m_tableLegsMtx[4], m_tableTopMtx, m_tableSideMtx, m_monitorMtx;

		dx_ptr<ID3D11SamplerState> m_samplerWrap;
		dx_ptr<ID3D11SamplerState> m_samplerBorder;

		dx_ptr<ID3D11ShaderResourceView> m_wallTexture;
		dx_ptr<ID3D11ShaderResourceView> m_posterTexture;
		dx_ptr<ID3D11ShaderResourceView> m_perlinTexture;
		dx_ptr<ID3D11ShaderResourceView> m_woodTexture;

		dx_ptr<ID3D11RasterizerState> m_rsCullFront;
		dx_ptr<ID3D11BlendState> m_bsAlpha;
		dx_ptr<ID3D11DepthStencilState> m_dssNoWrite;

		dx_ptr<ID3D11InputLayout> m_inputlayout;

		PhongEffect m_phongEffect;
		TexturedEffect m_texturedEffect;
		ColoredTextureEffect m_coloredTextureEffect;
		MultiTexturedEffect m_multiTexturedEffect;

		EnvironmentMapper m_envMapper;

		ParticleSystem m_particles;

		void UpdateCameraCB();
		void UpdateLamp(float dt);

		void DrawMesh(const Mesh& m, DirectX::XMFLOAT4X4 worldMtx);
		void DrawParticles() const;

		void SetWorldMtx(DirectX::XMFLOAT4X4 mtx);
		void SetSurfaceColor(DirectX::XMFLOAT4 color);

		void DrawWalls();
		void DrawTeapot();
		void DrawTableElement(Mesh& m, DirectX::XMFLOAT4X4 worldMtx);
		void DrawTableLegs(DirectX::XMVECTOR camVec);
		void DrawTransparentObjects();
		void DrawScene();
	};
}