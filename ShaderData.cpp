#include "Headers\ShaderData.h"

using namespace Microsoft::WRL;
using namespace std;

template<>
void ShaderData<ID3D11VertexShader>::createShader(ComPtr<ID3D11Device> device, ComPtr<ID3DBlob> blob)
{
	if (FAILED(device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, shader.ReleaseAndGetAddressOf())))
	{
		throw(runtime_error{ "Error creating vertex shader." });
	}
}

template<>
void ShaderData<ID3D11PixelShader>::createShader(ComPtr<ID3D11Device> device, ComPtr<ID3DBlob> blob)
{
	if (FAILED(device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, shader.ReleaseAndGetAddressOf())))
	{
		throw(runtime_error{ "Error creating pixel shader." });
	}
}

template<>
void ShaderData<ID3D11GeometryShader>::createShader(ComPtr<ID3D11Device> device, ComPtr<ID3DBlob> blob)
{
	if (FAILED(device->CreateGeometryShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, shader.ReleaseAndGetAddressOf())))
	{
		throw(runtime_error{ "Error creating geometry shader." });
	}
}