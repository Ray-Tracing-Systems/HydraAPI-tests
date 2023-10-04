//
// Created by frol on 2/1/19.
//

#include "tests.h"
#include <iomanip>

#include <stdlib.h>
#include <stdio.h>
#include "linmath.h"

#if defined(WIN32)
#include <FreeImage.h>
#include <GLFW/glfw3.h>
#pragma comment(lib, "glfw3dll.lib")
#pragma comment(lib, "FreeImage.lib")

#else
#include <FreeImage.h>
#include <GLFW/glfw3.h>
#endif

#include "mesh_utils.h"

#include "HR_HDRImageTool.h"
#include "HydraXMLHelpers.h"

#include "Timer.h"

///////////////////////////////////////////////////////////////////////////////////

#pragma warning(disable:4996)
#pragma warning(disable:4838)
#pragma warning(disable:4244)

extern GLFWwindow* g_window;
using namespace TEST_UTILS;


bool ALGR_TESTS::test_401_ibpt_and_glossy_glass()
{ 
  std::wstring nameTest                = L"test_401";
  std::filesystem::path libraryPath    = L"tests_algorithm/" + nameTest;
  std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

  hrErrorCallerPlace(nameTest.c_str());
  hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);
  
  ////////////////////
  // Materials
  ////////////////////  
  
  auto matGray  = hrMaterialCreate(L"matGray");
  auto matGlass = hrMaterialCreate(L"matGlass");
  
  hrMaterialOpen(matGray, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matGray);
    AddDiffuseNode(matNode, L"0.5 0.5 0.5");
  }
  hrMaterialClose(matGray);

  hrMaterialOpen(matGlass, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matGlass);
    
    auto refl = matNode.append_child(L"reflectivity");
    refl.append_attribute(L"brdf_type").set_value(L"phong");
    
    auto color = refl.append_child(L"color");
    color.append_attribute(L"val").set_value(L"1 1 1");
    
    refl.append_child(L"fresnel").append_attribute(L"val")     = 1;
    refl.append_child(L"fresnel_ior").append_attribute(L"val") = 1.5;
    refl.append_child(L"glossiness").append_attribute(L"val")  = 1;
    
    auto trans = matNode.append_child(L"transparency");
    
    trans.append_child(L"color").append_attribute(L"val")      = L"1 1 1";
    trans.append_child(L"fog_color").append_attribute(L"val")  = L"1 1 1";
    
    trans.append_child(L"fog_multiplier").append_attribute(L"val") = 0.0f;
    trans.append_child(L"glossiness").append_attribute(L"val")     = 0.90f;
    trans.append_child(L"ior").append_attribute(L"val")            = 1.5f;
    
    VERIFY_XML(matNode);
  }
  hrMaterialClose(matGlass);
  
  ////////////////////
  // Meshes
  ////////////////////
  
  auto sph1     = HRMeshFromSimpleMesh(L"sph1", CreateSphere(2.0f, 64),  matGlass.id);
  auto sph2     = HRMeshFromSimpleMesh(L"sph2", CreateSphere(2.0f, 64),  matGlass.id);
  auto cubeOpen = HRMeshFromSimpleMesh(L"my_cube", CreateCubeOpen(6.0f), matGray.id);
  
  ////////////////////
  // Light
  ////////////////////

  auto sphere1 = CreateLight(L"sphere1", L"area", L"sphere", L"uniform", 0.1f, 0.1f, L"1 0.5 1", 200.0f * IRRADIANCE_TO_RADIANCE);
  auto sphere2 = CreateLight(L"sphere1", L"area", L"sphere", L"uniform", 0.1f, 0.1f, L"0.5 1 0.5", 200.0f * IRRADIANCE_TO_RADIANCE);
    
  ////////////////////
  // Camera
  ////////////////////

  CreateCamera(45, L"0 3 18", L"0 3 0");
  
  ////////////////////
  // Render settings
  ////////////////////
  
  HRRenderRef renderRef = hrRenderCreate(L"HydraModern");
  hrRenderEnableDevice(renderRef, CURR_RENDER_DEVICE, true);
  
  hrRenderOpen(renderRef, HR_WRITE_DISCARD);
  {
    auto node = hrRenderParamNode(renderRef);
    
    node.append_child(L"width").text()  = 512;
    node.append_child(L"height").text() = 512;
  
    node.append_child(L"method_primary").text()   = L"IBPT";
    node.append_child(L"method_secondary").text() = L"IBPT";
    node.append_child(L"method_tertiary").text()  = L"IBPT";
    node.append_child(L"method_caustic").text()   = L"IBPT";
    node.append_child(L"shadows").text()          = L"1";
    
    node.append_child(L"trace_depth").text()      = L"10";
    node.append_child(L"diff_trace_depth").text() = L"5";
    node.append_child(L"maxRaysPerPixel").text()  = 4096;
    node.append_child(L"resources_path").text()   = L"..";
    node.append_child(L"offline_pt").text()       = 0;
  }
  hrRenderClose(renderRef);  
  
  ////////////////////
  // Create scene
  ////////////////////

  auto scnRef = hrSceneCreate((L"scene_" + nameTest).c_str());

  hrSceneOpen(scnRef, HR_WRITE_DISCARD);
  
  AddMeshToScene(scnRef, cubeOpen, float3(0, 3, 0), float3(0, 180, 0));
  AddMeshToScene(scnRef, sph1, float3(-3, 4.25f, 0));
  AddMeshToScene(scnRef, sph2, float3(3, 4.25f, 0));
  AddLightToScene(scnRef, sphere1, float3(5, 8, 0));
  AddLightToScene(scnRef, sphere2, float3(-5, 8, 0));
  
  hrSceneClose(scnRef);
  
  hrFlush(scnRef, renderRef);
  
  ////////////////////
  // Rendering, save and check image
  ////////////////////

  RenderProgress(renderRef);

  hrRenderSaveFrameBufferLDR(renderRef, saveRenderFile.wstring().c_str());
  
  return check_images(ws2s(nameTest).c_str(), 1, 27);
}


bool ALGR_TESTS::test_402_ibpt_and_glossy_double_glass()
{  
  std::wstring nameTest                = L"test_402";
  std::filesystem::path libraryPath    = L"tests_algorithm/" + nameTest;
  std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

  hrErrorCallerPlace(nameTest.c_str());
  hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);
  
  ////////////////////
  // Materials
  ////////////////////  
  
  auto matGray  = hrMaterialCreate(L"matGray");
  auto matGlass = hrMaterialCreate(L"matGlass");  
  
  hrMaterialOpen(matGray, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matGray);
    AddDiffuseNode(matNode, L"0.5 0.5 0.5");
  }
  hrMaterialClose(matGray);

  hrMaterialOpen(matGlass, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matGlass);
    
    auto refl = matNode.append_child(L"reflectivity");
    refl.append_attribute(L"brdf_type").set_value(L"phong");
    
    auto color = refl.append_child(L"color");
    color.append_attribute(L"val").set_value(L"1 1 1");
    
    refl.append_child(L"fresnel").append_attribute(L"val")     = 1;
    refl.append_child(L"fresnel_ior").append_attribute(L"val") = 1.5;
    refl.append_child(L"glossiness").append_attribute(L"val")  = 1;
    
    auto trans = matNode.append_child(L"transparency");
    
    trans.append_child(L"color").append_attribute(L"val")      = L"1 1 1";
    trans.append_child(L"fog_color").append_attribute(L"val")  = L"1 1 1";
    
    trans.append_child(L"fog_multiplier").append_attribute(L"val") = 0.0f;
    trans.append_child(L"glossiness").append_attribute(L"val")     = 0.90f;
    trans.append_child(L"ior").append_attribute(L"val")            = 1.5f;
    
    VERIFY_XML(matNode);
  }
  hrMaterialClose(matGlass);
  
  ////////////////////
  // Meshes
  ////////////////////
  
  auto sph1     = HRMeshFromSimpleMesh(L"sph1", CreateSphere(2.0f, 64),  matGlass.id);
  auto sph2     = HRMeshFromSimpleMesh(L"sph2", CreateSphere(2.0f, 64),  matGlass.id);
  auto cubeOpen = HRMeshFromSimpleMesh(L"my_cube", CreateCubeOpen(6.0f), matGray.id);
  
  ////////////////////
  // Light
  ////////////////////

  auto sphere1 = CreateLight(L"sphere1", L"area", L"sphere", L"uniform", 0.1f, 0.1f, L"1 0.5 1", 200.0f * IRRADIANCE_TO_RADIANCE);
  auto sphere2 = CreateLight(L"sphere1", L"area", L"sphere", L"uniform", 0.1f, 0.1f, L"0.5 1 0.5", 200.0f * IRRADIANCE_TO_RADIANCE);
    
  ////////////////////
  // Camera
  ////////////////////

  CreateCamera(45, L"0 3 18", L"0 3 0");

  ////////////////////
  // Render settings
  ////////////////////

  //HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 512, 512, 256, 4096);
  
  HRRenderRef renderRef = hrRenderCreate(L"HydraModern");
  hrRenderEnableDevice(renderRef, CURR_RENDER_DEVICE, true);
  //hrRenderEnableDevice(renderRef, 0, true);
  //hrRenderEnableDevice(renderRef, 1, true);
  
  hrRenderOpen(renderRef, HR_WRITE_DISCARD);
  {
    auto node = hrRenderParamNode(renderRef);
    
    node.append_child(L"width").text()  = 512;
    node.append_child(L"height").text() = 512;
    
    node.append_child(L"method_primary").text()   = L"IBPT";
    node.append_child(L"method_secondary").text() = L"IBPT";
    node.append_child(L"method_tertiary").text()  = L"IBPT";
    node.append_child(L"method_caustic").text()   = L"IBPT";
    node.append_child(L"shadows").text()          = L"1";
    
    node.append_child(L"trace_depth").text()      = L"10";
    node.append_child(L"diff_trace_depth").text() = L"10";
    node.append_child(L"maxRaysPerPixel").text()  = 4096;
    node.append_child(L"resources_path").text()   = L"..";
    node.append_child(L"offline_pt").text()       = 0;
  }
  hrRenderClose(renderRef);
  
  
  ////////////////////
  // Create scene
  ////////////////////

  auto scnRef = hrSceneCreate((L"scene_" + nameTest).c_str());

  hrSceneOpen(scnRef, HR_WRITE_DISCARD);

  AddMeshToScene(scnRef, cubeOpen, float3(0, 3, 0), float3(0, 180, 0));
  AddMeshToScene(scnRef, sph1, float3(-3, 4.25f, 0));
  AddMeshToScene(scnRef, sph1, float3(-3, 4.25f, 0), float3(), float3(0.65f, 0.65f, 0.65f));
  AddMeshToScene(scnRef, sph2, float3(3, 4.25f, 0));
  AddLightToScene(scnRef, sphere1, float3(5, 8, 0));
  AddLightToScene(scnRef, sphere2, float3(-5, 8, 0));
  
  hrSceneClose(scnRef);
  
  hrFlush(scnRef, renderRef);

  ////////////////////
  // Rendering, save and check image
  ////////////////////

  RenderProgress(renderRef);

  hrRenderSaveFrameBufferLDR(renderRef, saveRenderFile.wstring().c_str());
  
  return check_images(ws2s(nameTest).c_str(), 1, 37);  
}


bool ALGR_TESTS::test_403_light_inside_double_glass()
{
  std::wstring nameTest                = L"test_403";
  std::filesystem::path libraryPath    = L"tests_algorithm/" + nameTest;
  std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

  hrErrorCallerPlace(nameTest.c_str());
  hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);
  
  ////////////////////
  // Materials
  ////////////////////  
  
  auto matGray  = hrMaterialCreate(L"matGray");
  auto matGlass = hrMaterialCreate(L"matGlass");  
  
  hrMaterialOpen(matGray, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matGray);
    AddDiffuseNode(matNode, L"0.5 0.5 0.5");
  }
  hrMaterialClose(matGray);
  
  hrMaterialOpen(matGlass, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matGlass);
    
    auto refl = matNode.append_child(L"reflectivity");
    refl.append_attribute(L"brdf_type").set_value(L"phong");
    
    auto color = refl.append_child(L"color");
    color.append_attribute(L"val").set_value(L"1 1 1");
    
    refl.append_child(L"fresnel").append_attribute(L"val")     = 1;
    refl.append_child(L"fresnel_ior").append_attribute(L"val") = 1.5;
    refl.append_child(L"glossiness").append_attribute(L"val")  = 1;
    
    auto trans = matNode.append_child(L"transparency");
    
    trans.append_child(L"color").append_attribute(L"val")      = L"1 1 1";
    trans.append_child(L"fog_color").append_attribute(L"val")  = L"1 1 1";
    
    trans.append_child(L"fog_multiplier").append_attribute(L"val") = 0.0f;
    trans.append_child(L"glossiness").append_attribute(L"val")     = 1.0f;
    trans.append_child(L"ior").append_attribute(L"val")            = 1.5f;
    
    VERIFY_XML(matNode);
  }
  hrMaterialClose(matGlass);
  
  ////////////////////
  // Meshes
  ////////////////////
  
  auto sph1     = HRMeshFromSimpleMesh(L"sph1", CreateSphere(2.0f, 64),  matGlass.id);
  auto sph2     = HRMeshFromSimpleMesh(L"sph2", CreateSphere(2.0f, 64),  matGlass.id);
  auto cubeOpen = HRMeshFromSimpleMesh(L"my_cube", CreateCubeOpen(6.0f), matGray.id);
  
  ////////////////////
  // Light
  ////////////////////

  auto sphere1 = CreateLight(L"sphere1", L"area", L"sphere", L"uniform", 0.1f, 0.1f, L"1 0.5 1", 200.0f * IRRADIANCE_TO_RADIANCE);
  auto sphere2 = CreateLight(L"sphere1", L"area", L"sphere", L"uniform", 0.1f, 0.1f, L"0.5 1 0.5", 200.0f * IRRADIANCE_TO_RADIANCE);
  
  ////////////////////
  // Camera
  ////////////////////

  CreateCamera(45, L"0 3 18", L"0 3 0");
  
  ////////////////////
  // Render settings
  ////////////////////
  
  //HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 512, 512, 256, 4096);
  
  HRRenderRef renderRef = hrRenderCreate(L"HydraModern");
  hrRenderEnableDevice(renderRef, CURR_RENDER_DEVICE, true);
  //hrRenderEnableDevice(renderRef, 0, true);
  //hrRenderEnableDevice(renderRef, 1, true);
  
  hrRenderOpen(renderRef, HR_WRITE_DISCARD);
  {
    auto node = hrRenderParamNode(renderRef);
    
    node.append_child(L"width").text()  = 512;
    node.append_child(L"height").text() = 512;
    
    node.append_child(L"method_primary").text()   = L"IBPT";
    node.append_child(L"method_secondary").text() = L"IBPT";
    node.append_child(L"method_tertiary").text()  = L"IBPT";
    node.append_child(L"method_caustic").text()   = L"IBPT";
    node.append_child(L"shadows").text()          = L"1";
    
    node.append_child(L"trace_depth").text()      = L"10";
    node.append_child(L"diff_trace_depth").text() = L"10";
    node.append_child(L"maxRaysPerPixel").text()  = 4096;
    node.append_child(L"resources_path").text()   = L"..";
    node.append_child(L"offline_pt").text()       = 0;
  }
  hrRenderClose(renderRef);  
  
  ////////////////////
  // Create scene
  ////////////////////

  auto scnRef = hrSceneCreate((L"scene_" + nameTest).c_str());

  hrSceneOpen(scnRef, HR_WRITE_DISCARD);

  AddMeshToScene(scnRef, cubeOpen, float3(0, 3, 0), float3(0, 180, 0));
  AddMeshToScene(scnRef, sph1, float3(-3, 4.25f, 0));
  AddMeshToScene(scnRef, sph1, float3(-3, 4.25f, 0), float3(), float3(0.65f, 0.65f, 0.65f));
  AddMeshToScene(scnRef, sph2, float3(3, 4.25f, 0));
  AddLightToScene(scnRef, sphere1, float3(5, 8, 0));
  AddLightToScene(scnRef, sphere2, float3(-3, 4.25f, 0));
  
  hrSceneClose(scnRef);
  
  hrFlush(scnRef, renderRef);
  
  ////////////////////
  // Rendering, save and check image
  ////////////////////

  RenderProgress(renderRef);

  hrRenderSaveFrameBufferLDR(renderRef, saveRenderFile.wstring().c_str());

  return check_images(ws2s(nameTest).c_str(), 1, 75);
}


void test_three_algorithms(HRSceneInstRef scnRef, HRRenderRef renderRef, const std::wstring a_outPath[3])
{
  for (int algId = 0; algId < 3; algId++)
  { 
    std::cout << std::endl << "Algorithm " << algId + 1 << std::endl;

    switch (algId)
    {
      case 0 :
      {
        hrRenderOpen(renderRef, HR_OPEN_EXISTING);   
        {
          auto node                                    = hrRenderParamNode(renderRef);
          node.force_child(L"method_primary").text()   = L"pathtracing";
          node.force_child(L"method_secondary").text() = L"pathtracing";
          node.force_child(L"method_tertiary").text()  = L"pathtracing";
          node.force_child(L"method_caustic").text()   = L"pathtracing";
          node.force_child(L"qmc_variant").text()      = QMC_ALL;

          node.force_child(L"maxRaysPerPixel").text() = 2048;
        }
        hrRenderClose(renderRef);
      }
      break;
    
      case 1 :
      {
        hrRenderOpen(renderRef, HR_OPEN_EXISTING);  
        {
          auto node                                    = hrRenderParamNode(renderRef);
          node.force_child(L"method_primary").text()   = L"IBPT";
          node.force_child(L"method_secondary").text() = L"IBPT";
          node.force_child(L"method_tertiary").text()  = L"IBPT";
          node.force_child(L"method_caustic").text()   = L"IBPT";
          node.force_child(L"qmc_variant").text()      = QMC_ALL;

          node.force_child(L"maxRaysPerPixel").text()  = 512;
        }
        hrRenderClose(renderRef);
      }
      break;
    
      case 2 :
      default:
      {
        hrRenderOpen(renderRef, HR_OPEN_EXISTING);    // disable automatic process run for hrCommit, ... probably need to add custom params to tis function
        {
          auto node                                    = hrRenderParamNode(renderRef);
          node.force_child(L"method_primary").text()   = L"mmlt";
          node.force_child(L"method_secondary").text() = L"mmlt";
          node.force_child(L"method_tertiary").text()  = L"mmlt";
          node.force_child(L"method_caustic").text()   = L"mmlt";
          node.force_child(L"qmc_variant").text()      = QMC_ALL;

          node.force_child(L"mmlt_burn_iters").text() = 256;
          node.force_child(L"mmlt_step_power").text() = 1024.0f;
          node.force_child(L"mmlt_step_size").text()  = 0.5f;    

          node.force_child(L"maxRaysPerPixel").text() = 1024;
        }
        hrRenderClose(renderRef);
      }
      break;
    }

    ////////////////////////////////////////////////////////////////////////////

    hrFlush(scnRef, renderRef);

    Timer timer(true);

    RenderProgress(renderRef);

    hrRenderSaveFrameBufferLDR(renderRef, a_outPath[algId].c_str());

    std::cout << std::endl << "render time = " << timer.getElapsed() << " sec " << std::endl;    
  }
}


bool ALGR_TESTS::test_404_cornell_glossy()                   
{
  std::wstring nameTest                = L"test_404";
  std::filesystem::path libraryPath    = L"tests_algorithm/" + nameTest;

  hrErrorCallerPlace(nameTest.c_str());
  if(!hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_OPEN_EXISTING))
    return false;

  /////////////////////////////////////////////////////////
  HRRenderRef renderRef;
  HRSceneInstRef scnRef;
  {
    renderRef.id = 0;  // get first render 
    scnRef.id    = 0;  // and first scene
  }
  /////////////////////////////////////////////////////////

  hrRenderEnableDevice(renderRef, CURR_RENDER_DEVICE, true);

  std::wstring outNames[3] = { L"tests_images/test_404/z_out.png", 
                               L"tests_images/test_404/z_out2.png",
                               L"tests_images/test_404/z_out3.png" };

  test_three_algorithms(scnRef, renderRef, outNames);

  remove("tests_algorithm/test_404/statex_00002.xml");
  remove("tests_algorithm/test_404/statex_00003.xml");
  remove("tests_algorithm/test_404/statex_00004.xml");

  remove("tests_algorithm/test_404/change_00001.xml");
  remove("tests_algorithm/test_404/change_00002.xml");
  remove("tests_algorithm/test_404/change_00003.xml");

  return check_images(ws2s(nameTest).c_str(), 3, 50);
}


bool ALGR_TESTS::test_405_cornell_with_mirror()
{
  std::wstring nameTest                = L"test_405";
  std::filesystem::path libraryPath    = L"tests_algorithm/" + nameTest;

  hrErrorCallerPlace(nameTest.c_str());
  if(!hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_OPEN_EXISTING))
    return false;

  /////////////////////////////////////////////////////////
  HRRenderRef renderRef;
  HRSceneInstRef scnRef;
  {
    renderRef.id = 0;  // get first render 
    scnRef.id    = 0;  // and first scene
  }
  /////////////////////////////////////////////////////////

  hrRenderEnableDevice(renderRef, CURR_RENDER_DEVICE, true);

  std::wstring outNames[3] = { L"tests_images/test_405/z_out.png",
                               L"tests_images/test_405/z_out2.png",
                               L"tests_images/test_405/z_out3.png" };

  test_three_algorithms(scnRef, renderRef, outNames);

  remove("tests_algorithm/test_405/statex_00002.xml");
  remove("tests_algorithm/test_405/statex_00003.xml");
  remove("tests_algorithm/test_405/statex_00004.xml");

  remove("tests_algorithm/test_405/change_00001.xml");
  remove("tests_algorithm/test_405/change_00002.xml");
  remove("tests_algorithm/test_405/change_00003.xml");

  return check_images(ws2s(nameTest).c_str(), 3, 27);
}


bool ALGR_TESTS::test_406_env_glass_ball_caustic()
{
  std::wstring nameTest                = L"test_406";
  std::filesystem::path libraryPath    = L"tests_algorithm/" + nameTest;

  hrErrorCallerPlace(nameTest.c_str());
  if(!hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_OPEN_EXISTING))
    return false;

  /////////////////////////////////////////////////////////
  HRRenderRef renderRef;
  HRSceneInstRef scnRef;
  {
    renderRef.id = 0;  // get first render 
    scnRef.id    = 0;  // and first scene
  }
  /////////////////////////////////////////////////////////

  hrRenderEnableDevice(renderRef, CURR_RENDER_DEVICE, true);

  std::wstring outNames[3] = { L"tests_images/test_406/z_out.png",
                               L"tests_images/test_406/z_out2.png",
                               L"tests_images/test_406/z_out3.png" };

  test_three_algorithms(scnRef, renderRef, outNames);

  remove("tests_algorithm/test_406/statex_00002.xml");
  remove("tests_algorithm/test_406/statex_00003.xml");
  remove("tests_algorithm/test_406/statex_00004.xml");

  remove("tests_algorithm/test_406/change_00001.xml");
  remove("tests_algorithm/test_406/change_00002.xml");
  remove("tests_algorithm/test_406/change_00003.xml");

  return check_images(ws2s(nameTest).c_str(), 3, 12);
}
