Texture2D colorMap : register(t0);
Texture2D posterTex : register(t1);
SamplerState colorSampler : register(s0);

struct PSInput
{
	float4 pos : SV_POSITION;
	float2 tex: TEXCOORD0;
	float2 tex1: TEXCOORD1;
};

float4 main(PSInput i) : SV_TARGET
{
	float4 color = posterTex.Sample(colorSampler, i.tex1);
	if(color.w == 0.0f)
		color = colorMap.Sample(colorSampler, i.tex);
	
	return color;
}