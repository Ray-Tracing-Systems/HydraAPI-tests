
    float clamp(float x, float minVal, float maxVal)
    {
	    return min(max(x, minVal), maxVal);
    }

    float fract(float x)
    {
	    return x - floor(x);
    }

    float smoothstep(float edge0, float edge1, float x)
    {
      const float t = clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
      return t * t * (3.0f - 2.0f * t);
    }

    float median(float r, float g, float b) {
	    return max(min(r, g), min(max(r, g), b));
    }

    float mix(float x, float y, float a)
    {
      return x * (1.0f - a) + y * a;
    }

    float4 mix4(float4 x, float4 y, float a)
    {
      return make_float4(mix(x.x, y.x, a), mix(x.y, y.y, a), mix(x.z, y.z, a), mix(x.w, y.w, a));
    }

    float4 colorFromUint(unsigned val)
    {
      const float a = ((val & 0xFF000000) >> 24) / 255.0f;
      const float b = ((val & 0x00FF0000) >> 16) / 255.0f;
      const float g = ((val & 0x0000FF00) >> 8 ) / 255.0f;
      const float r = ((val & 0x000000FF)      ) / 255.0f;
  
      return make_float4(r, g, b, a);
    }

    #define SDF    0
    #define MSDF   1
    #define RASTER 2


    float4 main(const SurfaceInfo* sHit, int mode, sampler2D sdfTexture[
  25], unsigned objColors[25], int numTextures, float4 bgColor, float2 texScale)
    {
      const float2 texCoord = readAttr(sHit,"TexCoord0");
      float2 texCoord_adj = texCoord;
      texCoord_adj.x = fract(texCoord_adj.x * texScale.x);
      texCoord_adj.y = fract(texCoord_adj.y * texScale.y);
  

      const float smoothing = 1.0f/64.0f;
      const float4 red = make_float4(1.0, 0.0, 0.0, 1.0);
      const float4 blu = make_float4(0.0, 0.0, 1.0, 1.0);
  
      if(numTextures == 1)
      {
	    const float4 texColor = texture2D(sdfTexture[0], texCoord_adj, TEX_CLAMP_U | TEX_CLAMP_V);
	    if(mode == RASTER)
	    {
	      return mix4(bgColor, texColor, texColor.w);
	    }
	  
	    const float4 objColor = colorFromUint(objColors[0]);
	    const float distance  = median(texColor.x, texColor.y, texColor.z);
	  
	    float alpha = smoothstep(0.5f - smoothing, 0.5f + smoothing, distance);  
	
	    return mix4(bgColor, objColor, alpha);
      }
      else
      {
	    float4 resColor = bgColor;
	    for(int i = 0; i < numTextures; i += 1)
	    {
	      const float4 objColor = colorFromUint(objColors[i]);
	      const float4 texColor = texture2D(sdfTexture[i], texCoord_adj, TEX_CLAMP_U | TEX_CLAMP_V);
	      const float distance  = median(texColor.x, texColor.y, texColor.z);
	      float alpha = smoothstep(0.5f - smoothing, 0.5f + smoothing, distance);  
	  
	      resColor = mix4(resColor, objColor, alpha);
	    }
	    return resColor;
      }
    }
    