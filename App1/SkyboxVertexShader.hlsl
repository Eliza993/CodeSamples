// A constant buffer that stores the three basic column-major matrices for composing geometry.
cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
};

struct VertexShaderInput
{
	float3 pos : POSITION;
	float2 uv : TEXCOORD;
	float3 n : NORMAL;
};

// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 uv : TEXCOORD;	//make it float3 to put pos in
	float4 n : NORMAL;
};

// Simple shader to do vertex processing on the GPU.
PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;
	//make float3 ->float4 to multiply
	float4 pos = float4(input.pos, 1.0f);

	// Transform the vertex position into projected space.
	pos = mul(pos, model);

	pos = mul(pos, view);
	pos = mul(pos, projection);
	output.pos = pos;

	//this will be use to draw Skybox texture
	output.uv = input.pos;

	//move normal too
	float4 n = float4(input.n, 0.0f);
	n = mul(n, model);
	output.n = normalize(n);

	return output;
}