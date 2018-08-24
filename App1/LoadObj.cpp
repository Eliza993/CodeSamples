#include "pch.h"
#include "LoadObj.h"

bool loadOBJ(const char * path, vector<XMFLOAT3>& out_vertices, vector<XMFLOAT2>& out_uvs, vector<XMFLOAT3>& out_normals)
{
	//infor of Vertex, UV, Normal
	vector<XMFLOAT3> tempVertex;
	vector<XMFLOAT2> tempUV;
	vector<XMFLOAT3> tempNormal;
	//Index, tell us which one to use
	vector<unsigned int> vertexIndices, uvIndices, normalIndices;

	FILE * file;
	errno_t err = 0;
	err = fopen_s(&file, path, "r");
	if (err != 0)
	{
		printf("Can't open file \n");
		return false;
	}

	//read the file
	while (true)
	{
		char lineHeader[128];
		//EOF = end of file, need to add size in here (128)
		if (fscanf_s(file, "%s", lineHeader, 128) == EOF)
		{
			break;
		}

		//compare 2 chars, 0 means same, < 0 or > 0 mean not
		if (strcmp(lineHeader, "v") == 0)
		{
			XMFLOAT3 vertex;
			//read next 3 floats + put in var
			fscanf_s(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			tempVertex.push_back(vertex);
		}
		else if(strcmp(lineHeader, "vt") == 0)
		{
			XMFLOAT2 uv;
			fscanf_s(file, "%f %f\n", &uv.x, &uv.y);		
			uv.y = 1 - uv.y; //because in OpenGL, V is flipped
			tempUV.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0)
		{
			XMFLOAT3 normal;
			fscanf_s(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			tempNormal.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0)
		{
			//for each Tri
			string vertex1, vertex2, vertex3;			
			unsigned int vIndex[3], uvIndex[3], nIndex[3];
			int count = fscanf_s(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vIndex[0], &uvIndex[0], &nIndex[0], 
				&vIndex[1], &uvIndex[1], &nIndex[1], &vIndex[2], &uvIndex[2], &nIndex[2]);

			//there should be 9 indices for each tri
			if (count != 9)
			{
				printf("Error, please try to load another file\n");
				return false;
			}

			vertexIndices.push_back(vIndex[0]);
			vertexIndices.push_back(vIndex[1]);
			vertexIndices.push_back(vIndex[2]);
			uvIndices.push_back(uvIndex[0]);
			uvIndices.push_back(uvIndex[1]);
			uvIndices.push_back(uvIndex[2]);
			normalIndices.push_back(nIndex[0]);
			normalIndices.push_back(nIndex[1]);
			normalIndices.push_back(nIndex[2]);
		}
	}

	//Processing the data
	//for each vertex
	for (unsigned int i = 0; i < vertexIndices.size(); i++)
	{
		//need to -1 because OBJ start index at 1 instead of 0 like c++
		XMFLOAT3 vertex = tempVertex[vertexIndices[i] - 1];
		out_vertices.push_back(vertex);
	}

	for (unsigned int i = 0; i < uvIndices.size(); i++)
	{
		XMFLOAT2 uv = tempUV[uvIndices[i] - 1];
		out_uvs.push_back(uv);
	}

	for (unsigned int i = 0; i < normalIndices.size(); i++)
	{
		XMFLOAT3 normal = tempNormal[normalIndices[i] - 1];
		out_normals.push_back(normal);
	}
}

//void GetUniqueVertexAndIndices(const vector<XMFLOAT3> in_vertices, const vector<XMFLOAT2> in_uvs, const vector<XMFLOAT3> in_normals, unique_ptr<vector<VertexUvNormal>> vertices, unique_ptr<vector<unsigned short>> indices)
//void GetUniqueVertexAndIndices(const vector<XMFLOAT3> in_vertices, const vector<XMFLOAT2> in_uvs, const vector<XMFLOAT3> in_normals, vector<VertexUvNormal>* vertices, vector<unsigned short>* indices)
void GetUniqueVertexAndIndices(const vector<XMFLOAT3> in_vertices, const vector<XMFLOAT2> in_uvs, const vector<XMFLOAT3> in_normals, vector<VertexUvNormal>& vertices, vector<unsigned short>& indices)
{
	int x = in_vertices.size();
	int y = in_uvs.size();
	int z = in_normals.size();

	unsigned short index = 0;
	bool unique = true;
	for (unsigned int i = 0; i < in_vertices.size(); i++)
	{
		unique = true;

		//compare to all in list to check if vertex is unique
		//for (unsigned int j = 0; j < vertices->size(); j++)
		//for (unsigned int j = 0; j < vertices.size(); j++)
		//{
		//	if (in_vertices[i].x == vertices[j].pos.x && in_vertices[i].y == vertices[j].pos.y &&in_vertices[i].z == vertices[j].pos.z &&
		//		in_uvs[i].x == vertices[j].uv.x && in_uvs[i].y == vertices[j].uv.y &&
		//		in_normals[i].x == vertices[j].n.x && in_normals[i].y == vertices[j].n.y && in_normals[i].z == vertices[j].n.z)
		//	{
		//		unique = false;
		//		break;
		//	}
		//}

		indices.push_back(index);

		if (unique)
		{
			XMFLOAT3 v = { in_vertices[i].x, in_vertices[i].y, in_vertices[i].z };
			XMFLOAT2 uv = { in_uvs[i].x, in_uvs[i].y };
			XMFLOAT3 n = { in_normals[i].x, in_normals[i].y, in_normals[i].z };
			VertexUvNormal vertex = {v, uv, n};
			vertices.push_back(vertex);

			index++;
		}

	}
}

void IndexSwitchClockWiseAntiClockWise(vector<unsigned short>& indices)
{
	unsigned short temp = 0;
	for (unsigned int i = 0; i < indices.size(); i++)
	{
		if (i%3 == 0)
		{
			continue;
		}
		else if(i%3 == 1)
		{
			temp = indices[i];
		}
		else if(i%3 == 2)
		{
			indices[i - 1] = indices[i];
			indices[i] = temp;
		}
	}
}
