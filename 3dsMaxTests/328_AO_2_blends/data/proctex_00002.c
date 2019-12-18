float4 prtex2_main(const SurfaceInfo* sHit, float3 colorHit, sampler2D texHit, float3 colorMiss, sampler2D texMiss, float falloffPower, _PROCTEXTAILTAG_)
{
  const float2 texCoord = readAttr_TexCoord0(sHit);
  float  ao             = readAttr_AO(sHit);
 
  const float4 col1     = to_float4(colorHit, 1.0f)*texture2D(texHit, texCoord, 0);
  const float4 col2     = to_float4(colorMiss,1.0f)*texture2D(texMiss, texCoord, 0);
  
  ao = pow(ao, falloffPower);
  
  return col1 + ao*(col2 - col1);
}


