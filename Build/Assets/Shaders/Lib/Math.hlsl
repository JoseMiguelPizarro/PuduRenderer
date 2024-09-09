#include "lygia/generative/random.glsl"
#include "lygia/math/const.glsl" 

float4 basis_from_angle( float a )
{
    float2 sc = float2(sin(a),cos(a)); //note: sincos on hlsl

    float4 ret;
    ret.xy = float2(  sc.y, sc.x ); //p = float2(1,0); float2(cos,sin)
    ret.zw = float2( -sc.x, sc.y ); //p = float2(0,1); float2(-sin,cos)

    return ret;
}

float4 PoissonDiscSample(float2 uv,float radius, Sampler2D tex) {
    const int NUM_TAPS = 18;
    const float2 fTaps_Poisson[NUM_TAPS]
    = float[NUM_TAPS](float2(-0.220147, 0.976896),
                     float2(-0.735514, 0.693436),
                     float2(-0.200476, 0.310353),
                     float2(0.180822, 0.454146),
                     float2(0.292754, 0.937414),
                     float2(0.564255, 0.207879),
                     float2(0.178031, 0.024583),
                     float2(0.613912, -0.205936),
                     float2(-0.385540, -0.070092),
                     float2(0.962838, 0.378319),
                     float2(-0.886362, 0.032122),
                     float2(-0.466531, -0.741458),
                     float2(0.006773, -0.574796),
                     float2(-0.739828, -0.410584),
                     float2(0.590785, -0.697557),
                     float2(-0.081436, -0.963262),
                     float2(1.000000, -0.100160),
                     float2(0.622430, 0.680868));

        float rng = random(i) * TAU;
        float4 basis = basis_from_angle(rng);
    
    float4 sum = float4(0.);
    
    for (int i = 0; i < NUM_TAPS; i++){
        float2 offset = ftaps_poisson[i];
        offset = float2(dot(offset,basis.xy), dot(offset,basis.zw));
        
        uv = uv + offset*radius;
        
        sum+= tex.Sample(uv);
    }
    
    return sum;
}