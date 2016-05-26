#pragma once

#include <vector>
#include "Object.h"
#include "GeometryUtils.h"
#include "BufferFactory.h"
#include "TeapotData.h"

class ObjectFactory
{
public:
	ObjectFactory(std::shared_ptr<BufferFactory> bufferFactory) : bufferFactory{ bufferFactory }
	{

	}

	std::shared_ptr<Object> createPlane()
	{
		CheckboardPlaneMesh checkboardPlaneMesh{ GeometryGenerator::generateCheckBoard(10.0f, 10.0f, 20, 20) };

		BufferDataCollection bufferDataCollection;
		bufferDataCollection.vertexBufferDatas.push_back(bufferFactory->createVertexBuffer(checkboardPlaneMesh.positions));
		bufferDataCollection.vertexBufferDatas.push_back(bufferFactory->createVertexBuffer(checkboardPlaneMesh.normals));
		bufferDataCollection.vertexBufferDatas.push_back(bufferFactory->createVertexBuffer(checkboardPlaneMesh.colorIds));
		bufferDataCollection.indexBufferData = bufferFactory->createIndexBuffer(checkboardPlaneMesh.indices);

		shared_ptr<Mesh> mesh{ make_shared<Mesh>(bufferDataCollection) };

		return make_shared<Object>(mesh);
	}

	std::vector<std::shared_ptr<Object>> createKitana()
	{
		std::vector<std::shared_ptr<Object>> kitana;

		vector<Bone> bones;
		vector<vector<Vertex>> meshes;
		vector<vector<uint32_t>> faces;
		MeshAsciiParser::read("kitana", bones, meshes, faces);

		for (int i{ 0 }; i < meshes.size(); ++i)
		{
			if (i == 6 || i == 16 || i == 17 || i == 18 || i == 21 || i == 23 || i == 24 || i == 25 || i == 27)
			{
				vector<Vertex> meshData{ meshes[i] };
				vector<XMFLOAT3> vertices;
				vector<XMFLOAT3> normals;
				vector<XMINT3> bones;
				vector<XMFLOAT3> bonesWeights;
				for (auto& vertex : meshData)
				{
					vertices.push_back(vertex.pos);
					normals.push_back(vertex.normal);
					bones.push_back(vertex.bones);
					bonesWeights.push_back(vertex.bonesWeights);
				}

				vector<uint32_t> face{ faces[i] };
				vector<uint32_t> indices;
				for (uint32_t ind : face)
				{
					indices.push_back(ind);
				}

				BufferDataCollection bufferDataCollection;
				bufferDataCollection.vertexBufferDatas.push_back(bufferFactory->createVertexBuffer(vertices));
				bufferDataCollection.vertexBufferDatas.push_back(bufferFactory->createVertexBuffer(normals));
				bufferDataCollection.vertexBufferDatas.push_back(bufferFactory->createVertexBuffer(bones));
				bufferDataCollection.vertexBufferDatas.push_back(bufferFactory->createVertexBuffer(bonesWeights));
				bufferDataCollection.indexBufferData = bufferFactory->createIndexBuffer(indices);

				shared_ptr<Mesh> mesh{ make_shared<Mesh>(bufferDataCollection) };

				kitana.push_back(make_shared<Object>(mesh));
			}
		}

		return kitana;
	}

	std::vector<std::shared_ptr<Object>> createTeapot()
	{
		std::vector<std::shared_ptr<Object>> teapot;

		TeapotData teapotData;
		CheckboardPlaneMesh checkboardPlaneMesh{ GeometryGenerator::generateCheckBoard(10.0f, 10.0f, 20, 20) };

		for (auto& patch : teapotData.patches)
		{
			BufferDataCollection bufferDataCollection;
			bufferDataCollection.vertexBufferDatas.push_back(bufferFactory->createVertexBuffer(teapotData.points));
			bufferDataCollection.indexBufferData = bufferFactory->createIndexBuffer(patch);

			shared_ptr<Mesh> mesh{ make_shared<Mesh>(bufferDataCollection) };

			teapot.push_back(make_shared<Object>(mesh));
		}
		
		return teapot;
	}

private:
	std::shared_ptr<BufferFactory> bufferFactory;
};