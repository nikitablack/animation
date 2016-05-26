#include "Headers\Window.h"
#include "Headers\Graphics.h"
#include "Headers\Mesh.h"
#include "Headers\ShaderFactory.h"
#include "Headers\Object.h"
#include <Windows.h>
#include <dxgi1_2.h>
#include "Headers\GeometryUtils.h"
#include "Headers\MeshAsciiParser.h"
#include "Headers\DDSTextureLoader.h"
#include "Headers\ConstantBuffers.h"
#include "Headers\PlaneRenderer.h"
#include "Headers\KitanaRenderer.h"
#include "Headers\TeapotRenderer.h"
#include "Headers\ObjectFactory.h"

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

using namespace std;
using namespace Microsoft::WRL;
using namespace DirectX;

ID3D11Buffer* constantBuffers[4];
shared_ptr<Object> plane;
shared_ptr<PlaneRenderer> planeRenderer;
vector<shared_ptr<Object>> kitana;
shared_ptr<KitanaRenderer> kitanaRenderer;
vector<shared_ptr<Object>> teapot;
shared_ptr<TeapotRenderer> teapotRenderer;
D3D11_VIEWPORT viewport;

template<typename T>
void updateResource(ID3D11DeviceContext* context, ID3D11Resource* resource, const T& data)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	if (FAILED(context->Map(resource, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
	{
		throw(runtime_error{ "Error mapping constant buffer" });
	}

	T* cbPerObjDataPtr{ static_cast<T*>(mappedResource.pData) };
	*cbPerObjDataPtr = data;

	context->Unmap(resource, 0);
}

void render(ID3D11DeviceContext* context, ID3D11RenderTargetView* renderTargetView, ID3D11DepthStencilView* depthStencilView, IDXGISwapChain* swapChain)
{
	static const FLOAT clearColor[]{ 0.4f, 0.4f, 0.8f, 1.0f };
	context->ClearRenderTargetView(renderTargetView, clearColor);
	context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	context->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->RSSetViewports(1, &viewport);

	/////////////////////////// constant buffer per frame /////////////////
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	if (FAILED(context->Map(constantBuffers[2], 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
	{
		throw(runtime_error{ "Error mapping constant buffer" });
	}

	ConstantBufferPerFrame* cbPerFrameDataPtr{ static_cast<ConstantBufferPerFrame*>(mappedResource.pData) };

	XMVECTOR vecCamPosition(XMVectorSet(0.0f, 0.0f, -5.0f, 0.0f));
	XMVECTOR vecCamLookAt(XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f));
	XMVECTOR vecCamUp(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
	XMMATRIX matrixView(XMMatrixLookAtLH(vecCamPosition, vecCamLookAt, vecCamUp));
	XMMATRIX matrixProjection(XMMatrixPerspectiveFovLH(XMConvertToRadians(45), 800.0f / 600.0f, 1.0f, 100.0f));

	ConstantBufferPerFrame cbPerFrame;
	XMStoreFloat4x4(&cbPerFrame.viewMat, XMMatrixTranspose(matrixView));
	cbPerFrame.timePassed = 0;

	cbPerFrameDataPtr->viewMat = cbPerFrame.viewMat;
	cbPerFrameDataPtr->timePassed = cbPerFrame.timePassed;

	context->Unmap(constantBuffers[2], 0);
	////////////////////////////////////////////////////////////////////////

	context->VSSetConstantBuffers(0, 4, constantBuffers);
	context->HSSetConstantBuffers(0, 4, constantBuffers);
	context->DSSetConstantBuffers(0, 4, constantBuffers);

	//planeRenderer->render(plane, constantBuffers);

	/*for (shared_ptr<Object>& kitanaPart : kitana)
	{
		kitanaRenderer->render(kitanaPart, constantBuffers);
	}*/

	/*for (shared_ptr<Object>& teapotPart : teapot)
	{
		teapotRenderer->render(teapotPart, constantBuffers);
	}*/
	static float rot = 0.0f;
	rot += 0.02f;
	teapot[0]->setRotation(0.0f, 0.0f, 0.0f + rot);
	teapotRenderer->render(teapot[0], constantBuffers);
	teapot[1]->setRotation(0.0f, 0.0f, 0.0f + rot);
	teapotRenderer->render(teapot[1], constantBuffers);
	teapot[2]->setRotation(0.0f, 0.0f, 0.0f + rot);
	teapotRenderer->render(teapot[2], constantBuffers);
	teapot[3]->setRotation(0.0f, 0.0f, 0.0f + rot);
	teapotRenderer->render(teapot[3], constantBuffers);
	teapot[4]->setRotation(0.0f, 0.0f, 0.0f + rot);
	teapotRenderer->render(teapot[4], constantBuffers);

	teapot[0]->setRotation(0.0f, 0.0f, XM_PIDIV2 + rot);
	teapotRenderer->render(teapot[0], constantBuffers);
	teapot[1]->setRotation(0.0f, 0.0f, XM_PIDIV2 + rot);
	teapotRenderer->render(teapot[1], constantBuffers);
	teapot[2]->setRotation(0.0f, 0.0f, XM_PIDIV2 + rot);
	teapotRenderer->render(teapot[2], constantBuffers);
	teapot[3]->setRotation(0.0f, 0.0f, XM_PIDIV2 + rot);
	teapotRenderer->render(teapot[3], constantBuffers);
	teapot[4]->setRotation(0.0f, 0.0f, XM_PIDIV2 + rot);
	teapotRenderer->render(teapot[4], constantBuffers);

	teapot[0]->setRotation(0.0f, 0.0f, XM_PI + rot);
	teapotRenderer->render(teapot[0], constantBuffers);
	teapot[1]->setRotation(0.0f, 0.0f, XM_PI + rot);
	teapotRenderer->render(teapot[1], constantBuffers);
	teapot[2]->setRotation(0.0f, 0.0f, XM_PI + rot);
	teapotRenderer->render(teapot[2], constantBuffers);
	teapot[3]->setRotation(0.0f, 0.0f, XM_PI + rot);
	teapotRenderer->render(teapot[3], constantBuffers);
	teapot[4]->setRotation(0.0f, 0.0f, XM_PI + rot);
	teapotRenderer->render(teapot[4], constantBuffers);

	teapot[0]->setRotation(0.0f, 0.0f, -XM_PIDIV2 + rot);
	teapotRenderer->render(teapot[0], constantBuffers);
	teapot[1]->setRotation(0.0f, 0.0f, -XM_PIDIV2 + rot);
	teapotRenderer->render(teapot[1], constantBuffers);
	teapot[2]->setRotation(0.0f, 0.0f, -XM_PIDIV2 + rot);
	teapotRenderer->render(teapot[2], constantBuffers);
	teapot[3]->setRotation(0.0f, 0.0f, -XM_PIDIV2 + rot);
	teapotRenderer->render(teapot[3], constantBuffers);
	teapot[4]->setRotation(0.0f, 0.0f, -XM_PIDIV2 + rot);
	teapotRenderer->render(teapot[4], constantBuffers);

	teapot[5]->setScale(1.0f, 1.0f, 1.0f);
	teapot[5]->setRotation(0.0f, 0.0f, 0 + rot);
	teapotRenderer->render(teapot[5], constantBuffers);
	teapot[6]->setScale(1.0f, 1.0f, 1.0f);
	teapot[6]->setRotation(0.0f, 0.0f, 0 + rot);
	teapotRenderer->render(teapot[6], constantBuffers);

	teapot[5]->setScale(1.0f, -1.0f, 1.0f);
	teapot[5]->setRotation(0.0f, 0.0f, 0 + rot);
	teapotRenderer->render(teapot[5], constantBuffers);
	teapot[6]->setScale(1.0f, -1.0, 1.0f);
	teapot[6]->setRotation(0.0f, 0.0f, 0 + rot);
	teapotRenderer->render(teapot[6], constantBuffers);

	teapot[7]->setScale(1.0f, 1.0f, 1.0f);
	teapot[7]->setRotation(0.0f, 0.0f, 0 + rot);
	teapotRenderer->render(teapot[7], constantBuffers);
	teapot[8]->setScale(1.0f, 1.0f, 1.0f);
	teapot[8]->setRotation(0.0f, 0.0f, 0 + rot);
	teapotRenderer->render(teapot[8], constantBuffers);

	teapot[7]->setScale(1.0f, -1.0f, 1.0f);
	teapot[7]->setRotation(0.0f, 0.0f, 0 + rot);
	teapotRenderer->render(teapot[7], constantBuffers);
	teapot[8]->setScale(1.0f, -1.0f, 1.0f);
	teapot[8]->setRotation(0.0f, 0.0f, 0 + rot);
	teapotRenderer->render(teapot[8], constantBuffers);

	swapChain->Present(0, 0);
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	//MeshAsciiParser::parse("Kitana.mesh.ascii");
	//return 0;

	const LONG width{ 1280 };
	const LONG height{ 1024 };
	const UINT bufferCount{ 2 };

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = width;
	viewport.Height = height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	shared_ptr<Window> window;
	shared_ptr<Graphics> graphics;
	shared_ptr<BufferDataBase> constBufferImmutable;
	shared_ptr<BufferDataBase> constBufferProjectionMatrix;
	shared_ptr<BufferDataBase> constBufferPerFrame;
	shared_ptr<BufferDataBase> constBufferPerObject;
	shared_ptr<ShaderFactory> shaderFactory;
	shared_ptr<BufferFactory> bufferFactory;
	shared_ptr<ObjectFactory> objectFactory;

	try
	{
		window = make_shared<Window>(width, height);
		graphics = make_shared<Graphics>(window->getHandle(), bufferCount);

		shaderFactory = make_shared<ShaderFactory>(graphics->getDeviceCom());
		bufferFactory = make_shared<BufferFactory>(graphics->getDeviceCom());
		objectFactory = make_shared<ObjectFactory>(bufferFactory);

		ConstantBufferImmutable cbImmutable{ { XMFLOAT4{ 0.8f, 0.8f, 0.8f, 1.0f }, XMFLOAT4{ 0.5f, 0.5f, 0.5f, 1.0f } } };
		ConstantBufferProjectionMatrix cbProjectionMatrix;
		XMStoreFloat4x4(&cbProjectionMatrix.projMat, XMMatrixTranspose(XMMatrixPerspectiveFovLH(XMConvertToRadians(45), width / height, 1.0f, 100.0f)));

		CheckboardPlaneMesh checkboardPlaneMesh{ GeometryGenerator::generateCheckBoard(10.0f, 10.0f, 20, 20) };
		BoneArmatureMesh boneArmatureMesh{ GeometryGenerator::generateBone(0.5f) };

		constBufferImmutable = bufferFactory->createConstantBuffer(cbImmutable);
		constBufferProjectionMatrix = bufferFactory->createConstantBuffer<ConstantBufferProjectionMatrix>();
		constBufferPerFrame = bufferFactory->createConstantBuffer<ConstantBufferPerFrame>();
		constBufferPerObject = bufferFactory->createConstantBuffer<ConstantBufferPerObject>();

		constantBuffers[0] = constBufferImmutable->getBuffer();
		constantBuffers[1] = constBufferProjectionMatrix->getBuffer();
		constantBuffers[2] = constBufferPerFrame->getBuffer();
		constantBuffers[3] = constBufferPerObject->getBuffer();

		// plane
		plane = objectFactory->createPlane();
		planeRenderer = make_shared<PlaneRenderer>(graphics, shaderFactory);

		// kitana
		kitana = objectFactory->createKitana();
		kitanaRenderer = make_shared<KitanaRenderer>(graphics, shaderFactory);

		// teapot
		teapot = objectFactory->createTeapot();
		teapotRenderer = make_shared<TeapotRenderer>(graphics, shaderFactory);
	}
	catch (runtime_error& err)
	{
		MessageBox(nullptr, err.what(), "Error", MB_OK);
		return 0;
	}

	MSG msg;
	ZeroMemory(&msg, sizeof(msg));

	while (msg.message != WM_QUIT)
	{
		BOOL r{ PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) };
		if (r == 0)
		{
			try
			{
				UINT presentCount;
				graphics->getSwapChain()->GetLastPresentCount(&presentCount);

				render(graphics->getContext(), graphics->getRenderTargetView(presentCount % bufferCount), graphics->getDepthStencilView(), graphics->getSwapChain());
			}
			catch (runtime_error& err)
			{
				MessageBox(nullptr, err.what(), "Error", MB_OK);
				return 0;
			}
		}
		else
		{
			DispatchMessage(&msg);
		}
	}

	return 0;
}