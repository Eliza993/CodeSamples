//float4 main( float4 pos : POSITION ) : SV_POSITION
//{
//	return pos;
//}

texture2D baseTexture : register(t0);
SamplerState filter: register(s0);

// A constant buffer that stores the three basic column-major matrices for composing geometry.
cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
};

// Per-vertex data used as input to the vertex shader.
struct VertexShaderInput
{
	float3 pos : POSITION;
	float2 uv : TEXCOORD;
	float3 n : NORMAL;
};

// Per-pixel color data passed through the pixel shader.
struct HullShaderInput
{
	//float3 vPosition : WORLDPOS;
	//float2 uv : TEXCOORD;
	//float4 n : NORMAL;

	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
	float4 n : NORMAL;
	float4 worldPos : POSTION;

	float3 view : CAMERAPOS;	//try to get dis to camera
};

// Simple shader to do vertex processing on the GPU.
HullShaderInput main(VertexShaderInput input)
{
	//float4 baseColor = baseTexture.Sample(filter, input.uv);
	//use this instead, because this is VS, not PS
	float4 baseColor = baseTexture[input.uv];

	float maxHeight = 10.0f;
	float ratio = baseColor.x;
	float y = ratio * maxHeight;

	HullShaderInput output;

	float4 pos = float4(input.pos, 1.0f);
	pos = mul(pos, model);
	pos.y = pos.y + y;
	output.worldPos = pos;

	//pos = mul(pos, view);
	//pos = mul(pos, projection);
	output.pos = pos;
	output.uv = input.uv;
	float4 n = float4(input.n, 0.0f);
	n = mul(n, model);
	output.n = n;

	output.view = float3(view[3][0], view[3][1], view[3][2]);

	return output;
}
