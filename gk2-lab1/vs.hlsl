#include "structs.hlsli"

cbuffer transformations : register(b0)
{
	matrix MVP;
}

VSOut main(VSIn i)
{
	VSOut o;
	o.pos = mul(MVP, float4(i.pos, 1.0f));
	o.col = float4(i.col, 1.0f);
	return o;
}
