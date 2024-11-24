#pragma once

#define MAX_LIGHTS 3

#include "header.h"

using Microsoft::WRL::ComPtr;
using std::vector;
using namespace DirectX;


struct VertexConstant
{
    XMMATRIX world;
    XMMATRIX worldIT; //Inverse Transpose
};

struct Mesh {
    ComPtr<ID3D11Buffer> vertexBuffer;
    ComPtr<ID3D11Buffer> indexBuffer;

    //ComPtr<ID3D11Buffer> vertexConstBuffer;
    //ComPtr<ID3D11Buffer> pixelConstBuffer;


    ComPtr<ID3D11Texture2D> albedoTexture;
    //ComPtr<ID3D11Texture2D> emissiveTexture;
    //ComPtr<ID3D11Texture2D> normalTexture;
    //ComPtr<ID3D11Texture2D> heightTexture;
    //ComPtr<ID3D11Texture2D> aoTexture;
    //ComPtr<ID3D11Texture2D> metallicRoughnessTexture;

    ComPtr<ID3D11ShaderResourceView> albedoSRV;
    //ComPtr<ID3D11ShaderResourceView> emissiveSRV;
    //ComPtr<ID3D11ShaderResourceView> normalSRV;
    //ComPtr<ID3D11ShaderResourceView> heightSRV;
    //ComPtr<ID3D11ShaderResourceView> aoSRV;
    //ComPtr<ID3D11ShaderResourceView> metallicRoughnessSRV;

    UINT indexCount = 0; // Number of indiecs = 3 * number of triangles
    UINT vertexCount = 0;
    UINT stride = 0;
    UINT offset = 0;
};

// Vertex 구조 
struct VertexType
{
    XMFLOAT3 position;
    XMFLOAT3 normal;
    XMFLOAT2 texture;
    XMFLOAT3 tangent;
};

// Mesh에게 건내줄 Data
struct MeshData {
    std::vector<VertexType> vertices;
    std::vector<uint32_t> indices;

    std::string albedoTextureFilename;
    std::string emissiveTextureFilename;
    std::string normalTextureFilename;
    std::string heightTextureFilename;
    std::string aoTextureFilename; 
    std::string metallicTextureFilename;
    std::string roughnessTextureFilename;
};


struct Light {
    XMFLOAT3 strength = XMFLOAT3(1.0f, 1.0f, 1.0f);  // 12
    float fallOffStart = 0.0f;                       // 4
    XMFLOAT3 direction = XMFLOAT3(0.0f, -1.0f, 0.0f); // 12 , 빛의 방향
    float fallOffEnd = 10.0f;                        // 4
    XMFLOAT3 position = XMFLOAT3(1.0f, 1.0f, -1.0f); // 12
    float spotPower = 1.0f;                          // 4
};

// register(b1) 사용
__declspec(align(256)) struct GlobalConstants {
    XMMATRIX view;
    XMMATRIX proj;
    XMMATRIX invProj; 
    XMMATRIX viewProj;

    XMFLOAT3 eyeWorld;
    float strengthIBL = 0.0f;

    //int textureToDraw = 0; // 0: Env, 1: Specular, 2: Irradiance, 그외: 검은색

    Light lights[MAX_LIGHTS];	  // 48 * MAX_LIGHTS
};


//pixel 쉐이더 constant 
struct Material {
    XMFLOAT3 ambient = XMFLOAT3(0.1f, 0.1f, 0.1f);  // 12
    float shininess = 10.0f;							// 4
    XMFLOAT3 diffuse = XMFLOAT3(0.1f, 0.1f, 0.1f);  // 12
    float dummy1;									// 4
    XMFLOAT3 specular = XMFLOAT3(1.0f, 1.0f, 1.0f); // 12
    float dummy2;									// 4
};


struct PixelConstant
{
    Material material;			  // 48
};
