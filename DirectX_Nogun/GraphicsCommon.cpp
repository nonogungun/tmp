#include "GraphicsCommon.h"


// extern : 한 번 더 정의
namespace Graphics {

// Samplers
 ComPtr<ID3D11SamplerState> linearWrapSS;
 ComPtr<ID3D11SamplerState> linearClampSS;
//ComPtr<ID3D11SamplerState> shadowPointSS;
//ComPtr<ID3D11SamplerState> shadowCompareSS;
 vector<ID3D11SamplerState*> sampleStates;

// Rasterizer States
 ComPtr<ID3D11RasterizerState> solidRS;
 ComPtr<ID3D11RasterizerState> solidCCWRS; // Counter-ClockWise
 ComPtr<ID3D11RasterizerState> wireRS;
 ComPtr<ID3D11RasterizerState> wireCCWRS;


// Depth Stencil States
 ComPtr<ID3D11DepthStencilState> drawDSS;		  // 일반적으로 그리기
 ComPtr<ID3D11DepthStencilState> maskDSS;       // 스텐실버퍼에 표시
 ComPtr<ID3D11DepthStencilState> drawMaskedDSS; // 스텐실 표시된 곳만


// Shaders
 ComPtr<ID3D11VertexShader> basicVS;
 ComPtr<ID3D11PixelShader> basicPS;

 ComPtr<ID3D11VertexShader> skyboxVS;
 ComPtr<ID3D11PixelShader> skyboxPS;


// Input Layouts
 ComPtr<ID3D11InputLayout> basicIL;
 ComPtr<ID3D11InputLayout> skyboxIL;

// Blend States
ComPtr<ID3D11BlendState> mirrorBS;


// Graphics Pipeline States
 GraphicsPSO defaultSolidPSO;
 GraphicsPSO defaultWirePSO;

 GraphicsPSO skyboxSolidPSO;
 GraphicsPSO skyboxWirePSO;
}

void Graphics::InitAllStates(ComPtr<ID3D11Device>& device)
{
	InitSamplers(device);
	InitShaders(device);
	InitRasterizerStates(device);
	InitDepthStencilStates(device);
	InitBlendStates(device);
	InitPipelineStates(device);
}

void Graphics::InitSamplers(ComPtr<ID3D11Device>& device)
{
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	device->CreateSamplerState(&sampDesc, linearWrapSS.GetAddressOf());

	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	device->CreateSamplerState(&sampDesc, linearClampSS.GetAddressOf());


	//"common.hlsli"의 정의 순서와 같아야함 
	sampleStates.push_back(linearWrapSS.Get());
	sampleStates.push_back(linearClampSS.Get());

}

void Graphics::InitRasterizerStates(ComPtr<ID3D11Device>& device)
{
	// Rasterizer States
	D3D11_RASTERIZER_DESC rastDesc;
	ZeroMemory(&rastDesc, sizeof(D3D11_RASTERIZER_DESC));
	rastDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
	rastDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_BACK;
	rastDesc.FrontCounterClockwise = false;
	rastDesc.DepthClipEnable = true;
	rastDesc.MultisampleEnable = true;
	ThrowIfFailed(device->CreateRasterizerState(&rastDesc, solidRS.GetAddressOf()));


	// 거울에 반사되면 삼각형의 Winding이 바뀌기 때문에 CCW로 그려야함
	rastDesc.FrontCounterClockwise = true;
	ThrowIfFailed(device->CreateRasterizerState(&rastDesc, solidCCWRS.GetAddressOf()));

	rastDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_WIREFRAME;
	ThrowIfFailed(device->CreateRasterizerState(&rastDesc, wireCCWRS.GetAddressOf()));

	rastDesc.FrontCounterClockwise = false;
	ThrowIfFailed(device->CreateRasterizerState(&rastDesc, wireRS.GetAddressOf()));
}

void Graphics::InitBlendStates(ComPtr<ID3D11Device>& device)
{
	// "이미 그려져있는 화면"과 어떻게 섞을지를 결정
	// Dest: 이미 그려져 있는 값들을 의미
	// Src: 픽셀 쉐이더가 계산한 값들을 의미 (여기서는 마지막 거울)

	D3D11_BLEND_DESC mirrorBlendDesc;
	ZeroMemory(&mirrorBlendDesc, sizeof(mirrorBlendDesc));
	mirrorBlendDesc.AlphaToCoverageEnable = true; // MSAA
	mirrorBlendDesc.IndependentBlendEnable = false;

	// 개별 RenderTarget에 대해서 설정 (최대 8개)
	mirrorBlendDesc.RenderTarget[0].BlendEnable = true;
	mirrorBlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_BLEND_FACTOR;
	mirrorBlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_BLEND_FACTOR;
	mirrorBlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;

	mirrorBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	mirrorBlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	mirrorBlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

	// 필요하면 RGBA 각각에 대해서도 조절 가능
	mirrorBlendDesc.RenderTarget[0].RenderTargetWriteMask =
		D3D11_COLOR_WRITE_ENABLE_ALL;

	ThrowIfFailed(device->CreateBlendState(&mirrorBlendDesc, mirrorBS.GetAddressOf()));
}


void Graphics::InitDepthStencilStates(ComPtr<ID3D11Device>& device)
{   
	// StencilPassOp : 둘 다 pass일 때 할 일
	// StencilDepthFailOp : Stencil pass, Depth fail 일 때 할 일
	// StencilFailOp : 둘 다 fail 일 때 할 일


	// m_drawDSS: 기본 DSS
	D3D11_DEPTH_STENCIL_DESC dsDesc;
	ZeroMemory(&dsDesc, sizeof(dsDesc));
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	dsDesc.StencilEnable = false; // Stencil 불필요
	dsDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	dsDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	// 앞면에 대해서 어떻게 작동할지 설정
	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	// 뒷면에 대해 어떻게 작동할지 설정 (뒷면도 그릴 경우)
	dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	ThrowIfFailed(device->CreateDepthStencilState(&dsDesc, drawDSS.GetAddressOf()));


	// Stencil에 1로 표기해주는 DSS
	dsDesc.DepthEnable = true; // 이미 그려진 물체 유지
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	dsDesc.StencilEnable = true;    // Stencil 필수
	dsDesc.StencilReadMask = 0xFF;  // 모든 비트 다 사용
	dsDesc.StencilWriteMask = 0xFF; // 모든 비트 다 사용
	// 앞면에 대해서 어떻게 작동할지 설정
	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	ThrowIfFailed(device->CreateDepthStencilState(&dsDesc, maskDSS.GetAddressOf()));


	// Stencil에 1로 표기된 경우에"만" 그리는 DSS
	// DepthBuffer는 초기화된 상태로 가정
	// D3D11_COMPARISON_EQUAL 이미 1로 표기된 경우에만 그리기
	// OMSetDepthStencilState(..., 1); <- 여기의 1
	dsDesc.DepthEnable = true;   // 거울 속을 다시 그릴때 필요
	dsDesc.StencilEnable = true; // Stencil 사용
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL; // <- 주의
	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;

	ThrowIfFailed(device->CreateDepthStencilState(&dsDesc, drawMaskedDSS.GetAddressOf()));
}

void Graphics::InitShaders(ComPtr<ID3D11Device>& device)
{
	vector<D3D11_INPUT_ELEMENT_DESC> basicIEs = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
		 D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,
		 D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24,
		 D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32,
		 D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	Utility::CreateVertexShaderAndInputLayout(device,
		L"../DirectX_Nogun/BasicVS.hlsl", basicIEs, basicVS, basicIL);
	Utility::CreatePixelShader(device, L"../DirectX_Nogun/BasicPS.hlsl", basicPS);

	Utility::CreateVertexShaderAndInputLayout(device,
		L"../DirectX_Nogun/SkyboxVS.hlsl", basicIEs, skyboxVS, skyboxIL);
	Utility::CreatePixelShader(device, L"../DirectX_Nogun/SkyboxPS.hlsl", skyboxPS);

}


void Graphics::InitPipelineStates(ComPtr<ID3D11Device>& device)
{
	// default Solid
	defaultSolidPSO.m_vertexShader = basicVS;
	defaultSolidPSO.m_inputLayout = basicIL;
	defaultSolidPSO.m_pixelShader = basicPS;
	defaultSolidPSO.m_rasterizerState = solidRS;
	defaultSolidPSO.m_primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	defaultSolidPSO.m_depthStencilState = drawDSS;
	defaultSolidPSO.m_hullShader = nullptr;
	defaultSolidPSO.m_domainShader = nullptr;
	defaultSolidPSO.m_geometryShader = nullptr;
	defaultSolidPSO.m_blendState = nullptr;

	// default Wire
	defaultWirePSO = defaultSolidPSO;
	defaultWirePSO.m_rasterizerState = wireRS;


	// skyboxSolidPSO
	skyboxSolidPSO = defaultSolidPSO;
	skyboxSolidPSO.m_vertexShader = skyboxVS;
	skyboxSolidPSO.m_pixelShader = skyboxPS;
	skyboxSolidPSO.m_inputLayout = skyboxIL;

	// skyboxWirePSO
	skyboxWirePSO = skyboxSolidPSO;
	skyboxWirePSO.m_rasterizerState = wireRS;


}
