#include "Tmp.h"
#include <vector>
#include "Utility.h"

#define device m_Display->m_device 
#define context m_Display->m_context 


void Tmp::SetClass(shared_ptr<Camera> cam,
    shared_ptr<KeyManager> key, 
    shared_ptr<Display> dis)
{
    m_camera = cam;
    m_input = key;
    m_Display = dis;
}

void Tmp::Initialize(int width, int height, HWND hwnd)
{
    //임시로 Vertex, Pixel Shader, Sampler 공통으로 사용 
    vector<D3D11_INPUT_ELEMENT_DESC> LayOutElements = {
         {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT , 0, 0,
          D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 4 * 3,
          D3D11_INPUT_PER_VERTEX_DATA, 0},
         {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12 + 12,
          D3D11_INPUT_PER_VERTEX_DATA, 0}
    };
    Utility::CreateVertexShaderAndInputLayout(device, L"../DirectX_Nogun/VertexShader.hlsl"
        , LayOutElements, m_vertexshader, m_layout);
    Utility::CreatePixelShader(device, L"../DirectX_Nogun/PixelShader.hlsl", m_pixelshader);

    // Texture sampler 만들기
    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory(&sampDesc, sizeof(sampDesc));
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    m_Display->m_device->CreateSamplerState(&sampDesc, &m_samplerState);

    auto mesh = MeshLoader::MakeBox(1.0);
    mesh.albedoTextureFilename = "cat.png";
    m_object = make_shared<Object>(device, context ,vector{mesh});

}

void Tmp::Update()
{
    if (m_input->m_keys[87])
        m_camera->MoveForward(0.01);
    if (m_input->m_keys[83])
        m_camera->MoveForward(-0.01);
    if (m_input->m_keys[68])
        m_camera->MoveRight(0.01);
    if (m_input->m_keys[65])
        m_camera->MoveRight(-0.01);
    //if (m_input->m_keys[70]) m_drawWire = !m_drawWire; // f키 

    XMMATRIX world, view, proj;
    world = XMMatrixIdentity();
    view = m_camera->GetViewRow();
    proj = m_camera->GetProjRow();

    world = XMMatrixTranspose(world);
    view = XMMatrixTranspose(view);
    proj = XMMatrixTranspose(proj);


    //내 옛날 코드는 0번째 const를 공통으로 사용했는데
    //for문 돌면서 다 update해줘야하나? 
    m_object->m_VertexCnstCPU.view = view;
    m_object->m_VertexCnstCPU.projection = proj;
    Utility::UpdateBuffer(device, context,
        m_object->m_VertexCnstCPU, m_object->m_VertexCnstGPU);

}

void Tmp::Render()
{
    float clearColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    m_Display->m_context->ClearRenderTargetView(m_Display->m_backBufferRTV.Get(), clearColor);
    m_Display->m_context->ClearDepthStencilView(m_Display->m_DepthStencilView.Get(),
        D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    //Init3D()에서 이미 적용했는데 강의 코드는 계속 호출하고 있다.
    m_Display->m_context->OMSetRenderTargets(1, m_Display->m_backBufferRTV.GetAddressOf(),
        m_Display->m_DepthStencilView.Get());
    m_Display->m_context->OMSetDepthStencilState(m_Display->m_DepthStencilState.Get(), 0);

    //if (m_drawWire) m_context->RSSetState(m_wireRasterState);
    //else m_context->RSSetState(m_solidRasterState);
    // Set vertex buffer stride and offset. 이건 어따쓰는거지 나중에 찾아보자 ㅎ
    //unsigned int stride = sizeof(VertexType);
    //unsigned int offset = 0;

    m_Display->m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_Display->m_context->IASetInputLayout(m_layout.Get());

    m_Display->m_context->VSSetShader(m_vertexshader.Get(), NULL, 0);
    m_Display->m_context->PSSetShader(m_pixelshader.Get(), NULL, 0);
    m_Display->m_context->PSSetSamplers(0, 1, &m_samplerState);


    //object 클래스 
    m_object->Render(context);

}

