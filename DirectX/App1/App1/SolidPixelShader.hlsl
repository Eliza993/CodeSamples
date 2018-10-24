// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
	float4 n : NORMAL;
	float4 worldPos : POSTION;
};

texture2D baseTexture : register(t0);

//SamplerState filters[2]: register(s0); //filter 0 is Clamp, 1 is Wrap
SamplerState filter: register(s0); //filter 0 is Clamp, 1 is Wrap


float4 main(PixelShaderInput input) : SV_TARGET
{
	float4 baseColor = baseTexture.Sample(filter, input.uv);

	return baseColor;
}


