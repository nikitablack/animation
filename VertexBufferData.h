#pragma once

#include <wrl/client.h>
#include <string>
#include <stdexcept>
#include <d3d11.h>
#include <vector>
#include <tuple>

template<typename... Args>
class VertexBufferData
{
public:
	VertexBufferData(Microsoft::WRL::ComPtr<ID3D11Device> device, const std::tuple<std::pair<std::vector<Args>, std::string>...>& buffersAndSemantics) : device{ device }, buffersAndSemantics{ buffersAndSemantics }, vertexBuffers(sizeof...(Args)), vertexBuffersRaw(sizeof...(Args)), semantics(sizeof...(Args)), typeids(sizeof...(Args)), strides(sizeof...(Args))
	{
		createBuffers(buffersAndSemantics);
	}

	template<int N, typename T>
	void setVertexData(const std::vector<T>& data, const std::string& name)
	{
		semantics[N] = name;
		typeids[N] = typeid(T).name();
		strides[N] = sizeof(T);

		D3D11_BUFFER_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = static_cast<UINT>(data.size() * sizeof(T));
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		desc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA initialData;
		initialData.pSysMem = data.data();
		initialData.SysMemPitch = 0;
		initialData.SysMemSlicePitch = 0;

		Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;
		if (FAILED(device->CreateBuffer(&desc, &initialData, buffer.ReleaseAndGetAddressOf())))
		{
			throw(runtime_error{ "Error creating vertex buffer." });
		}

		vertexBuffers[N] = buffer;
		vertexBuffersRaw[N] = buffer.Get();
	}

	template<int N, typename T>
	const std::vector<T>& getVertexData()
	{
		return std::get<N>(vertexDatas);
	}

	const std::vector<ID3D11Buffer*>& getVertexBuffers()
	{
		return vertexBuffersRaw;
	}

	const std::vector<std::string>& getSemantics()
	{
		return semantics;
	}

	const std::vector<std::string>& getTypeids()
	{
		return typeids;
	}

	const std::vector<UINT>& getStrides()
	{
		return strides;
	}

private:
	std::tuple<std::pair<std::vector<Args>, std::string>...> buffersAndSemantics;
	Microsoft::WRL::ComPtr<ID3D11Device> device;
	std::vector<Microsoft::WRL::ComPtr<ID3D11Buffer>> vertexBuffers;
	std::vector<ID3D11Buffer*> vertexBuffersRaw;
	std::vector<std::string> semantics;
	std::vector<std::string> typeids;
	std::vector<UINT> strides;

private:
	template<size_t I = 0, typename... Args>
	typename std::enable_if<I == sizeof...(Args), void>::type
		createBuffers(const std::tuple<std::pair<std::vector<Args>, std::string>...>& t)
	{ }

	template<size_t I = 0, typename... Args>
	typename std::enable_if < (I < sizeof...(Args)), void>::type
		createBuffers(const std::tuple<std::pair<std::vector<Args>, std::string>...>& t)
	{
		auto p = get<I>(t);
		auto v = p.first;
		string s = p.second;

		setVertexData<I>(v, s);

		createBuffers<I + 1, Args...>(t);
	}
};