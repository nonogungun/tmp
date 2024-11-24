#include "common.hlsli"

cbuffer VertexConstantData : register(b0)
{
    matrix world;
    matrix worldIT; 
}

PixelInputType main(VertexInputType input)
{
    PixelInputType output;
    
    //world 
    input.position.w = 1; //float4 -> position.w is empty
    float4 pos = input.position;
    pos = mul(pos, world);
    output.posWorld = pos.xyz;
    pos = mul(pos, view);
    pos = mul(pos, proj);
    output.posProj = pos;
    
    //normal 
    float4 normal = float4(input.normalModel, 0.0f);
    output.normalWorld = mul(normal, worldIT).xyz; 
    output.normalWorld = normalize(output.normalWorld);
    
    //texcoord 
    output.texcoord = input.texcoord;
    
    return output;
}
