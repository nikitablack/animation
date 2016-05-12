#pragma once

#include <string>
#include <regex>
#include <fstream>

using namespace std;

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

public:
	static void parse(string filePath)
	{
		ParseState currState{ ParseState::NONE };

		int numBones;
		int numMeshes;
		int numUvLayers;
		int numTextures;
		int numVertices;

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

		ifstream infile(filePath);

		string line;
		while (getline(infile, line))
		{
			if (regex_search(line, bonesStartMatch, bonesStartPattern))
			{
				numBones = stoi(bonesStartMatch.str(1));
				currState = ParseState::BONES;
			}
			else if (regex_search(line, meshesStartMatch, meshesStartPattern))
			{
				numMeshes = stoi(meshesStartMatch.str(1));
				currState = ParseState::MESHES;
			}
			else if (regex_search(line, uvLayersStartMatch, uvLayersStartPattern))
			{
				numUvLayers = stoi(uvLayersStartMatch.str(1));
				currState = ParseState::UV_LAYERS;
			}
			else if (regex_search(line, texturesStartMatch, texturesStartPattern))
			{
				numTextures = stoi(texturesStartMatch.str(1));
				currState = ParseState::TEXTURES;
			}
			else if (regex_search(line, verticesStartMatch, verticesStartPattern))
			{
				numVertices = stoi(verticesStartMatch.str(1));
				currState = ParseState::VERTICES;
			}

			volatile int i;
			if (currState == ParseState::BONES)
			{
				i = 1;
			}
			else if (currState == ParseState::MESHES)
			{
				i = 1;
			}
			else if (currState == ParseState::TEXTURES)
			{
				i = 1;
			}
			else if (currState == ParseState::UV_LAYERS)
			{
				i = 1;
			}
			else if (currState == ParseState::VERTICES)
			{
				i = 1;
			}
		}
	}
};