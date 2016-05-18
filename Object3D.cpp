#include "Headers\Object3D.h"
#include <unordered_map>

using namespace std;
using namespace Microsoft::WRL;
using namespace DirectX;

namespace
{
	static const std::unordered_map<std::string, DXGI_FORMAT> dxgiFormatByType{
		{ typeid(DirectX::XMFLOAT3).name(), DXGI_FORMAT_R32G32B32_FLOAT },
		{ typeid(DirectX::XMINT3).name(), DXGI_FORMAT_R32G32B32A32_SINT },
		{ typeid(uint32_t).name(), DXGI_FORMAT_R32_UINT }
	};
}

Object3D::Object3D(ComPtr<ID3D11Device> device, shared_ptr<MeshBase> mesh, shared_ptr<ShaderData<ID3D11VertexShader>> vertexShaderData, shared_ptr<ShaderData<ID3D11PixelShader>> pixelShaderData, shared_ptr<ShaderData<ID3D11GeometryShader>> geometryShaderData, bool wireframe, bool backCulling, bool counterClockwise) : mesh{ mesh }, vertexShaderData{ vertexShaderData }, pixelShaderData{ pixelShaderData }, geometryShaderData{ geometryShaderData }
{
	const vector<string>& semantics = mesh->getSemantics();
	const vector<string>& typeids = mesh->getTypeids();

	vector<D3D11_INPUT_ELEMENT_DESC> descs;

	for (UINT i{ 0 }; i < typeids.size(); ++i)
	{
		auto dxgiFormatIt = dxgiFormatByType.find(typeids[i]);
		if (dxgiFormatIt == dxgiFormatByType.end())
		{
			throw(runtime_error{ "DXGI_FORMAT is not registered" });
		}

		DXGI_FORMAT dxgiFormat{ dxgiFormatIt->second };
		descs.push_back({ semantics[i].data(), 0, dxgiFormat, i, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 });
	}

	if (FAILED(device->CreateInputLayout(descs.data(), static_cast<UINT>(descs.size()), vertexShaderData->getBlob()->GetBufferPointer(), vertexShaderData->getBlob()->GetBufferSize(), inputLayout.ReleaseAndGetAddressOf())))
	{
		throw(runtime_error{ "Error creating input layout." });
	}

	D3D11_RASTERIZER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.FillMode = wireframe ? D3D11_FILL_WIREFRAME : D3D11_FILL_SOLID;
	desc.CullMode = backCulling ? D3D11_CULL_BACK : D3D11_CULL_NONE;
	desc.FrontCounterClockwise = counterClockwise ? TRUE : FALSE;
	desc.DepthBias = 0;
	desc.DepthBiasClamp = 0.0f;
	desc.SlopeScaledDepthBias = 0.0f;
	desc.DepthClipEnable = FALSE;
	desc.ScissorEnable = FALSE;
	desc.MultisampleEnable = FALSE;
	desc.AntialiasedLineEnable = FALSE;

	if (FAILED(device->CreateRasterizerState(&desc, rasterizerState.ReleaseAndGetAddressOf())))
	{
		throw(runtime_error{ "Error creating rasterizer state." });
	}
}

const vector<ID3D11Buffer*>& Object3D::getVertexBuffers()
{
	return mesh->getVertexBuffers();
}

ID3D11Buffer* Object3D::getIndexBuffer()
{
	return mesh->getIndexBuffer();
}

UINT Object3D::getNumIndices()
{
	return mesh->getNumIndices();
}

ID3D11VertexShader* Object3D::getVertexShader()
{
	return vertexShaderData->getShader();
}

ID3D11PixelShader* Object3D::getPixelShader()
{
	return pixelShaderData->getShader();
}

ID3D11GeometryShader* Object3D::getGeometryShader()
{
	return geometryShaderData->getShader();
}

ID3D11InputLayout* Object3D::getInputLayout()
{
	return inputLayout.Get();
}

ID3D11RasterizerState* Object3D::getRasterizerState()
{
	return rasterizerState.Get();
}

const vector<UINT>& Object3D::getVertexStrides()
{
	return mesh->getStrides();
}

const vector<UINT>& Object3D::getVertexOffsets()
{
	static vector<UINT> v(mesh->getVertexBuffersNum(), 0);
	return v;
}

void Object3D::setPosition(float x, float y, float z)
{
	pos._41 = x;
	pos._42 = y;
	pos._43 = z;

	posInverse._41 = -x;
	posInverse._42 = -y;
	posInverse._43 = -z;

	setDirty();
}

void Object3D::setPosition(const DirectX::XMFLOAT3& position)
{
	setPosition(position.x, position.y, position.z);
}

DirectX::XMFLOAT3 Object3D::getPosition()
{
	return{ pos._41, pos._42, pos._43 };
}

void Object3D::setRotation(float x, float y, float z)
{
	XMStoreFloat4x4(&rot, XMMatrixRotationRollPitchYaw(x, y, z));
	XMStoreFloat4x4(&rotInverse, XMMatrixRotationRollPitchYaw(-x, -y, -z));

	setDirty();
}

void Object3D::setScale(float x, float y, float z)
{
	scale._11 = x;
	scale._22 = y;
	scale._33 = z;

	scaleInverse._11 = x != 0 ? 1 / x : 0;
	scaleInverse._22 = y != 0 ? 1 / y : 0;
	scaleInverse._33 = z != 0 ? 1 / z : 0;

	setDirty();
}

XMFLOAT4X4 Object3D::getTransform()
{
	if (dirty)
	{
		update();
	}

	return transform;
}

XMFLOAT4X4 Object3D::getTransformGlobal()
{
	if (dirty)
	{
		updateGlobal();
	}

	return transformGlobal;
}

void Object3D::addChild(shared_ptr<Object3D> child)
{
	children.push_back(child);
	child->parent = this;

	child->setDirty();
}

void Object3D::removeChild(shared_ptr<Object3D> child)
{
	
}

void Object3D::removeAllChildren()
{
	//children.clear();
}

bool Object3D::hasChild(shared_ptr<Object3D> child)
{
	return false;
}

uint32_t Object3D::getChildrenNum()
{
	return static_cast<uint32_t>(children.size());
}

Object3D* Object3D::getParent()
{
	return parent;
}

void Object3D::update()
{
	XMMATRIX posDX{ XMLoadFloat4x4(&pos) };
	XMMATRIX rotDX{ XMLoadFloat4x4(&rot) };
	XMMATRIX scaleDX{ XMLoadFloat4x4(&scale) };
	
	XMStoreFloat4x4(&transform, rotDX * scaleDX * posDX);

	XMMATRIX posInverseDX{ XMLoadFloat4x4(&posInverse) };
	XMMATRIX rotInverseDX{ XMLoadFloat4x4(&rot) };
	XMMATRIX scaInverseleDX{ XMLoadFloat4x4(&scale) };

	XMStoreFloat4x4(&transformInverse, posInverseDX * scaInverseleDX * rotInverseDX);

	dirty = false;
}

void Object3D::updateGlobal()
{
	transformGlobal = getTransform();

	if (parent != nullptr)
	{
		XMFLOAT4X4 matParent{ parent->getTransformGlobal() };

		XMMATRIX matDX{ XMLoadFloat4x4(&transformGlobal) };
		XMMATRIX matParentDX{ XMLoadFloat4x4(&matParent) };

		XMStoreFloat4x4(&transformGlobal, matDX * matParentDX);
	}
}

void Object3D::setDirty()
{
	if (dirty) return;
	
	dirty = true;
	for(auto& child : children)
	{
		child->setDirty();
	}
}