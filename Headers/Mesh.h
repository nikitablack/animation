#pragma once

#include "BufferFactory.h"

class Mesh
{
public:
	Mesh(BufferDataCollection bufferDataCollection);

	ID3D11Buffer* getIndexBuffer();
	const std::vector<uint32_t>& getIndexData();
	const std::vector<ID3D11Buffer*>& getVertexBuffers();
	const std::vector<ID3D11Buffer*>& getVertexShaderResources();
	const std::vector<ID3D11Buffer*>& getGeometryShaderResources();
	const std::vector<ID3D11Buffer*>& getPixelShaderResources();

	/*template<typename T>
	const std::vector<T>& getVertexBufferData(uint32_t pos)
	{
		return static_cast<BufferData<T>>(vertexBufferDatas[pos]).getData();
	}*/

	UINT getVertexBuffersNum();
	UINT getNumIndices();
	const std::vector<UINT>& getVertexStrides();

private:
	std::vector<ID3D11Buffer*> vertexBuffers;
	BufferDataCollection bufferDataCollection;
	std::vector<UINT> vertexStrides;
};