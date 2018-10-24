//this GS convert Line to Quad
#pragma pack_matrix(row_major) 

cbuffer WorldMat : register(b0)
{
	float4x4 viewMatrix;
	float4x4 projMatrix;
};

struct GSOutput
{
	float4 pos : SV_POSITION;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
};

struct GSInput
{
	float4 pos : SV_POSITION;
	float4 color : COLOR;
	float2 size : SIZE;
	float2 uv : TEXCOORD; //in this case, dont use UV
	float rotate : ROTATE;
};

//texture + billboard
[maxvertexcount(4)]	//make point to Quad
void main(
	line GSInput gI[2],
	inout TriangleStream< GSOutput > output
)
{
	float2 halfSize = gI[0].size *0.5f;
	int numElement = 4;

	float sine, cosine;
	sincos(gI[0].rotate, sine, cosine);
	float3 viewUp = normalize(float3(viewMatrix._12, viewMatrix._22, viewMatrix._32));
	float3 viewRight = normalize(float3(viewMatrix._11, viewMatrix._21, viewMatrix._31));
	float3 viewRight2 = normalize(cosine*viewRight - sine*viewUp);
	float3 viewUp2 = normalize(sine*viewRight + cosine*viewUp);

	//float4 pos1 = float4(float3(gI[0].pos.xyz + (-viewRight2*halfSize.x + viewUp2*halfSize.y)), 1.0f);
	//float4 pos2 = float4(float3(gI[0].pos.xyz + (+viewRight2*halfSize.x + viewUp2*halfSize.y)), 1.0f);
	//float4 pos3 = float4(float3(gI[0].pos.xyz + (-viewRight2*halfSize.x - viewUp2*halfSize.y)), 1.0f);
	//float4 pos4 = float4(float3(gI[0].pos.xyz + (+viewRight2*halfSize.x - viewUp2*halfSize.y)), 1.0f);

	float4 pos1 = float4(float3(gI[0].pos.xyz + ( + viewUp2*halfSize.y)), 1.0f);
	float4 pos2 = float4(float3(gI[1].pos.xyz + ( + viewUp2*halfSize.y)), 1.0f);
	float4 pos3 = float4(float3(gI[0].pos.xyz + ( - viewUp2*halfSize.y)), 1.0f);
	float4 pos4 = float4(float3(gI[1].pos.xyz + ( - viewUp2*halfSize.y)), 1.0f);

	float4 c = gI[0].color;

	//GSOutput element[4] =
	//{
	//	pos1, c, float2(0, 0),
	//	pos2, c, float2(1, 0),
	//	pos3, c, float2(0, 1),
	//	pos4, c, float2(1, 1),
	//};

	//GSOutput element[4] =
	//{
	//	pos1, c, float2(0, gI[0].uv.x),
	//	pos2, c, float2(0, gI[1].uv.x),
	//	pos3, c, float2(1, gI[0].uv.x),
	//	pos4, c, float2(1, gI[1].uv.x),
	//};

	GSOutput element[4] =
	{
		pos1, c, float2(gI[0].uv.x, 0),
		pos2, c, float2(gI[1].uv.x, 0),
		pos3, c, float2(gI[0].uv.x, 1),
		pos4, c, float2(gI[1].uv.x, 1),
	};

	//float4x4 mVP = mul(view, projection);
	float4x4 mVP = mul(viewMatrix, projMatrix);
	for (int i = 0; i < numElement; i++)
	{
		element[i].pos = mul(element[i].pos, mVP);
		output.Append(element[i]);
	}
	output.RestartStrip();
}