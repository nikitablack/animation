#pragma once

#include "Object.h"
#include <DirectXMath.h>

class Camera : public Object
{
public:
	Camera(float ratio = 1.3f, float fov = XMConvertToRadians(60), float n = 1.0f, float f = 100.0f)
	{
		updateProjection(ratio, fov, n, f);
	}

	void updateProjection(float ratio = 1.3f, float fov = XMConvertToRadians(60), float n = 1.0f, float f = 100.0f)
	{
		XMStoreFloat4x4(&projectionMatrix, XMMatrixPerspectiveFovLH(XMConvertToRadians(45), 800.0f / 600.0f, 1.0f, 100.0f));
	}

	const XMFLOAT4X4& getProjectionMatrix()
	{
		return projectionMatrix;
	}

	const XMFLOAT4X4& getViewProjectionMatrix()
	{
		updateGlobalCamera();
		return viewProjectionMatrix;
	}

private:
	void updateGlobalCamera()
	{
		Object::updateGlobal();

		viewProjectionMatrix = transformGlobalInverse;

		XMMATRIX viewProjectionMatrixDX{ XMLoadFloat4x4(&viewProjectionMatrix) };
		XMMATRIX projectionMatrixDX{ XMLoadFloat4x4(&projectionMatrix) };
		XMStoreFloat4x4(&viewProjectionMatrix, viewProjectionMatrixDX * projectionMatrixDX);
	}

private:
	XMFLOAT4X4 projectionMatrix;
	XMFLOAT4X4 viewProjectionMatrix;
};