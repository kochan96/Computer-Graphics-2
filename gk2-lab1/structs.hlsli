struct VSIn
{
	float3 pos : POSITION;
	float3 col : COLOR;
};

struct VSOut
{
	float4 pos : SV_POSITION;
	float4 col : COLOR;
};