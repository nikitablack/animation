#pragma once

#include <wrl/client.h>
#include <stdexcept>
#include <d3d11.h>
#include <vector>
#include <memory>

class BufferDataBase
{
public:
	virtual ID3D11Buffer* getBuffer() = 0;
	virtual UINT getStride() = 0;
};

template<typename T>
class BufferData : public BufferDataBase
{
	friend class BufferFactory;

public:
	BufferData(Microsoft::WRL::ComPtr<ID3D11Buffer> buffer, UINT stride, const T& data) : buffer{ buffer }, stride{ stride }
	{
		this->data = data; // todo
	}

	BufferData(Microsoft::WRL::ComPtr<ID3D11Buffer> buffer, UINT stride) : buffer{ buffer }, stride{ stride }
	{

	}

	ID3D11Buffer* getBuffer() override
	{
		return buffer.Get();
	}

	UINT getStride() override
	{
		return stride;
	}

	const T& getData()
	{
		return data;
	}

private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;
	UINT stride;
	T data;
};

struct BufferDataCollection
{
	std::vector<std::shared_ptr<BufferDataBase>> vertexBufferDatas;
	std::shared_ptr<BufferDataBase> indexBufferData;
	std::vector<std::shared_ptr<BufferDataBase>> vertexShaderResources;
	std::vector<std::shared_ptr<BufferDataBase>> geometryShaderResources;
	std::vector<std::shared_ptr<BufferDataBase>> pixelShaderResources;
};

class BufferFactory
{
public:
	BufferFactory(Microsoft::WRL::ComPtr<ID3D11Device> device);

	// vertex
	template<typename T>
	std::shared_ptr<BufferDataBase> createVertexBuffer(UINT numElements)
	{
		auto buffer = createBuffer<T>(numElements, nullptr, D3D11_USAGE_DYNAMIC, D3D11_BIND_VERTEX_BUFFER, D3D11_CPU_ACCESS_WRITE, 0);
		return make_shared<BufferData<std::vector<T>>>(buffer, static_cast<UINT>(sizeof(T)));
	}

	template<typename T>
	std::shared_ptr<BufferDataBase> createVertexBuffer(const std::vector<T>& data)
	{
		auto buffer = createBuffer<T>(static_cast<UINT>(data.size()), data.data(), D3D11_USAGE_IMMUTABLE, D3D11_BIND_VERTEX_BUFFER, 0, 0);
		return make_shared<BufferData<std::vector<T>>>(buffer, static_cast<UINT>(sizeof(T)), data);
	}

	// index
	std::shared_ptr<BufferDataBase> createIndexBuffer(UINT numElements);
	std::shared_ptr<BufferDataBase> createIndexBuffer(const std::vector<uint32_t>& data);

	// constant
	template<typename T>
	std::shared_ptr<BufferDataBase> createConstantBuffer()
	{
		auto buffer = createBuffer<T>(1, nullptr, D3D11_USAGE_DYNAMIC, D3D11_BIND_CONSTANT_BUFFER, D3D11_CPU_ACCESS_WRITE, 0);
		return make_shared<BufferData<T>>(buffer, static_cast<UINT>(sizeof(T)));
	}

	template<typename T>
	std::shared_ptr<BufferDataBase> createConstantBuffer(const T& data)
	{
		auto buffer = createBuffer<T>(1, &data, D3D11_USAGE_IMMUTABLE, D3D11_BIND_CONSTANT_BUFFER, 0, 0);
		return make_shared<BufferData<T>>(buffer, static_cast<UINT>(sizeof(T)), data);
	}

	// structured
	template<typename T>
	std::shared_ptr<BufferDataBase> createStructuredBuffer(UINT numElements, bool gpuWrite = false)
	{
		UINT bindFlags{ D3D11_BIND_SHADER_RESOURCE };
		if (gpuWrite) bindFlags |= D3D11_BIND_UNORDERED_ACCESS;
		
		auto buffer = createBuffer<T>(numElements, nullptr, D3D11_USAGE_DYNAMIC, bindFlags, D3D11_CPU_ACCESS_WRITE, D3D11_RESOURCE_MISC_BUFFER_STRUCTURED);
		return make_shared<BufferData<std::vector<T>>>(buffer, static_cast<UINT>(sizeof(T)));
	}

	template<typename T>
	std::shared_ptr<BufferDataBase> createStructuredBuffer(const std::vector<T>& data, D3D11_USAGE usage, bool gpuWrite = false)
	{
		UINT bindFlags{ D3D11_BIND_SHADER_RESOURCE };
		if (gpuWrite) bindFlags |= D3D11_BIND_UNORDERED_ACCESS;

		auto buffer = createBuffer(static_cast<UINT>(data.size()), data.data, D3D11_USAGE_IMMUTABLE, bindFlags, 0, D3D11_RESOURCE_MISC_BUFFER_STRUCTURED);
		return make_shared<BufferData<std::vector<T>>>(buffer, static_cast<UINT>(sizeof(T)), data);
	}

private:

	template<typename T>
	Microsoft::WRL::ComPtr<ID3D11Buffer> createBuffer(UINT numElements, const void* data, D3D11_USAGE usage, D3D11_BIND_FLAG bindFlags, UINT CPUAccessFlags, UINT miscFlags)
	{
		D3D11_BUFFER_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.Usage = usage;
		desc.ByteWidth = static_cast<UINT>(numElements * sizeof(T));
		desc.BindFlags = bindFlags;
		desc.CPUAccessFlags = CPUAccessFlags;
		desc.MiscFlags = miscFlags;
		desc.StructureByteStride = static_cast<UINT>(sizeof(T));

		if (data != nullptr)
		{
			D3D11_SUBRESOURCE_DATA initialData;
			initialData.pSysMem = data;
			initialData.SysMemPitch = 0;
			initialData.SysMemSlicePitch = 0;

			return createBuffer(&desc, &initialData);
		}
		else
		{
			return createBuffer(&desc, nullptr);
		}
	}

	Microsoft::WRL::ComPtr<ID3D11Buffer> createBuffer(D3D11_BUFFER_DESC* desc, D3D11_SUBRESOURCE_DATA* initialData)
	{
		Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;
		if (FAILED(device->CreateBuffer(desc, initialData, buffer.ReleaseAndGetAddressOf())))
		{
			throw(std::runtime_error{ "Error creating a buffer." });
		}

		return buffer;
	}

private:
	Microsoft::WRL::ComPtr<ID3D11Device> device;
};