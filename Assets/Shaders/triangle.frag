Texture2D mainTex : register(t1);
SamplerState mainTexSampler : register(s1);

struct PS_INPUT {
    float3 fragColor : COLOR0;
    float2 uv : TEXCOORD0;
};

float4 main(PS_INPUT input):SV_TARGET {
    float4 output;
    output = float4(mainTex.Sample(mainTexSampler, input.uv).rgb,1.0);
    //output = float4(input.uv,0,1);
    return pow(output,1/2.2);
}