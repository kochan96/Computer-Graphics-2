#include "roomDemo.h"
#include <array>
#include "meshLoader.h"

using namespace mini;
using namespace utils;
using namespace gk2;
using namespace DirectX;
using namespace std;

const XMFLOAT4 RoomDemo::TABLE_POS{ 0.5f, -0.96f, 0.5f, 1.0f };

RoomDemo::RoomDemo(HINSTANCE appInstance)
	: Gk2ExampleBase(appInstance, 1280, 720, L"Pokój"), 
	//Constant Buffers
	m_cbWorldMtx(m_device.CreateConstantBuffer<XMFLOAT4X4>()),
	m_cbProjMtx(m_device.CreateConstantBuffer<XMFLOAT4X4>()),
	m_cbViewMtx(m_device.CreateConstantBuffer<XMFLOAT4X4, 2>()),
	m_cbSurfaceColor(m_device.CreateConstantBuffer<XMFLOAT4>()),
	m_cbLightPos(m_device.CreateConstantBuffer<XMFLOAT4>())
{
	//Projection matrix
	auto s = m_window.getClientSize();
	auto ar = static_cast<float>(s.cx) / s.cy;
	XMStoreFloat4x4(&m_projMtx, XMMatrixPerspectiveFovLH(XM_PIDIV4, ar, 0.01f, 100.0f));
	m_cbProjMtx.Update(m_device.context(), m_projMtx);
	UpdateCameraCB();

	//Meshes
	vector<VertexPositionNormal> vertices;
	vector<unsigned short> indices;
	tie(vertices, indices) = MeshLoader::CreateSquare(4.0f);
	m_wall = m_device.CreateMesh(indices, vertices);
	tie(vertices, indices) = MeshLoader::LoadMesh(L"resources/meshes/teapot.mesh");
	m_teapot = m_device.CreateMesh(indices, vertices);
	tie(vertices, indices) = MeshLoader::CreateSphere(0.3f, 8, 16);
	m_sphere = m_device.CreateMesh(indices, vertices);
	tie(vertices, indices) = MeshLoader::CreateBox();
	m_box = m_device.CreateMesh(indices, vertices);
	tie(vertices, indices) = MeshLoader::LoadMesh(L"resources/meshes/lamp.mesh");
	m_lamp = m_device.CreateMesh(indices, vertices);
	tie(vertices, indices) = MeshLoader::LoadMesh(L"resources/meshes/chair_seat.mesh");
	m_chairSeat = m_device.CreateMesh(indices, vertices);
	tie(vertices, indices) = MeshLoader::LoadMesh(L"resources/meshes/chair_back.mesh");
	m_chairBack = m_device.CreateMesh(indices, vertices);
	tie(vertices, indices) = MeshLoader::LoadMesh(L"resources/meshes/monitor.mesh");
	m_monitor = m_device.CreateMesh(indices, vertices);
	tie(vertices, indices) = MeshLoader::LoadMesh(L"resources/meshes/screen.mesh");
	m_screen = m_device.CreateMesh(indices, vertices);
	tie(vertices, indices) = MeshLoader::CreateCylinder(0.1f, TABLE_H - TABLE_TOP_H, 4, 9);
	m_tableLeg = m_device.CreateMesh(indices, vertices);
	tie(vertices, indices) = MeshLoader::CreateCylinder(TABLE_R, TABLE_TOP_H, 1, 16);
	m_tableSide = m_device.CreateMesh(indices, vertices);
	tie(vertices, indices) = MeshLoader::CreateDisk(TABLE_R, 16);
	m_tableTop = m_device.CreateMesh(indices, vertices);

	//World matrix of all objects
	auto temp = XMMatrixTranslation(0.0f, 0.0f, 2.0f);
	auto a = 0.f;
	for (auto i = 0U; i < 4U; ++i, a += XM_PIDIV2)
		XMStoreFloat4x4(&m_wallsMtx[i], temp * XMMatrixRotationY(a));
	XMStoreFloat4x4(&m_wallsMtx[4], temp * XMMatrixRotationX(XM_PIDIV2));
	XMStoreFloat4x4(&m_wallsMtx[5], temp * XMMatrixRotationX(-XM_PIDIV2));
	XMStoreFloat4x4(&m_teapotMtx, XMMatrixTranslation(0.0f, -2.3f, 0.f) * XMMatrixScaling(0.1f, 0.1f, 0.1f) *
		XMMatrixRotationY(-XM_PIDIV2) * XMMatrixTranslation(-1.3f, -0.74f, -0.6f));
	
	XMStoreFloat4x4(&m_sphereMtx, XMMatrixRotationY(-XM_PIDIV2) * XMMatrixTranslation(-1.3f, -0.74f, -0.6f));
	XMStoreFloat4x4(&m_boxMtx, XMMatrixTranslation(-1.4f, -1.46f, -0.6f));
	XMStoreFloat4x4(&m_chairMtx, XMMatrixRotationY(XM_PI + XM_PI / 9 ) *
		XMMatrixTranslation(-0.1f, -1.06f, -1.3f));
	XMStoreFloat4x4(&m_monitorMtx, XMMatrixRotationY(XM_PIDIV4) *
		XMMatrixTranslation(TABLE_POS.x, TABLE_POS.y + 0.42f, TABLE_POS.z));
	a = XM_PIDIV4;
	for (auto i = 0U; i < 4U; ++i, a += XM_PIDIV2)
		XMStoreFloat4x4(&m_tableLegsMtx[i], XMMatrixTranslation(0.0f, 0.0f, TABLE_R - 0.35f) * XMMatrixRotationY(a) *
			XMMatrixTranslation(TABLE_POS.x, TABLE_POS.y - (TABLE_H + TABLE_TOP_H) / 2, TABLE_POS.z));
	XMStoreFloat4x4(&m_tableSideMtx, XMMatrixRotationY(XM_PIDIV4 / 4) *
		XMMatrixTranslation(TABLE_POS.x, TABLE_POS.y - TABLE_TOP_H / 2, TABLE_POS.z));
	XMStoreFloat4x4(&m_tableTopMtx, XMMatrixRotationY(XM_PIDIV4 / 4) *
		XMMatrixTranslation(TABLE_POS.x, TABLE_POS.y, TABLE_POS.z));

	//Constant buffers content
	m_cbSurfaceColor.Update(m_device.context(), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

	//Render states
	RasterizerDescription rsDesc;
	rsDesc.CullMode = D3D11_CULL_NONE;
	m_rsCullNone = m_device.CreateRasterizerState(rsDesc);

	m_bsAlpha = m_device.CreateBlendState(BlendDescription::AlphaBlendDescription());
	DepthStencilDescription dssDesc;
	dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	m_dssNoWrite = m_device.CreateDepthStencilState(dssDesc);

	auto vsCode = m_device.LoadByteCode(L"phongVS.cso");
	auto psCode = m_device.LoadByteCode(L"phongPS.cso");
	m_phongEffect = PhongEffect(m_device.CreateVertexShader(vsCode), m_device.CreatePixelShader(psCode),
		m_cbWorldMtx, m_cbViewMtx, m_cbProjMtx, m_cbLightPos, m_cbSurfaceColor);
	m_inputlayout = m_device.CreateInputLayout(VertexPositionNormal::Layout, vsCode);

	m_lightShadowMap = LightAndShadowMap(m_device, m_cbWorldMtx, m_cbViewMtx, m_cbProjMtx, m_cbLightPos, m_cbSurfaceColor);
	m_particles = ParticleSystem(m_device, m_cbViewMtx, m_cbProjMtx, XMFLOAT3(-1.3f, -0.6f, -0.14f));

	m_device.context()->IASetInputLayout(m_inputlayout.get());
	m_device.context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	UpdateLamp(0.0f);
}

void RoomDemo::UpdateCameraCB()
{
	XMMATRIX viewMtx = m_camera.getViewMatrix();
	XMVECTOR det;
	XMMATRIX invViewMtx = XMMatrixInverse(&det, viewMtx);
	XMFLOAT4X4 view[2];
	XMStoreFloat4x4(view, viewMtx);
	XMStoreFloat4x4(view + 1, invViewMtx);
	m_cbViewMtx.Update(m_device.context(), view);
}

void RoomDemo::UpdateLamp(float dt)
{
	static auto time = 0.0f;
	time += dt;
	auto swing = 0.3f * XMScalarSin(XM_2PI*time / 8);
	auto rot = XM_2PI*time / 20;
	auto lamp = XMMatrixTranslation(0.0f, -0.4f, 0.0f) * XMMatrixRotationX(swing) * XMMatrixRotationY(rot) *
		XMMatrixTranslation(0.0f, 2.0f, 0.0f);
	XMFLOAT4 lightPos = m_lightShadowMap.UpdateLightPosition(m_device.context(), lamp);
	m_cbLightPos.Update(m_device.context(), lightPos);
	XMStoreFloat4x4(&m_lampMtx, lamp);
}

void RoomDemo::Update(const Clock& c)
{
	double dt = c.getFrameTime();
	HandleCameraInput(dt);
	UpdateLamp(static_cast<float>(dt));
	m_particles.Update(m_device.context(), static_cast<float>(dt), m_camera.getCameraPosition());
}

void RoomDemo::SetWorldMtx(DirectX::XMFLOAT4X4 mtx)
{
	m_cbWorldMtx.Update(m_device.context(), mtx);
}

void RoomDemo::DrawMesh(const Mesh& m, DirectX::XMFLOAT4X4 worldMtx)
{
	SetWorldMtx(worldMtx);
	m.Render(m_device.context());
}

void RoomDemo::DrawParticles() const
{
	m_particles.Render(m_device.context());
	//Particles use a geometry shader and different input layout and topology
	//which need to be reset before drawing anything else
	m_device.context()->GSSetShader(nullptr, nullptr, 0);
	m_device.context()->IASetInputLayout(m_inputlayout.get());
	m_device.context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void RoomDemo::DrawScene()
{
	//Draw walls
	for (auto& wallMtx : m_wallsMtx)
		DrawMesh(m_wall, wallMtx);
	//Draw teapot
	DrawMesh(m_teapot, m_teapotMtx);

	//Draw shelf
	DrawMesh(m_box, m_boxMtx);
	//Draw lamp
	DrawMesh(m_lamp, m_lampMtx);
	//Draw chair seat
	DrawMesh(m_chairSeat, m_chairMtx);
	//Draw chairframe
	DrawMesh(m_chairBack, m_chairMtx);
	//Draw monitor
	DrawMesh(m_monitor, m_monitorMtx);
	//Draw screen
	DrawMesh(m_screen, m_monitorMtx);
	m_device.context()->RSSetState(m_rsCullNone.get());
	//Draw table top
	DrawMesh(m_tableTop, m_tableTopMtx);
	//Draw table side
	DrawMesh(m_tableSide, m_tableSideMtx);
	//Draw table legs
	for (auto& tableLegMtx : m_tableLegsMtx)
		DrawMesh(m_tableLeg, tableLegMtx);
	m_device.context()->RSSetState(nullptr);
}

void RoomDemo::Render()
{
	Gk2ExampleBase::Render();

	// TODO : 3.15 Render objects and particles to the shadow map using a Phong Effect
	m_lightShadowMap.BeginShadowRender(m_device.context(), m_cbViewMtx, m_cbProjMtx);
	m_phongEffect.Begin(m_device.context());

	DrawScene();
	DrawParticles();

	getDefaultRenderTarget().Begin(m_device.context());
	m_cbProjMtx.Update(m_device.context(), m_projMtx);
	UpdateCameraCB();

	//TODO : 3.07 Replace with lightShadowEffect
	m_lightShadowMap.Begin(m_device.context());

	DrawScene();

	m_device.context()->OMSetBlendState(m_bsAlpha.get(), nullptr, UINT_MAX);
	m_device.context()->OMSetDepthStencilState(m_dssNoWrite.get(), 0);
	DrawParticles();
	m_device.context()->OMSetBlendState(nullptr, nullptr, UINT_MAX);
	m_device.context()->OMSetDepthStencilState(nullptr, 0);
}