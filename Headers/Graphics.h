#pragma once

#include <dxgi1_2.h>
#include <wrl/client.h>
#include <d3d11.h>
#include <vector>
#include <cstdint>

class Graphics
{
public:
	Graphics(HWND hWnd, UINT bufferCount);

	ID3D11Device* getDevice();
	Microsoft::WRL::ComPtr<ID3D11Device> getDeviceCom();
	ID3D11DeviceContext* getContext();
	IDXGISwapChain* getSwapChain();
	ID3D11RenderTargetView* getRenderTargetView(uint32_t ind);
	ID3D11DepthStencilView* getDepthStencilView();

private:
	void createFactory();
	void createDeviceAndContext();
	void createSwapChain();
	void createRenderTargetViews();
	void createDepthStencilAndView();
private:
	HWND hWnd;
	UINT bufferCount;
	Microsoft::WRL::ComPtr<IDXGIFactory1> factory;
	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
	Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;
	std::vector<Microsoft::WRL::ComPtr<ID3D11RenderTargetView>> renderTargetViews;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencil;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;
};