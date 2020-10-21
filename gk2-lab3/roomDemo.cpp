#include "roomDemo.h"
#include <array>
#include "meshLoader.h"
#include "textureGenerator.h"

using namespace mini;
using namespace utils;
using namespace gk2;
using namespace DirectX;
using namespace std;

const XMFLOAT4 RoomDemo::TABLE_POS{ 0.5f, -0.96f, 0.5f, 1.0f };
const XMFLOAT4 RoomDemo::LIGHT_POS[2] = { {1.0f, 1.0f, 1.0f, 1.0f}, {-1.0f, -1.0f, -1.0f, 1.0f} };

RoomDemo::RoomDemo(HINSTANCE appInstance)
	: Gk2ExampleBase(appInstance, 1280, 720, L"Pokój"),
	//Constant Buffers
	m_cbWorldMtx(m_device.CreateConstantBuffer<XMFLOAT4X4>()),
	m_cbProjMtx(m_device.CreateConstantBuffer<XMFLOAT4X4>()), 
	m_cbTex1Mtx(m_device.CreateConstantBuffer<XMFLOAT4X4>()),
	m_cbTex2Mtx(m_device.CreateConstantBuffer<XMFLOAT4X4>()),
	m_cbViewMtx(m_device.CreateConstantBuffer<XMFLOAT4X4, 2>()),
	m_cbSurfaceColor(m_device.CreateConstantBuffer<XMFLOAT4>()),
	m_cbLightPos(m_device.CreateConstantBuffer<XMFLOAT4, 2>()),
	//Textures
	m_wallTexture(m_device.CreateShaderResourceView(L"resources/textures/brick_wall.jpg")),
	m_posterTexture(m_device.CreateShaderResourceView(L"resources/textures/lautrec_divan.jpg")),
	m_perlinTexture(m_device.CreateShaderResourceView(L"resources/textures/perlin.jpg"))
{
	//Projection matrix
	auto s = m_window.getClientSize();
	auto ar = static_cast<float>(s.cx) / s.cy;
	XMStoreFloat4x4(&m_projMtx, XMMatrixPerspectiveFovLH(XM_PIDIV4, ar, 0.01f, 100.0f));
	m_cbProjMtx.Update(m_device.context(), m_projMtx);
	UpdateCameraCB();

	//Sampler States
	SamplerDescription sd;
	sd.AddressU = sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	m_samplerWrap = m_device.CreateSamplerState(sd);
	sd.AddressU = sd.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	// sd.MipLODBias = 2.0f;
	m_samplerBorder = m_device.CreateSamplerState(sd);

	//Wood texture
	constexpr auto woodTexWidth = 64U;
	constexpr auto woodTexHeight = 64U;
	constexpr auto woodTexBpp = 4U;
	constexpr auto woodTexStride = woodTexWidth * woodTexBpp;
	constexpr auto woodTexSize = woodTexStride * woodTexHeight;

	auto texDesc = Texture2DDescription(woodTexWidth, woodTexHeight);
	texDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
	auto woodTex = m_device.CreateTexture(texDesc);
	m_woodTexture = m_device.CreateShaderResourceView(woodTex);
	array<BYTE, woodTexSize> data;
	auto d = data.data();
	TextureGenerator txGen(6, 0.35f);
	for (auto i = 0; i < woodTexHeight; ++i)
	{
		auto x = i / static_cast<float>(woodTexHeight);
		for (auto j = 0; j < woodTexWidth; ++j)
		{
			auto y = j / static_cast<float>(woodTexWidth);
			auto c = txGen.Wood(x, y);
			auto ic = static_cast<BYTE>(c * 239);
			*(d++) = ic;
			ic = static_cast<BYTE>(c * 200);
			*(d++) = ic;
			ic = static_cast<BYTE>(c * 139);
			*(d++) = ic;
			*(d++) = 255;
		}
	}
	m_device.context()->UpdateSubresource(woodTex.get(), 0, nullptr, data.data(), woodTexStride, woodTexSize);
	m_device.context()->GenerateMips(m_woodTexture.get());

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
	XMStoreFloat4x4(&m_chairMtx, XMMatrixRotationY(XM_PI + XM_PI / 9) *
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
	m_cbLightPos.Update(m_device.context(), LIGHT_POS);
	XMFLOAT4X4 tempMtx;
	XMStoreFloat4x4(&tempMtx, XMMatrixScaling(0.25f, 0.25f, 1.0f) * XMMatrixTranslation(0.5f, 0.5f, 0.0f));
	m_cbTex1Mtx.Update(m_device.context(), tempMtx);

	XMStoreFloat4x4(&tempMtx,
		XMMatrixTranslation(0.5f, 0.0f, 0.0f) *
		XMMatrixRotationZ(XM_PI / 18.0f) *
		XMMatrixScaling(1.0f, -0.75f, 1.0f) *
		XMMatrixTranslation(0.5f, 0.5f, 0.0f) *
		XMMatrixIdentity());
	m_cbTex2Mtx.Update(m_device.context(), tempMtx);

	//Render states
	RasterizerDescription rsDesc;
	rsDesc.CullMode = D3D11_CULL_FRONT;
	m_rsCullFront = m_device.CreateRasterizerState(rsDesc);

	m_bsAlpha = m_device.CreateBlendState(BlendDescription::AlphaBlendDescription());
	DepthStencilDescription dssDesc;
	dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	m_dssNoWrite = m_device.CreateDepthStencilState(dssDesc);

	auto vsCode = m_device.LoadByteCode(L"phongVS.cso");
	auto psCode = m_device.LoadByteCode(L"phongPS.cso");
	m_phongEffect = PhongEffect(m_device.CreateVertexShader(vsCode),
		m_device.CreatePixelShader(psCode),
		m_cbWorldMtx, m_cbViewMtx, m_cbProjMtx, m_cbLightPos, m_cbSurfaceColor);
	m_inputlayout = m_device.CreateInputLayout(VertexPositionNormal::Layout, vsCode);

	vsCode = m_device.LoadByteCode(L"texturedVS.cso");
	psCode = m_device.LoadByteCode(L"texturedPS.cso");
	m_texturedEffect = TexturedEffect(m_device.CreateVertexShader(vsCode),
		m_device.CreatePixelShader(psCode),
		m_cbWorldMtx, m_cbViewMtx, m_cbProjMtx, m_cbTex1Mtx, m_samplerWrap, m_wallTexture);
	vsCode = m_device.LoadByteCode(L"colorTexVS.cso");
	psCode = m_device.LoadByteCode(L"colorTexPS.cso");
	m_coloredTextureEffect = ColoredTextureEffect(m_device.CreateVertexShader(vsCode),
		m_device.CreatePixelShader(psCode),
		m_cbWorldMtx, m_cbViewMtx, m_cbProjMtx, m_cbTex1Mtx, m_cbSurfaceColor, m_samplerWrap, m_perlinTexture);
	vsCode = m_device.LoadByteCode(L"multiTexVS.cso");
	psCode = m_device.LoadByteCode(L"multiTexPS.cso");
	m_multiTexturedEffect = MultiTexturedEffect(m_device.CreateVertexShader(vsCode),
		m_device.CreatePixelShader(psCode),
		m_cbWorldMtx, m_cbViewMtx, m_cbProjMtx, m_cbTex1Mtx, m_cbTex2Mtx,
		m_samplerBorder, m_wallTexture, m_posterTexture);

	vsCode = m_device.LoadByteCode(L"envMapperVS.cso");
	psCode = m_device.LoadByteCode(L"envMapperPS.cso");
	m_envMapper = EnvironmentMapper(m_device, m_device.CreateVertexShader(vsCode), m_device.CreatePixelShader(psCode),
		m_cbWorldMtx, m_cbViewMtx, m_cbProjMtx, m_cbSurfaceColor, m_samplerWrap, 0.4f, 8.0f, XMFLOAT3(-1.3f, -0.74f, -0.6f));
	m_particles = ParticleSystem(m_device, m_cbViewMtx, m_cbProjMtx, m_samplerWrap, XMFLOAT3(-1.3f, -0.6f, -0.14f));

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
	auto rot = XM_2PI * time / 20;
	auto lamp = XMMatrixTranslation(0.0f, -0.4f, 0.0f) * XMMatrixRotationX(swing) * XMMatrixRotationY(rot) *
		XMMatrixTranslation(0.0f, 2.0f, 0.0f);
	XMStoreFloat4x4(&m_lampMtx, lamp);
}

void RoomDemo::Update(const Clock& c)
{
	double dt = c.getFrameTime();
	HandleCameraInput(dt);
	UpdateLamp(static_cast<float>(dt));
	m_particles.Update(m_device.context(), static_cast<float>(dt), m_camera.getCameraPosition());
}

void RoomDemo::SetSurfaceColor(DirectX::XMFLOAT4 color)
{
	m_cbSurfaceColor.Update(m_device.context(), color);
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

void RoomDemo::DrawWalls()
{
	//draw floor
	XMFLOAT4X4 texMtx;
	XMStoreFloat4x4(&texMtx, XMMatrixScaling(0.25f, 4.0f, 1.0f) * XMMatrixTranslation(0.5f, 0.5f, 0.0f));
	m_cbTex1Mtx.Update(m_device.context(), texMtx);
	m_texturedEffect.SetTexture(m_woodTexture);
	m_texturedEffect.Begin(m_device.context());
	DrawMesh(m_wall, m_wallsMtx[4]);

	//draw floor
	SetSurfaceColor(XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f));
	XMStoreFloat4x4(&texMtx, XMMatrixScaling(0.25f, 0.25f, 0.25f) * XMMatrixTranslation(0.5f, 0.5f, 0.0f));
	m_cbTex1Mtx.Update(m_device.context(), texMtx);
	m_coloredTextureEffect.Begin(m_device.context());
	DrawMesh(m_wall, m_wallsMtx[5]);
	SetSurfaceColor(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

	//draw back wall
	m_multiTexturedEffect.Begin(m_device.context());
	DrawMesh(m_wall, m_wallsMtx[0]);

	//draw remainting walls
	m_texturedEffect.SetTexture(m_wallTexture);
	m_texturedEffect.Begin(m_device.context());
	for (auto i = 1; i < 4; ++i)
		DrawMesh(m_wall, m_wallsMtx[i]);
}

void RoomDemo::DrawTeapot()
{
	m_envMapper.Begin(m_device.context());

	// Comment the following line and uncomment the next to replace teapot with a sphere
	DrawMesh(m_teapot, m_teapotMtx);
	//DrawMesh(m_sphere, m_sphereMtx);

	SetSurfaceColor(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
}

void RoomDemo::DrawTableElement(Mesh& m, DirectX::XMFLOAT4X4 worldMtx)
{
	SetWorldMtx(worldMtx);
	m_device.context()->RSSetState(m_rsCullFront.get());
	m.Render(m_device.context());
	m_device.context()->RSSetState(nullptr);
	m.Render(m_device.context());
}

void RoomDemo::DrawTableLegs(XMVECTOR camVec)
{
	XMFLOAT4 v(1.0f, 0.0f, 0.0f, 0.0f);
	auto plane1 = XMLoadFloat4(&v);
	v = XMFLOAT4(0.0f, 0.0f, 1.0f, 0.0f);
	auto plane2 = XMLoadFloat4(&v);
	auto left = XMVector3Dot(camVec, plane1).m128_f32[0] > 0;
	auto back = XMVector3Dot(camVec, plane2).m128_f32[0] > 0;
	if (left)
	{
		if (back)
		{
			DrawTableElement(m_tableLeg, m_tableLegsMtx[2]);
			DrawTableElement(m_tableLeg, m_tableLegsMtx[3]);
			DrawTableElement(m_tableLeg, m_tableLegsMtx[1]);
			DrawTableElement(m_tableLeg, m_tableLegsMtx[0]);
		}
		else
		{
			DrawTableElement(m_tableLeg, m_tableLegsMtx[3]);
			DrawTableElement(m_tableLeg, m_tableLegsMtx[2]);
			DrawTableElement(m_tableLeg, m_tableLegsMtx[0]);
			DrawTableElement(m_tableLeg, m_tableLegsMtx[1]);
		}
	}
	else
	{

		if (back)
		{
			DrawTableElement(m_tableLeg, m_tableLegsMtx[1]);
			DrawTableElement(m_tableLeg, m_tableLegsMtx[0]);
			DrawTableElement(m_tableLeg, m_tableLegsMtx[2]);
			DrawTableElement(m_tableLeg, m_tableLegsMtx[3]);
		}
		else
		{
			DrawTableElement(m_tableLeg, m_tableLegsMtx[0]);
			DrawTableElement(m_tableLeg, m_tableLegsMtx[1]);
			DrawTableElement(m_tableLeg, m_tableLegsMtx[3]);
			DrawTableElement(m_tableLeg, m_tableLegsMtx[2]);
		}
	}
}

void RoomDemo::DrawTransparentObjects()
{
	m_device.context()->OMSetBlendState(m_bsAlpha.get(), nullptr, UINT_MAX);
	m_device.context()->OMSetDepthStencilState(m_dssNoWrite.get(), 0);
	SetSurfaceColor(XMFLOAT4(0.1f, 0.1f, 0.1f, 0.9f));
	auto v = m_camera.getCameraPosition();
	auto camVec = XMVector3Normalize(XMLoadFloat4(&v) - XMLoadFloat4(&TABLE_POS));
	v = XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f);
	auto plane = XMLoadFloat4(&v);
	if (XMVector3Dot(camVec, plane).m128_f32[0] > 0)
	{
		m_phongEffect.Begin(m_device.context());
		m_device.context()->RSSetState(m_rsCullFront.get());
		DrawMesh(m_tableSide, m_tableSideMtx);
		m_device.context()->RSSetState(nullptr);
		DrawTableLegs(camVec);
		DrawMesh(m_tableSide, m_tableSideMtx);
		DrawTableElement(m_tableTop, m_tableTopMtx);
		DrawParticles();
	}
	else
	{
		DrawParticles();
		m_phongEffect.Begin(m_device.context());
		DrawTableElement(m_tableTop, m_tableTopMtx);
		m_device.context()->RSSetState(m_rsCullFront.get());
		DrawMesh(m_tableSide, m_tableSideMtx);
		m_device.context()->RSSetState(nullptr);
		DrawTableLegs(camVec);
		DrawMesh(m_tableSide, m_tableSideMtx);
	}
	SetSurfaceColor(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
	m_device.context()->OMSetBlendState(nullptr, nullptr, UINT_MAX);
	m_device.context()->OMSetDepthStencilState(nullptr, 0);
}

void RoomDemo::DrawScene()
{
	DrawWalls();
	DrawTeapot();
	m_phongEffect.Begin(m_device.context());

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
	DrawTransparentObjects();
}

void RoomDemo::Render()
{
	Gk2ExampleBase::Render();

	// Render scene to each envionment cube map face
	for (size_t i = 0; i < 6; i++)
	{
		m_envMapper.BeginFace(m_device.context(), m_cbViewMtx, m_cbProjMtx,
			static_cast<D3D11_TEXTURECUBE_FACE>(i));
		//getDefaultRenderTarget().Begin(m_device.context());
		/*m_cbProjMtx.Update(m_device.context(), m_projMtx);
		UpdateCameraCB();*/
		DrawScene();
		m_envMapper.EndFace(m_device.context());
	}

	getDefaultRenderTarget().Begin(m_device.context());
	m_cbProjMtx.Update(m_device.context(), m_projMtx);
	UpdateCameraCB();

	DrawScene();
}