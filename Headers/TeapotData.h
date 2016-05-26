#pragma once

#include <vector>
#include <DirectXMath.h>

struct TeapotData
{
	TeapotData();
	std::vector<DirectX::XMFLOAT3> points;
	std::vector<std::vector<uint32_t>> patches;
};