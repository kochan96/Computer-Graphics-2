#define NLIGHTS 2

texture2D normTex : register(t0);
sampler samp : register(s0);

static const float PI = 3.14159265359;

float4 lightPos[NLIGHTS];
float3 lightColor[NLIGHTS];
float4 camPos;

texture2D albedoTex;
texture2D roughnessTex;
texture2D metallicTex;

float normalDistributionGGX(float3 N, float3 H, float roughness)
{
	float r2 = roughness * roughness;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;

	float denom = (NdotH2 * (r2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return r2 / denom;
}

float geometrySchlickGGX(float a, float roughness)
{
	float q = (roughness + 1) * (roughness + 1) / 8.0;
	return a / (a * (1 - q) + q);
}

float3 normalMapping(float3 N, float3 T, float3 tn)
{
	float3 B = cross(N, T) / length(cross(N, T));
	T = cross(B, N);
	float3x3 m = float3x3(T, B, N);

	return mul(tn, m);
}

float geometrySmith(float3 N, float3 V, float3 L, float roughness)
{
	float ggx1 = geometrySchlickGGX(max(dot(N, V), 0.0), roughness);
	float ggx2 = geometrySchlickGGX(max(dot(N, L), 0.0), roughness);

	return ggx1 * ggx2;
}

float3 fresnel(float cosTheta, float3 F0)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

struct PSInput
{
	float4 pos : SV_POSITION;
	float3 worldPos : POSITION0;
	float3 norm : NORMAL0;
	float3 view : VIEWVEC0;
	float2 tex : TEXCOORD0;
	float3 tangent : TANGENT0;
};

float4 main(PSInput i) : SV_TARGET
{
	float3 NN = normalize(i.norm);
	float3 T = normalize(i.tangent);
	float3 tn = normTex.Sample(samp, i.tex);
	tn = tn * 2 - 1;
	float3 norm = normalMapping(NN, T, tn);

	float3 albedo = albedoTex.Sample(samp, i.tex);
	float3 A = pow(albedo, 2.2);
	float roughness = roughnessTex.Sample(samp, i.tex);
	float metallic = metallicTex.Sample(samp, i.tex);

	float3 N = normalize(norm);
	float3 V = normalize(i.view);
	float3 F0 = float3(0.04f, 0.04f, 0.04f) * (1 - metallic) + A * metallic;

	float3 Lo = float3(0.0, 0.0, 0.0);
	for (int idx = 0; idx < NLIGHTS; idx++)
	{
		float3 L = normalize(lightPos[idx] - i.worldPos);
		float3 H = normalize(V + L);
		float distance = length(lightPos[idx] - i.worldPos);
		float attenuation = 1.0 / (distance * distance);
		float3 radiance = lightColor[idx] * attenuation;

		float NDF = normalDistributionGGX(N, H, roughness);
		float G = geometrySmith(N, V, L, roughness);
		float3 F = fresnel(max(dot(H, V), 0.0), F0);

		float3 ks = F;
		float3 kd = (1.0 - ks) * (1.0 - metallic);

		float3 numerator = F * G * NDF;
		float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
		float3 specular = numerator / max(denominator, 0.001);

		float3 Li = radiance * max(dot(N, L), 0.0);
		Lo += (kd * albedo / PI + specular) * Li;	}

	float3 ambient = 0.03 * albedo;
	float3 color = ambient + Lo;

	color = color / (color + 1.0);
	color = pow(color, 1.0 / 2.2);

	return float4(color, 1.0);
}