#pragma once
#include "windowApplication.h"
#include "dxDevice.h"
#include "vertexPositionColor.h"
#include <vector>
#include <DirectXMath.h>

using namespace DirectX;
using namespace mini;

class DxApplication : public mini::WindowApplication
{
public:
	explicit DxApplication(HINSTANCE hInstance);
	std::vector<XMFLOAT2> CreateTriangleVertices();

protected:
	int MainLoop() override;
	bool ProcessMessage(WindowMessage& msg) override;

private:
	void Render();
	void Draw0();
	void Draw1();

	static std::vector<VertexPositionColor> CreateCubeVertices();
	static std::vector<unsigned short> CreateCubeIndices();
	
	DxDevice m_device; 
	DirectX::XMFLOAT4X4 m_modelMtx0, m_modelMtx1, m_viewMtx, m_projMtx;
	mini::dx_ptr<ID3D11Buffer> m_cbMVP;
	mini::dx_ptr<ID3D11Buffer> m_indexBuffer;
	mini::dx_ptr<ID3D11RenderTargetView> m_backBuffer;
	mini::dx_ptr<ID3D11DepthStencilView> m_depthBuffer;
	mini::dx_ptr<ID3D11Buffer> m_vertexBuffer;
	mini::dx_ptr<ID3D11VertexShader> m_vertexShader;
	mini::dx_ptr<ID3D11PixelShader> m_pixelShader;
	mini::dx_ptr<ID3D11InputLayout> m_layout;

	float camera_angle = -0.0f;
	float camera_distance = 10.0f;
	void RecalculateViewMtx();
	POINT curr_mouse_pos;
	bool left_mouse_down = false;
	bool right_mouse_down = false;

	LARGE_INTEGER li;
	double PCFreq = 0.0;
	double angle = 0;
	__int64 counterStart = 0;
};
