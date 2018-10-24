#pragma pack_matrix(row_major)

struct VSin
{
	
	float3 coordinate : POSITION;
	float4 color : COLOR;
	float2 size : SIZE;
	float2 uv : TEXCOORD;
	float rotate : ROTATE;
	float4x4 worldMat : WORLD;
    float4x4 MVP : MVP;
};

//cbuffer ModelViewProjectionConstantBuffer : register(b0)
//{
//	float4x4 worldMatrix;
//	float4x4 viewMatrix;
//	float4x4 projMatrix;
//};

//cbuffer world : register(b0)
//{
//	float4x4 worldMatrix;
//}

//cbuffer WorldMat : register(b1)
//{
//	float4x4 viewMatrix;
//	float4x4 projMatrix;
//};

struct VSout
{
	float4 projectedCoordinate : SV_POSITION;
	float4 colorOut : COLOR;
	float2 size : SIZE;
	float2 uv : TEXCOORD;
	float rotate : ROTATE;
};

VSout main(VSin fromVertexBuffer)
{
	VSout sendToRasterizer = (VSout)0;
	
	//way1: this doesnt work, mul with view later, Quad need to be created in world space
	//sendToRasterizer.projectedCoordinate = mul(float4(fromVertexBuffer.coordinate, 1), mul(worldMatrix, viewMatrix));

	//way2:
	//sendToRasterizer.projectedCoordinate = mul(float4(fromVertexBuffer.coordinate, 1), worldMatrix);
	sendToRasterizer.projectedCoordinate = mul(float4(fromVertexBuffer.coordinate, 1), fromVertexBuffer.worldMat);
	sendToRasterizer.colorOut = fromVertexBuffer.color;
	sendToRasterizer.size = fromVertexBuffer.size;
	sendToRasterizer.uv = fromVertexBuffer.uv;
	sendToRasterizer.rotate = radians(fromVertexBuffer.rotate);

	return sendToRasterizer;
}