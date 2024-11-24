#pragma once


// 다용도 : 계속 반복되는 기능 몰아넣기
#ifndef _UTIL_
#define _UTIL_

#include "header.h"

inline void ThrowIfFailed(HRESULT hr) {
    if (FAILED(hr)) {
        // 디버깅할 때 여기에 breakpoint 설정
        throw std::exception();
    }
}

using Microsoft::WRL::ComPtr;
using std::wstring;
using std::shared_ptr;
using std::vector;

class Utility
{
public:

    template <typename T_VERTEX>
    static void CreateVertexBuffer(ComPtr<ID3D11Device>& device,
        const vector<T_VERTEX>& vertices,
        ComPtr<ID3D11Buffer>& vertexBuffer) {
        D3D11_BUFFER_DESC bufferDesc;
        ZeroMemory(&bufferDesc, sizeof(bufferDesc));
        bufferDesc.Usage = D3D11_USAGE_IMMUTABLE; // 초기화 후 변경X
        bufferDesc.ByteWidth = UINT(sizeof(T_VERTEX) * vertices.size());
        bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bufferDesc.CPUAccessFlags = 0; // 0 if no CPU access is necessary.
        bufferDesc.StructureByteStride = sizeof(T_VERTEX);

        D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 }; 
        vertexBufferData.pSysMem = vertices.data();
        vertexBufferData.SysMemPitch = 0;
        vertexBufferData.SysMemSlicePitch = 0;

        ThrowIfFailed(device->CreateBuffer(&bufferDesc, &vertexBufferData,
            vertexBuffer.GetAddressOf()));
    }

    static void CreateIndexBuffer(ComPtr<ID3D11Device>& device,
        const vector<uint32_t>& indices,
        ComPtr<ID3D11Buffer>& indexBuffer);



    static void CreateVertexShaderAndInputLayout(
        ComPtr<ID3D11Device>& device, const wstring& filename,
        const vector<D3D11_INPUT_ELEMENT_DESC>& inputElements,
        ComPtr<ID3D11VertexShader>& m_vertexShader,
        ComPtr<ID3D11InputLayout>& m_inputLayout);



    static void CreatePixelShader(ComPtr<ID3D11Device>& device,
        const wstring& filename,
        ComPtr<ID3D11PixelShader>& m_pixelShader);


    template <typename T_CONSTANT>
    static void CreateConstBuffer(ComPtr<ID3D11Device>& device,
        const T_CONSTANT& constantBufferData,
        ComPtr<ID3D11Buffer>& constantBuffer) {

        static_assert((sizeof(T_CONSTANT) % 16) == 0,
            "Constant Buffer size must be 16-byte aligned");

        D3D11_BUFFER_DESC desc;
        ZeroMemory(&desc, sizeof(desc));
        desc.ByteWidth = sizeof(constantBufferData);
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        desc.MiscFlags = 0;
        desc.StructureByteStride = 0;

        D3D11_SUBRESOURCE_DATA initData;
        ZeroMemory(&initData, sizeof(initData));
        initData.pSysMem = &constantBufferData;
        initData.SysMemPitch = 0;
        initData.SysMemSlicePitch = 0;

        ThrowIfFailed(device->CreateBuffer(&desc, &initData,
            constantBuffer.GetAddressOf()));
    }


    
    template <typename TEMPLATE_UPDATE_CNST>
    static void UpdateBuffer(ComPtr<ID3D11Device>& device,
        ComPtr<ID3D11DeviceContext>& context,
        const TEMPLATE_UPDATE_CNST& bufferData,
        ComPtr<ID3D11Buffer>& buffer) {

        if (!buffer) {
            std::cout << "UpdateBuffer() : There's no Buffer" << std::endl;
        }

        D3D11_MAPPED_SUBRESOURCE ms;
        context->Map(buffer.Get(), NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
        memcpy(ms.pData, &bufferData, sizeof(bufferData));
        context->Unmap(buffer.Get(), NULL);
    }


    static void CreateTexture(ComPtr<ID3D11Device>& device,
            ComPtr<ID3D11DeviceContext>& context,
            const std::string filename, const bool usSRGB,
            ComPtr<ID3D11Texture2D>& texture,
            ComPtr<ID3D11ShaderResourceView>& textureResourceView);

    static void CreateTextureArray(ComPtr<ID3D11Device>& device,
            ComPtr<ID3D11DeviceContext>& context,
            const std::vector<std::string> filenames,
            ComPtr<ID3D11Texture2D>& texture,
            ComPtr<ID3D11ShaderResourceView>& textureResourceView);



    static void CreateDDSTexture(
        ComPtr<ID3D11Device>& device, const wchar_t* filename, bool isCubeMap,
        ComPtr<ID3D11ShaderResourceView>& textureResourceView);


};


#endif