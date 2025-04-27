module PuduCore;

static const float PI = 3.14159265358979f;


public float LinearizeDepth(float d,float zNear,float zFar)
{
    return zNear * zFar / (zFar + d * (zNear - zFar));
}

public float3 DecodeNormal(float2 n)
{
    float2 m = n*2. - 1.;
    var z = sqrt(1. - m.x*m.x - m.y*m.y);

    return normalize(float3(m,z));
}

public float3 TangentToWorld(float3 v, float3 t, float3 bt, float3 n)
{
    return normalize( v.x*t + v.y*bt + v.z*n);
}

public float3 WorldToTangent(float3 v, float3 t, float3 bt, float3 n)
{
    return mul(float3x3(
        t,
        bt,
        n
    ), v);
}

public float2 ParallaxUV(float h, float2 uv, float3 viewDir, float scale, float3 t, float3 bt,float3 n)
{
    float3 tangentView = normalize(WorldToTangent(viewDir,t,bt, n));

    return uv + h*scale*tangentView.xy;
}

public float3 intersectLines3D(float3 p1, float3 d1, float3 p2, float3 d2)
{
    // Find the closest point between two infinite lines using vector algebra
    // p1, p2: points on the lines
    // d1, d2: normalized direction vectors of the lines
    
    float3 n = cross(d1, d2); // Normal vector to both lines
    float3 n2 = cross(d2, n); // Normal to plane containing line 2
    
    // Calculate intersection by projecting from p1 onto line 2's plane. Intersection of a line(L)-plane(P) is given when dot(L,P) = 0
    return p1 - d1 * dot(n2, p1) / dot(n2,d1);
}


public float2 ParallaxOcclusionMapping(Sampler2D heightMap, float2 uv, float3 viewDirTS, float scale) {
    const float minLayers = 32;
    const float maxLayers = 64;
    float numLayers = lerp(maxLayers, minLayers, abs(dot(float3(0.0, 0.0, 1.0), viewDirTS)));
    
    // calculate the size of each layer
    float layerDepth = 1.0 / numLayers;
    // depth of current layer
    float currentLayerDepth = 0.0;
    // the amount to shift the texture coordinates per layer (from vector P)
    float2 P = viewDirTS.xy / viewDirTS.z * scale;
    float2 deltaTexCoords = P / numLayers;

    // get initial values
    float2 currentUv = uv;
    float currentDepthMapValue = 1.0 - heightMap.Sample(currentUv).r;

    while (currentLayerDepth < currentDepthMapValue) {
        // shift texture coordinates along direction of P
        currentUv -= deltaTexCoords;
        // get depth of current layer
        currentDepthMapValue = 1.0 - heightMap.Sample(currentUv).r;
        // get depth of next layer
        currentLayerDepth += layerDepth;
    }

    return currentUv;
}
