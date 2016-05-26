#pragma once

#include "BaseRenderer.h"
#include "ConstantBuffers.h"

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

	void render(std::shared_ptr<Object> obj, ID3D11Buffer* constantBuffers[4])
	{
		ID3D11DeviceContext* context{ graphics->getContext() };

		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST);

		const vector<ID3D11Buffer*>& vertexBuffers{ obj->getVertexBuffers() };

		context->IASetVertexBuffers(0, static_cast<UINT>(vertexBuffers.size()), vertexBuffers.data(), obj->getVertexStrides().data(), obj->getVertexOffsets().data());
		context->IASetIndexBuffer(obj->getIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);

		context->IASetInputLayout(shaderData->vertexShaderData->getInputLayout());
		context->VSSetShader(shaderData->vertexShaderData->getShader(), nullptr, 0);
		context->PSSetShader(shaderData->pixelShaderData->getShader(), nullptr, 0);
		context->HSSetShader(shaderData->hullShaderData->getShader(), nullptr, 0);
		context->DSSetShader(shaderData->domainShaderData->getShader(), nullptr, 0);
		context->RSSetState(rasterizerState.Get());

		XMVECTOR vecCamPosition(XMVectorSet(0.0f, -10.0f, 5.0f, 0.0f));
		XMVECTOR vecCamLookAt(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f));
		XMVECTOR vecCamUp(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
		XMMATRIX matrixView(XMMatrixLookAtLH(vecCamPosition, vecCamLookAt, vecCamUp));
		XMMATRIX matrixProjection(XMMatrixPerspectiveFovLH(XMConvertToRadians(45), 800.0f / 600.0f, 1.0f, 100.0f));

		/*static float rot{ 0.0f };
		obj->setRotation(0, rot, 0);
		rot += 0.01f;*/
		XMMATRIX matrixObj(XMLoadFloat4x4(&obj->getTransformGlobal()));

		ConstantBufferPerObject cbPerObject;
		XMStoreFloat4x4(&cbPerObject.wMat, XMMatrixTranspose(matrixObj));
		XMStoreFloat4x4(&cbPerObject.wvpMat, XMMatrixTranspose(matrixObj * matrixView * matrixProjection));

		updateResource(constantBuffers[3], cbPerObject);

		context->DrawIndexed(obj->getNumIndices(), 0, 0);
	}

private:
	shared_ptr<ShaderDataCollection> shaderData;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState;
};