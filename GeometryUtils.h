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

struct BoneArmatureMesh
{
	std::vector<DirectX::XMFLOAT3> positions;
	std::vector<uint32_t> indices;
};

class GeometryGenerator
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
				DirectX::XMFLOAT3 p1{ j * segW - width * 0.5f, 0.0f, i * segH - height * 0.5f };
				DirectX::XMFLOAT3 p2{ (j + 1) * segW - width * 0.5f, 0.0f, i * segH - height * 0.5f };
				DirectX::XMFLOAT3 p3{ j * segW - width * 0.5f, 0.0f, (i + 1) * segH - height * 0.5f };
				DirectX::XMFLOAT3 p4{ (j + 1) * segW - width * 0.5f, 0.0f, (i + 1) * segH - height * 0.5f };

				uint32_t ind1{ 0 };
				uint32_t ind2{ 2 };
				uint32_t ind3{ 1 };
				uint32_t ind4{ 1 };
				uint32_t ind5{ 2 };
				uint32_t ind6{ 3 };

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

	static BoneArmatureMesh generateBone(float len)
	{
		BoneArmatureMesh mesh;

		const float SHORT_LEN{ len > 0.02f ? 0.02f : len / 2.0f };
		const float SIZE{ 0.02f };

		DirectX::XMFLOAT3 p1{ 0, 0, 0 };
		DirectX::XMFLOAT3 p2{ SIZE, SHORT_LEN, -SIZE };
		DirectX::XMFLOAT3 p3{ SIZE, SHORT_LEN, SIZE };
		DirectX::XMFLOAT3 p4{ -SIZE, SHORT_LEN, SIZE };
		DirectX::XMFLOAT3 p5{ -SIZE, SHORT_LEN, -SIZE };
		DirectX::XMFLOAT3 p6{ 0, len, 0 };

		mesh.positions.push_back(p1);
		mesh.positions.push_back(p2);
		mesh.positions.push_back(p3);
		mesh.positions.push_back(p4);
		mesh.positions.push_back(p5);
		mesh.positions.push_back(p6);

		mesh.indices.push_back(0);
		mesh.indices.push_back(1);
		mesh.indices.push_back(2);

		mesh.indices.push_back(0);
		mesh.indices.push_back(2);
		mesh.indices.push_back(3);
					 
		mesh.indices.push_back(0);
		mesh.indices.push_back(3);
		mesh.indices.push_back(4);
					 
		mesh.indices.push_back(0);
		mesh.indices.push_back(4);
		mesh.indices.push_back(1);
					 
		mesh.indices.push_back(5);
		mesh.indices.push_back(2);
		mesh.indices.push_back(1);
					 
		mesh.indices.push_back(5);
		mesh.indices.push_back(3);
		mesh.indices.push_back(2);
					 
		mesh.indices.push_back(5);
		mesh.indices.push_back(4);
		mesh.indices.push_back(3);
					 
		mesh.indices.push_back(5);
		mesh.indices.push_back(1);
		mesh.indices.push_back(4);

		return mesh;
	}
};