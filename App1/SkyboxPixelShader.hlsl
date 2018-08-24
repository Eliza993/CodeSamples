textureCUBE env: register(t0);
SamplerState envFilter: register(s0);

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 uv : TEXCOORD;
	float4 n : NORMAL;
};

// A pass-through function for the (interpolated) color data.
float4 main(PixelShaderInput input) : SV_TARGET
{
	return env.Sample(envFilter, input.uv);
}


