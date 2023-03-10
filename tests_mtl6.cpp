//
// Created by frol on 9/27/19.
//

#include "tests.h"

#if defined(WIN32)
#include <FreeImage.h>
#include <GLFW/glfw3.h>
#pragma comment(lib, "glfw3dll.lib")
#pragma comment(lib, "FreeImage.lib")
#else
#include <FreeImage.h>
#include <GLFW/glfw3.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sstream>

#include "HR_HDRImageTool.h"
#include "LiteMath.h"
#include "HydraXMLHelpers.h"

///////////////////////////////////////////////////////////////////////////////////

using namespace TEST_UTILS;

using LiteMath::float2;
using LiteMath::float3;
using LiteMath::float4;

namespace hlm = LiteMath;

struct InputMaterialTestParams
{
  std::wstring testName;
  std::wstring paramName;
  std::wstring brdfName;
  float values[4];
  float maxMSE;
  float initialGlosiness;
  float initialAniso;
  float initialRot;
};


bool PefrormMaterialTest(const InputMaterialTestParams& testParams)
{
  std::wstring nameTest                = testParams.testName;
  std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
  std::filesystem::path saveRenderFile1 = L"tests_images/" + nameTest + L"/z_out.png";
  std::filesystem::path saveRenderFile2 = L"tests_images/" + nameTest + L"/z_out2.png";
  std::filesystem::path saveRenderFile3 = L"tests_images/" + nameTest + L"/z_out3.png";
  std::filesystem::path saveRenderFile4 = L"tests_images/" + nameTest + L"/z_out4.png";
  
  hrErrorCallerPlace(nameTest.c_str());
  hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);

  ////////////////////
  // Materials
  ////////////////////
  
  
  
  SimpleMesh sphere   = CreateSphere(2.5f, 128);
  SimpleMesh cubeOpen = CreateCubeOpen(4.0f);
  
  HRMaterialRef mat0 = hrMaterialCreate(L"mysimplemat");
  HRMaterialRef mat1 = hrMaterialCreate(L"red");
  HRMaterialRef mat2 = hrMaterialCreate(L"green");
  HRMaterialRef mat3 = hrMaterialCreate(L"white");
  HRMaterialRef mat4 = hrMaterialCreate(L"glossy");
  
  hrMaterialOpen(mat0, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat0);
    AddDiffuseNode(matNode, L"0.5 0.5 0.5");
  }
  hrMaterialClose(mat0);
  
  hrMaterialOpen(mat1, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat1);
    AddDiffuseNode(matNode, L"0.5 0.0 0.0");
  }
  hrMaterialClose(mat1);
  
  hrMaterialOpen(mat2, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat2);
    AddDiffuseNode(matNode, L"0.0 0.5 0.0");    
  }
  hrMaterialClose(mat2);
  
  hrMaterialOpen(mat3, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat3);
    AddDiffuseNode(matNode, L"0.5 0.5 0.5");
  }
  hrMaterialClose(mat3);
  
  hrMaterialOpen(mat4, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat4);    
    auto refl        = matNode.append_child(L"reflectivity");    

    refl.append_attribute(L"brdf_type")                       = testParams.brdfName.c_str();
    refl.append_child(L"color").append_attribute(L"val")      = L"1.0 1.0 1.0";  
    refl.append_child(L"extrusion").append_attribute(L"val")  = L"maxcolor";
    refl.append_child(L"fresnel").append_attribute(L"val")    = 0;    
    refl.append_child(L"glossiness").append_attribute(L"val") = testParams.initialGlosiness;
    refl.append_child(L"anisotropy").append_attribute(L"val") = testParams.initialAniso;
    refl.child(L"anisotropy").append_attribute(L"rot")        = testParams.initialRot;    
    VERIFY_XML(matNode);
  }
  hrMaterialClose(mat4);
  
  ////////////////////
  // Meshes
  ////////////////////

  auto cubeOpenRef = hrMeshCreate(L"my_box");
  auto sphereRef   = hrMeshCreate(L"my_sphere");
  
  hrMeshOpen(cubeOpenRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
  {
    hrMeshVertexAttribPointer4f(cubeOpenRef, L"pos", &cubeOpen.vPos[0]);
    hrMeshVertexAttribPointer4f(cubeOpenRef, L"norm", &cubeOpen.vNorm[0]);
    hrMeshVertexAttribPointer2f(cubeOpenRef, L"texcoord", &cubeOpen.vTexCoord[0]);
    
    int cubeMatIndices[10] = { mat3.id, mat3.id, mat3.id, mat3.id, mat3.id, mat3.id, mat2.id, mat2.id, mat1.id, mat1.id };
    
    //hrMeshMaterialId(cubeRef, 0);
    hrMeshPrimitiveAttribPointer1i(cubeOpenRef, L"mind", cubeMatIndices);
    hrMeshAppendTriangles3(cubeOpenRef, int(cubeOpen.triIndices.size()), &cubeOpen.triIndices[0]);
  }
  hrMeshClose(cubeOpenRef);
  
  hrMeshOpen(sphereRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
  {
    hrMeshVertexAttribPointer4f(sphereRef, L"pos", &sphere.vPos[0]);
    hrMeshVertexAttribPointer4f(sphereRef, L"norm", &sphere.vNorm[0]);
    hrMeshVertexAttribPointer2f(sphereRef, L"texcoord", &sphere.vTexCoord[0]);
    
    for (size_t i = 0; i < sphere.matIndices.size(); i++)
      sphere.matIndices[i] = mat4.id;
    
    hrMeshPrimitiveAttribPointer1i(sphereRef, L"mind", &sphere.matIndices[0]);
    hrMeshAppendTriangles3(sphereRef, int32_t(sphere.triIndices.size()), &sphere.triIndices[0]);
  }
  hrMeshClose(sphereRef);
  
  ////////////////////
  // Light
  ////////////////////

  auto rectLight = CreateLight(L"Light01", L"area", L"rect", L"diffuse", 1, 1, L"1 1 1", 8.0f * IRRADIANCE_TO_RADIANCE);
    
  ////////////////////
  // Camera
  ////////////////////

  CreateCamera(45, L"0 0 14", L"0 0 0");
  
  ////////////////////
  // Render settings
  ////////////////////

  auto renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 512, 512, 256, 4096);

  ////////////////////
  // Create scene
  ////////////////////

  auto scnRef = hrSceneCreate((L"scene_" + nameTest).c_str());

  hrSceneOpen(scnRef, HR_WRITE_DISCARD);

  AddMeshToScene(scnRef, sphereRef, float3(0, -1.5F, 1));
  AddMeshToScene(scnRef, cubeOpenRef, float3(), float3(0, 180, 0));
  AddLightToScene(scnRef, rectLight, float3(0, 3.85F, 0));    
  
  hrSceneClose(scnRef);
  
  
  const std::wstring names[4] = { saveRenderFile1.c_str(), saveRenderFile2.c_str(), 
    saveRenderFile3.c_str(), saveRenderFile4.c_str()};
  
  for(int iter = 0; iter < 4; iter++)
  {
    hrMaterialOpen(mat4, HR_OPEN_EXISTING);
    {
      xml_node matNode = hrMaterialParamNode(mat4);
      auto refl        = matNode.child(L"reflectivity");

      if(testParams.paramName != L"rot")
        refl.child(testParams.paramName.c_str()).attribute(L"val") = testParams.values[iter];        
      else
        refl.child(L"anisotropy").attribute(L"rot") = testParams.values[iter];
      VERIFY_XML(matNode);
    }

    hrMaterialClose(mat4);       
    hrFlush(scnRef, renderRef);
    
    RenderProgress(renderRef);
        
    hrRenderSaveFrameBufferLDR(renderRef, names[iter].c_str());    
  }
    
  return check_images(ws2s(nameTest).c_str(), 4, testParams.maxMSE);
}


bool MTL_TESTS::test_175_beckman_isotropic()
{
  InputMaterialTestParams params;
  params.testName         = L"test_175";
  params.brdfName         = L"beckmann";
  params.paramName        = L"glossiness";
  params.values[0]        = 0.25f;
  params.values[1]        = 0.5f;
  params.values[2]        = 0.75f;
  params.values[3]        = 0.90f;
  params.maxMSE           = 30.0f;
  params.initialGlosiness = 0.0f;
  params.initialAniso     = 0.0f;
  params.initialRot       = 0.0f;
  
  return PefrormMaterialTest(params);
}


bool MTL_TESTS::test_176_beckman_anisotropic()
{
  InputMaterialTestParams params;
  params.testName         = L"test_176";
  params.brdfName         = L"beckmann";
  params.paramName        = L"anisotropy";
  params.values[0]        = 0.25f;
  params.values[1]        = 0.5f;
  params.values[2]        = 0.75f;
  params.values[3]        = 1.0f;
  params.maxMSE           = 30.0f;
  params.initialGlosiness = 0.0f;
  params.initialAniso     = 0.0f;
  params.initialRot       = 0.0f;
  
  return PefrormMaterialTest(params);
}


bool MTL_TESTS::test_177_beckman_aniso_rot()
{
  InputMaterialTestParams params;
  params.testName         = L"test_177";
  params.brdfName         = L"beckmann";
  params.paramName        = L"rot";
  params.values[0]        = 0.25f;
  params.values[1]        = 0.5f;
  params.values[2]        = 0.75f;
  params.values[3]        = 1.0f;
  params.maxMSE           = 30.0f;
  params.initialGlosiness = 0.0f;
  params.initialAniso     = 1.0f;
  params.initialRot       = 0.0f;
  
  return PefrormMaterialTest(params);
}


bool MTL_TESTS::test_178_trggx_isotropic()
{
  InputMaterialTestParams params;
  params.testName         = L"test_178";
  params.brdfName         = L"trggx";
  params.paramName        = L"glossiness";
  params.values[0]        = 0.25f;
  params.values[1]        = 0.5f;
  params.values[2]        = 0.75f;
  params.values[3]        = 0.90f;
  params.maxMSE           = 30.0f;
  params.initialGlosiness = 0.0f;
  params.initialAniso     = 0.0f;
  params.initialRot       = 0.0f;
  
  return PefrormMaterialTest(params);
}


bool MTL_TESTS::test_179_trggx_anisotropic()
{
  InputMaterialTestParams params;
  params.testName         = L"test_179";
  params.brdfName         = L"trggx";
  params.paramName        = L"anisotropy";
  params.values[0]        = 0.25f;
  params.values[1]        = 0.5f;
  params.values[2]        = 0.75f;
  params.values[3]        = 0.90f;
  params.maxMSE           = 30.0f;
  params.initialGlosiness = 0.0f;
  params.initialAniso     = 0.0f;
  params.initialRot       = 0.0f;
  
  return PefrormMaterialTest(params);
}


bool MTL_TESTS::test_180_trggx_aniso_rot()
{
  InputMaterialTestParams params;
  params.testName         = L"test_180";
  params.brdfName         = L"trggx";
  params.paramName        = L"rot";
  params.values[0]        = 0.25f;
  params.values[1]        = 0.5f;
  params.values[2]        = 0.75f;
  params.values[3]        = 1.0f;
  params.maxMSE           = 30.0f;
  params.initialGlosiness = 0.0f;
  params.initialAniso     = 1.0f;
  params.initialRot       = 0.0f;
  
  return PefrormMaterialTest(params);
}