#include "Headers/Mesh.h"

using namespace std;
using namespace Microsoft::WRL;

Mesh::Mesh(BufferDataCollection bufferDataCollection)
{
	this->bufferDataCollection = bufferDataCollection; // todo

	for (const shared_ptr<BufferDataBase>& vertexBufferData : bufferDataCollection.vertexBufferDatas)
	{
		vertexBuffers.push_back(vertexBufferData->getBuffer());
		vertexStrides.push_back(vertexBufferData->getStride());
	}
}

const vector<ID3D11Buffer*>& Mesh::getVertexBuffers()
{
	return vertexBuffers;
}

ID3D11Buffer* Mesh::getIndexBuffer()
{
	return bufferDataCollection.indexBufferData->getBuffer();
}

const vector<uint32_t>& Mesh::getIndexData()
{
	return static_pointer_cast<BufferData<vector<uint32_t>>>(bufferDataCollection.indexBufferData)->getData();
}

UINT Mesh::getVertexBuffersNum()
{
	return static_cast<UINT>(vertexBuffers.size());
}

UINT Mesh::getNumIndices()
{
	return static_cast<UINT>(getIndexData().size());
}

const std::vector<UINT>& Mesh::getVertexStrides()
{
	return vertexStrides;
}