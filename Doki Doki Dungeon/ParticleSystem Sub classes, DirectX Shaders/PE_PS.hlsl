#include "LightingFunctions.hlsli"
struct VSout
{
	float4 projectedCoordinate : SV_POSITION;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
};

struct MaterialComponent
{
	uint componentType;
	int textureIndex;
	uint2 padding;
	float4 values;
};
// Emissive = 0, Ambient = 1, Diffuse = 2, NormalMap = 3, Bump = 4, Transparency = 5, Displacement = 6, Vector_Displacement = 7, Specular = 8, Shininess = 9, eReflection = 10
cbuffer Material : register(b1)
{
	MaterialComponent components[11];
	uint numOfComponents;
	uint3 materialPadding;
}

Texture2D textures[4] : register(t0);

SamplerState state : register(s0);

static const uint emissiveTex = 0;
static const uint diffuseTex = 1;
static const uint normalMap = 2;
static const uint specularTex = 3;

static const uint lightColor = 0;
static const uint lightDir = 1;
static const uint lightPos = 2;
static const uint lightInfo = 3;

float4 main(VSout input) : SV_TARGET
{
	float4 colorFinal = textures[diffuseTex].Sample(state, input.uv);
	
	if (colorFinal.a == 0)
	{
		discard;
	}

	return (colorFinal * input.color);

	//return float4(1, 0, 0, 1);
}