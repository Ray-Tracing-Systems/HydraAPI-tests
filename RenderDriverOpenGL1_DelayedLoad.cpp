#include "RenderDriverOpenGL1.h"
#include <iostream>

#include "HydraObjectManager.h"

#pragma warning(disable:4996)

#if defined(WIN32)
#include "FreeImage.h"
#pragma comment(lib, "FreeImage.lib")
#else
#include <FreeImage.h>
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct RD_OGL1_Plain_DelayedLoad : public RD_OGL1_Plain
{
public:

  void GetRenderDriverName(std::wstring &name) override { name = std::wstring(L"opengl1DelayedLoad");};
  RD_OGL1_Plain_DelayedLoad() = default;
  ~RD_OGL1_Plain_DelayedLoad() = default;

//  HRDriverInfo Info() override;

  bool UpdateImage(int32_t a_texId, int32_t w, int32_t h, int32_t bpp, int32_t chan, const void* a_data, pugi::xml_node a_texNode)  override;
  bool UpdateImageFromFile(int32_t a_texId, const wchar_t* a_fileName, pugi::xml_node a_texNode) override;

protected:

};


IHRRenderDriver* CreateOpenGL1_DelayedLoad_RenderDriver()
{
  return new RD_OGL1_Plain_DelayedLoad;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


extern HRObjectManager g_objManager;

bool RD_OGL1_Plain_DelayedLoad::UpdateImageFromFile(int32_t a_texId, const wchar_t* a_fileName, pugi::xml_node a_texNode)
{
  const wchar_t* filename = a_fileName;

  std::wstring fname2(filename);

  if (fname2.find(L".image4ub") != std::wstring::npos)
  {
    std::ifstream fin;
    hr_ifstream_open(fin, a_fileName);
    
    if (fin.is_open())
    {
      int32_t wh[2];
      fin.read((char*)wh, sizeof(int32_t) * 2);

      size_t sizeInBytes = size_t(wh[0]) * size_t(wh[1]) * size_t(sizeof(int));

      std::vector<char> data(size_t(wh[0]) * size_t(wh[1]) * size_t(sizeof(int)) + size_t(16));
      if (data.empty())
        return false;

      fin.read(data.data(), sizeInBytes);
      return UpdateImage(a_texId, wh[0], wh[1], 4, 4, data.data(), a_texNode);
    }
    else
      return false;
  }
  else
  {
    int width, height, bpp, chan;
    std::vector<unsigned char> tmpBuf;
    bool loaded = g_objManager.m_pImgTool->LoadImageFromFile(filename, width, height, bpp, chan, tmpBuf);

    const bool res = UpdateImage(a_texId, width, height, bpp, chan, tmpBuf.data(), a_texNode);

    if (g_objManager.m_tempBuffer.size() > TEMP_BUFFER_MAX_SIZE_DONT_FREE)
      g_objManager.m_tempBuffer = g_objManager.EmptyBuffer();

    return res;
  }
}

bool RD_OGL1_Plain_DelayedLoad::UpdateImage(int32_t a_texId, int32_t w, int32_t h, int32_t bpp, int32_t chan, const void* a_data, pugi::xml_node a_texNode)
{
  if (a_data == nullptr)
    return false;

  if (bpp != 4 && chan != 4) // well, perhaps this is not error, we just don't support hdr textures in this render
    return true;

  glBindTexture(GL_TEXTURE_2D, m_texturesList[a_texId]);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, a_data);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  //glGenerateMipmap(GL_TEXTURE_2D); // this function is from OpenGL 3.0
  gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, w, h, GL_RGBA, GL_UNSIGNED_BYTE, a_data);

  return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct RD_OGL1_Plain_DelayedLoad2 : public RD_OGL1_Plain_DelayedLoad
{
public:

  RD_OGL1_Plain_DelayedLoad2() = default;
  ~RD_OGL1_Plain_DelayedLoad2() = default;

    void GetRenderDriverName(std::wstring &name) override { name = std::wstring(L"opengl1DelayedLoad2");};

//  HRDriverInfo Info() override;

  bool UpdateMesh(int32_t a_meshId, pugi::xml_node a_meshNode, const HRMeshDriverInput& a_input, const HRBatchInfo* a_batchList, int32_t a_listSize) override;
  bool UpdateMeshFromFile(int32_t a_meshId, pugi::xml_node a_meshNode, const wchar_t* a_fileName) override;

protected:

};

IHRRenderDriver* CreateOpenGL1_DelayedLoad_RenderDriver2()
{
  return new RD_OGL1_Plain_DelayedLoad2;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



std::vector<HRBatchInfo> CreateBatchesArrayRLE(const int32_t* matIndices, int32_t matIndicesSize)
{
  std::vector<HRBatchInfo> matDrawList;

  if (matIndicesSize == 0)
    return matDrawList;

  matDrawList.reserve(matIndicesSize / 4);

  int32_t tBegin = 0;
  int32_t tMatId = matIndices[0];

  for (size_t i = 0; i < matIndicesSize; i++)
  {
    int32_t mid = matIndices[i];

    if (matIndices[tBegin] != mid || (i == matIndicesSize - 1))
    {
      // append current tri sequence withe the same material id
      //
      HRBatchInfo elem;
      elem.matId    = tMatId;
      elem.triBegin = tBegin;
      elem.triEnd   = int32_t(i);

      if (i == matIndicesSize - 1)
        elem.triEnd = int32_t(matIndicesSize);

      matDrawList.push_back(elem);

      // save begin and material id for next  tri sequence withe the same material id
      //
      tBegin = elem.triEnd;
      tMatId = mid;
    }
  }

  return matDrawList;
}


bool RD_OGL1_Plain_DelayedLoad2::UpdateMeshFromFile(int32_t a_meshId, pugi::xml_node a_meshNode, const wchar_t* a_fileName)
{
  static bool wasThere = false;

  if (!wasThere)
  {
    std::cout << "call of OGLRD::UpdateMeshFromFile (first time)" << std::endl;
    wasThere = true;
  }

  //uint64_t dataOffset  = a_meshNode.attribute(L"offset").as_ullong();
  uint64_t sizeInBytes = a_meshNode.attribute(L"bytesize").as_ullong();

  std::vector<char> tempBuffer(size_t(sizeInBytes + uint64_t(16)));
  char* dataPtr = tempBuffer.data();

  const std::wstring path = m_libPath + std::wstring(L"/") + a_meshNode.attribute(L"loc").as_string();
  
  std::ifstream fin;
  hr_ifstream_open(fin, path.c_str());
  
  fin.read(dataPtr, sizeInBytes);

  HRMeshDriverInput input;

  uint64_t offsetPos  = a_meshNode.child(L"positions").attribute(L"offset").as_ullong();
  uint64_t offsetNorm = a_meshNode.child(L"normals").attribute(L"offset").as_ullong();
  uint64_t offsetTexc = a_meshNode.child(L"texcoords").attribute(L"offset").as_ullong();
  uint64_t offsetInd  = a_meshNode.child(L"indices").attribute(L"offset").as_ullong();
  uint64_t offsetMInd = a_meshNode.child(L"matindices").attribute(L"offset").as_ullong();

  input.vertNum       = a_meshNode.attribute(L"vertNum").as_int();
  input.triNum        = a_meshNode.attribute(L"triNum").as_int();

  input.pos4f         = (float*)(dataPtr + offsetPos);
  input.norm4f        = (float*)(dataPtr + offsetNorm);
  input.texcoord2f    = (float*)(dataPtr + offsetTexc);
  input.indices       = (int*)  (dataPtr + offsetInd);
  input.triMatIndices = (int*)  (dataPtr + offsetMInd);

  std::vector<HRBatchInfo> batches = CreateBatchesArrayRLE(input.triMatIndices, input.triNum);

  return UpdateMesh(a_meshId, a_meshNode, input, &batches[0], int32_t(batches.size()));
}

bool RD_OGL1_Plain_DelayedLoad2::UpdateMesh(int32_t a_meshId, pugi::xml_node a_meshNode, const HRMeshDriverInput& a_input, const HRBatchInfo* a_batchList, int32_t a_listSize)
{
  if (a_input.triNum == 0) // don't support loading mesh from file 'a_fileName'
    return false;

  bool invalidMaterial = m_diffTexId.empty();

  // DebugPrintMesh(a_input, "z_mesh.txt");

  glNewList(m_displayLists + GLuint(a_meshId), GL_COMPILE);

  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);

  glVertexPointer(4, GL_FLOAT, 0, a_input.pos4f);
  glNormalPointer(GL_FLOAT, sizeof(float) * 4, a_input.norm4f);
  glTexCoordPointer(2, GL_FLOAT, 0, a_input.texcoord2f);

  for (int32_t batchId = 0; batchId < a_listSize; batchId++)
  {
    HRBatchInfo batch = a_batchList[batchId];

    if (!invalidMaterial)
    {
      if (m_diffTexId[batch.matId] >= 0)
      {
        int texId = m_diffTexId[batch.matId];
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, m_texturesList[texId]);
      }
      else
        glDisable(GL_TEXTURE_2D);

      glColor3fv(&m_diffColors[batch.matId * 3 + 0]);
    }
    else
    {
      glDisable(GL_TEXTURE_2D);
      glColor3f(1.0f, 1.0f, 1.0f);
    }
    const int drawElementsNum = batch.triEnd - batch.triBegin;

    glDrawElements(GL_TRIANGLES, drawElementsNum * 3, GL_UNSIGNED_INT, a_input.indices + batch.triBegin * 3);
  }

  glEndList();

  return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
