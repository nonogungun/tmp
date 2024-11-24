#include "Display.h"


Display::Display()
{
}

Display::~Display()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

bool Display::Initialize(int width, int height, HWND hwnd)
{
    m_width = width;
    m_height = height;
    m_hwnd = hwnd;

    if (!Init3D()) return false;

    if (!InitImgui()) return false;

    return true;
}

void Display::UpdateImgui()
{

}
//Imgui로 Delta Time 구할 수 있긴 함
//ImGui::Text("Average %.3f ms/frame (%.1f FPS)",
//	1000.0f / ImGui::GetIO().Framerate,
//	ImGui::GetIO().Framerate);


bool Display::InitImgui() 
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    io.DisplaySize = ImVec2(float(m_width), float(m_height));

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;      // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;


    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }


    //Backend
    if (!ImGui_ImplDX11_Init(m_device.Get(), m_context.Get())) {
        return false;
    }

    if (!ImGui_ImplWin32_Init(m_hwnd)) {
        return false;
    }

    return true;
}


bool Display::Init3D()
{
    //Swapchain, device, device context 생성 
    DXGI_SWAP_CHAIN_DESC sc;
    ZeroMemory(&sc, sizeof(sc));
    sc.BufferDesc.Width = m_width;
    sc.BufferDesc.Height = m_height;
    sc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sc.BufferCount = 2; // Double-buffering
    sc.SampleDesc.Count = 1; // multisamples
    sc.SampleDesc.Quality = 0;
    sc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
    sc.OutputWindow = m_hwnd;
    sc.Windowed = true; // fullscreen일 경우 false
    sc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    //DXGI_SWAP_EFFECT_FLIP_DISCARD , DXGI_SWAP_EFFECT_DISCARD
    sc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    //주사율 60Hz로 고정 
    sc.BufferDesc.RefreshRate.Numerator = 60; //60
    sc.BufferDesc.RefreshRate.Denominator = 1;


    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
    UINT createDeviceFlags = 0;

#if defined(DEBUG) || defined(_DEBUG)
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

   // if (FAILED(D3D11CreateDeviceAndSwapChain(
   //     NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags,
   //     &featureLevel, 1, D3D11_SDK_VERSION, &sc,
   //     m_swapChain.GetAddressOf(), m_device.GetAddressOf(),
   //     NULL, m_context.GetAddressOf()))) return false;

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags,
        &featureLevel, 1, D3D11_SDK_VERSION, &sc,
        m_swapChain.GetAddressOf(), m_device.GetAddressOf(),
        NULL, m_context.GetAddressOf());


    //ComPtr<ID3D11Texture2D> backBuffer;
    //m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
    m_swapChain->GetBuffer(0, IID_PPV_ARGS(m_backBuffer.GetAddressOf()));
    if (m_backBuffer)
    {
        m_device->CreateRenderTargetView(m_backBuffer.Get(), 
            nullptr, m_backBufferRTV.GetAddressOf());
        m_device->CreateShaderResourceView(m_backBuffer.Get(),
            nullptr, m_backBufferSRV.GetAddressOf());
        // MSAA 가능한지 확인 
       // m_device->CheckMultisampleQualityLevels(DXGI_FORMAT_R16G16B16A16_FLOAT, 4, &m_numMSAA);
    }
    else return false; // 만들기 실패


    //viewport
    ZeroMemory(&m_viewport, sizeof(D3D11_VIEWPORT));
    m_viewport.Width = (float)m_width;
    m_viewport.Height = (float)m_height;
    m_viewport.MinDepth = 0.0f;
    m_viewport.MaxDepth = 1.0f;
    m_viewport.TopLeftX = 0.0f;
    m_viewport.TopLeftY = 0.0f;
    m_context->RSSetViewports(1, &m_viewport);


    //Depth-Stencil 버퍼 + State객체 + Resource View 생성 
   // 스텐실 버퍼는 모션 블러, 볼류메트릭 그림자, 거울 효과를 낼때 사용한다. 
    D3D11_TEXTURE2D_DESC depthBufferDesc;
    ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));
    depthBufferDesc.Width = m_width;
    depthBufferDesc.Height = m_height;
    depthBufferDesc.MipLevels = 1;
    depthBufferDesc.ArraySize = 1;
    depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthBufferDesc.SampleDesc.Count = 1;
    depthBufferDesc.SampleDesc.Quality = 0;
    depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthBufferDesc.CPUAccessFlags = 0;
    depthBufferDesc.MiscFlags = 0;
    if (FAILED(m_device->CreateTexture2D(
        &depthBufferDesc, NULL, m_DepthStencilBuffer.GetAddressOf()))) return false;


    // state 
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
    if (FAILED(m_device->CreateDepthStencilState(
        &depthStencilDesc, m_DepthStencilState.GetAddressOf()))) return false;


    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
    // FIX_ME : 홍정모는 이거 안적던데 안되면 이게 문제 ( 그냥 생략한듯 ) 
    ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));
    depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencilViewDesc.Texture2D.MipSlice = 0;
    if (FAILED(m_device->CreateDepthStencilView(m_DepthStencilBuffer.Get(),
        &depthStencilViewDesc, m_DepthStencilView.GetAddressOf()))) return false;
    

    return true;
}

