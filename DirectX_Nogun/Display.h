#pragma once


#ifndef _DISPLAY_
#define _DISPLAY_

#include "header.h"

#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
//#include <imconfig.h>

using namespace std;
using namespace DirectX;
using Microsoft::WRL::ComPtr;

class Display
{
public:
	Display();
	~Display();

	bool Initialize(int, int, HWND);
	bool Init3D();
	bool InitImgui();

	void UpdateImgui();

		
	HWND m_hwnd;
	int m_width;
	int m_height;

	ComPtr<ID3D11Device> m_device;
	ComPtr<ID3D11DeviceContext> m_context;
	ComPtr<IDXGISwapChain> m_swapChain;

	// Back buffer
	ComPtr<ID3D11Texture2D> m_backBuffer;
	ComPtr<ID3D11RenderTargetView>   m_backBufferRTV;
	ComPtr<ID3D11ShaderResourceView> m_backBufferSRV;
	// Depth buffer
	ComPtr<ID3D11Texture2D> m_DepthStencilBuffer;
	ComPtr<ID3D11DepthStencilView> m_DepthStencilView;
	ComPtr<ID3D11DepthStencilState> m_DepthStencilState;

	

	D3D11_VIEWPORT m_viewport;

};

#endif 