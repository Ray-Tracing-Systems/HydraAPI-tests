#include "tests.h"
#include <cmath>
#include <cstring>

#include <fstream>
#include <vector>
#include <sstream>

#include "simplerandom.h"

#include "HR_HDRImageTool.h"
#include "HydraTextureUtils.h"

///////////////////////////////////////////////////////////////////////////////////

#pragma warning(disable:4838)
using HDRImage4f = HydraRender::HDRImage4f;
std::vector<bool> g_resultTest;


namespace TEST_UTILS
{   
  SystemInfo::SystemInfo(const std::wstring& a_operationSystem, const std::wstring& a_videocard) :
    m_operationSystem(a_operationSystem),
    m_videocard(a_videocard) {}

  void SystemInfo::SetOperSys(const std::wstring& a_operationSystem) { m_operationSystem = a_operationSystem; }
  void SystemInfo::SetVideocard(const std::wstring& a_videocard) { m_videocard = a_videocard; }
  std::wstring SystemInfo::GetFolderNameFromInfo() { return m_operationSystem + L"_" + m_videocard; }
  std::wstring SystemInfo::GetOsName() { return m_operationSystem; }
  std::wstring SystemInfo::GetVideocardName() { return m_videocard; }


  void show_me_texture_ldr(const std::string& a_inFleName, const std::string& a_outFleName)
  {
    int32_t wh[2];

    std::ifstream fin(a_inFleName, std::iostream::binary);
    fin.read((char*)wh, sizeof(int) * 2);

    if (wh[0] <= 0 || wh[1] <= 0)
      return;

    std::vector<uint32_t> sdata(wh[0] * wh[1]);
    fin.read((char*)&sdata[0], sdata.size() * sizeof(uint32_t));
    fin.close();

    HydraRender::SaveImageToFile(a_outFleName, wh[0], wh[1], &sdata[0]);
  }

  void show_me_texture_hdr(const std::string& a_inFleName, const std::string& a_outFleName)
  {
    int32_t wh[2];

    std::ifstream fin(a_inFleName, std::iostream::binary);
    fin.read((char*)wh, sizeof(int) * 2);

    if (wh[0] <= 0 || wh[1] <= 0)
      return;

    HDRImage4f colorImg(wh[0], wh[1]);
    fin.read((char*)colorImg.data(), wh[0] * wh[1] * 4 * sizeof(float));
    fin.close();

    HydraRender::SaveImageToFile(a_outFleName, colorImg, 2.2f);
  }

  std::vector<unsigned int> CreateStripedImageData(unsigned int* a_colors, int a_stripsNum, int w, int h)
  {
    std::vector<unsigned int> imageData(w*h);

    int currH = 0;
    int strideH = (h / a_stripsNum);

    for (int stripId = 0; stripId < a_stripsNum; stripId++)
    {
      unsigned int color = a_colors[stripId];

      #pragma omp parallel for
      for (int y = currH; y < currH + strideH; y++)
      {
        for (int x = 0; x < w; x++)
          imageData[y*w + x] = color;
      }

      currH += strideH;
    }

    return imageData;
  }


  HRTextureNodeRef AddRandomTextureFromMemory(size_t& memTotal, simplerandom::RandomGen& rgen)
  {
    
    int choice = rand(rgen) % 3;

    int w = rand(rgen) % 2048 + 1 + 128;
    int h = rand(rgen) % 2048 + 1 + 128;

    if (choice == 0)      // add LDR texture
    {
      std::vector<int> data(w*h);
      for (size_t i = 0; i < data.size(); i++)
        data[i] = 0xFFFF00FF;

      memTotal += w*h * 4;
      return hrTexture2DCreateFromMemory(w, h, 4, &data[0]);
    }
    else if (choice == 1)      // add LDR texture
    {
      std::vector<int> data(w*h);
      for (size_t i = 0; i < data.size(); i++)
        data[i] = 0xFF7F0060;

      memTotal += w*h * 4;
      return hrTexture2DCreateFromMemory(w, h, 4, &data[0]);
    }
    else // add HDR texture
    {
      std::vector<int> data(w*h);
      for (size_t i = 0; i < data.size(); i++)
        data[i] = 0xFFA070F0;

      memTotal += w*h * 4;
      return hrTexture2DCreateFromMemory(w, h, 4, &data[0]);
    }
  }

  static inline float clamp(float u, float a, float b) { return fminf(fmaxf(a, u), b); }


  void procTexCheckerLDR(unsigned char* a_buffer, int w, int h, void* a_repeat)
  {
    if (a_repeat == nullptr)
      return;

    int* data = (int*)a_repeat;
    int repeats = *data;

    repeats *= 2;

    #pragma omp parallel for
    for (int y = 0; y < h; y++)
    {
      for (int x = 0; x < w; x++)
      {
        float cx = floor(repeats * (float(x) / w));
        float cy = floor(repeats * (float(y) / h));
        float result = fmod(cx + cy, 2.0f);
        clamp(result, 0.0, 1.0);

        a_buffer[(y*w + x) * 4 + 0] = (unsigned char)(result * 255);
        a_buffer[(y*w + x) * 4 + 1] = (unsigned char)(result * 255);
        a_buffer[(y*w + x) * 4 + 2] = (unsigned char)(result * 255);
        a_buffer[(y*w + x) * 4 + 3] = 255;
      }
    }
  }

  void procTexCheckerHDR(float* a_buffer, int w, int h, void* a_repeat)
  {
    if (a_repeat == nullptr)
      return;

    int* data = (int*)a_repeat;
    int repeats = *data;

    repeats *= 2;

    #pragma omp parallel for
    for (int y = 0; y < h; y++)
    {
      for (int x = 0; x < w; x++)
      {
        float cx = floor(repeats * (float(x) / w));
        float cy = floor(repeats * (float(y) / h));
        float result = fmod(cx + cy, 2.0f);
        if (result < 0.001f)
          result = 0.0f;
        else
          result = 10.0f;

        a_buffer[(y*w + x) * 4 + 0] = result;
        a_buffer[(y*w + x) * 4 + 1] = result;
        a_buffer[(y*w + x) * 4 + 2] = result;
        a_buffer[(y*w + x) * 4 + 3] = 1.0;
      }
    }
  }


  void customDisplacement1(const float *pos, const float *normal, const HRUtils::BBox &bbox, float displace_vec[3],
                           void* a_customData, uint32_t a_customDataSize)
  {

    auto *data = (displace_data_1 *) a_customData;

    float3 N(normal[0], normal[1], normal[2]);

    auto tmp = cross(data->global_dir, N);
    auto d = cross(N, tmp);

    d = normalize(d);

    float mult = 5.0f - pos[1];
    if(mult < 0.0f) mult = 0.0f;

    displace_vec[0] = d.x * data->mult * mult;
    displace_vec[1] = d.y * data->mult * mult;
    displace_vec[2] = d.z * data->mult * mult;
  }

  void customDisplacementSpots(const float *pos, const float *normal, const HRUtils::BBox &bbox, float displace_vec[3],
                               void* a_customData, uint32_t a_customDataSize)
  {

    auto *data = (displace_data_1 *) a_customData;

    float3 N(normal[0], normal[1], normal[2]);
    float3 position(pos[0], pos[1], pos[2]-2.2f);

    float spots_scale = 120.0f;
    float spots_detail = 0.1f;
    float spots_thr = 0.58;
    float n2 = HRTextureUtils::noise(position * spots_scale, 0.0f, spots_detail) - spots_thr;
    n2 = clamp(n2, 0.0f, 1.0f);
    n2 = powf(n2, 0.15f);

    /*float y_gen = (position.y) / 8.1f; //bbox_y = 8.1f

    n2 = clamp(n2, 0.0f, 1.0f) * (1.0f - HRTextureUtils::fitRange(y_gen, 0.4, 1.0f, 0.0f, 1.0f));
*/
//
//    float mult = 1.0f - pos[1];
//    if(mult < 0.0f) mult = 0.0f;

    auto d = N * n2;

    displace_vec[0] = d.x * data->mult;
    displace_vec[1] = d.y * data->mult;
    displace_vec[2] = d.z * data->mult;
  }

  void customDisplacementFBM(const float *p, const float *normal, const HRUtils::BBox &bbox, float displace_vec[3],
                               void* a_customData, uint32_t a_customDataSize)
  {

    auto *data = (displace_data_1 *) a_customData;

    float3 N(normal[0], normal[1], normal[2]);
    float3 pos(p[0]+0.45f, p[1]-0.5f, p[2]);

    //float3 gen_pos = make_float3(pos.x/(bbox.x_max - bbox.x_min), pos.y/(bbox.y_max - bbox.y_min), pos.z/(bbox.z_max - bbox.z_min));
    float3 gen_pos = make_float3(HRTextureUtils::fitRange(pos.x, bbox.x_min, bbox.x_max, 0.0f, 1.0f)-0.15f,
                                 HRTextureUtils::fitRange(pos.y, bbox.y_min, bbox.y_max, 0.0f, 1.0f),
                                 HRTextureUtils::fitRange(pos.z, bbox.z_min, bbox.z_max, 0.0f, 1.0f));
    float scale = 0.750f;
    float dimension = 0.1f;
    float octaves = 8;
    float lacunarity = 2*1.5f;

    gen_pos = gen_pos * scale;

//  float n = octave(gen_pos, 8, 0.5f, 2.0, 5.0f);
    float n1 = HRTextureUtils::noise_musgrave_fBm(gen_pos, dimension, lacunarity, octaves);
    float n2 = HRTextureUtils::noise_musgrave_fBm(gen_pos/(scale), dimension, 4.0f, 4);

    int tmp = 0;
    if(n1 < -0.5f)
      tmp = 1;
    float w = n2 / (tmp - n1);
    if(gen_pos.y / scale < 1.25f)
    {
      w = w / (gen_pos.y / scale);
    }
    else if(gen_pos.y / scale > 1.25f)
      w = 0.0f;

    float mult = 1.25f - gen_pos.y;
    if(mult < 0.0f) mult = 0.0f;

    if (pos.z < -2.0f && pos.x > 0.3f)
      mult *= 5.0f;
    else
      mult /=5.0f;

    w = clamp(w, 0.0, 1.0);

    auto d = N * w;

    displace_vec[0] = d.x * data->mult * mult;
    displace_vec[1] = d.y * data->mult * mult;
    displace_vec[2] = d.z * data->mult * mult;
  }

  void CreateTestBigTexturesFilesIfNeeded()
  {
    const int TXSZ = 4096;

    if (!FileExists("data/textures_gen/texture_big_red.png"))
    {
      std::cout << "creating test texture and saving it to disk ... " << std::endl;
      unsigned int color = 0xFF0000FF;
      CreateStripedImageFile("data/textures_gen/texture_big_red.png", &color, 1, TXSZ, TXSZ);
    }

    if (!FileExists("data/textures_gen/texture_green.png"))
    {
      std::cout << "creating test texture and saving it to disk ... " << std::endl;
      unsigned int color = 0xFF00FF00;
      CreateStripedImageFile("data/textures_gen/texture_green.png", &color, 1, TXSZ, TXSZ);
    }

    if (!FileExists("data/textures_gen/texture_blue.png"))
    {
      std::cout << "creating test texture and saving it to disk ... " << std::endl;
      unsigned int color = 0xFFFF0000;
      CreateStripedImageFile("data/textures_gen/texture_blue.png", &color, 1, TXSZ, TXSZ);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    if (!FileExists("data/textures_gen/texture_big_z01.png"))
    {
      std::cout << "creating test texture and saving it to disk ... " << std::endl;
      unsigned int colors[2] = { 0xFF0000FF, 0xFF00FF00 };
      CreateStripedImageFile("data/textures_gen/texture_big_z01.png", colors, 2, TXSZ, TXSZ);
    }

    if (!FileExists("data/textures_gen/texture_big_z02.png"))
    {
      std::cout << "creating test texture and saving it to disk ... " << std::endl;
      unsigned int colors[2] = { 0xFF0000FF, 0xFFFF0000 };
      CreateStripedImageFile("data/textures_gen/texture_big_z02.png", colors, 2, TXSZ, TXSZ);
    }

    if (!FileExists("data/textures_gen/texture_big_z03.png"))
    {
      std::cout << "creating test texture and saving it to disk ... " << std::endl;
      unsigned int colors[2] = { 0xFF00FFFF, 0xFF0000FF };
      CreateStripedImageFile("data/textures_gen/texture_big_z03.png", colors, 2, TXSZ, TXSZ);
    }

    if (!FileExists("data/textures_gen/texture_big_z04.png"))
    {
      std::cout << "creating test texture and saving it to disk ... " << std::endl;
      unsigned int colors[2] = { 0xFFFF0000, 0xFF00FF00 };
      CreateStripedImageFile("data/textures_gen/texture_big_z04.png", colors, 2, TXSZ, TXSZ);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    if (!FileExists("data/textures_gen/texture_big_z05.png"))
    {
      std::cout << "creating test texture and saving it to disk ... " << std::endl;
      unsigned int colors[4] = { 0xFF0000FF, 0xFF00FF00, 0xFFFF0000, 0xFFFF00FF };
      CreateStripedImageFile("data/textures_gen/texture_big_z05.png", colors, 4, TXSZ, TXSZ);
    }

    if (!FileExists("data/textures_gen/texture_big_z06.png"))
    {
      std::cout << "creating test texture and saving it to disk ... " << std::endl;
      unsigned int colors[4] = { 0xFF00FF00, 0xFFFF0000, 0xFFFF00FF, 0xFFFFFFFF };
      CreateStripedImageFile("data/textures_gen/texture_big_z06.png", colors, 4, TXSZ, TXSZ);
    }

    if (!FileExists("data/textures_gen/texture_big_z07.png"))
    {
      std::cout << "creating test texture and saving it to disk ... " << std::endl;
      unsigned int colors[4] = { 0xFF0000FF, 0xFF00FF00, 0xFF00FFFF, 0xFF000000 };
      CreateStripedImageFile("data/textures_gen/texture_big_z07.png", colors, 4, TXSZ, TXSZ);
    }
  }

  HRTextureNodeRef CreateRandomStrippedTextureFromMemory(size_t& a_byteSize, simplerandom::RandomGen& rgen)
  {
    int TXSZ = 1024;
    int choice1 = simplerandom::rand(rgen) % 3;
    if (choice1 == 0)
    {
      a_byteSize += size_t(2048 * 2048 * 4);
      TXSZ = 2048;
    }
    else if (choice1 == 2)
    {
      a_byteSize += size_t(512 * 512 * 4);
      TXSZ = 512;
    }
    else
    {
      a_byteSize += size_t(1024 * 1024 * 4);
      TXSZ = 1024;
    }

    int choice = simplerandom::rand(rgen) % 10;

    if (choice == 0)
    {
      unsigned int color = 0xFF0000FF;
      auto data = CreateStripedImageData(&color, 1, TXSZ, TXSZ);
      return hrTexture2DCreateFromMemory(TXSZ, TXSZ, 4, &data[0]);
    }
    else if (choice == 1)
    {
      unsigned int color = 0xFF00FF00;
      auto data = CreateStripedImageData(&color, 1, TXSZ, TXSZ);
      return hrTexture2DCreateFromMemory(TXSZ, TXSZ, 4, &data[0]);
    }
    else if (choice == 2)
    {
      unsigned int color = 0xFFFF0000;
      auto data = CreateStripedImageData(&color, 1, TXSZ, TXSZ);
      return hrTexture2DCreateFromMemory(TXSZ, TXSZ, 4, &data[0]);
    }
    else if (choice == 3)
    {
      unsigned int colors[2] = { 0xFF0000FF, 0xFF00FF00 };
      auto data = CreateStripedImageData(colors, 2, TXSZ, TXSZ);
      return hrTexture2DCreateFromMemory(TXSZ, TXSZ, 4, &data[0]);
    }
    else if (choice == 4)
    {
      unsigned int colors[2] = { 0xFF0000FF, 0xFFFF0000 };
      auto data = CreateStripedImageData(colors, 2, TXSZ, TXSZ);
      return hrTexture2DCreateFromMemory(TXSZ, TXSZ, 4, &data[0]);
    }
    else if (choice == 5)
    {
      unsigned int colors[2] = { 0xFF00FFFF, 0xFF0000FF };
      auto data = CreateStripedImageData(colors, 2, TXSZ, TXSZ);
      return hrTexture2DCreateFromMemory(TXSZ, TXSZ, 4, &data[0]);
    }
    else if (choice == 6)
    {
      unsigned int colors[2] = { 0xFFFF0000, 0xFF00FF00 };
      auto data = CreateStripedImageData(colors, 2, TXSZ, TXSZ);
      return hrTexture2DCreateFromMemory(TXSZ, TXSZ, 4, &data[0]);
    }
    else if (choice == 7)
    {
      unsigned int colors[4] = { 0xFF0000FF, 0xFF00FF00, 0xFFFF0000, 0xFFFF00FF };
      auto data = CreateStripedImageData(colors, 4, TXSZ, TXSZ);
      return hrTexture2DCreateFromMemory(TXSZ, TXSZ, 4, &data[0]);
    }
    else if (choice == 8)
    {
      unsigned int colors[4] = { 0xFF00FF00, 0xFFFF0000, 0xFFFF00FF, 0xFFFFFFFF };
      auto data = CreateStripedImageData(colors, 4, TXSZ, TXSZ);
      return hrTexture2DCreateFromMemory(TXSZ, TXSZ, 4, &data[0]);
    }
    else
    {
      unsigned int colors[4] = { 0xFF0000FF, 0xFF00FF00, 0xFF00FFFF, 0xFF000000 };
      auto data = CreateStripedImageData(colors, 4, TXSZ, TXSZ);
      return hrTexture2DCreateFromMemory(TXSZ, TXSZ, 4, &data[0]);
    }
  }

  bool FileExists(const char* a_fileName)
  {
    std::ifstream fin(a_fileName);
    bool res = fin.is_open();
    fin.close();
    return res;
  }


  void CreateStripedImageFile(const char* a_fileName, unsigned int* a_colors, int a_stripsNum, int w, int h)
  {
    std::vector<unsigned int> imageData = CreateStripedImageData(a_colors, a_stripsNum, w, h);
    HydraRender::SaveImageToFile(a_fileName, w, h, imageData.data());
  }

  std::vector<HRMeshRef> CreateRandomMeshesArray(int a_size, simplerandom::RandomGen& rgen)
  {
    std::vector<HRMeshRef> meshes(a_size);

    for (size_t i = 0; i < meshes.size(); i++)
    {
      int choice = simplerandom::rand(rgen) % 3;

      if (choice == 0)
        meshes[i] = HRMeshFromSimpleMesh(L"my_cube", CreateCube(0.5f), simplerandom::rand(rgen) % 50);
      else if (choice == 1)
        meshes[i] = HRMeshFromSimpleMesh(L"my_sphere", CreateSphere(0.5f, 128), simplerandom::rand(rgen) % 50);
      else
        meshes[i] = HRMeshFromSimpleMesh(L"my_torus2", CreateTorus(0.2f, 0.5f, 128, 128), simplerandom::rand(rgen) % 50);

      if (i % 20 == 0)
        std::cout << "[test_mbm]: MB, total meshes = " << i << "\r";
    }

    std::cout << std::endl;

    return meshes;
  }

  HRMeshRef HRMeshFromSimpleMesh(const wchar_t* a_name, const SimpleMesh& a_mesh, int a_matId)
  {
    HRMeshRef meshRef = hrMeshCreate(a_name);

    hrMeshOpen(meshRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
    {
      hrMeshVertexAttribPointer4f(meshRef, L"pos", &a_mesh.vPos[0]);
      hrMeshVertexAttribPointer4f(meshRef, L"norm", &a_mesh.vNorm[0]);
      hrMeshVertexAttribPointer2f(meshRef, L"texcoord", &a_mesh.vTexCoord[0]);

      hrMeshMaterialId(meshRef, a_matId);
      hrMeshAppendTriangles3(meshRef, int(a_mesh.triIndices.size()), &a_mesh.triIndices[0]);
    }
    hrMeshClose(meshRef);

    return meshRef;
  }

  HRMeshRef CreateCornellBox(const float a_size, HRMaterialRef a_leftWallMat, HRMaterialRef a_rightWallMat, HRMaterialRef a_ceilingMat, HRMaterialRef a_backWallMat, HRMaterialRef a_floorMat)
  {
    SimpleMesh cubeOpen = CreateCubeOpen(a_size);
    HRMeshRef meshRef   = hrMeshCreate(L"CornellBox");

    hrMeshOpen(meshRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
    {
      hrMeshVertexAttribPointer4f(meshRef, L"pos",      &cubeOpen.vPos[0]);
      hrMeshVertexAttribPointer4f(meshRef, L"norm",     &cubeOpen.vNorm[0]);
      hrMeshVertexAttribPointer2f(meshRef, L"texcoord", &cubeOpen.vTexCoord[0]);

      int cubeMatIndices[10] = { 
        a_ceilingMat.id,    a_ceilingMat.id, 
        a_backWallMat.id,   a_backWallMat.id, 
        a_floorMat.id,      a_floorMat.id, 
        a_rightWallMat.id,  a_rightWallMat.id, 
        a_leftWallMat.id,   a_leftWallMat.id };

      hrMeshPrimitiveAttribPointer1i(meshRef, L"mind", cubeMatIndices);
      hrMeshAppendTriangles3(meshRef, int(cubeOpen.triIndices.size()), &cubeOpen.triIndices[0]);
    }
    hrMeshClose(meshRef);

    return meshRef;
  }


  void AddDiffuseNode(HAPI pugi::xml_node& matNode, const wchar_t* a_diffuseColor,
    const wchar_t* a_brdfType, const float a_roughness, HRTextureNodeRef a_texture, 
    const wchar_t* a_addressingModeU, const wchar_t* a_addressingModeV, const float a_tileU, 
    const float a_tileV, const float a_inputGamma, const wchar_t* a_inputAlpha, 
    const wchar_t* a_texApplyMode)
  {
    auto diff  = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type") = a_brdfType;

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val") = a_diffuseColor;

    if (std::wstring(a_brdfType) == L"orennayar")
    {
      auto rough = diff.append_child(L"roughness");
      rough.append_attribute(L"val") = a_roughness;
    }

    if (a_texture.id != -1)
    {
      color.append_attribute(L"tex_apply_mode") = a_texApplyMode;
      hrTextureBind(a_texture, color);

      auto texNode = color.child(L"texture");
      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { a_tileU, 0,       0,       0,
                                  0,       a_tileV, 0,       0,
                                  0,       0,       1,       0,
                                  0,       0,       0,       1 };

      texNode.append_attribute(L"addressing_mode_u") = a_addressingModeU;
      texNode.append_attribute(L"addressing_mode_v") = a_addressingModeV;
      texNode.append_attribute(L"input_gamma")       = a_inputGamma;
      texNode.append_attribute(L"input_alpha")       = a_inputAlpha;

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);
    }
    VERIFY_XML(matNode);
  }


  void AddReflectionNode(HAPI pugi::xml_node& matNode, const wchar_t* a_brdfType, const wchar_t* a_color,
    const float a_glossiness, const bool a_fresnel, const float a_ior, const wchar_t* a_extrusion, const bool a_energyFix)
  {
    auto refl = matNode.append_child(L"reflectivity");

    refl.append_attribute(L"brdf_type")                        = a_brdfType;
    refl.append_child(L"color").append_attribute(L"val")       = a_color;
    refl.append_child(L"glossiness").append_attribute(L"val")  = a_glossiness;
    refl.append_child(L"extrusion").append_attribute(L"val")   = a_extrusion;
    refl.append_child(L"fresnel").append_attribute(L"val")     = (int)(a_fresnel);
    refl.append_child(L"fresnel_ior").append_attribute(L"val") = a_ior;
    refl.append_child(L"energy_fix").append_attribute(L"val")  = (int)(a_energyFix);
    VERIFY_XML(matNode);
  }


  void AddOpacityNode(HAPI pugi::xml_node& matNode, HRTextureNodeRef a_texture, const bool a_skipShadow,
    const wchar_t* a_addressingModeU, const wchar_t* a_addressingModeV,
    const float a_tileU, const float a_tileV, const float a_inputGamma, const wchar_t* a_inputAlpha)
  {
    auto opacity = matNode.append_child(L"opacity");
    opacity.append_child(L"skip_shadow").append_attribute(L"val") = (int)(a_skipShadow);

    auto texNode = hrTextureBind(a_texture, opacity);

    texNode.append_attribute(L"matrix");
    float samplerMatrix[16] = { a_tileU, 0,       0,       0,
                                0,       a_tileV, 0,       0,
                                0,       0,       1,       0,
                                0,       0,       0,       1 };

    texNode.append_attribute(L"addressing_mode_u") = a_addressingModeU;
    texNode.append_attribute(L"addressing_mode_v") = a_addressingModeV;
    texNode.append_attribute(L"input_gamma")       = a_inputGamma;
    texNode.append_attribute(L"input_alpha")       = a_inputAlpha;

    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

    VERIFY_XML(matNode);
  }


  void AddTranslucencyNode(HAPI pugi::xml_node& matNode, const wchar_t* a_color)
  {
    auto transl = matNode.append_child(L"translucency");
    transl.append_child(L"color").append_attribute(L"val") = a_color;
    VERIFY_XML(matNode);
  }


  void AddReliefNode(HAPI pugi::xml_node& matNode, const wchar_t* a_type, const float a_amount,
    HRTextureNodeRef a_texture, const wchar_t* a_addressingModeU, const wchar_t* a_addressingModeV,
    const float a_tileU, const float a_tileV, const float a_inputGamma, const wchar_t* a_inputAlpha)
  {
    auto displacement = matNode.append_child(L"displacement");
    
    pugi::xml_node heightNode;

    if      (std::wstring(a_type) == L"height_bump") heightNode = displacement.append_child(L"height_map");
    else if (std::wstring(a_type) == L"normal_bump") heightNode = displacement.append_child(L"normal_map");    

    displacement.append_attribute(L"type") = a_type;
    heightNode.append_attribute(L"amount") = a_amount;

    auto texNode = hrTextureBind(a_texture, heightNode);

    texNode.append_attribute(L"matrix");
    float samplerMatrix[16] = { a_tileU,  0,       0,      0,
                                0,  a_tileV,       0,      0,
                                0,        0,       1,      0,
                                0,        0,       0,      1 };

    texNode.append_attribute(L"addressing_mode_u") = a_addressingModeU;
    texNode.append_attribute(L"addressing_mode_v") = a_addressingModeV;
    texNode.append_attribute(L"input_gamma")       = a_inputGamma;
    texNode.append_attribute(L"input_alpha")       = a_inputAlpha;

    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);
    VERIFY_XML(matNode);
  }


  void CreateCamera(const float a_fov, const wchar_t* a_position, const wchar_t* a_lookAt,
    const wchar_t* a_name, const float a_nearClipPlane, const float a_farClipPlane, const wchar_t* a_up,
    const bool a_dof, const float a_dofLensRadius)
  {
    auto camRef = hrCameraCreate(a_name);

    hrCameraOpen(camRef, HR_WRITE_DISCARD);
    {
      auto camNode                                    = hrCameraParamNode(camRef);

      camNode.append_child(L"fov").text()             = a_fov;
      camNode.append_child(L"nearClipPlane").text()   = a_nearClipPlane;
      camNode.append_child(L"farClipPlane").text()    = a_farClipPlane;

      camNode.append_child(L"up").text()              = a_up;
      camNode.append_child(L"position").text()        = a_position;
      camNode.append_child(L"look_at").text()         = a_lookAt;

      camNode.append_child(L"enable_dof").text()      = (int)(a_dof);
      camNode.append_child(L"dof_lens_radius").text() = a_dofLensRadius;

      VERIFY_XML(camNode); 
    }
    hrCameraClose(camRef);
  }

  void AddMeshToScene(HRSceneInstRef& scnRef, HRMeshRef& a_meshRef, float3 pos, float3 rot, float3 scale,
    const int32_t* a_mmListm, int32_t a_mmListSize)
  {
    float4x4 mRotX;
    float4x4 mRotY;
    float4x4 mRotZ;
    float4x4 mScale;
    float4x4 mTranslate;
    float4x4 mRes;

    mRotX.identity();
    mRotY.identity();
    mRotZ.identity();
    mScale.identity();
    mTranslate.identity();
    mRes.identity();

    mScale     = scale4x4(scale);
    mRotZ      = rotate4x4Z(rot.z * DEG_TO_RAD);
    mRotY      = rotate4x4Y(rot.y * DEG_TO_RAD);
    mRotX      = rotate4x4X(rot.x * DEG_TO_RAD);
    mTranslate = translate4x4(pos);

    mRes       = mul(mScale, mRes);    
    mRes       = mul(mRotZ, mRes);
    mRes       = mul(mRotY, mRes);
    mRes       = mul(mRotX, mRes);
    mRes       = mul(mTranslate, mRes);

    if (a_mmListSize > 0)
      hrMeshInstance(scnRef, a_meshRef, mRes.L(), a_mmListm, a_mmListSize);
    else    
      hrMeshInstance(scnRef, a_meshRef, mRes.L());

    
    //float mRot[4][4];
    //float mTranslate[4][4];
    //float matrixT[4][4];
    //float mRes[4][4];

    //mat4x4_identity(mTranslate);
    //mat4x4_identity(mRot);

    //mat4x4_translate(mTranslate, pos.x, pos.y, pos.z);
    //mat4x4_rotate_Z(mRot, mRot, rot.z * DEG_TO_RAD);
    //mat4x4_rotate_Y(mRot, mRot, rot.y * DEG_TO_RAD);
    //mat4x4_rotate_X(mRot, mRot, rot.x * DEG_TO_RAD);

    //mat4x4_mul(mRes, mTranslate, mRot);
    //mat4x4_transpose(matrixT, mRes); //swap rows and columns

    //if (a_mmListSize > 0)
    //  hrMeshInstance(scnRef, a_meshRef, &matrixT[0][0], a_mmListm, a_mmListSize);
    //else    
    //  hrMeshInstance(scnRef, a_meshRef, &matrixT[0][0]);
  }


  void AddLightToScene(HRSceneInstRef& scnRef, HRLightRef& a_lightRef, const float3 a_pos, 
    const float3 a_rot, const float3 a_scale)
  {
    float4x4 mRotX;
    float4x4 mRotY;
    float4x4 mRotZ;
    float4x4 mScale;
    float4x4 mTranslate;
    float4x4 mRes;

    mRotX.identity();
    mRotY.identity();
    mRotZ.identity();
    mScale.identity();
    mTranslate.identity();
    mRes.identity();

    mScale     = scale4x4(a_scale);
    mRotZ      = rotate4x4Z(a_rot.z * DEG_TO_RAD);
    mRotY      = rotate4x4Y(a_rot.y * DEG_TO_RAD);
    mRotX      = rotate4x4X(a_rot.x * DEG_TO_RAD);
    mTranslate = translate4x4(a_pos);

    mRes       = mul(mScale, mRes);
    mRes       = mul(mRotZ, mRes);
    mRes       = mul(mRotY, mRes);
    mRes       = mul(mRotX, mRes);
    mRes       = mul(mTranslate, mRes);

    hrLightInstance(scnRef, a_lightRef, mRes.L());

    //float mRot[4][4];
    //float mTranslate[4][4];
    //float matrixT[4][4];
    //float mRes[4][4];

    //mat4x4_identity(mTranslate);
    //mat4x4_identity(mRot);

    //mat4x4_translate(mTranslate, a_pos.x, a_pos.y, a_pos.z);
    //mat4x4_rotate_Z(mRot, mRot, a_rot.z * DEG_TO_RAD);
    //mat4x4_rotate_Y(mRot, mRot, a_rot.y * DEG_TO_RAD);
    //mat4x4_rotate_X(mRot, mRot, a_rot.x * DEG_TO_RAD);

    //mat4x4_mul(mRes, mTranslate, mRot);
    //mat4x4_transpose(matrixT, mRes); //swap rows and columns

    //hrLightInstance(scnRef, a_lightRef, &matrixT[0][0]);
  }


  HRRenderRef CreateBasicTestRenderPT(int a_deviceId, int a_w, int a_h, int a_minRays, int a_maxRays,
                                      int a_rayBounce, int a_diffBounce, const wchar_t* a_drvName)
  {
    auto renderRef = hrRenderCreate(a_drvName);
    hrRenderEnableDevice(renderRef, a_deviceId, true);

    hrRenderOpen(renderRef, HR_WRITE_DISCARD);
    {
      auto node = hrRenderParamNode(renderRef);

      node.append_child(L"width").text()  = a_w;
      node.append_child(L"height").text() = a_h;

      node.append_child(L"method_primary").text()   = L"pathtracing";
      node.append_child(L"method_secondary").text() = L"pathtracing";
      node.append_child(L"method_tertiary").text()  = L"pathtracing";
      node.append_child(L"method_caustic").text()   = L"pathtracing";
      node.append_child(L"qmc_variant").text()      = QMC_ALL;


      node.append_child(L"trace_depth").text()      = a_rayBounce;
      node.append_child(L"diff_trace_depth").text() = a_diffBounce;
      node.append_child(L"maxRaysPerPixel").text()  = a_maxRays;
      node.append_child(L"resources_path").text()   = L"..";
      node.append_child(L"offline_pt").text()       = 0;
    }
    hrRenderClose(renderRef);

    return renderRef;
  }

  HRRenderRef CreateBasicTestRenderPTNoCaust(int deviceId, int w, int h, int minRays, int maxRays,
    const float a_clamp)
  {
    auto renderRef = hrRenderCreate(L"HydraModern");
    hrRenderEnableDevice(renderRef, deviceId, true);

    hrRenderOpen(renderRef, HR_WRITE_DISCARD);
    {
      auto node = hrRenderParamNode(renderRef);

      node.append_child(L"width").text()  = w;
      node.append_child(L"height").text() = h;

      node.append_child(L"method_primary").text()   = L"pathtracing";
      node.append_child(L"method_secondary").text() = L"pathtracing";
      node.append_child(L"method_tertiary").text()  = L"pathtracing";
      node.append_child(L"method_caustic").text()   = L"none";

      node.append_child(L"trace_depth").text()      = L"5";
      node.append_child(L"diff_trace_depth").text() = L"3";
      node.append_child(L"qmc_variant").text()      = QMC_ALL;

      node.append_child(L"minRaysPerPixel").text() = minRays;
      node.append_child(L"maxRaysPerPixel").text() = maxRays;
      node.append_child(L"clamping").text()        = a_clamp;
    }
    hrRenderClose(renderRef);

    return renderRef;
  }

  HRRenderRef CreateBasicTestRenderPTFastBackground(int deviceId, int w, int h, int minRays, int maxRays, const wchar_t* a_drvName)
  {
    auto renderRef = hrRenderCreate(a_drvName);
    hrRenderEnableDevice(renderRef, deviceId, true);

    hrRenderOpen(renderRef, HR_WRITE_DISCARD);
    {
      auto node = hrRenderParamNode(renderRef);

      node.append_child(L"width").text()  = w;
      node.append_child(L"height").text() = h;

      node.append_child(L"method_primary").text()   = L"pathtracing";
      node.append_child(L"method_secondary").text() = L"pathtracing";
      node.append_child(L"method_tertiary").text()  = L"pathtracing";
      node.append_child(L"method_caustic").text()   = L"pathtracing";
      //node.append_child(L"qmc_variant").text()      = QMC_ALL; not work with offline_pt


      node.append_child(L"trace_depth").text()      = 6;
      node.append_child(L"diff_trace_depth").text() = 4;
      node.append_child(L"maxRaysPerPixel").text()  = maxRays;
      node.append_child(L"resources_path").text()   = L"..";
      node.append_child(L"offline_pt").text()       = 1;
    }
    hrRenderClose(renderRef);

    return renderRef;
  }

  HRLightRef CreateLight(const wchar_t* a_name, const wchar_t* a_type, const wchar_t* a_shape,
    const wchar_t* a_distribution, const float a_halfLength, const float a_halfWidth,
    const wchar_t* a_color, const float a_multiplier, const bool a_spot,
    const float a_innerRadius, const float a_outerRadius, const float a_shadowSoft)
  {
    auto light = hrLightCreate(a_name);

    hrLightOpen(light, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(light);

      lightNode.attribute(L"type")         = a_type;
      lightNode.attribute(L"shape")        = a_shape;
      lightNode.attribute(L"distribution") = a_distribution;
      lightNode.force_child(L"shadow_softness").force_attribute(L"val") = a_shadowSoft;

      auto sizeNode = lightNode.append_child(L"size");

      if (std::wstring(a_shape) == L"rect")
      {
        sizeNode.force_attribute(L"half_length") = a_halfLength;
        sizeNode.force_attribute(L"half_width")  = a_halfWidth;
      }
      else if (std::wstring(a_shape) == L"sphere")
      {
        sizeNode.force_attribute(L"radius") = a_halfLength;
      }

      if (a_spot)
      {
        sizeNode.force_attribute(L"inner_radius") = a_innerRadius;
        sizeNode.force_attribute(L"outer_radius") = a_outerRadius;
      }

      auto intensityNode = lightNode.force_child(L"intensity");

      intensityNode.force_child(L"color").force_attribute(L"val")        = a_color;
      intensityNode.force_child(L"multiplier").force_attribute(L"val")   = a_multiplier;
			VERIFY_XML(lightNode);
    }
    hrLightClose(light);

    return light;
  }

  HRLightRef CreateSky(const wchar_t* a_name, const wchar_t* a_color, const float a_multiplier, 
    const wchar_t* a_distribution, const int a_sunId, const float a_turbidity, HRTextureNodeRef a_texture,
    const wchar_t* a_addressingModeU, const wchar_t* a_addressingModeV, const float a_tileU,
    const float a_tileV, const float a_inputGamma, const wchar_t* a_inputAlpha,
    const wchar_t* a_texApplyMode)
  {
    HRLightRef sky = hrLightCreate(a_name);

    hrLightOpen(sky, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(sky);
      lightNode.attribute(L"type").set_value(L"sky");
      lightNode.attribute(L"distribution") = a_distribution;
      
      auto intensityNode = lightNode.append_child(L"intensity");
      intensityNode.append_child(L"color").append_attribute(L"val")      = a_color;
      intensityNode.append_child(L"multiplier").append_attribute(L"val") = a_multiplier;

      if (std::wstring(a_distribution) == L"perez")
      {
        auto sunModel = lightNode.append_child(L"perez");
        sunModel.append_attribute(L"sun_id")    = a_sunId;
        sunModel.append_attribute(L"turbidity") = a_turbidity;
      }
      else if (std::wstring(a_distribution) == L"map")
      {
        auto texNode = hrTextureBind(a_texture, intensityNode.child(L"color"));

        texNode.append_attribute(L"matrix");
        float samplerMatrix[16] = { a_tileU, 0,       0,       0,
                                    0,       a_tileV, 0,       0,
                                    0,       0,       1,       0,
                                    0,       0,       0,       1 };

        texNode.append_attribute(L"addressing_mode_u") = a_addressingModeU;
        texNode.append_attribute(L"addressing_mode_v") = a_addressingModeV;
        texNode.append_attribute(L"input_gamma")       = a_inputGamma;
        texNode.append_attribute(L"input_alpha")       = a_inputAlpha;

        HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);
      }
      VERIFY_XML(lightNode);
    }
    hrLightClose(sky);

    return sky;
  }


  ResultTest::ResultTest(const std::wstring a_name, const std::vector<bool> a_res, const bool a_skip, const std::vector<float> a_mse,
    const float a_rendTime, const std::vector<std::wstring>& a_linkRefImgs, const std::vector<std::wstring>& a_linkRenderImgs)
  {
    m_nameTest = a_name;

    if (a_skip)
    {
      m_result       = L"skipped";
      m_resultHtml   = L"&#128465;"; // trash can
      m_mse          = L"-";
      m_mseHtml      = L"-";
      m_renderTime   = L"-";
    }
    else
    {
      m_renderTime = std::to_wstring(a_rendTime).substr(0, 4) + L" sec.";

      bool createTable = false;

      for (int i = 0; i < a_res.size(); ++i)
      {
        std::wstring mse = L"-";

        if (a_mse.size() == a_res.size())        
          mse = std::to_wstring(a_mse[i]).substr(0, 4);
        
        m_mse     += (mse + L", ");
        m_mseHtml += (mse + L"<br>");

        if (a_res[i] == true)
        {                
          m_result       += L"ok, ";
          m_resultHtml   += L"&#10004;<br>"; // check mark
        }
        else
        {

          m_result       += L"FAILED, ";
          m_resultHtml   += L"&#10060;<br>"; // red cross 

          if (!createTable)
          {
            m_linkRefImgs += L"\n";
            m_linkRefImgs += L"    <table>\n";
          }

          m_linkRefImgs  += L"    <td class=\"td-vert\">\n";
          m_linkRefImgs  += L"        <img src = " + a_linkRefImgs[i] + L">\n";
          m_linkRefImgs  += L"     </td>\n";

          if (!createTable)
          {
            m_linkRendImgs += L"\n";
            m_linkRendImgs += L"    <table>\n";
            createTable     = true;
          }

          m_linkRendImgs += L"    <td class=\"td-vert\">\n";
          m_linkRendImgs += L"        <img class=\"imageTop\" src = " + a_linkRenderImgs[i] + L">\n";
          m_linkRendImgs += L"        <img src = " + a_linkRefImgs[i] + L">\n";
          m_linkRendImgs += L"    </td>\n";
        }
      }

      if (createTable)
      {
        m_linkRefImgs  += L"    </table>\n";
        m_linkRendImgs += L"    </table>\n";
      }
    }
  }




  void CreateHtml(std::wofstream& a_fileOut)
  {
    a_fileOut << "<!DOCTYPE html>\n";
    a_fileOut << "<html>\n";
    a_fileOut << "<head>\n";
    a_fileOut << "<link rel = \"stylesheet\" href = \"reportStyle.css\">\n";
    a_fileOut << "</head>\n\n";
  }


  void AddTextHtml(const std::wstring& a_text, std::wofstream& a_fileOut, const int a_size)
  {
    a_fileOut << L"<h" << std::to_wstring(a_size) << L">" << a_text << L"</h" << std::to_wstring(a_size) << L">" << std::endl;
  }


  void CreateHtmlHeaderTable(const std::vector<std::wstring>& a_header, std::wofstream& a_fileOut)
  {
    a_fileOut << "<table>\n";    
    a_fileOut << "  <tr>\n";

    for (auto head : a_header) 
      a_fileOut << "    <th>" << head.c_str() << "</th>\n";

    a_fileOut << "  </tr>" << std::endl;     
  }



  void AddRowHtmlTable(ResultTest a_data, std::wofstream& a_fileOut)
  {
    a_fileOut << "  <tr>\n";
    a_fileOut << "    <td>" << a_data.GetName()          << "</td>\n";    
    a_fileOut << "    <td>" << a_data.GetStrResultHtml() << "</td>\n";
    a_fileOut << "    <td>" << a_data.GetMseHtml()       << "</td>\n";
    a_fileOut << "    <td>" << a_data.GetRendTime()      << "</td>\n";
    a_fileOut << "    <td>" << a_data.GetLinkRef()       << "</td>\n";
    a_fileOut << "    <td>" << a_data.GetLinkRend()      << "</td>\n";
    a_fileOut << "  </tr>"  << std::endl;     
  }


  void CloseHtmlTable(std::wofstream& a_fileOut)
  {
    a_fileOut << "</table>" << std::endl;    
  }


  void CloseHtml(std::wofstream& a_fileOut)
  {
    a_fileOut << "</html>";
    a_fileOut.flush();
    a_fileOut.close();
  }


  HRMeshRef CreateTriStrip(int rows, int cols, float size)
  {
    //int numIndices = 2 * cols*(rows - 1) + rows - 1;

    std::vector<float> vertices_vec;
    vertices_vec.reserve(rows * cols * 4);

    std::vector<float> normals_vec;
    normals_vec.reserve(rows * cols * 4);

    std::vector<float> texcoords_vec;
    texcoords_vec.reserve(rows * cols * 2);

    std::vector<float3> normals_vec_tmp(rows * cols, float3(0.0f, 0.0f, 0.0f));

    std::vector<int> indices_vec;
    std::vector<int> mind_vec;

    for (int z = 0; z < rows; ++z)
    {
      for (int x = 0; x < cols; ++x)
      {
        float xx = -size / 2 + x*size / cols;
        float zz = -size / 2 + z*size / rows;
        float yy = 0.0f;
        //float r = sqrt(xx*xx + zz*zz);
        //float yy = 5.0f * (r != 0.0f ? sin(r) / r : 1.0f);

        vertices_vec.push_back(xx);
        vertices_vec.push_back(yy);
        vertices_vec.push_back(zz);
        vertices_vec.push_back(1.0f);

        texcoords_vec.push_back(x / float(cols - 1));
        texcoords_vec.push_back(z / float(rows - 1));
      }
    }

    int numTris = 0;
    for (int z = 0; z < rows - 1; ++z)
    {
      for (int x = 0; x < cols - 1; ++x)
      {
        unsigned int offset = z * cols + x;

        indices_vec.push_back(offset + 0);
        indices_vec.push_back(offset + cols);
        indices_vec.push_back(offset + 1);
        indices_vec.push_back(offset + cols);
        indices_vec.push_back(offset + cols + 1);
        indices_vec.push_back(offset + 1);

        float3 A(vertices_vec.at(4 * offset + 0), vertices_vec.at(4 * offset + 1),
                 vertices_vec.at(4 * offset + 2));
        float3 B(vertices_vec.at(4 * (offset + cols) + 0), vertices_vec.at(4 * (offset + cols) + 1),
                 vertices_vec.at(4 * (offset + cols) + 2));
        float3 C(vertices_vec.at(4 * (offset + 1) + 0), vertices_vec.at(4 * (offset + 1) + 1),
                 vertices_vec.at(4 * (offset + 1) + 2));
        float3 D(vertices_vec.at(4 * (offset + cols + 1) + 0), vertices_vec.at(4 * (offset + cols + 1) + 1),
                 vertices_vec.at(4 * (offset + cols + 1) + 2));

        float3 edge1B(normalize(B - A));
        float3 edge1C(normalize(C - A));

        float3 face_normal1 = cross(edge1B, edge1C);

        float3 edge2D(normalize(D - B));
        float3 edge2C(normalize(C - B));

        float3 face_normal2 = cross(edge2D, edge2C);

        normals_vec_tmp.at(offset) += face_normal1;
        normals_vec_tmp.at(offset + cols) += face_normal1 + face_normal2;
        normals_vec_tmp.at(offset + 1) += face_normal1 + face_normal2;
        normals_vec_tmp.at(offset + cols + 1) += face_normal2;

        numTris += 2;
      }
    }


    ///////////////////////
    for (auto& N : normals_vec_tmp)
    {
      N = normalize(N);
      normals_vec.push_back(N.x);
      normals_vec.push_back(N.y);
      normals_vec.push_back(N.z);
      normals_vec.push_back(1.0f);
    }

    ///////////////////////
    for(int i = 0; i < indices_vec.size() / 3; ++i)
    {
      mind_vec.push_back(0/*i % 5 == 0 ? 0 : 1*/);
    }


    HRMeshRef meshRef = hrMeshCreate(L"tri_strip");

    hrMeshOpen(meshRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
    {
      hrMeshVertexAttribPointer4f(meshRef, L"pos", &vertices_vec[0]);
      hrMeshVertexAttribPointer4f(meshRef, L"norm", &normals_vec[0]);
      hrMeshVertexAttribPointer2f(meshRef, L"texcoord", &texcoords_vec[0]);

      //hrMeshMaterialId(cubeRef, 0);
      hrMeshPrimitiveAttribPointer1i(meshRef, L"mind", &mind_vec[0]);
      hrMeshAppendTriangles3(meshRef, int(indices_vec.size()), &indices_vec[0]);
    }
    hrMeshClose(meshRef);

    return meshRef;
  }


  void RenderProgress(HRRenderRef& a_renderRef)
  {
    std::cout << std::endl;

    while (true)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));

      auto info = hrRenderHaveUpdate(a_renderRef);

      if (info.haveUpdateFB)
      {
        auto pres = std::cout.precision(2);
        std::cout << "rendering progress = " << info.progress << "% \r"; std::cout.flush();
        std::cout.precision(pres);
      }

      if (info.finalUpdate)
      {
        std::cout << "                                           \r";
        break;
      }
    }
  }
}