// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
	float4 n : NORMAL;
};

texture2D baseTexture : register(t0);
texture2D detailTexture : register(t1);

SamplerState filter: register(s0); //filter 0 is Clamp, 1 is Wrap
								   
float4 main(PixelShaderInput input) : SV_TARGET
{
	float4 baseColor = baseTexture.Sample(filter, input.uv);
	//TODO: remove this after add alpha color blending
	if (baseColor.a < 1.0f)
	{
		discard;
	}
	return baseColor;
}


