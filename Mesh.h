#pragma once

#include <memory>
#include "VertexBufferData.h"

class MeshBase
{
public:
	virtual const std::vector<ID3D11Buffer*>& getVertexBuffers() = 0;
	virtual UINT getVertexBuffersNum() = 0;
	virtual ID3D11Buffer* getIndexBuffer() = 0;
	virtual UINT getNumIndices() = 0;
	virtual const std::vector<std::string>& getSemantics() = 0;
	virtual const std::vector<std::string>& getTypeids() = 0;
	virtual const std::vector<UINT>& getStrides() = 0;
};

template<typename ...Args>
class Mesh : public MeshBase
{
public:
	template<int N>
	UINT getSize()
	{
		stride
			return 0;
	}

	Mesh(Microsoft::WRL::ComPtr<ID3D11Device> device, std::shared_ptr<VertexBufferData<Args...>> vertexBufferData, const std::vector<uint32_t>& indexData) : device{ device }, vertexBufferData{ vertexBufferData }
	{
		this->indexData = indexData; // todo

		using T = remove_reference<decltype(indexData)>::type::value_type;

		D3D11_BUFFER_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = static_cast<UINT>(indexData.size() * sizeof(T));
		desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		desc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA initialData;
		initialData.pSysMem = indexData.data();
		initialData.SysMemPitch = 0;
		initialData.SysMemSlicePitch = 0;

		ComPtr<ID3D11Buffer> buffer;
		if (FAILED(device->CreateBuffer(&desc, &initialData, buffer.ReleaseAndGetAddressOf())))
		{
			throw(runtime_error{ "Error creating index buffer." });
		}

		indexBuffer = buffer;
	}

	void setDevice(Microsoft::WRL::ComPtr<ID3D11Device> device)
	{
		this->device = device;
	}

	void setIndexData(const std::vector<uint32_t>& data)
	{
		indexData = data
	}

	const std::vector<uint32_t>& getIndexData()
	{
		return indexData;
	}

	const std::vector<ID3D11Buffer*>& getVertexBuffers() override
	{
		return vertexBufferData->getVertexBuffers();
	}

	UINT getVertexBuffersNum() override
	{
		return sizeof...(Args);
	}

	ID3D11Buffer* getIndexBuffer() override
	{
		return indexBuffer.Get();
	}

	UINT getNumIndices() override
	{
		return static_cast<UINT>(indexData.size());
	}

	const std::vector<std::string>& getSemantics() override
	{
		return vertexBufferData->getSemantics();
	}

	const std::vector<std::string>& getTypeids() override
	{
		return vertexBufferData->getTypeids();
	}

	const std::vector<UINT>& getStrides() override
	{
		return vertexBufferData->getStrides();
	}

private:
	Microsoft::WRL::ComPtr<ID3D11Device> device;
	std::shared_ptr<VertexBufferData<Args...>> vertexBufferData;
	std::vector<uint32_t> indexData;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
};