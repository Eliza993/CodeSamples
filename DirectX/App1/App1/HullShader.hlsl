// Input control point
struct VS_CONTROL_POINT_OUTPUT
{
	//float3 vPosition : WORLDPOS;
	
	// TODO: change/add other stuff
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
	float4 n : NORMAL;
	float4 worldPos : POSTION;

	float3 view : CAMERAPOS;
};

// Output control point
struct HS_CONTROL_POINT_OUTPUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
	float4 n : NORMAL;
	float4 worldPos : POSTION;

	//float3 vPosition : WORLDPOS; 
	//float2 uv : TEXCOORD;
	//float4 n : NORMAL;
};

// Output patch constant data.
struct HS_CONSTANT_DATA_OUTPUT
{
	float EdgeTessFactor[3]			: SV_TessFactor; // e.g. would be [4] for a quad domain
	float InsideTessFactor			: SV_InsideTessFactor; // e.g. would be Inside[2] for a quad domain
	// TODO: change/add other stuff
};

#define NUM_CONTROL_POINTS 3

// Patch Constant Function
HS_CONSTANT_DATA_OUTPUT CalcHSPatchConstants(
	InputPatch<VS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> ip,
	uint PatchID : SV_PrimitiveID)
{
	HS_CONSTANT_DATA_OUTPUT Output;

	//lerp by dis to Camera
	float maxDistance = 100.0f;
	float3 cameraPos = float3(ip[0].view);
	float dist = distance(cameraPos, ip[0].pos);
	int tessValue = lerp(1, 5, 1 - dist / maxDistance);	//start from 0 because it will disappear at 0 

	Output.EdgeTessFactor[0] =
		Output.EdgeTessFactor[1] =
		Output.EdgeTessFactor[2] =
		Output.InsideTessFactor = tessValue;

	return Output;
}

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("CalcHSPatchConstants")]
HS_CONTROL_POINT_OUTPUT main( 
	InputPatch<VS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> ip, 
	uint i : SV_OutputControlPointID,
	uint PatchID : SV_PrimitiveID )
{
	HS_CONTROL_POINT_OUTPUT Output;

	// Insert code to compute Output here
	Output.pos = ip[i].pos; //may not need this now
	Output.uv = ip[i].uv;
	Output.n = ip[i].n;
	Output.worldPos = ip[i].worldPos;

	return Output;
}
