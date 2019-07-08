#pragma once

#include "HydraRenderDriverAPI.h"

#ifdef WIN32
  #include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>

#include <vector>
#include <string>
#include <sstream>
#include <fstream>

#include "LiteMath.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct RD_OGL1_Plain : public IHRRenderDriver
{
  RD_OGL1_Plain();

  void GetRenderDriverName(std::wstring &name) override { name = std::wstring(L"opengl1");};
  void              ClearAll() override;
  HRDriverAllocInfo AllocAll(HRDriverAllocInfo a_info) override;
       
  bool UpdateImage(int32_t a_texId, int32_t w, int32_t h, int32_t bpp, const void* a_data, pugi::xml_node a_texNode) override;
  bool UpdateMaterial(int32_t a_matId, pugi::xml_node a_materialNode) override;
  bool UpdateLight(int32_t a_lightIdId, pugi::xml_node a_lightNode) override;
  bool UpdateMesh(int32_t a_meshId, pugi::xml_node a_meshNode, const HRMeshDriverInput& a_input, const HRBatchInfo* a_batchList, int32_t listSize) override;
       
  bool UpdateImageFromFile(int32_t a_texId, const wchar_t* a_fileName, pugi::xml_node a_texNode) override { return false; }
  bool UpdateMeshFromFile(int32_t a_meshId, pugi::xml_node a_meshNode, const wchar_t* a_fileName) override { return false; }

       
  bool UpdateCamera(pugi::xml_node a_camNode) override;
  bool UpdateSettings(pugi::xml_node a_settingsNode) override;

  /////////////////////////////////////////////////////////////////////////////////////////////

  void BeginScene(pugi::xml_node a_sceneNode) override;
  void EndScene() override;
  void InstanceMeshes(int32_t a_mesh_id, const float* a_matrices, int32_t a_instNum, const int* a_lightInstId, const int* a_remapId, const int* a_realInstId) override;
  void InstanceLights(int32_t a_light_id, const float* a_matrix, pugi::xml_node* a_custAttrArray, int32_t a_instNum, int32_t a_lightGroupId) override;

  void Draw() override;

  HRRenderUpdateInfo HaveUpdateNow(int a_maxRaysPerPixel) override;

  void GetFrameBufferHDR(int32_t w, int32_t h, float*   a_out, const wchar_t* a_layerName) override;
  void GetFrameBufferLDR(int32_t w, int32_t h, int32_t* a_out) override;

  void GetGBufferLine(int32_t a_lineNumber, HRGBufferPixel* a_lineData, int32_t a_startX, int32_t a_endX, const std::unordered_set<int32_t>& a_shadowCatchers) override {}

  const HRRenderDeviceInfoListElem* DeviceList() const override { return nullptr; } //#TODO: implement quering GPU info bu glGetString(GL_VENDOR) and e.t.c.
  bool EnableDevice(int32_t id, bool a_enable) override { return true; }

protected:

  std::wstring m_libPath;

  GLuint  m_displayLists;
  GLsizei m_listNum;

  std::vector<GLuint> m_texturesList;

  // camera parameters
  //
  float camPos[3];
  float camLookAt[3];
  float camUp[3];

  float camFov;
  float camNearPlane;
  float camFarPlane;
  int   m_width;
  int   m_height;

  HydraLiteMath::float4x4 camWorldViewMartrixTransposed;
  HydraLiteMath::float4x4 camProjMatrixTransposed;
  bool                    camUseMatrices;

  std::vector<float> m_diffColors;
  std::vector<int>   m_diffTexId;

};

static inline void mat4x4_transpose(float M[16], const float N[16])
{
  for (int j = 0; j < 4; j++)
  {
    for (int i = 0; i < 4; i++)
      M[i * 4 + j] = N[j * 4 + i];
  }
}


struct RD_OGL1_Debug : public RD_OGL1_Plain
{
  RD_OGL1_Debug()
  {
    m_renderNormalLength = 0.5f;
    m_drawNormals        = true;
    m_drawTangents       = false;
    m_drawSolid          = true;
    m_drawWire           = false;
    m_drawAxis           = false;

    m_axisArrorLen       = 1.0f;
    m_axisArrorThickness = 0.1f;
    m_meshNum            = 0;
  }

  void GetRenderDriverName(std::wstring &name) override { name = std::wstring(L"opengl1Debug");};

  void ClearAll() override;
  HRDriverAllocInfo AllocAll(HRDriverAllocInfo a_info) override;

  bool UpdateMesh(int32_t a_meshId, pugi::xml_node a_meshNode, const HRMeshDriverInput& a_input,
      const HRBatchInfo* a_batchList, int32_t listSize) override;

  bool UpdateSettings(pugi::xml_node a_settingsNode) override;

  /////////////////////////////////////////////////////////////////////////////////////////////

  void InstanceMeshes(int32_t a_mesh_id, const float* a_matrices, int32_t a_instNum, const int* a_lightInstId,
      const int* a_remapId, const int* a_realInstId) override;

  void EndScene() override;

protected:
  unsigned int m_meshNum;
  GLfloat m_renderNormalLength;

  bool m_drawSolid;
  bool m_drawWire;
  bool m_drawAxis;
  bool m_drawNormals;
  bool m_drawTangents;

  float m_axisArrorLen;
  float m_axisArrorThickness;
};

struct RD_OGL1_ShowCustomAttr : public RD_OGL1_Plain
{
  RD_OGL1_ShowCustomAttr() = default;

  void GetRenderDriverName(std::wstring &name) override { name = std::wstring(L"opengl1TestCustomAttributes");};

  bool UpdateMesh(int32_t a_meshId, pugi::xml_node a_meshNode, const HRMeshDriverInput& a_input,
      const HRBatchInfo* a_batchList, int32_t listSize) override;

protected:
  
};


