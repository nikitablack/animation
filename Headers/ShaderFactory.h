#pragma once

#include <wrl/client.h>
#include <stdexcept>
#include <d3d11.h>
#include <string>
#include <d3dcompiler.h>
#include <vector>
#include <memory>

template<typename T>
class ShaderData
{
public:
	ShaderData(Microsoft::WRL::ComPtr<T> shader, Microsoft::WRL::ComPtr<ID3DBlob> blob) : shader{ shader }, blob{ blob }
	{
	}

	virtual T* getShader()
	{
		return shader.Get();
	}

	virtual ID3DBlob* getBlob()
	{
		return blob.Get();
	}

private:
	Microsoft::WRL::ComPtr<T> shader;
	Microsoft::WRL::ComPtr<ID3DBlob> blob;
};

class VertexShaderData : public ShaderData<ID3D11VertexShader>
{
public:
	VertexShaderData(Microsoft::WRL::ComPtr<ID3D11VertexShader> shader, Microsoft::WRL::ComPtr<ID3DBlob> blob, Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout);

	ID3D11InputLayout* getInputLayout();

private:
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
};

struct ShaderDataCollection
{
	std::shared_ptr<VertexShaderData> vertexShaderData;
	std::shared_ptr<ShaderData<ID3D11PixelShader>> pixelShaderData;
};

class ShaderFactory
{
public:
	ShaderFactory(Microsoft::WRL::ComPtr<ID3D11Device> device);

	std::shared_ptr<VertexShaderData> createVertexShader(const std::wstring& path);
	std::shared_ptr<ShaderData<ID3D11PixelShader>> createPixelShader(const std::wstring& path);

private:
	Microsoft::WRL::ComPtr<ID3DBlob> getBlob(const std::wstring& path);

private:
	Microsoft::WRL::ComPtr<ID3D11Device> device;
};