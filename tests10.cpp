//
// Created by vsan on 09.02.18.
//

#include <iomanip>

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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <fstream>

#include "HydraXMLHelpers.h"
#include "HR_HDRImageTool.h"

#include "tests.h"

///////////////////////////////////////////////////////////////////////////////////

#pragma warning(disable:4996)
using namespace TEST_UTILS;
using HydraRender::SaveImageToFile;
extern GLFWwindow* g_window;


bool test_700_loadlibrary_and_edit()
{
  hrSceneLibraryOpen(L"tests/test_700", HR_OPEN_EXISTING);

  /////////////////////////////////////////////////////////
  HRRenderRef renderRef;
  renderRef.id = 0;

  HRSceneInstRef scnRef;
  scnRef.id = 0;
  /////////////////////////////////////////////////////////

  auto pList = hrRenderGetDeviceList(renderRef);

  while (pList != nullptr)
  {
    std::wcout << L"device id = " << pList->id << L", name = " << pList->name << L", driver = " << pList->driver << std::endl;
    pList = pList->next;
  }

  hrRenderEnableDevice(renderRef, 0, true);

  hrCommit(scnRef, renderRef);
  hrRenderCommand(renderRef, L"pause");
  std::this_thread::sleep_for(std::chrono::milliseconds(500)); //so render has some time to actually stop


  HRLightRef sky;
  sky.id = 0;

  hrLightOpen(sky, HR_WRITE_DISCARD);
  {
    auto lightNode = hrLightParamNode(sky);

    lightNode.attribute(L"type").set_value(L"sky");
    lightNode.attribute(L"distribution").set_value(L"map");

    auto intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
    intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"0.5");

    VERIFY_XML(lightNode);
  }
  hrLightClose(sky);

  HRMaterialRef matRefl;
  matRefl.id = 1;
  hrMaterialOpen(matRefl, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matRefl);

    auto refl = matNode.append_child(L"reflectivity");
    refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
    refl.append_child(L"color").append_attribute(L"val").set_value(L"0.05 0.8 0.025");
    refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.99");
    refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
    refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
    refl.append_child(L"fresnel_IOR").append_attribute(L"val").set_value(16.0f);
  }
  hrMaterialClose(matRefl);


  hrFlush(scnRef, renderRef);

  //hrCommit(scnRef, renderRef);
  //hrRenderCommand(renderRef, L"resume");
  bool firstUpdate = true;
  
  while (true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);

    if (info.haveUpdateFB)
    {
      if(firstUpdate)
      {
        std::remove("tests/test_700/change_00001.xml");
        std::remove("tests/test_700/statex_00002.xml");
        firstUpdate = false;
      }

      auto pres = std::cout.precision(2);
      std::cout << "rendering progress = " << info.progress << "% \r";
      std::cout.precision(pres);
    }

    if (info.finalUpdate)
      break;
  }

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_700/z_out.png");

  return check_images("test_700", 1, 50.0f);
}

bool test_701_loadlibrary_and_add_textures()
{
  

  hrSceneLibraryOpen(L"tests/test_701", HR_OPEN_EXISTING);

  /////////////////////////////////////////////////////////
  HRRenderRef renderRef;
  renderRef.id = 0;

  HRSceneInstRef scnRef;
  scnRef.id = 0;
  /////////////////////////////////////////////////////////

  auto pList = hrRenderGetDeviceList(renderRef);

  while (pList != nullptr)
  {
    std::wcout << L"device id = " << pList->id << L", name = " << pList->name << L", driver = " << pList->driver << std::endl;
    pList = pList->next;
  }

  hrRenderEnableDevice(renderRef, 0, true);

  hrCommit(scnRef, renderRef);
  hrRenderCommand(renderRef, L"pause");
  std::this_thread::sleep_for(std::chrono::milliseconds(500)); //so render has some time to actually stop
  

  HRTextureNodeRef texEnv = hrTexture2DCreateFromFile(L"data/textures/LA_Downtown_Afternoon_Fishing_B_8k.jpg"); //23_antwerp_night.hdr LA_Downtown_Afternoon_Fishing_B_8k.jpg

  HRLightRef sky;
  sky.id = 0;

  hrLightOpen(sky, HR_WRITE_DISCARD);
  {
    auto lightNode = hrLightParamNode(sky);

    lightNode.attribute(L"type").set_value(L"sky");
    lightNode.attribute(L"distribution").set_value(L"map");

    auto intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
    intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"1.0");

    auto texNode = hrTextureBind(texEnv, intensityNode.child(L"color"));

    texNode.append_attribute(L"matrix");
    float samplerMatrix[16] = { 1, 0, 0, 0,
                                0, 1, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1 };

    texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
    texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");

    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

    VERIFY_XML(lightNode);
  }
  hrLightClose(sky);

  hrFlush(scnRef, renderRef);

  //hrCommit(scnRef, renderRef);
  //hrRenderCommand(renderRef, L"resume");
  bool firstUpdate = true;

  while (true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);

    if (info.haveUpdateFB)
    {
      if(firstUpdate)
      {
        std::remove("tests/test_701/change_00001.xml");
        std::remove("tests/test_701/statex_00002.xml");
        firstUpdate = false;
      }

      auto pres = std::cout.precision(2);
      std::cout << "rendering progress = " << info.progress << "% \r";
      std::cout.precision(pres);
    }

    if (info.finalUpdate)
      break;
  }

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_701/z_out.png");

  return check_images("test_701", 1, 50.0f);
}

bool test_702_get_material_by_name_and_edit()
{
  

  hrSceneLibraryOpen(L"tests/test_702", HR_OPEN_EXISTING);

  /////////////////////////////////////////////////////////
  HRRenderRef renderRef;
  renderRef.id = 0;

  HRSceneInstRef scnRef;
  scnRef.id = 0;
  /////////////////////////////////////////////////////////

  auto pList = hrRenderGetDeviceList(renderRef);

  while (pList != nullptr)
  {
    std::wcout << L"device id = " << pList->id << L", name = " << pList->name << L", driver = " << pList->driver << std::endl;
    pList = pList->next;
  }

  hrRenderEnableDevice(renderRef, 0, true);

  hrCommit(scnRef, renderRef);
  hrRenderCommand(renderRef, L"pause");
  std::this_thread::sleep_for(std::chrono::milliseconds(500)); //so render has some time to actually stop

  std::wstring mat1(L"matRefl");
  HRMaterialRef matRefl = hrFindMaterialByName(mat1.c_str());

  if(matRefl.id != -1)
  {
    hrMaterialOpen(matRefl, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matRefl);
      auto refl = matNode.append_child(L"reflectivity");
      refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"0.8 0.8 0.025");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.5");
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_IOR").append_attribute(L"val").set_value(16.0f);
    }
    hrMaterialClose(matRefl);
  }

  std::wstring mat2(L"matGray");
  HRMaterialRef matGray = hrFindMaterialByName(mat2.c_str());

  if(matGray.id != -1)
  {
    hrMaterialOpen(matGray, HR_OPEN_EXISTING);
    {
      auto matNode = hrMaterialParamNode(matGray);
      auto diff = matNode.force_child(L"diffuse");
      diff.force_child(L"color").force_attribute(L"val").set_value(L"0.1 0.2 0.725");
    }
    hrMaterialClose(matGray);
  }

  std::wstring mat3(L"matSomething");
  HRMaterialRef matSmth = hrFindMaterialByName(mat3.c_str());

  if(matSmth.id != -1)
  {
    hrMaterialOpen(matSmth, HR_OPEN_EXISTING);
    {
      auto matNode = hrMaterialParamNode(matSmth);
      auto diff = matNode.force_child(L"diffuse");
      diff.force_child(L"color").force_attribute(L"val").set_value(L"1.0 0.1 0.1");
    }
    hrMaterialClose(matSmth);
  }


  hrFlush(scnRef, renderRef);

  //hrCommit(scnRef, renderRef);
  //hrRenderCommand(renderRef, L"resume");
  bool firstUpdate = true;
  
  while (true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);

    if (info.haveUpdateFB)
    {
      if(firstUpdate)
      {
        std::remove("tests/test_702/change_00001.xml");
        std::remove("tests/test_702/statex_00002.xml");
        firstUpdate = false;
      }

      auto pres = std::cout.precision(2);
      std::cout << "rendering progress = " << info.progress << "% \r";
      std::cout.precision(pres);
    }

    if (info.finalUpdate)
      break;
  }

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_702/z_out.png");

  return check_images("test_702", 1, 50.0f);
}

bool test_703_get_light_by_name_and_edit()
{
  

  hrSceneLibraryOpen(L"tests/test_703", HR_OPEN_EXISTING);

  /////////////////////////////////////////////////////////
  HRRenderRef renderRef;
  renderRef.id = 0;

  HRSceneInstRef scnRef;
  scnRef.id = 0;
  /////////////////////////////////////////////////////////

  auto pList = hrRenderGetDeviceList(renderRef);

  while (pList != nullptr)
  {
    std::wcout << L"device id = " << pList->id << L", name = " << pList->name << L", driver = " << pList->driver << std::endl;
    pList = pList->next;
  }

  hrRenderEnableDevice(renderRef, 0, true);

  hrCommit(scnRef, renderRef);
  hrRenderCommand(renderRef, L"pause");
  std::this_thread::sleep_for(std::chrono::milliseconds(500)); //so render has some time to actually stop

  std::wstring light1(L"sphere1");
  auto lightRef1 = hrFindLightByName(light1.c_str());

  if(lightRef1.id != -1)
  {
    hrLightOpen(lightRef1, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(lightRef1);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"rect");
      lightNode.attribute(L"distribution").set_value(L"diffuse");

      pugi::xml_node sizeNode = lightNode.append_child(L"size");

      sizeNode.append_attribute(L"half_length").set_value(L"1.0");
      sizeNode.append_attribute(L"half_width").set_value(L"1.0");

      auto intensityNode = lightNode.append_child(L"intensity");
      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"0.0 1.0 0.1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(4.0f*IRRADIANCE_TO_RADIANCE);

      VERIFY_XML(lightNode);
    }
    hrLightClose(lightRef1);
  }

  std::wstring light2(L"sphere1");
  auto lightRef2 = hrFindLightByName(light2.c_str());

  if(lightRef2.id != -1)
  {
    hrLightOpen(lightRef2, HR_OPEN_EXISTING);
    {
      auto lightNode = hrLightParamNode(lightRef2);

      auto intensityNode = lightNode.force_child(L"intensity");
      intensityNode.force_child(L"color").force_attribute(L"val").set_value(L"1.0 0.3 0.0");
      intensityNode.force_child(L"multiplier").force_attribute(L"val").set_value(12.0f*IRRADIANCE_TO_RADIANCE);

      VERIFY_XML(lightNode);
    }
    hrLightClose(lightRef2);
  }

  std::wstring light3(L"lightDoesntExist");
  auto lightRef3 = hrFindLightByName(light3.c_str());

  if(lightRef3.id != -1)
  {
    hrLightOpen(lightRef3, HR_OPEN_EXISTING);
    {
      auto lightNode = hrLightParamNode(lightRef3);

      auto intensityNode = lightNode.force_child(L"intensity");
      intensityNode.force_child(L"multiplier").force_attribute(L"val").set_value(24.0f*IRRADIANCE_TO_RADIANCE);

      VERIFY_XML(lightNode);
    }
    hrLightClose(lightRef3);
  }

  hrFlush(scnRef, renderRef);

  //hrCommit(scnRef, renderRef);
  //hrRenderCommand(renderRef, L"resume");
  bool firstUpdate = true;
  
  while (true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);

    if (info.haveUpdateFB)
    {
      if(firstUpdate)
      {
        std::remove("tests/test_703/change_00001.xml");
        std::remove("tests/test_703/statex_00002.xml");
        firstUpdate = false;
      }

      auto pres = std::cout.precision(2);
      std::cout << "rendering progress = " << info.progress << "% \r";
      std::cout.precision(pres);
    }

    if (info.finalUpdate)
      break;
  }

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_703/z_out.png");

  return check_images("test_703", 1, 50.0f);
}

bool test_704_get_camera_by_name_and_edit()
{
  

  hrSceneLibraryOpen(L"tests/test_704", HR_OPEN_EXISTING);

  /////////////////////////////////////////////////////////
  HRRenderRef renderRef;
  renderRef.id = 0;

  HRSceneInstRef scnRef;
  scnRef.id = 0;
  /////////////////////////////////////////////////////////

  auto pList = hrRenderGetDeviceList(renderRef);

  while (pList != nullptr)
  {
    std::wcout << L"device id = " << pList->id << L", name = " << pList->name << L", driver = " << pList->driver << std::endl;
    pList = pList->next;
  }

  hrRenderEnableDevice(renderRef, 0, true);

  hrCommit(scnRef, renderRef);
  hrRenderCommand(renderRef, L"pause");
  std::this_thread::sleep_for(std::chrono::milliseconds(500)); //so render has some time to actually stop

  std::wstring cam1(L"my camera");
  HRCameraRef camRef1 = hrFindCameraByName(cam1.c_str());

  if(camRef1.id != -1)
  {
    hrCameraOpen(camRef1, HR_OPEN_EXISTING);
    {
      auto camNode = hrCameraParamNode(camRef1);
      camNode.force_child(L"position").text().set(L"25 15 6");
    }
    hrCameraClose(camRef1);
  }

  std::wstring cam2(L"my non-existent camera");
  HRCameraRef camRef2 = hrFindCameraByName(cam2.c_str());

  if(camRef2.id != -1)
  {
    hrCameraOpen(camRef2, HR_OPEN_EXISTING);
    {
      auto camNode = hrCameraParamNode(camRef2);
      auto pos = camNode.child(L"position").text().set(L"0 150 0");
    }
    hrCameraClose(camRef2);
  }

  hrFlush(scnRef, renderRef, camRef1);

  //hrCommit(scnRef, renderRef);
  //hrRenderCommand(renderRef, L"resume");
  bool firstUpdate = true;

  while (true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);

    if (info.haveUpdateFB)
    {
      if(firstUpdate)
      {
        std::remove("tests/test_704/change_00001.xml");
        std::remove("tests/test_704/statex_00002.xml");
        firstUpdate = false;
      }

      auto pres = std::cout.precision(2);
      std::cout << "rendering progress = " << info.progress << "% \r";
      std::cout.precision(pres);
    }

    if (info.finalUpdate)
      break;
  }

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_704/z_out.png");

  return check_images("test_704", 1, 50.0f);
}

bool test_705_transform_all_instances()
{
  hrSceneLibraryOpen(L"tests/test_705", HR_OPEN_EXISTING);

  /////////////////////////////////////////////////////////
  HRRenderRef renderRef;
  renderRef.id = 0;

  HRSceneInstRef scnRef;
  scnRef.id = 0;
  /////////////////////////////////////////////////////////

  auto pList = hrRenderGetDeviceList(renderRef);

  while (pList != nullptr)
  {
    std::wcout << L"device id = " << pList->id << L", name = " << pList->name << L", driver = " << pList->driver << std::endl;
    pList = pList->next;
  }

  hrRenderEnableDevice(renderRef, 0, true);

  hrCommit(scnRef, renderRef);
  hrRenderCommand(renderRef, L"pause");
  std::this_thread::sleep_for(std::chrono::milliseconds(500)); //so render has some time to actually stop
  

  float matrix[16] = { 0.7071f, 0, -0.7071f, 0,
                             0, 1,        0, 1,
                       0.7071f, 0,  0.7071f, 0,
                             0, 0,        0, 1 };


  HRUtils::InstanceSceneIntoScene(scnRef, scnRef, matrix, false);

  hrFlush(scnRef, renderRef);

  //hrCommit(scnRef, renderRef);
  //hrRenderCommand(renderRef, L"resume");
  bool firstUpdate = true;
  
  while (true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);

    if (info.haveUpdateFB)
    {
      if(firstUpdate)
      {
        std::remove("tests/test_705/change_00001.xml");
        std::remove("tests/test_705/statex_00002.xml");
        firstUpdate = false;
      }
      
      auto pres = std::cout.precision(2);
      std::cout << "rendering progress = " << info.progress << "% \r";
      std::cout.precision(pres);
    }

    if (info.finalUpdate)
      break;
  }

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_705/z_out.png");

  return check_images("test_705", 1, 100.0f);
}

bool test_706_transform_all_instances_origin()
{
  

  hrSceneLibraryOpen(L"tests/test_706", HR_OPEN_EXISTING);

  /////////////////////////////////////////////////////////
  HRRenderRef renderRef;
  renderRef.id = 0;

  HRSceneInstRef scnRef;
  scnRef.id = 0;
  /////////////////////////////////////////////////////////

  auto pList = hrRenderGetDeviceList(renderRef);

  while (pList != nullptr)
  {
    std::wcout << L"device id = " << pList->id << L", name = " << pList->name << L", driver = " << pList->driver << std::endl;
    pList = pList->next;
  }

  hrRenderEnableDevice(renderRef, 0, true);

  hrCommit(scnRef, renderRef);
  hrRenderCommand(renderRef, L"pause");
  std::this_thread::sleep_for(std::chrono::milliseconds(500)); //so render has some time to actually stop


  float matrix[16] = { 0.7071f, 0, -0.7071f, 0,
                       0, 1,        0,       0,
                       0.7071f, 0,  0.7071f, 0,
                       0, 0,        0,       1 };


  HRUtils::InstanceSceneIntoScene(scnRef, scnRef, matrix);

  hrFlush(scnRef, renderRef);

  //hrCommit(scnRef, renderRef);
  //hrRenderCommand(renderRef, L"resume");
  bool firstUpdate = true;
  
  while (true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);

    if (info.haveUpdateFB)
    {
      if(firstUpdate)
      {
        std::remove("tests/test_706/change_00001.xml");
        std::remove("tests/test_706/statex_00002.xml");
        firstUpdate = false;
      }

      auto pres = std::cout.precision(2);
      std::cout << "rendering progress = " << info.progress << "% \r";
      std::cout.precision(pres);
    }

    if (info.finalUpdate)
      break;
  }

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_706/z_out.png");

  return check_images("test_706", 1, 100.0f);
}

bool test_707_merge_library()
{
  

  hrErrorCallerPlace(L"test_707");

  hrSceneLibraryOpen(L"tests/test_707", HR_WRITE_DISCARD);

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Materials
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


  HRMaterialRef matGray = hrMaterialCreate(L"matGray");
  HRMaterialRef matRefl = hrMaterialCreate(L"matRefl");


  hrMaterialOpen(matGray, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matGray);

    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.5 0.5 0.5");
  }
  hrMaterialClose(matGray);

  hrMaterialOpen(matRefl, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matRefl);

    auto refl = matNode.append_child(L"reflectivity");
    refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
    refl.append_child(L"color").append_attribute(L"val").set_value(L"0.8 0.8 0.8");
    refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.98");
    refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
    refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
    refl.append_child(L"fresnel_IOR").append_attribute(L"val").set_value(8.0f);
  }
  hrMaterialClose(matRefl);


  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Meshes
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  HRMeshRef cubeR = HRMeshFromSimpleMesh(L"cubeR", CreateCube(2.0f), matGray.id);
  HRMeshRef sphereG = HRMeshFromSimpleMesh(L"sphereG", CreateSphere(4.0f, 64), matRefl.id);
  HRMeshRef torusB = HRMeshFromSimpleMesh(L"torusB", CreateTorus(0.8f, 2.0f, 64, 64), matGray.id);
  HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), matGray.id);
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Light
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRLightRef sky = hrLightCreate(L"sky");

  HRTextureNodeRef texEnv = hrTexture2DCreateFromFile(L"data/textures/Factory_Catwalk_2k_BLUR.exr");

  hrLightOpen(sky, HR_WRITE_DISCARD);
  {
    auto lightNode = hrLightParamNode(sky);

    lightNode.attribute(L"type").set_value(L"sky");
    lightNode.attribute(L"distribution").set_value(L"map");

    auto intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
    intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"1.0");

    auto texNode = hrTextureBind(texEnv, intensityNode.child(L"color"));

    texNode.append_attribute(L"matrix");
    float samplerMatrix[16] = { 1, 0, 0, 0,
                                0, 1, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1 };

    texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
    texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");

    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

    VERIFY_XML(lightNode);
  }
  hrLightClose(sky);


  HRUtils::MergeLibraryIntoLibrary(L"tests_f/test_131");

  HRMeshRef mergedMesh;
  mergedMesh.id = 5;

  auto blend = hrFindMaterialByName(L"matBlend3");

  hrMeshOpen(torusB, HR_TRIANGLE_IND3, HR_OPEN_EXISTING);
  {
    hrMeshMaterialId(torusB, blend.id);
  }
  hrMeshClose(torusB);


  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Camera
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRCameraRef camRef = hrCameraCreate(L"my camera");

  hrCameraOpen(camRef, HR_WRITE_DISCARD);
  {
    auto camNode = hrCameraParamNode(camRef);

    camNode.append_child(L"fov").text().set(L"45");
    camNode.append_child(L"nearClipPlane").text().set(L"0.01");
    camNode.append_child(L"farClipPlane").text().set(L"100.0");

    camNode.append_child(L"up").text().set(L"0 1 0");
    camNode.append_child(L"position").text().set(L"0 13 16");
    camNode.append_child(L"look_at").text().set(L"0 0 0");
  }
  hrCameraClose(camRef);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Render settings
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 1024, 768, 256, 1024);


  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Create scene
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

  using namespace LiteMath;

  float4x4 mRot, mRot2;
  float4x4 mTranslate;
  float4x4 mScale;
  float4x4 mRes;

  const float DEG_TO_RAD = 0.01745329251f; // float(3.14159265358979323846f) / 180.0f;

  hrSceneOpen(scnRef, HR_WRITE_DISCARD);
  ///////////

  mTranslate.identity();
  mRes.identity();
  mRot.identity();


  mTranslate = translate4x4(float3(0.0f, -1.0f, 0.0f));
  mRes = mul(mTranslate, mRes);

  hrMeshInstance(scnRef, planeRef, mRes.L());

  ///////////

  mTranslate.identity();
  mRes.identity();
  mRot.identity();

  mTranslate = translate4x4(float3(-4.75f, 1.0f, 5.0f));
  mRot = rotate4x4Y(60.0f*DEG_TO_RAD);
  mRes = mul(mTranslate, mRot);

  hrMeshInstance(scnRef, cubeR, mRes.L());

  ///////////

  mTranslate.identity();
  mRes.identity();
  mRot.identity();
  mRot2.identity();

  mTranslate = translate4x4(float3(4.0f, 1.0f, 5.5f));
  mRot = rotate4x4Y(-60.0f*DEG_TO_RAD);
  mRot2 = rotate4x4X(90.0f*DEG_TO_RAD);
  mRes = mul(mRot, mRot2);
  mRes = mul(mTranslate, mRes);

  hrMeshInstance(scnRef, torusB, mRes.L());

  ///////////

  mTranslate.identity();
  mRes.identity();
  mRot.identity();

  mTranslate = translate4x4(float3(0.0f, 2.0f, -1.0f));
  mRes = mul(mTranslate, mRes);

  hrMeshInstance(scnRef, sphereG, mRes.L());

  ///////////
  mRes.identity();

  mTranslate = translate4x4(float3(8.0f, 2.0f, 0.0f));
  mRes = mul(mTranslate, mRes);

  hrMeshInstance(scnRef, mergedMesh, mRes.L());

  ///////////

  mRes.identity();

  hrLightInstance(scnRef, sky, mRes.L());

  ///////////

  hrSceneClose(scnRef);

  hrFlush(scnRef, renderRef);
  
  while (true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);

    if (info.haveUpdateFB)
    {
      auto pres = std::cout.precision(2);
      std::cout << "rendering progress = " << info.progress << "% \r";
      std::cout.precision(pres);
    }

    if (info.finalUpdate)
      break;
  }

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_707/z_out.png");

  return check_images("test_707", 1, 60);
}

bool test_708_merge_one_texture()
{
  hrErrorCallerPlace(L"test_708");

  hrSceneLibraryOpen(L"tests/test_708", HR_WRITE_DISCARD);

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Materials
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


  HRMaterialRef matGray = hrMaterialCreate(L"matGray");
  HRMaterialRef matRefl = hrMaterialCreate(L"matRefl");


  hrMaterialOpen(matGray, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matGray);

    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.5 0.5 0.5");
  }
  hrMaterialClose(matGray);

  hrMaterialOpen(matRefl, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matRefl);

    auto refl = matNode.append_child(L"reflectivity");
    refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
    refl.append_child(L"color").append_attribute(L"val").set_value(L"0.8 0.8 0.8");
    refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.98");
    refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
    refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
    refl.append_child(L"fresnel_IOR").append_attribute(L"val").set_value(8.0f);
  }
  hrMaterialClose(matRefl);


  //

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Meshes
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  HRMeshRef cubeR = HRMeshFromSimpleMesh(L"cubeR", CreateCube(2.0f), matGray.id);
  HRMeshRef sphereG = HRMeshFromSimpleMesh(L"sphereG", CreateSphere(4.0f, 64), matRefl.id);
  HRMeshRef torusB = HRMeshFromSimpleMesh(L"torusB", CreateTorus(0.8f, 2.0f, 64, 64), matGray.id);
  HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), matGray.id);
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Light
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRLightRef sky = hrLightCreate(L"sky");

  HRTextureNodeRef texEnv = hrTexture2DCreateFromFile(L"data/textures/Factory_Catwalk_2k_BLUR.exr");

  hrLightOpen(sky, HR_WRITE_DISCARD);
  {
    auto lightNode = hrLightParamNode(sky);

    lightNode.attribute(L"type").set_value(L"sky");
    lightNode.attribute(L"distribution").set_value(L"map");

    auto intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
    intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"1.0");

    auto texNode = hrTextureBind(texEnv, intensityNode.child(L"color"));

    texNode.append_attribute(L"matrix");
    float samplerMatrix[16] = { 1, 0, 0, 0,
      0, 1, 0, 0,
      0, 0, 1, 0,
      0, 0, 0, 1 };

    texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
    texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");

    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

    VERIFY_XML(lightNode);
  }
  hrLightClose(sky);

  HRTextureNodeRef mergedTex = HRUtils::MergeOneTextureIntoLibrary(L"tests_f/test_131", L"data/textures/yinyang.png");

  hrMaterialOpen(matGray, HR_OPEN_EXISTING);
  {
    auto matNode = hrMaterialParamNode(matGray);

    auto color = matNode.child(L"diffuse").child(L"color");
    auto texNode = hrTextureBind(mergedTex, color);

    texNode.append_attribute(L"matrix");
    float samplerMatrix[16] = { 2, 0, 0, 0,
                                0, 2, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1 };

    texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
    texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");

    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);
  }
  hrMaterialClose(matGray);


  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Camera
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRCameraRef camRef = hrCameraCreate(L"my camera");

  hrCameraOpen(camRef, HR_WRITE_DISCARD);
  {
    auto camNode = hrCameraParamNode(camRef);

    camNode.append_child(L"fov").text().set(L"45");
    camNode.append_child(L"nearClipPlane").text().set(L"0.01");
    camNode.append_child(L"farClipPlane").text().set(L"100.0");

    camNode.append_child(L"up").text().set(L"0 1 0");
    camNode.append_child(L"position").text().set(L"0 13 16");
    camNode.append_child(L"look_at").text().set(L"0 0 0");
  }
  hrCameraClose(camRef);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Render settings
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 1024, 768, 256, 1024);


  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Create scene
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

  using namespace LiteMath;

  float4x4 mRot, mRot2;
  float4x4 mTranslate;
  float4x4 mScale;
  float4x4 mRes;

  const float DEG_TO_RAD = 0.01745329251f; // float(3.14159265358979323846f) / 180.0f;

  hrSceneOpen(scnRef, HR_WRITE_DISCARD);
  ///////////

  mTranslate.identity();
  mRes.identity();
  mRot.identity();


  mTranslate = translate4x4(float3(0.0f, -1.0f, 0.0f));
  mRes = mul(mTranslate, mRes);

  hrMeshInstance(scnRef, planeRef, mRes.L());

  ///////////

  mTranslate.identity();
  mRes.identity();
  mRot.identity();

  mTranslate = translate4x4(float3(-4.75f, 1.0f, 5.0f));
  mRot = rotate4x4Y(60.0f*DEG_TO_RAD);
  mRes = mul(mTranslate, mRot);

  hrMeshInstance(scnRef, cubeR, mRes.L());

  ///////////

  mTranslate.identity();
  mRes.identity();
  mRot.identity();
  mRot2.identity();

  mTranslate = translate4x4(float3(4.0f, 1.0f, 5.5f));
  mRot = rotate4x4Y(-60.0f*DEG_TO_RAD);
  mRot2 = rotate4x4X(90.0f*DEG_TO_RAD);
  mRes = mul(mRot, mRot2);
  mRes = mul(mTranslate, mRes);

  hrMeshInstance(scnRef, torusB, mRes.L());

  ///////////

  mTranslate.identity();
  mRes.identity();
  mRot.identity();

  mTranslate = translate4x4(float3(0.0f, 2.0f, -1.0f));
  mRes = mul(mTranslate, mRes);

  hrMeshInstance(scnRef, sphereG, mRes.L());

  ///////////

  mRes.identity();

  hrLightInstance(scnRef, sky, mRes.L());

  ///////////

  hrSceneClose(scnRef);

  hrFlush(scnRef, renderRef);

  while (true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);

    if (info.haveUpdateFB)
    {
      auto pres = std::cout.precision(2);
      std::cout << "rendering progress = " << info.progress << "% \r";
      std::cout.precision(pres);
    }

    if (info.finalUpdate)
      break;
  }

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_708/z_out.png");

  return check_images("test_708", 1, 60);
}


bool test_709_merge_one_material()
{
  hrErrorCallerPlace(L"test_709");

  hrSceneLibraryOpen(L"tests/test_709", HR_WRITE_DISCARD);

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Materials
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


  HRMaterialRef matGray = hrMaterialCreate(L"matGray");
  HRMaterialRef matRefl = hrMaterialCreate(L"matRefl");


  hrMaterialOpen(matGray, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matGray);

    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.5 0.5 0.5");
  }
  hrMaterialClose(matGray);

  hrMaterialOpen(matRefl, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matRefl);

    auto refl = matNode.append_child(L"reflectivity");
    refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
    refl.append_child(L"color").append_attribute(L"val").set_value(L"0.8 0.8 0.8");
    refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.98");
    refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
    refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
    refl.append_child(L"fresnel_IOR").append_attribute(L"val").set_value(8.0f);
  }
  hrMaterialClose(matRefl);


  //

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Meshes
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  HRMeshRef cubeR = HRMeshFromSimpleMesh(L"cubeR", CreateCube(2.0f), matGray.id);
  HRMeshRef sphereG = HRMeshFromSimpleMesh(L"sphereG", CreateSphere(4.0f, 64), matRefl.id);
  HRMeshRef torusB = HRMeshFromSimpleMesh(L"torusB", CreateTorus(0.8f, 2.0f, 64, 64), matGray.id);
  HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), matGray.id);
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Light
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRLightRef sky = hrLightCreate(L"sky");

  HRTextureNodeRef texEnv = hrTexture2DCreateFromFile(L"data/textures/Factory_Catwalk_2k_BLUR.exr");

  hrLightOpen(sky, HR_WRITE_DISCARD);
  {
    auto lightNode = hrLightParamNode(sky);

    lightNode.attribute(L"type").set_value(L"sky");
    lightNode.attribute(L"distribution").set_value(L"map");

    auto intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
    intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"1.0");

    auto texNode = hrTextureBind(texEnv, intensityNode.child(L"color"));

    texNode.append_attribute(L"matrix");
    float samplerMatrix[16] = { 1, 0, 0, 0,
      0, 1, 0, 0,
      0, 0, 1, 0,
      0, 0, 0, 1 };

    texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
    texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");

    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

    VERIFY_XML(lightNode);
  }
  hrLightClose(sky);



  HRMaterialRef mergedMat1 = HRUtils::MergeOneMaterialIntoLibrary(L"tests_f/test_131", L"matBlend1");
  HRMaterialRef mergedMat2 = HRUtils::MergeOneMaterialIntoLibrary(L"tests_f/test_131", L"matBlend2");
  HRMaterialRef mergedMat3 = HRUtils::MergeOneMaterialIntoLibrary(L"tests_f/test_131", L"matBlend1");

  hrMeshOpen(sphereG, HR_TRIANGLE_IND3, HR_OPEN_EXISTING);
  {
    hrMeshMaterialId(sphereG, mergedMat1.id);
  }
  hrMeshClose(sphereG);

  hrMeshOpen(torusB, HR_TRIANGLE_IND3, HR_OPEN_EXISTING);
  {
    hrMeshMaterialId(torusB, mergedMat2.id);
  }
  hrMeshClose(torusB);

  hrMeshOpen(planeRef, HR_TRIANGLE_IND3, HR_OPEN_EXISTING);
  {
    hrMeshMaterialId(planeRef, mergedMat3.id);
  }
  hrMeshClose(planeRef);


  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Camera
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRCameraRef camRef = hrCameraCreate(L"my camera");

  hrCameraOpen(camRef, HR_WRITE_DISCARD);
  {
    auto camNode = hrCameraParamNode(camRef);

    camNode.append_child(L"fov").text().set(L"45");
    camNode.append_child(L"nearClipPlane").text().set(L"0.01");
    camNode.append_child(L"farClipPlane").text().set(L"100.0");

    camNode.append_child(L"up").text().set(L"0 1 0");
    camNode.append_child(L"position").text().set(L"0 13 16");
    camNode.append_child(L"look_at").text().set(L"0 0 0");
  }
  hrCameraClose(camRef);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Render settings
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 1024, 768, 256, 1024);


  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Create scene
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

  using namespace LiteMath;

  float4x4 mRot, mRot2;
  float4x4 mTranslate;
  float4x4 mScale;
  float4x4 mRes;

  const float DEG_TO_RAD = 0.01745329251f; // float(3.14159265358979323846f) / 180.0f;

  hrSceneOpen(scnRef, HR_WRITE_DISCARD);
  ///////////

  mTranslate.identity();
  mRes.identity();
  mRot.identity();


  mTranslate = translate4x4(float3(0.0f, -1.0f, 0.0f));
  mRes = mul(mTranslate, mRes);

  hrMeshInstance(scnRef, planeRef, mRes.L());

  ///////////

  mTranslate.identity();
  mRes.identity();
  mRot.identity();

  mTranslate = translate4x4(float3(-4.75f, 1.0f, 5.0f));
  mRot = rotate4x4Y(60.0f*DEG_TO_RAD);
  mRes = mul(mTranslate, mRot);

  hrMeshInstance(scnRef, cubeR, mRes.L());

  ///////////

  mTranslate.identity();
  mRes.identity();
  mRot.identity();
  mRot2.identity();

  mTranslate = translate4x4(float3(4.0f, 1.0f, 5.5f));
  mRot = rotate4x4Y(-60.0f*DEG_TO_RAD);
  mRot2 = rotate4x4X(90.0f*DEG_TO_RAD);
  mRes = mul(mRot, mRot2);
  mRes = mul(mTranslate, mRes);

  hrMeshInstance(scnRef, torusB, mRes.L());

  ///////////

  mTranslate.identity();
  mRes.identity();
  mRot.identity();

  mTranslate = translate4x4(float3(0.0f, 2.0f, -1.0f));
  mRes = mul(mTranslate, mRes);

  hrMeshInstance(scnRef, sphereG, mRes.L());

  ///////////

  mRes.identity();

  hrLightInstance(scnRef, sky, mRes.L());

  ///////////

  hrSceneClose(scnRef);

  hrFlush(scnRef, renderRef);

  while (true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);

    if (info.haveUpdateFB)
    {
      auto pres = std::cout.precision(2);
      std::cout << "rendering progress = " << info.progress << "% \r";
      std::cout.precision(pres);
    }

    if (info.finalUpdate)
      break;
  }

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_709/z_out.png");

  return check_images("test_709", 1, 60);
}

bool test_710_merge_one_mesh()
{
  hrErrorCallerPlace(L"test_710");

  hrSceneLibraryOpen(L"tests/test_710", HR_WRITE_DISCARD);

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Materials
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


  HRMaterialRef matGray = hrMaterialCreate(L"matGray");
  HRMaterialRef matRefl = hrMaterialCreate(L"matRefl");


  hrMaterialOpen(matGray, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matGray);

    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.5 0.5 0.5");
  }
  hrMaterialClose(matGray);

  hrMaterialOpen(matRefl, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matRefl);

    auto refl = matNode.append_child(L"reflectivity");
    refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
    refl.append_child(L"color").append_attribute(L"val").set_value(L"0.8 0.8 0.8");
    refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.98");
    refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
    refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
    refl.append_child(L"fresnel_IOR").append_attribute(L"val").set_value(8.0f);
  }
  hrMaterialClose(matRefl);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Meshes
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  HRMeshRef cubeR = HRMeshFromSimpleMesh(L"cubeR", CreateCube(2.0f), matGray.id);
  HRMeshRef sphereG = HRMeshFromSimpleMesh(L"sphereG", CreateSphere(4.0f, 64), matRefl.id);
  HRMeshRef torusB = HRMeshFromSimpleMesh(L"torusB", CreateTorus(0.8f, 2.0f, 64, 64), matGray.id);
  HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), matGray.id);
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Light
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRLightRef sky = hrLightCreate(L"sky");

  HRTextureNodeRef texEnv = hrTexture2DCreateFromFile(L"data/textures/Factory_Catwalk_2k_BLUR.exr");

  hrLightOpen(sky, HR_WRITE_DISCARD);
  {
    auto lightNode = hrLightParamNode(sky);

    lightNode.attribute(L"type").set_value(L"sky");
    lightNode.attribute(L"distribution").set_value(L"map");

    auto intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
    intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"1.0");

    auto texNode = hrTextureBind(texEnv, intensityNode.child(L"color"));

    texNode.append_attribute(L"matrix");
    float samplerMatrix[16] = { 1, 0, 0, 0,
      0, 1, 0, 0,
      0, 0, 1, 0,
      0, 0, 0, 1 };

    texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
    texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");

    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

    VERIFY_XML(lightNode);
  }
  hrLightClose(sky);



  HRMeshRef mergedTree = HRUtils::MergeOneMeshIntoLibrary(L"tests_f/test_005", L"data/meshes/bigtree.vsgf");
  HRMeshRef mergedLucy = HRUtils::MergeOneMeshIntoLibrary(L"tests_f/test_002", L"data/meshes/lucy.vsgf");


  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Camera
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRCameraRef camRef = hrCameraCreate(L"my camera");

  hrCameraOpen(camRef, HR_WRITE_DISCARD);
  {
    auto camNode = hrCameraParamNode(camRef);

    camNode.append_child(L"fov").text().set(L"45");
    camNode.append_child(L"nearClipPlane").text().set(L"0.01");
    camNode.append_child(L"farClipPlane").text().set(L"100.0");

    camNode.append_child(L"up").text().set(L"0 1 0");
    camNode.append_child(L"position").text().set(L"0 13 16");
    camNode.append_child(L"look_at").text().set(L"0 0 0");
  }
  hrCameraClose(camRef);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Render settings
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 1024, 768, 256, 1024);


  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Create scene
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

  using namespace LiteMath;

  float4x4 mRot, mRot2;
  float4x4 mTranslate;
  float4x4 mScale;
  float4x4 mRes;

  const float DEG_TO_RAD = 0.01745329251f; // float(3.14159265358979323846f) / 180.0f;

  hrSceneOpen(scnRef, HR_WRITE_DISCARD);
  ///////////

  mTranslate.identity();
  mRes.identity();
  mRot.identity();


  mTranslate = translate4x4(float3(0.0f, -1.0f, 0.0f));
  mRes = mul(mTranslate, mRes);

  hrMeshInstance(scnRef, planeRef, mRes.L());

  ///////////

  mTranslate.identity();
  mRes.identity();
  mRot.identity();

  mTranslate = translate4x4(float3(-4.75f, 1.0f, 5.0f));
  mRot = rotate4x4Y(60.0f*DEG_TO_RAD);
  mRes = mul(mTranslate, mRot);

  hrMeshInstance(scnRef, cubeR, mRes.L());

  ///////////

  mTranslate.identity();
  mRes.identity();
  mRot.identity();
  mRot2.identity();

  mTranslate = translate4x4(float3(4.0f, 1.0f, 5.5f));
  mRot = rotate4x4Y(-60.0f*DEG_TO_RAD);
  mRot2 = rotate4x4X(90.0f*DEG_TO_RAD);
  mRes = mul(mRot, mRot2);
  mRes = mul(mTranslate, mRes);

  hrMeshInstance(scnRef, torusB, mRes.L());

  ///////////

  mTranslate.identity();
  mRes.identity();
  mRot.identity();

  mTranslate = translate4x4(float3(0.0f, 2.0f, -1.0f));
  mRes = mul(mTranslate, mRes);

  hrMeshInstance(scnRef, sphereG, mRes.L());

  ///////////

  mTranslate.identity();
  mRes.identity();
  mRot.identity();

  mTranslate = translate4x4(float3(6.0f, 0.0f, -1.0f));
  mRes = mul(mTranslate, mRes);

  hrMeshInstance(scnRef, mergedTree, mRes.L());

  ///////////

  mTranslate.identity();
  mRes.identity();
  mRot.identity();

  mTranslate = translate4x4(float3(-6.0f, 0.0f, 1.0f));
  mRes = mul(mTranslate, mRes);

  hrMeshInstance(scnRef, mergedLucy, mRes.L());

  ///////////

  mRes.identity();

  hrLightInstance(scnRef, sky, mRes.L());

  ///////////

  hrSceneClose(scnRef);

  hrFlush(scnRef, renderRef);

  while (true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);

    if (info.haveUpdateFB)
    {
      auto pres = std::cout.precision(2);
      std::cout << "rendering progress = " << info.progress << "% \r";
      std::cout.precision(pres);
    }

    if (info.finalUpdate)
      break;
  }

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_710/z_out.png");

  return check_images("test_710", 1, 60);
}

bool test_711_merge_scene()
{
  hrErrorCallerPlace(L"test_711");

  hrSceneLibraryOpen(L"tests/test_711", HR_WRITE_DISCARD);

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Materials
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


  HRMaterialRef matGray = hrMaterialCreate(L"matGray");
  HRMaterialRef matRefl = hrMaterialCreate(L"matRefl");


  hrMaterialOpen(matGray, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matGray);

    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.5 0.5 0.5");
  }
  hrMaterialClose(matGray);

  hrMaterialOpen(matRefl, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matRefl);

    auto refl = matNode.append_child(L"reflectivity");
    refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
    refl.append_child(L"color").append_attribute(L"val").set_value(L"0.8 0.8 0.8");
    refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.98");
    refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
    refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
    refl.append_child(L"fresnel_IOR").append_attribute(L"val").set_value(8.0f);
  }
  hrMaterialClose(matRefl);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Meshes
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  HRMeshRef cubeR = HRMeshFromSimpleMesh(L"cubeR", CreateCube(2.0f), matGray.id);
  HRMeshRef sphereG = HRMeshFromSimpleMesh(L"sphereG", CreateSphere(4.0f, 64), matRefl.id);
  HRMeshRef torusB = HRMeshFromSimpleMesh(L"torusB", CreateTorus(0.8f, 2.0f, 64, 64), matGray.id);
  HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), matGray.id);
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Light
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRLightRef sky = hrLightCreate(L"sky");

  HRTextureNodeRef texEnv = hrTexture2DCreateFromFile(L"data/textures/Factory_Catwalk_2k_BLUR.exr");

  hrLightOpen(sky, HR_WRITE_DISCARD);
  {
    auto lightNode = hrLightParamNode(sky);

    lightNode.attribute(L"type").set_value(L"sky");
    lightNode.attribute(L"distribution").set_value(L"map");

    auto intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
    intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"1.0");

    auto texNode = hrTextureBind(texEnv, intensityNode.child(L"color"));

    texNode.append_attribute(L"matrix");
    float samplerMatrix[16] = { 1, 0, 0, 0,
                                0, 1, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1 };

    texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
    texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");

    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

    VERIFY_XML(lightNode);
  }
  hrLightClose(sky);


  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Camera
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRCameraRef camRef = hrCameraCreate(L"my camera");

  hrCameraOpen(camRef, HR_WRITE_DISCARD);
  {
    auto camNode = hrCameraParamNode(camRef);

    camNode.append_child(L"fov").text().set(L"45");
    camNode.append_child(L"nearClipPlane").text().set(L"0.01");
    camNode.append_child(L"farClipPlane").text().set(L"100.0");

    camNode.append_child(L"up").text().set(L"0 1 0");
    camNode.append_child(L"position").text().set(L"0 13 16");
    camNode.append_child(L"look_at").text().set(L"0 0 0");
  }
  hrCameraClose(camRef);

  HRCameraRef camRef2 = hrCameraCreate(L"my camera");

  hrCameraOpen(camRef2, HR_WRITE_DISCARD);
  {
    auto camNode = hrCameraParamNode(camRef2);

    camNode.append_child(L"fov").text().set(L"60");
    camNode.append_child(L"nearClipPlane").text().set(L"0.01");
    camNode.append_child(L"farClipPlane").text().set(L"100.0");

    camNode.append_child(L"up").text().set(L"0 1 0");
    camNode.append_child(L"position").text().set(L"0 20 16");
    camNode.append_child(L"look_at").text().set(L"0 0 0");
  }
  hrCameraClose(camRef2);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Render settings
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 1024, 768, 256, 1024);


  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Create scene
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

  using namespace LiteMath;

  float4x4 mRot, mRot2;
  float4x4 mTranslate;
  float4x4 mScale;
  float4x4 mRes;

  const float DEG_TO_RAD = 0.01745329251f; // float(3.14159265358979323846f) / 180.0f;

  hrSceneOpen(scnRef, HR_WRITE_DISCARD);
  ///////////

  mTranslate.identity();
  mRes.identity();
  mRot.identity();


  mTranslate = translate4x4(float3(0.0f, -1.0f, 0.0f));
  mRes = mul(mTranslate, mRes);

  hrMeshInstance(scnRef, planeRef, mRes.L());

  ///////////

  mTranslate.identity();
  mRes.identity();
  mRot.identity();

  mTranslate = translate4x4(float3(-4.75f, 1.0f, 5.0f));
  mRot = rotate4x4Y(60.0f*DEG_TO_RAD);
  mRes = mul(mTranslate, mRot);

  hrMeshInstance(scnRef, cubeR, mRes.L());

  ///////////

  mTranslate.identity();
  mRes.identity();
  mRot.identity();
  mRot2.identity();

  mTranslate = translate4x4(float3(4.0f, 1.0f, 5.5f));
  mRot = rotate4x4Y(-60.0f*DEG_TO_RAD);
  mRot2 = rotate4x4X(90.0f*DEG_TO_RAD);
  mRes = mul(mRot, mRot2);
  mRes = mul(mTranslate, mRes);

  hrMeshInstance(scnRef, torusB, mRes.L());

  ///////////

  mTranslate.identity();
  mRes.identity();
  mRot.identity();

  mTranslate = translate4x4(float3(0.0f, 2.0f, -1.0f));
  mRes = mul(mTranslate, mRes);

  hrMeshInstance(scnRef, sphereG, mRes.L());

  ///////////

  mRes.identity();

  hrLightInstance(scnRef, sky, mRes.L());

  ///////////

  hrSceneClose(scnRef);


  HRSceneInstRef scnRef2 = HRUtils::MergeLibraryIntoLibrary(L"/home/vsan/repos/HydraAPI/main/tests/test_718", true, true); //L"tests_f/test_131

  mTranslate = translate4x4(float3(0.0f, 5.0f, 0.0f));

  HRUtils::InstanceSceneIntoScene(scnRef2, scnRef, mTranslate.L());

  hrFlush(scnRef, renderRef, camRef);
  
  while (true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);

    if (info.haveUpdateFB)
    {
      auto pres = std::cout.precision(2);
      std::cout << "rendering progress = " << info.progress << "% \r";
      std::cout.precision(pres);
    }

    if (info.finalUpdate)
      break;
  }

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_711/z_out.png");

  return check_images("test_711", 1, 60);
}

bool test_712_merge_one_light()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_712");

  hrSceneLibraryOpen(L"tests/test_712", HR_WRITE_DISCARD);

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Materials
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


  HRMaterialRef matGray = hrMaterialCreate(L"matGray");
  HRMaterialRef matRefl = hrMaterialCreate(L"matRefl");


  hrMaterialOpen(matGray, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matGray);

    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.5 0.5 0.5");
  }
  hrMaterialClose(matGray);

  hrMaterialOpen(matRefl, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matRefl);

    auto refl = matNode.append_child(L"reflectivity");
    refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
    refl.append_child(L"color").append_attribute(L"val").set_value(L"0.8 0.8 0.8");
    refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.98");
    refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
    refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
    refl.append_child(L"fresnel_IOR").append_attribute(L"val").set_value(8.0f);
  }
  hrMaterialClose(matRefl);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Meshes
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  HRMeshRef cubeR = HRMeshFromSimpleMesh(L"cubeR", CreateCube(2.0f), matGray.id);
  HRMeshRef sphereG = HRMeshFromSimpleMesh(L"sphereG", CreateSphere(4.0f, 64), matRefl.id);
  HRMeshRef torusB = HRMeshFromSimpleMesh(L"torusB", CreateTorus(0.8f, 2.0f, 64, 64), matGray.id);
  HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), matGray.id);
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Light
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRLightRef sky = hrLightCreate(L"sky");

  HRTextureNodeRef texEnv = hrTexture2DCreateFromFile(L"data/textures/Factory_Catwalk_2k_BLUR.exr");

  hrLightOpen(sky, HR_WRITE_DISCARD);
  {
    auto lightNode = hrLightParamNode(sky);

    lightNode.attribute(L"type").set_value(L"sky");
    lightNode.attribute(L"distribution").set_value(L"map");

    auto intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
    intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"1.0");

    auto texNode = hrTextureBind(texEnv, intensityNode.child(L"color"));

    texNode.append_attribute(L"matrix");
    float samplerMatrix[16] = { 1, 0, 0, 0,
                                0, 1, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1 };

    texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
    texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");

    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

    VERIFY_XML(lightNode);
  }
  hrLightClose(sky);



  HRLightRef mergedLight1 = HRUtils::MergeOneLightIntoLibrary(L"tests_f/test_221", L"spot2");
  HRLightRef mergedLight2 = HRUtils::MergeOneLightIntoLibrary(L"tests_f/test_221", L"spot3");


  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Camera
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRCameraRef camRef = hrCameraCreate(L"my camera");

  hrCameraOpen(camRef, HR_WRITE_DISCARD);
  {
    auto camNode = hrCameraParamNode(camRef);

    camNode.append_child(L"fov").text().set(L"45");
    camNode.append_child(L"nearClipPlane").text().set(L"0.01");
    camNode.append_child(L"farClipPlane").text().set(L"100.0");

    camNode.append_child(L"up").text().set(L"0 1 0");
    camNode.append_child(L"position").text().set(L"0 13 16");
    camNode.append_child(L"look_at").text().set(L"0 0 0");
  }
  hrCameraClose(camRef);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Render settings
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 1024, 768, 256, 1024);


  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Create scene
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

  using namespace LiteMath;

  float4x4 mRot, mRot2;
  float4x4 mTranslate;
  float4x4 mScale;
  float4x4 mRes;

  const float DEG_TO_RAD = 0.01745329251f; // float(3.14159265358979323846f) / 180.0f;

  hrSceneOpen(scnRef, HR_WRITE_DISCARD);
  ///////////

  mTranslate.identity();
  mRes.identity();
  mRot.identity();


  mTranslate = translate4x4(float3(0.0f, -1.0f, 0.0f));
  mRes = mul(mTranslate, mRes);

  hrMeshInstance(scnRef, planeRef, mRes.L());

  ///////////

  mTranslate.identity();
  mRes.identity();
  mRot.identity();

  mTranslate = translate4x4(float3(-4.75f, 1.0f, 5.0f));
  mRot = rotate4x4Y(60.0f*DEG_TO_RAD);
  mRes = mul(mTranslate, mRot);

  hrMeshInstance(scnRef, cubeR, mRes.L());

  ///////////

  mTranslate.identity();
  mRes.identity();
  mRot.identity();
  mRot2.identity();

  mTranslate = translate4x4(float3(4.0f, 1.0f, 5.5f));
  mRot = rotate4x4Y(-60.0f*DEG_TO_RAD);
  mRot2 = rotate4x4X(90.0f*DEG_TO_RAD);
  mRes = mul(mRot, mRot2);
  mRes = mul(mTranslate, mRes);

  hrMeshInstance(scnRef, torusB, mRes.L());

  ///////////

  mTranslate.identity();
  mRes.identity();
  mRot.identity();

  mTranslate = translate4x4(float3(0.0f, 2.0f, -1.0f));
  mRes = mul(mTranslate, mRes);

  hrMeshInstance(scnRef, sphereG, mRes.L());

  ///////////

  mTranslate.identity();
  mRes.identity();
  mRot.identity();

  mTranslate = translate4x4(float3(6.0f, 4.0f, 0.0f));
  mRes = mul(mTranslate, mRes);

  hrLightInstance(scnRef, mergedLight1, mRes.L());

  ///////////

  mTranslate.identity();
  mRes.identity();
  mRot.identity();

  mTranslate = translate4x4(float3(-6.0f, 4.0f, 0.0f));
  mRes = mul(mTranslate, mRes);

  hrLightInstance(scnRef, mergedLight2, mRes.L());

  ///////////

  mRes.identity();

  hrLightInstance(scnRef, sky, mRes.L());

  ///////////

  hrSceneClose(scnRef);

  hrFlush(scnRef, renderRef);

  while (true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);

    if (info.haveUpdateFB)
    {
      auto pres = std::cout.precision(2);
      std::cout << "rendering progress = " << info.progress << "% \r";
      std::cout.precision(pres);
    }

    if (info.finalUpdate)
      break;
  }

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_712/z_out.png");

  return check_images("test_712", 1, 60);
}


bool test_713_commit_without_render()
{
  initGLIfNeeded();

  hrSceneLibraryOpen(L"tests/test_713", HR_OPEN_EXISTING);

  /////////////////////////////////////////////////////////

  HRSceneInstRef scnRef;
  scnRef.id = 0;
  /////////////////////////////////////////////////////////

  hrCommit(scnRef);

  HRLightRef sky;
  sky.id = 0;

  HRTextureNodeRef texEnv = hrTexture2DCreateFromFile(L"data/textures/LA_Downtown_Afternoon_Fishing_B_8k.jpg"); //23_antwerp_night.hdr LA_Downtown_Afternoon_Fishing_B_8k.jpg

  hrLightOpen(sky, HR_WRITE_DISCARD);
  {
    auto lightNode = hrLightParamNode(sky);

    lightNode.attribute(L"type").set_value(L"sky");
    lightNode.attribute(L"distribution").set_value(L"map");

    auto intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
    intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"1.0");

    auto texNode = hrTextureBind(texEnv, intensityNode.child(L"color"));

    texNode.append_attribute(L"matrix");
    float samplerMatrix[16] = { 1, 0, 0, 0,
                                0, 1, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1 };

    texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
    texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");

    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

    VERIFY_XML(lightNode);
  }
  hrLightClose(sky);

  HRMaterialRef matRefl;
  matRefl.id = 1;
  hrMaterialOpen(matRefl, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matRefl);

    auto refl = matNode.append_child(L"reflectivity");
    refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
    refl.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.8 0.025");
    refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.99");
    refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
    refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
    refl.append_child(L"fresnel_IOR").append_attribute(L"val").set_value(16.0f);
  }
  hrMaterialClose(matRefl);

  HRRenderRef renderRef;
  renderRef.id = 0;

  auto pList = hrRenderGetDeviceList(renderRef);

  while (pList != nullptr)
  {
    std::wcout << L"device id = " << pList->id << L", name = " << pList->name << L", driver = " << pList->driver << std::endl;
    pList = pList->next;
  }

  hrRenderEnableDevice(renderRef, 0, true);


  hrFlush(scnRef, renderRef);

  //hrCommit(scnRef, renderRef);
  //hrRenderCommand(renderRef, L"resume");
  bool firstUpdate = true;

  while (true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);

    if (info.haveUpdateFB)
    {
      if(firstUpdate)
      {
        std::remove("tests/test_713/change_00001.xml");
        std::remove("tests/test_713/statex_00002.xml");
        firstUpdate = false;
      }

      auto pres = std::cout.precision(2);
      std::cout << "rendering progress = " << info.progress << "% \r";
      std::cout.precision(pres);
    }

    if (info.finalUpdate)
      break;
  }

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_713/z_out.png");

  return check_images("test_713", 1, 50.0f);
}

bool test_714_print_matlib_map()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_714");

  hrSceneLibraryOpen(L"tests/test_714", HR_WRITE_DISCARD);

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Materials
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


  HRMaterialRef matGray = hrMaterialCreate(L"matGray");
  HRMaterialRef matRefl = hrMaterialCreate(L"matRefl");
  HRMaterialRef matGold    = hrMaterialCreate(L"matGold");
  HRMaterialRef matSilver  = hrMaterialCreate(L"matSilver");


  hrMaterialOpen(matGray, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matGray);

    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.5 0.5 0.5");
  }
  hrMaterialClose(matGray);

  hrMaterialOpen(matRefl, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matRefl);

    auto refl = matNode.append_child(L"reflectivity");
    refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
    refl.append_child(L"color").append_attribute(L"val").set_value(L"0.8 0.8 0.8");
    refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.98");
    refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
    refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
    refl.append_child(L"fresnel_IOR").append_attribute(L"val").set_value(8.0f);
  }
  hrMaterialClose(matRefl);

  hrMaterialOpen(matGold, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matGold);

    auto refl = matNode.append_child(L"reflectivity");
    refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
    refl.append_child(L"color").append_attribute(L"val").set_value(L"0.88 0.61 0.05");
    refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.98");
    refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
    refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
    refl.append_child(L"fresnel_IOR").append_attribute(L"val").set_value(8.0f);

    VERIFY_XML(matNode);
  }
  hrMaterialClose(matGold);

  hrMaterialOpen(matSilver, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matSilver);

    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 0.5");

    auto refl = matNode.append_child(L"reflectivity");
    refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
    refl.append_child(L"color").append_attribute(L"val").set_value(L"0.9 0.9 0.9");
    refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"1.0");
    refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
    refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
    refl.append_child(L"fresnel_IOR").append_attribute(L"val").set_value(16.0f);

    VERIFY_XML(matNode);
  }
  hrMaterialClose(matSilver);


  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Meshes
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  HRMeshRef cubeR = HRMeshFromSimpleMesh(L"cubeR", CreateCube(2.0f), matGray.id);
  HRMeshRef sphereG = HRMeshFromSimpleMesh(L"sphereG", CreateSphere(4.0f, 64), matSilver.id);
  HRMeshRef torusB = HRMeshFromSimpleMesh(L"torusB", CreateTorus(0.8f, 2.0f, 64, 64), matGold.id);
  HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), matGray.id);
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Light
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRLightRef sky = hrLightCreate(L"sky");

  HRTextureNodeRef texEnv = hrTexture2DCreateFromFile(L"data/textures/Factory_Catwalk_2k_BLUR.exr");

  hrLightOpen(sky, HR_WRITE_DISCARD);
  {
    auto lightNode = hrLightParamNode(sky);

    lightNode.attribute(L"type").set_value(L"sky");
    lightNode.attribute(L"distribution").set_value(L"map");

    auto intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
    intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"1.0");

    auto texNode = hrTextureBind(texEnv, intensityNode.child(L"color"));

    texNode.append_attribute(L"matrix");
    float samplerMatrix[16] = { 1, 0, 0, 0,
                                0, 1, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1 };

    texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
    texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");

    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

    VERIFY_XML(lightNode);
  }
  hrLightClose(sky);


  HRUtils::MergeLibraryIntoLibrary(L"tests_f/test_131");

  HRMeshRef mergedMesh;
  mergedMesh.id = 5;

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Camera
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRCameraRef camRef = hrCameraCreate(L"my camera");

  hrCameraOpen(camRef, HR_WRITE_DISCARD);
  {
    auto camNode = hrCameraParamNode(camRef);

    camNode.append_child(L"fov").text().set(L"45");
    camNode.append_child(L"nearClipPlane").text().set(L"0.01");
    camNode.append_child(L"farClipPlane").text().set(L"100.0");

    camNode.append_child(L"up").text().set(L"0 1 0");
    camNode.append_child(L"position").text().set(L"0 13 16");
    camNode.append_child(L"look_at").text().set(L"0 0 0");
  }
  hrCameraClose(camRef);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Render settings
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 1024, 768, 256, 1024);


  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Create scene
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

  using namespace LiteMath;

  float4x4 mRot, mRot2;
  float4x4 mTranslate;
  float4x4 mScale;
  float4x4 mRes;

  const float DEG_TO_RAD = 0.01745329251f; // float(3.14159265358979323846f) / 180.0f;

  hrSceneOpen(scnRef, HR_WRITE_DISCARD);
  ///////////

  mTranslate.identity();
  mRes.identity();
  mRot.identity();


  mTranslate = translate4x4(float3(0.0f, -1.0f, 0.0f));
  mRes = mul(mTranslate, mRes);

  hrMeshInstance(scnRef, planeRef, mRes.L());

  ///////////

  mTranslate.identity();
  mRes.identity();
  mRot.identity();

  mTranslate = translate4x4(float3(-4.75f, 1.0f, 5.0f));
  mRot = rotate4x4Y(60.0f*DEG_TO_RAD);
  mRes = mul(mTranslate, mRot);

  hrMeshInstance(scnRef, cubeR, mRes.L());

  ///////////

  mTranslate.identity();
  mRes.identity();
  mRot.identity();
  mRot2.identity();

  mTranslate = translate4x4(float3(4.0f, 1.0f, 5.5f));
  mRot = rotate4x4Y(-60.0f*DEG_TO_RAD);
  mRot2 = rotate4x4X(90.0f*DEG_TO_RAD);
  mRes = mul(mRot, mRot2);
  mRes = mul(mTranslate, mRes);

  hrMeshInstance(scnRef, torusB, mRes.L());

  ///////////

  mTranslate.identity();
  mRes.identity();
  mRot.identity();

  mTranslate = translate4x4(float3(0.0f, 2.0f, -1.0f));
  mRes = mul(mTranslate, mRes);

  hrMeshInstance(scnRef, sphereG, mRes.L());

  ///////////
  mRes.identity();

  mTranslate = translate4x4(float3(8.0f, 2.0f, 0.0f));
  mRes = mul(mTranslate, mRes);

  hrMeshInstance(scnRef, mergedMesh, mRes.L());

  ///////////

  mRes.identity();

  hrLightInstance(scnRef, sky, mRes.L());

  ///////////

  hrSceneClose(scnRef);

  hrCommit(scnRef);

  auto materials = HydraXMLHelpers::GetMaterialNameToIdMap();

  std::stringstream ss;
  for(auto mat : materials)
  {
    ss << mat.first << " : " << mat.second << std::endl;
  }

  std::string outStr = ss.str();

  std::string correctStr = """matGray : 0\n"
                             "matRefl : 1\n"
                             "matGold : 2\n"
                             "matSilver : 3\n"
                             "matGold : 4\n"
                             "matSilver : 5\n"
                             "matLacquer : 6\n"
                             "matGlass : 7\n"
                             "matBricks1 : 8\n"
                             "matBricks2 : 9\n"
                             "matGray : 10\n"
                             "matBlend1 : 11\n"
                             "matBlend2 : 12\n"
                             "matBlend3 : 13\n"
                             "my_area_light_material : 14\n"
                             "sky_material : 15\n""";


  hrFlush(scnRef, renderRef, camRef);

  while (true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);

    if (info.haveUpdateFB)
    {
      auto pres = std::cout.precision(2);
      std::cout << "rendering progress = " << info.progress << "% \r";
      std::cout.precision(pres);
    }

    if (info.finalUpdate)
      break;
  }

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_714/z_out.png");

  return check_images("test_714", 1, 60) && (outStr == correctStr);
}

bool test_715_merge_scene_with_remaps()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_715");

  hrSceneLibraryOpen(L"tests/test_715", HR_WRITE_DISCARD);


  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Materials
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


  HRMaterialRef matGray = hrMaterialCreate(L"matGray");
  HRMaterialRef matRefl = hrMaterialCreate(L"matRefl");


  hrMaterialOpen(matGray, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matGray);

    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.5 0.5 0.5");
  }
  hrMaterialClose(matGray);

  hrMaterialOpen(matRefl, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matRefl);

    auto refl = matNode.append_child(L"reflectivity");
    refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
    refl.append_child(L"color").append_attribute(L"val").set_value(L"0.8 0.8 0.8");
    refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.98");
    refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
    refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
    refl.append_child(L"fresnel_IOR").append_attribute(L"val").set_value(8.0f);
  }
  hrMaterialClose(matRefl);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Meshes
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  HRMeshRef cubeR = HRMeshFromSimpleMesh(L"cubeR", CreateCube(2.0f), matGray.id);
  HRMeshRef sphereG = HRMeshFromSimpleMesh(L"sphereG", CreateSphere(4.0f, 64), matRefl.id);
  HRMeshRef torusB = HRMeshFromSimpleMesh(L"torusB", CreateTorus(0.8f, 2.0f, 64, 64), matGray.id);
  HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), matGray.id);


  HRLightRef sky = hrLightCreate(L"sky");

  HRTextureNodeRef texEnv = hrTexture2DCreateFromFile(L"data/textures/Factory_Catwalk_2k_BLUR.exr");

  hrLightOpen(sky, HR_WRITE_DISCARD);
  {
    auto lightNode = hrLightParamNode(sky);

    lightNode.attribute(L"type").set_value(L"sky");
    lightNode.attribute(L"distribution").set_value(L"map");

    auto intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
    intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"1.0");

    auto texNode = hrTextureBind(texEnv, intensityNode.child(L"color"));

    texNode.append_attribute(L"matrix");
    float samplerMatrix[16] = { 1, 0, 0, 0,
                                0, 1, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1 };

    texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
    texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");

    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

    VERIFY_XML(lightNode);
  }
  hrLightClose(sky);


  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Camera
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRCameraRef camRef = hrCameraCreate(L"my camera");

  hrCameraOpen(camRef, HR_WRITE_DISCARD);
  {
    auto camNode = hrCameraParamNode(camRef);

    camNode.append_child(L"fov").text().set(L"45");
    camNode.append_child(L"nearClipPlane").text().set(L"0.01");
    camNode.append_child(L"farClipPlane").text().set(L"100.0");

    camNode.append_child(L"up").text().set(L"0 1 0");
    camNode.append_child(L"position").text().set(L"0 13 16");
    camNode.append_child(L"look_at").text().set(L"0 0 0");
  }
  hrCameraClose(camRef);


  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Render settings
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 1024, 768, 256, 256);
  hrRenderOpen(renderRef, HR_OPEN_EXISTING);
  {
    auto node = hrRenderParamNode(renderRef);
    node.append_child(L"evalgbuffer").text() = L"1";
  }
  hrRenderClose(renderRef);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Create scene
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

  using namespace LiteMath;

  float4x4 mRot, mRot2;
  float4x4 mTranslate;
  float4x4 mScale;
  float4x4 mRes;

  const float DEG_TO_RAD = 0.01745329251f; // float(3.14159265358979323846f) / 180.0f;

  hrSceneOpen(scnRef, HR_WRITE_DISCARD);

  mTranslate.identity();
  mRes.identity();
  mRot.identity();

  mTranslate = translate4x4(float3(-6.0f, 1.5f, -4.5f));
  mRot = rotate4x4Y(60.0f*DEG_TO_RAD);
  mRes = mul(mTranslate, mRot);

  hrMeshInstance(scnRef, cubeR, mRes.L());

  ///////////

  mTranslate.identity();
  mRes.identity();
  mRot.identity();
  mRot2.identity();

  mTranslate = translate4x4(float3(6.0f, 1.5f, -4.5f));
  mRot = rotate4x4Y(-60.0f*DEG_TO_RAD);
  mRot2 = rotate4x4X(90.0f*DEG_TO_RAD);
  mRes = mul(mRot, mRot2);
  mRes = mul(mTranslate, mRes);

  hrMeshInstance(scnRef, torusB, mRes.L());


  ///////////

  mRes.identity();

  hrLightInstance(scnRef, sky, mRes.L());

  ///////////

  hrSceneClose(scnRef);


  HRSceneInstRef scnRef2 = HRUtils::MergeLibraryIntoLibrary(L"tests/test_715_lib_to_merge", false, true);

  mTranslate = scale4x4(float3(7.0f, 7.0f, 7.0f));

  HRUtils::InstanceSceneIntoScene(scnRef2, scnRef, mTranslate.L(), false);

  hrFlush(scnRef, renderRef, camRef);
  
  while (true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);

    if (info.haveUpdateFB)
    {
      auto pres = std::cout.precision(2);
      std::cout << "rendering progress = " << info.progress << "% \r";
      std::cout.precision(pres);
    }

    if (info.finalUpdate)
      break;
  }

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_715/z_out.png");
  hrRenderSaveGBufferLayerLDR(renderRef, L"tests_images/test_715/z_scnid.png", L"scnid");

  return check_images("test_715", 1, 60);
}

bool test_716_merge_scene_remap_override()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_716");

  hrSceneLibraryOpen(L"tests/test_716", HR_WRITE_DISCARD);


  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Materials
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


  HRMaterialRef matGray = hrMaterialCreate(L"matGray");
  HRMaterialRef matRefl = hrMaterialCreate(L"matRefl");


  hrMaterialOpen(matGray, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matGray);

    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.5 0.5 0.5");
  }
  hrMaterialClose(matGray);

  hrMaterialOpen(matRefl, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matRefl);

    auto refl = matNode.append_child(L"reflectivity");
    refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
    refl.append_child(L"color").append_attribute(L"val").set_value(L"0.8 0.8 0.8");
    refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.98");
    refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
    refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
    refl.append_child(L"fresnel_IOR").append_attribute(L"val").set_value(8.0f);
  }
  hrMaterialClose(matRefl);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Meshes
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  HRMeshRef cubeR = HRMeshFromSimpleMesh(L"cubeR", CreateCube(2.0f), matGray.id);
  HRMeshRef sphereG = HRMeshFromSimpleMesh(L"sphereG", CreateSphere(4.0f, 64), matRefl.id);
  HRMeshRef torusB = HRMeshFromSimpleMesh(L"torusB", CreateTorus(0.8f, 2.0f, 64, 64), matGray.id);
  HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), matGray.id);


  HRLightRef sky = hrLightCreate(L"sky");

  HRTextureNodeRef texEnv = hrTexture2DCreateFromFile(L"data/textures/Factory_Catwalk_2k_BLUR.exr");

  hrLightOpen(sky, HR_WRITE_DISCARD);
  {
    auto lightNode = hrLightParamNode(sky);

    lightNode.attribute(L"type").set_value(L"sky");
    lightNode.attribute(L"distribution").set_value(L"map");

    auto intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
    intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"1.0");

    auto texNode = hrTextureBind(texEnv, intensityNode.child(L"color"));

    texNode.append_attribute(L"matrix");
    float samplerMatrix[16] = { 1, 0, 0, 0,
                                0, 1, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1 };

    texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
    texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");

    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

    VERIFY_XML(lightNode);
  }
  hrLightClose(sky);


  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Camera
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRCameraRef camRef = hrCameraCreate(L"my camera");

  hrCameraOpen(camRef, HR_WRITE_DISCARD);
  {
    auto camNode = hrCameraParamNode(camRef);

    camNode.append_child(L"fov").text().set(L"45");
    camNode.append_child(L"nearClipPlane").text().set(L"0.01");
    camNode.append_child(L"farClipPlane").text().set(L"100.0");

    camNode.append_child(L"up").text().set(L"0 1 0");
    camNode.append_child(L"position").text().set(L"0 13 16");
    camNode.append_child(L"look_at").text().set(L"0 0 0");
  }
  hrCameraClose(camRef);


  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Render settings
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 1024, 768, 256, 1024);


  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Create scene
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

  using namespace LiteMath;

  float4x4 mRot, mRot2;
  float4x4 mTranslate;
  float4x4 mScale;
  float4x4 mRes;

  const float DEG_TO_RAD = 0.01745329251f; // float(3.14159265358979323846f) / 180.0f;

  hrSceneOpen(scnRef, HR_WRITE_DISCARD);

  mTranslate.identity();
  mRes.identity();
  mRot.identity();

  mTranslate = translate4x4(float3(-6.0f, 1.5f, -4.5f));
  mRot = rotate4x4Y(60.0f*DEG_TO_RAD);
  mRes = mul(mTranslate, mRot);

  hrMeshInstance(scnRef, cubeR, mRes.L());

  ///////////

  mTranslate.identity();
  mRes.identity();
  mRot.identity();
  mRot2.identity();

  mTranslate = translate4x4(float3(6.0f, 1.5f, -4.5f));
  mRot = rotate4x4Y(-60.0f*DEG_TO_RAD);
  mRot2 = rotate4x4X(90.0f*DEG_TO_RAD);
  mRes = mul(mRot, mRot2);
  mRes = mul(mTranslate, mRes);

  hrMeshInstance(scnRef, torusB, mRes.L());


  ///////////

  mRes.identity();

  hrLightInstance(scnRef, sky, mRes.L());

  ///////////

  hrSceneClose(scnRef);


  HRSceneInstRef scnRef2 = HRUtils::MergeLibraryIntoLibrary(L"tests/test_715_lib_to_merge", false, true);

  hrCommit(scnRef);

  auto materials = HydraXMLHelpers::GetMaterialNameToIdMap();


  mTranslate = scale4x4(float3(7.0f, 7.0f, 7.0f));

  //std::vector<int32_t> override_list = {4, matRefl.id};

  std::vector<int32_t> override_list = {4, 13, 6, 12, 7, 11, 8, 13, 0, 1, 2, 3, 10, 15, 20, 25};
  //std::vector<int32_t> override_list = {4, 13, 6, 13, 7, 13, 8, 13, 0, 1, 2, 3, 10, 15, 20, 25};
  //std::vector<int32_t> override_list = {4, 12};

  HRUtils::InstanceSceneIntoScene(scnRef2, scnRef, mTranslate.L(), false, &override_list[0], int32_t(override_list.size()));

  hrFlush(scnRef, renderRef, camRef);

  glViewport(0, 0, 1024, 768);
  std::vector<int32_t> image(1024 * 768);

  while (true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);

    if (info.haveUpdateFB)
    {
      hrRenderGetFrameBufferLDR1i(renderRef, 1024, 768, &image[0]);

      glDisable(GL_TEXTURE_2D);
      glDrawPixels(1024, 768, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);

      auto pres = std::cout.precision(2);
      std::cout << "rendering progress = " << info.progress << "% \r";
      std::cout.precision(pres);
    }

    if (info.finalUpdate)
      break;
  }

  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_716/z_out.png");

  return check_images("test_716", 1, 60);
}

bool test_717_merge_scene_scene_id_mask()
{
  initGLIfNeeded();

  hrErrorCallerPlace(L"test_717");

  hrSceneLibraryOpen(L"tests/test_717", HR_WRITE_DISCARD);


  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Materials
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


  HRMaterialRef matGray = hrMaterialCreate(L"matGray");
  HRMaterialRef matRefl = hrMaterialCreate(L"matRefl");


  hrMaterialOpen(matGray, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matGray);

    auto diff = matNode.append_child(L"diffuse");
    diff.append_attribute(L"brdf_type").set_value(L"lambert");

    auto color = diff.append_child(L"color");
    color.append_attribute(L"val").set_value(L"0.5 0.5 0.5");
  }
  hrMaterialClose(matGray);

  hrMaterialOpen(matRefl, HR_WRITE_DISCARD);
  {
    auto matNode = hrMaterialParamNode(matRefl);

    auto refl = matNode.append_child(L"reflectivity");
    refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
    refl.append_child(L"color").append_attribute(L"val").set_value(L"0.8 0.8 0.8");
    refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.98");
    refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
    refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
    refl.append_child(L"fresnel_IOR").append_attribute(L"val").set_value(8.0f);
  }
  hrMaterialClose(matRefl);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Meshes
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  HRMeshRef cubeR = HRMeshFromSimpleMesh(L"cubeR", CreateCube(2.0f), matGray.id);
  HRMeshRef torusB = HRMeshFromSimpleMesh(L"torusB", CreateTorus(0.8f, 2.0f, 64, 64), matGray.id);

  HRLightRef sky = hrLightCreate(L"sky");

  HRTextureNodeRef texEnv = hrTexture2DCreateFromFile(L"data/textures/Factory_Catwalk_2k_BLUR.exr");

  hrLightOpen(sky, HR_WRITE_DISCARD);
  {
    auto lightNode = hrLightParamNode(sky);

    lightNode.attribute(L"type").set_value(L"sky");
    lightNode.attribute(L"distribution").set_value(L"map");

    auto intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
    intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"1.0");

    auto texNode = hrTextureBind(texEnv, intensityNode.child(L"color"));

    texNode.append_attribute(L"matrix");
    float samplerMatrix[16] = { 1, 0, 0, 0,
                                0, 1, 0, 0,
                                0, 0, 1, 0,
                                0, 0, 0, 1 };

    texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
    texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");

    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

    VERIFY_XML(lightNode);
  }
  hrLightClose(sky);


  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Camera
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRCameraRef camRef = hrCameraCreate(L"my camera");

  hrCameraOpen(camRef, HR_WRITE_DISCARD);
  {
    auto camNode = hrCameraParamNode(camRef);

    camNode.append_child(L"fov").text().set(L"45");
    camNode.append_child(L"nearClipPlane").text().set(L"0.01");
    camNode.append_child(L"farClipPlane").text().set(L"100.0");

    camNode.append_child(L"up").text().set(L"0 1 0");
    camNode.append_child(L"position").text().set(L"0 13 16");
    camNode.append_child(L"look_at").text().set(L"0 0 0");
  }
  hrCameraClose(camRef);


  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Render settings
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 1024, 768, 32, 512);
  hrRenderOpen(renderRef, HR_OPEN_EXISTING);
  {
    auto node = hrRenderParamNode(renderRef);
    node.append_child(L"evalgbuffer").text() = L"1";
  }
  hrRenderClose(renderRef);

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Create scene
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

  using namespace LiteMath;

  float4x4 mRot, mRot2;
  float4x4 mTranslate;
  float4x4 mScale;
  float4x4 mRes;

  const float DEG_TO_RAD = 0.01745329251f; // float(3.14159265358979323846f) / 180.0f;

  hrSceneOpen(scnRef, HR_WRITE_DISCARD);

  mTranslate.identity();
  mRes.identity();
  mRot.identity();

  mTranslate = translate4x4(float3(-6.0f, 1.5f, -4.5f));
  mRot = rotate4x4Y(60.0f*DEG_TO_RAD);
  mRes = mul(mTranslate, mRot);

  hrMeshInstance(scnRef, cubeR, mRes.L());

  ///////////

  mTranslate.identity();
  mRes.identity();
  mRot.identity();
  mRot2.identity();

  mTranslate = translate4x4(float3(6.0f, 1.5f, -4.5f));
  mRot = rotate4x4Y(-60.0f*DEG_TO_RAD);
  mRot2 = rotate4x4X(90.0f*DEG_TO_RAD);
  mRes = mul(mRot, mRot2);
  mRes = mul(mTranslate, mRes);

  hrMeshInstance(scnRef, torusB, mRes.L());


  ///////////

  mRes.identity();

  hrLightInstance(scnRef, sky, mRes.L());

  ///////////

  hrSceneClose(scnRef);


  HRSceneInstRef scnRef2 = HRUtils::MergeLibraryIntoLibrary(L"tests/test_715_lib_to_merge", false, true);
  mTranslate = translate4x4(float3(-6.0f, -2.0f, 0.0f));
  mScale = scale4x4(float3(3.0f, 3.0f, 3.0f));
  mRes = mul(mTranslate, mScale);
  auto box = HRUtils::InstanceSceneIntoScene(scnRef2, scnRef, mRes.L(), false);


  HRSceneInstRef scnRef3 = HRUtils::MergeLibraryIntoLibrary(L"tests/test_715_lib_to_merge", false, true);
  mTranslate = translate4x4(float3(6.0f, -2.0f, 0.0f));
  mScale = scale4x4(float3(3.0f, 3.0f, 3.0f));
  mRes = mul(mTranslate, mScale);
  HRUtils::InstanceSceneIntoScene(scnRef3, scnRef, mRes.L(), false);

  mTranslate = translate4x4(float3(0.0f, -4.0f, 0.0f));
  mScale = scale4x4(float3(3.0f, 3.0f, 3.0f));
  mRes = mul(mTranslate, mScale);
  HRUtils::InstanceSceneIntoScene(scnRef3, scnRef, mRes.L(), false);

  hrFlush(scnRef, renderRef, camRef);
  
  while (true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);

    if (info.haveUpdateFB)
    {
      auto pres = std::cout.precision(2);
      std::cout << "rendering progress = " << info.progress << "% \r";
      std::cout.precision(pres);
    }

    if (info.finalUpdate)
      break;
  }

  hrRenderSaveFrameBufferLDR(renderRef,  L"tests_images/test_717/z_out.png");
  hrRenderSaveGBufferLayerLDR(renderRef, L"tests_images/test_717/z_out2.png", L"scnid");
  hrRenderSaveGBufferLayerLDR(renderRef, L"tests_images/test_717/z_out3.png", L"scnsid");
  hrRenderSaveGBufferLayerLDR(renderRef, L"tests_images/test_717/z_out4.png", L"instid");
  hrRenderSaveGBufferLayerLDR(renderRef, L"tests_images/test_717/z_out5.png", L"objid");


  return check_images("test_717", 1, 60);
}