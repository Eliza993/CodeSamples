#pragma once
#include <vector>
#include <cmath>
#include <WindowsNumerics.h>
//#include "ShaderStructures.h"

//#include "../Content/ShaderStructures.h"
using namespace std;
using namespace DirectX;
//using namespace App1;

struct VertexUvNormal
{
	XMFLOAT3 pos;
	XMFLOAT2 uv;
	XMFLOAT3 n;
};

bool loadOBJ(const char * path, vector<XMFLOAT3> & out_vertices, vector<XMFLOAT2> & out_uvs, vector<XMFLOAT3> & out_normals);
//void GetUniqueVertexAndIndices(const vector<XMFLOAT3> in_vertices, const vector<XMFLOAT2> in_uvs, const vector<XMFLOAT3> in_normals, unique_ptr<vector<VertexUvNormal>> vertices, unique_ptr<vector<unsigned short>> indices);
//void GetUniqueVertexAndIndices(const vector<XMFLOAT3> in_vertices, const vector<XMFLOAT2> in_uvs, const vector<XMFLOAT3> in_normals, vector<VertexUvNormal>* vertices, vector<unsigned short>* indices);
void GetUniqueVertexAndIndices(const vector<XMFLOAT3> in_vertices, const vector<XMFLOAT2> in_uvs, const vector<XMFLOAT3> in_normals, vector<VertexUvNormal>& vertices, vector<unsigned short>& indices);
void IndexSwitchClockWiseAntiClockWise(vector<unsigned short>& indices);
