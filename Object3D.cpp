#include "Object3D.h"
#include <unordered_map>

using namespace std;
using namespace Microsoft::WRL;
using namespace DirectX;

namespace
{
	static const std::unordered_map<std::string, DXGI_FORMAT> dxgiFormatByType{
		{ typeid(DirectX::XMFLOAT3).name(), DXGI_FORMAT_R32G32B32_FLOAT },
		{ typeid(uint32_t).name(), DXGI_FORMAT_R32_UINT }
	};
}

Object3D::Object3D(ComPtr<ID3D11Device> device, shared_ptr<MeshBase> mesh, shared_ptr<ShaderData<ID3D11VertexShader>> vertexShaderData, shared_ptr<ShaderData<ID3D11PixelShader>> pixelShaderData, bool wireframe, bool backCulling) : mesh{ mesh }, vertexShaderData{ vertexShaderData }, pixelShaderData{ pixelShaderData }
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
	desc.FrontCounterClockwise = FALSE;
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
	pos._31 = x;
	pos._32 = y;
	pos._33 = z;

	posInverse._31 = -x;
	posInverse._32 = -y;
	posInverse._33 = -z;

	updated = true;
}

void Object3D::setRotation(float x, float y, float z)
{
	XMStoreFloat3x3(&rot, XMMatrixRotationRollPitchYaw(x, y, z));
	XMStoreFloat3x3(&rotInverse, XMMatrixRotationRollPitchYaw(-x, -y, -z));

	updated = true;
}

void Object3D::setScale(float x, float y, float z)
{
	scale._11 = x;
	scale._22 = y;
	scale._33 = z;

	scaleInverse._11 = x != 0 ? 1 / x : 0;
	scaleInverse._22 = y != 0 ? 1 / y : 0;
	scaleInverse._33 = z != 0 ? 1 / z : 0;

	updated = true;
}

XMFLOAT3X3 Object3D::getTransform()
{
	if (updated)
	{
		update();
	}

	return transform;
}
void Object3D::update()
{
	XMMATRIX posDX{ XMLoadFloat3x3(&pos) };
	XMMATRIX rotDX{ XMLoadFloat3x3(&rot) };
	XMMATRIX scaleDX{ XMLoadFloat3x3(&scale) };
	
	XMStoreFloat3x3(&transform, rotDX * scaleDX * posDX);

	XMMATRIX posInverseDX{ XMLoadFloat3x3(&posInverse) };
	XMMATRIX rotInverseDX{ XMLoadFloat3x3(&rot) };
	XMMATRIX scaInverseleDX{ XMLoadFloat3x3(&scale) };

	XMStoreFloat3x3(&transformInverse, posInverseDX * scaInverseleDX * rotInverseDX);

	updated = false;
}