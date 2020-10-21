#include "meshLoader.h"
#include <fstream>

using namespace std;
using namespace DirectX;
using namespace mini;


MeshLoader::vpn_mesh_t MeshLoader::CreatePentagon(float radius)
{
	vpn_verts_t vertices;
	vertices.reserve(5);
	float a = 0, da = XM_2PI / 5.0f;
	for (int i = 0; i < 5; ++i, a -= da)
	{
		float sina, cosa;
		XMScalarSinCos(&sina, &cosa, a);
		vertices.push_back(
			{
				XMFLOAT3{ cosa*radius, sina*radius, 0.0f },
				XMFLOAT3{ 0.0f, 0.0f, -1.0f }
			});
	}
	return{ move(vertices), { 0, 1, 2, 0, 2, 3, 0, 3, 4 } };
}

MeshLoader::vpn_verts_t MeshLoader::BoxVertices(float width, float height, float depth)
{
	return{
		//Front face
		{ XMFLOAT3(-0.5f*width, -0.5f*height, -0.5f*depth), XMFLOAT3(0.0f, 0.0f, -1.0f) },
		{ XMFLOAT3(+0.5f*width, -0.5f*height, -0.5f*depth), XMFLOAT3(0.0f, 0.0f, -1.0f) },
		{ XMFLOAT3(+0.5f*width, +0.5f*height, -0.5f*depth), XMFLOAT3(0.0f, 0.0f, -1.0f) },
		{ XMFLOAT3(-0.5f*width, +0.5f*height, -0.5f*depth), XMFLOAT3(0.0f, 0.0f, -1.0f) },

		//Back face
		{ XMFLOAT3(+0.5f*width, -0.5f*height, +0.5f*depth), XMFLOAT3(0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(-0.5f*width, -0.5f*height, +0.5f*depth), XMFLOAT3(0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(-0.5f*width, +0.5f*height, +0.5f*depth), XMFLOAT3(0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(+0.5f*width, +0.5f*height, +0.5f*depth), XMFLOAT3(0.0f, 0.0f, 1.0f) },

		//Left face
		{ XMFLOAT3(-0.5f*width, -0.5f*height, +0.5f*depth), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(-0.5f*width, -0.5f*height, -0.5f*depth), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(-0.5f*width, +0.5f*height, -0.5f*depth), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(-0.5f*width, +0.5f*height, +0.5f*depth), XMFLOAT3(-1.0f, 0.0f, 0.0f) },

		//Right face
		{ XMFLOAT3(+0.5f*width, -0.5f*height, -0.5f*depth), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(+0.5f*width, -0.5f*height, +0.5f*depth), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(+0.5f*width, +0.5f*height, +0.5f*depth), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(+0.5f*width, +0.5f*height, -0.5f*depth), XMFLOAT3(1.0f, 0.0f, 0.0f) },

		//Bottom face
		{ XMFLOAT3(-0.5f*width, -0.5f*height, +0.5f*depth), XMFLOAT3(0.0f, -1.0f, 0.0f) },
		{ XMFLOAT3(+0.5f*width, -0.5f*height, +0.5f*depth), XMFLOAT3(0.0f, -1.0f, 0.0f) },
		{ XMFLOAT3(+0.5f*width, -0.5f*height, -0.5f*depth), XMFLOAT3(0.0f, -1.0f, 0.0f) },
		{ XMFLOAT3(-0.5f*width, -0.5f*height, -0.5f*depth), XMFLOAT3(0.0f, -1.0f, 0.0f) },

		//Top face
		{ XMFLOAT3(-0.5f*width, +0.5f*height, -0.5f*depth), XMFLOAT3(0.0f, 1.0f, 0.0f) },
		{ XMFLOAT3(+0.5f*width, +0.5f*height, -0.5f*depth), XMFLOAT3(0.0f, 1.0f, 0.0f) },
		{ XMFLOAT3(+0.5f*width, +0.5f*height, +0.5f*depth), XMFLOAT3(0.0f, 1.0f, 0.0f) },
		{ XMFLOAT3(-0.5f*width, +0.5f*height, +0.5f*depth), XMFLOAT3(0.0f, 1.0f, 0.0f) },
	};
}

MeshLoader::indices_t MeshLoader::BoxIndices()
{
	return {
		0,2,1, 0,3,2,
		4,6,5, 4,7,6,
		8,10,9, 8,11,10,
		12,14,13, 12,15,14,
		16,18,17, 16,19,18,
		20,22,21, 20,23,22
	};
}

MeshLoader::vpn_mesh_t MeshLoader::CreateDoubleSidedRectangle(float width,
	float height)
{
	vpn_verts_t vertices;
	vertices.push_back({
		XMFLOAT3{ -0.5f * width, -0.5f * height, 0 },
		XMFLOAT3{ 0.0f, 0.0f, -1.0f} });
	vertices.push_back({
		XMFLOAT3{ 0.5f * width, -0.5f * height, 0},
		XMFLOAT3{ 0.0f, 0.0f, -1.0f } });
	vertices.push_back({
		XMFLOAT3{ 0.5f * width, 0.5f * height, 0.0f},
		XMFLOAT3{ 0.0f, 0.0f, -1.0f } });
	vertices.push_back({
		XMFLOAT3{-0.5f * width, 0.5f * height, 0.0f},
		XMFLOAT3{ 0.0f, 0.0f, -1.0f } });
	vertices.push_back({
		XMFLOAT3{ -0.5f * width, -0.5f * height, 0 },
		XMFLOAT3{ 0.0f, 0.0f, 1.0f } });
	vertices.push_back({
		XMFLOAT3{ 0.5f * width, -0.5f * height, 0 },
		XMFLOAT3{ 0.0f, 0.0f, 1.0f } });
	vertices.push_back({
		XMFLOAT3{ 0.5f * width, 0.5f * height, 0.0f },
		XMFLOAT3{ 0.0f, 0.0f, 1.0f } });
	vertices.push_back({
		XMFLOAT3{ -0.5f * width, 0.5f * height, 0.0f },
		XMFLOAT3{ 0.0f, 0.0f, 1.0f } });
	return { vertices, {0,2,1, 0,3,2, 4,5,6, 4,6,7} };
}

MeshLoader::vp_mesh_t MeshLoader::CreateRectangleBilboard(float width, float height)
// Initialize vertex and index list for bilboards
{
	return
	{
		{
			{ -0.5f*width, -0.5f*height, 0.0f },
			{ -0.5f*width, +0.5f*height, 0.0f },
			{ +0.5f*width, +0.5f*height, 0.0f },
			{ +0.5f*width, -0.5f*height, 0.0f }
		},
			{0,1,2,0,2,3}
	};
}