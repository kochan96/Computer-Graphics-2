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
		vertices.push_back({ XMFLOAT3{ cosa*radius, sina*radius, 0.0f }, XMFLOAT3{ 0.0f, 0.0f, -1.0f } });
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

MeshLoader::vpn_mesh_t MeshLoader::CreateDoubleSidedRectangle(float width, float height)
{
	return{
	{
		{ XMFLOAT3(-0.5f*width, -0.5f*height, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(+0.5f*width, -0.5f*height, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(+0.5f*width, +0.5f*height, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(-0.5f*width, +0.5f*height, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },

		{ XMFLOAT3(-0.5f*width, -0.5f*height, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
		{ XMFLOAT3(-0.5f*width, +0.5f*height, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
		{ XMFLOAT3(+0.5f*width, +0.5f*height, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
		{ XMFLOAT3(+0.5f*width, -0.5f*height, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) }
	},
	{ 0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7 } };
}

MeshLoader::vp_mesh_t MeshLoader::CreateRectangleBilboard(float width, float height)
{
	return{
		{
			{-0.5f*width, -0.5f*height, 0.0f},
			{-0.5f*width, +0.5f*height, 0.0f},
			{+0.5f*width, +0.5f*height, 0.0f},
			{+0.5f*width, -0.5f*height, 0.0f}
		},
		{ 0, 1, 2, 0, 2, 3 }
	};
}

MeshLoader::vpn_mesh_t MeshLoader::CreateRectangle(float width, float height)
{
	return{
		{
			{ XMFLOAT3(-0.5f*width, -0.5f*height, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
			{ XMFLOAT3(-0.5f*width, +0.5f*height, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
			{ XMFLOAT3(+0.5f*width, +0.5f*height, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
			{ XMFLOAT3(+0.5f*width, -0.5f*height, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) }
		},
		{ 0, 1, 2, 0, 2, 3 }
	};
}

MeshLoader::vpn_mesh_t MeshLoader::CreateSphere(float radius, unsigned stacks, unsigned slices)
{
	assert(stacks > 2 && slices > 1);
	auto n = (stacks - 1U) * slices + 2U;
	vector<VertexPositionNormal> vertices(n);
	vertices[0].position = XMFLOAT3(0.0f, radius, 0.0f);
	vertices[0].normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
	auto dp = XM_PI / stacks;
	auto phi = dp;
	auto k = 1U;
	for (auto i = 0U; i < stacks - 1U; ++i, phi += dp)
	{
		float cosp, sinp;
		XMScalarSinCos(&sinp, &cosp, phi);
		auto thau = 0.0f;
		auto dt = XM_2PI / slices;
		auto stackR = radius * sinp;
		auto stackY = radius * cosp;
		for (auto j = 0U; j < slices; ++j, thau += dt)
		{
			float cost, sint;
			XMScalarSinCos(&sint, &cost, thau);
			vertices[k].position = XMFLOAT3(stackR * cost, stackY, stackR * sint);
			vertices[k++].normal = XMFLOAT3(cost * sinp, cosp, sint * sinp);
		}
	}
	vertices[k].position = XMFLOAT3(0.0f, -radius, 0.0f);
	vertices[k].normal = XMFLOAT3(0.0f, -1.0f, 0.0f);
	auto in = (stacks - 1U) * slices * 6U;
	vector<unsigned short> indices(in);
	k = 0U;
	for (auto j = 0U; j < slices - 1U; ++j)
	{
		indices[k++] = 0U;
		indices[k++] = j + 2;
		indices[k++] = j + 1;
	}
	indices[k++] = 0U;
	indices[k++] = 1U;
	indices[k++] = slices;
	auto i = 0U;
	for (; i < stacks - 2U; ++i)
	{
		auto j = 0U;
		for (; j < slices - 1U; ++j)
		{
			indices[k++] = i*slices + j + 1;
			indices[k++] = i*slices + j + 2;
			indices[k++] = (i + 1)*slices + j + 2;
			indices[k++] = i*slices + j + 1;
			indices[k++] = (i + 1)*slices + j + 2;
			indices[k++] = (i + 1)*slices + j + 1;
		}
		indices[k++] = i*slices + j + 1;
		indices[k++] = i*slices + 1;
		indices[k++] = (i + 1)*slices + 1;
		indices[k++] = i*slices + j + 1;
		indices[k++] = (i + 1)*slices + 1;
		indices[k++] = (i + 1)*slices + j + 1;
	}
	for (auto j = 0U; j < slices - 1U; ++j)
	{
		indices[k++] = i*slices + j + 1;
		indices[k++] = i*slices + j + 2;
		indices[k++] = n - 1;
	}
	indices[k++] = (i + 1)*slices;
	indices[k++] = i*slices + 1;
	indices[k] = n - 1;
	return{ move(vertices), move(indices) };
}

MeshLoader::vpn_mesh_t MeshLoader::CreateCylinder(float radius, float height, unsigned stacks, unsigned slices)
{
	assert(stacks > 0 && slices > 1);
	auto n = (stacks + 1) * slices;
	vector<VertexPositionNormal> vertices(n);
	auto y = height / 2;
	auto dy = height / stacks;
	auto dp = XM_2PI / slices;
	auto k = 0U;
	for (auto i = 0U; i <= stacks; ++i, y -= dy)
	{
		auto phi = 0.0f;
		for (auto j = 0U; j < slices; ++j, phi += dp)
		{
			float sinp, cosp;
			XMScalarSinCos(&sinp, &cosp, phi);
			vertices[k].position = XMFLOAT3(radius*cosp, y, radius*sinp);
			vertices[k++].normal = XMFLOAT3(cosp, 0, sinp);
		}
	}
	auto in = 6 * stacks * slices;
	vector<unsigned short> indices(in);
	k = 0;
	for (auto i = 0U; i < stacks; ++i)
	{
		auto j = 0U;
		for (; j < slices - 1; ++j)
		{
			indices[k++] = i*slices + j;
			indices[k++] = i*slices + j + 1;
			indices[k++] = (i + 1)*slices + j + 1;
			indices[k++] = i*slices + j;
			indices[k++] = (i + 1)*slices + j + 1;
			indices[k++] = (i + 1)*slices + j;
		}
		indices[k++] = i*slices + j;
		indices[k++] = i*slices;
		indices[k++] = (i + 1)*slices;
		indices[k++] = i*slices + j;
		indices[k++] = (i + 1)*slices;
		indices[k++] = (i + 1)*slices + j;
	}
	return{ move(vertices), move(indices) };
}

MeshLoader::vpn_mesh_t MeshLoader::CreateDisk(float radius, unsigned slices)
{
	assert(slices > 1);
	auto n = slices + 1;
	vector<VertexPositionNormal> vertices(n);
	vertices[0].position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	vertices[0].normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
	auto phi = 0.0f;
	auto dp = XM_2PI / slices;
	auto k = 1;
	for (auto i = 1U; i <= slices; ++i, phi += dp)
	{
		float cosp, sinp;
		XMScalarSinCos(&sinp, &cosp, phi);
		vertices[k].position = XMFLOAT3(radius * cosp, 0.0f, radius * sinp);
		vertices[k++].normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
	}
	auto in = slices * 3;
	vector<unsigned short> indices(in);
	k = 0;
	for (auto i = 0U; i < slices - 1; ++i)
	{
		indices[k++] = 0;
		indices[k++] = i + 2;
		indices[k++] = i + 1;
	}
	indices[k++] = 0;
	indices[k++] = 1;
	indices[k] = slices;
	return{ move(vertices), move(indices) };
}

MeshLoader::vpn_mesh_t MeshLoader::LoadMesh(const std::wstring& fileName)
{
	//File format for VN vertices and IN indices (IN divisible by 3, i.e. IN/3 triangles):
	//VN IN
	//pos.x pos.y pos.z norm.x norm.y norm.z tex.x tex.y [VN times, i.e. for each vertex]
	//t.i1 t.i2 t.i3 [IN/3 times, i.e. for each triangle]

	ifstream input;
	// In general we shouldn't throw exceptions on end-of-file,
	// however, in case of this file format if we reach the end
	// of a file before we read all values, the file is
	// ill-formated and we would need to throw an exception anyway
	input.exceptions(ios::badbit | ios::failbit | ios::eofbit);
	input.open(fileName);

	int vn, in;
	input >> vn >> in;

	vector<VertexPositionNormal> verts(vn);
	XMFLOAT2 ignoreTextureCoords;
	for (auto i = 0; i < vn; ++i)
		input >> verts[i].position.x >> verts[i].position.y >> verts[i].position.z
			>> verts[i].normal.x >> verts[i].normal.y >> verts[i].normal.z
			>> ignoreTextureCoords.x >> ignoreTextureCoords.y;

	vector<unsigned short> inds(in);
	for (auto i = 0; i < in; ++i)
		input >> inds[i];

	return{ move(verts), move(inds) };
}