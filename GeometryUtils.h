#pragma once

#include <DirectXMath.h>
#include <vector>

struct CheckboardPlaneMesh
{
	std::vector<DirectX::XMFLOAT3> positions;
	std::vector<DirectX::XMFLOAT3> normals;
	std::vector<uint32_t> colorIds;
	std::vector<uint32_t> indices;
};

class PlaneGeometry
{
public:
	static CheckboardPlaneMesh generateCheckBoard(float width, float height, int numSegmentsH, int numSegmentsV)
	{
		CheckboardPlaneMesh mesh;

		float segW{ width / numSegmentsH };
		float segH{ height / numSegmentsV };

		uint32_t ind{ 0 };
		for (int i{ 0 }; i < numSegmentsV; ++i)
		{
			for (int j{ 0 }; j < numSegmentsH; ++j)
			{
				DirectX::XMFLOAT3 p1{ j * segW, 0, i * segH };
				DirectX::XMFLOAT3 p2{ (j + 1) * segW, 0, i * segH };
				DirectX::XMFLOAT3 p3{ j * segW, 0, (i + 1) * segH };
				DirectX::XMFLOAT3 p4{ (j + 1) * segW, 0, (i + 1) * segH };

				uint32_t ind1{ 0 };
				uint32_t ind2{ 1 };
				uint32_t ind3{ 2 };
				uint32_t ind4{ 1 };
				uint32_t ind5{ 3 };
				uint32_t ind6{ 2 };

				mesh.positions.push_back(p1);
				mesh.positions.push_back(p2);
				mesh.positions.push_back(p3);
				mesh.positions.push_back(p4);

				mesh.normals.emplace_back(0.0f, 1.0f, 0.0f);
				mesh.normals.emplace_back(0.0f, 1.0f, 0.0f);
				mesh.normals.emplace_back(0.0f, 1.0f, 0.0f);
				mesh.normals.emplace_back(0.0f, 1.0f, 0.0f);

				uint32_t colorId{ static_cast<uint32_t>(((j % 2) == (i % 2)) ? 0 : 1) };
				mesh.colorIds.push_back(colorId);
				mesh.colorIds.push_back(colorId);
				mesh.colorIds.push_back(colorId);
				mesh.colorIds.push_back(colorId);

				mesh.indices.push_back(ind + ind1);
				mesh.indices.push_back(ind + ind2);
				mesh.indices.push_back(ind + ind3);
				mesh.indices.push_back(ind + ind4);
				mesh.indices.push_back(ind + ind5);
				mesh.indices.push_back(ind + ind6);

				ind += 4;
			}
		}

		return mesh;
	}
};