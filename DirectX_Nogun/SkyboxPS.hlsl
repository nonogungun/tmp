#include "common.hlsli"

struct SkyboxPixelInputType
{
    float4 posProj : SV_POSITION;
    float3 posModel : POSITION;
};

struct PixelShaderOutput
{
    float4 pixelColor : SV_Target0;
};

PixelShaderOutput main(SkyboxPixelInputType input)
{
    PixelShaderOutput output;
    
    //if (textureToDraw == 0)
    //    output.pixelColor = envIBLTex.SampleLevel(linearWrapSampler, input.posModel.xyz, envLodBias);
    //else if (textureToDraw == 1)
    //    output.pixelColor = specularIBLTex.SampleLevel(linearWrapSampler, input.posModel.xyz, envLodBias);
    //else if (textureToDraw == 2)
    //    output.pixelColor = irradianceIBLTex.SampleLevel(linearWrapSampler, input.posModel.xyz, envLodBias);
    //else
    //    output.pixelColor = float4(135 / 255, 206 / 255, 235 / 255, 1); // 검은 배경
    
    
    output.pixelColor = envIBLTex.SampleLevel(linearWrapSampler, input.posModel.xyz, 0);
    
    
    //color *= strengthIBL; //음 초기화에서 0.0인데 곱해도 되는건가? 
    
    return output;
    
}