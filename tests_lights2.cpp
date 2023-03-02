#include "tests.h"
#include <iomanip>
#include <cstring>
#include <stdlib.h>
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

///////////////////////////////////////////////////////////////////////////////////

#pragma warning(disable:4996)
#pragma warning(disable:4838)
#pragma warning(disable:4244)

extern GLFWwindow* g_window;
using namespace TEST_UTILS;


namespace LGHT_TESTS
{
  const int TEST_SCREEN_SIZE = 512;

  bool test_219_cylinder_tex()
  {
    std::wstring nameTest                = L"test_219";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.c_str(), HR_WRITE_DISCARD);
    
    ////////////////////
    // Materials
    ////////////////////

    HRMaterialRef matGray = hrMaterialCreate(L"matGray");

    hrMaterialOpen(matGray, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGray);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.5 0.5 0.5");

      VERIFY_XML(matNode);
    }
    hrMaterialClose(matGray);


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Meshes
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    HRMeshRef sph1     = HRMeshFromSimpleMesh(L"sph1", CreateSphere(2.0f, 64), matGray.id);
    HRMeshRef sph2     = HRMeshFromSimpleMesh(L"sph2", CreateSphere(2.0f, 64), matGray.id);
    HRMeshRef cubeOpen = HRMeshFromSimpleMesh(L"my_cube", CreateCubeOpen(6.0f), matGray.id);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Light textures
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    float hdrTexData[][4] = { { 0.0f, 0.0f, 0.0f, 0.0f },
                              { 1.0f, 1.0f, 1.0f, 0.0f },
                              { 1.0f, 2.0f, 1.0f, 0.0f },
                              { 1.0f, 2.0f, 1.0f, 0.0f },
                              { 2.0f, 2.0f, 1.0f, 0.0f },
                              { 2.0f, 1.0f, 2.0f, 0.0f },
                              { 1.0f, 1.0f, 2.0f, 0.0f },
                              { 1.0f, 1.0f, 1.0f, 0.0f },
                              { 2.0f, 1.0f, 1.0f, 0.0f },
                              { 2.0f, 1.0f, 1.0f, 0.0f },
                              { 1.0f, 0.0f, 1.0f, 0.0f },
                              { 1.0f, 1.0f, 0.0f, 0.0f },
                              { 1.0f, 0.0f, 1.0f, 0.0f },
                              { 0.0f, 0.0f, 0.0f, 0.0f },
    };

    constexpr int arraySize = sizeof(hdrTexData) / (sizeof(float) * 4);

    for (int i = 0; i < arraySize; i++)
    {
      hdrTexData[i][0] *= 0.5f;
      hdrTexData[i][1] *= 0.5f;
      hdrTexData[i][2] *= 0.5f;
      hdrTexData[i][3] *= 0.5f;
    }

    HRTextureNodeRef texForLight = hrTexture2DCreateFromMemory(arraySize, 1, 16, hdrTexData[0]);


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Light
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRLightRef cylinder2 = hrLightCreate(L"cylinder2");

    hrLightOpen(cylinder2, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(cylinder2);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"cylinder");
      lightNode.attribute(L"distribution").set_value(L"diffuse");

      pugi::xml_node sizeNode = lightNode.append_child(L"size");
      sizeNode.append_attribute(L"radius").set_value(0.5f);
      sizeNode.append_attribute(L"height").set_value(4.0f);
      sizeNode.append_attribute(L"angle").set_value(360.0f);

      auto intensityNode = lightNode.append_child(L"intensity");
      auto colorNode     = intensityNode.append_child(L"color");
      auto multNode      = intensityNode.append_child(L"multiplier");

      colorNode.append_attribute(L"val").set_value(L"1 1 1");
      multNode.append_attribute(L"val").set_value(1.0f*IRRADIANCE_TO_RADIANCE);

      auto texNode = hrTextureBind(texForLight, colorNode);

      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(L"1.0");       //#NOTE: important for HDR textures, input_gamma must be specified !!!

      VERIFY_XML(lightNode);
    }
    hrLightClose(cylinder2);

    HRLightRef spot3 = hrLightCreate(L"spot3");

    hrLightOpen(spot3, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(spot3);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"rect");
      lightNode.attribute(L"distribution").set_value(L"diffuse");

      auto sizeNode = lightNode.append_child(L"size");
      sizeNode.append_attribute(L"half_length").set_value(1.0f);
      sizeNode.append_attribute(L"half_width").set_value(1.0f);

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(1.0f*IRRADIANCE_TO_RADIANCE);

      VERIFY_XML(lightNode);
    }
    hrLightClose(spot3);


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
      camNode.append_child(L"position").text().set(L"0 3 18");
      camNode.append_child(L"look_at").text().set(L"0 3 0");

      VERIFY_XML(camNode);
    }
    hrCameraClose(camRef);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Render settings
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRRenderRef renderRef = hrRenderCreate(L"HydraModern");

    hrRenderEnableDevice(renderRef, CURR_RENDER_DEVICE, true);
    //hrRenderEnableDevice(renderRef, 0, true);
    //hrRenderEnableDevice(renderRef, 1, true);

    hrRenderOpen(renderRef, HR_WRITE_DISCARD);
    {
      auto node = hrRenderParamNode(renderRef);

      node.append_child(L"width").text()  = 512;
      node.append_child(L"height").text() = 512;

      node.append_child(L"method_primary").text()   = L"pathtracing";
      node.append_child(L"method_secondary").text() = L"pathtracing";
      node.append_child(L"method_tertiary").text()  = L"pathtracing";
      node.append_child(L"method_caustic").text()   = L"pathtracing";
      node.append_child(L"shadows").text()          = L"1";

      node.append_child(L"trace_depth").text()      = L"5";
      node.append_child(L"diff_trace_depth").text() = L"3";

      node.append_child(L"pt_error").text()         = L"2";
      node.append_child(L"minRaysPerPixel").text()  = 256;
      node.append_child(L"maxRaysPerPixel").text()  = 2048;
    }
    hrRenderClose(renderRef);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Create scene
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

    using namespace LiteMath;

    float4x4 mRot, mTranslate, mScale, mRes;
    const float DEG_TO_RAD = 0.01745329251f; // float(3.14159265358979323846f) / 180.0f;

    hrSceneOpen(scnRef, HR_WRITE_DISCARD);
    ///////////

    mTranslate = translate4x4(float3(0.0f, 3.0f, 0.0f));
    mRot = rotate4x4Y(180.0f*DEG_TO_RAD);
    mRes = mul(mTranslate, mRot);

    hrMeshInstance(scnRef, cubeOpen, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(-3.0f, 1.25f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph1, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(3.0f, 2.25f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph2, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(-3.0f, 8.0f, 0.0f));
    mRot = rotate4x4Y(90.0f*DEG_TO_RAD);
    mRes = mul(mTranslate, mRot);

    hrLightInstance(scnRef, cylinder2, mRes.L());

    ///////////
    float4x4 mRot2;
    mTranslate = translate4x4(float3(4.0f, -1.5f, 0.0f));
    mRot  = rotate4x4Y(20.0f*DEG_TO_RAD);
    mRot2 = rotate4x4X(-20.0f*DEG_TO_RAD);
    mRes  = mul(mTranslate, mul(mRot, mRot2));

    hrLightInstance(scnRef, cylinder2, mRes.L());

    ///////////

    hrSceneClose(scnRef);

    hrFlush(scnRef, renderRef);

    int counter = 0;

    ////////////////////
    // Rendering, save and check image
    ////////////////////

    RenderProgress(renderRef);

    std::filesystem::create_directories(saveRenderFile.parent_path());
    hrRenderSaveFrameBufferLDR(renderRef, saveRenderFile.c_str());

    return check_images(ws2s(nameTest).c_str(),1 , 25);
  }


  bool test_220_cylinder_tex2()
  {
    std::wstring nameTest                = L"test_220";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.c_str(), HR_WRITE_DISCARD);
    
    ////////////////////
    // Materials
    ////////////////////

    HRMaterialRef matGray = hrMaterialCreate(L"matGray");

    hrMaterialOpen(matGray, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGray);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.5 0.5 0.5");

      VERIFY_XML(matNode);
    }
    hrMaterialClose(matGray);


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Meshes
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    HRMeshRef sph1     = HRMeshFromSimpleMesh(L"sph1",    CreateSphere(2.0f, 64), matGray.id);
    HRMeshRef sph2     = HRMeshFromSimpleMesh(L"sph2",    CreateSphere(2.0f, 64), matGray.id);
    HRMeshRef cubeOpen = HRMeshFromSimpleMesh(L"my_cube", CreateCubeOpen(6.0f), matGray.id);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Light textures
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRTextureNodeRef texForLight = hrTexture2DCreateFromFile(L"data/textures/coloredCylinder.png");

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Light
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRLightRef spot2 = hrLightCreate(L"spot2");

    hrLightOpen(spot2, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(spot2);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"cylinder");
      lightNode.attribute(L"distribution").set_value(L"diffuse");

      pugi::xml_node sizeNode = lightNode.append_child(L"size");
      sizeNode.append_attribute(L"radius").set_value(0.5f);
      sizeNode.append_attribute(L"height").set_value(4.0f);
      sizeNode.append_attribute(L"angle").set_value(360.0f);

      auto intensityNode = lightNode.append_child(L"intensity");
      auto colorNode = intensityNode.append_child(L"color");
      auto multNode = intensityNode.append_child(L"multiplier");

      colorNode.append_attribute(L"val").set_value(L"1 1 1");
      multNode.append_attribute(L"val").set_value(1.0f*IRRADIANCE_TO_RADIANCE);

      auto texNode = hrTextureBind(texForLight, colorNode);

      texNode.append_attribute(L"addressing_mode_u").set_value(L"clamp");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"clamp");

      VERIFY_XML(lightNode);
    }
    hrLightClose(spot2);

    HRLightRef spot3 = hrLightCreate(L"spot3");

    hrLightOpen(spot3, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(spot3);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"rect");
      lightNode.attribute(L"distribution").set_value(L"diffuse");

      auto sizeNode = lightNode.append_child(L"size");
      sizeNode.append_attribute(L"half_length").set_value(1.0f);
      sizeNode.append_attribute(L"half_width").set_value(1.0f);

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(1.0f*IRRADIANCE_TO_RADIANCE);

      VERIFY_XML(lightNode);
    }
    hrLightClose(spot3);


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
      camNode.append_child(L"position").text().set(L"0 3 18");
      camNode.append_child(L"look_at").text().set(L"0 3 0");

      VERIFY_XML(camNode);
    }
    hrCameraClose(camRef);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Render settings
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRRenderRef renderRef = hrRenderCreate(L"HydraModern");

    hrRenderEnableDevice(renderRef, CURR_RENDER_DEVICE, true);
    //hrRenderEnableDevice(renderRef, 0, true);
    //hrRenderEnableDevice(renderRef, 1, true);

    hrRenderOpen(renderRef, HR_WRITE_DISCARD);
    {
      auto node = hrRenderParamNode(renderRef);

      node.append_child(L"width").text()  = 512;
      node.append_child(L"height").text() = 512;

      node.append_child(L"method_primary").text() = L"pathtracing";
      node.append_child(L"method_secondary").text() = L"pathtracing";
      node.append_child(L"method_tertiary").text() = L"pathtracing";
      node.append_child(L"method_caustic").text() = L"pathtracing";
      node.append_child(L"shadows").text() = L"1";

      node.append_child(L"trace_depth").text() = L"5";
      node.append_child(L"diff_trace_depth").text() = L"3";

      node.append_child(L"pt_error").text() = L"2";
      node.append_child(L"minRaysPerPixel").text() = 256;
      node.append_child(L"maxRaysPerPixel").text() = 2048;
    }
    hrRenderClose(renderRef);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Create scene
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

    using namespace LiteMath;

    float4x4 mRot, mTranslate, mScale, mRes;
    const float DEG_TO_RAD = 0.01745329251f; // float(3.14159265358979323846f) / 180.0f;

    hrSceneOpen(scnRef, HR_WRITE_DISCARD);
    ///////////

    mTranslate = translate4x4(float3(0.0f, 3.0f, 0.0f));
    mRot = rotate4x4Y(180.0f*DEG_TO_RAD);
    mRes = mul(mTranslate, mRot);

    hrMeshInstance(scnRef, cubeOpen, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(-3.0f, 1.25f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph1, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(3.0f, 2.25f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph2, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(-3.0f, 8.0f, 0.0f));
    mRot = rotate4x4Y(90.0f*DEG_TO_RAD);
    mRes = mul(mTranslate, mRot);

    hrLightInstance(scnRef, spot2, mRes.L());

    ///////////
    float4x4 mRot2;
    mTranslate = translate4x4(float3(4.0f, -1.5f, 0.0f));
    mRot = rotate4x4Y(20.0f*DEG_TO_RAD);
    mRot2 = rotate4x4X(-20.0f*DEG_TO_RAD);
    mRes = mul(mTranslate, mul(mRot, mRot2));

    hrLightInstance(scnRef, spot2, mRes.L());

    ///////////

    hrSceneClose(scnRef);

    hrFlush(scnRef, renderRef);

    int counter = 0;

    ////////////////////
    // Rendering, save and check image
    ////////////////////

    RenderProgress(renderRef);

    std::filesystem::create_directories(saveRenderFile.parent_path());
    hrRenderSaveFrameBufferLDR(renderRef, saveRenderFile.c_str());

    return check_images(ws2s(nameTest).c_str(), 1, 25);
  }


  bool test_221_cylinder_tex3()
  {
    std::wstring nameTest                = L"test_221";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.c_str(), HR_WRITE_DISCARD);
    
    ////////////////////
    // Materials
    ////////////////////

    HRMaterialRef matGray = hrMaterialCreate(L"matGray");

    hrMaterialOpen(matGray, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGray);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.5 0.5 0.5");

      VERIFY_XML(matNode);
    }
    hrMaterialClose(matGray);


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Meshes
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    HRMeshRef sph1 = HRMeshFromSimpleMesh(L"sph1", CreateSphere(2.0f, 64), matGray.id);
    HRMeshRef sph2 = HRMeshFromSimpleMesh(L"sph2", CreateSphere(2.0f, 64), matGray.id);
    HRMeshRef cubeOpen = HRMeshFromSimpleMesh(L"my_cube", CreateCubeOpen(6.0f), matGray.id);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Light textures
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    float hdrTexData[][4] = { { 0.0f, 0.0f, 0.0f, 0.0f },
                              { 0.0f, 0.0f, 0.0f, 0.0f },
                              { 1.0f, 1.0f, 1.0f, 0.0f },
                              { 0.0f, 0.0f, 0.0f, 0.0f },
                              { 0.0f, 0.0f, 0.0f, 0.0f },
                              { 0.0f, 0.0f, 0.0f, 0.0f },
                              { 0.0f, 0.0f, 0.0f, 0.0f },
                              { 0.0f, 0.0f, 0.0f, 0.0f },
                              { 0.0f, 0.0f, 0.0f, 0.0f },
                              { 0.0f, 0.0f, 0.0f, 0.0f },
                              { 0.0f, 0.0f, 0.0f, 0.0f },
                              { 0.0f, 0.0f, 0.0f, 0.0f },
                              { 0.0f, 0.0f, 0.0f, 0.0f },
                              { 0.0f, 0.0f, 0.0f, 0.0f },
                              { 0.0f, 0.0f, 0.0f, 0.0f },
                              { 0.0f, 0.0f, 0.0f, 0.0f },
                              { 0.0f, 0.0f, 0.0f, 0.0f },
                              { 0.0f, 0.0f, 0.0f, 0.0f },
                              { 0.0f, 0.0f, 0.0f, 0.0f },
                              { 0.0f, 0.0f, 0.0f, 0.0f },
                              { 0.0f, 0.0f, 0.0f, 0.0f },
                              { 0.0f, 0.0f, 0.0f, 0.0f },
                              { 0.0f, 0.0f, 0.0f, 0.0f },
                              { 0.0f, 0.0f, 0.0f, 0.0f },
                              { 0.0f, 0.0f, 0.0f, 0.0f },
                              { 1.0f, 1.0f, 1.0f, 0.0f },
                              { 0.0f, 0.0f, 0.0f, 0.0f },
                              { 0.0f, 0.0f, 0.0f, 0.0f },
    };

    constexpr int arraySize = sizeof(hdrTexData) / (sizeof(float) * 4);

    for (int i = 0; i < arraySize; i++)
    {
      hdrTexData[i][0] *= 5.0f;
      hdrTexData[i][1] *= 5.0f;
      hdrTexData[i][2] *= 5.0f;
      hdrTexData[i][3] *= 5.0f;
    }

    HRTextureNodeRef texForLight = hrTexture2DCreateFromMemory(arraySize, 1, 16, hdrTexData[0]);


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Light
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRLightRef spot2 = hrLightCreate(L"spot2");

    hrLightOpen(spot2, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(spot2);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"cylinder");
      lightNode.attribute(L"distribution").set_value(L"diffuse");

      pugi::xml_node sizeNode = lightNode.append_child(L"size");
      sizeNode.append_attribute(L"radius").set_value(0.5f);
      sizeNode.append_attribute(L"height").set_value(4.0f);
      sizeNode.append_attribute(L"angle").set_value(360.0f);

      auto intensityNode = lightNode.append_child(L"intensity");
      auto colorNode = intensityNode.append_child(L"color");
      auto multNode = intensityNode.append_child(L"multiplier");

      colorNode.append_attribute(L"val").set_value(L"1 1 1");
      multNode.append_attribute(L"val").set_value(1.0f*IRRADIANCE_TO_RADIANCE);

      auto texNode = hrTextureBind(texForLight, colorNode);

      texNode.append_attribute(L"addressing_mode_u").set_value(L"clamp");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"clamp");
      texNode.append_attribute(L"input_gamma").set_value(L"1.0");              // #NOTE: Important for HDR textures!!! input_gamma must be specified!!!

      VERIFY_XML(lightNode);
    }
    hrLightClose(spot2);

    HRLightRef spot3 = hrLightCreate(L"spot3");

    hrLightOpen(spot3, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(spot3);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"rect");
      lightNode.attribute(L"distribution").set_value(L"diffuse");

      auto sizeNode = lightNode.append_child(L"size");
      sizeNode.append_attribute(L"half_length").set_value(1.0f);
      sizeNode.append_attribute(L"half_width").set_value(1.0f);

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(1.0f*IRRADIANCE_TO_RADIANCE);

      VERIFY_XML(lightNode);
    }
    hrLightClose(spot3);


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
      camNode.append_child(L"position").text().set(L"0 3 18");
      camNode.append_child(L"look_at").text().set(L"0 3 0");

      VERIFY_XML(camNode);
    }
    hrCameraClose(camRef);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Render settings
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRRenderRef renderRef = hrRenderCreate(L"HydraModern");

    hrRenderEnableDevice(renderRef, CURR_RENDER_DEVICE, true);
    //hrRenderEnableDevice(renderRef, 0, true);
    //hrRenderEnableDevice(renderRef, 1, true);

    hrRenderOpen(renderRef, HR_WRITE_DISCARD);
    {
      auto node = hrRenderParamNode(renderRef);

      node.append_child(L"width").text()  = 512;
      node.append_child(L"height").text() = 512;

      node.append_child(L"method_primary").text() = L"pathtracing";
      node.append_child(L"method_secondary").text() = L"pathtracing";
      node.append_child(L"method_tertiary").text() = L"pathtracing";
      node.append_child(L"method_caustic").text() = L"pathtracing";
      node.append_child(L"shadows").text() = L"1";

      node.append_child(L"trace_depth").text() = L"5";
      node.append_child(L"diff_trace_depth").text() = L"3";

      node.append_child(L"pt_error").text() = L"2";
      node.append_child(L"minRaysPerPixel").text() = 256;
      node.append_child(L"maxRaysPerPixel").text() = 2048;
    }
    hrRenderClose(renderRef);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Create scene
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

    using namespace LiteMath;

    float4x4 mRot, mTranslate, mScale, mRes;
    const float DEG_TO_RAD = 0.01745329251f; // float(3.14159265358979323846f) / 180.0f;

    hrSceneOpen(scnRef, HR_WRITE_DISCARD);
    ///////////

    mTranslate = translate4x4(float3(0.0f, 3.0f, 0.0f));
    mRot = rotate4x4Y(180.0f*DEG_TO_RAD);
    mRes = mul(mTranslate, mRot);

    hrMeshInstance(scnRef, cubeOpen, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(-3.0f, 1.25f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph1, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(3.0f, 2.25f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph2, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(-3.0f, 8.0f, 0.0f));
    mRot = rotate4x4Y(90.0f*DEG_TO_RAD);
    mRes = mul(mTranslate, mRot);

    hrLightInstance(scnRef, spot2, mRes.L());

    ///////////
    float4x4 mRot2;
    mTranslate = translate4x4(float3(4.0f, -1.5f, 0.0f));
    mRot = rotate4x4Y(20.0f*DEG_TO_RAD);
    mRot2 = rotate4x4X(-20.0f*DEG_TO_RAD);
    mRes = mul(mTranslate, mul(mRot, mRot2));

    hrLightInstance(scnRef, spot2, mRes.L());

    ///////////

    hrSceneClose(scnRef);

    hrFlush(scnRef, renderRef);

    int counter = 0;

    ////////////////////
    // Rendering, save and check image
    ////////////////////

    RenderProgress(renderRef);

    std::filesystem::create_directories(saveRenderFile.parent_path());
    hrRenderSaveFrameBufferLDR(renderRef, saveRenderFile.c_str());

    return check_images(ws2s(nameTest).c_str(), 1, 50);
  }


  using LiteMath::float4x4;
  using LiteMath::float3;
  using LiteMath::translate4x4;
  using LiteMath::rotate4x4X;

  HRLightGroupExt MyCAD_CreateCylinderLightWithEndFace(const std::wstring& a_lightName, float cylinderHeight)
  {
    const std::wstring cylinderName = a_lightName + L"_cylinder_face";
    const std::wstring endFace1Name = a_lightName + L"_end_face_plus";
    const std::wstring endFace2Name = a_lightName + L"_end_face_minus";

    HRLightGroupExt res(3);
    
    res.lights[0] = hrLightCreate(cylinderName.c_str());
    res.lights[1] = hrLightCreate(endFace1Name.c_str());
    res.lights[2] = hrLightCreate(endFace2Name.c_str());

    ////////////////////////////////////////////////////////////////////////////////////////////////////

    const float DEG_TO_RAD = 0.01745329251f; // float(3.14159265358979323846f) / 180.0f;

    float4x4 m1, m2, m3;
    m1.identity();

    float4x4 translate1 = translate4x4(float3(0, 0, -0.5f*cylinderHeight));
    float4x4 translate2 = translate4x4(float3(0, 0, +0.5f*cylinderHeight));

    m2 = mul(translate1, rotate4x4X(DEG_TO_RAD*90.0f));
    m3 = mul(translate2, rotate4x4X(-DEG_TO_RAD*90.0f));

    memcpy(res.matrix[0], m1.L(), 16 * sizeof(float));
    memcpy(res.matrix[1], m2.L(), 16 * sizeof(float));
    memcpy(res.matrix[2], m3.L(), 16 * sizeof(float));

    ////////////////////////////////////////////////////////////////////////////////////////////////////

    return res;
  }


  bool test_222_cylinder_with_end_face()
  {
    std::wstring nameTest                = L"test_222";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.c_str(), HR_WRITE_DISCARD);
    
    ////////////////////
    // Materials
    ////////////////////

    HRMaterialRef matGray = hrMaterialCreate(L"matGray");

    hrMaterialOpen(matGray, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGray);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.5 0.5 0.5");

      VERIFY_XML(matNode);
    }
    hrMaterialClose(matGray);


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Meshes
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    HRMeshRef sph1     = HRMeshFromSimpleMesh(L"sph1", CreateSphere(2.0f, 64), matGray.id);
    HRMeshRef sph2     = HRMeshFromSimpleMesh(L"sph2", CreateSphere(2.0f, 64), matGray.id);
    HRMeshRef cubeOpen = HRMeshFromSimpleMesh(L"my_cube", CreateCubeOpen(6.0f), matGray.id);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Light
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    const float cylinderHeight = 4.0f;
    const float cylinderRadius = 0.5f;

    auto myGroup = MyCAD_CreateCylinderLightWithEndFace(L"MyCylinderLightWithEndFace", cylinderHeight);

    HRLightRef cylinder1 = myGroup.lights[0];

    hrLightOpen(cylinder1, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(cylinder1);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"cylinder");
      lightNode.attribute(L"distribution").set_value(L"diffuse");

      pugi::xml_node sizeNode = lightNode.append_child(L"size");
      sizeNode.append_attribute(L"radius").set_value(cylinderRadius);
      sizeNode.append_attribute(L"height").set_value(cylinderHeight);
      sizeNode.append_attribute(L"angle").set_value(360.0f);

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(1.0f*IRRADIANCE_TO_RADIANCE);

      VERIFY_XML(lightNode);
    }
    hrLightClose(cylinder1);

    HRLightRef disk_11 = myGroup.lights[1];

    hrLightOpen(disk_11, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(disk_11);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"disk");
      lightNode.attribute(L"distribution").set_value(L"diffuse");

      auto sizeNode = lightNode.append_child(L"size");
      sizeNode.append_attribute(L"radius").set_value(cylinderRadius);

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"0 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(1.0f*IRRADIANCE_TO_RADIANCE);

      VERIFY_XML(lightNode);
    }
    hrLightClose(disk_11);

    HRLightRef disk_12 = myGroup.lights[2];

    hrLightOpen(disk_12, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(disk_12);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"disk");
      lightNode.attribute(L"distribution").set_value(L"diffuse");

      auto sizeNode = lightNode.append_child(L"size");
      sizeNode.append_attribute(L"radius").set_value(cylinderRadius);

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 0 0");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(1.0f*IRRADIANCE_TO_RADIANCE);

      VERIFY_XML(lightNode);
    }
    hrLightClose(disk_12);



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
      camNode.append_child(L"position").text().set(L"0 3 18");
      camNode.append_child(L"look_at").text().set(L"0 3 0");

      VERIFY_XML(camNode);
    }
    hrCameraClose(camRef);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Render settings
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRRenderRef renderRef = hrRenderCreate(L"HydraModern");

    hrRenderEnableDevice(renderRef, CURR_RENDER_DEVICE, true);
    //hrRenderEnableDevice(renderRef, 0, true);
    //hrRenderEnableDevice(renderRef, 1, true);

    hrRenderOpen(renderRef, HR_WRITE_DISCARD);
    {
      auto node = hrRenderParamNode(renderRef);

      node.append_child(L"width").text()  = 512;
      node.append_child(L"height").text() = 512;

      node.append_child(L"method_primary").text() = L"pathtracing";
      node.append_child(L"method_secondary").text() = L"pathtracing";
      node.append_child(L"method_tertiary").text() = L"pathtracing";
      node.append_child(L"method_caustic").text() = L"pathtracing";
      node.append_child(L"shadows").text() = L"1";

      node.append_child(L"trace_depth").text() = L"5";
      node.append_child(L"diff_trace_depth").text() = L"3";

      node.append_child(L"pt_error").text() = L"2";
      node.append_child(L"minRaysPerPixel").text() = 256;
      node.append_child(L"maxRaysPerPixel").text() = 2048;
    }
    hrRenderClose(renderRef);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Create scene
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

    using namespace LiteMath;

    float4x4 mRot, mTranslate, mScale, mRes;
    const float DEG_TO_RAD = 0.01745329251f; // float(3.14159265358979323846f) / 180.0f;

    hrSceneOpen(scnRef, HR_WRITE_DISCARD);
    ///////////

    mTranslate = translate4x4(float3(0.0f, 3.0f, 0.0f));
    mRot = rotate4x4Y(180.0f*DEG_TO_RAD);
    mRes = mul(mTranslate, mRot);

    hrMeshInstance(scnRef, cubeOpen, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(-3.0f, 1.25f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph1, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(3.0f, 2.25f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph2, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(-3.0f, 8.0f, 0.0f));
    mRot       = rotate4x4Y(90.0f*DEG_TO_RAD);
    mRes       = mul(mTranslate, mRot);

    //hrLightInstance(scnRef, cylinder1, mRes.L());
    hrLightGroupInstanceExt(scnRef, myGroup, mRes.L());

    ///////////
    float4x4 mRot2;
    mTranslate = translate4x4(float3(4.0f, -1.5f, 0.0f));
    mRot       = rotate4x4Y(20.0f*DEG_TO_RAD);
    mRot2      = rotate4x4X(-20.0f*DEG_TO_RAD);
    mRes       = mul(mTranslate, mul(mRot, mRot2));

    //hrLightInstance(scnRef, cylinder1, mRes.L());
    hrLightGroupInstanceExt(scnRef, myGroup, mRes.L());

    ///////////

    hrSceneClose(scnRef);

    hrFlush(scnRef, renderRef);

    int counter = 0;

    ////////////////////
    // Rendering, save and check image
    ////////////////////

    RenderProgress(renderRef);

    std::filesystem::create_directories(saveRenderFile.parent_path());
    hrRenderSaveFrameBufferLDR(renderRef, saveRenderFile.c_str());

    return check_images(ws2s(nameTest).c_str(), 1, 25);
  }


  bool test_223_rotated_area_light()
  {
    std::wstring nameTest                = L"test_223";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.c_str(), HR_WRITE_DISCARD);
    
    ////////////////////
    // Materials
    ////////////////////

    SimpleMesh cube     = CreateCube(0.75f);
    SimpleMesh plane    = CreatePlane(10.0f);
    SimpleMesh sphere   = CreateSphere(1.0f, 32);
    SimpleMesh torus    = CreateTorus(0.25f, 0.6f, 32, 32);
    SimpleMesh cubeOpen = CreateCubeOpen(4.0f);

    for (size_t i = 0; i < plane.vTexCoord.size(); i++)
      plane.vTexCoord[i] *= 2.0f;


    HRTextureNodeRef testTex2 = hrTexture2DCreateFromFileDL(L"data/textures/chess_red.bmp");

    HRMaterialRef mat0 = hrMaterialCreate(L"mysimplemat");
    HRMaterialRef mat1 = hrMaterialCreate(L"mysimplemat2");
    HRMaterialRef mat2 = hrMaterialCreate(L"mysimplemat3");
    HRMaterialRef mat3 = hrMaterialCreate(L"mysimplemat4");
    HRMaterialRef mat4 = hrMaterialCreate(L"myblue");
    HRMaterialRef mat5 = hrMaterialCreate(L"mymatplane");

    HRMaterialRef mat6 = hrMaterialCreate(L"red");
    HRMaterialRef mat7 = hrMaterialCreate(L"green");
    HRMaterialRef mat8 = hrMaterialCreate(L"white");

    HRMaterialRef mat9 = hrMaterialCreate(L"glossy_wall_mat");

    hrMaterialOpen(mat0, HR_WRITE_DISCARD);
    {
      xml_node matNode = hrMaterialParamNode(mat0);

      xml_node diff = matNode.append_child(L"diffuse");

      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.75 0.5");

      HRTextureNodeRef testTex = hrTexture2DCreateFromFile(L"data/textures/texture1.bmp"); // hrTexture2DCreateFromFileDL
      hrTextureBind(testTex, diff);
    }
    hrMaterialClose(mat0);

    hrMaterialOpen(mat1, HR_WRITE_DISCARD);
    {
      xml_node matNode = hrMaterialParamNode(mat1);
      xml_node diff = matNode.append_child(L"diffuse");

      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").append_attribute(L"val").set_value(L"0.207843 0.188235 0");

      xml_node refl = matNode.append_child(L"reflectivity");

      refl.append_attribute(L"brdf_type").set_value(L"phong");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"0.367059 0.345882 0");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.5");
      refl.append_child(L"energy_fix").append_attribute(L"val") = 1;
    }
    hrMaterialClose(mat1);

    hrMaterialOpen(mat2, HR_WRITE_DISCARD);
    {
      xml_node matNode = hrMaterialParamNode(mat2);

      xml_node diff = matNode.append_child(L"diffuse");

      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").text().set(L"0.75 0.75 0.75");

      HRTextureNodeRef testTex = hrTexture2DCreateFromFile(L"data/textures/relief_wood.jpg");
      hrTextureBind(testTex, diff);
    }
    hrMaterialClose(mat2);

    hrMaterialOpen(mat3, HR_WRITE_DISCARD);
    {
      xml_node matNode = hrMaterialParamNode(mat3);

      xml_node diff = matNode.append_child(L"diffuse");

      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").text().set(L"0.75 0.75 0.75");

      HRTextureNodeRef testTex = hrTexture2DCreateFromFileDL(L"data/textures/163.jpg");
      hrTextureBind(testTex, diff);
    }
    hrMaterialClose(mat3);

    hrMaterialOpen(mat4, HR_WRITE_DISCARD);
    {
      xml_node matNode = hrMaterialParamNode(mat4);

      xml_node diff = matNode.append_child(L"diffuse");

      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").text().set(L"0.1 0.1 0.75");
    }
    hrMaterialClose(mat4);

    hrMaterialOpen(mat5, HR_WRITE_DISCARD);
    {
      xml_node matNode = hrMaterialParamNode(mat5);

      xml_node diff = matNode.append_child(L"diffuse");

      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").text().set(L"0.75 0.75 0.25");

      HRTextureNodeRef testTex = hrTexture2DCreateFromFileDL(L"data/textures/texture1.bmp");
      hrTextureBind(testTex, diff);
    }
    hrMaterialClose(mat5);


    hrMaterialOpen(mat6, HR_WRITE_DISCARD);
    {
      xml_node matNode = hrMaterialParamNode(mat6);
      xml_node diff = matNode.append_child(L"diffuse");

      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").text().set(L"0.5 0.0 0.0");
    }
    hrMaterialClose(mat6);

    hrMaterialOpen(mat7, HR_WRITE_DISCARD);
    {
      xml_node matNode = hrMaterialParamNode(mat7);
      xml_node diff = matNode.append_child(L"diffuse");

      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").text().set(L"0.0 0.5 0.0");
    }
    hrMaterialClose(mat7);

    hrMaterialOpen(mat8, HR_WRITE_DISCARD);
    {
      xml_node matNode = hrMaterialParamNode(mat8);
      xml_node diff = matNode.append_child(L"diffuse");

      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").text().set(L"0.5 0.5 0.5");
    }
    hrMaterialClose(mat8);

    hrMaterialOpen(mat9, HR_WRITE_DISCARD);
    {
      xml_node matNode = hrMaterialParamNode(mat9);
      xml_node diff = matNode.append_child(L"diffuse");

      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").append_attribute(L"val").set_value(L"0.0 0.0 0");

      xml_node refl = matNode.append_child(L"reflectivity");

      refl.append_attribute(L"brdf_type").set_value(L"phong");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"0.4 0.4 0.4");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.75");
      refl.append_child(L"energy_fix").append_attribute(L"val") = 1;
    }
    hrMaterialClose(mat9);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRMeshRef teapotRef = hrMeshCreateFromFileDL(L"data/meshes/teapot.vsgf"); // chunk_00009.vsgf // teapot.vsgf // chunk_00591.vsgf

    HRMeshRef cubeOpenRef = hrMeshCreate(L"my_box");
    HRMeshRef planeRef = hrMeshCreate(L"my_plane");
    HRMeshRef sphereRef = hrMeshCreate(L"my_sphere");
    HRMeshRef torusRef = hrMeshCreate(L"my_torus");

    hrMeshOpen(cubeOpenRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
    {
      hrMeshVertexAttribPointer4f(cubeOpenRef, L"pos", &cubeOpen.vPos[0]);
      hrMeshVertexAttribPointer4f(cubeOpenRef, L"norm", &cubeOpen.vNorm[0]);
      hrMeshVertexAttribPointer2f(cubeOpenRef, L"texcoord", &cubeOpen.vTexCoord[0]);

      int cubeMatIndices[10] = { mat8.id, mat8.id, mat8.id, mat8.id, mat9.id, mat9.id, mat7.id, mat7.id, mat6.id, mat6.id };

      //hrMeshMaterialId(cubeRef, 0);
      hrMeshPrimitiveAttribPointer1i(cubeOpenRef, L"mind", cubeMatIndices);
      hrMeshAppendTriangles3(cubeOpenRef, int(cubeOpen.triIndices.size()), &cubeOpen.triIndices[0]);
    }
    hrMeshClose(cubeOpenRef);


    hrMeshOpen(planeRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
    {
      hrMeshVertexAttribPointer4f(planeRef, L"pos", &plane.vPos[0]);
      hrMeshVertexAttribPointer4f(planeRef, L"norm", &plane.vNorm[0]);
      hrMeshVertexAttribPointer2f(planeRef, L"texcoord", &plane.vTexCoord[0]);

      hrMeshMaterialId(planeRef, mat5.id);
      hrMeshAppendTriangles3(planeRef, int32_t(plane.triIndices.size()), &plane.triIndices[0]);
    }
    hrMeshClose(planeRef);

    hrMeshOpen(sphereRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
    {
      hrMeshVertexAttribPointer4f(sphereRef, L"pos", &sphere.vPos[0]);
      hrMeshVertexAttribPointer4f(sphereRef, L"norm", &sphere.vNorm[0]);
      hrMeshVertexAttribPointer2f(sphereRef, L"texcoord", &sphere.vTexCoord[0]);

      for (size_t i = 0; i < sphere.matIndices.size() / 2; i++)
        sphere.matIndices[i] = mat0.id;

      for (size_t i = sphere.matIndices.size() / 2; i < sphere.matIndices.size(); i++)
        sphere.matIndices[i] = mat2.id;

      hrMeshPrimitiveAttribPointer1i(sphereRef, L"mind", &sphere.matIndices[0]);
      hrMeshAppendTriangles3(sphereRef, int32_t(sphere.triIndices.size()), &sphere.triIndices[0]);
    }
    hrMeshClose(sphereRef);

    hrMeshOpen(torusRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
    {
      hrMeshVertexAttribPointer4f(torusRef, L"pos", &torus.vPos[0]);
      hrMeshVertexAttribPointer4f(torusRef, L"norm", &torus.vNorm[0]);
      hrMeshVertexAttribPointer2f(torusRef, L"texcoord", &torus.vTexCoord[0]);

      for (size_t i = 0; i < torus.matIndices.size() / 3; i++)
        torus.matIndices[i] = mat0.id;

      for (size_t i = 1 * torus.matIndices.size() / 3; i < 2 * torus.matIndices.size() / 3; i++)
        torus.matIndices[i] = mat3.id;

      for (size_t i = 2 * torus.matIndices.size() / 3; i < torus.matIndices.size(); i++)
        torus.matIndices[i] = mat2.id;

      //hrMeshMaterialId(torusRef, mat0.id);
      hrMeshPrimitiveAttribPointer1i(torusRef, L"mind", &torus.matIndices[0]);
      hrMeshAppendTriangles3(torusRef, int32_t(torus.triIndices.size()), &torus.triIndices[0]);
    }
    hrMeshClose(torusRef);

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

      sizeNode.append_attribute(L"half_length").set_value(L"0.75");
      sizeNode.append_attribute(L"half_width").set_value(L"1.5");

      pugi::xml_node intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(10.0f*IRRADIANCE_TO_RADIANCE);
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
      camNode.append_child(L"position").text().set(L"0 0 15");
      camNode.append_child(L"look_at").text().set(L"0 0 0");
    }
    hrCameraClose(camRef);

    // set up render settings
    //
    HRRenderRef renderRef = hrRenderCreate(L"HydraModern"); // opengl1 

    hrRenderEnableDevice(renderRef, CURR_RENDER_DEVICE, true);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    hrRenderOpen(renderRef, HR_WRITE_DISCARD);
    {
      pugi::xml_node node = hrRenderParamNode(renderRef);

      node.append_child(L"width").text()  = TEST_SCREEN_SIZE;
      node.append_child(L"height").text() = TEST_SCREEN_SIZE;

      node.append_child(L"method_primary").text()   = L"pathtracing";
      node.append_child(L"method_secondary").text() = L"pathtracing";
      node.append_child(L"method_tertiary").text()  = L"pathtracing";
      node.append_child(L"method_caustic").text()   = L"pathtracing";
      node.append_child(L"shadows").text()          = L"1";

      node.append_child(L"trace_depth").text()      = L"6";
      node.append_child(L"diff_trace_depth").text() = L"3";
      node.append_child(L"maxRaysPerPixel").text()  = 4096;
    }
    hrRenderClose(renderRef);

    // create scene
    //
    HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

    static GLfloat	rtri = 25.0f; // Angle For The Triangle ( NEW )
    static GLfloat	rquad = 40.0f;
    static float    g_FPS = 60.0f;
    static int      frameCounter = 0;

    const float DEG_TO_RAD = float(3.14159265358979323846f) / 180.0f;

    float matrixT[4][4];
    float mRot1[4][4], mRot2[4][4], mTranslate[4][4], mRes[4][4];
    //float mTranslateDown[4][4];

    hrSceneOpen(scnRef, HR_WRITE_DISCARD);

    int mmIndex = 0;
    mat4x4_identity(mRot1);
    mat4x4_identity(mTranslate);
    mat4x4_identity(mRes);

    mat4x4_translate(mTranslate, 0.0f, -0.70f*3.65f, 0.0f);
    mat4x4_scale(mRot1, mRot1, 3.65f);
    mat4x4_mul(mRes, mTranslate, mRot1);
    mat4x4_transpose(matrixT, mRes); // this fucking math library swap rows and columns
    matrixT[3][3] = 1.0f;

    hrMeshInstance(scnRef, teapotRef, &matrixT[0][0]);

    mat4x4_identity(mRot1);
    mat4x4_rotate_Y(mRot1, mRot1, 180.0f*DEG_TO_RAD);
    //mat4x4_rotate_Y(mRot1, mRot1, rquad*DEG_TO_RAD);
    mat4x4_transpose(matrixT, mRot1);
    hrMeshInstance(scnRef, cubeOpenRef, &matrixT[0][0]);

    /////////////////////////////////////////////////////////////////////// instance light (!!!)

    mat4x4_identity(mTranslate);
    mat4x4_identity(mRot1);
    mat4x4_identity(mRot2);

    mat4x4_translate(mTranslate, -2.5f, 3.25f, -1.5);
    mat4x4_rotate_Y(mRot1, mRot1, -20.0f*DEG_TO_RAD);
    mat4x4_rotate_X(mRot2, mRot2, 25.0f*DEG_TO_RAD);
    mat4x4_mul(mRot1, mRot2, mRot1);
    mat4x4_mul(mRes, mTranslate, mRot1);
    mat4x4_transpose(matrixT, mRes);
    hrLightInstance(scnRef, rectLight, &matrixT[0][0]);

    hrSceneClose(scnRef);

    hrFlush(scnRef, renderRef);
    
    ////////////////////
    // Rendering, save and check image
    ////////////////////

    RenderProgress(renderRef);

    std::filesystem::create_directories(saveRenderFile.parent_path());
    hrRenderSaveFrameBufferLDR(renderRef, saveRenderFile.c_str());

    return check_images(ws2s(nameTest).c_str(), 1, 20);
  }


  bool test_224_rotated_area_light2()
  {
    std::wstring nameTest                 = L"test_224";
    std::filesystem::path libraryPath     = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile  = L"tests_images/" + nameTest + L"/z_out.png";
    std::filesystem::path saveRenderFile2 = L"tests_images/" + nameTest + L"/z_out2.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.c_str(), HR_WRITE_DISCARD);
    
    ////////////////////
    // Materials
    ////////////////////

    SimpleMesh cube = CreateCube(0.75f);
    SimpleMesh plane = CreatePlane(10.0f);
    SimpleMesh sphere = CreateSphere(1.0f, 32);
    SimpleMesh torus = CreateTorus(0.25f, 0.6f, 32, 32);
    SimpleMesh cubeOpen = CreateCubeOpen(4.0f);

    for (size_t i = 0; i < plane.vTexCoord.size(); i++)
      plane.vTexCoord[i] *= 2.0f;


    HRTextureNodeRef testTex2 = hrTexture2DCreateFromFileDL(L"data/textures/chess_red.bmp");

    HRMaterialRef mat0 = hrMaterialCreate(L"mysimplemat");
    HRMaterialRef mat1 = hrMaterialCreate(L"mysimplemat2");
    HRMaterialRef mat2 = hrMaterialCreate(L"mysimplemat3");
    HRMaterialRef mat3 = hrMaterialCreate(L"mysimplemat4");
    HRMaterialRef mat4 = hrMaterialCreate(L"myblue");
    HRMaterialRef mat5 = hrMaterialCreate(L"mymatplane");

    HRMaterialRef mat6 = hrMaterialCreate(L"red");
    HRMaterialRef mat7 = hrMaterialCreate(L"green");
    HRMaterialRef mat8 = hrMaterialCreate(L"white");

    HRMaterialRef mat9 = hrMaterialCreate(L"glossy_wall_mat");

    hrMaterialOpen(mat0, HR_WRITE_DISCARD);
    {
      xml_node matNode = hrMaterialParamNode(mat0);

      xml_node diff = matNode.append_child(L"diffuse");

      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.75 0.5");

      HRTextureNodeRef testTex = hrTexture2DCreateFromFile(L"data/textures/texture1.bmp"); // hrTexture2DCreateFromFileDL
      hrTextureBind(testTex, diff);
    }
    hrMaterialClose(mat0);

    hrMaterialOpen(mat1, HR_WRITE_DISCARD);
    {
      xml_node matNode = hrMaterialParamNode(mat1);
      xml_node diff = matNode.append_child(L"diffuse");

      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").append_attribute(L"val").set_value(L"0.207843 0.188235 0");

      xml_node refl = matNode.append_child(L"reflectivity");

      refl.append_attribute(L"brdf_type").set_value(L"phong");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"0.367059 0.345882 0");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.5");
      refl.append_child(L"energy_fix").append_attribute(L"val") = 1;
    }
    hrMaterialClose(mat1);

    hrMaterialOpen(mat2, HR_WRITE_DISCARD);
    {
      xml_node matNode = hrMaterialParamNode(mat2);

      xml_node diff = matNode.append_child(L"diffuse");

      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").text().set(L"0.75 0.75 0.75");

      HRTextureNodeRef testTex = hrTexture2DCreateFromFile(L"data/textures/relief_wood.jpg");
      hrTextureBind(testTex, diff);
    }
    hrMaterialClose(mat2);

    hrMaterialOpen(mat3, HR_WRITE_DISCARD);
    {
      xml_node matNode = hrMaterialParamNode(mat3);

      xml_node diff = matNode.append_child(L"diffuse");

      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").text().set(L"0.75 0.75 0.75");

      HRTextureNodeRef testTex = hrTexture2DCreateFromFileDL(L"data/textures/163.jpg");
      hrTextureBind(testTex, diff);
    }
    hrMaterialClose(mat3);

    hrMaterialOpen(mat4, HR_WRITE_DISCARD);
    {
      xml_node matNode = hrMaterialParamNode(mat4);

      xml_node diff = matNode.append_child(L"diffuse");

      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").text().set(L"0.1 0.1 0.75");
    }
    hrMaterialClose(mat4);

    hrMaterialOpen(mat5, HR_WRITE_DISCARD);
    {
      xml_node matNode = hrMaterialParamNode(mat5);

      xml_node diff = matNode.append_child(L"diffuse");

      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").text().set(L"0.75 0.75 0.25");

      HRTextureNodeRef testTex = hrTexture2DCreateFromFileDL(L"data/textures/texture1.bmp");
      hrTextureBind(testTex, diff);
    }
    hrMaterialClose(mat5);


    hrMaterialOpen(mat6, HR_WRITE_DISCARD);
    {
      xml_node matNode = hrMaterialParamNode(mat6);
      xml_node diff = matNode.append_child(L"diffuse");

      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").text().set(L"0.5 0.0 0.0");
    }
    hrMaterialClose(mat6);

    hrMaterialOpen(mat7, HR_WRITE_DISCARD);
    {
      xml_node matNode = hrMaterialParamNode(mat7);
      xml_node diff = matNode.append_child(L"diffuse");

      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").text().set(L"0.0 0.5 0.0");
    }
    hrMaterialClose(mat7);

    hrMaterialOpen(mat8, HR_WRITE_DISCARD);
    {
      xml_node matNode = hrMaterialParamNode(mat8);
      xml_node diff = matNode.append_child(L"diffuse");

      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").text().set(L"0.5 0.5 0.5");
    }
    hrMaterialClose(mat8);

    hrMaterialOpen(mat9, HR_WRITE_DISCARD);
    {
      xml_node matNode = hrMaterialParamNode(mat9);
      xml_node diff = matNode.append_child(L"diffuse");

      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").append_attribute(L"val").set_value(L"0.0 0.0 0");

      xml_node refl = matNode.append_child(L"reflectivity");

      refl.append_attribute(L"brdf_type").set_value(L"phong");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"0.4 0.4 0.4");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.85");
      refl.append_child(L"energy_fix").append_attribute(L"val") = 1;
    }
    hrMaterialClose(mat9);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRMeshRef teapotRef = hrMeshCreateFromFileDL(L"data/meshes/teapot.vsgf"); // chunk_00009.vsgf // teapot.vsgf // chunk_00591.vsgf

    HRMeshRef cubeOpenRef = hrMeshCreate(L"my_box");
    HRMeshRef planeRef = hrMeshCreate(L"my_plane");
    HRMeshRef sphereRef = hrMeshCreate(L"my_sphere");
    HRMeshRef torusRef = hrMeshCreate(L"my_torus");

    hrMeshOpen(cubeOpenRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
    {
      hrMeshVertexAttribPointer4f(cubeOpenRef, L"pos", &cubeOpen.vPos[0]);
      hrMeshVertexAttribPointer4f(cubeOpenRef, L"norm", &cubeOpen.vNorm[0]);
      hrMeshVertexAttribPointer2f(cubeOpenRef, L"texcoord", &cubeOpen.vTexCoord[0]);

      int cubeMatIndices[10] = { mat8.id, mat8.id, mat8.id, mat9.id, mat9.id, mat9.id, mat7.id, mat7.id, mat6.id, mat6.id };

      //hrMeshMaterialId(cubeRef, 0);
      hrMeshPrimitiveAttribPointer1i(cubeOpenRef, L"mind", cubeMatIndices);
      hrMeshAppendTriangles3(cubeOpenRef, int(cubeOpen.triIndices.size()), &cubeOpen.triIndices[0]);
    }
    hrMeshClose(cubeOpenRef);


    hrMeshOpen(planeRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
    {
      hrMeshVertexAttribPointer4f(planeRef, L"pos", &plane.vPos[0]);
      hrMeshVertexAttribPointer4f(planeRef, L"norm", &plane.vNorm[0]);
      hrMeshVertexAttribPointer2f(planeRef, L"texcoord", &plane.vTexCoord[0]);

      hrMeshMaterialId(planeRef, mat5.id);
      hrMeshAppendTriangles3(planeRef, int32_t(plane.triIndices.size()), &plane.triIndices[0]);
    }
    hrMeshClose(planeRef);

    hrMeshOpen(sphereRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
    {
      hrMeshVertexAttribPointer4f(sphereRef, L"pos", &sphere.vPos[0]);
      hrMeshVertexAttribPointer4f(sphereRef, L"norm", &sphere.vNorm[0]);
      hrMeshVertexAttribPointer2f(sphereRef, L"texcoord", &sphere.vTexCoord[0]);

      for (size_t i = 0; i < sphere.matIndices.size() / 2; i++)
        sphere.matIndices[i] = mat0.id;

      for (size_t i = sphere.matIndices.size() / 2; i < sphere.matIndices.size(); i++)
        sphere.matIndices[i] = mat2.id;

      hrMeshPrimitiveAttribPointer1i(sphereRef, L"mind", &sphere.matIndices[0]);
      hrMeshAppendTriangles3(sphereRef, int32_t(sphere.triIndices.size()), &sphere.triIndices[0]);
    }
    hrMeshClose(sphereRef);

    hrMeshOpen(torusRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
    {
      hrMeshVertexAttribPointer4f(torusRef, L"pos", &torus.vPos[0]);
      hrMeshVertexAttribPointer4f(torusRef, L"norm", &torus.vNorm[0]);
      hrMeshVertexAttribPointer2f(torusRef, L"texcoord", &torus.vTexCoord[0]);

      for (size_t i = 0; i < torus.matIndices.size() / 3; i++)
        torus.matIndices[i] = mat0.id;

      for (size_t i = 1 * torus.matIndices.size() / 3; i < 2 * torus.matIndices.size() / 3; i++)
        torus.matIndices[i] = mat3.id;

      for (size_t i = 2 * torus.matIndices.size() / 3; i < torus.matIndices.size(); i++)
        torus.matIndices[i] = mat2.id;

      //hrMeshMaterialId(torusRef, mat0.id);
      hrMeshPrimitiveAttribPointer1i(torusRef, L"mind", &torus.matIndices[0]);
      hrMeshAppendTriangles3(torusRef, int32_t(torus.triIndices.size()), &torus.triIndices[0]);
    }
    hrMeshClose(torusRef);

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

      sizeNode.append_attribute(L"half_length").set_value(L"0.25");
      sizeNode.append_attribute(L"half_width").set_value(L"0.5");

      pugi::xml_node intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(50.0f*IRRADIANCE_TO_RADIANCE);
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

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    hrRenderOpen(renderRef, HR_WRITE_DISCARD);
    {
      pugi::xml_node node = hrRenderParamNode(renderRef);

      node.append_child(L"width").text()  = TEST_SCREEN_SIZE;
      node.append_child(L"height").text() = TEST_SCREEN_SIZE;

      node.append_child(L"method_primary").text()   = L"IBPT";
      node.append_child(L"method_secondary").text() = L"IBPT";
      node.append_child(L"method_tertiary").text()  = L"IBPT";
      node.append_child(L"method_caustic").text()   = L"IBPT";
      node.append_child(L"shadows").text() = L"1";

      node.append_child(L"trace_depth").text()      = L"6";
      node.append_child(L"diff_trace_depth").text() = L"3";
      node.append_child(L"maxRaysPerPixel").text()  = 2048;
    }
    hrRenderClose(renderRef);

    // create scene
    //
    HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

    static GLfloat	rtri = 25.0f; // Angle For The Triangle ( NEW )
    static GLfloat	rquad = 40.0f;
    static float    g_FPS = 60.0f;
    static int      frameCounter = 0;

    const float DEG_TO_RAD = float(3.14159265358979323846f) / 180.0f;

    float matrixT[4][4];
    float mRot1[4][4], mRot2[4][4], mTranslate[4][4], mRes[4][4];
    //float mTranslateDown[4][4];

    hrSceneOpen(scnRef, HR_WRITE_DISCARD);

    int mmIndex = 0;
    mat4x4_identity(mRot1);
    mat4x4_identity(mTranslate);
    mat4x4_identity(mRes);

    mat4x4_translate(mTranslate, 0.0f, -0.70f*3.65f, 0.0f);
    mat4x4_scale(mRot1, mRot1, 3.65f);
    mat4x4_mul(mRes, mTranslate, mRot1);
    mat4x4_transpose(matrixT, mRes); // this fucking math library swap rows and columns
    matrixT[3][3] = 1.0f;
    hrMeshInstance(scnRef, teapotRef, &matrixT[0][0]);

    mat4x4_identity(mRot1);
    mat4x4_rotate_Y(mRot1, mRot1, 180.0f*DEG_TO_RAD);
    //mat4x4_rotate_Y(mRot1, mRot1, rquad*DEG_TO_RAD);
    mat4x4_transpose(matrixT, mRot1);
    hrMeshInstance(scnRef, cubeOpenRef, &matrixT[0][0]);

    /////////////////////////////////////////////////////////////////////// instance light (!!!)

    mat4x4_identity(mTranslate);
    mat4x4_identity(mRot1);
    mat4x4_identity(mRot2);

    mat4x4_translate(mTranslate, -3.25f, 3.25f, -2.0);
    mat4x4_rotate_Y(mRot1, mRot1, -20.0f*DEG_TO_RAD);
    mat4x4_rotate_X(mRot2, mRot2, 30.0f*DEG_TO_RAD);
    mat4x4_mul(mRot1, mRot2, mRot1);
    mat4x4_mul(mRes, mTranslate, mRot1);
    mat4x4_transpose(matrixT, mRes);
    hrLightInstance(scnRef, rectLight, &matrixT[0][0]);


    mat4x4_identity(mTranslate);
    mat4x4_translate(mTranslate, 3.75f, 3.95f, -3.5f);
    mat4x4_transpose(matrixT, mTranslate);
    hrLightInstance(scnRef, rectLight, &matrixT[0][0]);

    hrSceneClose(scnRef);

    hrFlush(scnRef, renderRef);

    ////////////////////
    // Rendering and save
    ////////////////////

    RenderProgress(renderRef);

    std::filesystem::create_directories(saveRenderFile.parent_path());
    hrRenderSaveFrameBufferLDR(renderRef, saveRenderFile.c_str());

  
    hrRenderOpen(renderRef, HR_OPEN_EXISTING);
    {
      pugi::xml_node node = hrRenderParamNode(renderRef);
      
      node.force_child(L"method_primary").text()   = L"pathtracing";
      node.force_child(L"method_secondary").text() = L"mmlt";
  
      node.force_child(L"mmlt_burn_iters").text()     = 1024;   // [1024, 2048, 4096, 8192, 16384, 65536]
      node.force_child(L"mmlt_sds_fixed_prob").text() = 0;      // [0, 0.25, 0.5, 0,75, 0 85, 0.9, 0.95 ]
      node.force_child(L"mmlt_threads").text()        = 262144; // [524288, 262144, 131072(?), 65536, 16384]
  
      node.force_child(L"maxRaysPerPixel").text()     = 8192;
    }
    hrRenderClose(renderRef);
  
    hrFlush(scnRef, renderRef);
  
    ////////////////////
    // Rendering, save and check image
    ////////////////////

    RenderProgress(renderRef);

    std::filesystem::create_directories(saveRenderFile.parent_path());
    hrRenderSaveFrameBufferLDR(renderRef, saveRenderFile2.c_str());

    return check_images(ws2s(nameTest).c_str(), 2, 40);
  }


  bool test_225_point_spot_simple()
  {
    std::wstring nameTest                = L"test_225";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.c_str(), HR_WRITE_DISCARD);
    
    ////////////////////
    // Materials
    ////////////////////

    HRMaterialRef matGray = hrMaterialCreate(L"matGray");

    hrMaterialOpen(matGray, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGray);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.5 0.5 0.5");

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matGray);


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Meshes
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    HRMeshRef sph1     = HRMeshFromSimpleMesh(L"sph1",    CreateSphere(2.0f, 64), matGray.id);
    HRMeshRef sph2     = HRMeshFromSimpleMesh(L"sph2",    CreateSphere(2.0f, 64), matGray.id);
    HRMeshRef cubeOpen = HRMeshFromSimpleMesh(L"my_cube", CreateCubeOpen(6.0f),   matGray.id);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Light
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRLightRef spot1 = hrLightCreate(L"spot1");

    hrLightOpen(spot1, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(spot1);

      lightNode.attribute(L"type").set_value(L"point");
      lightNode.attribute(L"shape").set_value(L"point");
      lightNode.attribute(L"distribution").set_value(L"spot");

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
			intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(50.0f*IRRADIANCE_TO_RADIANCE);

      lightNode.append_child(L"falloff_angle").append_attribute(L"val").set_value(100);
      lightNode.append_child(L"falloff_angle2").append_attribute(L"val").set_value(60);

			VERIFY_XML(lightNode);
    }
    hrLightClose(spot1);

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
      camNode.append_child(L"position").text().set(L"0 3 18");
      camNode.append_child(L"look_at").text().set(L"0 3 0");

			VERIFY_XML(camNode);
    }
    hrCameraClose(camRef);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Render settings
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, TEST_SCREEN_SIZE, TEST_SCREEN_SIZE, 256, 2048);


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Create scene
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

    using namespace LiteMath;

    float4x4 mRot;
    float4x4 mTranslate;
    float4x4 mScale;
    float4x4 mRes;

    const float DEG_TO_RAD = 0.01745329251f; // float(3.14159265358979323846f) / 180.0f;

    hrSceneOpen(scnRef, HR_WRITE_DISCARD);
    ///////////

    mTranslate.identity();
    mRes.identity();
    mRot.identity();


    mTranslate = translate4x4(float3(0.0f, 3.0f, 0.0f));
    mRot = rotate4x4Y(180.0f*DEG_TO_RAD);
    mRes = mul(mTranslate, mRot);

    hrMeshInstance(scnRef, cubeOpen, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(-3.0f, 1.25f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph1, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(3.0f, 1.25f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph2, mRes.L());


    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0.0f, 8.95f, -5.0f));
    mRes = mul(mTranslate, mRes);

    hrLightInstance(scnRef, spot1, mRes.L());

    ///////////

    hrSceneClose(scnRef);
    hrFlush(scnRef, renderRef);

    ////////////////////
    // Rendering, save and check image
    ////////////////////

    RenderProgress(renderRef);

    std::filesystem::create_directories(saveRenderFile.parent_path());
    hrRenderSaveFrameBufferLDR(renderRef, saveRenderFile.c_str());

    return check_images(ws2s(nameTest).c_str(), 1, 30);
  }
};