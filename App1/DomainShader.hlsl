cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
}; 

struct DS_OUTPUT
{
	//float4 vPosition  : SV_POSITION;

	// TODO: change/add other stuff
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
	float4 n : NORMAL;
	float4 worldPos : POSTION;
};

// Output control point
struct HS_CONTROL_POINT_OUTPUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
	float4 n : NORMAL;
	float4 worldPos : POSTION;

	//float3 vPosition : WORLDPOS; 
};

// Output patch constant data.
struct HS_CONSTANT_DATA_OUTPUT
{
	float EdgeTessFactor[3]			: SV_TessFactor; // e.g. would be [4] for a quad domain
	float InsideTessFactor			: SV_InsideTessFactor; // e.g. would be Inside[2] for a quad domain
	// TODO: change/add other stuff
};

#define NUM_CONTROL_POINTS 3

[domain("tri")]
DS_OUTPUT main(
	HS_CONSTANT_DATA_OUTPUT input,
	float3 domain : SV_DomainLocation,
	const OutputPatch<HS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> patch)
{
	DS_OUTPUT Output;

	//Output.vPosition = float4(
	//	patch[0].vPosition*domain.x+patch[1].vPosition*domain.y+patch[2].vPosition*domain.z,1);

	//Output.worldPos = float4(
	//	patch[0].worldPos*domain.x + patch[1].worldPos*domain.y + patch[2].worldPos*domain.z, 1);

	float3 wPos = patch[0].worldPos*domain.x + patch[1].worldPos*domain.y + patch[2].worldPos*domain.z;
	Output.worldPos = float4(wPos, 1);
	//TODO: lerp for other infor, may fix this later
	Output.uv = 
		patch[0].uv[0]*domain.x + patch[1].uv[1]*domain.y;

	Output.n = patch[0].n*domain.x + patch[1].n*domain.y + patch[2].n*domain.z;
	float3 pos = patch[0].pos*domain.x + patch[1].pos*domain.y + patch[2].pos*domain.z;
	Output.pos = float4(pos, 1);

	float4x4 mVP = mul(view, projection);
	Output.pos = mul(Output.pos, mVP);

	return Output;
}
