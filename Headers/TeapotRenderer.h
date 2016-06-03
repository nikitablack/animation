#pragma once

#include "BaseRenderer.h"
#include "ConstantBuffers.h"
#include "Camera.h"

class TeapotRenderer : public BaseRenderer
{
public:
	TeapotRenderer(std::shared_ptr<Graphics> graphics, std::shared_ptr<ShaderFactory> shaderFactory) : BaseRenderer{ graphics }
	{
		D3D11_RASTERIZER_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.FillMode = D3D11_FILL_WIREFRAME;
		desc.CullMode = D3D11_CULL_NONE;
		desc.FrontCounterClockwise = FALSE;
		desc.DepthBias = 0;
		desc.DepthBiasClamp = 0.0f;
		desc.SlopeScaledDepthBias = 0.0f;
		desc.DepthClipEnable = FALSE;
		desc.ScissorEnable = FALSE;
		desc.MultisampleEnable = FALSE;
		desc.AntialiasedLineEnable = FALSE;

		if (FAILED(graphics->getDevice()->CreateRasterizerState(&desc, rasterizerState.ReleaseAndGetAddressOf())))
		{
			throw(runtime_error{ "Error creating rasterizer state." });
		}

		shaderData = make_shared<ShaderDataCollection>();
		shaderData->vertexShaderData = shaderFactory->createVertexShader(L"TeapotVertexShader.cso");
		shaderData->pixelShaderData = shaderFactory->createPixelShader(L"TeapotPixelShader.cso");
		shaderData->hullShaderData = shaderFactory->createHullShader(L"TeapotHullShader.cso");
		shaderData->domainShaderData = shaderFactory->createDomainShader(L"TeapotDomainShader.cso");
	}

	void render(const std::vector<std::shared_ptr<Object>>& teapot, ID3D11Buffer* constantBuffers[4], std::shared_ptr<Camera> camera)
	{
		ID3D11DeviceContext* context{ graphics->getContext() };

		context->IASetInputLayout(shaderData->vertexShaderData->getInputLayout());
		context->VSSetShader(shaderData->vertexShaderData->getShader(), nullptr, 0);
		context->PSSetShader(shaderData->pixelShaderData->getShader(), nullptr, 0);
		context->HSSetShader(shaderData->hullShaderData->getShader(), nullptr, 0);
		context->DSSetShader(shaderData->domainShaderData->getShader(), nullptr, 0);
		context->RSSetState(rasterizerState.Get());

		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST);

		static float rotZ = 0.0f;
		rotZ += 0.02f;

		drawBodyPart(context, teapot[0], constantBuffers, camera, XMFLOAT3{ 0.0f, 0.0f, rotZ });
		drawBodyPart(context, teapot[1], constantBuffers, camera, XMFLOAT3{ 0.0f, 0.0f, rotZ });
		drawBodyPart(context, teapot[2], constantBuffers, camera, XMFLOAT3{ 0.0f, 0.0f, rotZ });
		drawBodyPart(context, teapot[3], constantBuffers, camera, XMFLOAT3{ 0.0f, 0.0f, rotZ });
		drawBodyPart(context, teapot[4], constantBuffers, camera, XMFLOAT3{ 0.0f, 0.0f, rotZ });
		drawSpoutOrHandle(context, teapot[5], constantBuffers, camera, XMFLOAT3{ 0.0f, 0.0f, rotZ });
		drawSpoutOrHandle(context, teapot[6], constantBuffers, camera, XMFLOAT3{ 0.0f, 0.0f, rotZ });
		drawSpoutOrHandle(context, teapot[7], constantBuffers, camera, XMFLOAT3{ 0.0f, 0.0f, rotZ });
		drawSpoutOrHandle(context, teapot[8], constantBuffers, camera, XMFLOAT3{ 0.0f, 0.0f, rotZ });
	}

private:
	void drawBodyPart(ID3D11DeviceContext* context, std::shared_ptr<Object> obj, ID3D11Buffer* constantBuffers[4], std::shared_ptr<Camera> camera, XMFLOAT3 rotation)
	{
		const vector<ID3D11Buffer*>& vertexBuffers{ obj->getVertexBuffers() };

		context->IASetVertexBuffers(0, static_cast<UINT>(vertexBuffers.size()), vertexBuffers.data(), obj->getVertexStrides().data(), obj->getVertexOffsets().data());
		context->IASetIndexBuffer(obj->getIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);

		drawObjectWithScaleAndRotation(context, obj, constantBuffers, camera, XMFLOAT3{ rotation.x, rotation.y, 0.0f + rotation.z });
		drawObjectWithScaleAndRotation(context, obj, constantBuffers, camera, XMFLOAT3{ rotation.x, rotation.y, XM_PIDIV2 + rotation.z });
		drawObjectWithScaleAndRotation(context, obj, constantBuffers, camera, XMFLOAT3{ rotation.x, rotation.y, XM_PI + rotation.z });
		drawObjectWithScaleAndRotation(context, obj, constantBuffers, camera, XMFLOAT3{ rotation.x, rotation.y, -XM_PIDIV2 + rotation.z });
	}

	void drawSpoutOrHandle(ID3D11DeviceContext* context, std::shared_ptr<Object> obj, ID3D11Buffer* constantBuffers[4], std::shared_ptr<Camera> camera, XMFLOAT3 rotation)
	{
		const vector<ID3D11Buffer*>& vertexBuffers{ obj->getVertexBuffers() };

		context->IASetVertexBuffers(0, static_cast<UINT>(vertexBuffers.size()), vertexBuffers.data(), obj->getVertexStrides().data(), obj->getVertexOffsets().data());
		context->IASetIndexBuffer(obj->getIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);

		drawObjectWithScaleAndRotation(context, obj, constantBuffers, camera, XMFLOAT3{ rotation.x, rotation.y, rotation.z });
		drawObjectWithScaleAndRotation(context, obj, constantBuffers, camera, XMFLOAT3{ rotation.x, rotation.y, rotation.z }, XMFLOAT3{ 1.0f, -1.0f, 1.0f });
	}

	void drawObjectWithScaleAndRotation(ID3D11DeviceContext* context, std::shared_ptr<Object> obj, ID3D11Buffer* constantBuffers[4], std::shared_ptr<Camera> camera, XMFLOAT3 rotation, XMFLOAT3 scale = XMFLOAT3{ 1.0f, 1.0f, 1.0f })
	{
		obj->setScale(scale.x, scale.y, scale.z);
		obj->setRotation(rotation.x, rotation.y, rotation.z);

		XMMATRIX matrixObjDX(XMLoadFloat4x4(&obj->getTransformGlobal()));
		XMMATRIX viewProjectionDX(XMLoadFloat4x4(&camera->getViewProjectionMatrix()));

		ConstantBufferPerObject cbPerObject;
		XMStoreFloat4x4(&cbPerObject.wMat, matrixObjDX);
		XMStoreFloat4x4(&cbPerObject.wvpMat, matrixObjDX * viewProjectionDX);

		updateResource(constantBuffers[3], cbPerObject);

		context->DrawIndexed(obj->getNumIndices(), 0, 0);
	}

private:
	shared_ptr<ShaderDataCollection> shaderData;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState;
};