#include "GraphicsCommon.h"


// extern : �� �� �� ����
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
 ComPtr<ID3D11DepthStencilState> drawDSS;		  // �Ϲ������� �׸���
 ComPtr<ID3D11DepthStencilState> maskDSS;       // ���ٽǹ��ۿ� ǥ��
 ComPtr<ID3D11DepthStencilState> drawMaskedDSS; // ���ٽ� ǥ�õ� ����


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


	//"common.hlsli"�� ���� ������ ���ƾ��� 
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


	// �ſ￡ �ݻ�Ǹ� �ﰢ���� Winding�� �ٲ�� ������ CCW�� �׷�����
	rastDesc.FrontCounterClockwise = true;
	ThrowIfFailed(device->CreateRasterizerState(&rastDesc, solidCCWRS.GetAddressOf()));

	rastDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_WIREFRAME;
	ThrowIfFailed(device->CreateRasterizerState(&rastDesc, wireCCWRS.GetAddressOf()));

	rastDesc.FrontCounterClockwise = false;
	ThrowIfFailed(device->CreateRasterizerState(&rastDesc, wireRS.GetAddressOf()));
}

void Graphics::InitBlendStates(ComPtr<ID3D11Device>& device)
{
	// "�̹� �׷����ִ� ȭ��"�� ��� �������� ����
	// Dest: �̹� �׷��� �ִ� ������ �ǹ�
	// Src: �ȼ� ���̴��� ����� ������ �ǹ� (���⼭�� ������ �ſ�)

	D3D11_BLEND_DESC mirrorBlendDesc;
	ZeroMemory(&mirrorBlendDesc, sizeof(mirrorBlendDesc));
	mirrorBlendDesc.AlphaToCoverageEnable = true; // MSAA
	mirrorBlendDesc.IndependentBlendEnable = false;

	// ���� RenderTarget�� ���ؼ� ���� (�ִ� 8��)
	mirrorBlendDesc.RenderTarget[0].BlendEnable = true;
	mirrorBlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_BLEND_FACTOR;
	mirrorBlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_BLEND_FACTOR;
	mirrorBlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;

	mirrorBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	mirrorBlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	mirrorBlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

	// �ʿ��ϸ� RGBA ������ ���ؼ��� ���� ����
	mirrorBlendDesc.RenderTarget[0].RenderTargetWriteMask =
		D3D11_COLOR_WRITE_ENABLE_ALL;

	ThrowIfFailed(device->CreateBlendState(&mirrorBlendDesc, mirrorBS.GetAddressOf()));
}


void Graphics::InitDepthStencilStates(ComPtr<ID3D11Device>& device)
{   
	// StencilPassOp : �� �� pass�� �� �� ��
	// StencilDepthFailOp : Stencil pass, Depth fail �� �� �� ��
	// StencilFailOp : �� �� fail �� �� �� ��


	// m_drawDSS: �⺻ DSS
	D3D11_DEPTH_STENCIL_DESC dsDesc;
	ZeroMemory(&dsDesc, sizeof(dsDesc));
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	dsDesc.StencilEnable = false; // Stencil ���ʿ�
	dsDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	dsDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	// �ո鿡 ���ؼ� ��� �۵����� ����
	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	// �޸鿡 ���� ��� �۵����� ���� (�޸鵵 �׸� ���)
	dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	ThrowIfFailed(device->CreateDepthStencilState(&dsDesc, drawDSS.GetAddressOf()));


	// Stencil�� 1�� ǥ�����ִ� DSS
	dsDesc.DepthEnable = true; // �̹� �׷��� ��ü ����
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	dsDesc.StencilEnable = true;    // Stencil �ʼ�
	dsDesc.StencilReadMask = 0xFF;  // ��� ��Ʈ �� ���
	dsDesc.StencilWriteMask = 0xFF; // ��� ��Ʈ �� ���
	// �ո鿡 ���ؼ� ��� �۵����� ����
	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	ThrowIfFailed(device->CreateDepthStencilState(&dsDesc, maskDSS.GetAddressOf()));


	// Stencil�� 1�� ǥ��� ��쿡"��" �׸��� DSS
	// DepthBuffer�� �ʱ�ȭ�� ���·� ����
	// D3D11_COMPARISON_EQUAL �̹� 1�� ǥ��� ��쿡�� �׸���
	// OMSetDepthStencilState(..., 1); <- ������ 1
	dsDesc.DepthEnable = true;   // �ſ� ���� �ٽ� �׸��� �ʿ�
	dsDesc.StencilEnable = true; // Stencil ���
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL; // <- ����
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
