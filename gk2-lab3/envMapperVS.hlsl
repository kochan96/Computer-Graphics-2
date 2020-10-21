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

struct VSInput
{
	float3 pos : POSITION;
	float3 norm : NORMAL0;
};

struct PSInput
{
	float4 pos : SV_POSITION;
	float3 tex: TEXCOORD0;
};

PSInput main(VSInput i)
{
	PSInput o = (PSInput)0;
	o.pos = float4(i.pos, 1.0f);
	o.pos = mul(worldMatrix, o.pos);

	float4 new_norm = float4(i.norm, 0.0f);
	new_norm = mul(worldMatrix, new_norm);
	new_norm = normalize(new_norm);

	float4 cam_pos = mul(invViewMatrix, float4(0.0f, 0.0f, 0.0f, 1.0f));
	float4 vec_from_cam = o.pos - cam_pos;

	o.tex = reflect(vec_from_cam, new_norm).xyz;
	o.pos = mul(viewMatrix, o.pos);
	o.pos = mul(projMatrix, o.pos);
	return o;
}
