#pragma once

#include <DirectXMath.h>
#include <memory>
#include "Mesh.h"

class Object
{
public:
	Object(std::shared_ptr<Mesh> mesh);

	const std::vector<ID3D11Buffer*>& getVertexBuffers();
	ID3D11Buffer* getIndexBuffer();
	UINT getNumIndices();

	const std::vector<UINT>& getVertexStrides();
	const std::vector<UINT>& getVertexOffsets();

	void setPosition(float x, float y, float z);
	void setPosition(const DirectX::XMFLOAT3& position);
	DirectX::XMFLOAT3 getPosition();
	void setRotation(float x, float y, float z);
	void setScale(float x, float y, float z);
	DirectX::XMFLOAT4X4 getTransform();
	DirectX::XMFLOAT4X4 getTransformGlobal();

	void addChild(std::shared_ptr<Object> child);
	void removeChild(std::shared_ptr<Object> child);
	void removeAllChildren();
	bool hasChild(std::shared_ptr<Object> child);
	uint32_t getChildrenNum();
	Object* getParent();

private:
	void update();
	void updateGlobal();
	void setDirty();

private:
	DirectX::XMFLOAT4X4 pos{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
	DirectX::XMFLOAT4X4 rot{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
	DirectX::XMFLOAT4X4 scale{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
	DirectX::XMFLOAT4X4 transform{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
	DirectX::XMFLOAT4X4 transformGlobal{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };

	DirectX::XMFLOAT4X4 posInverse{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
	DirectX::XMFLOAT4X4 rotInverse{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
	DirectX::XMFLOAT4X4 scaleInverse{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
	DirectX::XMFLOAT4X4 transformInverse{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };

	std::shared_ptr<Mesh> mesh;

	bool dirty{ false };

	std::vector<std::shared_ptr<Object>> children;
	Object* parent{ nullptr };
};