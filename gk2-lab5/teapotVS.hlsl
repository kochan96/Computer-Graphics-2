matrix modelMtx, modelInvTMtx, viewProjMtx;
float4 camPos;
float h0;
float time, xmax, vmax, thalf, tmax;

struct VSInput
{
	float3 pos : POSITION0;
	float3 norm : NORMAL0;
	float2 tex : TEXCOORD0;
};

struct VSOutput
{
	float4 pos : SV_POSITION;
	float3 worldPos : POSITION0;
	float3 norm : NORMAL0;
	float3 view : VIEWVEC0;
	float2 tex : TEXCOORD0;
};

float spring_height(float time)
{
	return xmax * exp(log(0.5) * time / thalf) * sin(time * vmax / xmax);
}

VSOutput main(VSInput i)
{
	float4 worldPos = mul(modelMtx, float4(i.pos, 1.0f));
	float integer_part;
	float t = modf(time / tmax, integer_part) * tmax;
	worldPos.y += h0 + spring_height(t);

	VSOutput o;
	o.tex = i.tex / 4;
	o.view = normalize(camPos.xyz - worldPos.xyz);
	o.norm = normalize(mul(modelInvTMtx, float4(i.norm, 0.0f)).xyz);
	o.worldPos = worldPos.xyz;
	o.pos = mul(viewProjMtx, worldPos);
	return o;
}