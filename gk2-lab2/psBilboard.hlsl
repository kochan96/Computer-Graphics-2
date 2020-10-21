cbuffer cbSurfaceColor : register(b0) //Pixel Shader constant buffer slot 0
{
	float4 surfaceColor;
}

struct PSInput
{
	float4 pos : SV_POSITION;
	float2 tex : TEXCOORD0;
};

float4 main(PSInput i) : SV_TARGET
{
	// Calculate bilboard pixel color
	float d = length(i.tex);
	d = clamp(1 - d, 0.0f, 1.0f);
	return float4(surfaceColor.xyz * d, 1.0f);
}