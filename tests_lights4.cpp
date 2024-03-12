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
  using LiteMath::float4;
  using LiteMath::float3;
  using LiteMath::float2;

  const int TEST_IMG_SIZE = 512;

  bool test_237_cubemap_ldr()
  {
    std::wstring nameTest                = L"test_237";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);
    
    ////////////////////
    // Materials
    ////////////////////

    HRMaterialRef matGray = hrMaterialCreate(L"matGray");
    HRMaterialRef matRefl = hrMaterialCreate(L"matRefl");


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //HRTextureNodeRef texEnv = hrTexture2DCreateFromFile(L"data/textures/LA_Downtown_Afternoon_Fishing_B_8k.jpg");
    
    HRTextureNodeRef cube[6] = {
      hrTexture2DCreateFromFile(L"data/textures/yokohama3/posx.jpg"),
      hrTexture2DCreateFromFile(L"data/textures/yokohama3/negx.jpg"),
      hrTexture2DCreateFromFile(L"data/textures/yokohama3/posy.jpg"),
      hrTexture2DCreateFromFile(L"data/textures/yokohama3/negy.jpg"),
      hrTexture2DCreateFromFile(L"data/textures/yokohama3/posz.jpg"),
      hrTexture2DCreateFromFile(L"data/textures/yokohama3/negz.jpg"),
    };

    HRTextureNodeRef texEnv = HRUtils::Cube2SphereLDR(cube);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"1.0");
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_IOR").append_attribute(L"val").set_value(8.0f);
    }
    hrMaterialClose(matRefl);


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Meshes
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    HRMeshRef cubeR    = HRMeshFromSimpleMesh(L"cubeR", CreateCube(2.0f), matGray.id);
    HRMeshRef sphereG  = HRMeshFromSimpleMesh(L"sphereG", CreateSphere(4.0f, 64), matRefl.id);
    HRMeshRef torusB   = HRMeshFromSimpleMesh(L"torusB", CreateTorus(0.8f, 2.0f, 64, 64), matGray.id);
    HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), matGray.id);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Light
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRLightRef sky = hrLightCreate(L"sky");

    hrLightOpen(sky, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(sky);

      lightNode.attribute(L"type").set_value(L"sky");
      lightNode.attribute(L"distribution").set_value(L"map");

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"2.0");

      auto texNode = hrTextureBind(texEnv, intensityNode.child(L"color"));

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 1, 0, 0, 0,
                                  0, 1, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };

      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);

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

    HRRenderRef renderRef = CreateBasicTestRenderPTNoCaust(CURR_RENDER_DEVICE, 1024, 768, 256, 2048);


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
    mRot       = rotate4x4Y(-60.0f*DEG_TO_RAD);
    mRot2      = rotate4x4X(90.0f*DEG_TO_RAD);
    mRes       = mul(mRot, mRot2);
    mRes       = mul(mTranslate, mRes);

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

    ////////////////////
    // Rendering, save and check image
    ////////////////////

    RenderProgress(renderRef);

    std::filesystem::create_directories(saveRenderFile.parent_path());
    hrRenderSaveFrameBufferLDR(renderRef, saveRenderFile.wstring().c_str());

    return check_images(ws2s(nameTest).c_str(), 1, 115);
  }


  bool test_238_mesh_light_one_triangle()
  {
    std::wstring nameTest                = L"test_238";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);
    
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

    // create mesh that we will use for light
    //
    HRMeshRef lightMesh = hrMeshCreate(L"single_triangle_mesh");
    hrMeshOpen(lightMesh, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
    {
      const float sq3 = sqrtf(3);
      float3 pos [3]  = { float3(0,0,2*sq3), float3(-2,0,0), float3(2,0,0)  };
      float3 norm[3]  = { float3(0,-1,0),    float3(0,-1,0), float3(0,-1,0) };
      float2 texC[3]  = { float2(1,1),       float2(0,0),    float2(1,0)    };
      int    ind [3]  = { 0, 1, 2 };

      hrMeshVertexAttribPointer3f(lightMesh, L"pos",      &pos[0].x);
      hrMeshVertexAttribPointer3f(lightMesh, L"norm",     &norm[0].x);
      hrMeshVertexAttribPointer2f(lightMesh, L"texcoord", &texC[0].x);
      
      hrMeshMaterialId(lightMesh, 0); // does not matter actually for light meshes.
      hrMeshAppendTriangles3(lightMesh, 3, ind);
    }
    hrMeshClose(lightMesh);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Light
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRLightRef spot1 = hrLightCreate(L"spot1");

    hrLightOpen(spot1, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(spot1);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"disk");
      lightNode.attribute(L"distribution").set_value(L"spot");

      lightNode.append_child(L"size").append_attribute(L"radius").set_value(1.0f);

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"0.5 1 0.5");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(8.0*IRRADIANCE_TO_RADIANCE);

      lightNode.append_child(L"falloff_angle").append_attribute(L"val").set_value(90);
      lightNode.append_child(L"falloff_angle2").append_attribute(L"val").set_value(60);

			VERIFY_XML(lightNode);
    }
    hrLightClose(spot1);

    HRLightRef spot2 = hrLightCreate(L"spot2");

    hrLightOpen(spot2, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(spot2);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"mesh");
      lightNode.attribute(L"distribution").set_value(L"diffuse");
      lightNode.append_child(L"mesh").append_attribute(L"id") = lightMesh.id;

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
			intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(8.0f*IRRADIANCE_TO_RADIANCE);
			
			VERIFY_XML(lightNode);
    }
    hrLightClose(spot2);


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

      // camNode.append_child(L"up").text().set(L"0 1 0");
      // camNode.append_child(L"position").text().set(L"3 1 12");
      // camNode.append_child(L"look_at").text().set(L"-1 3 0");

      camNode.append_child(L"up").text().set(L"0 1 0");
      camNode.append_child(L"position").text().set(L"0 3 18");
      camNode.append_child(L"look_at").text().set(L"0 3 0");

			VERIFY_XML(camNode);
    }
    hrCameraClose(camRef);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Render settings
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, TEST_IMG_SIZE, TEST_IMG_SIZE, 256, 2048);


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
    mRot       = rotate4x4Y(180.0f*DEG_TO_RAD);
    mRes       = mul(mTranslate, mRot);

    hrMeshInstance(scnRef, cubeOpen, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(-3.0f, 1.25f, 0.0f));
    mRes       = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph1, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(3.0f, 1.25f, 0.0f));
    mRes       = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph2, mRes.L());

    // mTranslate.identity();
    // mTranslate = translate4x4(float3(-1.0f, 4.0f, -1.0f));
    // hrMeshInstance(scnRef, lightMesh, mTranslate.L());

    mTranslate.identity();
    mRes.identity();
    mTranslate = translate4x4(float3(-1.0f, 4.0f, -1.0f));
    mRes       = mul(mTranslate, mRes);

    hrLightInstance(scnRef, spot2, mRes.L());
    
    ///////////

    hrSceneClose(scnRef);

    hrFlush(scnRef, renderRef);

    ////////////////////
    // Rendering, save and check image
    ////////////////////

    RenderProgress(renderRef);

    std::filesystem::create_directories(saveRenderFile.parent_path());
    hrRenderSaveFrameBufferLDR(renderRef, saveRenderFile.wstring().c_str());

    return check_images(ws2s(nameTest).c_str(), 1, 40);
  }


  bool test_239_mesh_light_two_triangle()
  {
    std::wstring nameTest                = L"test_239";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);
    
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

    // create mesh that we will use for light
    //
    HRMeshRef lightMesh = hrMeshCreate(L"triangle_mesh");
    hrMeshOpen(lightMesh, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
    {
      float3 pos [6] = { float3(-1,0,-1), float3(+1,0,-1), float3(+1,0,+1), float3(+1,0,+1), float3(-1,0,+1), float3(-1,0,-1) };
      float3 norm[6] = { float3(0,-1,0),  float3(0,-1,0),  float3(0,-1,0),  float3(0,-1,0),  float3(0,-1,0),  float3(0,-1,0) };
      float2 texC[6] = { float2(0,0),     float2(1,0),     float2(1,1),     float2(1,1),     float2(0,1),     float2(0,0) };
      int    ind [6] = { 0, 1, 2, 3, 4, 5 };

      hrMeshVertexAttribPointer3f(lightMesh, L"pos",      &pos[0].x);
      hrMeshVertexAttribPointer3f(lightMesh, L"norm",     &norm[0].x);
      hrMeshVertexAttribPointer2f(lightMesh, L"texcoord", &texC[0].x);

      hrMeshMaterialId(lightMesh, 0); // does not matter actually for light meshes.
      hrMeshAppendTriangles3(lightMesh, 6, ind);
    }
    hrMeshClose(lightMesh);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Light
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRLightRef areaLightRef = hrLightCreate(L"myarea");

    hrLightOpen(areaLightRef, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(areaLightRef);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"rect");
      lightNode.attribute(L"distribution").set_value(L"diffuse");

      auto sizeNode = lightNode.append_child(L"size");
      sizeNode.append_attribute(L"half_length").set_value(1.0f);
      sizeNode.append_attribute(L"half_width").set_value(1.0f);

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(2.0f*IRRADIANCE_TO_RADIANCE);

      VERIFY_XML(lightNode);
    }
    hrLightClose(areaLightRef);

    HRLightRef meshLightRef = hrLightCreate(L"mymesh");

    hrLightOpen(meshLightRef, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(meshLightRef);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"mesh");
      lightNode.attribute(L"distribution").set_value(L"diffuse");
      lightNode.append_child(L"mesh").append_attribute(L"id") = lightMesh.id;

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(2.0f*IRRADIANCE_TO_RADIANCE);

      VERIFY_XML(lightNode);
    }
    hrLightClose(meshLightRef);


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

    HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 512, 512, 256, 2048);


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
    mRot       = rotate4x4Y(180.0f*DEG_TO_RAD);
    mRes       = mul(mTranslate, mRot);

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

    // mTranslate.identity();
    // mTranslate = translate4x4(float3(-1.0f, 4.0f, -1.0f));
    // hrMeshInstance(scnRef, lightMesh, mTranslate.L());

    mTranslate.identity();
    mRes.identity();
    mTranslate = translate4x4(float3(-3.0f, 8.0f, -5.0f));
    mRes = mul(mTranslate, mRes);
    hrLightInstance(scnRef, meshLightRef, mRes.L());

    mTranslate.identity();
    mRes.identity();
    mTranslate = translate4x4(float3(+3.0f, 8.0f, -5.0f));
    mRes = mul(mTranslate, mRes);
    hrLightInstance(scnRef, areaLightRef, mRes.L());

    ///////////

    hrSceneClose(scnRef);

    hrFlush(scnRef, renderRef);

    ////////////////////
    // Rendering, save and check image
    ////////////////////

    RenderProgress(renderRef);

    std::filesystem::create_directories(saveRenderFile.parent_path());
    hrRenderSaveFrameBufferLDR(renderRef, saveRenderFile.wstring().c_str());

    return check_images(ws2s(nameTest).c_str(), 1, 40);
  }


  bool test_240_mesh_light_torus()
  {
    std::wstring nameTest                = L"test_240";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);
    
    ////////////////////
    // Materials
    ////////////////////

    SimpleMesh cube     = CreateCube(0.75f);
    SimpleMesh plane    = CreatePlane(10.0f);
    SimpleMesh sphere   = CreateSphere(1.0f, 32);
    SimpleMesh torus    = CreateTorus(0.05f, 0.6f, 32, 32);
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

    hrMaterialOpen(mat0, HR_WRITE_DISCARD);
    {
      xml_node matNode = hrMaterialParamNode(mat0);

      xml_node diff = matNode.append_child(L"diffuse");

      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").text().set(L"0.5 0.75 0.5");

      HRTextureNodeRef testTex = hrTexture2DCreateFromFile(L"data/textures/texture1.bmp"); // hrTexture2DCreateFromFileDL
      hrTextureBind(testTex, diff);
    }
    hrMaterialClose(mat0);

    hrMaterialOpen(mat1, HR_WRITE_DISCARD);
    {
      xml_node matNode = hrMaterialParamNode(mat1);

      xml_node diff = matNode.append_child(L"diffuse");

      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").text().set(L"0.207843 0.188235 0");

      // hrTextureBind(testTex2, diff);

      xml_node refl = matNode.append_child(L"reflectivity");

      refl.append_attribute(L"brdf_type").set_value(L"phong");
      refl.append_child(L"color").text().set(L"0.367059 0.345882 0");
      refl.append_child(L"glossiness").text().set(L"0.5");
      refl.append_child(L"energy_fix").append_attribute(L"val") = 1;
      //refl.append_child(L"fresnel_IOR").text().set(L"1.5");
      //refl.append_child(L"fresnel").text().set(L"1");

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

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRMeshRef teapotRef   = hrMeshCreateFromFileDL(L"data/meshes/teapot.vsgf"); // chunk_00009.vsgf // teapot.vsgf // chunk_00591.vsgf

    HRMeshRef cubeOpenRef = hrMeshCreate(L"my_box");
    HRMeshRef planeRef    = hrMeshCreate(L"my_plane");
    HRMeshRef torusRef    = hrMeshCreate(L"my_torus");

    hrMeshOpen(cubeOpenRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
    {
      hrMeshVertexAttribPointer4f(cubeOpenRef, L"pos", &cubeOpen.vPos[0]);
      hrMeshVertexAttribPointer4f(cubeOpenRef, L"norm", &cubeOpen.vNorm[0]);
      hrMeshVertexAttribPointer2f(cubeOpenRef, L"texcoord", &cubeOpen.vTexCoord[0]);

      int cubeMatIndices[10] = { mat8.id, mat8.id, mat8.id, mat8.id, mat8.id, mat8.id, mat7.id, mat7.id, mat6.id, mat6.id };

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

    hrMeshOpen(torusRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
    {
      hrMeshVertexAttribPointer4f(torusRef, L"pos",      &torus.vPos[0]);
      hrMeshVertexAttribPointer4f(torusRef, L"norm",     &torus.vNorm[0]);
      hrMeshVertexAttribPointer2f(torusRef, L"texcoord", &torus.vTexCoord[0]);

      hrMeshMaterialId(torusRef, mat0.id);
      hrMeshAppendTriangles3(torusRef, int32_t(torus.triIndices.size()), &torus.triIndices[0]);
    }
    hrMeshClose(torusRef);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRLightRef meshLightRef = hrLightCreate(L"my_area_light");

    hrLightOpen(meshLightRef, HR_WRITE_DISCARD);
    {
      pugi::xml_node lightNode = hrLightParamNode(meshLightRef);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"mesh");
      lightNode.attribute(L"distribution").set_value(L"diffuse");
      lightNode.append_child(L"mesh").append_attribute(L"id") = torusRef.id;

      pugi::xml_node intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val")      = L"1 1 1";
      intensityNode.append_child(L"multiplier").append_attribute(L"val") = 2.0f*IRRADIANCE_TO_RADIANCE;
    }
    hrLightClose(meshLightRef);

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

    //hrRenderEnableDevice(renderRef, 0, true);
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
      node.append_child(L"shadows").text() = L"1";

      node.append_child(L"trace_depth").text()      = L"5";
      node.append_child(L"diff_trace_depth").text() = L"3";

      node.append_child(L"pt_error").text() = L"2";
      node.append_child(L"minRaysPerPixel").text() = L"256";
      node.append_child(L"maxRaysPerPixel").text() = L"1024";

      node.append_child(L"draw_tiles").text() = L"0";
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
    float mRot1[4][4], mTranslate[4][4], mRes[4][4];
    float mScale[4][4];

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

    /////////////////////////////////////////////////////////////////////// instance light

    mat4x4_identity(mTranslate);
    mat4x4_identity(mRot1);
    mat4x4_identity(mScale);

    mat4x4_translate(mTranslate, 0, -2.0f, 0);
    mat4x4_rotate_X (mRot1, mRot1, -75.0f*DEG_TO_RAD);
    mat4x4_scale    (mScale, mScale, 4.0f);

    mat4x4_mul(mRot1, mRot1, mScale);
    mat4x4_mul(mRes, mTranslate, mRot1);
    mat4x4_transpose(matrixT, mRes);
    matrixT[3][3] = 1.0f;
    hrLightInstance(scnRef, meshLightRef, &matrixT[0][0]);

    hrSceneClose(scnRef);

    hrFlush(scnRef, renderRef);
    
    ////////////////////
    // Rendering, save and check image
    ////////////////////

    RenderProgress(renderRef);

    std::filesystem::create_directories(saveRenderFile.parent_path());
    hrRenderSaveFrameBufferLDR(renderRef, saveRenderFile.wstring().c_str());

    return check_images(ws2s(nameTest).c_str(), 1, 40);
  }


  bool test_241_mesh_light_torus_texture_ldr()
  {
    std::wstring nameTest                = L"test_241";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);
    
    ////////////////////
    // Materials
    ////////////////////

    SimpleMesh cube     = CreateCube(0.75f);
    SimpleMesh plane    = CreatePlane(10.0f);
    SimpleMesh sphere   = CreateSphere(1.0f, 32);
    SimpleMesh torus    = CreateTorus(0.025f, 0.25f, 32, 32);
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

    hrMaterialOpen(mat0, HR_WRITE_DISCARD);
    {
      xml_node matNode = hrMaterialParamNode(mat0);

      xml_node diff = matNode.append_child(L"diffuse");

      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").text().set(L"0.5 0.75 0.5");

      HRTextureNodeRef testTex = hrTexture2DCreateFromFile(L"data/textures/texture1.bmp"); // hrTexture2DCreateFromFileDL
      hrTextureBind(testTex, diff);
    }
    hrMaterialClose(mat0);

    hrMaterialOpen(mat1, HR_WRITE_DISCARD);
    {
      xml_node matNode = hrMaterialParamNode(mat1);

      xml_node diff = matNode.append_child(L"diffuse");

      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").text().set(L"0.207843 0.188235 0");

      // hrTextureBind(testTex2, diff);

      xml_node refl = matNode.append_child(L"reflectivity");

      refl.append_attribute(L"brdf_type").set_value(L"phong");
      refl.append_child(L"color").text().set(L"0.367059 0.345882 0");
      refl.append_child(L"glossiness").text().set(L"0.5");
      refl.append_child(L"energy_fix").append_attribute(L"val") = 1;
      //refl.append_child(L"fresnel_IOR").text().set(L"1.5");
      //refl.append_child(L"fresnel").text().set(L"1");

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

      auto refl = matNode.append_child(L"reflectivity");
      refl.append_attribute(L"brdf_type").set_value(L"phong");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"0.75 0.75 0.75");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"1.0");
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(0);
      refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(1.5);
      refl.append_child(L"energy_fix").append_attribute(L"val") = 1;
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

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRMeshRef teapotRef   = hrMeshCreateFromFileDL(L"data/meshes/teapot.vsgf"); // chunk_00009.vsgf // teapot.vsgf // chunk_00591.vsgf

    HRMeshRef cubeOpenRef = hrMeshCreate(L"my_box");
    HRMeshRef planeRef    = hrMeshCreate(L"my_plane");
    HRMeshRef torusRef    = hrMeshCreate(L"my_torus");

    hrMeshOpen(cubeOpenRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
    {
      hrMeshVertexAttribPointer4f(cubeOpenRef, L"pos", &cubeOpen.vPos[0]);
      hrMeshVertexAttribPointer4f(cubeOpenRef, L"norm", &cubeOpen.vNorm[0]);
      hrMeshVertexAttribPointer2f(cubeOpenRef, L"texcoord", &cubeOpen.vTexCoord[0]);

      int cubeMatIndices[10] = { mat8.id, mat8.id, mat8.id, mat8.id, mat8.id, mat8.id, mat7.id, mat7.id, mat6.id, mat6.id };

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

    hrMeshOpen(torusRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
    {
      hrMeshVertexAttribPointer4f(torusRef, L"pos",      &torus.vPos[0]);
      hrMeshVertexAttribPointer4f(torusRef, L"norm",     &torus.vNorm[0]);
      hrMeshVertexAttribPointer2f(torusRef, L"texcoord", &torus.vTexCoord[0]);

      hrMeshMaterialId(torusRef, mat0.id);
      hrMeshAppendTriangles3(torusRef, int32_t(torus.triIndices.size()), &torus.triIndices[0]);
    }
    hrMeshClose(torusRef);

    ////////////////////
    // Light
    ////////////////////

    HRTextureNodeRef texForLight = hrTexture2DCreateFromFile(L"data/textures/chess_red_green_blue.bmp");

    HRLightRef meshLightRef = hrLightCreate(L"my_area_light");

    hrLightOpen(meshLightRef, HR_WRITE_DISCARD);
    {
      pugi::xml_node lightNode = hrLightParamNode(meshLightRef);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"mesh");
      lightNode.attribute(L"distribution").set_value(L"diffuse");
      lightNode.append_child(L"mesh").append_attribute(L"id") = torusRef.id;

      pugi::xml_node intensityNode = lightNode.append_child(L"intensity");

      auto colorNode = intensityNode.append_child(L"color");
      intensityNode.append_child(L"multiplier").append_attribute(L"val") = 8.0f*IRRADIANCE_TO_RADIANCE;

      colorNode.append_attribute(L"val") = L"1 1 1";

      auto texNode = hrTextureBind(texForLight, colorNode);
      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 1, 0, 0, -0.5,
                                  0, 4, 0, -0.5,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };
      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);
    }
    hrLightClose(meshLightRef);

    ////////////////////
    // Camera
    ////////////////////

    CreateCamera(45, L"0 0 15", L"0 0 0");

    ////////////////////
    // Render settings
    ////////////////////

    auto renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 512, 512, 256, 4096);



    // create scene
    //
    HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

    static GLfloat	rtri = 25.0f; // Angle For The Triangle ( NEW )
    static GLfloat	rquad = 40.0f;
    static float    g_FPS = 60.0f;
    static int      frameCounter = 0;


    float matrixT[4][4];
    float mRot1[4][4], mTranslate[4][4], mRes[4][4];
    float mScale[4][4];

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
    mat4x4_transpose(matrixT, mRot1);
    matrixT[3][3] = 1.0f;
    hrMeshInstance(scnRef, cubeOpenRef, &matrixT[0][0]);

    // mirror plane
    //
    mat4x4_identity(mTranslate);
    mat4x4_identity(mRot1);
    mat4x4_identity(mScale);

    mat4x4_rotate_Z(mRot1, mRot1, -90.0f*DEG_TO_RAD);
    mat4x4_scale(mScale, mScale, 0.25f);
    mat4x4_translate(mTranslate, -3.85f, 0.0f, 0);
    
    mat4x4_mul(mRot1, mRot1, mScale);
    mat4x4_mul(mRes, mTranslate, mRot1);
    mat4x4_transpose(matrixT, mRes);
    matrixT[3][3] = 1.0f;
    hrMeshInstance(scnRef, planeRef, &matrixT[0][0]);

    /////////////////////////////////////////////////////////////////////// instance light

    mat4x4_identity(mTranslate);
    mat4x4_identity(mRot1);
    mat4x4_identity(mScale);

    mat4x4_translate(mTranslate, 0, -0.75f, 0);
    mat4x4_rotate_X (mRot1, mRot1, -85.0f*DEG_TO_RAD);
    mat4x4_scale    (mScale, mScale, 5.0f);

    mat4x4_mul(mRot1, mRot1, mScale);
    mat4x4_mul(mRes, mTranslate, mRot1);
    mat4x4_transpose(matrixT, mRes);
    matrixT[3][3] = 1.0f;
    hrLightInstance(scnRef, meshLightRef, &matrixT[0][0]);

    hrSceneClose(scnRef);

    hrFlush(scnRef, renderRef);

    ////////////////////
    // Rendering, save and check image
    ////////////////////

    RenderProgress(renderRef);

    std::filesystem::create_directories(saveRenderFile.parent_path());
    hrRenderSaveFrameBufferLDR(renderRef, saveRenderFile.wstring().c_str());

    return check_images(ws2s(nameTest).c_str(), 1, 30);
  }


  bool test_242_mesh_light_torus_texture_hdr()
  {
    std::wstring nameTest                = L"test_242";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);
    
    ////////////////////
    // Materials
    ////////////////////

    SimpleMesh cube     = CreateCube(0.75f);
    SimpleMesh plane    = CreatePlane(10.0f);
    SimpleMesh sphere   = CreateSphere(1.0f, 32);
    SimpleMesh torus    = CreateTorus(0.025f, 0.25f, 32, 32);
    SimpleMesh cubeOpen = CreateCubeOpen(4.0f);

    for (size_t i = 0; i < plane.vTexCoord.size(); i++)
      plane.vTexCoord[i] *= 2.0f;

    //HRTextureNodeRef testTex2 = hrTexture2DCreateFromFileDL(L"data/textures/chess_red.bmp");

    HRMaterialRef mat0 = hrMaterialCreate(L"mysimplemat");
    HRMaterialRef mat1 = hrMaterialCreate(L"mysimplemat2");
    HRMaterialRef mat2 = hrMaterialCreate(L"mysimplemat3");
    HRMaterialRef mat3 = hrMaterialCreate(L"mysimplemat4");
    HRMaterialRef mat4 = hrMaterialCreate(L"myblue");
    HRMaterialRef mat5 = hrMaterialCreate(L"mymatplane");

    HRMaterialRef mat6 = hrMaterialCreate(L"red");
    HRMaterialRef mat7 = hrMaterialCreate(L"green");
    HRMaterialRef mat8 = hrMaterialCreate(L"white");

    hrMaterialOpen(mat0, HR_WRITE_DISCARD);
    {
      xml_node matNode = hrMaterialParamNode(mat0);

      xml_node diff = matNode.append_child(L"diffuse");

      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").text().set(L"0.5 0.75 0.5");

      HRTextureNodeRef testTex = hrTexture2DCreateFromFile(L"data/textures/texture1.bmp"); // hrTexture2DCreateFromFileDL
      hrTextureBind(testTex, diff);
    }
    hrMaterialClose(mat0);

    hrMaterialOpen(mat1, HR_WRITE_DISCARD);
    {
      xml_node matNode = hrMaterialParamNode(mat1);

      xml_node diff = matNode.append_child(L"diffuse");

      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").text().set(L"0.207843 0.188235 0");

      // hrTextureBind(testTex2, diff);

      xml_node refl = matNode.append_child(L"reflectivity");

      refl.append_attribute(L"brdf_type").set_value(L"phong");
      refl.append_child(L"color").text().set(L"0.367059 0.345882 0");
      refl.append_child(L"glossiness").text().set(L"0.5");
      refl.append_child(L"energy_fix").append_attribute(L"val") = 1;
      //refl.append_child(L"fresnel_IOR").text().set(L"1.5");
      //refl.append_child(L"fresnel").text().set(L"1");

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

      auto refl = matNode.append_child(L"reflectivity");
      refl.append_attribute(L"brdf_type").set_value(L"phong");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"0.75 0.75 0.75");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"1.0");
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(0);
      refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(1.5);
      refl.append_child(L"energy_fix").append_attribute(L"val") = 1;
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

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRMeshRef teapotRef = hrMeshCreateFromFileDL(L"data/meshes/teapot.vsgf"); // chunk_00009.vsgf // teapot.vsgf // chunk_00591.vsgf

    HRMeshRef cubeOpenRef = hrMeshCreate(L"my_box");
    HRMeshRef planeRef = hrMeshCreate(L"my_plane");
    HRMeshRef torusRef = hrMeshCreate(L"my_torus");

    hrMeshOpen(cubeOpenRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
    {
      hrMeshVertexAttribPointer4f(cubeOpenRef, L"pos", &cubeOpen.vPos[0]);
      hrMeshVertexAttribPointer4f(cubeOpenRef, L"norm", &cubeOpen.vNorm[0]);
      hrMeshVertexAttribPointer2f(cubeOpenRef, L"texcoord", &cubeOpen.vTexCoord[0]);

      int cubeMatIndices[10] = { mat8.id, mat8.id, mat8.id, mat8.id, mat8.id, mat8.id, mat7.id, mat7.id, mat6.id, mat6.id };

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

    hrMeshOpen(torusRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
    {
      hrMeshVertexAttribPointer4f(torusRef, L"pos", &torus.vPos[0]);
      hrMeshVertexAttribPointer4f(torusRef, L"norm", &torus.vNorm[0]);
      hrMeshVertexAttribPointer2f(torusRef, L"texcoord", &torus.vTexCoord[0]);

      hrMeshMaterialId(torusRef, mat0.id);
      hrMeshAppendTriangles3(torusRef, int32_t(torus.triIndices.size()), &torus.triIndices[0]);
    }
    hrMeshClose(torusRef);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    float4 texData[16] = { float4(2,0,0,0), float4(0,2,0,0), float4(0,0,2,0), float4(1,1,1,0),
                           float4(0,1,0,0), float4(0,0,1,0), float4(1,0,0,0), float4(1,1,1,0),
                           float4(0,0,2,0), float4(0,2,2,0), float4(2,0,0,0), float4(1,1,1,0),
                           float4(0,2,0,0), float4(1,2,0,0), float4(0,0,0,0), float4(2,0,0,0) };

    HRTextureNodeRef texForLight = hrTexture2DCreateFromMemory(4, 4, 16, texData);

    HRLightRef meshLightRef = hrLightCreate(L"my_area_light");

    hrLightOpen(meshLightRef, HR_WRITE_DISCARD);
    {
      pugi::xml_node lightNode = hrLightParamNode(meshLightRef);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"mesh");
      lightNode.attribute(L"distribution").set_value(L"diffuse");
      lightNode.append_child(L"mesh").append_attribute(L"id") = torusRef.id;

      pugi::xml_node intensityNode = lightNode.append_child(L"intensity");

      auto colorNode = intensityNode.append_child(L"color");
      intensityNode.append_child(L"multiplier").append_attribute(L"val") = 4.0f*IRRADIANCE_TO_RADIANCE;

      colorNode.append_attribute(L"val") = L"1 1 1";

      auto texNode = hrTextureBind(texForLight, colorNode);
      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 1, 0, 0, -0.5,
                                  0, 4, 0, -0.5,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };
      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);
    }
    hrLightClose(meshLightRef);

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

                                                            //hrRenderEnableDevice(renderRef, 0, true);
    hrRenderEnableDevice(renderRef, CURR_RENDER_DEVICE, true);

    hrRenderOpen(renderRef, HR_WRITE_DISCARD);
    {
      pugi::xml_node node = hrRenderParamNode(renderRef);

      node.append_child(L"width").text() = 512;
      node.append_child(L"height").text() = 512;

      node.append_child(L"method_primary").text() = L"pathtracing";
      node.append_child(L"method_secondary").text() = L"pathtracing";
      node.append_child(L"method_tertiary").text() = L"pathtracing";
      node.append_child(L"method_caustic").text() = L"pathtracing";
      node.append_child(L"shadows").text() = L"1";

      node.append_child(L"trace_depth").text() = L"5";
      node.append_child(L"diff_trace_depth").text() = L"3";

      node.append_child(L"pt_error").text()        = L"1";
      node.append_child(L"minRaysPerPixel").text() = 256;
      node.append_child(L"maxRaysPerPixel").text() = 4096;

      node.append_child(L"draw_tiles").text() = L"0";
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
    float mRot1[4][4], mTranslate[4][4], mRes[4][4];
    float mScale[4][4];

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
    mat4x4_transpose(matrixT, mRot1);
    matrixT[3][3] = 1.0f;
    hrMeshInstance(scnRef, cubeOpenRef, &matrixT[0][0]);

    // mirror plane
    //
    mat4x4_identity(mTranslate);
    mat4x4_identity(mRot1);
    mat4x4_identity(mScale);

    mat4x4_rotate_Z(mRot1, mRot1, -90.0f*DEG_TO_RAD);
    mat4x4_scale(mScale, mScale, 0.25f);
    mat4x4_translate(mTranslate, -3.85f, 0.0f, 0);

    mat4x4_mul(mRot1, mRot1, mScale);
    mat4x4_mul(mRes, mTranslate, mRot1);
    mat4x4_transpose(matrixT, mRes);
    matrixT[3][3] = 1.0f;
    hrMeshInstance(scnRef, planeRef, &matrixT[0][0]);

    /////////////////////////////////////////////////////////////////////// instance light

    mat4x4_identity(mTranslate);
    mat4x4_identity(mRot1);
    mat4x4_identity(mScale);

    mat4x4_translate(mTranslate, 0, -0.75f, 0);
    mat4x4_rotate_X(mRot1, mRot1, -85.0f*DEG_TO_RAD);
    mat4x4_scale(mScale, mScale, 5.0f);

    mat4x4_mul(mRot1, mRot1, mScale);
    mat4x4_mul(mRes, mTranslate, mRot1);
    mat4x4_transpose(matrixT, mRes);
    matrixT[3][3] = 1.0f;
    hrLightInstance(scnRef, meshLightRef, &matrixT[0][0]);

    hrSceneClose(scnRef);

    hrFlush(scnRef, renderRef);

    ////////////////////
    // Rendering, save and check image
    ////////////////////

    RenderProgress(renderRef);

    std::filesystem::create_directories(saveRenderFile.parent_path());
    hrRenderSaveFrameBufferLDR(renderRef, saveRenderFile.wstring().c_str());

    return check_images(ws2s(nameTest).c_str(), 1, 40);
  }


  bool test_243_mesh_light_do_not_sample_me()
  {
    std::wstring nameTest                = L"test_243";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);
    
    ////////////////////
    // Materials
    ////////////////////

    SimpleMesh cube     = CreateCube(0.75f);
    SimpleMesh plane    = CreatePlane(10.0f);
    SimpleMesh sphere   = CreateSphere(1.0f, 32);
    SimpleMesh torus    = CreateTorus(0.05f, 0.6f, 32, 32);
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

    hrMaterialOpen(mat0, HR_WRITE_DISCARD);
    {
      xml_node matNode = hrMaterialParamNode(mat0);

      xml_node diff = matNode.append_child(L"diffuse");

      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").text().set(L"0.5 0.75 0.5");

      HRTextureNodeRef testTex = hrTexture2DCreateFromFile(L"data/textures/texture1.bmp"); // hrTexture2DCreateFromFileDL
      hrTextureBind(testTex, diff);
    }
    hrMaterialClose(mat0);

    hrMaterialOpen(mat1, HR_WRITE_DISCARD);
    {
      xml_node matNode = hrMaterialParamNode(mat1);

      xml_node diff = matNode.append_child(L"diffuse");

      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").text().set(L"0.207843 0.188235 0");

      // hrTextureBind(testTex2, diff);

      xml_node refl = matNode.append_child(L"reflectivity");

      refl.append_attribute(L"brdf_type").set_value(L"phong");
      refl.append_child(L"color").text().set(L"0.367059 0.345882 0");
      refl.append_child(L"glossiness").text().set(L"0.5");
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

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRMeshRef teapotRef = hrMeshCreateFromFileDL(L"data/meshes/teapot.vsgf"); // chunk_00009.vsgf // teapot.vsgf // chunk_00591.vsgf

    HRMeshRef cubeOpenRef = hrMeshCreate(L"my_box");
    HRMeshRef planeRef = hrMeshCreate(L"my_plane");
    HRMeshRef torusRef = hrMeshCreate(L"my_torus");

    hrMeshOpen(cubeOpenRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
    {
      hrMeshVertexAttribPointer4f(cubeOpenRef, L"pos", &cubeOpen.vPos[0]);
      hrMeshVertexAttribPointer4f(cubeOpenRef, L"norm", &cubeOpen.vNorm[0]);
      hrMeshVertexAttribPointer2f(cubeOpenRef, L"texcoord", &cubeOpen.vTexCoord[0]);

      int cubeMatIndices[10] = { mat8.id, mat8.id, mat8.id, mat8.id, mat8.id, mat8.id, mat7.id, mat7.id, mat6.id, mat6.id };

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

    hrMeshOpen(torusRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
    {
      hrMeshVertexAttribPointer4f(torusRef, L"pos", &torus.vPos[0]);
      hrMeshVertexAttribPointer4f(torusRef, L"norm", &torus.vNorm[0]);
      hrMeshVertexAttribPointer2f(torusRef, L"texcoord", &torus.vTexCoord[0]);

      hrMeshMaterialId(torusRef, mat0.id);
      hrMeshAppendTriangles3(torusRef, int32_t(torus.triIndices.size()), &torus.triIndices[0]);
    }
    hrMeshClose(torusRef);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRLightRef meshLightRef = hrLightCreate(L"my_area_light");

    hrLightOpen(meshLightRef, HR_WRITE_DISCARD);
    {
      pugi::xml_node lightNode = hrLightParamNode(meshLightRef);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"mesh");
      lightNode.attribute(L"distribution").set_value(L"diffuse");
      lightNode.append_child(L"mesh").append_attribute(L"id") = torusRef.id;

      pugi::xml_node intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val")      = L"1 1 1";
      intensityNode.append_child(L"multiplier").append_attribute(L"val") = 2.0f*IRRADIANCE_TO_RADIANCE;
    }
    hrLightClose(meshLightRef);

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

                                                            //hrRenderEnableDevice(renderRef, 0, true);
    hrRenderEnableDevice(renderRef, CURR_RENDER_DEVICE, true);

    hrRenderOpen(renderRef, HR_WRITE_DISCARD);
    {
      pugi::xml_node node = hrRenderParamNode(renderRef);

      node.append_child(L"width").text() = 512;
      node.append_child(L"height").text() = 512;

      node.append_child(L"method_primary").text() = L"pathtracing";
      node.append_child(L"method_secondary").text() = L"pathtracing";
      node.append_child(L"method_tertiary").text() = L"pathtracing";
      node.append_child(L"method_caustic").text() = L"pathtracing";
      node.append_child(L"shadows").text() = L"1";

      node.append_child(L"trace_depth").text() = L"5";
      node.append_child(L"diff_trace_depth").text() = L"3";

      node.append_child(L"pt_error").text() = L"2";
      node.append_child(L"minRaysPerPixel").text() = L"256";
      node.append_child(L"maxRaysPerPixel").text() = L"4096";

      node.append_child(L"draw_tiles").text() = L"0";
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
    float mRot1[4][4], mTranslate[4][4], mRes[4][4];
    float mScale[4][4];

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

    /////////////////////////////////////////////////////////////////////// instance light

    mat4x4_identity(mTranslate);
    mat4x4_identity(mRot1);
    mat4x4_identity(mScale);

    mat4x4_translate(mTranslate, 0, -2.0f, 0);
    mat4x4_rotate_X(mRot1, mRot1, -75.0f*DEG_TO_RAD);
    mat4x4_scale(mScale, mScale, 4.0f);

    mat4x4_mul(mRot1, mRot1, mScale);
    mat4x4_mul(mRes, mTranslate, mRot1);
    mat4x4_transpose(matrixT, mRes);
    matrixT[3][3] = 1.0f;
    hrLightInstance(scnRef, meshLightRef, &matrixT[0][0], L"do_not_sample_me=\"1\" ");

    hrSceneClose(scnRef);

    hrFlush(scnRef, renderRef);

    ////////////////////
    // Rendering, save and check image
    ////////////////////

    RenderProgress(renderRef);

    std::filesystem::create_directories(saveRenderFile.parent_path());
    hrRenderSaveFrameBufferLDR(renderRef, saveRenderFile.wstring().c_str());

    return check_images(ws2s(nameTest).c_str(), 1, 35);
  }


  bool test_244_do_not_sample_me()
  {
    std::wstring nameTest                = L"test_244";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);
    
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
    // Light
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRLightRef areaLightRef = hrLightCreate(L"myarea");

    hrLightOpen(areaLightRef, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(areaLightRef);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"rect");
      lightNode.attribute(L"distribution").set_value(L"diffuse");

      auto sizeNode = lightNode.append_child(L"size");
      sizeNode.append_attribute(L"half_length").set_value(1.0f);
      sizeNode.append_attribute(L"half_width").set_value(1.0f);

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(2.0f*IRRADIANCE_TO_RADIANCE);

      VERIFY_XML(lightNode);
    }
    hrLightClose(areaLightRef);


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

    HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 512, 512, 256, 2048);


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
    mRot       = rotate4x4Y(180.0f*DEG_TO_RAD);
    mRes       = mul(mTranslate, mRot);

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

    // mTranslate.identity();
    // mTranslate = translate4x4(float3(-1.0f, 4.0f, -1.0f));
    // hrMeshInstance(scnRef, lightMesh, mTranslate.L());

    mTranslate.identity();
    mRes.identity();
    mTranslate = translate4x4(float3(-3.0f, 8.0f, -5.0f));
    mRes = mul(mTranslate, mRes);
    hrLightInstance(scnRef, areaLightRef, mRes.L());

    mTranslate.identity();
    mRes.identity();
    mTranslate = translate4x4(float3(+3.0f, 8.0f, -5.0f));
    mRes = mul(mTranslate, mRes);
    hrLightInstance(scnRef, areaLightRef, mRes.L(), L"do_not_sample_me = \"1\" ");

    ///////////

    hrSceneClose(scnRef);

    hrFlush(scnRef, renderRef);

    ////////////////////
    // Rendering, save and check image
    ////////////////////

    RenderProgress(renderRef);

    std::filesystem::create_directories(saveRenderFile.parent_path());
    hrRenderSaveFrameBufferLDR(renderRef, saveRenderFile.wstring().c_str());

    return check_images(ws2s(nameTest).c_str(), 1, 115);
  }


  bool test_245_cylinder_tex_nearest()
  {
    std::wstring nameTest                = L"test_245";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);
    
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
      multNode.append_attribute(L"val").set_value(1.0*IRRADIANCE_TO_RADIANCE);

      auto texNode = hrTextureBind(texForLight, colorNode);

      texNode.append_attribute(L"addressing_mode_u").set_value(L"clamp");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"clamp");
      texNode.append_attribute(L"filter").set_value(L"point");

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
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(1.0*IRRADIANCE_TO_RADIANCE);

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

    hrSceneOpen(scnRef, HR_WRITE_DISCARD);
    ///////////

    mTranslate = translate4x4(float3(0.0f, 3.0f, 0.0f));
    mRot       = rotate4x4Y(180.0f*DEG_TO_RAD);
    mRes       = mul(mTranslate, mRot);

    hrMeshInstance(scnRef, cubeOpen, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(-3.0f, 1.25f, 0.0f));
    mRes       = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph1, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(3.0f, 2.25f, 0.0f));
    mRes       = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph2, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(-3.0f, 8.0f, 0.0f));
    mRot       = rotate4x4Y(90.0f*DEG_TO_RAD);
    mRes       = mul(mTranslate, mRot);

    hrLightInstance(scnRef, spot2, mRes.L());

    ///////////
    float4x4 mRot2;
    mTranslate = translate4x4(float3(4.0f, -1.5f, 0.0f));
    mRot       = rotate4x4Y(20.0f*DEG_TO_RAD);
    mRot2      = rotate4x4X(-20.0f*DEG_TO_RAD);
    mRes       = mul(mTranslate, mul(mRot, mRot2));

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
    hrRenderSaveFrameBufferLDR(renderRef, saveRenderFile.wstring().c_str());

    return check_images(ws2s(nameTest).c_str(), 1, 20);
  }

  bool test_248_rotated_area_mis_specular()
  {
    std::wstring nameTest                 = L"test_248";
    std::filesystem::path libraryPath     = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile  = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);
    
    ////////////////////
    // Materials
    ////////////////////

    auto mat_red    = hrMaterialCreate(L"red");
    auto mat_green  = hrMaterialCreate(L"green");
    auto mat_white  = hrMaterialCreate(L"white");    
    auto mat_glossy = hrMaterialCreate(L"glossy_wall_mat");
    auto mat_mirror = hrMaterialCreate(L"mirror");

    hrMaterialOpen(mat_red, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(mat_red);
      AddDiffuseNode(matNode, L"0.5 0 0");
    }
    hrMaterialClose(mat_red);

    hrMaterialOpen(mat_green, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(mat_green);
      AddDiffuseNode(matNode, L"0 0.5 0");
    }
    hrMaterialClose(mat_green);

    hrMaterialOpen(mat_white, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(mat_white);
      AddDiffuseNode(matNode, L"0.7 0.7 0.7");
    }
    hrMaterialClose(mat_white);

    hrMaterialOpen(mat_glossy, HR_WRITE_DISCARD);
    {
      xml_node matNode = hrMaterialParamNode(mat_glossy);
      AddReflectionNode(matNode, L"ggx", L"0.5 0.5 0.5", 0.85, false);
    }
    hrMaterialClose(mat_glossy);

    hrMaterialOpen(mat_mirror, HR_WRITE_DISCARD);
    {
      xml_node matNode = hrMaterialParamNode(mat_mirror);
      AddReflectionNode(matNode, L"ggx", L"0.85 0.85 0.85", 1.0, false);
    }
    hrMaterialClose(mat_mirror);

    ////////////////////
    // Meshes
    ////////////////////
    
    auto cubeOpen    = CreateCubeOpen(4.0f);
    auto teapotRef   = hrMeshCreateFromFile(L"data/meshes/teapot.vsgf"); // chunk_00009.vsgf // teapot.vsgf // chunk_00591.vsgf
    auto cubeOpenRef = hrMeshCreate(L"my_box");

    hrMeshOpen(cubeOpenRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
    {
      hrMeshVertexAttribPointer4f(cubeOpenRef, L"pos", &cubeOpen.vPos[0]);
      hrMeshVertexAttribPointer4f(cubeOpenRef, L"norm", &cubeOpen.vNorm[0]);
      hrMeshVertexAttribPointer2f(cubeOpenRef, L"texcoord", &cubeOpen.vTexCoord[0]);

      int cubeMatIndices[10] = { mat_white.id, mat_white.id, mat_white.id, mat_glossy.id, mat_glossy.id, mat_glossy.id, mat_green.id, mat_green.id, mat_red.id, mat_red.id };

      //hrMeshMaterialId(cubeRef, 0);
      hrMeshPrimitiveAttribPointer1i(cubeOpenRef, L"mind", cubeMatIndices);
      hrMeshAppendTriangles3(cubeOpenRef, int(cubeOpen.triIndices.size()), &cubeOpen.triIndices[0]);
    }
    hrMeshClose(cubeOpenRef);

    ////////////////////
    // Light
    ////////////////////

    auto rectLight = CreateLight(L"my_area_light", L"area", L"rect", L"diffuse", 0.5f, 1, L"1 1 1", 10.0f * IRRADIANCE_TO_RADIANCE);

    ////////////////////
    // Camera
    ////////////////////

    CreateCamera(45, L"0 0 14", L"0 0 0");

    ////////////////////
    // Render settings
    ////////////////////

    auto renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 512, 512, 256, 4096, 10, 10);

    ////////////////////
    // Create scene
    ////////////////////
    
    auto scnRef = hrSceneCreate((L"scene_" + nameTest).c_str());

    hrSceneOpen(scnRef, HR_WRITE_DISCARD);

    int32_t remapList[2] = { 1, mat_mirror.id };    
    AddMeshToScene(scnRef, teapotRef,   float3(0, -0.7f * 3.65f, 0), float3(), float3(3.65f, 3.65f, 3.65f), remapList, 2);
    AddMeshToScene(scnRef, cubeOpenRef, float3(0, 0, 0), float3(0, 180, 0));
    AddLightToScene(scnRef, rectLight,  float3(-2.7f, 3, 0), float3(30, 20, 0));
    AddLightToScene(scnRef, rectLight,  float3(3, 3.85, -2.85f));

    hrSceneClose(scnRef);
      
    hrFlush(scnRef, renderRef);
  
    ////////////////////
    // Rendering, save and check image
    ////////////////////

    RenderProgress(renderRef);

    hrRenderSaveFrameBufferLDR(renderRef, saveRenderFile.wstring().c_str());

    return check_images(ws2s(nameTest).c_str(), 1, 20);
  }

  bool test_249_point_spot_projective()
  {
    std::wstring nameTest                = L"test_249";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);
    
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
    
    auto projectiveTexId = hrTexture2DCreateFromFile(L"data/textures/smile.png");

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

      auto projective = lightNode.append_child(L"projective");
      projective.append_attribute(L"type").set_value(L"uvn");
      
      projective.append_child(L"fov").text() = 45;
      projective.append_child(L"nearClipPlane").text() = 1.0f;
      projective.append_child(L"farClipPlane").text()  = 100.0f;

      auto texNode = hrTextureBind(projectiveTexId, projective);
      texNode.append_attribute(L"addressing_mode_u") = L"clamp";
      texNode.append_attribute(L"addressing_mode_v") = L"clamp";

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

    HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 512, 512, 256, 256);


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

    mTranslate = translate4x4(float3(0.0f, 8.95f, -1.0f));
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
    hrRenderSaveFrameBufferLDR(renderRef, saveRenderFile.wstring().c_str());

    return check_images(ws2s(nameTest).c_str(), 1, 30);
  }
};