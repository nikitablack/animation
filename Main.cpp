#include <Windows.h>
#include <dxgi1_2.h>
#include "Headers\GeometryUtils.h"
#include "Headers\Object3D.h"
#include "Headers\MeshAsciiParser.h"
#include "Headers\DDSTextureLoader.h"

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

using namespace std;
using namespace Microsoft::WRL;
using namespace DirectX;

const float CAMERA_STEP{ 0.1f };
float cameraX{ 0.0f };
float cameraY{ 1.0f };

struct ConstantBufferImmutable
{
	XMFLOAT4 checkboardColors[2];
};

struct ConstantBufferProjectionMatrix
{
	XMFLOAT4X4 projMat;
};

struct ConstantBufferPerFrame
{
	XMFLOAT4X4 viewMat;
	float timePassed;
	char padding[12];
};

struct ConstantBufferPerObject
{
	XMFLOAT4X4 wvpMat;
	XMFLOAT4X4 wMat;
};

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case 65:
			cameraX -= CAMERA_STEP;
			break;
		case 87:
			cameraY += CAMERA_STEP;
			break;
		case 68:
			cameraX += CAMERA_STEP;
			break;
		case 83:
			cameraY -= CAMERA_STEP;
			break;
		}
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

HWND createWindow(LONG width, LONG height)
{
	WNDCLASSEX wcex;
	ZeroMemory(&wcex, sizeof(wcex));
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = (HMODULE)GetModuleHandle(0);
	wcex.hIcon = LoadIcon(NULL, IDI_SHIELD);
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = "WindowClass";
	wcex.hIconSm = LoadIcon(NULL, IDI_WARNING);

	if (RegisterClassEx(&wcex) == 0)
	{
		throw(runtime_error{ "Error registering window." });
	}

	RECT rect{ 0, 0, width, height };
	AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, FALSE, 0);

	HWND hWnd{ CreateWindowEx(0, "WindowClass", "Hello, Triangle!", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, rect.right - rect.left, rect.bottom - rect.top, nullptr, nullptr, nullptr, nullptr) };
	if (!hWnd)
	{
		throw(runtime_error{ "Error creating window." });
	}

	ShowWindow(hWnd, SW_SHOW);

	return hWnd;
}

ComPtr<IDXGIFactory1> createFactory()
{
	ComPtr<IDXGIFactory2> dxgiFactory;
	if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory))))
	{
		throw(runtime_error{ "Error creating IDXGIFactory." });
	}

	return dxgiFactory;
}

pair<ComPtr<ID3D11Device>, ComPtr<ID3D11DeviceContext>> createDeviceAndContext()
{
	UINT flags{ D3D11_CREATE_DEVICE_BGRA_SUPPORT };
#if defined(_DEBUG)
	// If the project is in a debug build, enable the debug layer.
	flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	ComPtr<ID3D11Device> device;
	ComPtr<ID3D11DeviceContext> context;
	vector<D3D_FEATURE_LEVEL> featureLevels{ D3D_FEATURE_LEVEL_11_0 };
	if (FAILED(D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, flags, featureLevels.data(), static_cast<UINT>(featureLevels.size()), D3D11_SDK_VERSION, device.ReleaseAndGetAddressOf(), NULL, context.ReleaseAndGetAddressOf())))
	{
		throw(runtime_error{ "Error creating device and context." });
	}

	return{ device, context };
}

ComPtr<IDXGISwapChain> createSwapChain(IDXGIFactory1* factory, ID3D11Device* device, HWND hWnd, UINT bufferCount)
{
	RECT rect;
	if (!GetClientRect(hWnd, &rect))
	{
		throw(runtime_error{ "Error getting window size." });
	}

	DXGI_SWAP_CHAIN_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.BufferDesc = { static_cast<UINT>(rect.right - rect.left), static_cast<UINT>(rect.bottom - rect.top), { 0, 1 }, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED, DXGI_MODE_SCALING_CENTERED };
	desc.SampleDesc = { 1, 0 };
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.BufferCount = bufferCount;
	desc.OutputWindow = hWnd;
	desc.Windowed = TRUE;
	desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	desc.Flags = 0;

	ComPtr<IDXGISwapChain> swapChain;
	if (FAILED(factory->CreateSwapChain(device, &desc, swapChain.ReleaseAndGetAddressOf())))
	{
		throw(runtime_error{ "Error creating swap chain." });
	}

	return swapChain;
}

vector<ComPtr<ID3D11RenderTargetView>> createRenderTargetViews(IDXGISwapChain* swapChain, ID3D11Device* device, UINT bufferCount)
{
	vector<ComPtr<ID3D11RenderTargetView>> renderTargetViews;
	for (UINT i{ 0 }; i < bufferCount; ++i)
	{
		ComPtr<ID3D11Texture2D> backBuffer;
		if (FAILED(swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.ReleaseAndGetAddressOf()))))
		{
			throw(runtime_error{ "Error getting back buffer." });
		}

		ComPtr<ID3D11RenderTargetView> renderTargetView;
		if (FAILED(device->CreateRenderTargetView(backBuffer.Get(), NULL, renderTargetView.ReleaseAndGetAddressOf())))
		{
			throw(runtime_error{ "Error creating render target view." });
		}

		renderTargetViews.push_back(renderTargetView);
	}

	return renderTargetViews;
}

pair<ComPtr<ID3D11Texture2D>, ComPtr<ID3D11DepthStencilView>> createDepthStencilPair(IDXGISwapChain* swapChain, ID3D11Device* device)
{
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	swapChain->GetDesc(&swapChainDesc);

	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Width = swapChainDesc.BufferDesc.Width;
	desc.Height = swapChainDesc.BufferDesc.Height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	desc.SampleDesc = swapChainDesc.SampleDesc;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	ComPtr<ID3D11Texture2D> depthStencil;
	if (FAILED(device->CreateTexture2D(&desc, NULL, depthStencil.ReleaseAndGetAddressOf())))
	{
		throw(runtime_error{ "Error creating depth stencil texture." });
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC viewDesc;
	ZeroMemory(&viewDesc, sizeof(viewDesc));
	viewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	viewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	viewDesc.Texture2D.MipSlice = 0;
	viewDesc.Flags = 0;

	ComPtr<ID3D11DepthStencilView> depthStencilView;
	if (FAILED(device->CreateDepthStencilView(depthStencil.Get(), &viewDesc, depthStencilView.ReleaseAndGetAddressOf())))
	{
		throw(runtime_error{ "Error creating depth stencil view." });
	}

	return{ depthStencil, depthStencilView };
}

ComPtr<ID3D11Buffer> createConstantBufferImmutable(ID3D11Device* device, const ConstantBufferImmutable& data)
{
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.ByteWidth = sizeof(data);
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA initialData;
	initialData.pSysMem = &data;
	initialData.SysMemPitch = 0;
	initialData.SysMemSlicePitch = 0;

	ComPtr<ID3D11Buffer> constBuffer;
	if (FAILED(device->CreateBuffer(&desc, &initialData, constBuffer.ReleaseAndGetAddressOf())))
	{
		throw(runtime_error{ "Error creating constant buffer." });
	}

	return constBuffer;
}

ComPtr<ID3D11Buffer> createConstantBufferProjectionMatrix(ID3D11Device* device, const ConstantBufferProjectionMatrix& data)
{
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.ByteWidth = sizeof(data);
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA initialData;
	initialData.pSysMem = &data;
	initialData.SysMemPitch = 0;
	initialData.SysMemSlicePitch = 0;

	ComPtr<ID3D11Buffer> constBuffer;
	if (FAILED(device->CreateBuffer(&desc, &initialData, constBuffer.ReleaseAndGetAddressOf())))
	{
		throw(runtime_error{ "Error creating constant buffer." });
	}

	return constBuffer;
}

ComPtr<ID3D11Buffer> createConstantBufferPerFrame(ID3D11Device* device)
{
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.ByteWidth = sizeof(ConstantBufferPerFrame);
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	ComPtr<ID3D11Buffer> constBuffer;
	if (FAILED(device->CreateBuffer(&desc, NULL, constBuffer.ReleaseAndGetAddressOf())))
	{
		throw(runtime_error{ "Error creating constant buffer." });
	}

	return constBuffer;
}

ComPtr<ID3D11Buffer> createConstantBufferPerObject(ID3D11Device* device)
{
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.ByteWidth = sizeof(ConstantBufferPerObject);
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	ComPtr<ID3D11Buffer> constBuffer;
	if (FAILED(device->CreateBuffer(&desc, NULL, constBuffer.ReleaseAndGetAddressOf())))
	{
		throw(runtime_error{ "Error creating constant buffer." });
	}

	return constBuffer;
}

void render(ID3D11DeviceContext* context, ID3D11RenderTargetView* renderTargetView, ID3D11DepthStencilView* depthStencilView, IDXGISwapChain* swapChain, D3D11_VIEWPORT* viewport,
	ID3D11Buffer* constantBuffers[4], vector<shared_ptr<Object3D>> objects, ID3D11ShaderResourceView* matcapView, ID3D11SamplerState* matcapSampler, ID3D11DepthStencilState* depthStencilState,
	shared_ptr<Object3D> kitana)
{
	static const FLOAT clearColor[]{ 0.5f, 0.5f, 0.5f, 1.0f };
	context->ClearRenderTargetView(renderTargetView, clearColor);
	context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	context->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->RSSetViewports(1, viewport);

	/////////////////////////// constant buffer per frame /////////////////
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	if (FAILED(context->Map(constantBuffers[2], 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
	{
		throw(runtime_error{ "Error mapping constant buffer" });
	}

	ConstantBufferPerFrame* cbPerFrameDataPtr{ static_cast<ConstantBufferPerFrame*>(mappedResource.pData) };

	XMVECTOR vecCamPosition(XMVectorSet(cameraX, cameraY, -3.0f, 0.0f));
	XMVECTOR vecCamLookAt(XMVectorSet(0.0f, 1.0f, 1.0f, 0.0f));
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

	context->PSSetShaderResources(0, 1, &matcapView);
	context->PSSetSamplers(0, 1, &matcapSampler);
	context->OMSetDepthStencilState(depthStencilState, 1);

	static float rot{ 0 };
	kitana->setRotation(0, rot, 0);
	rot += 0.01f;

	for (shared_ptr<Object3D>& obj : objects)
	{
		const vector<ID3D11Buffer*>& vertexBuffers{ obj->getVertexBuffers() };

		context->IASetVertexBuffers(0, static_cast<UINT>(vertexBuffers.size()), vertexBuffers.data(), obj->getVertexStrides().data(), obj->getVertexOffsets().data());

		context->IASetIndexBuffer(obj->getIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);

		context->IASetInputLayout(obj->getInputLayout());
		context->VSSetShader(obj->getVertexShader(), nullptr, 0);
		context->PSSetShader(obj->getPixelShader(), nullptr, 0);
		context->RSSetState(obj->getRasterizerState());

		/////////////////////////// constant buffer per object /////////////////
		if (FAILED(context->Map(constantBuffers[3], 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		{
			throw(runtime_error{ "Error mapping constant buffer" });
		}

		ConstantBufferPerObject* cbPerObjDataPtr{ static_cast<ConstantBufferPerObject*>(mappedResource.pData) };

		//obj->setRotation(0, XM_PI, 0);
		obj->setRotation(0, 0, 0);
		XMMATRIX matrixObj(XMLoadFloat3x3(&obj->getTransformGlobal()));

		ConstantBufferPerObject cbPerObject;
		XMStoreFloat4x4(&cbPerObject.wvpMat, XMMatrixTranspose(matrixObj * matrixView * matrixProjection));

		XMFLOAT4X4 tmp;
		XMStoreFloat4x4(&tmp, XMMatrixTranspose(XMLoadFloat3x3(&obj->getTransformGlobal())));
		cbPerObjDataPtr->wMat = tmp;
		cbPerObjDataPtr->wvpMat = cbPerObject.wvpMat;

		context->Unmap(constantBuffers[3], 0);
		////////////////////////////////////////////////////////////////////////

		context->DrawIndexed(obj->getNumIndices(), 0, 0);
	}

	swapChain->Present(0, 0);
}

vector<shared_ptr<Object3D>> createKitana(ComPtr<ID3D11Device> device, shared_ptr<Object3D> kitanaObj)
{
	vector<shared_ptr<Object3D>> kitana;

	vector<Bone> bones;
	vector<vector<Vertex>> meshes;
	vector<vector<uint32_t>> faces;
	MeshAsciiParser::read("kitana", bones, meshes, faces);

	for (int i{ 0 }; i < meshes.size(); ++i)
	{
		vector<Vertex> meshData{ meshes[i] };
		vector<XMFLOAT3> vertices;
		vector<XMFLOAT3> normals;
		vector<XMINT3> bones;
		vector<XMFLOAT3> bonesWeights;
		for (auto& vertex : meshData)
		{
			vertices.push_back(vertex.pos);
			normals.push_back(vertex.normal);
			bones.push_back(vertex.bones);
			bonesWeights.push_back(vertex.bonesWeights);
		}

		vector<uint32_t> face{ faces[i] };
		vector<uint32_t> indices;
		for (uint32_t ind : face)
		{
			indices.push_back(ind);
		}

		pair<vector<XMFLOAT3>, string> p1{ vertices, "POSITION" };
		pair<vector<XMFLOAT3>, string> p2{ normals, "NORMAL" };
		pair<vector<XMINT3>, string> p3{ bones, "BONE" };
		pair<vector<XMFLOAT3>, string> p4{ bonesWeights, "BONE_WEIGHT" };
		auto t = make_tuple(p1, p2, p3, p4);
		shared_ptr<VertexBufferData<XMFLOAT3, XMFLOAT3, XMINT3, XMFLOAT3>> vertexBufferData{ make_shared<VertexBufferData<XMFLOAT3, XMFLOAT3, XMINT3, XMFLOAT3>>(device, t) };
		shared_ptr<Mesh<XMFLOAT3, XMFLOAT3, XMINT3, XMFLOAT3>> mesh{ make_shared<Mesh<XMFLOAT3, XMFLOAT3, XMINT3, XMFLOAT3>>(device, vertexBufferData, indices) };
		shared_ptr<ShaderData<ID3D11VertexShader>> vertexShaderData{ make_shared<ShaderData<ID3D11VertexShader>>(device, L"MatcapVertexShader.cso") };
		shared_ptr<ShaderData<ID3D11PixelShader>> pixelShaderData{ make_shared<ShaderData<ID3D11PixelShader>>(device, L"MatcapPixelShader.cso") };
		shared_ptr<Object3D> obj{ make_shared<Object3D>(device, mesh, vertexShaderData, pixelShaderData) };

		if (i == 6 || i == 16 || i == 17 || i == 18 || i == 21 || i == 23 || i == 24 || i == 25 || i == 27)
		{
			kitana.push_back(obj);
			kitanaObj->addChild(obj);
		}
	}
	
	return kitana;
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	//MeshAsciiParser::parse("Kitana.mesh.ascii");
	//return 0;

	const LONG width{ 800 };
	const LONG height{ 600 };
	const UINT bufferCount{ 2 };

	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = width;
	viewport.Height = height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	HWND hWnd;
	ComPtr<IDXGIFactory1> factory;
	ComPtr<IDXGIAdapter2> adapter;
	ComPtr<ID3D11Device> device;
	ComPtr<ID3D11DeviceContext> context;
	ComPtr<IDXGISwapChain> swapChain;
	vector<ComPtr<ID3D11RenderTargetView>> renderTargetViews;
	ComPtr<ID3D11Texture2D> depthStencilTexture;
	ComPtr<ID3D11DepthStencilView> depthStencilView;
	ComPtr<ID3D11Texture2D> matcapTexture;
	ComPtr<ID3D11ShaderResourceView> matcapView;
	ComPtr<ID3D11SamplerState> matcapSampler;
	ComPtr<ID3D11DepthStencilState> depthStencilState;

	ComPtr<ID3D11Buffer> constBufferImmutable;
	ComPtr<ID3D11Buffer> constBufferProjectionMatrix;
	ComPtr<ID3D11Buffer> constBufferPerFrame;
	ComPtr<ID3D11Buffer> constBufferPerObject;

	shared_ptr<Object3D> plane;
	shared_ptr<Object3D> bone;
	shared_ptr<Object3D> kitana = make_shared<Object3D>();
	vector<shared_ptr<Object3D>> objects;
	vector<shared_ptr<Object3D>> bones;

	try
	{
		ConstantBufferImmutable cbImmutable{ { XMFLOAT4{ 0.8f, 0.8f, 0.8f, 1.0f }, XMFLOAT4{ 0.5f, 0.5f, 0.5f, 1.0f } } };
		ConstantBufferProjectionMatrix cbProjectionMatrix;
		XMStoreFloat4x4(&cbProjectionMatrix.projMat, XMMatrixTranspose(XMMatrixPerspectiveFovLH(XMConvertToRadians(45), width / height, 1.0f, 100.0f)));

		CheckboardPlaneMesh checkboardPlaneMesh{ GeometryGenerator::generateCheckBoard(10.0f, 10.0f, 100, 100) };
		BoneArmatureMesh boneArmatureMesh{ GeometryGenerator::generateBone(0.5f) };

		hWnd = createWindow(width, height);
		factory = createFactory();
		auto deviceAndContext = createDeviceAndContext();
		device = deviceAndContext.first;
		context = deviceAndContext.second;
		swapChain = createSwapChain(factory.Get(), device.Get(), hWnd, bufferCount);
		renderTargetViews = createRenderTargetViews(swapChain.Get(), device.Get(), bufferCount);
		auto depthStencilPair = createDepthStencilPair(swapChain.Get(), device.Get());
		depthStencilTexture = depthStencilPair.first;
		depthStencilView = depthStencilPair.second;

		constBufferImmutable = createConstantBufferImmutable(device.Get(), cbImmutable);
		constBufferProjectionMatrix = createConstantBufferProjectionMatrix(device.Get(), cbProjectionMatrix);
		constBufferPerFrame = createConstantBufferPerFrame(device.Get());
		constBufferPerObject = createConstantBufferPerObject(device.Get());

		HRESULT hr = CreateDDSTextureFromFile(device.Get(), L"textures/matcap2.dds", (ID3D11Resource**)matcapTexture.ReleaseAndGetAddressOf(), matcapView.ReleaseAndGetAddressOf());
		
		D3D11_SAMPLER_DESC matcapSamplerDesc;
		ZeroMemory(&matcapSamplerDesc, sizeof(matcapSamplerDesc));
		matcapSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		matcapSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		matcapSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		matcapSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		matcapSamplerDesc.MipLODBias = 0.0f;
		matcapSamplerDesc.MaxAnisotropy = 1;
		matcapSamplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		matcapSamplerDesc.BorderColor[0] = 0;
		matcapSamplerDesc.BorderColor[1] = 0;
		matcapSamplerDesc.BorderColor[2] = 0;
		matcapSamplerDesc.BorderColor[3] = 0;
		matcapSamplerDesc.MinLOD = 0;
		matcapSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
		hr = device->CreateSamplerState(&matcapSamplerDesc, matcapSampler.ReleaseAndGetAddressOf());

		D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
		ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));
		depthStencilDesc.DepthEnable = true;
		depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
		depthStencilDesc.StencilEnable = true;
		depthStencilDesc.StencilReadMask = 0xFF;
		depthStencilDesc.StencilWriteMask = 0xFF;
		depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
		depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
		depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		hr = device->CreateDepthStencilState(&depthStencilDesc, depthStencilState.ReleaseAndGetAddressOf());

		// plane
		pair<vector<XMFLOAT3>, string> p1{ checkboardPlaneMesh.positions, "POSITION" };
		pair<vector<XMFLOAT3>, string> p2{ checkboardPlaneMesh.normals, "NORMAL" };
		pair<vector<uint32_t>, string> p3{ checkboardPlaneMesh.colorIds, "COLOR_ID" };
		auto pt = make_tuple(p1, p2, p3);
		shared_ptr<VertexBufferData<XMFLOAT3, XMFLOAT3, uint32_t>> planeVertexBufferData{ make_shared<VertexBufferData<XMFLOAT3, XMFLOAT3, uint32_t>>(device, pt) };
		shared_ptr<Mesh<XMFLOAT3, XMFLOAT3, uint32_t>> planeMesh{ make_shared<Mesh<XMFLOAT3, XMFLOAT3, uint32_t>>(device, planeVertexBufferData, checkboardPlaneMesh.indices) };
		shared_ptr<ShaderData<ID3D11VertexShader>> planeVertexShaderData{ make_shared<ShaderData<ID3D11VertexShader>>(device, L"PlaneVertexShader.cso") };
		shared_ptr<ShaderData<ID3D11PixelShader>> planePixelShaderData{ make_shared<ShaderData<ID3D11PixelShader>>(device, L"PlanePixelShader.cso") };
		plane = make_shared<Object3D>(device, planeMesh, planeVertexShaderData, planePixelShaderData);

		//objects.push_back(plane);
		
		// bone
		pair<vector<XMFLOAT3>, string> b1{ boneArmatureMesh.positions, "POSITION" };
		auto bt = make_tuple(b1);
		shared_ptr<VertexBufferData<XMFLOAT3>> boneVertexBufferData{ make_shared<VertexBufferData<XMFLOAT3>>(device, bt) };
		shared_ptr<Mesh<XMFLOAT3>> boneMesh{ make_shared<Mesh<XMFLOAT3>>(device, boneVertexBufferData, boneArmatureMesh.indices) };
		shared_ptr<ShaderData<ID3D11VertexShader>> boneVertexShaderData{ make_shared<ShaderData<ID3D11VertexShader>>(device, L"BoneVertexShader.cso") };
		shared_ptr<ShaderData<ID3D11PixelShader>> bonePixelShaderData{ make_shared<ShaderData<ID3D11PixelShader>>(device, L"BonePixelShader.cso") };
		bone = make_shared<Object3D>(device, boneMesh, boneVertexShaderData, bonePixelShaderData, true, false);

		//objects.push_back(bone);

		// kitana
		objects = createKitana(device, kitana);
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
				swapChain->GetLastPresentCount(&presentCount);

				ID3D11Buffer* constantBuffers[4]{ constBufferImmutable.Get(), constBufferProjectionMatrix.Get(), constBufferPerFrame.Get(), constBufferPerObject.Get() };

				render(context.Get(), renderTargetViews[presentCount % bufferCount].Get(), depthStencilView.Get(), swapChain.Get(), &viewport,
					constantBuffers, objects, matcapView.Get(), matcapSampler.Get(), depthStencilState.Get(), kitana);
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