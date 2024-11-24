#pragma once

#include "GraphicsPSO.h"

namespace Graphics {

// Samplers
extern ComPtr<ID3D11SamplerState> linearWrapSS;
extern ComPtr<ID3D11SamplerState> linearClampSS;
//extern ComPtr<ID3D11SamplerState> shadowPointSS;
//extern ComPtr<ID3D11SamplerState> shadowCompareSS;
extern vector<ID3D11SamplerState*> sampleStates;

// Rasterizer States
extern ComPtr<ID3D11RasterizerState> solidRS;
extern ComPtr<ID3D11RasterizerState> solidCCWRS; // Counter-ClockWise
extern ComPtr<ID3D11RasterizerState> wireRS;
extern ComPtr<ID3D11RasterizerState> wireCCWRS;


// Depth Stencil States
extern ComPtr<ID3D11DepthStencilState> drawDSS;		  // �Ϲ������� �׸���
extern ComPtr<ID3D11DepthStencilState> maskDSS;       // ���ٽǹ��ۿ� ǥ��
extern ComPtr<ID3D11DepthStencilState> drawMaskedDSS; // ���ٽ� ǥ�õ� ����


// Shaders
extern ComPtr<ID3D11VertexShader> basicVS;
extern ComPtr<ID3D11PixelShader> basicPS;

extern ComPtr<ID3D11VertexShader> skyboxVS;
extern ComPtr<ID3D11PixelShader> skyboxPS;


// Input Layouts
extern ComPtr<ID3D11InputLayout> basicIL;
extern ComPtr<ID3D11InputLayout> skyboxIL;


// Blend States
extern ComPtr<ID3D11BlendState> mirrorBS;


// Graphics Pipeline States
extern GraphicsPSO defaultSolidPSO;
extern GraphicsPSO defaultWirePSO;

extern GraphicsPSO skyboxSolidPSO;
extern GraphicsPSO skyboxWirePSO;



void InitAllStates(ComPtr<ID3D11Device>& device);
//  InitAllStates()���� ��� ȣ�� 
void InitSamplers(ComPtr<ID3D11Device>& device);
void InitRasterizerStates(ComPtr<ID3D11Device>& device);
void InitBlendStates(ComPtr<ID3D11Device>& device);
void InitDepthStencilStates(ComPtr<ID3D11Device>& device);
void InitPipelineStates(ComPtr<ID3D11Device>& device);
void InitShaders(ComPtr<ID3D11Device>& device);

}