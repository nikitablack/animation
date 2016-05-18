#pragma once

#include <DirectXMath.h>
#include "ShaderData.h"
#include "VertexBufferData.h"
#include "Mesh.h"

class Object3D
{
public:
	Object3D() {};
	Object3D(Microsoft::WRL::ComPtr<ID3D11Device> device, std::shared_ptr<MeshBase> mesh, std::shared_ptr<ShaderData<ID3D11VertexShader>> vertexShaderData, std::shared_ptr<ShaderData<ID3D11PixelShader>> pixelShaderData, std::shared_ptr<ShaderData<ID3D11GeometryShader>> geometryShaderData, bool wireframe = false, bool backCulling = true, bool counterClockwise = true);

	const std::vector<ID3D11Buffer*>& getVertexBuffers();
	ID3D11Buffer* getIndexBuffer();
	UINT getNumIndices();

	ID3D11VertexShader* getVertexShader();
	ID3D11PixelShader* getPixelShader();
	ID3D11GeometryShader* getGeometryShader();
	ID3D11InputLayout* getInputLayout();
	ID3D11RasterizerState* getRasterizerState();
	const std::vector<UINT>& getVertexStrides();
	const std::vector<UINT>& getVertexOffsets();

	void setPosition(float x, float y, float z);
	void setPosition(const DirectX::XMFLOAT3& position);
	DirectX::XMFLOAT3 getPosition();
	void setRotation(float x, float y, float z);
	void setScale(float x, float y, float z);
	DirectX::XMFLOAT4X4 getTransform();
	DirectX::XMFLOAT4X4 getTransformGlobal();

	void addChild(std::shared_ptr<Object3D> child);
	void removeChild(std::shared_ptr<Object3D> child);
	void removeAllChildren();
	bool hasChild(std::shared_ptr<Object3D> child);
	uint32_t getChildrenNum();
	Object3D* getParent();

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

	std::shared_ptr<MeshBase> mesh;
	std::shared_ptr<ShaderData<ID3D11VertexShader>> vertexShaderData{ nullptr };
	std::shared_ptr<ShaderData<ID3D11PixelShader>> pixelShaderData{ nullptr };
	std::shared_ptr<ShaderData<ID3D11GeometryShader>> geometryShaderData{ nullptr };
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