#pragma once

#include "BaseRenderer.h"
#include "ConstantBuffers.h"

class KitanaRenderer : public BaseRenderer
{
public:
	KitanaRenderer(std::shared_ptr<Graphics> graphics, std::shared_ptr<ShaderFactory> shaderFactory) : BaseRenderer{ graphics }
	{
		D3D11_RASTERIZER_DESC rasterizerDesc;
		ZeroMemory(&rasterizerDesc, sizeof(rasterizerDesc));
		rasterizerDesc.FillMode = D3D11_FILL_SOLID;
		rasterizerDesc.CullMode = D3D11_CULL_BACK;
		rasterizerDesc.FrontCounterClockwise = TRUE;
		rasterizerDesc.DepthBias = 0;
		rasterizerDesc.DepthBiasClamp = 0.0f;
		rasterizerDesc.SlopeScaledDepthBias = 0.0f;
		rasterizerDesc.DepthClipEnable = FALSE;
		rasterizerDesc.ScissorEnable = FALSE;
		rasterizerDesc.MultisampleEnable = FALSE;
		rasterizerDesc.AntialiasedLineEnable = FALSE;

		if (FAILED(graphics->getDevice()->CreateRasterizerState(&rasterizerDesc, rasterizerState.ReleaseAndGetAddressOf())))
		{
			throw(runtime_error{ "Error creating rasterizer state." });
		}

		shaderData = make_shared<ShaderDataCollection>();
		shaderData->vertexShaderData = shaderFactory->createVertexShader(L"MatcapVertexShader.cso");
		shaderData->pixelShaderData = shaderFactory->createPixelShader(L"MatcapPixelShader.cso");

		if (FAILED(CreateDDSTextureFromFile(graphics->getDevice(), L"textures/matcap2.dds", (ID3D11Resource**)matcapTexture.ReleaseAndGetAddressOf(), matcapView.ReleaseAndGetAddressOf())))
		{
			throw(runtime_error{ "Error creating a texture" });
		}

		D3D11_SAMPLER_DESC samplerDesc;
		ZeroMemory(&samplerDesc, sizeof(samplerDesc));
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.MipLODBias = 0.0f;
		samplerDesc.MaxAnisotropy = 1;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		samplerDesc.BorderColor[0] = 0;
		samplerDesc.BorderColor[1] = 0;
		samplerDesc.BorderColor[2] = 0;
		samplerDesc.BorderColor[3] = 0;
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		if (FAILED(graphics->getDevice()->CreateSamplerState(&samplerDesc, standardSampler.ReleaseAndGetAddressOf())))
		{
			throw(runtime_error{ "Error creating a sampler" });
		}
	}

	void render(std::shared_ptr<Object> obj, ID3D11Buffer* constantBuffers[4])
	{
		ID3D11DeviceContext* context{ graphics->getContext() };

		const vector<ID3D11Buffer*>& vertexBuffers{ obj->getVertexBuffers() };

		context->IASetVertexBuffers(0, static_cast<UINT>(vertexBuffers.size()), vertexBuffers.data(), obj->getVertexStrides().data(), obj->getVertexOffsets().data());
		context->IASetIndexBuffer(obj->getIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);

		context->IASetInputLayout(shaderData->vertexShaderData->getInputLayout());
		context->VSSetShader(shaderData->vertexShaderData->getShader(), nullptr, 0);
		context->PSSetShader(shaderData->pixelShaderData->getShader(), nullptr, 0);
		context->RSSetState(rasterizerState.Get());

		XMVECTOR vecCamPosition(XMVectorSet(0.0f, 1.0f, -5.0f, 0.0f));
		XMVECTOR vecCamLookAt(XMVectorSet(0.0f, 1.0f, 1.0f, 0.0f));
		XMVECTOR vecCamUp(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
		XMMATRIX matrixView(XMMatrixLookAtLH(vecCamPosition, vecCamLookAt, vecCamUp));
		XMMATRIX matrixProjection(XMMatrixPerspectiveFovLH(XMConvertToRadians(45), 800.0f / 600.0f, 1.0f, 100.0f));

		static float rot{ 0.0f };
		obj->setRotation(0, rot, 0);
		rot += 0.01f;
		XMMATRIX matrixObj(XMLoadFloat4x4(&obj->getTransformGlobal()));

		ConstantBufferPerObject cbPerObject;
		XMStoreFloat4x4(&cbPerObject.wMat, XMMatrixTranspose(matrixObj));
		XMStoreFloat4x4(&cbPerObject.wvpMat, XMMatrixTranspose(matrixObj * matrixView * matrixProjection));

		updateResource(constantBuffers[3], cbPerObject);

		ID3D11ShaderResourceView* viewPtr = matcapView.Get();
		ID3D11SamplerState* samplerPtr = standardSampler.Get();
		context->PSSetShaderResources(0, 1, &viewPtr);
		context->PSSetSamplers(0, 1, &samplerPtr);

		context->DrawIndexed(obj->getNumIndices(), 0, 0);
	}

private:
	shared_ptr<ShaderDataCollection> shaderData;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> matcapTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> matcapView;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> standardSampler;
};