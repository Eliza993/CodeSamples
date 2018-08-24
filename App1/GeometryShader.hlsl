//this GS convert 1 Point to Triangle

cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
};

//instancing in geometric
cbuffer GeometryConstantBuffer: register(b1)
{
	float4 posIns[40];
};

struct GSOutput
{
	float4 pos : SV_POSITION;
	//float3 color : COLOR;

	float2 uv : TEXCOORD;
	float4 n : NORMAL;
};

struct GSInput
{
	float4 pos : SV_POSITION;
	float3 color : COLOR0;
	uint insID: SV_InstanceID;
};


//texture + billboard
[maxvertexcount(6)]	//make point to Quad
void main(
	point GSInput gI[1],
	inout TriangleStream< GSOutput > output
)
{
	gI[0].pos = mul(gI[0].pos, model);

	int numElement = 6;
	float iX, iY, iZ;
	iX = posIns[gI[0].insID].x;
	iY = posIns[gI[0].insID].y;
	iZ = posIns[gI[0].insID].z;

	GSOutput element[6] =
	{
		float4(gI[0].pos.x + iX,        gI[0].pos.y + iY ,	     gI[0].pos.z + iZ, 1.0f), float2(0, 0), float4(0, 0, -1.0f, 1),
		float4(gI[0].pos.x + iX - 3.0f, gI[0].pos.y + iY,	     gI[0].pos.z + iZ, 1.0f), float2(1, 0), float4(0, 0, -1.0f, 1),
		float4(gI[0].pos.x + iX,		gI[0].pos.y + iY - 2.0f, gI[0].pos.z + iZ, 1.0f), float2(0, 1), float4(0, 0, -1.0f, 1),
		float4(gI[0].pos.x + iX - 3.0f, gI[0].pos.y + iY,	     gI[0].pos.z + iZ, 1.0f), float2(1, 0), float4(0, 0, -1.0f, 1),
		float4(gI[0].pos.x + iX,		gI[0].pos.y + iY - 2.0f, gI[0].pos.z + iZ, 1.0f), float2(0, 1), float4(0, 0, -1.0f, 1),
		float4(gI[0].pos.x + iX - 3.0f, gI[0].pos.y + iY - 2.0f, gI[0].pos.z + iZ, 1.0f), float2(1, 1), float4(0, 0, -1.0f, 1)
	};

	//GSOutput element[6] =
	//{
	//	float4(gI[0].pos.x + iX,        gI[0].pos.y + iY ,	     gI[0].pos.z + iZ, 1.0f), float2(0, 0), float4(0, 0, -1.0f, 1),
	//	float4(gI[0].pos.x + iX,		gI[0].pos.y + iY - 2.0f, gI[0].pos.z + iZ, 1.0f), float2(0, 1), float4(0, 0, -1.0f, 1),
	//	float4(gI[0].pos.x + iX - 3.0f, gI[0].pos.y + iY,	     gI[0].pos.z + iZ, 1.0f), float2(1, 0), float4(0, 0, -1.0f, 1),
	//	float4(gI[0].pos.x + iX - 3.0f, gI[0].pos.y + iY,	     gI[0].pos.z + iZ, 1.0f), float2(1, 0), float4(0, 0, -1.0f, 1),
	//	float4(gI[0].pos.x + iX - 3.0f, gI[0].pos.y + iY - 2.0f, gI[0].pos.z + iZ, 1.0f), float2(1, 1), float4(0, 0, -1.0f, 1),
	//	float4(gI[0].pos.x + iX,		gI[0].pos.y + iY - 2.0f, gI[0].pos.z + iZ, 1.0f), float2(0, 1), float4(0, 0, -1.0f, 1)
	//};

	float4x4 mVP = mul(view, projection);
	for (uint i = 0; i < numElement; i++)
	{
		element[i].pos = mul(element[i].pos, mVP);
		output.Append(element[i]);
	}
	output.RestartStrip();
}

////texture 

//[maxvertexcount(6)]	//make point to Quad
//void main(
//	point GSInput gI[1],
//	inout TriangleStream< GSOutput > output
//)
//{
//	int numElement = 6;
//	float iX, iY, iZ;
//	iX = posIns[gI[0].insID].x;
//	iY = posIns[gI[0].insID].y;
//	iZ = posIns[gI[0].insID].z;
//
//	GSOutput element[6] =
//	{
//		float4(gI[0].pos.x + iX,        gI[0].pos.y + iY ,	     gI[0].pos.z + iZ, 1.0f), float2(0, 0), float4(0, 0, -1.0f, 1),
//		float4(gI[0].pos.x + iX - 3.0f, gI[0].pos.y + iY,	     gI[0].pos.z + iZ, 1.0f), float2(1, 0), float4(0, 0, -1.0f, 1),
//		float4(gI[0].pos.x + iX,		gI[0].pos.y + iY - 2.0f, gI[0].pos.z + iZ, 1.0f), float2(0, 1), float4(0, 0, -1.0f, 1),
//		float4(gI[0].pos.x + iX - 3.0f, gI[0].pos.y + iY,	     gI[0].pos.z + iZ, 1.0f), float2(1, 0), float4(0, 0, -1.0f, 1),
//		float4(gI[0].pos.x + iX,		gI[0].pos.y + iY - 2.0f, gI[0].pos.z + iZ, 1.0f), float2(0, 1), float4(0, 0, -1.0f, 1),
//		float4(gI[0].pos.x + iX - 3.0f, gI[0].pos.y + iY - 2.0f, gI[0].pos.z + iZ, 1.0f), float2(1, 1), float4(0, 0, -1.0f, 1)
//	};
//
//	float4x4 mVP = mul(view, projection);
//	for (uint i = 0; i < numElement; i++)
//	{
//		element[i].pos = mul(element[i].pos, mVP);
//		output.Append(element[i]);
//	}
//	output.RestartStrip();
//}

////color only

////[maxvertexcount(3)] //make point to Tri
//[maxvertexcount(6)]	//make point to Quad
//void main(
//	//point GSOutput input[1] : SV_POSITION,
//	point GSInput gI[1],
//	inout TriangleStream< GSOutput > output
//
//)
//{
//	//int numElement = 3;
//	int numElement = 6;
//	float iX, iY, iZ;
//	iX = posIns[gI[0].insID].x;
//	iY = posIns[gI[0].insID].y;
//	iZ = posIns[gI[0].insID].z;
//
//	//GSOutput element[3] =
//	//{
//	//	float4(gI[0].pos.x + iX,        gI[0].pos.y + iY ,	   gI[0].pos.z + iZ, 1.0f),      gI[0].color,
//	//	float4(gI[0].pos.x + iX - 1.0f, gI[0].pos.y + iY,	       gI[0].pos.z + iZ, 1.0f),  gI[0].color ,
//	//	float4(gI[0].pos.x + iX,		gI[0].pos.y + iY - 1.0f, gI[0].pos.z + iZ, 1.0f),    gI[0].color
//	//};
//
//	GSOutput element[6] =
//	{
//		float4(gI[0].pos.x + iX,        gI[0].pos.y + iY ,	     gI[0].pos.z + iZ, 1.0f), gI[0].color,
//		float4(gI[0].pos.x + iX - 1.0f, gI[0].pos.y + iY,	     gI[0].pos.z + iZ, 1.0f), gI[0].color,
//		float4(gI[0].pos.x + iX,		gI[0].pos.y + iY - 1.0f, gI[0].pos.z + iZ, 1.0f), gI[0].color,
//		float4(gI[0].pos.x + iX - 1.0f, gI[0].pos.y + iY,	     gI[0].pos.z + iZ, 1.0f), gI[0].color,
//		float4(gI[0].pos.x + iX,		gI[0].pos.y + iY - 1.0f, gI[0].pos.z + iZ, 1.0f), gI[0].color,
//		float4(gI[0].pos.x + iX - 1.0f, gI[0].pos.y + iY - 1.0f, gI[0].pos.z + iZ, 1.0f), gI[0].color
//	};
//
//	float4x4 mVP = mul(view, projection);
//	for (uint i = 0; i < numElement; i++)
//	{
//		element[i].pos = mul(element[i].pos, mVP);
//		output.Append(element[i]);
//	}
//
//	output.RestartStrip();
//}