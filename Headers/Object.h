#pragma once

#include <DirectXMath.h>
#include <memory>
#include "Mesh.h"

class Object
{
public:
	Object();
	Object(std::shared_ptr<Mesh> mesh);

	bool haveMesh();

	const std::vector<ID3D11Buffer*>& getVertexBuffers();
	ID3D11Buffer* getIndexBuffer();
	UINT getNumIndices();

	const std::vector<UINT>& getVertexStrides();
	const std::vector<UINT>& getVertexOffsets();

	void setPosition(float x, float y, float z);
	void setPosition(const DirectX::XMFLOAT3& position);
	void addPosition(float x, float y, float z);
	void addPosition(const DirectX::XMFLOAT3& position);
	DirectX::XMFLOAT3 getPosition();
	void setRotation(float x, float y, float z);
	void addRotation(float x, float y, float z);
	void addRotation(const DirectX::XMFLOAT3& axis, float ang);
	void setScale(float x, float y, float z);
	const DirectX::XMFLOAT4X4& getTransform();
	const DirectX::XMFLOAT4X4& getTransformGlobal();

	void addChild(std::shared_ptr<Object> child);
	void removeChild(std::shared_ptr<Object> child);
	std::shared_ptr<Object>& removeChildAt(uint32_t index);
	void removeAllChildren();
	bool hasChild(std::shared_ptr<Object> child);
	std::shared_ptr<Object> getChildAt(uint32_t pos);
	uint32_t getNumChildren();
	Object* getParent();

protected:
	void update();
	void updateGlobal();
	void setDirty();

protected:
	DirectX::XMFLOAT4X4 pos{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
	DirectX::XMFLOAT4X4 rot{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
	DirectX::XMFLOAT4X4 scale{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
	DirectX::XMFLOAT4X4 transform{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };

	DirectX::XMFLOAT4X4 posGlobal{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
	DirectX::XMFLOAT4X4 rotGlobal{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
	DirectX::XMFLOAT4X4 scaleGlobal{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
	DirectX::XMFLOAT4X4 transformGlobal{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };

	DirectX::XMFLOAT4X4 posInverse{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
	DirectX::XMFLOAT4X4 rotInverse{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
	DirectX::XMFLOAT4X4 scaleInverse{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
	DirectX::XMFLOAT4X4 transformInverse{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };

	DirectX::XMFLOAT4X4 posInverseGlobal{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
	DirectX::XMFLOAT4X4 rotInverseGlobal{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
	DirectX::XMFLOAT4X4 scaleInverseGlobal{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
	DirectX::XMFLOAT4X4 transformGlobalInverse{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };

	std::shared_ptr<Mesh> mesh{ nullptr };
	std::vector<UINT> offsets;

	bool dirty{ false };

	std::vector<std::shared_ptr<Object>> children;
	Object* parent{ nullptr };
};