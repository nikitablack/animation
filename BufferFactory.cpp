#include "Headers/BufferFactory.h"

using namespace std;
using namespace Microsoft::WRL;

BufferFactory::BufferFactory(ComPtr<ID3D11Device> device) : device{ device }
{
}

shared_ptr<BufferDataBase> BufferFactory::createIndexBuffer(UINT numElements)
{
	auto buffer = createBuffer<uint32_t>(numElements, nullptr, D3D11_USAGE_DYNAMIC, D3D11_BIND_INDEX_BUFFER, D3D11_CPU_ACCESS_WRITE, 0);
	return make_shared<BufferData<std::vector<uint32_t>>>(buffer, static_cast<UINT>(sizeof(uint32_t)));
}

shared_ptr<BufferDataBase> BufferFactory::createIndexBuffer(const vector<uint32_t>& data)
{
	auto buffer = createBuffer<uint32_t>(static_cast<UINT>(data.size()), data.data(), D3D11_USAGE_IMMUTABLE, D3D11_BIND_INDEX_BUFFER, 0, 0);
	return make_shared<BufferData<std::vector<uint32_t>>>(buffer, static_cast<UINT>(sizeof(uint32_t)), data);
}