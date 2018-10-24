#pragma once
#include <vector>
using namespace std;

namespace App1
{
	const int numGeoIns = 40;
	// Constant buffer used to send MVP matrices to the vertex shader.

	struct ModelViewProjectionConstantBuffer
	{
		DirectX::XMFLOAT4X4 model;
		DirectX::XMFLOAT4X4 view;
		DirectX::XMFLOAT4X4 projection;
	};

	//way 2 for instancing
	//struct InstanceConstantBuffer
	//{
	//	DirectX::XMFLOAT4X4 model;
	//	DirectX::XMFLOAT4X4 projection;
	//};

	struct ConstantBuffer
	{
		DirectX::XMFLOAT4 lightPos[3];	//want to use 3 types of light in the screen
		DirectX::XMFLOAT4 lightDir[3];
		DirectX::XMFLOAT4 lightColor[3];
	};

	struct GeometryConstantBuffer
	{
		//DirectX::XMFLOAT4 pos[40];
		DirectX::XMFLOAT4 pos[numGeoIns];
	};

	// Used to send per-vertex data to the vertex shader.
	struct VertexPositionColor
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 color;
	};

	struct InstanceData
	{
		DirectX::XMFLOAT3 pos;
	};
}