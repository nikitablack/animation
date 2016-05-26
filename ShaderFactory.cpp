#include "Headers/ShaderFactory.h"

using namespace std;
using namespace Microsoft::WRL;

VertexShaderData::VertexShaderData(ComPtr<ID3D11VertexShader> shader, ComPtr<ID3DBlob> blob, ComPtr<ID3D11InputLayout> inputLayout) : ShaderData<ID3D11VertexShader>(shader, blob), inputLayout{ inputLayout }
{

}

ID3D11InputLayout* VertexShaderData::getInputLayout()
{
	return inputLayout.Get();
}

ShaderFactory::ShaderFactory(ComPtr<ID3D11Device> device) : device{ device }
{

}

shared_ptr<VertexShaderData> ShaderFactory::createVertexShader(const wstring& path)
{
	ComPtr<ID3DBlob> blob{ getBlob(path) };

	ComPtr<ID3D11VertexShader> shader;
	if (FAILED(device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, shader.ReleaseAndGetAddressOf())))
	{
		throw(runtime_error{ "Error creating vertex shader." });
	}

	ComPtr<ID3D11ShaderReflection> vertexShaderReflection;
	if (FAILED(D3DReflect(blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(vertexShaderReflection.ReleaseAndGetAddressOf()))))
	{
		throw(runtime_error{ "Error reflecting vertex shader." });
	}

	D3D11_SHADER_DESC shaderDesc;
	vertexShaderReflection->GetDesc(&shaderDesc);

	vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDescs;
	for (UINT i{ 0 }; i < shaderDesc.InputParameters; i++)
	{
		D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
		vertexShaderReflection->GetInputParameterDesc(i, &paramDesc);

		// fill out input element desc
		D3D11_INPUT_ELEMENT_DESC elementDesc;
		elementDesc.SemanticName = paramDesc.SemanticName;
		elementDesc.SemanticIndex = paramDesc.SemanticIndex;
		elementDesc.InputSlot = i;
		elementDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		elementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		elementDesc.InstanceDataStepRate = 0;

		// determine DXGI format
		if (paramDesc.Mask == 1)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32_FLOAT;
		}
		else if (paramDesc.Mask <= 3)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
		}
		else if (paramDesc.Mask <= 7)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
		}
		else if (paramDesc.Mask <= 15)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		}

		//save element desc
		inputLayoutDescs.push_back(elementDesc);
	}

	ComPtr<ID3D11InputLayout> inputLayout;
	if (FAILED(device->CreateInputLayout(inputLayoutDescs.data(), static_cast<UINT>(inputLayoutDescs.size()), blob->GetBufferPointer(), blob->GetBufferSize(), inputLayout.ReleaseAndGetAddressOf())))
	{
		throw(runtime_error{ "Error creating input layout." });
	}

	return make_shared<VertexShaderData>(shader, blob, inputLayout);
}

shared_ptr<ShaderData<ID3D11PixelShader>> ShaderFactory::createPixelShader(const wstring& path)
{
	ComPtr<ID3DBlob> blob{ getBlob(path) };

	ComPtr<ID3D11PixelShader> shader;
	if (FAILED(device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, shader.ReleaseAndGetAddressOf())))
	{
		throw(runtime_error{ "Error creating pixel shader." });
	}

	return make_shared<ShaderData<ID3D11PixelShader>>(shader, blob);
}

shared_ptr<ShaderData<ID3D11HullShader>> ShaderFactory::createHullShader(const wstring& path)
{
	ComPtr<ID3DBlob> blob{ getBlob(path) };

	ComPtr<ID3D11HullShader> shader;
	if (FAILED(device->CreateHullShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, shader.ReleaseAndGetAddressOf())))
	{
		throw(runtime_error{ "Error creating hull shader." });
	}

	return make_shared<ShaderData<ID3D11HullShader>>(shader, blob);
}

shared_ptr<ShaderData<ID3D11DomainShader>> ShaderFactory::createDomainShader(const wstring& path)
{
	ComPtr<ID3DBlob> blob{ getBlob(path) };

	ComPtr<ID3D11DomainShader> shader;
	if (FAILED(device->CreateDomainShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, shader.ReleaseAndGetAddressOf())))
	{
		throw(runtime_error{ "Error creating domain shader." });
	}

	return make_shared<ShaderData<ID3D11DomainShader>>(shader, blob);
}

Microsoft::WRL::ComPtr<ID3DBlob> ShaderFactory::getBlob(const wstring& path)
{
	ComPtr<ID3DBlob> blob;
	if (FAILED(D3DReadFileToBlob(path.data(), blob.ReleaseAndGetAddressOf())))
	{
		throw(runtime_error{ "Error reading the shader." });
	}

	return blob;
}