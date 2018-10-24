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
	float3 color : COLOR0;
};

// Per-pixel color data passed through the pixel shader.
struct GSInput
{
	float4 pos : SV_POSITION;
	float3 color : COLOR0;
	uint insID: SV_InstanceID;
};

// Simple shader to do vertex processing on the GPU.
GSInput main(VertexShaderInput input, uint insIndex: SV_InstanceID)
{
	GSInput output;
	float4 pos = float4(input.pos, 1.0f);

	// Transform the vertex position into projected space.
	//pos = mul(pos, model);

	output.pos = pos;

	// Pass the color through without modification.
	output.color = input.color;

	output.insID = insIndex;
	return output;
}
