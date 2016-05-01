#include <Windows.h>
#include <wrl/client.h>
#include <stdexcept>
#include <dxgi1_3.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <vector>
#include <DirectXMath.h>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

using namespace std;
using namespace Microsoft::WRL;
using namespace DirectX;

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

ComPtr<IDXGIFactory3> createFactory()
{
	UINT factoryFlags{ 0 };
#if _DEBUG
	factoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

	ComPtr<IDXGIFactory3> dxgiFactory;
	if (FAILED(CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&dxgiFactory))))
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

ComPtr<IDXGISwapChain2> createSwapChain(IDXGIFactory3* factory, ID3D11Device* device, HWND hWnd, UINT bufferCount)
{
	RECT rect;
	if (!GetWindowRect(hWnd, &rect))
	{
		throw(runtime_error{ "Error getting window size." });
	}

	DXGI_SWAP_CHAIN_DESC1 desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Width = rect.right - rect.left;
	desc.Height = rect.bottom - rect.top;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.Stereo = FALSE;
	desc.SampleDesc = { 1, 0 };
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.BufferCount = bufferCount;
	desc.Scaling = DXGI_SCALING_NONE;
	desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	desc.Flags = 0;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullscreenDesc;
	ZeroMemory(&fullscreenDesc, sizeof(fullscreenDesc));
	fullscreenDesc.RefreshRate = { 0, 1 };
	fullscreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	fullscreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	fullscreenDesc.Windowed = TRUE;

	ComPtr<IDXGISwapChain1> swapChain1;
	if (FAILED(factory->CreateSwapChainForHwnd(device, hWnd, &desc, &fullscreenDesc, NULL, swapChain1.ReleaseAndGetAddressOf())))
	{
		throw(runtime_error{ "Error creating swap chain." });
	}

	ComPtr<IDXGISwapChain2> swapChain;
	if (FAILED(swapChain1.As(&swapChain)))
	{
		throw(runtime_error{ "Error getting swap chain." });
	}
	
	return swapChain;
}

vector<ComPtr<ID3D11RenderTargetView>> createRenderTargetViews(IDXGISwapChain2* swapChain, ID3D11Device* device, UINT bufferCount)
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

pair<ComPtr<ID3D11Texture2D>, ComPtr<ID3D11DepthStencilView>> createDepthStencilPair(IDXGISwapChain2* swapChain, ID3D11Device* device)
{
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
	swapChain->GetDesc1(&swapChainDesc);

	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Width = swapChainDesc.Width;
	desc.Height = swapChainDesc.Height;
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

void render(ID3D11DeviceContext* context, ID3D11RenderTargetView* renderTargetView, ID3D11DepthStencilView* depthStencilView, IDXGISwapChain2* swapChain)
{
	static const FLOAT clearColor[]{ 0.0f, 0.5f, 0.0f, 1.0f };
	context->ClearRenderTargetView(renderTargetView, clearColor);
	context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	context->OMSetRenderTargets(1, &renderTargetView, depthStencilView);

	swapChain->Present(0, 0);
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	const LONG width{ 800 };
	const LONG height{ 600 };
	const UINT bufferCount{ 2 };

	HWND hWnd;
	ComPtr<IDXGIFactory3> factory;
	ComPtr<IDXGIAdapter2> adapter;
	ComPtr<ID3D11Device> device;
	ComPtr<ID3D11DeviceContext> context;
	ComPtr<IDXGISwapChain2> swapChain;
	vector<ComPtr<ID3D11RenderTargetView>> renderTargetViews;
	ComPtr<ID3D11Texture2D> depthStencilTexture;
	ComPtr<ID3D11DepthStencilView> depthStencilView;
	ComPtr<ID3D11Buffer> constBufferPerFrame;
	ComPtr<ID3D11Buffer> constBufferPerObject;

	try
	{
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
		constBufferPerFrame = createConstantBufferPerFrame(device.Get());
		constBufferPerObject = createConstantBufferPerObject(device.Get());
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
				render(context.Get(), renderTargetViews[presentCount % bufferCount].Get(), depthStencilView.Get(), swapChain.Get());
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