#include "common.hlsli"

struct SkyboxPixelInputType
{
    float4 posProj : SV_POSITION;
    float3 posModel : POSITION;
};

SkyboxPixelInputType main(VertexInputType input)
{
    SkyboxPixelInputType output;
   
    output.posModel = input.position;
    output.posProj = mul(float4(input.position.xyz, 0.0), view); 
    output.posProj = mul(float4(output.posProj.xyz, 1.0), proj);
   
    return output;
}