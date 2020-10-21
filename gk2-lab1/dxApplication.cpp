#include "dxApplication.h"
#include <iostream>

using namespace mini;
using namespace std;

DxApplication::DxApplication(HINSTANCE hInstance)
	: WindowApplication(hInstance), m_device(m_window)
{
	GetCursorPos(&curr_mouse_pos);

	ID3D11Texture2D *temp = nullptr;
	m_device.swapChain()->GetBuffer(
		0,
		__uuidof(ID3D11Texture2D),
		reinterpret_cast<void**>(&temp));
	const dx_ptr<ID3D11Texture2D> backTexture{ temp };
	m_backBuffer = m_device.CreateRenderTargetView(backTexture);

	SIZE wndSize = m_window.getClientSize();
	m_depthBuffer = m_device.CreateDepthStencilView(wndSize);
	auto backBuffer = m_backBuffer.get();
	m_device.context()->OMSetRenderTargets(1,
		&backBuffer, m_depthBuffer.get());
	Viewport viewport{ wndSize };
	m_device.context()->RSSetViewports(1, &viewport);

	const auto vsBytes = DxDevice::LoadByteCode(L"vs.cso");
	const auto psBytes = DxDevice::LoadByteCode(L"ps.cso");
	m_vertexShader = m_device.CreateVertexShader(vsBytes);
	m_pixelShader = m_device.CreatePixelShader(psBytes);

	const auto vertices = CreateCubeVertices();
	m_vertexBuffer = m_device.CreateVertexBuffer(vertices);
	const auto indices = CreateCubeIndices();
	m_indexBuffer = m_device.CreateIndexBuffer(indices);
	vector<D3D11_INPUT_ELEMENT_DESC> elements
	{
		{
			"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
			D3D11_INPUT_PER_VERTEX_DATA, 0
		},
		{
			"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
			offsetof(VertexPositionColor, color),
			D3D11_INPUT_PER_VERTEX_DATA, 0
		}
	};
	m_layout = m_device.CreateInputLayout(elements, vsBytes);

	XMStoreFloat4x4(&m_modelMtx0, XMMatrixIdentity());
	XMStoreFloat4x4(&m_modelMtx1, XMMatrixIdentity());
	RecalculateViewMtx();
	XMStoreFloat4x4(&m_projMtx, XMMatrixPerspectiveFovLH(
		XMConvertToRadians(45),
		static_cast<float>(wndSize.cx) / wndSize.cy,
		0.1f, 100.0f));
	m_cbMVP = m_device.CreateConstantBuffer<XMFLOAT4X4>();

	QueryPerformanceFrequency(&li);
	PCFreq = double(li.QuadPart) * XM_PIDIV4;

	QueryPerformanceCounter(&li);
	counterStart = li.QuadPart;
}

std::vector<XMFLOAT2> DxApplication::CreateTriangleVertices()
{
	auto v = std::vector<XMFLOAT2>();
	v.push_back(XMFLOAT2(-0.5, -0.5));
	v.push_back(XMFLOAT2(-0.5, 0.5));
	v.push_back(XMFLOAT2(0.5, -0.5));

	return v;
}

int DxApplication::MainLoop()
{
	MSG msg{};
	do {
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			Render();
			m_device.swapChain()->Present(0, 0);
		}

	} while (msg.message != WM_QUIT);

	return msg.wParam;
}

std::vector<VertexPositionColor> DxApplication::CreateCubeVertices()
{
	return
	{
		//Front Face
		{ { -0.5f, -0.5f, -0.5f },{ 1.0f, 0.0f, 0.0f } },
		{ { +0.5f, -0.5f, -0.5f },{ 1.0f, 0.0f, 0.0f } },
		{ { +0.5f, +0.5f, -0.5f },{ 1.0f, 0.0f, 0.0f } },
		{ { -0.5f, +0.5f, -0.5f },{ 1.0f, 0.0f, 0.0f } },

		//Back Face
		{ { +0.5f, -0.5f, +0.5f },{ 0.0f,  1.0f, 0.0f } },
		{ { -0.5f, -0.5f, +0.5f },{ 0.0f,  1.0f, 0.0f } },
		{ { -0.5f, +0.5f, +0.5f },{ 0.0f,  1.0f, 0.0f } },
		{ { +0.5f, +0.5f, +0.5f },{ 0.0f,  1.0f, 0.0f } },

		//Right Face
		{ { +0.5f, -0.5f, -0.5f },{ 0.0f, 0.0f, 1.0f } },
		{ { +0.5f, -0.5f, +0.5f },{ 0.0f, 0.0f, 1.0f } },
		{ { +0.5f, +0.5f, +0.5f },{ 0.0f, 0.0f, 1.0f } },
		{ { +0.5f, +0.5f, -0.5f },{ 0.0f, 0.0f, 1.0f } },

		//Left Face
		{ { -0.5f, -0.5f, +0.5f },{ 1.0f, 1.0f, 0.0f } },
		{ { -0.5f, -0.5f, -0.5f },{ 1.0f, 1.0f, 0.0f } },
		{ { -0.5f, +0.5f, -0.5f },{ 1.0f, 1.0f, 0.0f } },
		{ { -0.5f, +0.5f, +0.5f },{ 1.0f, 1.0f, 0.0f } },

		//Top Face
		{ { -0.5f, +0.5f, -0.5f },{ 1.0f, 0.0f, 1.0f } },
		{ { +0.5f, +0.5f, -0.5f },{ 1.0f, 0.0f, 1.0f } },
		{ { +0.5f, +0.5f, +0.5f },{ 1.0f, 0.0f, 1.0f } },
		{ { -0.5f, +0.5f, +0.5f },{ 1.0f, 0.0f, 1.0f } },

		//Bottom Face
		{ { -0.5f, -0.5f, +0.5f },{ 0.0f, 1.0f, 1.0f } },
		{ { +0.5f, -0.5f, +0.5f },{ 0.0f, 1.0f, 1.0f } },
		{ { +0.5f, -0.5f, -0.5f },{ 0.0f, 1.0f, 1.0f } },
		{ { -0.5f, -0.5f, -0.5f },{ 0.0f, 1.0f, 1.0f } },
	};
}

std::vector<unsigned short> DxApplication::CreateCubeIndices()
{
	return {
		0,3,2,    0,2,1,
		4,7,6,    4,6,5,
		8,11,10,  8,10,9,
		12,15,14, 12,14,13,
		16,19,18, 16,18,17,
		20,23,22, 20,22,21
	};
}

void DxApplication::Render()
{
	const float clearColor[] = { 0.5f, 0.5f, 1.0f, 1.0f };
	m_device.context()->ClearRenderTargetView(m_backBuffer.get(), clearColor);

	m_device.context()->ClearDepthStencilView(m_depthBuffer.get(),
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	m_device.context()->VSSetShader(m_vertexShader.get(), nullptr, 0);
	m_device.context()->PSSetShader(m_pixelShader.get(), nullptr, 0);
	m_device.context()->IASetInputLayout(m_layout.get());
	m_device.context()->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ID3D11Buffer* cbs[] = { m_cbMVP.get() };
	m_device.context()->VSSetConstantBuffers(0, 1, cbs);

	ID3D11Buffer* vbs[] = { m_vertexBuffer.get() };
	UINT strides[] = { sizeof(VertexPositionColor) };
	UINT offsets[] = { 0 };
	m_device.context()->IASetVertexBuffers(
		0, 1, vbs, strides, offsets);
	m_device.context()->IASetIndexBuffer(m_indexBuffer.get(),
		DXGI_FORMAT_R16_UINT, 0);

	Draw0();
	Draw1();
}

void DxApplication::Draw0()
{
	QueryPerformanceCounter(&li);
	angle += double(li.QuadPart - counterStart) / PCFreq;
	counterStart = li.QuadPart;

	DirectX::XMFLOAT4X4 new_m_modelMtx;
	XMStoreFloat4x4(&new_m_modelMtx, XMLoadFloat4x4(&m_modelMtx0) *
		XMMatrixRotationY(angle));
	D3D11_MAPPED_SUBRESOURCE res;
	m_device.context()->Map(m_cbMVP.get(), 0, D3D11_MAP_WRITE_DISCARD, 0,
		&res);
	XMMATRIX mvp = XMLoadFloat4x4(&new_m_modelMtx) * XMLoadFloat4x4(&m_viewMtx) *
		XMLoadFloat4x4(&m_projMtx);
	memcpy(res.pData, &mvp, sizeof(XMMATRIX));
	m_device.context()->Unmap(m_cbMVP.get(), 0);

	m_device.context()->DrawIndexed(36, 0, 0);
}

void DxApplication::Draw1()
{
	DirectX::XMFLOAT4X4 new_m_modelMtx;

	XMStoreFloat4x4(&new_m_modelMtx, XMLoadFloat4x4(&m_modelMtx1) *
		XMMatrixTranslation(-5.0f, 0.0f, 0.0f));

	D3D11_MAPPED_SUBRESOURCE res;
	m_device.context()->Map(m_cbMVP.get(), 0, D3D11_MAP_WRITE_DISCARD, 0,
		&res);
	XMMATRIX mvp = XMLoadFloat4x4(&new_m_modelMtx) * XMLoadFloat4x4(&m_viewMtx) *
		XMLoadFloat4x4(&m_projMtx);
	memcpy(res.pData, &mvp, sizeof(XMMATRIX));
	m_device.context()->Unmap(m_cbMVP.get(), 0);

	m_device.context()->DrawIndexed(36, 0, 0);
}

void DxApplication::RecalculateViewMtx()
{
	XMStoreFloat4x4(&m_viewMtx, XMMatrixTranslation(0.0f, 0.0f, camera_distance)
		* XMMatrixRotationY(XMConvertToRadians(camera_angle)));
}

bool DxApplication::ProcessMessage(WindowMessage & msg)
{
	if (msg.message == WM_LBUTTONDOWN)
		left_mouse_down = true;

	if (msg.message == WM_RBUTTONDOWN)
		right_mouse_down = true;

	if (msg.message == WM_LBUTTONUP)
		left_mouse_down = false;

	if (msg.message == WM_RBUTTONUP)
		right_mouse_down = false;

	if (msg.message == WM_MOUSEMOVE)
	{
		POINT cursorPos;
		GetCursorPos(&cursorPos);

		if (left_mouse_down)
		{
			camera_angle -= 0.5f * (cursorPos.x - curr_mouse_pos.x);
			if (camera_angle < -90.0f) camera_angle = -90.0f;
			else if (camera_angle > 90.0f) camera_angle = 90.0f;
		}
		if (right_mouse_down)
		{
			camera_distance += 0.1f * (cursorPos.y - curr_mouse_pos.y);
			if (camera_distance < 0.0f) camera_distance = 0.0f;
			else if (camera_distance > 10.0f) camera_distance = 10.0f;
		}
		curr_mouse_pos = cursorPos;
		RecalculateViewMtx();
	}

	return false;
}
