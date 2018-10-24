//float4 main( float4 pos : POSITION ) : SV_POSITION
//{
//	return pos;
//}

//#pragma pack_matrix(row_major)

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
	float3 posIns : POSITION_INSTANCE;
};

// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
	float4 n : NORMAL;
};

// Simple shader to do vertex processing on the GPU.
PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;
	//float4 pos = float4(input.pos, 1.0f);
	float4 pos = float4(input.pos + input.posIns, 1.0f);

	// Transform the vertex position into projected space.
	pos = mul(pos, model);

	pos = mul(pos, view);
	pos = mul(pos, projection);
	output.pos = pos;

	output.uv = input.uv;

	//move normal too
	float4 n = float4(input.n, 0.0f);
	n = mul(n, model);
	output.n = normalize(n);

	return output;
}

//
//cbuffer InstanceConstantBuffer : register(b0)
//{
//	matrix model;
//	matrix projection;
//};
//
//// Per-vertex data used as input to the vertex shader.
//struct VertexShaderInput
//{
//	float3 pos : POSITION;
//	float2 uv : TEXCOORD;
//	float3 n : NORMAL;
//};
//
//// Per-pixel color data passed through the pixel shader.
//struct PixelShaderInput
//{
//	float4 pos : SV_POSITION;
//	float2 uv : TEXCOORD;
//	float4 n : NORMAL;
//	float4 worldPos : POSTION;
//};
//
//// Simple shader to do vertex processing on the GPU.
//PixelShaderInput main(VertexShaderInput input)
//{
//	PixelShaderInput output;
//	float4 pos = float4(input.pos, 1.0f);
//	//float4 pos = float4(input.pos + input.posIns, 1.0f);
//
//	// Transform the vertex position into projected space.
//	pos = mul(pos, model);
//
//	output.worldPos = pos;
//
//	pos = mul(pos, view);
//	pos = mul(pos, projection);
//	output.pos = pos;
//
//	output.uv = input.uv;
//
//	//move normal too
//	float4 n = float4(input.n, 0.0f);
//	n = mul(n, model);
//	output.n = normalize(n);
//
//	return output;
//}
