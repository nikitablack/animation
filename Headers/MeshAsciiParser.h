#pragma once

#include <string>
#include <regex>
#include <fstream>
#include <DirectXMath.h>
#include <vector>
#include <cassert>

using namespace std;
using namespace DirectX;

namespace detail
{
	char* memblock{ new char[4] };
}

using namespace detail;

struct Bone
{
	int32_t parent;
	string name;
	XMFLOAT3 pos;
};

struct Vertex
{
	XMFLOAT3 pos;
	XMFLOAT3 normal;
	XMINT4 color;
	XMFLOAT2 uv;
	XMINT3 bones;
	XMFLOAT3 bonesWeights;
};

ostream& operator<<(ostream& os, Bone& bone)
{
	os.write(reinterpret_cast<char*>(&bone.parent), sizeof(int32_t));

	uint32_t numChars{ static_cast<uint32_t>(bone.name.size()) };
	os.write(reinterpret_cast<char*>(&numChars), sizeof(uint32_t));
	os.write(bone.name.data(), sizeof(char) * numChars);

	os.write(reinterpret_cast<char*>(&bone.pos.x), sizeof(float));
	os.write(reinterpret_cast<char*>(&bone.pos.y), sizeof(float));
	os.write(reinterpret_cast<char*>(&bone.pos.z), sizeof(float));

	return os;
}

ostream& operator<<(ostream& os, Vertex& vertex)
{
	os.write(reinterpret_cast<char*>(&vertex.pos.x), sizeof(float));
	os.write(reinterpret_cast<char*>(&vertex.pos.y), sizeof(float));
	os.write(reinterpret_cast<char*>(&vertex.pos.z), sizeof(float));

	os.write(reinterpret_cast<char*>(&vertex.normal.x), sizeof(float));
	os.write(reinterpret_cast<char*>(&vertex.normal.y), sizeof(float));
	os.write(reinterpret_cast<char*>(&vertex.normal.z), sizeof(float));

	os.write(reinterpret_cast<char*>(&vertex.color.x), sizeof(int32_t));
	os.write(reinterpret_cast<char*>(&vertex.color.y), sizeof(int32_t));
	os.write(reinterpret_cast<char*>(&vertex.color.z), sizeof(int32_t));
	os.write(reinterpret_cast<char*>(&vertex.color.w), sizeof(int32_t));

	os.write(reinterpret_cast<char*>(&vertex.uv.x), sizeof(float));
	os.write(reinterpret_cast<char*>(&vertex.uv.y), sizeof(float));

	os.write(reinterpret_cast<char*>(&vertex.bones.x), sizeof(int32_t));
	os.write(reinterpret_cast<char*>(&vertex.bones.y), sizeof(int32_t));
	os.write(reinterpret_cast<char*>(&vertex.bones.z), sizeof(int32_t));

	os.write(reinterpret_cast<char*>(&vertex.bonesWeights.x), sizeof(float));
	os.write(reinterpret_cast<char*>(&vertex.bonesWeights.y), sizeof(float));
	os.write(reinterpret_cast<char*>(&vertex.bonesWeights.z), sizeof(float));

	return os;
}

istream& operator >> (istream& is, Bone& bone)
{
	is.read(memblock, sizeof(int32_t));
	bone.parent = *(reinterpret_cast<int32_t*>(memblock));

	is.read(memblock, sizeof(uint32_t));
	uint32_t numChars{ *(reinterpret_cast<uint32_t*>(memblock)) };

	char* strBlock{ new char[numChars] };
	is.read(strBlock, sizeof(char) * numChars);
	bone.name = string{ strBlock, numChars };
	delete[] strBlock;

	is.read(memblock, sizeof(float));
	bone.pos.x = *(reinterpret_cast<float*>(memblock));
	is.read(memblock, sizeof(float));
	bone.pos.y = *(reinterpret_cast<float*>(memblock));
	is.read(memblock, sizeof(float));
	bone.pos.z = *(reinterpret_cast<float*>(memblock));

	return is;
}

istream& operator>>(istream& is, Vertex& vertex)
{
	// pos
	is.read(memblock, sizeof(float));
	vertex.pos.x = *(reinterpret_cast<float*>(memblock));
	is.read(memblock, sizeof(float));
	vertex.pos.y = *(reinterpret_cast<float*>(memblock));
	is.read(memblock, sizeof(float));
	vertex.pos.z = *(reinterpret_cast<float*>(memblock));

	// normal
	is.read(memblock, sizeof(float));
	vertex.normal.x = *(reinterpret_cast<float*>(memblock));
	is.read(memblock, sizeof(float));
	vertex.normal.y = *(reinterpret_cast<float*>(memblock));
	is.read(memblock, sizeof(float));
	vertex.normal.z = *(reinterpret_cast<float*>(memblock));

	// color
	is.read(memblock, sizeof(int32_t));
	vertex.color.x = *(reinterpret_cast<int32_t*>(memblock));
	is.read(memblock, sizeof(int32_t));
	vertex.color.y = *(reinterpret_cast<int32_t*>(memblock));
	is.read(memblock, sizeof(int32_t));
	vertex.color.z = *(reinterpret_cast<int32_t*>(memblock));
	is.read(memblock, sizeof(int32_t));
	vertex.color.w = *(reinterpret_cast<int32_t*>(memblock));

	// uv
	is.read(memblock, sizeof(float));
	vertex.uv.x = *(reinterpret_cast<float*>(memblock));
	is.read(memblock, sizeof(float));
	vertex.uv.y = *(reinterpret_cast<float*>(memblock));

	// bones
	is.read(memblock, sizeof(int32_t));
	vertex.bones.x = *(reinterpret_cast<int32_t*>(memblock));
	is.read(memblock, sizeof(int32_t));
	vertex.bones.y = *(reinterpret_cast<int32_t*>(memblock));
	is.read(memblock, sizeof(int32_t));
	vertex.bones.z = *(reinterpret_cast<int32_t*>(memblock));

	// bones weights
	is.read(memblock, sizeof(float));
	vertex.bonesWeights.x = *(reinterpret_cast<float*>(memblock));
	is.read(memblock, sizeof(float));
	vertex.bonesWeights.y = *(reinterpret_cast<float*>(memblock));
	is.read(memblock, sizeof(float));
	vertex.bonesWeights.z = *(reinterpret_cast<float*>(memblock));

	return is;
}

class MeshAsciiParser
{
private:
	enum class ParseState
	{
		NONE,
		BONES,
		MESHES,
		UV_LAYERS,
		TEXTURES,
		VERTICES
	};

	enum class BoneState
	{
		NAME,
		PARENT,
		POSITION
	};

	enum class VertexState
	{
		POSITION,
		NORMAL,
		COLOR,
		UV,
		BONES,
		BONES_WEIGHTS
	};

public:
	static void parse(string filePath)
	{
		vector<Bone> bones;
		vector<vector<Vertex>> meshes;
		vector<vector<uint32_t>> faces;

		ParseState currState{ ParseState::NONE };
		VertexState currVertexState;

		regex bonesStartPattern("(\\d+) *# *bones");
		smatch bonesStartMatch;
		
		regex meshesStartPattern("(\\d+) *# *meshes");
		smatch meshesStartMatch;

		regex uvLayersStartPattern("(\\d+) *# *uv *layers");
		smatch uvLayersStartMatch;

		regex texturesStartPattern("(\\d+) *# *textures");
		smatch texturesStartMatch;

		regex verticesStartPattern("(\\d+) *# *vertices");
		smatch verticesStartMatch;

		regex facesStartPattern("(\\d+) *# *faces");
		smatch facesStartMatch;

		ifstream infile(filePath);

		string line;
		while (getline(infile, line))
		{
			if (regex_search(line, bonesStartMatch, bonesStartPattern))
			{
				int numBones{ stoi(bonesStartMatch.str(1)) };
				bones = parseBones(numBones, infile);
				continue;
			}
			else if (regex_search(line, meshesStartMatch, meshesStartPattern))
			{
				int numMeshes{ stoi(meshesStartMatch.str(1)) };
				currState = ParseState::MESHES;
				currVertexState = VertexState::POSITION;
				continue;
			}
			else if (regex_search(line, uvLayersStartMatch, uvLayersStartPattern))
			{
				int numUvLayers{ stoi(uvLayersStartMatch.str(1)) };
				currState = ParseState::UV_LAYERS;
				continue;
			}
			else if (regex_search(line, texturesStartMatch, texturesStartPattern))
			{
				int numTextures{ stoi(texturesStartMatch.str(1)) };
				currState = ParseState::TEXTURES;
				continue;
			}
			else if (regex_search(line, verticesStartMatch, verticesStartPattern))
			{
				int numVertices{ stoi(verticesStartMatch.str(1)) };
				meshes.push_back(parseVertices(numVertices, infile));
				continue;
			}
			else if (regex_search(line, facesStartMatch, facesStartPattern))
			{
				int numFaces{ stoi(facesStartMatch.str(1)) };
				faces.push_back(parseFaces(numFaces, infile));
				continue;
			}
		}

		ofstream outfile("kitana", ios::binary | ios::out | ios::trunc);

		uint32_t numBones{ static_cast<uint32_t>(bones.size()) };
		outfile.write(reinterpret_cast<char*>(&numBones), sizeof(uint32_t));

		for (Bone& bone : bones)
		{
			outfile << bone;
		}

		uint32_t numMeshes{ static_cast<uint32_t>(meshes.size()) };
		outfile.write(reinterpret_cast<char*>(&numMeshes), sizeof(uint32_t));

		for (int i{ 0 }; i < meshes.size(); ++i)
		{
			// write vertices
			vector<Vertex> vertices{ meshes[i] };

			uint32_t numVertices{ static_cast<uint32_t>(vertices.size()) };
			outfile.write(reinterpret_cast<char*>(&numVertices), sizeof(uint32_t));
			
			for (Vertex& vertex : vertices)
			{
				outfile << vertex;
			}

			// write indices
			vector<uint32_t> indices{ faces[i] };

			uint32_t numIndices{ static_cast<uint32_t>(indices.size()) };
			outfile.write(reinterpret_cast<char*>(&numIndices), sizeof(uint32_t));

			for (uint32_t ind : indices)
			{
				outfile.write(reinterpret_cast<char*>(&ind), sizeof(uint32_t));
			}
		}
	}

	static void read(const string& filePath, vector<Bone>& bones, vector<vector<Vertex>>& meshes, vector<vector<uint32_t>>& faces)
	{
		ifstream infile(filePath, ios::binary | ios::in);

		infile.read(memblock, sizeof(uint32_t));
		uint32_t numBones{ *(reinterpret_cast<uint32_t*>(memblock)) };

		for (uint32_t i{ 0 }; i < numBones; ++i)
		{
			Bone bone;
			infile >> bone;

			bones.push_back(bone);
		}

		infile.read(memblock, sizeof(uint32_t));
		uint32_t numMeshes{ *(reinterpret_cast<uint32_t*>(memblock)) };

		for (uint32_t i{ 0 }; i < numMeshes; ++i)
		{
			// meshes
			vector<Vertex> vertices;

			infile.read(memblock, sizeof(uint32_t));
			uint32_t numVertices{ *(reinterpret_cast<uint32_t*>(memblock)) };

			for (uint32_t j{ 0 }; j < numVertices; ++j)
			{
				Vertex vertex;
				infile >> vertex;

				vertices.push_back(vertex);
			}

			meshes.push_back(vertices);

			// faces
			vector<uint32_t> indicies;

			infile.read(memblock, sizeof(uint32_t));
			uint32_t numIndices{ *(reinterpret_cast<uint32_t*>(memblock)) };

			for (uint32_t j{ 0 }; j < numIndices; ++j)
			{
				infile.read(memblock, sizeof(uint32_t));
				uint32_t ind{ *(reinterpret_cast<uint32_t*>(memblock)) };

				indicies.push_back(ind);
			}

			faces.push_back(indicies);
		}
	}

private:
	static vector<uint32_t> parseFaces(int num, ifstream& infile)
	{
		num *= 3;

		vector<uint32_t> data;
		regex threeIntsPattern("(\\d+) +(\\d+) +(\\d+)");
		smatch threeIntsMatch;

		string line;
		while (getline(infile, line))
		{
			regex_search(line, threeIntsMatch, threeIntsPattern);
			assert(threeIntsMatch.size() == 4);

			data.push_back(static_cast<uint32_t>(stoi(threeIntsMatch.str(1))));
			data.push_back(static_cast<uint32_t>(stoi(threeIntsMatch.str(2))));
			data.push_back(static_cast<uint32_t>(stoi(threeIntsMatch.str(3))));

			if (data.size() == num)
			{
				break;
			}
		}

		assert(num == data.size());

		return data;
	}

	static vector<Vertex> parseVertices(int num, ifstream& infile)
	{
		vector<Vertex> data;
		regex threeFloatsPattern("(.*) +(.*) +(.*)");
		smatch threeFloatsMatch;
		regex fourIntsPattern("(\\d+) +(\\d+) +(\\d+) +(\\d+)");
		smatch fourIntsMatch;
		regex twoFloatsPattern("(.*) +(.*)");
		smatch twoFloatsMatch;
		regex threeIntsPattern("(\\d+) +(\\d+) +(\\d+)");
		smatch threeIntsMatch;
		VertexState currState{ VertexState::POSITION };

		string line;
		while (getline(infile, line))
		{
			if (currState == VertexState::POSITION)
			{
				regex_search(line, threeFloatsMatch, threeFloatsPattern);
				assert(threeFloatsMatch.size() == 4);

				Vertex vertex;
				vertex.pos = { stof(threeFloatsMatch.str(1)), stof(threeFloatsMatch.str(2)), stof(threeFloatsMatch.str(3)) };

				data.push_back(vertex);

				currState = VertexState::NORMAL;
			}
			else if (currState == VertexState::NORMAL)
			{
				regex_search(line, threeFloatsMatch, threeFloatsPattern);
				assert(threeFloatsMatch.size() == 4);

				Vertex& vertex{ data[data.size() - 1] };
				vertex.normal = { stof(threeFloatsMatch.str(1)), stof(threeFloatsMatch.str(2)), stof(threeFloatsMatch.str(3)) };

				currState = VertexState::COLOR;
			}
			else if (currState == VertexState::COLOR)
			{
				regex_search(line, fourIntsMatch, fourIntsPattern);
				assert(fourIntsMatch.size() == 5);

				Vertex& vertex{ data[data.size() - 1] };
				vertex.color = { stoi(fourIntsMatch.str(1)), stoi(fourIntsMatch.str(2)), stoi(fourIntsMatch.str(3)), stoi(fourIntsMatch.str(4)) };

				currState = VertexState::UV;
			}
			else if (currState == VertexState::UV)
			{
				regex_search(line, twoFloatsMatch, twoFloatsPattern);
				assert(twoFloatsMatch.size() == 3);

				Vertex& vertex{ data[data.size() - 1] };
				vertex.uv = { stof(twoFloatsMatch.str(1)), stof(twoFloatsMatch.str(2)) };

				currState = VertexState::BONES;
			}
			else if (currState == VertexState::BONES)
			{
				regex_search(line, threeIntsMatch, threeIntsPattern);
				assert(threeIntsMatch.size() == 4);

				Vertex& vertex{ data[data.size() - 1] };
				vertex.bones = { stoi(threeIntsMatch.str(1)), stoi(threeIntsMatch.str(2)), stoi(threeIntsMatch.str(3)) };

				currState = VertexState::BONES_WEIGHTS;
			}
			else if (currState == VertexState::BONES_WEIGHTS)
			{
				regex_search(line, threeFloatsMatch, threeFloatsPattern);
				assert(threeFloatsMatch.size() == 4);

				Vertex& vertex{ data[data.size() - 1] };
				vertex.bonesWeights = { stof(threeFloatsMatch.str(1)), stof(threeFloatsMatch.str(2)), stof(threeFloatsMatch.str(3)) };

				currState = VertexState::POSITION;

				if (data.size() == num)
				{
					break;
				}
			}
		}

		assert(num == data.size());

		return data;
	}

	static vector<Bone> parseBones(int num, ifstream& infile)
	{
		vector<Bone> data;
		regex threeFloatsPattern("(.*) +(.*) +(.*)");
		smatch threeFloatsMatch;
		BoneState currState{ BoneState::NAME };

		string line;
		while (getline(infile, line))
		{
			if (currState == BoneState::NAME)
			{
				Bone bone;
				bone.name = line;

				data.push_back(bone);

				currState = BoneState::PARENT;
			}
			else if (currState == BoneState::PARENT)
			{
				Bone& bone{ data[data.size() - 1] };
				bone.parent = stoi(line);

				currState = BoneState::POSITION;
			}
			else if (currState == BoneState::POSITION)
			{
				regex_search(line, threeFloatsMatch, threeFloatsPattern);
				assert(threeFloatsMatch.size() == 4);

				Bone& bone{ data[data.size() - 1] };
				bone.pos = { stof(threeFloatsMatch.str(1)), stof(threeFloatsMatch.str(2)), stof(threeFloatsMatch.str(3)) };

				currState = BoneState::NAME;

				if (data.size() == num)
				{
					break;
				}
			}
		}

		assert(num == data.size());

		return data;
	}
};