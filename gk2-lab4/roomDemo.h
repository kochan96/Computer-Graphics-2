#pragma once
#include "gk2ExampleBase.h"
#include "constantBuffer.h"
#include "mesh.h"
#include "PhongEffect.h"
#include "particleSystem.h"
#include "lightAndShadowMap.h"

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
#pragma endregion
		ConstantBuffer<DirectX::XMFLOAT4X4> m_cbWorldMtx, //vertex shader constant buffer slot 0
			m_cbProjMtx;	//vertex shader constant buffer slot 2 & geometry shader constant buffer slot 0
		ConstantBuffer<DirectX::XMFLOAT4X4, 2> m_cbViewMtx; //vertex shader constant buffer slot 1
		ConstantBuffer<DirectX::XMFLOAT4> m_cbSurfaceColor;	//pixel shader constant buffer slot 0
		ConstantBuffer<DirectX::XMFLOAT4> m_cbLightPos; //pixel shader constant buffer slot 1

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

		dx_ptr<ID3D11RasterizerState> m_rsCullNone;
		dx_ptr<ID3D11BlendState> m_bsAlpha;
		dx_ptr<ID3D11DepthStencilState> m_dssNoWrite;

		dx_ptr<ID3D11InputLayout> m_inputlayout;

		PhongEffect m_phongEffect;
		LightAndShadowMap m_lightShadowMap;

		ParticleSystem m_particles;

		void UpdateCameraCB();
		void UpdateLamp(float dt);

		void DrawMesh(const Mesh& m, DirectX::XMFLOAT4X4 worldMtx);
		void DrawParticles() const;

		void SetWorldMtx(DirectX::XMFLOAT4X4 mtx);

		void DrawScene();
	};
}