#include "lightAndShadowMap.h"
#include <dxDevice.h>

using namespace mini;
using namespace gk2;
using namespace utils;
using namespace DirectX;

const float LightAndShadowMap::LIGHT_FOV_ANGLE = DirectX::XM_PI / 3.0f;

LightAndShadowMap::LightAndShadowMap(const DxDevice& device, const ConstantBuffer<XMFLOAT4X4>& cbWorld,
	const ConstantBuffer<XMFLOAT4X4, 2> cbView, const ConstantBuffer<XMFLOAT4X4>& cbProj,
	const ConstantBuffer<XMFLOAT4> cbLightPos, const ConstantBuffer<XMFLOAT4> cbSurfaceColor)
	: StaticEffect(
		PhongEffect(device.CreateVertexShader(L"lightAndShadowVS.cso"),
			device.CreatePixelShader(L"lightAndShadowPS.cso"),
			cbWorld, cbView, cbProj, cbLightPos, cbSurfaceColor),
		PSShaderResources{}, PSSamplers{}), m_cbMapMtx(device.CreateConstantBuffer<XMFLOAT4X4>())
{
	SetPSConstantBuffer(2, m_cbMapMtx);
	auto lightMap = device.CreateShaderResourceView(L"resources/textures/light_cookie.png");
	SetPSShaderResource(0, lightMap);

	SamplerDescription sd;
	// TODO : 3.04 Create sampler with appropriate addressing (border) and filtering 
	// (bilinear) modes
	sd.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	sd.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	sd.BorderColor[0] = 0;
	sd.BorderColor[1] = 0;
	sd.BorderColor[2] = 0;
	sd.BorderColor[3] = 0;
	sd.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;

	SetPSSampler(0, device.CreateSamplerState(sd));

	// TODO : 3.09 Create shadow texture with appropriate width, height, format, 
	// mip levels and bind flags
	Texture2DDescription td;
	td.Width = TEXTURE_SIZE;
	td.Height = TEXTURE_SIZE;
	td.Format = DXGI_FORMAT_R32_TYPELESS;
	td.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	td.MipLevels = 1;

	dx_ptr<ID3D11Texture2D> shadowTexture = device.CreateTexture(td);
	
	// TODO : 3.10 Create depth-stencil view for the shadow texture with 
	// appropriate format
	DepthViewDescription dvd;
	dvd.Format = DXGI_FORMAT_D32_FLOAT;

	m_shadowDepthBuffer = device.CreateDepthStencilView(shadowTexture, dvd);

	// TODO : 3.11 Create shader resource view for the shadow texture with 
	// appropriate format, view dimensions, mip levels and most detailed mip level
	ShaderResourceViewDescription srvd;
	srvd.Format = DXGI_FORMAT_R32_FLOAT;
	srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvd.Texture2D.MipLevels = 1;
	srvd.Texture2D.MostDetailedMip = 0;

	dx_ptr<ID3D11ShaderResourceView> shadowMap = device.CreateShaderResourceView(shadowTexture, srvd);
	SetPSShaderResource(1, shadowMap);
}

XMFLOAT4 LightAndShadowMap::UpdateLightPosition(const dx_ptr<ID3D11DeviceContext>& context,
	XMMATRIX lightMtx)
{
	XMFLOAT4 lightPos{ 0.0f, -0.05f, 0.0f, 1.0f };
	XMFLOAT4 lightTarget{ 0.0f, -10.0f, 0.0f, 1.0f };
	XMFLOAT4 upDir{ 1.0f, 0.0f, 0.0f, 0.0f };

	// TODO : 3.01 Calculate view, inverted view and projection matrix and store them in
	// appropriate class fields
	XMMATRIX viewMtx = XMMatrixLookAtLH(
		XMVector3TransformCoord(XMLoadFloat4(&lightPos), lightMtx), 
		XMVector3TransformCoord(XMLoadFloat4(&lightTarget), lightMtx), 
		XMVector3TransformNormal(XMLoadFloat4(&upDir), lightMtx));
	XMStoreFloat4x4(&m_lightViewMtx[0], viewMtx);
	XMMATRIX inverseMtx = XMMatrixInverse(nullptr, viewMtx);
	XMStoreFloat4x4(&m_lightViewMtx[1], inverseMtx);
	XMMATRIX projMtx = XMMatrixPerspectiveFovLH(LIGHT_FOV_ANGLE, 1, LIGHT_NEAR, LIGHT_FAR);
	XMStoreFloat4x4(&m_lightProjMtx, projMtx);

	// TODO : 3.02 Calculate map transform matrix
	XMFLOAT4X4 texFloat4x4;
	XMMATRIX textMtx = viewMtx * projMtx * XMMatrixScaling(0.5f, -0.5f, 1.0f)
		* XMMatrixTranslation(0.5f, 0.5f, -0.00001f);

	// TODO : 3.17 Modify map transform to fix z-fighting

	XMStoreFloat4x4(&texFloat4x4, textMtx);

	m_cbMapMtx.Update(context, texFloat4x4);

	// TODO : 3.03 Return light position in world coordinates
	XMVECTOR globalLightPos = XMVector3TransformCoord(XMLoadFloat4(&lightPos), lightMtx);
	XMFLOAT4 globalLightPosFLoat4;
	XMStoreFloat4(&globalLightPosFLoat4, globalLightPos);
	return globalLightPosFLoat4;
}

void LightAndShadowMap::BeginShadowRender(const dx_ptr<ID3D11DeviceContext>& context,
	ConstantBuffer<XMFLOAT4X4, 2>& cbView, ConstantBuffer<XMFLOAT4X4>& cbProj) const
{
	cbView.Update(context, m_lightViewMtx);
	cbProj.Update(context, m_lightProjMtx);
	// TODO : 3.12 Set up view port of the appropriate size
	ViewportDescription viewportDescription 
		= ViewportDescription(TEXTURE_SIZE, TEXTURE_SIZE);

	context.get()->RSSetViewports(1, &viewportDescription);
	// TODO : 3.13 Bind no render targets and the shadow map as depth buffer
	context.get()->OMSetRenderTargets(0, 0, m_shadowDepthBuffer.get());
	// TODO : 3.14 Clear the depth buffer
	context.get()->ClearDepthStencilView(m_shadowDepthBuffer.get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
}