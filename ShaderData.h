#pragma once

#include <wrl/client.h>
#include <string>
#include <stdexcept>
#include <d3d11.h>
#include <d3dcompiler.h>

template<typename T>
class ShaderData
{
public:
	ShaderData(Microsoft::WRL::ComPtr<ID3D11Device> device, const std::wstring& shaderName)
	{
		if (FAILED(D3DReadFileToBlob(shaderName.data(), blob.ReleaseAndGetAddressOf())))
		{
			throw(std::runtime_error{ "Error reading the shader." });
		}

		createShader(device, blob);
	}

	ID3DBlob* getBlob()
	{
		return blob.Get();
	}

	T* getShader()
	{
		return shader.Get();
	}

private:
	Microsoft::WRL::ComPtr<ID3DBlob> blob;
	Microsoft::WRL::ComPtr<T> shader;

private:
	void createShader(Microsoft::WRL::ComPtr<ID3D11Device> device, Microsoft::WRL::ComPtr<ID3DBlob> blob);
};