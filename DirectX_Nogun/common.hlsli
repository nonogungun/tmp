#ifndef __COMMON_HLSLI__
#define __COMMON_HLSLI__

#define MAX_LIGHTS 3 
#define NUM_DIR_LIGHTS 1
#define NUM_POINT_LIGHTS 1
#define NUM_SPOT_LIGHTS 1


// 공통 Sampler
SamplerState linearWrapSampler : register(s0);
SamplerState linearClampSampler : register(s1);


// 공용 texture t10 부터 시작
TextureCube envIBLTex : register(t10);
TextureCube specularIBLTex : register(t11);
TextureCube irradianceIBLTex : register(t12);
Texture2D brdfTex : register(t13);


// 재질
struct Material
{
    float3 ambient;
    float shininess;
    float3 diffuse;
    float dummy1; // 16 bytes 맞춰주기 위해 추가
    float3 specular;
    float dummy2;
};

// 조명
struct Light
{
    float3 strength;
    float fallOffStart;
    float3 direction;
    float fallOffEnd;
    float3 position;
    float spotPower;
};

// 공용 Constants 
cbuffer GlobalConstants : register(b1)
{
    matrix view;
    matrix proj;
    matrix invProj; // 역프로젝션행렬 for mirror
    matrix viewProj;
    
    float3 eyeWorld;
    float dummy;
    //int textureToDraw = 0; // 0: Env, 1: Specular, 2: Irradiance, 그외: 검은색
    
    Light light[MAX_LIGHTS];
};


struct VertexInputType
{
    float4 position : POSITION;
    float3 normalModel : NORMAL0;
    float2 texcoord : TEXCOORD0;
    float3 tangentModel : TANGENT0;
};

struct PixelInputType
{
    float4 posProj : SV_POSITION; // Screen 좌표
    float3 posWorld : POSITION;   // World 좌표 
    float3 normalWorld : NORMAL;
    float2 texcoord : TEXCOORD;
};


float3 BlinnPhong(float3 lightStrength, float3 lightVec, float3 normal,
                   float3 toEye, Material mat)
{
    float3 halfway = normalize(toEye + lightVec);
    float hdotn = dot(halfway, normal);
    float3 specular = mat.specular * pow(max(hdotn, 0.0f), mat.shininess);

    return mat.ambient + (mat.diffuse + specular) * lightStrength;
}


float3 ComputeDirectionalLight(Light L, Material mat, float3 normal, float3 toEye)
{
    float3 lightVec = -L.direction;

    float ndotl = max(dot(lightVec, normal), 0.0f);
    float3 lightStrength = L.strength * ndotl;

    // Specular 계산에도 Lambert's law가 적용된 lightStrength를 사용합니다.
    return BlinnPhong(lightStrength, lightVec, normal, toEye, mat);
}


float CalcAttenuation(float d, float falloffStart, float falloffEnd)
{
    // Linear falloff
    return saturate((falloffEnd - d) / (falloffEnd - falloffStart));
}

float3 ComputePointLight(Light L, Material mat, float3 pos, float3 normal,
                          float3 toEye)
{
    float3 lightVec = L.position - pos;

    // 쉐이딩할 지점부터 조명까지의 거리 계산
    float d = length(lightVec);

    // 너무 멀면 조명이 적용되지 않음
    if (d > L.fallOffEnd)
    {
        return float3(0.0, 0.0, 0.0);
    }
    else
    {
        lightVec /= d;

        float ndotl = max(dot(lightVec, normal), 0.0f);
        float3 lightStrength = L.strength * ndotl;

        float att = CalcAttenuation(d, L.fallOffStart, L.fallOffEnd);
        lightStrength *= att;

        return BlinnPhong(lightStrength, lightVec, normal, toEye, mat);
    }
}

float3 ComputeSpotLight(Light L, Material mat, float3 pos, float3 normal,
                         float3 toEye)
{
    float3 lightVec = L.position - pos;

    // 쉐이딩할 지점부터 조명까지의 거리 계산
    float d = length(lightVec);

    // 너무 멀면 조명이 적용되지 않음
    if (d > L.fallOffEnd)
    {
        return float3(0.0f, 0.0f, 0.0f);
    }
    else
    {
        lightVec /= d;

        float ndotl = max(dot(lightVec, normal), 0.0f);
        float3 lightStrength = L.strength * ndotl;

        float att = CalcAttenuation(d, L.fallOffStart, L.fallOffEnd);
        lightStrength *= att;

        float spotFactor = pow(max(-dot(lightVec, L.direction), 0.0f), L.spotPower);
        lightStrength *= spotFactor;

        return BlinnPhong(lightStrength, lightVec, normal, toEye, mat);
    }
    
    // if에 else가 없을 경우 경고 발생
    // warning X4000: use of potentially uninitialized variable
}













#endif