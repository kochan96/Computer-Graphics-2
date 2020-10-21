struct Light
{
	float4 position;
	float4 color;
};

struct Lighting
{
	float4 ambient;
	float4 surface;
	Light lights[3];
};

cbuffer cbSurfaceColor : register(b0) //Pixel Shader constant buffer slot 0 - matches slot in psBilboard.hlsl
{
	float4 surfaceColor;
}

cbuffer cbLighting : register(b1) //Pixel Shader constant buffer slot 1
{
	Lighting lighting;
}

struct PSInput
{
	float4 pos : SV_POSITION;
	float3 worldPos : POSITION0;
	float3 norm : NORMAL0;
	float3 viewVec : TEXCOORD0;
};

float4 main(PSInput i) : SV_TARGET
{
	float3 viewVec = normalize(i.viewVec);
	float3 normal = normalize(i.norm);
	float3 color = lighting.ambient.xyz * lighting.surface.x;
	float specAlpha = 0.0f;
	for (int k = 0; k < 3; k++)
	{
		Light l = lighting.lights[k];
		if (l.color.w != 0) //light on
		{
			float3 lightVec = normalize(l.position.xyz - i.worldPos);
			float3 halfVec = normalize(viewVec + lightVec);
			color += l.color.xyz * surfaceColor.xyz * lighting.surface.y*clamp(dot(normal, lightVec), 0.0f, 1.0f); //diffuse color
			float nh = dot(normal, halfVec);
			nh = clamp(nh, 0.0f, 1.0);
			nh = pow(nh, lighting.surface.w);
			nh *= lighting.surface.z;
			specAlpha += nh;
			color += l.color.xyz * nh;
		}
	}

	return saturate(float4(color, surfaceColor.w + specAlpha));
}