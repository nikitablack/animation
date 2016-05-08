#include <Windows.h>
#include <wrl/client.h>
#include <stdexcept>
#include <dxgi1_2.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <vector>
#include <DirectXMath.h>
#include "GeometryUtils.h"

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

using namespace std;
using namespace Microsoft::WRL;
using namespace DirectX;

struct ConstantBufferImmutable
{
	XMFLOAT4 checkboardColors[2];
};

struct ConstantBufferPerFrame
{
	XMFLOAT4X4 viewMat;
	XMFLOAT4X4 projMat;
	float timePassed;
	char padding[12];
};

struct ConstantBufferPerObject
{
	XMFLOAT4X4 wvpMat;
};

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
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

	return { device, context };
}

ComPtr<IDXGISwapChain> createSwapChain(IDXGIFactory1* factory, ID3D11Device* device, HWND hWnd, UINT bufferCount)
{
	RECT rect;
	if (!GetWindowRect(hWnd, &rect))
	{
		throw(runtime_error{ "Error getting window size." });
	}

	DXGI_SWAP_CHAIN_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.BufferDesc = { static_cast<UINT>(rect.right - rect.left), static_cast<UINT>(rect.bottom - rect.top), { 0, 1 }, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED, DXGI_MODE_SCALING_UNSPECIFIED };
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

	return { depthStencil, depthStencilView };
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

template<typename T>
ComPtr<ID3D11Buffer> createVertexBuffer(ID3D11Device* device, const vector<T>& data)
{
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.ByteWidth = static_cast<UINT>(data.size() * sizeof(T));
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA initialData;
	initialData.pSysMem = data.data();
	initialData.SysMemPitch = 0;
	initialData.SysMemSlicePitch = 0;

	ComPtr<ID3D11Buffer> buffer;
	if (FAILED(device->CreateBuffer(&desc, &initialData, buffer.ReleaseAndGetAddressOf())))
	{
		throw(runtime_error{ "Error creating vertex buffer." });
	}

	return buffer;
}

ComPtr<ID3D11Buffer> createIndexBuffer(ID3D11Device* device, const vector<uint32_t>& data)
{
	using T = remove_reference<decltype(data)>::type::value_type;

	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.ByteWidth = static_cast<UINT>(data.size() * sizeof(T));
	desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA initialData;
	initialData.pSysMem = data.data();
	initialData.SysMemPitch = 0;
	initialData.SysMemSlicePitch = 0;

	ComPtr<ID3D11Buffer> buffer;
	if (FAILED(device->CreateBuffer(&desc, &initialData, buffer.ReleaseAndGetAddressOf())))
	{
		throw(runtime_error{ "Error creating index buffer." });
	}

	return buffer;
}

pair<ComPtr<ID3D11VertexShader>, ComPtr<ID3DBlob>> createVertexShader(ID3D11Device* device)
{
	ComPtr<ID3DBlob> shaderBlob;
	if (FAILED(D3DReadFileToBlob(L"VertexShader.cso", shaderBlob.ReleaseAndGetAddressOf())))
	{
		throw(runtime_error{ "Error reading vertex shader." });
	}

	ComPtr<ID3D11VertexShader> vertexShader;
	if (FAILED(device->CreateVertexShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, vertexShader.ReleaseAndGetAddressOf())))
	{
		throw(runtime_error{ "Error creating vertex shader." });
	}

	return { vertexShader, shaderBlob };
}

ComPtr<ID3D11PixelShader> createPixelShader(ID3D11Device* device)
{
	ComPtr<ID3DBlob> shaderBlob;
	if (FAILED(D3DReadFileToBlob(L"PixelShader.cso", shaderBlob.ReleaseAndGetAddressOf())))
	{
		throw(runtime_error{ "Error reading pixel shader." });
	}

	ComPtr<ID3D11PixelShader> pixelShader;
	if (FAILED(device->CreatePixelShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, pixelShader.ReleaseAndGetAddressOf())))
	{
		throw(runtime_error{ "Error creating vertex shader." });
	}

	return pixelShader;
}

ComPtr<ID3D11InputLayout> createInputLayout(ID3D11Device* device, ID3DBlob* vertexShaderBlob)
{
	D3D11_INPUT_ELEMENT_DESC desc[3]{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	ComPtr<ID3D11InputLayout> inputLayout;
	if (FAILED(device->CreateInputLayout(desc, 3, vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), inputLayout.ReleaseAndGetAddressOf())))
	{
		throw(runtime_error{ "Error creating input layout." });
	}

	return inputLayout;
}

void render(ID3D11DeviceContext* context, ID3D11RenderTargetView* renderTargetView, ID3D11DepthStencilView* depthStencilView, IDXGISwapChain* swapChain, ID3D11Buffer* vertexBuffers[3])
{
	static const FLOAT clearColor[]{ 0.0f, 0.5f, 0.0f, 1.0f };
	context->ClearRenderTargetView(renderTargetView, clearColor);
	context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	context->OMSetRenderTargets(1, &renderTargetView, depthStencilView);

	UINT strides[3]{ static_cast<UINT>(sizeof(XMFLOAT3)), static_cast<UINT>(sizeof(XMFLOAT3)), static_cast<UINT>(sizeof(uint32_t)) };
	UINT offsets[3]{ 0, 0, 0 };
	context->IASetVertexBuffers(0, 3, vertexBuffers, strides, offsets);

	swapChain->Present(0, 0);
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	const LONG width{ 800 };
	const LONG height{ 600 };
	const UINT bufferCount{ 2 };

	HWND hWnd;
	ComPtr<IDXGIFactory1> factory;
	ComPtr<IDXGIAdapter2> adapter;
	ComPtr<ID3D11Device> device;
	ComPtr<ID3D11DeviceContext> context;
	ComPtr<IDXGISwapChain> swapChain;
	vector<ComPtr<ID3D11RenderTargetView>> renderTargetViews;
	ComPtr<ID3D11Texture2D> depthStencilTexture;
	ComPtr<ID3D11DepthStencilView> depthStencilView;
	ComPtr<ID3D11Buffer> constBufferImmutable;
	ComPtr<ID3D11Buffer> constBufferPerFrame;
	ComPtr<ID3D11Buffer> constBufferPerObject;
	ComPtr<ID3D11Buffer> positionsBuffer;
	ComPtr<ID3D11Buffer> normalsBuffer;
	ComPtr<ID3D11Buffer> colorIdsBuffer;
	ComPtr<ID3D11Buffer> indexBuffer;
	ComPtr<ID3D11VertexShader> vertexShader;
	ComPtr<ID3D11PixelShader> pixelShader;
	ComPtr<ID3D11InputLayout> inputLayout;

	try
	{
		CheckboardPlaneMesh planeMesh{ PlaneGeometry::generateCheckBoard(2, 2, 2, 2) };
		ConstantBufferImmutable consBufferImmutable{ { XMFLOAT4{ 0.5f, 0.5f, 0.5f, 0.5f }, XMFLOAT4{ 0.1f, 0.1f, 0.1f, 0.1f } } };

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
		constBufferImmutable = createConstantBufferImmutable(device.Get(), consBufferImmutable);
		constBufferPerFrame = createConstantBufferPerFrame(device.Get());
		constBufferPerObject = createConstantBufferPerObject(device.Get());
		positionsBuffer = createVertexBuffer(device.Get(), planeMesh.positions);
		normalsBuffer = createVertexBuffer(device.Get(), planeMesh.normals);
		colorIdsBuffer = createVertexBuffer(device.Get(), planeMesh.colorIds);
		indexBuffer = createIndexBuffer(device.Get(), planeMesh.indices);
		auto vertexShaderPair = createVertexShader(device.Get());
		vertexShader = vertexShaderPair.first;
		pixelShader = createPixelShader(device.Get());
		inputLayout = createInputLayout(device.Get(), vertexShaderPair.second.Get());
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

				ID3D11Buffer* vertexBuffers[3]{ positionsBuffer.Get(), normalsBuffer.Get(), colorIdsBuffer.Get() };
				render(context.Get(), renderTargetViews[presentCount % bufferCount].Get(), depthStencilView.Get(), swapChain.Get(), vertexBuffers);
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