#include "butterflyDemo.h"
#include "meshLoader.h"

using namespace mini;
using namespace utils;
using namespace gk2;
using namespace DirectX;
using namespace std;

#pragma region Constants
const float ButterflyDemo::DODECAHEDRON_R = sqrt(0.375f + 0.125f * sqrt(5.0f));
const float ButterflyDemo::DODECAHEDRON_H = 1.0f + 2.0f * DODECAHEDRON_R;
const float ButterflyDemo::DODECAHEDRON_A = XMScalarACos(-0.2f * sqrt(5.0f));

const float ButterflyDemo::MOEBIUS_R = 1.0f;
const float ButterflyDemo::MOEBIUS_W = 0.1f;
const unsigned int ButterflyDemo::MOEBIUS_N = 128;

const float ButterflyDemo::LAP_TIME = 20.0f;
const float ButterflyDemo::FLAP_TIME = 2.0f;
const float ButterflyDemo::WING_W = 0.15f;
const float ButterflyDemo::WING_H = 0.1f;
const float ButterflyDemo::WING_MAX_A = 8.0f * XM_PIDIV2 / 9.0f; //80 degrees
const float ButterflyDemo::BILBOARD_SCALE = 0.3f;

const unsigned int ButterflyDemo::BS_MASK = 0xffffffff;

const XMFLOAT4 ButterflyDemo::GREEN_LIGHT_POS = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
const XMFLOAT4 ButterflyDemo::BLUE_LIGHT_POS = XMFLOAT4(-1.0f, -1.0f, -1.0f, 1.0f);
const XMFLOAT4 ButterflyDemo::GREEN_COLOR = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
const XMFLOAT4 ButterflyDemo::BLUE_COLOR = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
const XMFLOAT4 ButterflyDemo::WHITE_COLOR = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

const XMFLOAT4 ButterflyDemo::COLORS[] = {
	XMFLOAT4(253.0f / 255.0f, 198.0f / 255.0f, 137.0f / 255.0f, 100.0f / 255.0f),
	XMFLOAT4(255.0f / 255.0f, 247.0f / 255.0f, 153.0f / 255.0f, 100.0f / 255.0f),
	XMFLOAT4(196.0f / 255.0f, 223.0f / 255.0f, 155.0f / 255.0f, 100.0f / 255.0f),
	XMFLOAT4(162.0f / 255.0f, 211.0f / 255.0f, 156.0f / 255.0f, 100.0f / 255.0f),
	XMFLOAT4(130.0f / 255.0f, 202.0f / 255.0f, 156.0f / 255.0f, 100.0f / 255.0f),
	XMFLOAT4(122.0f / 255.0f, 204.0f / 255.0f, 200.0f / 255.0f, 100.0f / 255.0f),
	XMFLOAT4(109.0f / 255.0f, 207.0f / 255.0f, 246.0f / 255.0f, 100.0f / 255.0f),
	XMFLOAT4(125.0f / 255.0f, 167.0f / 255.0f, 216.0f / 255.0f, 100.0f / 255.0f),
	XMFLOAT4(131.0f / 255.0f, 147.0f / 255.0f, 202.0f / 255.0f, 100.0f / 255.0f),
	XMFLOAT4(135.0f / 255.0f, 129.0f / 255.0f, 189.0f / 255.0f, 100.0f / 255.0f),
	XMFLOAT4(161.0f / 255.0f, 134.0f / 255.0f, 190.0f / 255.0f, 100.0f / 255.0f),
	XMFLOAT4(244.0f / 255.0f, 154.0f / 255.0f, 193.0f / 255.0f, 100.0f / 255.0f)
};
#pragma endregion

#pragma region Initalization
ButterflyDemo::ButterflyDemo(HINSTANCE hInstance)
	: Gk2ExampleBase(hInstance, 1280, 720, L"Motyl"),
	m_cbWorld(m_device.CreateConstantBuffer<XMFLOAT4X4>()),
	m_cbView(m_device.CreateConstantBuffer<XMFLOAT4X4, 2>()),
	m_cbLighting(m_device.CreateConstantBuffer<Lighting>()),
	m_cbSurfaceColor(m_device.CreateConstantBuffer<XMFLOAT4>())

{
	//Projection matrix
	auto s = m_window.getClientSize();
	auto ar = static_cast<float>(s.cx) / s.cy;
	XMStoreFloat4x4(&m_projMtx, XMMatrixPerspectiveFovLH(XM_PIDIV4, ar, 0.01f, 100.0f));
	m_cbProj = m_device.CreateConstantBuffer<XMFLOAT4X4>(m_projMtx);
	XMFLOAT4X4 cameraMtx;
	XMStoreFloat4x4(&cameraMtx, m_camera.getViewMatrix());
	UpdateCameraCB(cameraMtx);

	//Regular shaders
	auto vsCode = m_device.LoadByteCode(L"vs.cso");
	auto psCode = m_device.LoadByteCode(L"ps.cso");
	m_vs = m_device.CreateVertexShader(vsCode);
	m_ps = m_device.CreatePixelShader(psCode);
	m_il = m_device.CreateInputLayout(VertexPositionNormal::Layout, vsCode);

	//Bilboard shaders
	vsCode = m_device.LoadByteCode(L"vsBilboard.cso");
	psCode = m_device.LoadByteCode(L"psBilboard.cso");
	m_vsBilboard = m_device.CreateVertexShader(vsCode);
	m_psBilboard = m_device.CreatePixelShader(psCode);
	D3D11_INPUT_ELEMENT_DESC elements[1] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};
	m_ilBilboard = m_device.CreateInputLayout(elements, vsCode);

	//Render states
	CreateRenderStates();

	//Meshes
	vector<VertexPositionNormal> vertices;
	vector<unsigned short> indices;
	tie(vertices, indices) = MeshLoader::CreateBox();
	m_box = m_device.CreateMesh(indices, vertices);
	tie(vertices, indices) = MeshLoader::CreatePentagon();
	m_pentagon = m_device.CreateMesh(indices, vertices);
	tie(vertices, indices) = MeshLoader::CreateDoubleSidedSquare(2.0);
	m_wing = m_device.CreateMesh(indices, vertices);
	CreateMoebuisStrip();

	vector<XMFLOAT3> bilboardVertices;
	tie(bilboardVertices, indices) = MeshLoader::CreateSquareBilboard(2.0f);
	m_bilboard = m_device.CreateMesh(indices, bilboardVertices);

	//Model matrices
	CreateDodecahadronMtx();

	SetShaders();
	ID3D11Buffer* vsb[] = { m_cbWorld,  m_cbView, m_cbProj };
	m_device.context()->VSSetConstantBuffers(0, 3, vsb);
	ID3D11Buffer* psb[] = { m_cbSurfaceColor, m_cbLighting };
	m_device.context()->PSSetConstantBuffers(0, 2, psb);
}

void ButterflyDemo::CreateRenderStates()
//Setup render states used in various stages of the scene rendering
{
	DepthStencilDescription dssDesc;

	// Setup depth stancil state for writing
	m_dssWrite = m_device.CreateDepthStencilState(dssDesc.StencilWriteDescription());

	// Setup depth stencil state for testing
	m_dssTest = m_device.CreateDepthStencilState(dssDesc.StencilTestDescription());

	// Setup rasterizer state with ccw front faces
	m_rsCCW = m_device.CreateRasterizerState(RasterizerDescription(true));

	BlendDescription bsDesc;

	// Setup alpha blending state
	m_bsAlpha = m_device.CreateBlendState(bsDesc.AlphaBlendDescription());

	// Setup additive blending state
	m_bsAdd = m_device.CreateBlendState(bsDesc.AdditiveBlendDescription());
}

void ButterflyDemo::CreateDodecahadronMtx()
//Compute dodecahedronMtx and mirrorMtx
{
	XMMATRIX mtx;

	XMStoreFloat4x4(&m_dodecahedronMtx[0],
		XMMatrixTranslation(0.0f, 0.0f, DODECAHEDRON_H / 2));

	mtx = XMLoadFloat4x4(&m_dodecahedronMtx[0]);
	XMStoreFloat4x4(&m_dodecahedronMtx[1], mtx *
		XMMatrixRotationZ(XM_PI) *
		XMMatrixRotationY(DODECAHEDRON_A - XM_PI));

	mtx = XMLoadFloat4x4(&m_dodecahedronMtx[1]);
	for (size_t i = 1; i <= 5; i++)
	{
		XMStoreFloat4x4(&m_dodecahedronMtx[i + 1], mtx *
			XMMatrixRotationZ(i * XM_2PI / 5.0f));
	}

	for (size_t i = 0; i < 6; i++)
	{
		mtx = XMLoadFloat4x4(&m_dodecahedronMtx[i]);
		XMStoreFloat4x4(&m_dodecahedronMtx[i + 6], mtx *
			XMMatrixRotationY(XM_PI));
	}

	for (size_t i = 0; i < 12; i++)
	{
		mtx = XMLoadFloat4x4(&m_dodecahedronMtx[i]);
		XMStoreFloat4x4(&m_dodecahedronMtx[i], mtx *
			XMMatrixScaling(2.0f, 2.0f, 2.0f));
	}

	XMMATRIX m_scale = XMMatrixScaling(1.0f, 1.0f, -1.0f);
	for (size_t i = 0; i < 12; i++)
	{
		XMMATRIX m = XMLoadFloat4x4(&m_dodecahedronMtx[i]);
		XMMATRIX m_inverse = XMMatrixInverse(nullptr, m);
		XMStoreFloat4x4(&m_mirrorMtx[i], m_inverse * m_scale * m);
	}
}

XMFLOAT3 ButterflyDemo::MoebiusStripPos(float t, float s)
{
	return XMFLOAT3(
		XMScalarCos(t) * (MOEBIUS_R + MOEBIUS_W * s * XMScalarCos(0.5f * t)),
		XMScalarSin(t) * (MOEBIUS_R + MOEBIUS_W * s * XMScalarCos(0.5f * t)),
		MOEBIUS_W * s * XMScalarSin(0.5f * t)
	);
}

XMVECTOR ButterflyDemo::MoebiusStripDs(float t, float s)
{
	return XMVector3Normalize(XMVectorSet(
		XMScalarCos(0.5f * t)*XMScalarCos(t),
		XMScalarCos(0.5f * t)*XMScalarSin(t),
		XMScalarSin(0.5f * t),
		0
	));
}

XMVECTOR ButterflyDemo::MoebiusStripDt(float t, float s)
{
	return XMVector3Normalize(XMVectorSet(-MOEBIUS_R * XMScalarSin(t) -
		0.5f * s * MOEBIUS_W * XMScalarSin(0.5f) * XMScalarCos(t) -
		MOEBIUS_W * s * XMScalarCos(0.5 * t) * XMScalarSin(t),

		MOEBIUS_R * XMScalarCos(t) -
		0.5f * s * MOEBIUS_W * XMScalarSin(0.5f) * XMScalarSin(t) +
		MOEBIUS_W * s * XMScalarCos(0.5 * t) * XMScalarCos(t),

		0.5f * s * MOEBIUS_W * XMScalarCos(t),

		0
	));
}

void ButterflyDemo::CreateMoebuisStrip()
{
	vector<VertexPositionNormal> vertices = vector<VertexPositionNormal>();
	vector<unsigned short> indices = vector<unsigned short>();

	VertexPositionNormal vpn;
	for (size_t i = 0; i < MOEBIUS_N; i++)
	{
		float angle = i * 4 * XM_PI / MOEBIUS_N;

		vpn.position = MoebiusStripPos(angle, -1);
		XMStoreFloat3(&vpn.normal,
			XMVector3Cross(MoebiusStripDt(angle, -1), MoebiusStripDs(angle, -1)));
		vertices.push_back(vpn);

		vpn.position = MoebiusStripPos(angle, 0);
		XMStoreFloat3(&vpn.normal,
			XMVector3Cross(MoebiusStripDt(angle, 0), MoebiusStripDs(angle, 0)));
		vertices.push_back(vpn);

		vpn.position = MoebiusStripPos(angle, 1);
		XMStoreFloat3(&vpn.normal,
			XMVector3Cross(MoebiusStripDt(angle, 1), MoebiusStripDs(angle, 1)));
		vertices.push_back(vpn);
	}

	int MOEBIUS_3N = MOEBIUS_N * 3;
	for (size_t i = 0; i < MOEBIUS_3N; i += 3)
	{
		indices.push_back(i);
		indices.push_back((i + 4) % MOEBIUS_3N);
		indices.push_back(i + 1);

		indices.push_back(i);
		indices.push_back((i + 5) % MOEBIUS_3N);
		indices.push_back(i + 2);

		indices.push_back(i);
		indices.push_back((i + 3) % MOEBIUS_3N);
		indices.push_back((i + 4) % MOEBIUS_3N);

		indices.push_back(i + 1);
		indices.push_back((i + 4) % MOEBIUS_3N);
		indices.push_back((i + 5) % MOEBIUS_3N);
	}

	m_moebius = m_device.CreateMesh(indices, vertices);
}
#pragma endregion

#pragma region Per-Frame Update
void ButterflyDemo::Update(const Clock& c)
{
	double dt = c.getFrameTime();
	if (HandleCameraInput(dt))
	{
		XMFLOAT4X4 cameraMtx;
		XMStoreFloat4x4(&cameraMtx, m_camera.getViewMatrix());
		UpdateCameraCB(cameraMtx);
	}
	UpdateButterfly(static_cast<float>(dt));
}

void ButterflyDemo::UpdateCameraCB(DirectX::XMFLOAT4X4 cameraMtx)
{
	XMMATRIX mtx = XMLoadFloat4x4(&cameraMtx);
	XMVECTOR det;
	auto invvmtx = XMMatrixInverse(&det, mtx);
	XMFLOAT4X4 view[2] = { cameraMtx };
	XMStoreFloat4x4(view + 1, invvmtx);
	m_cbView.Update(m_device.context(), view);
}

void ButterflyDemo::UpdateButterfly(float dtime)
{
	static float lap = 0.0f;
	lap += dtime;
	while (lap > LAP_TIME)
		lap -= LAP_TIME;
	//Value of the Moebius strip t parameter
	float t = 2 * lap / LAP_TIME;
	//Angle between wing current and vertical position
	float a = t * WING_MAX_A;
	t *= XM_2PI;
	if (a > WING_MAX_A)
		a = 2 * WING_MAX_A - a;
	//Write the rest of code here

	XMVECTOR pt = MoebiusStripDt(t, 0);
	XMVECTOR ps = MoebiusStripDs(t, 0);
	XMVECTOR pt_cross_ps = XMVector3Cross(ps, pt);

	XMFLOAT3 pos = MoebiusStripPos(t, 0);
	XMFLOAT4 pos4(pos.x, pos.y, pos.z, 1.0f);
	XMVECTOR p = XMLoadFloat4(&pos4);
	XMMATRIX moebiusMtx;
	moebiusMtx.r[0] = pt;
	moebiusMtx.r[1] = pt_cross_ps;
	moebiusMtx.r[2] = ps;
	moebiusMtx.r[3] = p;

	XMMATRIX common_model_mtx = XMMatrixTranslation(0.0f, 1.0f, 0.0f) *
		XMMatrixScaling(WING_W, WING_H, 1);

	XMStoreFloat4x4(&m_wingMtx[1], common_model_mtx * XMMatrixRotationX(-a) *
		moebiusMtx);
	XMStoreFloat4x4(&m_wingMtx[0], common_model_mtx * XMMatrixRotationX(a) *
		moebiusMtx);
}
#pragma endregion

#pragma region Frame Rendering Setup
void ButterflyDemo::SetShaders()
{
	m_device.context()->VSSetShader(m_vs.get(), 0, 0);
	m_device.context()->PSSetShader(m_ps.get(), 0, 0);
	m_device.context()->IASetInputLayout(m_il.get());
	m_device.context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void ButterflyDemo::SetBilboardShaders()
{
	m_device.context()->VSSetShader(m_vsBilboard.get(), 0, 0);
	m_device.context()->PSSetShader(m_psBilboard.get(), 0, 0);
	m_device.context()->IASetInputLayout(m_ilBilboard.get());
	m_device.context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void ButterflyDemo::Set1Light()
{
	Lighting l{
		/*.ambientColor = */ XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		/*.surface = */ XMFLOAT4(0.2f, 0.8f, 0.8f, 200.0f),
		/*.lights =*/ {
			{ /*.position =*/ m_camera.getCameraPosition(), /*.color =*/ WHITE_COLOR }
			//other 2 lights set to 0
		}
	};
	m_cbLighting.Update(m_device.context(), l);
}

void ButterflyDemo::Set3Lights()
{
	Lighting l{
		/*.ambientColor = */ XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		/*.surface = */ XMFLOAT4(0.2f, 0.8f, 0.8f, 200.0f),
		/*.lights =*/{
			{ /*.position =*/ m_camera.getCameraPosition(), /*.color =*/ WHITE_COLOR },
			{ /*.position =*/ GREEN_LIGHT_POS, /*.color =*/ GREEN_COLOR },
			{ /*.position =*/ BLUE_LIGHT_POS, /*.color =*/ BLUE_COLOR }
		}
	};

	m_cbLighting.Update(m_device.context(), l);
}
#pragma endregion

#pragma region Drawing
void ButterflyDemo::DrawBox()
{
	XMFLOAT4X4 worldMtx;
	XMStoreFloat4x4(&worldMtx, XMMatrixIdentity());
	m_cbWorld.Update(m_device.context(), worldMtx);
	m_box.Render(m_device.context());
}

void ButterflyDemo::DrawDodecahedron(bool colors)
//Draw dodecahedron. If color is true, use render faces with coresponding colors. Otherwise render using white color
{
	XMFLOAT4 white_color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	for (size_t i = 0; i < 12; i++)
	{
		m_cbSurfaceColor.Update(m_device.context(), colors ? COLORS[i] : white_color);
		m_cbWorld.Update(m_device.context(), m_dodecahedronMtx[i]);
		m_pentagon.Render(m_device.context());
	}
}

void ButterflyDemo::DrawMoebiusStrip()
{
	XMFLOAT4X4 worldMtx;
	XMStoreFloat4x4(&worldMtx, XMMatrixIdentity());
	m_cbWorld.Update(m_device.context(), worldMtx);
	m_moebius.Render(m_device.context());
}

void ButterflyDemo::DrawButterfly()
{
	for (size_t i = 0; i < 2; i++)
	{
		m_cbWorld.Update(m_device.context(), m_wingMtx[i]);
		m_wing.Render(m_device.context());
	}
}

void ButterflyDemo::DrawBilboards()
//Setup bilboards rendering and draw them
{
	// Setup shaders and blend state
	SetBilboardShaders();
	m_device.context()->OMSetBlendState(m_bsAdd.get(), nullptr, BS_MASK);

	// Draw both bilboards with appropriate colors and transformations
	XMFLOAT4X4 bilboardMtx;

	m_cbSurfaceColor.Update(m_device.context(), GREEN_COLOR);
	XMStoreFloat4x4(&bilboardMtx,
		XMMatrixScaling(BILBOARD_SCALE, BILBOARD_SCALE, BILBOARD_SCALE) *
		XMMatrixTranslation(GREEN_LIGHT_POS.x, GREEN_LIGHT_POS.y, GREEN_LIGHT_POS.z));
	m_cbWorld.Update(m_device.context(), bilboardMtx);
	m_bilboard.Render(m_device.context());

	m_cbSurfaceColor.Update(m_device.context(), BLUE_COLOR);
	XMStoreFloat4x4(&bilboardMtx,
		XMMatrixScaling(BILBOARD_SCALE, BILBOARD_SCALE, BILBOARD_SCALE) *
		XMMatrixTranslation(BLUE_LIGHT_POS.x, BLUE_LIGHT_POS.y, BLUE_LIGHT_POS.z));
	m_cbWorld.Update(m_device.context(), bilboardMtx);
	m_bilboard.Render(m_device.context());

	// Restore rendering state to it's original values
	m_cbSurfaceColor.Update(m_device.context(), WHITE_COLOR);
	m_device.context()->OMSetBlendState(nullptr, nullptr, BS_MASK);
	SetShaders();
}

void ButterflyDemo::DrawMirroredWorld(unsigned int i)
//Draw the mirrored scene reflected in the i-th dodecahedron face
{
	// Setup render state for writing to the stencil buffer
	m_device.context()->OMSetDepthStencilState(m_dssWrite.get(), i + 1);

	// Draw the i-th face
	m_cbWorld.Update(m_device.context(), m_dodecahedronMtx[i]);
	m_pentagon.Render(m_device.context());

	// Setup depth stencil state for rendering mirrored world
	m_device.context()->OMSetDepthStencilState(m_dssTest.get(), i + 1);

	// Setup rasterizer state and view matrix for rendering the mirrored world
	m_device.context()->RSSetState(m_rsCCW.get());

	// Draw objects of the mirrored scene - dodecahedron should be drawn with only one light
	// and no colors and without blending
	XMMATRIX m_view = m_camera.getViewMatrix();
	XMFLOAT4X4 new_view;
	XMStoreFloat4x4(&new_view, XMLoadFloat4x4(&m_mirrorMtx[i]) * m_view);
	UpdateCameraCB(new_view);
	Set3Lights();
	DrawMoebiusStrip();
	DrawButterfly();
	Set1Light();
	DrawDodecahedron(false);

	// Restore rasterizer state to it's original value
	m_device.context()->RSSetState(nullptr);

	// Draw mirrored bilboards - they need to be drawn after restoring rasterizer state, 
	// but with mirrored view matrix
	DrawBilboards();

	// Restore view matrix to its original value
	XMFLOAT4X4 old_view;
	XMStoreFloat4x4(&old_view, m_view);
	UpdateCameraCB(old_view);

	// Restore depth stencil state to it's original value
	m_device.context()->OMSetDepthStencilState(nullptr, 0);
}

void ButterflyDemo::Render()
{
	Gk2ExampleBase::Render();

	//render mirrored worlds
	for (int i = 0; i < 12; ++i)
		DrawMirroredWorld(i);

	//render dodecahedron with one light and alpha blending
	m_device.context()->OMSetBlendState(m_bsAlpha.get(), nullptr, BS_MASK);
	Set1Light();
	DrawDodecahedron(true);
	m_device.context()->OMSetBlendState(nullptr, nullptr, BS_MASK);

	//render the rest of the scene with all lights
	Set3Lights();
	m_cbSurfaceColor.Update(m_device.context(), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
	//DrawBox();
	DrawMoebiusStrip();
	DrawButterfly();
	DrawBilboards();
}
#pragma endregion