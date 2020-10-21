cbuffer cbWorld : register(b0) //Vertex Shader constant buffer slot 0
{
	matrix worldMatrix;
};

cbuffer cbView : register(b1) //Vertex Shader constant buffer slot 1
{
	matrix viewMatrix;
	matrix invViewMatrix;
};

cbuffer cbProj : register(b2) //Vertex Shader constant buffer slot 2
{
	matrix projMatrix;
};

struct PSInput
{
	float4 pos : SV_POSITION;
	float2 tex : TEXCOORD0;
};

PSInput main( float3 pos : POSITION )
{
	PSInput o;
	o.pos = float4(pos, 1.0f);

	// Calculate on-screen position of bilboard vertex
	o.pos = mul(invViewMatrix, o.pos);
	
	float4 a = float4(0.0f, 0.0f, 0.0f, 1.0f);
	a = -mul(invViewMatrix, a);
	o.pos += a;
	o.pos.w = 1.0f;

	o.pos = mul(worldMatrix, o.pos);
	o.pos = mul(viewMatrix, o.pos);
	o.pos = mul(projMatrix, o.pos);

	o.tex = pos.xy;
	return o;
}