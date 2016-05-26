#pragma once

#include "Graphics.h"

class BaseRenderer
{
public:
	BaseRenderer(std::shared_ptr<Graphics> graphics) : graphics{ graphics }
	{

	}

protected:
	template<typename T>
	void updateResource(ID3D11Resource* resource, const T& data)
	{
		ID3D11DeviceContext* context{ graphics->getContext() };

		D3D11_MAPPED_SUBRESOURCE mappedResource;
		if (FAILED(context->Map(resource, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		{
			throw(runtime_error{ "Error mapping constant buffer" });
		}

		T* cbPerObjDataPtr{ static_cast<T*>(mappedResource.pData) };
		*cbPerObjDataPtr = data;

		context->Unmap(resource, 0);
	}

protected:
	shared_ptr<Graphics> graphics;
};