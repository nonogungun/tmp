#include "common.hlsli"

Texture2D g_tex : register(t0);

cbuffer BasicPixelConstantBuffer : register(b0)
{
    Material material;
};

float4 main(PixelInputType input) : SV_TARGET
{
    
    float3 toEye = normalize(eyeWorld - input.posWorld);
    
    float3 color = float3(0.0, 0.0, 0.0);
   
    int i = 0;
    
    [unroll] 
    for (i = 0; i < NUM_DIR_LIGHTS; ++i)
    {
        color += ComputeDirectionalLight(light[i], material, input.normalWorld, toEye);
    }

    [unroll]
    for (i = NUM_DIR_LIGHTS; i < NUM_DIR_LIGHTS + NUM_POINT_LIGHTS;
                  ++i)
    {
        color += ComputePointLight(light[i], material, input.posWorld,
                                   input.normalWorld, toEye);
    }

    [unroll]
    for (i = NUM_DIR_LIGHTS + NUM_POINT_LIGHTS;
                  i < NUM_DIR_LIGHTS + NUM_POINT_LIGHTS + NUM_SPOT_LIGHTS;
                  ++i)
    {
        color += ComputeSpotLight(light[i], material, input.posWorld, input.normalWorld, toEye);
    }
    
    
    return float4(color, 1.0) * g_tex.SampleLevel(linearWrapSampler, input.texcoord, 0);
    
    /* IBL 
    float4 diffuse = g_DiffuseCube.Sample(g_sampler, input.normalWorld);
    float4 specular = g_SpecularCube.Sample(g_sampler, reflect(-toEye, input.normalWorld));
    
    diffuse.xyz *= material.diffuse;
    specular.xyz *= material.specular;
    diffuse.xyz *= g_tex.Sample(g_sampler, input.texcoord);
    specular.xyz *= g_tex.Sample(g_sampler, input.texcoord);
    
    return diffuse + specular;
    */
}