#include "lygia/generative/random.glsl"
#include "lygia/math/const.glsl" 

vec4 basis_from_angle( float a )
{
    vec2 sc = vec2(sin(a),cos(a)); //note: sincos on hlsl

    vec4 ret;
    ret.xy = vec2(  sc.y, sc.x ); //p = vec2(1,0); vec2(cos,sin)
    ret.zw = vec2( -sc.x, sc.y ); //p = vec2(0,1); vec2(-sin,cos)

    return ret;
}

vec4 PoissonDiscSample(vec2 uv,float radius, sampler2D tex) {
    const int NUM_TAPS = 18;
    const vec2 fTaps_Poisson[NUM_TAPS]
    = vec2[NUM_TAPS](vec2(-0.220147, 0.976896),
                     vec2(-0.735514, 0.693436),
                     vec2(-0.200476, 0.310353),
                     vec2(0.180822, 0.454146),
                     vec2(0.292754, 0.937414),
                     vec2(0.564255, 0.207879),
                     vec2(0.178031, 0.024583),
                     vec2(0.613912, -0.205936),
                     vec2(-0.385540, -0.070092),
                     vec2(0.962838, 0.378319),
                     vec2(-0.886362, 0.032122),
                     vec2(-0.466531, -0.741458),
                     vec2(0.006773, -0.574796),
                     vec2(-0.739828, -0.410584),
                     vec2(0.590785, -0.697557),
                     vec2(-0.081436, -0.963262),
                     vec2(1.000000, -0.100160),
                     vec2(0.622430, 0.680868));

        float rng = random(i) * TAU;
        vec4 basis = basis_from_angle(rng);
    
    vec4 sum = vec4(0.);
    
    for (int i = 0; i < NUM_TAPS; i++){
        vec2 offset = ftaps_poisson[i];
        offset = vec2(dot(offset,basis.xy), dot(offset,basis.zw));
        
        uv = uv + offset*radius;
        
        sum+= texture(tex,uv);
    }
    
    return sum;
}