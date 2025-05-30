module PuduRandomModule;

///<summary>
/// Returns a random point on an hemisphere
/// https://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
/// </summary>
/// <param name="i">Seed</param>
/// <param name="N">Total number of points</param>
public float2 HammersleyHemisphere(uint i, uint N) {
  uint bits = (i << 16u) | (i >> 16u);
  bits = ((bits & 0x55555555u)<<1u)|((bits & 0xAAAAAAAAu)>>1u);
  bits = ((bits & 0x33333333u)<<2u)|((bits & 0xCCCCCCCCu)>>2u);
  bits = ((bits & 0x0F0F0F0Fu)<<4u)|((bits & 0xF0F0F0F0u)>>4u);
  bits = ((bits & 0x00FF00FFu)<<8u)|((bits & 0xFF00FF00u)>>8u);
  float rdi = float(bits) * 2.3283064365386963e-10;
  return float2(float(i) / float(N), rdi);
}

float mod(float x, float y)
{
  return x - y * floor(x/y);
}

///<summary>From https://byteblacksmith.com/improvements-to-the-canonical-one-liner-glsl-rand-for-opengl-es-2-0/ </summary>
public float random(float2 co) {
  float a  = 12.9898;
  float b  = 78.233;
  float c  = 43758.5453;
  float dt = dot( co.xy ,float2(a,b) );
  float sn = mod(dt ,3.14);
  return fract(sin(sn) * c);
}
