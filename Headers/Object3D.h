#pragma once

#include <DirectXMath.h>
#include "ShaderData.h"
#include "VertexBufferData.h"
#include "Mesh.h"

class Object3D
{
public:
	Object3D() {};
	Object3D(Microsoft::WRL::ComPtr<ID3D11Device> device, std::shared_ptr<MeshBase> mesh, std::shared_ptr<ShaderData<ID3D11VertexShader>> vertexShaderData, std::shared_ptr<ShaderData<ID3D11PixelShader>> pixelShaderData, bool wireframe = false, bool backCulling = true);

	const std::vector<ID3D11Buffer*>& getVertexBuffers();
	ID3D11Buffer* getIndexBuffer();
	UINT getNumIndices();

	ID3D11VertexShader* getVertexShader();
	ID3D11PixelShader* getPixelShader();
	ID3D11InputLayout* getInputLayout();
	ID3D11RasterizerState* getRasterizerState();
	const std::vector<UINT>& getVertexStrides();
	const std::vector<UINT>& getVertexOffsets();

	void setPosition(float x, float y, float z);
	void setRotation(float x, float y, float z);
	void setScale(float x, float y, float z);
	DirectX::XMFLOAT3X3 getTransform();
	DirectX::XMFLOAT3X3 getTransformGlobal();

	void addChild(std::shared_ptr<Object3D> child);
	void removeChild(std::shared_ptr<Object3D> child);
	void removeAllChildren();
	bool hasChild(std::shared_ptr<Object3D> child);
	uint32_t getChildrenNum();
	Object3D* getParent();

private:
	DirectX::XMFLOAT3X3 pos{ 1, 0, 0, 0, 1, 0, 0, 0, 1 };
	DirectX::XMFLOAT3X3 rot{ 1, 0, 0, 0, 1, 0, 0, 0, 1 };
	DirectX::XMFLOAT3X3 scale{ 1, 0, 0, 0, 1, 0, 0, 0, 1 };
	DirectX::XMFLOAT3X3 transform{ 1, 0, 0, 0, 1, 0, 0, 0, 1 };
	DirectX::XMFLOAT3X3 transformGlobal{ 1, 0, 0, 0, 1, 0, 0, 0, 1 };

	DirectX::XMFLOAT3X3 posInverse{ 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	DirectX::XMFLOAT3X3 rotInverse{ 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	DirectX::XMFLOAT3X3 scaleInverse{ 1, 0, 0, 0, 1, 0, 0, 0, 1 };
	DirectX::XMFLOAT3X3 transformInverse{ 1, 0, 0, 0, 1, 0, 0, 0, 1 };

	std::shared_ptr<MeshBase> mesh;
	std::shared_ptr<ShaderData<ID3D11VertexShader>> vertexShaderData{ nullptr };
	std::shared_ptr<ShaderData<ID3D11PixelShader>> pixelShaderData{ nullptr };
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState;

	bool dirty{ false };

	std::vector<std::shared_ptr<Object3D>> children;
	Object3D* parent{ nullptr };

private:
	void update();
	void updateGlobal();
	void setDirty();
};