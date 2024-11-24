#define _CRT_SECURE_NO_WARNINGS // stb_image_write compile error fix

#include "Utility.h"

//for 큐브맵
#include <directxtk/DDSTextureLoader.h> 

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"


void CheckResult(HRESULT hr, ID3DBlob* errorBlob) {
    if (FAILED(hr)) {
        // 파일이 없을 경우
        if ((hr & D3D11_ERROR_FILE_NOT_FOUND) != 0) {
            std::cout << "File not found." << std::endl;
        }

        // 에러 메시지가 있으면 출력
        if (errorBlob) {
            std::cout << "Shader compile error\n"
                << (char*)errorBlob->GetBufferPointer() << std::endl;
        }
    }
}


void Utility::CreateIndexBuffer(ComPtr<ID3D11Device>& device, 
    const vector<uint32_t>& indices, ComPtr<ID3D11Buffer>& indexBuffer)
{
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_IMMUTABLE; // 초기화 후 변경X
    bufferDesc.ByteWidth = UINT(sizeof(uint32_t) * indices.size());
    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0; // 0 if no CPU access is necessary.
    bufferDesc.StructureByteStride = sizeof(uint32_t);

    D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
    indexBufferData.pSysMem = indices.data();
    indexBufferData.SysMemPitch = 0;
    indexBufferData.SysMemSlicePitch = 0;

    device->CreateBuffer(&bufferDesc, &indexBufferData, indexBuffer.GetAddressOf());
}

void Utility::CreateVertexShaderAndInputLayout(ComPtr<ID3D11Device>& device,
    const wstring& filename, const vector<D3D11_INPUT_ELEMENT_DESC>& inputElements,
    ComPtr<ID3D11VertexShader>& m_vertexShader, ComPtr<ID3D11InputLayout>& m_inputLayout)
{

    ComPtr<ID3DBlob> shaderBlob;
    ComPtr<ID3DBlob> errorBlob;

    UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    // D3D_COMPILE_STANDARD_FILE_INCLUDE : Shader에 Include있을시 
    HRESULT hr = D3DCompileFromFile(
        filename.c_str(), 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main",
        "vs_5_0", compileFlags, 0, &shaderBlob, &errorBlob);

    CheckResult(hr, errorBlob.Get());

    device->CreateVertexShader(shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(), NULL, &m_vertexShader);

    device->CreateInputLayout(inputElements.data(), UINT(inputElements.size()),
        shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), &m_inputLayout);


    // vertexShaderBuffer->Release();
    // vertexShaderBuffer = 0;
}


void Utility::CreatePixelShader(ComPtr<ID3D11Device>& device, const wstring& filename,
    ComPtr<ID3D11PixelShader>& m_pixelShader)
{
    ComPtr<ID3DBlob> shaderBlob;
    ComPtr<ID3DBlob> errorBlob;

    UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    // 쉐이더의 시작점의 이름이 "main"인 함수로 지정
    // D3D_COMPILE_STANDARD_FILE_INCLUDE 추가: 쉐이더에서 include 사용
    HRESULT hr = D3DCompileFromFile(
        filename.c_str(), 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main",
        "ps_5_0", compileFlags, 0, &shaderBlob, &errorBlob);

    CheckResult(hr, errorBlob.Get());

    device->CreatePixelShader(shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(), NULL, &m_pixelShader);

    // pixelShaderBuffer->Release();
    // pixelShaderBuffer = 0;
}

void ReadImage(const std::string filename, std::vector<uint8_t>& image,
    int& width, int& height) {

    int channels;
    unsigned char* img = stbi_load(filename.c_str(), &width, &height, &channels, 0);
    // assert(channels == 4);
    std::cout << filename << " " << width << " " << height << " " << channels << std::endl;

    // 4채널로 통일 -> Format이 맞는게 없음 
    image.resize(width * height * 4);

    if (channels == 1) {
        for (size_t i = 0; i < width * height; i++) {
            uint8_t g = img[i * channels + 0];
            for (size_t c = 0; c < 4; c++) {
                image[4 * i + c] = g;
            }
        }
    }
    else if (channels == 2) {
        for (size_t i = 0; i < width * height; i++) {
            for (size_t c = 0; c < 2; c++) {
                image[4 * i + c] = img[i * channels + c];
            }
            image[4 * i + 2] = 255;
            image[4 * i + 3] = 255;
        }
    }
    else if (channels == 3) {
        for (size_t i = 0; i < width * height; i++) {
            for (size_t c = 0; c < 3; c++) {
                image[4 * i + c] = img[i * channels + c];
            }
            image[4 * i + 3] = 255;
        }
    }
    else if (channels == 4) {
        for (size_t i = 0; i < width * height; i++) {
            for (size_t c = 0; c < 4; c++) {
                image[4 * i + c] = img[i * channels + c];
            }
        }
    }
    else {
        std::cout << "Cannot read " << channels << " channels" << std::endl;
    }
}

//Mipmap을 위한 원본이미지 
ComPtr<ID3D11Texture2D>
CreateStagingTexture(ComPtr<ID3D11Device>& device,
    ComPtr<ID3D11DeviceContext>& context, const int width,
    const int height, const std::vector<uint8_t>& image,
    const DXGI_FORMAT pixelFormat = DXGI_FORMAT_R8G8B8A8_UNORM,
    const int mipLevels = 1, const int arraySize = 1) {

    // 스테이징 텍스춰 만들기
    D3D11_TEXTURE2D_DESC txtDesc;
    ZeroMemory(&txtDesc, sizeof(txtDesc));
    txtDesc.Width = width;
    txtDesc.Height = height;
    txtDesc.MipLevels = mipLevels;
    txtDesc.ArraySize = arraySize;
    txtDesc.Format = pixelFormat;
    txtDesc.SampleDesc.Count = 1;
    txtDesc.Usage = D3D11_USAGE_STAGING;
    txtDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;

    ComPtr<ID3D11Texture2D> stagingTexture;
    if (FAILED(device->CreateTexture2D(&txtDesc, NULL,
        stagingTexture.GetAddressOf()))) {
        std::cout << "Failed()" << std::endl;
    }

    // CPU에서 이미지 데이터 복사
    size_t pixelSize = sizeof(uint8_t) * 4;
    if (pixelFormat == DXGI_FORMAT_R16G16B16A16_FLOAT) {
        pixelSize = sizeof(uint16_t) * 4;
    }

    D3D11_MAPPED_SUBRESOURCE ms;
    context->Map(stagingTexture.Get(), NULL, D3D11_MAP_WRITE, NULL, &ms);
    uint8_t* pData = (uint8_t*)ms.pData;
    for (UINT h = 0; h < UINT(height); h++) { // 가로줄 한 줄씩 복사
        memcpy(&pData[h * ms.RowPitch], &image[h * width * pixelSize],
            width * pixelSize);
    }
    context->Unmap(stagingTexture.Get(), NULL);

    return stagingTexture;
}


void Utility::CreateTexture(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context,
    const std::string filename,  const bool usSRGB, ComPtr<ID3D11Texture2D>& texture, 
    ComPtr<ID3D11ShaderResourceView>& textureResourceView)
{
    int width = 0, height = 0;
    std::vector<uint8_t> image;
    DXGI_FORMAT pixelFormat = usSRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;

    ReadImage(filename, image, width, height);
   
    // 스테이징 텍스춰 만들고 CPU에서 이미지를 복사합니다.
    ComPtr<ID3D11Texture2D> stagingTexture =
        CreateStagingTexture(device, context, width, height, image, pixelFormat);


    // 실제로 사용할 텍스춰 설정
    D3D11_TEXTURE2D_DESC txtDesc;
    ZeroMemory(&txtDesc, sizeof(txtDesc));
    txtDesc.Width = width;
    txtDesc.Height = height;
    txtDesc.MipLevels = 0; // 밉맵 레벨 최대
    txtDesc.ArraySize = 1;
    txtDesc.Format = pixelFormat;
    txtDesc.SampleDesc.Count = 1;
    txtDesc.Usage = D3D11_USAGE_DEFAULT; // 스테이징 텍스춰로부터 복사 가능
    txtDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    txtDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS; // 밉맵 사용
    txtDesc.CPUAccessFlags = 0;

    // 초기 데이터 없이 텍스춰 생성 (전부 검은색)
    device->CreateTexture2D(&txtDesc, NULL, texture.GetAddressOf());

    // 실제로 생성된 MipLevels를 확인해보고 싶을 경우
    // texture->GetDesc(&txtDesc);
    // cout << txtDesc.MipLevels << endl;

    // 스테이징 텍스춰로부터 가장 해상도가 높은 이미지 복사
    context->CopySubresourceRegion(texture.Get(), 0, 0, 0, 0,
        stagingTexture.Get(), 0, NULL);

    // ResourceView 만들기
    device->CreateShaderResourceView(texture.Get(), 0,
        textureResourceView.GetAddressOf());

    // 해상도를 낮춰가며 밉맵 생성
    context->GenerateMips(textureResourceView.Get());

    // HLSL 쉐이더 안에서는 SampleLevel() 사용
}

void Utility::CreateTextureArray(ComPtr<ID3D11Device>& device,
    ComPtr<ID3D11DeviceContext>& context, const std::vector<std::string> filenames, 
    ComPtr<ID3D11Texture2D>& texture, ComPtr<ID3D11ShaderResourceView>& textureResourceView)
{
}

void Utility::CreateDDSTexture(
    ComPtr<ID3D11Device>& device, const wchar_t* filename, bool isCubeMap,
    ComPtr<ID3D11ShaderResourceView>& textureResourceView) {

    ComPtr<ID3D11Texture2D> texture;

    UINT miscFlags = 0;
    if (isCubeMap) {
        miscFlags |= D3D11_RESOURCE_MISC_TEXTURECUBE;
    }

    // https://github.com/microsoft/DirectXTK/wiki/DDSTextureLoader
   DirectX::CreateDDSTextureFromFileEx(device.Get(), filename, 0, D3D11_USAGE_DEFAULT,
        D3D11_BIND_SHADER_RESOURCE, 0, miscFlags, DirectX::DDS_LOADER_FLAGS(false),
        (ID3D11Resource**)texture.GetAddressOf(), textureResourceView.GetAddressOf(), NULL);
}
