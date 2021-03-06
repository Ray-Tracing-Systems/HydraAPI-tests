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

using LiteMath::float2;
using LiteMath::float3;
using LiteMath::float4;

namespace hlm = LiteMath;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
  hrErrorCallerPlace(testParams.testName.c_str());
  
  const std::wstring pathToSceneLib = std::wstring(L"tests_f/") + testParams.testName;
  
  hrSceneLibraryOpen(pathToSceneLib.c_str(), HR_WRITE_DISCARD);
  
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
    xml_node diff    = matNode.append_child(L"diffuse");
    
    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val") = L"0.5 0.5 0.5";
  }
  hrMaterialClose(mat0);
  
  hrMaterialOpen(mat1, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat1);
    xml_node diff = matNode.append_child(L"diffuse");
    
    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val") = L"0.5 0.0 0.0";
  }
  hrMaterialClose(mat1);
  
  hrMaterialOpen(mat2, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat2);
    xml_node diff = matNode.append_child(L"diffuse");
    
    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val").set_value(L"0.0 0.5 0.0");
  }
  hrMaterialClose(mat2);
  
  hrMaterialOpen(mat3, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat3);
    xml_node diff = matNode.append_child(L"diffuse");
    
    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 0.5");
  }
  hrMaterialClose(mat3);
  
  hrMaterialOpen(mat4, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat4);
    
    auto refl        = matNode.append_child(L"reflectivity");
    
    refl.append_attribute(L"brdf_type") = testParams.brdfName.c_str();
    refl.append_child(L"color").append_attribute(L"val")      = L"1.0 1.0 1.0";
  
    refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
    refl.append_child(L"fresnel").append_attribute(L"val").set_value(0);
    
    refl.append_child(L"glossiness").append_attribute(L"val") = testParams.initialGlosiness;
    refl.append_child(L"anisotropy").append_attribute(L"val") = testParams.initialAniso;
    refl.child(L"anisotropy").append_attribute(L"rot") = testParams.initialRot;
    
    VERIFY_XML(matNode);
  }
  hrMaterialClose(mat4);
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  
  HRMeshRef cubeOpenRef = hrMeshCreate(L"my_box");
  HRMeshRef sphereRef   = hrMeshCreate(L"my_sphere");
  
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
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  
  HRLightRef rectLight = hrLightCreate(L"my_area_light");
  
  hrLightOpen(rectLight, HR_WRITE_DISCARD);
  {
    pugi::xml_node lightNode = hrLightParamNode(rectLight);
    
    lightNode.attribute(L"type").set_value(L"area");
    lightNode.attribute(L"shape").set_value(L"rect");
    lightNode.attribute(L"distribution").set_value(L"diffuse");
    
    pugi::xml_node sizeNode = lightNode.append_child(L"size");
    
    sizeNode.append_attribute(L"half_length") = 1.0f;
    sizeNode.append_attribute(L"half_width")  = 1.0f;
    
    pugi::xml_node intensityNode = lightNode.append_child(L"intensity");
    
    intensityNode.append_child(L"color").append_attribute(L"val")      = L"1 1 1";
    intensityNode.append_child(L"multiplier").append_attribute(L"val") = 8.0f*IRRADIANCE_TO_RADIANCE;
  }
  hrLightClose(rectLight);
  
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  
  // camera
  //
  HRCameraRef camRef = hrCameraCreate(L"my camera");
  
  hrCameraOpen(camRef, HR_WRITE_DISCARD);
  {
    xml_node camNode = hrCameraParamNode(camRef);
    
    camNode.append_child(L"fov").text().set(L"45");
    camNode.append_child(L"nearClipPlane").text().set(L"0.01");
    camNode.append_child(L"farClipPlane").text().set(L"100.0");
    
    camNode.append_child(L"up").text().set(L"0 1 0");
    camNode.append_child(L"position").text().set(L"0 0 14");
    camNode.append_child(L"look_at").text().set(L"0 0 0");
  }
  hrCameraClose(camRef);
  
  // set up render settings
  //
  HRRenderRef renderRef = hrRenderCreate(L"HydraModern"); // opengl1
  hrRenderEnableDevice(renderRef, CURR_RENDER_DEVICE, true);
  
  hrRenderOpen(renderRef, HR_WRITE_DISCARD);
  {
    pugi::xml_node node = hrRenderParamNode(renderRef);
    
    node.append_child(L"width").text()  = 512;
    node.append_child(L"height").text() = 512;
    
    node.append_child(L"method_primary").text()   = L"pathtracing";
    node.append_child(L"method_secondary").text() = L"pathtracing";
    node.append_child(L"method_tertiary").text()  = L"pathtracing";
    node.append_child(L"method_caustic").text()   = L"pathtracing";
    node.append_child(L"qmc_variant").text()      = 7;
    
    node.append_child(L"trace_depth").text()      = 8;
    node.append_child(L"diff_trace_depth").text() = 4;
    node.append_child(L"maxRaysPerPixel").text()  = 1024;
  }
  hrRenderClose(renderRef);
  
  // create scene
  //
  HRSceneInstRef scnRef = hrSceneCreate(L"my scene");
  
  const float DEG_TO_RAD = float(3.14159265358979323846f) / 180.0f;
  
  hrSceneOpen(scnRef, HR_WRITE_DISCARD);
  {
    // instance sphere and cornell box
    //
    auto mtranslate = hlm::translate4x4(hlm::float3(0.0f, -1.5f, 1.0f));
    hrMeshInstance(scnRef, sphereRef, mtranslate.L());
    
    auto mrot = hlm::rotate4x4Y(180.0f*DEG_TO_RAD);
    hrMeshInstance(scnRef, cubeOpenRef, mrot.L());
    
    mtranslate = hlm::translate4x4(hlm::float3(0, 3.85f, 0));
    hrLightInstance(scnRef, rectLight, mtranslate.L());
  }
  hrSceneClose(scnRef);
  
  const std::wstring testImages1 = std::wstring(L"tests_images/") + testParams.testName + L"/z_out.png";
  const std::wstring testImages2 = std::wstring(L"tests_images/") + testParams.testName + L"/z_out2.png";
  const std::wstring testImages3 = std::wstring(L"tests_images/") + testParams.testName + L"/z_out3.png";
  const std::wstring testImages4 = std::wstring(L"tests_images/") + testParams.testName + L"/z_out4.png";
  
  const std::wstring names[4] = {testImages1.c_str(), testImages2.c_str(), testImages3.c_str(), testImages4.c_str()};
  
  for(int iter = 0; iter < 4; iter++)
  {
    hrMaterialOpen(mat4, HR_OPEN_EXISTING);
    {
      xml_node matNode = hrMaterialParamNode(mat4);
      auto refl = matNode.child(L"reflectivity");
      if(testParams.paramName != L"rot")
        refl.child(testParams.paramName.c_str()).attribute(L"val") = testParams.values[iter];
      else
        refl.child(L"anisotropy").attribute(L"rot") = testParams.values[iter];
      VERIFY_XML(matNode);
    }
    hrMaterialClose(mat4);
    
    if(iter == 3)
    {
      hrRenderOpen(renderRef, HR_OPEN_EXISTING);
      {
        pugi::xml_node node = hrRenderParamNode(renderRef);
        node.child(L"maxRaysPerPixel").text() = 4096;
      }
      hrRenderClose(renderRef);
    }
    
    hrFlush(scnRef, renderRef, camRef);
    
    while (true)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
      
      HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);
      
      if (info.haveUpdateFB)
      {
        auto pres = std::cout.precision(2);
        std::cout << "rendering progress = " << info.progress << "% \r";
        std::cout.flush();
        std::cout.precision(pres);
      }
      
      if (info.finalUpdate)
        break;
    }
    
    hrRenderSaveFrameBufferLDR(renderRef, names[iter].c_str());
    
  }
  
  const std::string name = ws2s(testParams.testName);
  return check_images(name.c_str(), 4, testParams.maxMSE);
}


bool MTL_TESTS::test_175_beckman_isotropic()
{
  InputMaterialTestParams params;
  params.testName  = L"test_175";
  params.brdfName  = L"beckmann";
  params.paramName = L"glossiness";
  params.values[0] = 0.25f;
  params.values[1] = 0.5f;
  params.values[2] = 0.75f;
  params.values[3] = 0.90f;
  params.maxMSE    = 25.0f;
  params.initialGlosiness = 0.0f;
  params.initialAniso     = 0.0f;
  params.initialRot       = 0.0f;
  
  return PefrormMaterialTest(params);
}

bool MTL_TESTS::test_176_beckman_anisotropic()
{
  InputMaterialTestParams params;
  params.testName  = L"test_176";
  params.brdfName  = L"beckmann";
  params.paramName = L"anisotropy";
  params.values[0] = 0.25f;
  params.values[1] = 0.5f;
  params.values[2] = 0.75f;
  params.values[3] = 1.0f;
  params.maxMSE    = 25.0f;
  params.initialGlosiness = 0.0f;
  params.initialAniso     = 0.0f;
  params.initialRot       = 0.0f;
  
  return PefrormMaterialTest(params);
}

bool MTL_TESTS::test_177_beckman_aniso_rot()
{
  InputMaterialTestParams params;
  params.testName  = L"test_177";
  params.brdfName  = L"beckmann";
  params.paramName = L"rot";
  params.values[0] = 0.25f;
  params.values[1] = 0.5f;
  params.values[2] = 0.75f;
  params.values[3] = 1.0f;
  params.maxMSE    = 25.0f;
  params.initialGlosiness = 0.0f;
  params.initialAniso     = 1.0f;
  params.initialRot       = 0.0f;
  
  return PefrormMaterialTest(params);
}

bool MTL_TESTS::test_178_trggx_isotropic()
{
  InputMaterialTestParams params;
  params.testName  = L"test_178";
  params.brdfName  = L"trggx";
  params.paramName = L"glossiness";
  params.values[0] = 0.25f;
  params.values[1] = 0.5f;
  params.values[2] = 0.75f;
  params.values[3] = 0.90f;
  params.maxMSE    = 25.0f;
  params.initialGlosiness = 0.0f;
  params.initialAniso     = 0.0f;
  params.initialRot       = 0.0f;
  
  return PefrormMaterialTest(params);
}

bool MTL_TESTS::test_179_trggx_anisotropic()
{
  InputMaterialTestParams params;
  params.testName  = L"test_179";
  params.brdfName  = L"trggx";
  params.paramName = L"anisotropy";
  params.values[0] = 0.25f;
  params.values[1] = 0.5f;
  params.values[2] = 0.75f;
  params.values[3] = 0.90f;
  params.maxMSE    = 25.0f;
  params.initialGlosiness = 0.0f;
  params.initialAniso     = 0.0f;
  params.initialRot       = 0.0f;
  
  return PefrormMaterialTest(params);
}

bool MTL_TESTS::test_180_trggx_aniso_rot()
{
  InputMaterialTestParams params;
  params.testName  = L"test_180";
  params.brdfName  = L"trggx";
  params.paramName = L"rot";
  params.values[0] = 0.25f;
  params.values[1] = 0.5f;
  params.values[2] = 0.75f;
  params.values[3] = 1.0f;
  params.maxMSE    = 25.0f;
  params.initialGlosiness = 0.0f;
  params.initialAniso     = 1.0f;
  params.initialRot       = 0.0f;
  
  return PefrormMaterialTest(params);
}
