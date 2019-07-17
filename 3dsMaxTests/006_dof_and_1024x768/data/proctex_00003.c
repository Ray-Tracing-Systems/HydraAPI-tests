
float4 prtex3_mix(float4 x, float4 y, float a)
{
  return x*(1.0f - a) + y*a;
}

float4 prtex3_main(const SurfaceInfo* sHit, float4 color1, float4 color2, _PROCTEXTAILTAG_)
{
  const float3 pos  = readAttr_LocalPos(sHit);
  const float3 norm = readAttr_ShadeNorm(sHit);
  
  const float3 rayDir = hr_viewVectorHack;
  float cosAlpha      = fabs(dot(norm,rayDir));
  
  return prtex3_mix(color1, color2, cosAlpha);
}


