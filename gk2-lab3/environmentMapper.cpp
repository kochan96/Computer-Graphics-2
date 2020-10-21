#include "environmentMapper.h"
#include "dxstructures.h"
#include "dxDevice.h"

using namespace mini;
using namespace gk2;
using namespace utils;
using namespace DirectX;
using namespace std;

const int EnvironmentMapper::TEXTURE_SIZE = 256;

EnvironmentMapper::EnvironmentMapper(const DxDevice& device,
	dx_ptr<ID3D11VertexShader>&& vs, dx_ptr<ID3D11PixelShader>&& ps,
	const ConstantBuffer<XMFLOAT4X4>& cbWorld,
	const ConstantBuffer<XMFLOAT4X4, 2>& cbView,
	const ConstantBuffer<XMFLOAT4X4>& cbProj,
	const ConstantBuffer<XMFLOAT4>& cbSurfaceColor,
	const dx_ptr<ID3D11SamplerState>& sampler,
	float nearPlane, float farPlane,
	XMFLOAT3 position)
	: BasicEffect(move(vs), move(ps)),
	VSConstantBuffers{ cbWorld, cbView, cbProj },
	PSConstantBuffers{ cbSurfaceColor },
	PSSamplers{ sampler }, PSShaderResources(),
	m_nearPlane(nearPlane), m_farPlane(farPlane),
	m_position(position.x, position.y, position.z, 1.0f),
	m_face(static_cast<D3D11_TEXTURECUBE_FACE>(-1))
{
	Texture2DDescription texDesc;
	// Setup texture width, height, mip levels and bind flags
	texDesc.Width = TEXTURE_SIZE;
	texDesc.Height = TEXTURE_SIZE;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
	texDesc.MipLevels = 1;

	m_faceTexture = device.CreateTexture(texDesc);
	m_renderTarget = device.CreateRenderTargetView(m_faceTexture);

	SIZE s;
	s.cx = s.cy = TEXTURE_SIZE;
	m_depthBuffer = device.CreateDepthStencilView(s);

	// Create description for empty texture used as environment cube map, setup 
	// texture's width, height, mipLevels, bindflags, array size and miscFlags
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
	texDesc.ArraySize = 6;
	m_envTexture = device.CreateTexture(texDesc);

	// Create description of shader resource view for cube map
	ShaderResourceViewDescription srvDesc;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MipLevels = 1;
	srvDesc.TextureCube.MostDetailedMip = 0;

	auto envResourceView = device.CreateShaderResourceView(m_envTexture, srvDesc);
	SetPSShaderResource(0, envResourceView);
}

void EnvironmentMapper::Begin(const dx_ptr<ID3D11DeviceContext>& context) const
{
	BasicEffect::Begin(context);
	VSConstantBuffers::Begin(context);
	PSConstantBuffers::Begin(context);
	PSSamplers::Begin(context);
	PSShaderResources::Begin(context);
}

void EnvironmentMapper::BeginFace(const dx_ptr<ID3D11DeviceContext>& context,
	ConstantBuffer<DirectX::XMFLOAT4X4, 2>& cbView,
	ConstantBuffer<DirectX::XMFLOAT4X4>& cbProj, D3D11_TEXTURECUBE_FACE face)
{
	m_face = face;

	// Setup view matrix
	XMFLOAT3 eyePosFloat = XMFLOAT3(m_position.x, m_position.y, m_position.z);
	XMFLOAT3 eyeDirFloat;
	XMFLOAT3 upDirFloat;
	switch (face)
	{
	case 0:
		eyeDirFloat = XMFLOAT3(1.0f, 0.0f, 0.0f);
		upDirFloat = XMFLOAT3(0.0f, 1.0f, 0.0f);
		break;
	case 1:
		eyeDirFloat = XMFLOAT3(-1.0f, 0.0f, 0.0f);
		upDirFloat = XMFLOAT3(0.0f, 1.0f, 0.0f);
		break;
	case 2:
		eyeDirFloat = XMFLOAT3(0.0f, 1.0f, 0.0f);
		upDirFloat = XMFLOAT3(0.0f, 0.0f, -1.0f);
		break;
	case 3:
		eyeDirFloat = XMFLOAT3(0.0f, -1.0f, 0.0f);
		upDirFloat = XMFLOAT3(0.0f, 0.0f, 1.0f);
		break;
	case 4:
		eyeDirFloat = XMFLOAT3(0.0f, 0.0f, 1.0f);
		upDirFloat = XMFLOAT3(0.0f, 1.0f, 0.0f);
		break;
	case 5:
		eyeDirFloat = XMFLOAT3(0.0f, 0.0f, -1.0f);
		upDirFloat = XMFLOAT3(0.0f, 1.0f, 0.0f);
		break;
	default:
		break;
	}
	XMVECTOR eyePos = XMLoadFloat3(&eyePosFloat);
	XMVECTOR eyeDir = XMLoadFloat3(&eyeDirFloat);
	XMVECTOR upDir = XMLoadFloat3(&upDirFloat);
	XMMATRIX viewMtx = XMMatrixLookToLH(eyePos, eyeDir, upDir);

	XMFLOAT4X4 view[2];
	XMVECTOR det;
	XMStoreFloat4x4(view, viewMtx);
	XMStoreFloat4x4(view + 1, XMMatrixInverse(&det, viewMtx));
	cbView.Update(context, view);
	XMFLOAT4X4 proj;
	XMMATRIX projMtx = XMMatrixPerspectiveFovLH(XM_PIDIV2, 1.0f, m_nearPlane, m_farPlane);
	XMStoreFloat4x4(&proj, projMtx);

	cbProj.Update(context, proj);

	D3D11_VIEWPORT viewport;

	// Setup viewport
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = TEXTURE_SIZE;
	viewport.Height = TEXTURE_SIZE;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	context->RSSetViewports(1, &viewport);
	ID3D11RenderTargetView* targets[1] = { m_renderTarget.get() };
	context->OMSetRenderTargets(1, targets, m_depthBuffer.get());
	float clearColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	context->ClearRenderTargetView(m_renderTarget.get(), clearColor);
	context->ClearDepthStencilView(m_depthBuffer.get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void EnvironmentMapper::EndFace(const dx_ptr<ID3D11DeviceContext>& context)
{
	if (m_face < 0 || m_face > 5)
		return;

	// Copy face to environment cube map
	context->CopySubresourceRegion(m_envTexture.get(), m_face,
		0, 0, 0, m_faceTexture.get(), 0, nullptr);

	m_face = static_cast<D3D11_TEXTURECUBE_FACE>(-1);
}
