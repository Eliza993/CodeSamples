//#pragma pack_matrix(row_major)

cbuffer ConstantBuffer : register(b1)
{
	vector lightPos[3];
	vector lightDir[3];
	vector lightColor[3];
};

//float4 main() : SV_TARGET
//{
//	return float4(1.0f, 1.0f, 1.0f, 1.0f);
//}

// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
	float4 n : NORMAL;
	float4 worldPos : POSTION;
};

texture2D baseTexture : register(t0);
texture2D detailTexture : register(t1);
//will add 2nd filter in later
//SamplerState filters[2]: register(s0); //filter 0 is Clamp, 1 is Wrap
SamplerState filter: register(s0); //filter 0 is Clamp, 1 is Wrap


// A pass-through function for the (interpolated) color data.
float4 main(PixelShaderInput input) : SV_TARGET
//float4 main(PixelShaderInput input, float4 worldPos : POSITION, float4 worldNormal: NORMAL) : SV_TARGET

//can add lighting in here later
//float4 main(float2 baseUV: TEXCOORD0, float2 detailUV: TEXCOORD1, float4 modulate: COLOR) : SV_TARGET
{
	//TODO: 4 - find barycentric and color later
	//float4 baseColor = baseTexture.Sample(filters[0], baseUV) * modulate;
	//float4  detailColor = detailTexture.Sample(filters[1], detailUV);
	
	//float4 baseColor = baseTexture.Sample(filter, baseUV) * modulate;
	//float4  detailColor = detailTexture.Sample(filter, detailUV);

	////lerp RGB and keep A
	//float4 color = float4(lerp(baseColor.rgb, detailColor.rgb, detailColor.a), baseColor.a);

	float4 baseColor = baseTexture.Sample(filter, input.uv);

	if (baseColor.a < 1.0f)
	{
		discard;
	}
	
	//Directional Light
	float dLightRatio = saturate(saturate(dot(-lightDir[0].xyz, input.n.xyz)) + 0.3f);
	float4 colorDirect = saturate(dLightRatio * lightColor[0] * baseColor);

	//Point Light
	float radius = 5.0f;	
	float3 pLightDir = normalize((lightPos[1].xyz - input.worldPos.xyz));
	float pLightRatio = saturate(dot(pLightDir, normalize(input.n.xyz)));
	//way 1:
	//float4 colorPoint =  saturate(lightColor[1] * pLightRatio* baseColor);

	//way 2:
	float attenuation = 1 - saturate((length(lightPos[1].xyz - input.worldPos.xyz) / radius));
	attenuation = attenuation*attenuation;
	float4 colorPoint = saturate(attenuation * lightColor[1] * baseColor *pLightRatio);

	//Spot Light
	
	////way 1:
	//float coneRatio = 0.9;
	////float3 sLightDir = normalize(lightPos[2].xyz - input.worldPos.xyz);
	////float surfaceRatio = saturate(dot(-sLightDir, lightDir[2].xyz));
	//float3 sLightDir = normalize(lightPos[2] - input.worldPos);
	//float surfaceRatio = saturate(dot(-sLightDir, normalize(lightDir[2])));
	//float spotFactor = 0;
	//if (surfaceRatio > coneRatio)
	//{
	//	spotFactor = 1;
	//}
	//float sLightRatio = saturate(dot(sLightDir, input.n));
	//float4 colorSpot = saturate(spotFactor  * lightColor[2] * sLightRatio * baseColor);

	//way 2:
	float innerConeRatio = 0.95;
	float outerConeRatio = 0.8;

	float3 sLightDir = normalize(lightPos[2].xyz - input.worldPos.xyz);
	float surfaceRatio = saturate(dot(-sLightDir, normalize(lightDir[2].xyz)));
	float spotFactor = 0;
	if (surfaceRatio > outerConeRatio)
	{
		spotFactor = 1;
	}
	float sLightRatio = saturate(dot(sLightDir, normalize(input.n.xyz)));

	float spotAttenuation = 1 - saturate((innerConeRatio - surfaceRatio) / (innerConeRatio - outerConeRatio));
	spotAttenuation = spotAttenuation * spotAttenuation;

	float4 colorSpot = saturate(spotFactor  * lightColor[2] * spotAttenuation * baseColor * sLightRatio);
	
	float4 color = 0;
	color = saturate(colorDirect + colorPoint + colorSpot);

	return color;
}


