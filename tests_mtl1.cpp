#include "tests.h"
#include <iomanip>

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
#include "../hydra_api/HR_HDRImageTool.h"

///////////////////////////////////////////////////////////////////////////////////

#pragma warning(disable:4996)
#pragma warning(disable:4838)
#pragma warning(disable:4244)

extern GLFWwindow* g_window;
using namespace TEST_UTILS;


namespace MTL_TESTS
{
  namespace hlm = LiteMath;
  
  bool test_100_diffuse_orennayar()
  {
    std::wstring nameTest                = L"test_100";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);

    ///////////////
    // Materials
    ///////////////

    auto mat01   = hrMaterialCreate(L"mat01");
    auto mat02   = hrMaterialCreate(L"mat02");
    auto mat03   = hrMaterialCreate(L"mat03");
    auto matGray = hrMaterialCreate(L"matGray");

    hrMaterialOpen(mat01, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(mat01);
      AddDiffuseNode(matNode, L"0.8 0.8 0.8", L"orennayar", 0);
    }
    hrMaterialClose(mat01);

    hrMaterialOpen(mat02, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(mat02);
      AddDiffuseNode(matNode, L"0.8 0.8 0.8", L"orennayar", 0.5f);
    }
    hrMaterialClose(mat02);

    hrMaterialOpen(mat03, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(mat03);
      AddDiffuseNode(matNode, L"0.8 0.8 0.8", L"orennayar", 1);
    }
    hrMaterialClose(mat03);

    hrMaterialOpen(matGray, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGray);
      AddDiffuseNode(matNode, L"0.5 0.5 0.5");
    }
    hrMaterialClose(matGray);

    ////////////////////
    // Meshes
    ////////////////////

    auto sphere01 = HRMeshFromSimpleMesh(L"cubeR", CreateSphere(2.0f, 64), mat01.id);
    auto sphere02 = HRMeshFromSimpleMesh(L"cubeG", CreateSphere(2.0f, 64), mat02.id);
    auto sphere03 = HRMeshFromSimpleMesh(L"cubeB", CreateSphere(2.0f, 64), mat03.id);
    auto plane    = HRMeshFromSimpleMesh(L"plane", CreatePlane(10.0f), matGray.id);

    ////////////////////
    // Light
    ////////////////////
    
    auto rectLight = CreateLight(L"Light01", L"area", L"rect", L"diffuse", 12.0f, 12.0f, L"1 1 1", 2.0f);

    ////////////////////
    // Camera
    ////////////////////
    
    CreateCamera(18, L"0 15 0.1", L"0 1 0");

    ////////////////////
    // Render settings
    ////////////////////

    auto renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 1024, 340, 256, 256);

    ////////////////////
    // Create scene
    ////////////////////

    auto scnRef = hrSceneCreate((L"scene_"+ nameTest).c_str());

    hrSceneOpen(scnRef, HR_WRITE_DISCARD);

    AddMeshToScene (scnRef, plane,     float3(0.0f, -1.0f, 0.0f));
    AddMeshToScene (scnRef, sphere01,  float3(-4.25f, 1.0f, 0.0f));
    AddMeshToScene (scnRef, sphere03,  float3(4.25f, 1.0f, 0.0f));
    AddMeshToScene (scnRef, sphere02,  float3(0.0f, 1.0f, 0.0f));
    AddLightToScene(scnRef, rectLight, float3(0.0f, 16.0f, 0.0f));

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



  bool test_101_diffuse_lambert_orbspec_mat01()
  {
    std::wstring nameTest                = L"test_101";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);

    ////////////////////
    // Materials
    ////////////////////

    auto mat1 = hrMaterialCreate(L"red");
    auto mat2 = hrMaterialCreate(L"green");
    auto mat3 = hrMaterialCreate(L"white");

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
      AddDiffuseNode(matNode, L"0.7 0.7 0.7");
    }
    hrMaterialClose(mat3);

    ////////////////////
    // Meshes
    ////////////////////

    auto sphereRef   = HRMeshFromSimpleMesh(L"sphereG", CreateSphere(2.5f, 128), mat3.id);
    auto cubeOpenRef = CreateCornellBox(4.0f, mat1, mat2, mat3, mat3, mat3);

    ////////////////////
    // Light
    ////////////////////

    auto rectLight = CreateLight(L"Light01", L"area", L"rect", L"diffuse", 1.0f, 1.0f, L"1 1 1", 6.0f * IRRADIANCE_TO_RADIANCE);

    ////////////////////
    // Camera
    ////////////////////

    CreateCamera(45, L"0 0 14", L"0 0 0");    

    ////////////////////
    // Render settings
    ////////////////////

    auto renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 512, 512, 256, 2048);
        
    ////////////////////
    // Create scene
    ////////////////////

    auto scnRef = hrSceneCreate((L"scene_" + nameTest).c_str());

    hrSceneOpen(scnRef, HR_WRITE_DISCARD);

    AddMeshToScene(scnRef, sphereRef, float3(0.0f, -1.5f, 1.0f));
    AddMeshToScene(scnRef, cubeOpenRef, float3(0.0f, 0.0f, 0.0f), float3(0.0f, 180.0f, 0.0f));
    AddLightToScene(scnRef, rectLight, float3(0.0f, 3.85f, 0.0f));
    
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



  bool test_102_mirror_orbspec_mat02()
  {
    std::wstring nameTest                = L"test_102";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);

    ////////////////////
    // Materials
    ////////////////////

    HRMaterialRef mat1 = hrMaterialCreate(L"red");
    HRMaterialRef mat2 = hrMaterialCreate(L"green");
    HRMaterialRef mat3 = hrMaterialCreate(L"white");
    HRMaterialRef mat4 = hrMaterialCreate(L"mirror");


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
      AddDiffuseNode(matNode, L"0.7 0.7 0.7");
    }
    hrMaterialClose(mat3);

    hrMaterialOpen(mat4, HR_WRITE_DISCARD);
    {
      xml_node matNode = hrMaterialParamNode(mat4);      
      AddReflectionNode(matNode, L"phong", L"0.75 0.75 0.75", 1, false);
    }
    hrMaterialClose(mat4);

    ////////////////////
    // Meshes
    ////////////////////

    auto cubeOpenRef = CreateCornellBox(4.0f, mat4, mat2, mat3, mat3, mat3);
    auto teapotRef   = hrMeshCreateFromFile(L"data/meshes/teapot.vsgf");

    ////////////////////
    // Light
    ////////////////////

    auto rectLight = CreateLight(L"Light01", L"area", L"rect", L"diffuse", 1.0f, 1.0f, L"1 1 1", 8.0f * IRRADIANCE_TO_RADIANCE);

    ////////////////////
    // Camera
    ////////////////////

    CreateCamera(45, L"0 0 14", L"0 0 0");

    ////////////////////
    // Render settings
    ////////////////////

    auto renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 512, 512, 256, 4096, 10, 6);

    ////////////////////
    // Create scene
    ////////////////////

    auto scnRef = hrSceneCreate((L"scene_" + nameTest).c_str());

    hrSceneOpen(scnRef, HR_WRITE_DISCARD);

    int32_t remapList[2] = { 1, mat4.id };
    AddMeshToScene(scnRef, teapotRef, float3(0.0f, -2.5f, 0.0f), float3(0.0f, 0.0f, 0.0f), float3(3.75f, 3.75f, 3.75f), remapList, 2);
    AddMeshToScene(scnRef, cubeOpenRef, float3(0.0f, 0.0f, 0.0f), float3(0.0f, 180.0f, 0.0f));
    AddLightToScene(scnRef, rectLight, float3(0.0f, 3.85f, 0.0f));

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



  bool test_103_diffuse_texture()
  {
    std::wstring nameTest                = L"test_103";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);

    ////////////////////
    // Materials
    ////////////////////

    HRMaterialRef matR           = hrMaterialCreate(L"matR");
    HRMaterialRef matG           = hrMaterialCreate(L"matG");
    HRMaterialRef matB           = hrMaterialCreate(L"matB");
    HRMaterialRef matGray        = hrMaterialCreate(L"matGray");

    HRTextureNodeRef texFloor    = hrTexture2DCreateFromFile(L"data/textures/diff.jpg");
    HRTextureNodeRef texChecker  = hrTexture2DCreateFromFile(L"data/textures/chess_white.bmp");
    HRTextureNodeRef texChecker2 = hrTexture2DCreateFromFile(L"data/textures/checker_8x8.gif");

    hrMaterialOpen(matR, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matR);
      AddDiffuseNode(matNode, L"0.95 0.05 0.05", L"lambert", 0, texChecker);
    }
    hrMaterialClose(matR);

    hrMaterialOpen(matG, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matG);
      AddDiffuseNode(matNode, L"0.05 0.95 0.05", L"lambert", 0, texChecker, L"wrap", L"wrap", 4, 4);
    }
    hrMaterialClose(matG);

    hrMaterialOpen(matB, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matB);
      AddDiffuseNode(matNode, L"0.05 0.05 0.95", L"lambert", 0, texChecker, L"wrap", L"wrap", 4, 4);
    }
    hrMaterialClose(matB);

    hrMaterialOpen(matGray, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGray);
      AddDiffuseNode(matNode, L"1.0 1.0 1.0", L"lambert", 0, texFloor);
    }
    hrMaterialClose(matGray);

    ////////////////////
    // Meshes
    ////////////////////

    auto cubeR    = HRMeshFromSimpleMesh(L"cubeR",    CreateCube(2.0f), matR.id);
    auto sphereG  = HRMeshFromSimpleMesh(L"sphereG",  CreateSphere(4.0f, 64), matG.id);
    auto torusB   = HRMeshFromSimpleMesh(L"torusB",   CreateTorus(0.8f, 2.0f, 64, 64), matB.id);
    auto planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(10.0f), matGray.id);

    ////////////////////
    // Light
    ////////////////////

    auto rectLight = CreateLight(L"Light01", L"area", L"rect", L"diffuse", 8, 8, L"1 1 1", 4.0f * IRRADIANCE_TO_RADIANCE);

    ////////////////////
    // Camera
    ////////////////////

    CreateCamera(45, L"0 13 16", L"0 0 0");

    ////////////////////
    // Render settings
    ////////////////////

    HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 512, 512, 256, 512);

    ////////////////////
    // Create scene
    ////////////////////

    HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

    hrSceneOpen(scnRef, HR_WRITE_DISCARD);

    AddMeshToScene(scnRef, planeRef, float3(0.0f, -1.0f, 0.0f));
    AddMeshToScene(scnRef, cubeR, float3(-4.0f, 1.0f, 4.0f), float3(0.0f, 60.0f, 0.0f));
    AddMeshToScene(scnRef, torusB, float3(4.0f, 1.0f, 4.5f), float3(80.0f, -60.0f, 0.0f)); 
    AddMeshToScene(scnRef, sphereG, float3(0.0f, 2.0f, -1.0f));
    AddLightToScene(scnRef, rectLight, float3(0.0f, 12.0f, 0.0f));

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


  namespace testphong
  {
    const float glosscoeff[10][4] = {
     { 8.88178419700125e-14f, -1.77635683940025e-14f, 5.0f, 1.0f },     //0-0.1
     { 357.142857142857f, -35.7142857142857f, 5.0f, 1.5f },             //0.1-0.2
     { -2142.85714285714f, 428.571428571429f, 8.57142857142857f, 2.0f}, //0.2-0.3
     { 428.571428571431f, -42.8571428571432f, 30.0f, 5.0f },            //0.3-0.4
     { 2095.23809523810f, -152.380952380952f, 34.2857142857143f, 8.0f}, //0.4-0.5
     { -4761.90476190476f, 1809.52380952381f, 66.6666666666667f, 12.0f},//0.5-0.6
     { 9914.71215351811f, 1151.38592750533f, 285.714285714286f, 32.0f}, //0.6-0.7
     { 45037.7068059246f, 9161.90096119855f, 813.432835820895f, 82.0f}, //0.7-0.8
     { 167903.678757035f, 183240.189801913f, 3996.94423223835f, 300.0f},//0.8-0.9
     { -20281790.7444668f, 6301358.14889336f, 45682.0925553320f,2700.0f}//0.9-1.0
    };

    static inline float cosPowerFromGlosiness(float glosiness)
    {
      const float cMin = 1.0f;
      const float cMax = 1000000.0f;

      const float x  = glosiness;
      const int   k  = (fabs(x - 1.0f) < 1e-5f) ? 10 : (int)(x*10.0f);
      const float x1 = (x - (float)(k)*0.1f);

      if (k == 10 || x >= 0.99f)
        return cMax;
      else
        return glosscoeff[k][3] + glosscoeff[k][2] * x1    \
                                + glosscoeff[k][1] * x1*x1 \
                                + glosscoeff[k][0] * x1*x1*x1;
    }
  };


  bool test_104_reflect_phong_orbspec_mat03()
  {
    std::wstring nameTest                = L"test_104";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);

    ////////////////////
    // Materials
    ////////////////////

    HRMaterialRef mat0 = hrMaterialCreate(L"mysimplemat");
    HRMaterialRef mat1 = hrMaterialCreate(L"red");
    HRMaterialRef mat2 = hrMaterialCreate(L"green");
    HRMaterialRef mat3 = hrMaterialCreate(L"white");

    hrMaterialOpen(mat0, HR_WRITE_DISCARD);
    {
      xml_node matNode = hrMaterialParamNode(mat0);
      auto refl        = matNode.append_child(L"reflectivity");

      refl.append_attribute(L"brdf_type").set_value(L"phong");
      refl.append_child(L"color").append_attribute(L"val")      = L"0.95 0.95 0.95";
      refl.append_child(L"glossiness").append_attribute(L"val") = 0.8f;
      refl.append_child(L"extrusion").append_attribute(L"val")  = L"maxcolor";
      refl.append_child(L"fresnel").append_attribute(L"val")    = 0;
      refl.append_child(L"energy_fix").append_attribute(L"val") = 0;

      VERIFY_XML(matNode);
    }
    hrMaterialClose(mat0);

    //std::cout << "cosPower(0.8) = " << testphong::cosPowerFromGlosiness(0.8f) << std::endl;

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

    ////////////////////
    ////////////////////
    ////////////////////
    ////////////////////
    SimpleMesh sphere   = CreateSphere(2.5f, 128);
    SimpleMesh cubeOpen = CreateCubeOpen(4.0f);

    HRMeshRef cubeOpenRef = hrMeshCreate(L"my_box");
    HRMeshRef planeRef    = hrMeshCreate(L"my_plane");
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
        sphere.matIndices[i] = mat0.id;

      hrMeshPrimitiveAttribPointer1i(sphereRef, L"mind", &sphere.matIndices[0]);
      hrMeshAppendTriangles3(sphereRef, int32_t(sphere.triIndices.size()), &sphere.triIndices[0]);
    }
    hrMeshClose(sphereRef);

    ////////////////////
    ////////////////////

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

    ////////////////////
    ////////////////////
    ////////////////////
    ////////////////////

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

    ////////////////////
    // Render settings
    ////////////////////

    auto renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 512, 512, 256, 4096);


    // create scene
    //
    HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

    const float DEG_TO_RAD = float(3.14159265358979323846f) / 180.0f;

    hrSceneOpen(scnRef, HR_WRITE_DISCARD);
    {
      // instance sphere and cornell box
      //
      auto mtranslate = hlm::translate4x4(hlm::float3(0, -1.5, 1));
      hrMeshInstance(scnRef, sphereRef, mtranslate.L());

      auto mrot = hlm::rotate4x4Y(180.0f*DEG_TO_RAD);
      hrMeshInstance(scnRef, cubeOpenRef, mrot.L());

      //// instance light (!!!)
      //
      mtranslate = hlm::translate4x4(hlm::float3(0, 3.85f, 0));
      hrLightInstance(scnRef, rectLight, mtranslate.L());
    }
    hrSceneClose(scnRef);

    float glossValues[3]     = {0.5f, 0.6f, 0.7f};
    std::wstring outNames[3] = {L"tests_images/test_104/z_out.png",
                                L"tests_images/test_104/z_out2.png",
                                L"tests_images/test_104/z_out3.png"};

    for(int j=0;j<3;j++)
    {
      hrMaterialOpen(mat0, HR_OPEN_EXISTING);
      {
        xml_node matNode = hrMaterialParamNode(mat0);
        matNode.child(L"reflectivity").child(L"glossiness").attribute(L"val") = glossValues[j];
      }
      hrMaterialClose(mat0);

      std::cout << "cosPower(" << glossValues[j] << ") = " << testphong::cosPowerFromGlosiness(glossValues[j]) << std::endl;

      hrFlush(scnRef, renderRef, camRef);

      ////////////////////
      // Rendering, save and check image
      ////////////////////

      RenderProgress(renderRef);

      hrRenderSaveFrameBufferLDR(renderRef, outNames[j].c_str());
    } // for(int j=0;j<3;j++)

    return check_images("test_104", 3, 30);
  }


  bool test_105_reflect_torranse_sparrow()
  {
    std::wstring nameTest                = L"test_105";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);

    ////////////////////
    // Materials
    ////////////////////

    HRMaterialRef matMetal        = hrMaterialCreate(L"matMetal");
    HRMaterialRef matMetalBrushed = hrMaterialCreate(L"matMetalBrushed");
    HRMaterialRef matSomething    = hrMaterialCreate(L"matSomething");
    HRMaterialRef matGray         = hrMaterialCreate(L"matGray");

    hrMaterialOpen(matMetal, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matMetal);

      auto refl = matNode.append_child(L"reflectivity");

      refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"0.99 0.6 0.0");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.999");
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(8.0f);

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matMetal);

    hrMaterialOpen(matMetalBrushed, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matMetalBrushed);

      auto refl = matNode.append_child(L"reflectivity");

      refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"0.8 0.8 0.8");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.75");
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(8.0f);

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matMetalBrushed);

    hrMaterialOpen(matSomething, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matSomething);

      auto diff = matNode.append_child(L"diffuse");

      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").append_attribute(L"val").set_value(L"0.0 0.8 0.0");

      auto refl = matNode.append_child(L"reflectivity");

      refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 0.5");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.85");
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_IOR").append_attribute(L"val").set_value(1.5);

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matSomething);

    hrMaterialOpen(matGray, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGray);

      auto diff = matNode.append_child(L"diffuse");

      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").append_attribute(L"val").set_value(L"0.25 0.25 0.25");

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matGray);

    ////////////////////
    // Meshes
    ////////////////////
    HRMeshRef sphere00 = HRMeshFromSimpleMesh(L"cubeR", CreateSphere(2.0f, 64), matMetal.id);
    HRMeshRef sphere03 = HRMeshFromSimpleMesh(L"cubeG", CreateSphere(2.0f, 64), matMetalBrushed.id);
    HRMeshRef sphere08 = HRMeshFromSimpleMesh(L"cubeB", CreateSphere(2.0f, 64), matSomething.id);
    HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(10.0f), matGray.id);

    ////////////////////
    // Light
    ////////////////////

    HRLightRef rectLight = hrLightCreate(L"my_area_light");

    hrLightOpen(rectLight, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(rectLight);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"rect");
      lightNode.attribute(L"distribution").set_value(L"diffuse");

      auto sizeNode = lightNode.append_child(L"size");

      sizeNode.append_attribute(L"half_length").set_value(12.0f);
      sizeNode.append_attribute(L"half_width").set_value(12.0f);

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(2.0f*IRRADIANCE_TO_RADIANCE);
			VERIFY_XML(lightNode);
    }
    hrLightClose(rectLight);

    ////////////////////
    // Camera
    ////////////////////

    HRCameraRef camRef = hrCameraCreate(L"my camera");

    hrCameraOpen(camRef, HR_WRITE_DISCARD);
    {
      auto camNode = hrCameraParamNode(camRef);

      camNode.append_child(L"fov").text().set(L"30");
      camNode.append_child(L"nearClipPlane").text().set(L"0.01");
      camNode.append_child(L"farClipPlane").text().set(L"100.0");

      camNode.append_child(L"up").text().set(L"0 1 0");
      camNode.append_child(L"position").text().set(L"0 1 12.2");
      camNode.append_child(L"look_at").text().set(L"0 1 0");
    }
    hrCameraClose(camRef);

    ////////////////////
    // Render settings
    ////////////////////

    HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 1024, 500, 256, 1024);

    ////////////////////
    // Create scene
    ////////////////////

    HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

    float matrixT[4][4];
    float mTranslate[4][4];
    float mRes[4][4];
    float mRes2[4][4];

    hrSceneOpen(scnRef, HR_WRITE_DISCARD);

    ///////////

    mat4x4_identity(mTranslate);
    mat4x4_identity(mRes);

    mat4x4_translate(mTranslate, 0.0f, -1.0f, 0.0f);
    mat4x4_mul(mRes2, mTranslate, mRes);
    mat4x4_transpose(matrixT, mRes2);

    hrMeshInstance(scnRef, planeRef, &matrixT[0][0]);

    ///////////

    mat4x4_identity(mTranslate);
    mat4x4_identity(mRes);

    mat4x4_translate(mTranslate, -4.25f, 1.0f, 0.0f);
    mat4x4_mul(mRes2, mTranslate, mRes);
    mat4x4_transpose(matrixT, mRes2);

    hrMeshInstance(scnRef, sphere00, &matrixT[0][0]);

    ///////////

    mat4x4_identity(mTranslate);
    mat4x4_identity(mRes);

    mat4x4_translate(mTranslate, 4.25f, 1.0f, 0.0f);
    mat4x4_mul(mRes2, mTranslate, mRes);
    mat4x4_transpose(matrixT, mRes2); //swap rows and columns

    hrMeshInstance(scnRef, sphere08, &matrixT[0][0]);

    ///////////

    mat4x4_identity(mTranslate);
    mat4x4_identity(mRes);

    mat4x4_translate(mTranslate, 0.0f, 1.0f, 0.0f);
    mat4x4_mul(mRes2, mTranslate, mRes);
    mat4x4_transpose(matrixT, mRes2); //swap rows and columns

    hrMeshInstance(scnRef, sphere03, &matrixT[0][0]);

    ///////////

    mat4x4_identity(mTranslate);
    mat4x4_translate(mTranslate, 0, 16.0f, 0.0);
    mat4x4_transpose(matrixT, mTranslate);

    hrLightInstance(scnRef, rectLight, &matrixT[0][0]);

    ///////////

    hrSceneClose(scnRef);

    hrFlush(scnRef, renderRef);

    ////////////////////
    // Rendering, save and check image
    ////////////////////

    RenderProgress(renderRef);

    std::filesystem::create_directories(saveRenderFile.parent_path());
    hrRenderSaveFrameBufferLDR(renderRef, saveRenderFile.wstring().c_str());

    return check_images(ws2s(nameTest).c_str(), 1, 60);
  }


  bool test_106_reflect_fresnel_ior()
  {
    std::wstring nameTest                = L"test_106";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);

    ////////////////////
    // Materials
    ////////////////////

    HRMaterialRef mat1_5  = hrMaterialCreate(L"mat1_5");
    HRMaterialRef mat4    = hrMaterialCreate(L"mat4");
    HRMaterialRef mat16   = hrMaterialCreate(L"mat16");
    HRMaterialRef matGray = hrMaterialCreate(L"matGray");

    hrMaterialOpen(mat1_5, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(mat1_5);

      auto refl    = matNode.append_child(L"reflectivity");

      refl.append_attribute(L"brdf_type").set_value(L"ggx");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"1.0");
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(1.5f);

			VERIFY_XML(matNode);
    }
    hrMaterialClose(mat1_5);

    hrMaterialOpen(mat4, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(mat4);

      auto refl = matNode.append_child(L"reflectivity");

      refl.append_attribute(L"brdf_type").set_value(L"ggx");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"1.0");
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(4.0f);

			VERIFY_XML(matNode);
    }
    hrMaterialClose(mat4);

    hrMaterialOpen(mat16, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(mat16);

      auto refl = matNode.append_child(L"reflectivity");

      refl.append_attribute(L"brdf_type").set_value(L"ggx");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"1.0");
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(16.0f);

			VERIFY_XML(matNode);
    }
    hrMaterialClose(mat16);

    hrMaterialOpen(matGray, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGray);

      auto diff = matNode.append_child(L"diffuse");

      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").append_attribute(L"val").set_value(L"0.3 0.3 0.3");

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matGray);

    ////////////////////
    // Meshes
    ////////////////////
    HRMeshRef sphere1 = HRMeshFromSimpleMesh(L"sphere1", CreateSphere(2.0f, 64), mat1_5.id);
    HRMeshRef sphere2 = HRMeshFromSimpleMesh(L"sphere2", CreateSphere(2.0f, 64), mat4.id);
    HRMeshRef sphere3 = HRMeshFromSimpleMesh(L"sphere3", CreateSphere(2.0f, 64), mat16.id);
    HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(10.0f), matGray.id);

    ////////////////////
    // Light
    ////////////////////
    HRLightRef envir = hrLightCreate(L"sky");

    hrLightOpen(envir, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(envir);

      lightNode.attribute(L"type").set_value(L"sky");

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(1);

      VERIFY_XML(lightNode);
    }
    hrLightClose(envir);


    HRLightRef rectLight = hrLightCreate(L"my_area_light");

    hrLightOpen(rectLight, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(rectLight);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"rect");
      lightNode.attribute(L"distribution").set_value(L"diffuse");

      auto sizeNode = lightNode.append_child(L"size");

      sizeNode.append_attribute(L"half_length").set_value(L"12");
      sizeNode.append_attribute(L"half_width").set_value(L"12");

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(2.0f*IRRADIANCE_TO_RADIANCE);
			VERIFY_XML(lightNode);
    }
    hrLightClose(rectLight);

    ////////////////////
    // Camera
    ////////////////////

    HRCameraRef camRef = hrCameraCreate(L"my camera");

    hrCameraOpen(camRef, HR_WRITE_DISCARD);
    {
      auto camNode = hrCameraParamNode(camRef);

      camNode.append_child(L"fov").text().set(L"30");
      camNode.append_child(L"nearClipPlane").text().set(L"0.01");
      camNode.append_child(L"farClipPlane").text().set(L"100.0");

      camNode.append_child(L"up").text().set(L"0 1 0");
      camNode.append_child(L"position").text().set(L"0 1 12.2");
      camNode.append_child(L"look_at").text().set(L"0 1 0");
    }
    hrCameraClose(camRef);

    ////////////////////
    // Render settings
    ////////////////////

    HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 1024, 500, 256, 256);

    ////////////////////
    // Create scene
    ////////////////////

    HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

    float matrixT[4][4];
    float mTranslate[4][4];
    float mRes[4][4];
    float mRes2[4][4];

    hrSceneOpen(scnRef, HR_WRITE_DISCARD);

    ///////////

    mat4x4_identity(mTranslate);
    mat4x4_identity(mRes);

    mat4x4_translate(mTranslate, 0.0f, 0.0f, 0.0f);
    mat4x4_mul(mRes2, mTranslate, mRes);
    mat4x4_transpose(matrixT, mRes2);

    hrMeshInstance(scnRef, planeRef, &matrixT[0][0]);

    ///////////

    mat4x4_identity(mTranslate);
    mat4x4_identity(mRes);

    mat4x4_translate(mTranslate, -4.25f, 1.0f, 0.0f);
    mat4x4_mul(mRes2, mTranslate, mRes);
    mat4x4_transpose(matrixT, mRes2);

    hrMeshInstance(scnRef, sphere1, &matrixT[0][0]);

    ///////////

    mat4x4_identity(mTranslate);
    mat4x4_identity(mRes);

    mat4x4_translate(mTranslate, 4.25f, 1.0f, 0.0f);
    mat4x4_mul(mRes2, mTranslate, mRes);
    mat4x4_transpose(matrixT, mRes2); //swap rows and columns

    hrMeshInstance(scnRef, sphere3, &matrixT[0][0]);

    ///////////

    mat4x4_identity(mTranslate);
    mat4x4_identity(mRes);

    mat4x4_translate(mTranslate, 0.0f, 1.0f, 0.0f);
    mat4x4_mul(mRes2, mTranslate, mRes);
    mat4x4_transpose(matrixT, mRes2); //swap rows and columns

    hrMeshInstance(scnRef, sphere2, &matrixT[0][0]);

    ///////////

    mat4x4_identity(mTranslate);
    mat4x4_translate(mTranslate, 0, 16.0f, 0.0);
    mat4x4_transpose(matrixT, mTranslate);

    hrLightInstance(scnRef, envir, &matrixT[0][0]);
    hrLightInstance(scnRef, rectLight, &matrixT[0][0]);

    ///////////

    hrSceneClose(scnRef);

    hrFlush(scnRef, renderRef);

    ////////////////////
    // Rendering, save and check image
    ////////////////////

    RenderProgress(renderRef);

    std::filesystem::create_directories(saveRenderFile.parent_path());
    hrRenderSaveFrameBufferLDR(renderRef, saveRenderFile.wstring().c_str());

    return check_images(ws2s(nameTest).c_str(), 1, 60);
  }


  bool test_107_reflect_extrusion()
  {
    std::wstring nameTest                = L"test_107";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);

    ////////////////////
    // Materials
    ////////////////////

	  HRMaterialRef matMaxcolor  = hrMaterialCreate(L"matMaxcolor");
	  HRMaterialRef matLuminance = hrMaterialCreate(L"matLuminance");
	  HRMaterialRef matColored   = hrMaterialCreate(L"matColored");
	  HRMaterialRef matGray      = hrMaterialCreate(L"matGray");

	  hrMaterialOpen(matMaxcolor, HR_WRITE_DISCARD);
	  {
		  auto matNode = hrMaterialParamNode(matMaxcolor);

		  auto diff = matNode.append_child(L"diffuse");

		  diff.append_attribute(L"brdf_type").set_value(L"lambert");
		  diff.append_child(L"color").append_attribute(L"val").set_value(L"0.9 0.9 0.0");

		  auto refl = matNode.append_child(L"reflectivity");

		  refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
		  refl.append_child(L"color").append_attribute(L"val").set_value(L"0.95 0.0 0.0");
		  refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.8");
		  refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
		  refl.append_child(L"fresnel").append_attribute(L"val").set_value(0);

			VERIFY_XML(matNode);
	  }
	  hrMaterialClose(matMaxcolor);

	  hrMaterialOpen(matLuminance, HR_WRITE_DISCARD);
	  {
		  auto matNode = hrMaterialParamNode(matLuminance);

		  auto diff = matNode.append_child(L"diffuse");

		  diff.append_attribute(L"brdf_type").set_value(L"lambert");
		  diff.append_child(L"color").append_attribute(L"val").set_value(L"0.9 0.9 0.0");

		  auto refl = matNode.append_child(L"reflectivity");

		  refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
		  refl.append_child(L"color").append_attribute(L"val").set_value(L"0.95 0.0 0.0");
		  refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.8");
		  refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"luminance");
		  refl.append_child(L"fresnel").append_attribute(L"val").set_value(0);

			VERIFY_XML(matNode);
	  }
	  hrMaterialClose(matLuminance);

	  hrMaterialOpen(matColored, HR_WRITE_DISCARD);
	  {
		  auto matNode = hrMaterialParamNode(matColored);

		  auto diff = matNode.append_child(L"diffuse");

		  diff.append_attribute(L"brdf_type").set_value(L"lambert");
		  diff.append_child(L"color").append_attribute(L"val").set_value(L"0.9 0.9 0.0");

		  auto refl = matNode.append_child(L"reflectivity");

		  refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
		  refl.append_child(L"color").append_attribute(L"val").set_value(L"0.95 0.0 0.0");
		  refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.8");
		  refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"colored");
		  refl.append_child(L"fresnel").append_attribute(L"val").set_value(0);

			VERIFY_XML(matNode);
	  }
	  hrMaterialClose(matColored);

	  hrMaterialOpen(matGray, HR_WRITE_DISCARD);
	  {
		  auto matNode = hrMaterialParamNode(matGray);

		  auto diff = matNode.append_child(L"diffuse");

		  diff.append_attribute(L"brdf_type").set_value(L"lambert");
		  diff.append_child(L"color").append_attribute(L"val").set_value(L"0.25 0.25 0.25");

			VERIFY_XML(matNode);
	  }
	  hrMaterialClose(matGray);

	  ////////////////////
	  // Meshes
	  ////////////////////
	  HRMeshRef sphereMaxcolor = HRMeshFromSimpleMesh(L"sphereMaxcolor", CreateSphere(2.0f, 64), matMaxcolor.id);
	  HRMeshRef sphereLum = HRMeshFromSimpleMesh(L"sphereLum", CreateSphere(2.0f, 64), matLuminance.id);
	  HRMeshRef sphereColored = HRMeshFromSimpleMesh(L"sphereColored", CreateSphere(2.0f, 64), matColored.id);
	  HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(10.0f), matGray.id);

	  ////////////////////
	  // Light
	  ////////////////////

	  HRLightRef rectLight = hrLightCreate(L"my_area_light");

	  hrLightOpen(rectLight, HR_WRITE_DISCARD);
	  {
		  auto lightNode = hrLightParamNode(rectLight);

		  lightNode.attribute(L"type").set_value(L"area");
		  lightNode.attribute(L"shape").set_value(L"rect");
		  lightNode.attribute(L"distribution").set_value(L"diffuse");

      auto sizeNode = lightNode.append_child(L"size");

      sizeNode.append_attribute(L"half_length").set_value(L"12");
      sizeNode.append_attribute(L"half_width").set_value(L"12");

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(2.0f*IRRADIANCE_TO_RADIANCE);
			VERIFY_XML(lightNode);
	  }
	  hrLightClose(rectLight);

	  ////////////////////
	  // Camera
	  ////////////////////

	  HRCameraRef camRef = hrCameraCreate(L"my camera");

	  hrCameraOpen(camRef, HR_WRITE_DISCARD);
	  {
		  auto camNode = hrCameraParamNode(camRef);

		  camNode.append_child(L"fov").text().set(L"45");
		  camNode.append_child(L"nearClipPlane").text().set(L"0.01");
		  camNode.append_child(L"farClipPlane").text().set(L"100.0");

		  camNode.append_child(L"up").text().set(L"0 1 0");
		  camNode.append_child(L"position").text().set(L"0 0 12");
		  camNode.append_child(L"look_at").text().set(L"0 0 0");
	  }
	  hrCameraClose(camRef);

	  ////////////////////
	  // Render settings
	  ////////////////////

	  HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 1024, 768, 256, 2048);

	  ////////////////////
	  // Create scene
	  ////////////////////

	  HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

	  float matrixT[4][4];
	  float mTranslate[4][4];
	  float mRes[4][4];
	  float mRes2[4][4];

	  hrSceneOpen(scnRef, HR_WRITE_DISCARD);

	  ///////////

	  mat4x4_identity(mTranslate);
	  mat4x4_identity(mRes);

	  mat4x4_translate(mTranslate, 0.0f, -1.0f, 0.0f);
	  mat4x4_mul(mRes2, mTranslate, mRes);
	  mat4x4_transpose(matrixT, mRes2);

	  hrMeshInstance(scnRef, planeRef, &matrixT[0][0]);

	  ///////////

	  mat4x4_identity(mTranslate);
	  mat4x4_identity(mRes);

	  mat4x4_translate(mTranslate, -4.25f, 1.0f, 0.0f);
	  mat4x4_mul(mRes2, mTranslate, mRes);
	  mat4x4_transpose(matrixT, mRes2);

	  hrMeshInstance(scnRef, sphereMaxcolor, &matrixT[0][0]);

	  ///////////

	  mat4x4_identity(mTranslate);
	  mat4x4_identity(mRes);

	  mat4x4_translate(mTranslate, 4.25f, 1.0f, 0.0f);
	  mat4x4_mul(mRes2, mTranslate, mRes);
	  mat4x4_transpose(matrixT, mRes2); //swap rows and columns

	  hrMeshInstance(scnRef, sphereColored, &matrixT[0][0]);

	  ///////////

	  mat4x4_identity(mTranslate);
	  mat4x4_identity(mRes);

	  mat4x4_translate(mTranslate, 0.0f, 1.0f, 0.0f);
	  mat4x4_mul(mRes2, mTranslate, mRes);
	  mat4x4_transpose(matrixT, mRes2); //swap rows and columns

	  hrMeshInstance(scnRef, sphereLum, &matrixT[0][0]);

	  ///////////

	  mat4x4_identity(mTranslate);
	  mat4x4_translate(mTranslate, 0, 16.0f, 0.0);
	  mat4x4_transpose(matrixT, mTranslate);

	  hrLightInstance(scnRef, rectLight, &matrixT[0][0]);

	  ///////////

	  hrSceneClose(scnRef);

	  hrFlush(scnRef, renderRef);

    ////////////////////
    // Rendering, save and check image
    ////////////////////

    RenderProgress(renderRef);

    std::filesystem::create_directories(saveRenderFile.parent_path());
    hrRenderSaveFrameBufferLDR(renderRef, saveRenderFile.wstring().c_str());

    return check_images(ws2s(nameTest).c_str(), 1, 60);
  }


  bool test_108_reflect_texture()
  {
    std::wstring nameTest                = L"test_108";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);

    ////////////////////
    // Materials
    ////////////////////

	  HRMaterialRef matR = hrMaterialCreate(L"matR");
	  HRMaterialRef matG = hrMaterialCreate(L"matG");
	  HRMaterialRef matB = hrMaterialCreate(L"matB");
	  HRMaterialRef matGray = hrMaterialCreate(L"matGray");

	  HRTextureNodeRef texChecker = hrTexture2DCreateFromFile(L"data/textures/chess_white.bmp");
	  HRTextureNodeRef texChecker2 = hrTexture2DCreateFromFile(L"data/textures/checker_8x8.gif");

	  hrMaterialOpen(matR, HR_WRITE_DISCARD);
	  {
		  auto matNode = hrMaterialParamNode(matR);

		  auto diff = matNode.append_child(L"diffuse");
		  diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.01 0.01 0.01");

		  auto refl = matNode.append_child(L"reflectivity");
		  refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"0.95 0.05 0.05");
		  refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.95");
		  refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
		  refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
		  refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(8.0f);

      hrTextureBind(texChecker, refl.child(L"color"));

      auto texNode = refl.child(L"color").child(L"texture");
      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 1, 0, 0, 0,
                                  0, 1, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };

      texNode.append_attribute(L"addressing_mode_u").set_value(L"clamp");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"clamp");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

			VERIFY_XML(matNode);
    }
	  hrMaterialClose(matR);

	  hrMaterialOpen(matG, HR_WRITE_DISCARD);
	  {
      auto matNode = hrMaterialParamNode(matG);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.01 0.01 0.01");

      auto refl = matNode.append_child(L"reflectivity");
      refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"0.05 0.95 0.05");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.95");
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(8.0f);

			auto texNode = hrTextureBind(texChecker, refl.child(L"color"));

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 4, 0, 0, 0,
                                  0, 4, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };

      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

			VERIFY_XML(matNode);
    }
	  hrMaterialClose(matG);

	  hrMaterialOpen(matB, HR_WRITE_DISCARD);
	  {
      auto matNode = hrMaterialParamNode(matB);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.01 0.01 0.01");

      auto refl = matNode.append_child(L"reflectivity");
      refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"0.05 0.05 0.95");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.95");
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(8.0f);

			auto texNode = hrTextureBind(texChecker, refl.child(L"color"));

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 4, 0, 0, 0,
                                  0, 4, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };

      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

			VERIFY_XML(matNode);
    }
	  hrMaterialClose(matB);

	  hrMaterialOpen(matGray, HR_WRITE_DISCARD);
	  {
		  auto matNode = hrMaterialParamNode(matGray);

		  auto diff = matNode.append_child(L"diffuse");
		  diff.append_attribute(L"brdf_type").set_value(L"lambert");
		  diff.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 0.5");

			VERIFY_XML(matNode);
	  }
	  hrMaterialClose(matGray);

	  ////////////////////
	  // Meshes
	  ////////////////////
	  HRMeshRef cubeR = HRMeshFromSimpleMesh(L"cubeR", CreateCube(2.0f), matR.id);
	  HRMeshRef sphereG = HRMeshFromSimpleMesh(L"sphereG", CreateSphere(4.0f, 64), matG.id);
	  HRMeshRef torusB = HRMeshFromSimpleMesh(L"torusB", CreateTorus(0.8f, 2.0f, 64, 64), matB.id);
	  //HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(10.0f), matGray.id);
	  HRMeshRef cubeOpen = HRMeshFromSimpleMesh(L"my_cube", CreateCubeOpen(18.0f), matGray.id);

	  ////////////////////
	  // Light
	  ////////////////////

	  HRLightRef rectLight = hrLightCreate(L"my_area_light");

	  hrLightOpen(rectLight, HR_WRITE_DISCARD);
	  {
		  auto lightNode = hrLightParamNode(rectLight);

		  lightNode.attribute(L"type").set_value(L"area");
		  lightNode.attribute(L"shape").set_value(L"rect");
		  lightNode.attribute(L"distribution").set_value(L"diffuse");

      auto sizeNode = lightNode.append_child(L"size");

      sizeNode.append_attribute(L"half_length").set_value(L"8.0");
      sizeNode.append_attribute(L"half_width").set_value(L"8.0");

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(4.0f*IRRADIANCE_TO_RADIANCE);
			VERIFY_XML(lightNode);
	  }
	  hrLightClose(rectLight);

	  ////////////////////
	  // Camera
	  ////////////////////

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

	  ////////////////////
	  // Render settings
	  ////////////////////

	  HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 512, 512, 256, 2048);

	  ////////////////////
	  // Create scene
	  ////////////////////

	  HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

	  float matrixT[4][4];
	  float mTranslate[4][4];
	  float mRes[4][4];
	  float mRes2[4][4];
	  float mRot[4][4];

	  const float DEG_TO_RAD = 0.01745329251f; // float(3.14159265358979323846f) / 180.0f;

	  hrSceneOpen(scnRef, HR_WRITE_DISCARD);

	  ///////////

	  mat4x4_identity(mTranslate);
	  mat4x4_identity(mRes);
	  mat4x4_identity(mRot);

	  mat4x4_translate(mTranslate, 0.0f, -1.0f, 0.0f);
	  mat4x4_rotate_Y(mRot, mRot, 180.0f*DEG_TO_RAD);
	  mat4x4_mul(mRes2, mTranslate, mRot);
	  mat4x4_transpose(matrixT, mRes2);

	  hrMeshInstance(scnRef, cubeOpen, &matrixT[0][0]);

	  ///////////

	  mat4x4_identity(mTranslate);
	  mat4x4_identity(mRes);
	  mat4x4_identity(mRot);

	  mat4x4_translate(mTranslate, -4.0f, 1.0f, 4.0f);
	  mat4x4_rotate_Y(mRot, mRot, 60.0f*DEG_TO_RAD);
	  mat4x4_mul(mRes2, mTranslate, mRot);
	  mat4x4_transpose(matrixT, mRes2);

	  hrMeshInstance(scnRef, cubeR, &matrixT[0][0]);

	  ///////////

	  mat4x4_identity(mTranslate);
	  mat4x4_identity(mRes);
	  mat4x4_identity(mRot);

	  mat4x4_translate(mTranslate, 4.0f, 1.0f, 4.5f);
	  mat4x4_rotate_Y(mRot, mRot, -60.0f*DEG_TO_RAD);
	  mat4x4_rotate_X(mRot, mRot, 80.0f*DEG_TO_RAD);
	  mat4x4_mul(mRes2, mTranslate, mRot);
	  mat4x4_transpose(matrixT, mRes2); //swap rows and columns

	  hrMeshInstance(scnRef, torusB, &matrixT[0][0]);

	  ///////////

	  mat4x4_identity(mTranslate);
	  mat4x4_identity(mRes);

	  mat4x4_translate(mTranslate, 0.0f, 2.0f, -1.0f);
	  mat4x4_mul(mRes2, mTranslate, mRes);
	  mat4x4_transpose(matrixT, mRes2); //swap rows and columns

	  hrMeshInstance(scnRef, sphereG, &matrixT[0][0]);

	  ///////////

	  mat4x4_identity(mTranslate);
	  mat4x4_translate(mTranslate, 0, 12.0f, 0);
	  mat4x4_transpose(matrixT, mTranslate);

	  hrLightInstance(scnRef, rectLight, &matrixT[0][0]);

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


  bool test_109_reflect_glossiness_texture()
  {
    std::wstring nameTest                = L"test_109";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);

    ////////////////////
    // Materials
    ////////////////////

	  HRMaterialRef matGlossy02 = hrMaterialCreate(L"matGlossy02");
    HRMaterialRef matGlossy04 = hrMaterialCreate(L"matGlossy04");
    HRMaterialRef matGlossy06 = hrMaterialCreate(L"matGlossy06");
    HRMaterialRef matGlossy08 = hrMaterialCreate(L"matGlossy08");
    HRMaterialRef matGlossy1  = hrMaterialCreate(L"matGlossy1");
    HRMaterialRef matGlossyGrad = hrMaterialCreate(L"matGlossyGrad");

	  HRMaterialRef matGray     = hrMaterialCreate(L"matGray"); 
    HRMaterialRef matChecker2 = hrMaterialCreate(L"matChecker2");

	  HRTextureNodeRef texChecker2 = hrTexture2DCreateFromFile(L"data/textures/checker_16x16.bmp");
    HRTextureNodeRef texEnv      = hrTexture2DCreateFromFile(L"data/textures/kitchen.hdr");
    HRTextureNodeRef texGrad     = hrTexture2DCreateFromFile(L"data/textures/gradient5.png");


	  hrMaterialOpen(matGlossy02, HR_WRITE_DISCARD);
	  {
      auto matNode = hrMaterialParamNode(matGlossy02);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.01 0.01 0.01");

		  auto refl = matNode.append_child(L"reflectivity");
		  refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
		  refl.append_child(L"color").append_attribute(L"val").set_value(L"0.95 0.95 0.95");
		  refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.2");
		  refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
		  refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
		  refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(8.0f);

			auto texNode = hrTextureBind(texChecker2, refl.child(L"glossiness"));

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 1, 0, 0, 0,
                                  0, 1, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };

      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

			VERIFY_XML(matNode);
	  }
	  hrMaterialClose(matGlossy02);

    hrMaterialOpen(matGlossy04, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGlossy04);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.01 0.01 0.01");

      auto refl = matNode.append_child(L"reflectivity");
      refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"0.95 0.95 0.95");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.4");
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(8.0f);

      auto texNode = hrTextureBind(texChecker2, refl.child(L"glossiness"));

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 1, 0, 0, 0,
                                  0, 1, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };

      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

      VERIFY_XML(matNode);
    }
    hrMaterialClose(matGlossy04);

    hrMaterialOpen(matGlossy06, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGlossy06);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.01 0.01 0.01");

      auto refl = matNode.append_child(L"reflectivity");
      refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"0.95 0.95 0.95");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.6");
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(8.0f);

      auto texNode = hrTextureBind(texChecker2, refl.child(L"glossiness"));

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1 };

      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

      VERIFY_XML(matNode);
    }
    hrMaterialClose(matGlossy06);

    hrMaterialOpen(matGlossy08, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGlossy08);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.01 0.01 0.01");

      auto refl = matNode.append_child(L"reflectivity");
      refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"0.95 0.95 0.95");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.8");
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(8.0f);

      auto texNode = hrTextureBind(texChecker2, refl.child(L"glossiness"));

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1 };

      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

      VERIFY_XML(matNode);
    }
    hrMaterialClose(matGlossy08);

    hrMaterialOpen(matGlossy1, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGlossy1);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.01 0.01 0.01");

      auto refl = matNode.append_child(L"reflectivity");
      refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"0.95 0.95 0.95");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"1.0");
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(8.0f);

      auto texNode = hrTextureBind(texChecker2, refl.child(L"glossiness"));

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1 };

      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

      VERIFY_XML(matNode);
    }
    hrMaterialClose(matGlossy1);

    hrMaterialOpen(matGlossyGrad, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGlossyGrad);

      auto refl = matNode.append_child(L"reflectivity");
      refl.append_attribute(L"brdf_type").set_value(L"phong");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"1.0");
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(0);
      refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(8.0f);

      auto texNode = hrTextureBind(texGrad, refl.child(L"glossiness"));

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 1, 0, 0, 0,
                                  0, -1, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };

      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

      VERIFY_XML(matNode);
    }
    hrMaterialClose(matGlossyGrad);

	  hrMaterialOpen(matGray, HR_WRITE_DISCARD);
	  {
		  auto matNode = hrMaterialParamNode(matGray);

		  auto diff = matNode.append_child(L"diffuse");
		  diff.append_attribute(L"brdf_type").set_value(L"lambert");
		  diff.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 0.5");

			VERIFY_XML(matNode);
	  }
	  hrMaterialClose(matGray);

    hrMaterialOpen(matChecker2, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matChecker2);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.7 0.7 0.7");
      color.append_attribute(L"tex_apply_mode").set_value(L"replace");

      auto texNode = hrTextureBind(texChecker2, color);

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 2, 0, 0, 0,
                                  0, 2, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };
      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

      VERIFY_XML(matNode);
    }
    hrMaterialClose(matChecker2);

	  ////////////////////
	  // Meshes
	  ////////////////////
	  
    HRMeshRef sphere01 = HRMeshFromSimpleMesh(L"sphere1", CreateSphere(4.0f, 64), matGlossy02.id);
    HRMeshRef sphere02 = HRMeshFromSimpleMesh(L"sphere2", CreateSphere(4.0f, 64), matGlossy04.id);
    HRMeshRef sphere03 = HRMeshFromSimpleMesh(L"sphere3", CreateSphere(4.0f, 64), matGlossy06.id);
    HRMeshRef sphere04 = HRMeshFromSimpleMesh(L"sphere4", CreateSphere(4.0f, 64), matGlossy08.id);
    HRMeshRef sphere05 = HRMeshFromSimpleMesh(L"sphere5", CreateSphere(4.0f, 64), matGlossy1.id);
    HRMeshRef plane01 = HRMeshFromSimpleMesh(L"Plane", CreatePlane(5.0f), matGlossyGrad.id);
    HRMeshRef planeFloor = HRMeshFromSimpleMesh(L"Floor", CreatePlane(20.0f), matChecker2.id);


	  ////////////////////
	  // Light
	  ////////////////////
    HRLightRef sky = hrLightCreate(L"sky");
    
    hrLightOpen(sky, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(sky);
    
      lightNode.attribute(L"type").set_value(L"sky");
      lightNode.attribute(L"distribution").set_value(L"map");
    
      auto intensityNode = lightNode.append_child(L"intensity");
    
      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"0.5");
    
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

	  HRLightRef rectLight = hrLightCreate(L"my_area_light");

	  hrLightOpen(rectLight, HR_WRITE_DISCARD);
	  {
		  auto lightNode = hrLightParamNode(rectLight);

		  lightNode.attribute(L"type").set_value(L"area");
		  lightNode.attribute(L"shape").set_value(L"rect");
		  lightNode.attribute(L"distribution").set_value(L"diffuse");

      auto sizeNode = lightNode.append_child(L"size");

      sizeNode.append_attribute(L"half_length").set_value(L"8.0");
      sizeNode.append_attribute(L"half_width").set_value(L"8.0");

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(1.0f*IRRADIANCE_TO_RADIANCE);
			VERIFY_XML(lightNode);
	  }
	  hrLightClose(rectLight);

	  ////////////////////
	  // Camera
	  ////////////////////

	  HRCameraRef camRef = hrCameraCreate(L"my camera");

	  hrCameraOpen(camRef, HR_WRITE_DISCARD);
	  {
		  auto camNode = hrCameraParamNode(camRef);

		  camNode.append_child(L"fov").text().set(L"45");
		  camNode.append_child(L"nearClipPlane").text().set(L"0.01");
		  camNode.append_child(L"farClipPlane").text().set(L"100.0");

		  camNode.append_child(L"up").text().set(L"0 1 0");
		  camNode.append_child(L"position").text().set(L"0 30 15");
		  camNode.append_child(L"look_at").text().set(L"0 0 0");
	  }
	  hrCameraClose(camRef);

	  ////////////////////
	  // Render settings
	  ////////////////////

	  HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 1024, 768, 512, 4096);

	  ////////////////////
	  // Create scene
	  ////////////////////

	  HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

	  float matrixT[4][4];
	  float mTranslate[4][4];
	  float mRes[4][4];
	  float mRes2[4][4];
	  float mRot[4][4];

	  const float DEG_TO_RAD = 0.01745329251f; // float(3.14159265358979323846f) / 180.0f;

	  hrSceneOpen(scnRef, HR_WRITE_DISCARD);

	  ///////////

    mat4x4_identity(mTranslate);
    mat4x4_identity(mRes);

    mat4x4_translate(mTranslate, -10.0f, 4.0f, -5.0f);
    mat4x4_mul(mRes2, mTranslate, mRes);
    mat4x4_transpose(matrixT, mRes2); //swap rows and columns

    hrMeshInstance(scnRef, sphere01, &matrixT[0][0]);

    ///////////

    mat4x4_identity(mTranslate);
    mat4x4_identity(mRes);

    mat4x4_translate(mTranslate, 0.0f, 4.0f, -5.0f);
    mat4x4_mul(mRes2, mTranslate, mRes);
    mat4x4_transpose(matrixT, mRes2); //swap rows and columns

    hrMeshInstance(scnRef, sphere02, &matrixT[0][0]);

    ///////////

    mat4x4_identity(mTranslate);
    mat4x4_identity(mRes);

    mat4x4_translate(mTranslate, 10.0f, 4.0f, -5.0f);
    mat4x4_mul(mRes2, mTranslate, mRes);
    mat4x4_transpose(matrixT, mRes2); //swap rows and columns

    hrMeshInstance(scnRef, sphere03, &matrixT[0][0]);

    ///////////


    mat4x4_identity(mTranslate);
    mat4x4_identity(mRes);

    mat4x4_translate(mTranslate, -10.0f, 4.0f, 5.0f);
    mat4x4_mul(mRes2, mTranslate, mRes);
    mat4x4_transpose(matrixT, mRes2); //swap rows and columns

    hrMeshInstance(scnRef, sphere04, &matrixT[0][0]);

    ///////////

    mat4x4_identity(mTranslate);
    mat4x4_identity(mRes);

    mat4x4_translate(mTranslate, 0.0f, 4.0f, 5.0f);
    mat4x4_mul(mRes2, mTranslate, mRes);
    mat4x4_transpose(matrixT, mRes2); //swap rows and columns

    hrMeshInstance(scnRef, sphere05, &matrixT[0][0]);

    ///////////

    mat4x4_identity(mTranslate);
    mat4x4_identity(mRes);

    mat4x4_translate(mTranslate, 10.0f, 0.1f, 5.0f);
    mat4x4_mul(mRes2, mTranslate, mRes);
    mat4x4_transpose(matrixT, mRes2); //swap rows and columns

    hrMeshInstance(scnRef, plane01, &matrixT[0][0]);

	  ///////////

    mat4x4_identity(mTranslate);
    mat4x4_identity(mRes);
    mat4x4_identity(mRot);

    mat4x4_translate(mTranslate, 0.0f, 0.0f, 0.0f);
    mat4x4_rotate_Y(mRot, mRot, 180.0f*DEG_TO_RAD);
    mat4x4_mul(mRes2, mTranslate, mRot);
    mat4x4_transpose(matrixT, mRes2);

    hrMeshInstance(scnRef, planeFloor, &matrixT[0][0]);

    ///////////

	  mat4x4_identity(mTranslate);
	  mat4x4_translate(mTranslate, 0, 50.0f, 0);
	  mat4x4_transpose(matrixT, mTranslate);
    
	  hrLightInstance(scnRef, rectLight, &matrixT[0][0]);

	  ///////////

    mat4x4_identity(mTranslate);
    hrLightInstance(scnRef, sky, &matrixT[0][0]);

    ///////////

	  hrSceneClose(scnRef);

	  hrFlush(scnRef, renderRef);

    ////////////////////
    // Rendering, save and check image
    ////////////////////

    RenderProgress(renderRef);

    std::filesystem::create_directories(saveRenderFile.parent_path());
    hrRenderSaveFrameBufferLDR(renderRef, saveRenderFile.wstring().c_str());

    return check_images(ws2s(nameTest).c_str(), 1, 60);
  }


  bool test_110_texture_sampler()
  {
    std::wstring nameTest                = L"test_110";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);

    ////////////////////
    // Materials
    ////////////////////

    HRMaterialRef matR = hrMaterialCreate(L"matR");
    HRMaterialRef matG = hrMaterialCreate(L"matG");
    HRMaterialRef matB = hrMaterialCreate(L"matB");
    HRMaterialRef matGray = hrMaterialCreate(L"matGray");

    HRTextureNodeRef texChecker = hrTexture2DCreateFromFile(L"data/textures/chess_white.bmp");

    hrMaterialOpen(matR, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matR);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.95 0.05 0.05");
      color.append_attribute(L"tex_apply_mode").set_value(L"multiply");

			auto texNode = hrTextureBind(texChecker, color);

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 2, 0, 0, -0.5,
                                  0, 2, 0, -0.5,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };

      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matR);

    hrMaterialOpen(matG, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matG);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.05 0.95 0.05");

			auto texNode = hrTextureBind(texChecker, color);

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 4, 0, 0, -0.5,
                                  0, 4, 0, -0.5,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };

      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matG);

    hrMaterialOpen(matB, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matB);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.05 0.05 0.95");

			auto texNode = hrTextureBind(texChecker, color);

      texNode.append_attribute(L"matrix");
      float sqrtOfTwo = static_cast<float>(2.0f);
      float samplerMatrix[16] = { 4.0f * sqrtOfTwo, -4.0f * sqrtOfTwo,  0.0f,                     0.5f,
                                  4.0f * sqrtOfTwo,  4.0f * sqrtOfTwo,  0.0f, -4.0f * sqrtOfTwo + 0.5f,
                                  0.0f,              0.0f,              1.0f,                     0.0f,
                                  0.0f,              0.0f,              0.0f,                     1.0f};

      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matB);

    hrMaterialOpen(matGray, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGray);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 0.5");

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matGray);

    ////////////////////
    // Meshes
    ////////////////////
    HRMeshRef sphereR  = HRMeshFromSimpleMesh(L"sphereR", CreateSphere(2.0f, 64), matR.id);
    HRMeshRef sphereG  = HRMeshFromSimpleMesh(L"sphereG", CreateSphere(2.0f, 64), matG.id);
    HRMeshRef sphereB  = HRMeshFromSimpleMesh(L"sphereB", CreateSphere(2.0f, 64), matB.id);
    HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(10.0f), matGray.id);

    ////////////////////
    // Light
    ////////////////////

    HRLightRef rectLight = hrLightCreate(L"my_area_light");

    hrLightOpen(rectLight, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(rectLight);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"rect");
      lightNode.attribute(L"distribution").set_value(L"diffuse");

      auto sizeNode = lightNode.append_child(L"size");

      sizeNode.append_attribute(L"half_length").set_value(L"8.0");
      sizeNode.append_attribute(L"half_width").set_value(L"8.0");

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(4.0*IRRADIANCE_TO_RADIANCE);
			VERIFY_XML(lightNode);
    }
    hrLightClose(rectLight);

    ////////////////////
    // Camera
    ////////////////////

    HRCameraRef camRef = hrCameraCreate(L"my camera");

    hrCameraOpen(camRef, HR_WRITE_DISCARD);
    {
      auto camNode = hrCameraParamNode(camRef);

      camNode.append_child(L"fov").text().set(L"45");
      camNode.append_child(L"nearClipPlane").text().set(L"0.01");
      camNode.append_child(L"farClipPlane").text().set(L"100.0");

      camNode.append_child(L"up").text().set(L"0 1 0");
      camNode.append_child(L"position").text().set(L"0 10 10");
      camNode.append_child(L"look_at").text().set(L"0 0 0");
    }
    hrCameraClose(camRef);

    ////////////////////
    // Render settings
    ////////////////////

    HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 1024, 768, 256, 2048);

    ////////////////////
    // Create scene
    ////////////////////

    HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

    float matrixT[4][4];
    float mTranslate[4][4];
    float mRes[4][4];
    float mRes2[4][4];

    hrSceneOpen(scnRef, HR_WRITE_DISCARD);

    ///////////

    mat4x4_identity(mTranslate);
    mat4x4_identity(mRes);

    mat4x4_translate(mTranslate, 0.0f, -1.0f, 0.0f);
    mat4x4_mul(mRes2, mTranslate, mRes);
    mat4x4_transpose(matrixT, mRes2);

    hrMeshInstance(scnRef, planeRef, &matrixT[0][0]);

    ///////////

    mat4x4_identity(mTranslate);
    mat4x4_identity(mRes);

    mat4x4_translate(mTranslate, -4.25f, 1.0f, 0.0f);
    mat4x4_mul(mRes2, mTranslate, mRes);
    mat4x4_transpose(matrixT, mRes2);

    hrMeshInstance(scnRef, sphereR, &matrixT[0][0]);

    ///////////

    mat4x4_identity(mTranslate);
    mat4x4_identity(mRes);

    mat4x4_translate(mTranslate, 4.25f, 1.0f, 0.0f);
    mat4x4_mul(mRes2, mTranslate, mRes);
    mat4x4_transpose(matrixT, mRes2); //swap rows and columns

    hrMeshInstance(scnRef, sphereB, &matrixT[0][0]);

    ///////////

    mat4x4_identity(mTranslate);
    mat4x4_identity(mRes);

    mat4x4_translate(mTranslate, 0.0f, 1.0f, 0.0f);
    mat4x4_mul(mRes2, mTranslate, mRes);
    mat4x4_transpose(matrixT, mRes2); //swap rows and columns

    hrMeshInstance(scnRef, sphereG, &matrixT[0][0]);

    ///////////

    mat4x4_identity(mTranslate);
    mat4x4_translate(mTranslate, 0, 16.0f, 0.0);
    mat4x4_transpose(matrixT, mTranslate);

    hrLightInstance(scnRef, rectLight, &matrixT[0][0]);

    ///////////

    hrSceneClose(scnRef);

    hrFlush(scnRef, renderRef);
    
    ////////////////////
    // Rendering, save and check image
    ////////////////////

    RenderProgress(renderRef);

    std::filesystem::create_directories(saveRenderFile.parent_path());
    hrRenderSaveFrameBufferLDR(renderRef, saveRenderFile.wstring().c_str());

    return check_images(ws2s(nameTest).c_str(), 1, 60);
  }


  bool test_111_glossiness_texture_sampler()
  {
    std::wstring nameTest                = L"test_111";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);

    ////////////////////
    // Materials
    ////////////////////

    HRMaterialRef matR = hrMaterialCreate(L"matR");
    HRMaterialRef matG = hrMaterialCreate(L"matG");
    HRMaterialRef matB = hrMaterialCreate(L"matB");
    HRMaterialRef matGray = hrMaterialCreate(L"matGray");

    HRTextureNodeRef texChecker = hrTexture2DCreateFromFile(L"data/textures/chess_white.bmp");
    
    hrMaterialOpen(matR, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matR);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.05 0.05 0.05");

      auto refl = matNode.append_child(L"reflectivity");
      refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"0.95 0.05 0.05");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.3");
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(8.0f);

			auto texNode = hrTextureBind(texChecker, refl.child(L"glossiness"));

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 2, 0, 0, -0.5,
                                  0, 2, 0, -0.5,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };

      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matR);

    hrMaterialOpen(matG, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matG);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.05 0.05 0.05");

      auto refl = matNode.append_child(L"reflectivity");
      refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"0.05 0.95 0.05");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.3");
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(8.0f);

			auto texNode = hrTextureBind(texChecker, refl.child(L"glossiness"));

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 4, 0, 0, -0.5,
                                  0, 4, 0, -0.5,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };
      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matG);

    hrMaterialOpen(matB, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matB);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.05 0.05 0.05");

      auto refl = matNode.append_child(L"reflectivity");
      refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"0.05 0.05 0.95");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.3");
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(8.0f);

			auto texNode = hrTextureBind(texChecker, refl.child(L"glossiness"));

      texNode.append_attribute(L"matrix");
      float sqrtOfTwo = static_cast<float>(2.0f);
      float samplerMatrix[16] = { 4.0f * sqrtOfTwo, -4.0f * sqrtOfTwo,  0.0f,                     0.5f,
                                  4.0f * sqrtOfTwo,  4.0f * sqrtOfTwo,  0.0f, -4.0f * sqrtOfTwo + 0.5f,
                                  0.0f,              0.0f,              1.0f,                     0.0f,
                                  0.0f,              0.0f,              0.0f,                     1.0f};
      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matB);


    hrMaterialOpen(matGray, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGray);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 0.5");

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matGray);

    ////////////////////
    // Meshes
    ////////////////////
    HRMeshRef cubeR = HRMeshFromSimpleMesh(L"cubeR", CreateCube(2.0f), matR.id);
    HRMeshRef sphereG = HRMeshFromSimpleMesh(L"sphereG", CreateSphere(4.0f, 64), matG.id);
    HRMeshRef torusB = HRMeshFromSimpleMesh(L"torusB", CreateTorus(0.8f, 2.0f, 64, 64), matB.id);
    //HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(10.0f), matGray.id);
    HRMeshRef cubeOpen = HRMeshFromSimpleMesh(L"my_cube", CreateCubeOpen(18.0f), matGray.id);

    ////////////////////
    // Light
    ////////////////////

    HRLightRef rectLight = hrLightCreate(L"my_area_light");

    hrLightOpen(rectLight, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(rectLight);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"rect");
      lightNode.attribute(L"distribution").set_value(L"diffuse");

      auto sizeNode = lightNode.append_child(L"size");

      sizeNode.append_attribute(L"half_length").set_value(L"8.0");
      sizeNode.append_attribute(L"half_width").set_value(L"8.0");

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(1.0f*IRRADIANCE_TO_RADIANCE);
			VERIFY_XML(lightNode);
    }
    hrLightClose(rectLight);

    ////////////////////
    // Camera
    ////////////////////

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

    ////////////////////
    // Render settings
    ////////////////////

    HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 512, 512, 256, 2048);

    ////////////////////
    // Create scene
    ////////////////////

    HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

    float matrixT[4][4];
    float mTranslate[4][4];
    float mRes[4][4];
    float mRes2[4][4];
    float mRot[4][4];

    const float DEG_TO_RAD = 0.01745329251f; // float(3.14159265358979323846f) / 180.0f;

    hrSceneOpen(scnRef, HR_WRITE_DISCARD);

    ///////////

    mat4x4_identity(mTranslate);
    mat4x4_identity(mRes);
    mat4x4_identity(mRot);

    mat4x4_translate(mTranslate, 0.0f, 17.0f, 0.0f);
    mat4x4_rotate_Y(mRot, mRot, 180.0f*DEG_TO_RAD);
    mat4x4_mul(mRes2, mTranslate, mRot);
    mat4x4_transpose(matrixT, mRes2);

    hrMeshInstance(scnRef, cubeOpen, &matrixT[0][0]);

    ///////////

    mat4x4_identity(mTranslate);
    mat4x4_identity(mRes);
    mat4x4_identity(mRot);

    mat4x4_translate(mTranslate, -4.0f, 1.0f, 4.0f);
    mat4x4_rotate_Y(mRot, mRot, 60.0f*DEG_TO_RAD);
    mat4x4_mul(mRes2, mTranslate, mRot);
    mat4x4_transpose(matrixT, mRes2);

    hrMeshInstance(scnRef, cubeR, &matrixT[0][0]);

    ///////////

    mat4x4_identity(mTranslate);
    mat4x4_identity(mRes);
    mat4x4_identity(mRot);

    mat4x4_translate(mTranslate, 4.0f, 1.0f, 4.5f);
    mat4x4_rotate_Y(mRot, mRot, -60.0f*DEG_TO_RAD);
    mat4x4_rotate_X(mRot, mRot, 80.0f*DEG_TO_RAD);
    mat4x4_mul(mRes2, mTranslate, mRot);
    mat4x4_transpose(matrixT, mRes2); //swap rows and columns

    hrMeshInstance(scnRef, torusB, &matrixT[0][0]);

    ///////////

    mat4x4_identity(mTranslate);
    mat4x4_identity(mRes);

    mat4x4_translate(mTranslate, 0.0f, 2.0f, -1.0f);
    mat4x4_mul(mRes2, mTranslate, mRes);
    mat4x4_transpose(matrixT, mRes2); //swap rows and columns

    hrMeshInstance(scnRef, sphereG, &matrixT[0][0]);

    ///////////

    mat4x4_identity(mTranslate);
    mat4x4_translate(mTranslate, 0.0f, 12.0f, 0.0f);
    mat4x4_transpose(matrixT, mTranslate);

    hrLightInstance(scnRef, rectLight, &matrixT[0][0]);

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


  bool test_112_transparency()
  {
    std::wstring nameTest                = L"test_112";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);

    ////////////////////
    // Materials
    ////////////////////

    HRMaterialRef matGlass  = hrMaterialCreate(L"matGlass");
    HRMaterialRef matGlossy = hrMaterialCreate(L"matGlossy");
    HRMaterialRef matNoRefl = hrMaterialCreate(L"matNoRefl");
    HRMaterialRef matBG     = hrMaterialCreate(L"matBG");
    HRMaterialRef matGray   = hrMaterialCreate(L"matGray");

    HRTextureNodeRef texChecker = hrTexture2DCreateFromFile(L"data/textures/chess_white.bmp");

    hrMaterialOpen(matGlass, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGlass);

      auto refl = matNode.append_child(L"reflectivity");

      refl.append_attribute(L"brdf_type").set_value(L"phong");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(1.0f);
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(1.5f);

      auto transp = matNode.append_child(L"transparency");

      transp.append_attribute(L"brdf_type").set_value(L"phong");
      transp.append_child(L"color").append_attribute(L"val").set_value(L"1.0 0.5 0.5");
      transp.append_child(L"glossiness").append_attribute(L"val").set_value(1.0f);
      transp.append_child(L"thin_walled").append_attribute(L"val").set_value(0);
      transp.append_child(L"fog_color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      transp.append_child(L"fog_multiplier").append_attribute(L"val").set_value(1.0f);
      transp.append_child(L"ior").append_attribute(L"val").set_value(1.5f);

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matGlass);

    hrMaterialOpen(matGlossy, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGlossy);

      auto refl = matNode.append_child(L"reflectivity");

      refl.append_attribute(L"brdf_type").set_value(L"phong");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(1.0f);
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(1.5f);

      auto transp = matNode.append_child(L"transparency");

      transp.append_attribute(L"brdf_type").set_value(L"phong");
      transp.append_child(L"color").append_attribute(L"val").set_value(L"0.5 1.0 0.5");
      transp.append_child(L"glossiness").append_attribute(L"val").set_value(0.9f);
      transp.append_child(L"thin_walled").append_attribute(L"val").set_value(0);
      transp.append_child(L"fog_color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      transp.append_child(L"fog_multiplier").append_attribute(L"val").set_value(1.0f);
      transp.append_child(L"ior").append_attribute(L"val").set_value(1.5f);

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matGlossy);

    hrMaterialOpen(matNoRefl, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matNoRefl);

      auto transp = matNode.append_child(L"transparency");

      transp.append_attribute(L"brdf_type").set_value(L"phong");
      transp.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 1.0");
      transp.append_child(L"glossiness").append_attribute(L"val").set_value(1.0f);
      transp.append_child(L"thin_walled").append_attribute(L"val").set_value(0);
      transp.append_child(L"fog_color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      transp.append_child(L"fog_multiplier").append_attribute(L"val").set_value(1.0f);
      transp.append_child(L"ior").append_attribute(L"val").set_value(1.5f);

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matNoRefl);

    hrMaterialOpen(matBG, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matBG);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.25 0.25 1.0");
      color.append_attribute(L"tex_apply_mode").set_value(L"multiply");

			auto texNode = hrTextureBind(texChecker, color);

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 32, 0, 0, 0,
                                  0, 16, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };
      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matBG);

    hrMaterialOpen(matGray, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGray);

      auto diff = matNode.append_child(L"diffuse");

      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      color.append_attribute(L"tex_apply_mode").set_value(L"replace");

			auto texNode = hrTextureBind(texChecker, color);

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 16, 0, 0, 0,
                                  0, 16, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };
      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matGray);

    ////////////////////
    // Meshes
    ////////////////////
    HRMeshRef box1     = HRMeshFromSimpleMesh(L"box1",     CreateCube(1.0f),   matGlass.id);
    HRMeshRef box2     = HRMeshFromSimpleMesh(L"box2",     CreateCube(1.0f),   matGlossy.id);
    HRMeshRef box3     = HRMeshFromSimpleMesh(L"box3",     CreateCube(1.0f),   matNoRefl.id);
    HRMeshRef boxBG    = HRMeshFromSimpleMesh(L"boxBG",    CreateCube(1.0f),   matBG.id);
    HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), matGray.id);

    ////////////////////
    // Light
    ////////////////////

    HRLightRef rectLight = hrLightCreate(L"my_area_light");

    hrLightOpen(rectLight, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(rectLight);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"rect");
      lightNode.attribute(L"distribution").set_value(L"diffuse");

      auto sizeNode = lightNode.append_child(L"size");

      sizeNode.append_attribute(L"half_length").set_value(L"10.0");
      sizeNode.append_attribute(L"half_width").set_value(L"10.0");

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(2.0f*IRRADIANCE_TO_RADIANCE);
			VERIFY_XML(lightNode);
    }
    hrLightClose(rectLight);

    ////////////////////
    // Camera
    ////////////////////

    HRCameraRef camRef = hrCameraCreate(L"my camera");

    hrCameraOpen(camRef, HR_WRITE_DISCARD);
    {
      auto camNode = hrCameraParamNode(camRef);

      camNode.append_child(L"fov").text().set(L"45");
      camNode.append_child(L"nearClipPlane").text().set(L"0.01");
      camNode.append_child(L"farClipPlane").text().set(L"100.0");

      camNode.append_child(L"up").text().set(L"0 1 0");
      camNode.append_child(L"position").text().set(L"0 1 16");
      camNode.append_child(L"look_at").text().set(L"0 1 0");
    }
    hrCameraClose(camRef);

    ////////////////////
    // Render settings
    ////////////////////

    HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 1024, 768, 256, 2048);

    hrRenderOpen(renderRef, HR_OPEN_EXISTING);
    {
      auto settingsNode = hrRenderParamNode(renderRef);
      settingsNode.force_child(L"trace_depth").text() = 10;
    }
    hrRenderClose(renderRef);

    ////////////////////
    // Create scene
    ////////////////////

    HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

    using namespace LiteMath;

    float4x4 mRot;
    float4x4 mTranslate;
    float4x4 mScale;
    float4x4 mRes;

    hrSceneOpen(scnRef, HR_WRITE_DISCARD);

    ///////////
    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0.0f, -1.0f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, planeRef, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(-4.25f, 1.25f, 4.0f));
    mScale = scale4x4(float3(2.0f, 2.0f, 0.5f));
    mRes = mul(mScale, mRes);
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, box1, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0.0f, 1.25f, 4.0f));
    mScale = scale4x4(float3(2.0f, 2.0f, 0.5f));
    mRes = mul(mScale, mRes);
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, box2, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(4.25f, 1.25f, 4.0f));
    mScale = scale4x4(float3(2.0f, 2.0f, 0.5f));
    mRes = mul(mScale, mRes);
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, box3, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();
    mRot.identity();

    const float DEG_TO_RAD = 0.01745329251f; // float(3.14159265358979323846f) / 180.0f;

    mTranslate = translate4x4(float3(0.0f, 1.0f, -4.0f));
    mScale = scale4x4(float3(16.0f, 8.0f, 0.5f));
    mRes = mul(mScale, mRes);
    mRes = mul(mTranslate, mRes);
    
    hrMeshInstance(scnRef, boxBG, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0, 15.0f, 0.0));
    mRes = mul(mTranslate, mRes);
  
    hrLightInstance(scnRef, rectLight, mRes.L());

    ///////////

    hrSceneClose(scnRef);

    hrFlush(scnRef, renderRef);
    
    ////////////////////
    // Rendering, save and check image
    ////////////////////

    RenderProgress(renderRef);

    std::filesystem::create_directories(saveRenderFile.parent_path());
    hrRenderSaveFrameBufferLDR(renderRef, saveRenderFile.wstring().c_str());

    return check_images(ws2s(nameTest).c_str(), 1, 60);
  }


  bool test_113_transparency_ior()
  {
    std::wstring nameTest                = L"test_113";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);

    ////////////////////
    // Materials
    ////////////////////

    HRMaterialRef mat105 = hrMaterialCreate(L"mat10");
    HRMaterialRef mat133 = hrMaterialCreate(L"mat133");
    HRMaterialRef mat241 = hrMaterialCreate(L"mat241");
    HRMaterialRef matBG = hrMaterialCreate(L"matBG");
    HRMaterialRef matGray = hrMaterialCreate(L"matGray");

    HRTextureNodeRef texChecker = hrTexture2DCreateFromFile(L"data/textures/chess_white.bmp");

    hrMaterialOpen(mat105, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(mat105);

      auto refl = matNode.append_child(L"reflectivity");

      refl.append_attribute(L"brdf_type").set_value(L"phong");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(1.0f);
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(1.05f);

      auto transp = matNode.append_child(L"transparency");

      transp.append_attribute(L"brdf_type").set_value(L"phong");
      transp.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      transp.append_child(L"glossiness").append_attribute(L"val").set_value(1.0);
      transp.append_child(L"thin_walled").append_attribute(L"val").set_value(0);
      transp.append_child(L"fog_color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      transp.append_child(L"fog_multiplier").append_attribute(L"val").set_value(1.0f);
      transp.append_child(L"ior").append_attribute(L"val").set_value(1.05f);

			VERIFY_XML(matNode);
    }
    hrMaterialClose(mat105);

    hrMaterialOpen(mat133, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(mat133);

      auto refl = matNode.append_child(L"reflectivity");

      refl.append_attribute(L"brdf_type").set_value(L"phong");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(1.0f);
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(1.33f);

      auto transp = matNode.append_child(L"transparency");

      transp.append_attribute(L"brdf_type").set_value(L"phong");
      transp.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      transp.append_child(L"glossiness").append_attribute(L"val").set_value(1.0f);
      transp.append_child(L"thin_walled").append_attribute(L"val").set_value(0);
      transp.append_child(L"fog_color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      transp.append_child(L"fog_multiplier").append_attribute(L"val").set_value(1.0f);
      transp.append_child(L"ior").append_attribute(L"val").set_value(1.33f);

			VERIFY_XML(matNode);
    }
    hrMaterialClose(mat133);

    hrMaterialOpen(mat241, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(mat241);

      auto refl = matNode.append_child(L"reflectivity");

      refl.append_attribute(L"brdf_type").set_value(L"phong");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(1.0f);
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(2.41f);

      auto transp = matNode.append_child(L"transparency");

      transp.append_attribute(L"brdf_type").set_value(L"phong");
      transp.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      transp.append_child(L"glossiness").append_attribute(L"val").set_value(1.0f);
      transp.append_child(L"thin_walled").append_attribute(L"val").set_value(0);
      transp.append_child(L"fog_color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      transp.append_child(L"fog_multiplier").append_attribute(L"val").set_value(1.0f);
      transp.append_child(L"ior").append_attribute(L"val").set_value(2.41f);

			VERIFY_XML(matNode);
    }
    hrMaterialClose(mat241);

    hrMaterialOpen(matBG, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matBG);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.25 0.25 1.0");
      color.append_attribute(L"tex_apply_mode").set_value(L"multiply");

			auto texNode = hrTextureBind(texChecker, color);

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 32, 0, 0, 0,
                                  0, 16, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };
      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matBG);

    hrMaterialOpen(matGray, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGray);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      color.append_attribute(L"tex_apply_mode").set_value(L"replace");

			auto texNode = hrTextureBind(texChecker, color);

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 16, 0, 0, 0,
                                  0, 16, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };
      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matGray);

    ////////////////////
    // Meshes
    ////////////////////
    HRMeshRef sph1 = HRMeshFromSimpleMesh(L"sph1", CreateSphere(2.0f, 64), mat105.id);
    HRMeshRef sph2 = HRMeshFromSimpleMesh(L"sph2", CreateSphere(2.0f, 64), mat133.id);
    HRMeshRef sph3 = HRMeshFromSimpleMesh(L"sph3", CreateSphere(2.0f, 64), mat241.id);
    HRMeshRef boxBG = HRMeshFromSimpleMesh(L"boxBG", CreateCube(1.0f), matBG.id);
    HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), matGray.id);

    ////////////////////
    // Light
    ////////////////////

    HRLightRef rectLight = hrLightCreate(L"my_area_light");

    hrLightOpen(rectLight, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(rectLight);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"rect");
      lightNode.attribute(L"distribution").set_value(L"diffuse");

      auto sizeNode = lightNode.append_child(L"size");

      sizeNode.append_attribute(L"half_length").set_value(L"10.0");
      sizeNode.append_attribute(L"half_width").set_value(L"10.0");

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(2.0f*IRRADIANCE_TO_RADIANCE);
			VERIFY_XML(lightNode);
    }
    hrLightClose(rectLight);

    ////////////////////
    // Camera
    ////////////////////

    HRCameraRef camRef = hrCameraCreate(L"my camera");

    hrCameraOpen(camRef, HR_WRITE_DISCARD);
    {
      auto camNode = hrCameraParamNode(camRef);

      camNode.append_child(L"fov").text().set(L"45");
      camNode.append_child(L"nearClipPlane").text().set(L"0.01");
      camNode.append_child(L"farClipPlane").text().set(L"100.0");

      camNode.append_child(L"up").text().set(L"0 1 0");
      camNode.append_child(L"position").text().set(L"0 1 16");
      camNode.append_child(L"look_at").text().set(L"0 1 0");
    }
    hrCameraClose(camRef);

    ////////////////////
    // Render settings
    ////////////////////

    HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 1024, 768, 512, 4096);


    ////////////////////
    // Create scene
    ////////////////////

    HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

    using namespace LiteMath;

    float4x4 mRot;
    float4x4 mTranslate;
    float4x4 mScale;
    float4x4 mRes;

    hrSceneOpen(scnRef, HR_WRITE_DISCARD);

    ///////////
    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0.0f, -1.0f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, planeRef, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(-4.25f, 1.25f, 4.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph1, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0.0f, 1.25f, 4.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph2, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(4.25f, 1.25f, 4.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph3, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();
    mRot.identity();

    const float DEG_TO_RAD = 0.01745329251f; // float(3.14159265358979323846f) / 180.0f;

    mTranslate = translate4x4(float3(0.0f, 1.0f, -4.0f));
    mScale = scale4x4(float3(16.0f, 8.0f, 0.5f));
    mRes = mul(mScale, mRes);
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, boxBG, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0, 15.0f, 0.0));
    mRes = mul(mTranslate, mRes);

    hrLightInstance(scnRef, rectLight, mRes.L());

    ///////////

    hrSceneClose(scnRef);

    hrFlush(scnRef, renderRef);
    
    ////////////////////
    // Rendering, save and check image
    ////////////////////

    RenderProgress(renderRef);

    std::filesystem::create_directories(saveRenderFile.parent_path());
    hrRenderSaveFrameBufferLDR(renderRef, saveRenderFile.wstring().c_str());

    return check_images(ws2s(nameTest).c_str(), 1, 70);
  }


  bool test_114_transparency_fog()
  {
    std::wstring nameTest                = L"test_114";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);

    ////////////////////
    // Materials
    ////////////////////

    HRMaterialRef matR = hrMaterialCreate(L"matR");
    HRMaterialRef matG = hrMaterialCreate(L"matG");
    HRMaterialRef matB = hrMaterialCreate(L"matB");
    HRMaterialRef matBG = hrMaterialCreate(L"matBG");
    HRMaterialRef matGray = hrMaterialCreate(L"matGray");

    HRTextureNodeRef texChecker = hrTexture2DCreateFromFile(L"data/textures/chess_white.bmp");

    hrMaterialOpen(matR, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matR);

      auto refl = matNode.append_child(L"reflectivity");

      refl.append_attribute(L"brdf_type").set_value(L"phong");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(1.0f);
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(1.33f);

      auto transp = matNode.append_child(L"transparency");

      transp.append_attribute(L"brdf_type").set_value(L"phong");
      transp.append_child(L"color").append_attribute(L"val").set_value(L"1.0 0.25 0.25");
      transp.append_child(L"glossiness").append_attribute(L"val").set_value(1.0);
      transp.append_child(L"thin_walled").append_attribute(L"val").set_value(0);
      transp.append_child(L"fog_color").append_attribute(L"val").set_value(L"1.0 0.0 0.0");
      transp.append_child(L"fog_multiplier").append_attribute(L"val").set_value(1.0f);
      transp.append_child(L"ior").append_attribute(L"val").set_value(1.33f);

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matR);

    hrMaterialOpen(matG, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matG);

      auto refl = matNode.append_child(L"reflectivity");

      refl.append_attribute(L"brdf_type").set_value(L"phong");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(1.0f);
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(1.33f);

      auto transp = matNode.append_child(L"transparency");

      transp.append_attribute(L"brdf_type").set_value(L"phong");
      transp.append_child(L"color").append_attribute(L"val").set_value(L"1.0 0.25 0.25");
      transp.append_child(L"glossiness").append_attribute(L"val").set_value(1.0);
      transp.append_child(L"thin_walled").append_attribute(L"val").set_value(0);
      transp.append_child(L"fog_color").append_attribute(L"val").set_value(L"0.0 1.0 0.0");
      transp.append_child(L"fog_multiplier").append_attribute(L"val").set_value(1.0f);
      transp.append_child(L"ior").append_attribute(L"val").set_value(1.33f);

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matG);

    hrMaterialOpen(matB, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matB);

      auto refl = matNode.append_child(L"reflectivity");

      refl.append_attribute(L"brdf_type").set_value(L"phong");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(1.0f);
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(1.33f);

      auto transp = matNode.append_child(L"transparency");

      transp.append_attribute(L"brdf_type").set_value(L"phong");
      transp.append_child(L"color").append_attribute(L"val").set_value(L"1.0 0.25 0.25");
      transp.append_child(L"glossiness").append_attribute(L"val").set_value(1.0);
      transp.append_child(L"thin_walled").append_attribute(L"val").set_value(0);
      transp.append_child(L"fog_color").append_attribute(L"val").set_value(L"0.0 0.0 1.0");
      transp.append_child(L"fog_multiplier").append_attribute(L"val").set_value(1.0f);
      transp.append_child(L"ior").append_attribute(L"val").set_value(1.33f);

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matB);

    hrMaterialOpen(matBG, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matBG);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.25 0.25 1.0");
      color.append_attribute(L"tex_apply_mode").set_value(L"multiply");

			auto texNode = hrTextureBind(texChecker, color);

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 32, 0, 0, 0,
                                  0, 16, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };
      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matBG);

    hrMaterialOpen(matGray, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGray);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      color.append_attribute(L"tex_apply_mode").set_value(L"replace");

			auto texNode = hrTextureBind(texChecker, color);

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 16, 0, 0, 0,
                                  0, 16, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };
      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matGray);

    ////////////////////
    // Meshes
    ////////////////////
    HRMeshRef sph1 = HRMeshFromSimpleMesh(L"sph1", CreateSphere(2.0f, 64), matR.id);
    HRMeshRef sph2 = HRMeshFromSimpleMesh(L"sph2", CreateSphere(2.0f, 64), matG.id);
    HRMeshRef sph3 = HRMeshFromSimpleMesh(L"sph3", CreateSphere(2.0f, 64), matB.id);
    HRMeshRef boxBG = HRMeshFromSimpleMesh(L"boxBG", CreateCube(1.0f), matBG.id);
    HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), matGray.id);

    ////////////////////
    // Light
    ////////////////////

    HRLightRef rectLight = hrLightCreate(L"my_area_light");

    hrLightOpen(rectLight, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(rectLight);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"rect");
      lightNode.attribute(L"distribution").set_value(L"diffuse");

      auto sizeNode = lightNode.append_child(L"size");

      sizeNode.append_attribute(L"half_length").set_value(L"10.0");
      sizeNode.append_attribute(L"half_width").set_value(L"10.0");

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(2.0f*IRRADIANCE_TO_RADIANCE);
			VERIFY_XML(lightNode);
    }
    hrLightClose(rectLight);

    ////////////////////
    // Camera
    ////////////////////

    HRCameraRef camRef = hrCameraCreate(L"my camera");

    hrCameraOpen(camRef, HR_WRITE_DISCARD);
    {
      auto camNode = hrCameraParamNode(camRef);

      camNode.append_child(L"fov").text().set(L"45");
      camNode.append_child(L"nearClipPlane").text().set(L"0.01");
      camNode.append_child(L"farClipPlane").text().set(L"100.0");

      camNode.append_child(L"up").text().set(L"0 1 0");
      camNode.append_child(L"position").text().set(L"0 1 16");
      camNode.append_child(L"look_at").text().set(L"0 1 0");
    }
    hrCameraClose(camRef);

    ////////////////////
    // Render settings
    ////////////////////

    HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 1024, 768, 256, 2048);


    ////////////////////
    // Create scene
    ////////////////////

    HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

    using namespace LiteMath;

    float4x4 mRot;
    float4x4 mTranslate;
    float4x4 mScale;
    float4x4 mRes;

    hrSceneOpen(scnRef, HR_WRITE_DISCARD);

    ///////////
    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0.0f, -1.0f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, planeRef, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(-4.25f, 1.25f, 4.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph1, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0.0f, 1.25f, 4.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph2, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(4.25f, 1.25f, 4.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph3, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();
    mRot.identity();

    const float DEG_TO_RAD = 0.01745329251f; // float(3.14159265358979323846f) / 180.0f;

    mTranslate = translate4x4(float3(0.0f, 1.0f, -4.0f));
    mScale = scale4x4(float3(16.0f, 8.0f, 0.5f));
    mRes = mul(mScale, mRes);
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, boxBG, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0, 15.0f, 0.0));
    mRes = mul(mTranslate, mRes);

    hrLightInstance(scnRef, rectLight, mRes.L());

    ///////////

    hrSceneClose(scnRef);

    hrFlush(scnRef, renderRef);

    ////////////////////
    // Rendering, save and check image
    ////////////////////

    RenderProgress(renderRef);

    std::filesystem::create_directories(saveRenderFile.parent_path());
    hrRenderSaveFrameBufferLDR(renderRef, saveRenderFile.wstring().c_str());

    return check_images(ws2s(nameTest).c_str(), 1, 85);
  }


  bool test_115_transparency_fog_mult()
  {
    std::wstring nameTest                = L"test_115";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);

    ////////////////////
    // Materials
    ////////////////////

    HRMaterialRef mat05 = hrMaterialCreate(L"mat05");
    HRMaterialRef mat20 = hrMaterialCreate(L"mat20");
    HRMaterialRef mat40 = hrMaterialCreate(L"mat40");
    HRMaterialRef matBG = hrMaterialCreate(L"matBG");
    HRMaterialRef matGray = hrMaterialCreate(L"matGray");

    HRTextureNodeRef texChecker = hrTexture2DCreateFromFile(L"data/textures/chess_white.bmp");

    hrMaterialOpen(mat05, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(mat05);

      auto refl = matNode.append_child(L"reflectivity");

      refl.append_attribute(L"brdf_type").set_value(L"phong");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(1.0f);
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(1.33f);

      auto transp = matNode.append_child(L"transparency");

      transp.append_attribute(L"brdf_type").set_value(L"phong");
      transp.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      transp.append_child(L"glossiness").append_attribute(L"val").set_value(1.0);
      transp.append_child(L"thin_walled").append_attribute(L"val").set_value(0);
      transp.append_child(L"fog_color").append_attribute(L"val").set_value(L"0.0 1.0 0.0");
      transp.append_child(L"fog_multiplier").append_attribute(L"val").set_value(0.5f);
      transp.append_child(L"ior").append_attribute(L"val").set_value(1.33f);

			VERIFY_XML(matNode);
    }
    hrMaterialClose(mat05);

    hrMaterialOpen(mat20, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(mat20);

      auto refl = matNode.append_child(L"reflectivity");

      refl.append_attribute(L"brdf_type").set_value(L"phong");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(1.0f);
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(1.33f);

      auto transp = matNode.append_child(L"transparency");

      transp.append_attribute(L"brdf_type").set_value(L"phong");
      transp.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      transp.append_child(L"glossiness").append_attribute(L"val").set_value(1.0);
      transp.append_child(L"thin_walled").append_attribute(L"val").set_value(0);
      transp.append_child(L"fog_color").append_attribute(L"val").set_value(L"0.0 1.0 0.0");
      transp.append_child(L"fog_multiplier").append_attribute(L"val").set_value(2.0f);
      transp.append_child(L"ior").append_attribute(L"val").set_value(1.33f);

			VERIFY_XML(matNode);
    }
    hrMaterialClose(mat20);

    hrMaterialOpen(mat40, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(mat40);

      auto refl = matNode.append_child(L"reflectivity");

      refl.append_attribute(L"brdf_type").set_value(L"phong");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(1.0f);
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(1.33f);

      auto transp = matNode.append_child(L"transparency");

      transp.append_attribute(L"brdf_type").set_value(L"phong");
      transp.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      transp.append_child(L"glossiness").append_attribute(L"val").set_value(1.0);
      transp.append_child(L"thin_walled").append_attribute(L"val").set_value(0);
      transp.append_child(L"fog_color").append_attribute(L"val").set_value(L"0.0 1.0 0.0");
      transp.append_child(L"fog_multiplier").append_attribute(L"val").set_value(4.0f);
      transp.append_child(L"ior").append_attribute(L"val").set_value(1.33f);

			VERIFY_XML(matNode);
    }
    hrMaterialClose(mat40);

    hrMaterialOpen(matBG, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matBG);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.25 0.25 1.0");
      color.append_attribute(L"tex_apply_mode").set_value(L"multiply");

      hrTextureBind(texChecker, color);

      auto texNode = color.child(L"texture");
      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 32, 0, 0, 0,
                                  0, 16, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };
      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matBG);

    hrMaterialOpen(matGray, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGray);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      color.append_attribute(L"tex_apply_mode").set_value(L"replace");

      hrTextureBind(texChecker, color);

      auto texNode = color.child(L"texture");
      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 16, 0, 0, 0,
                                  0, 16, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };
      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matGray);

    ////////////////////
    // Meshes
    ////////////////////
    HRMeshRef sph1 = HRMeshFromSimpleMesh(L"sph1", CreateSphere(2.0f, 64), mat05.id);
    HRMeshRef sph2 = HRMeshFromSimpleMesh(L"sph2", CreateSphere(2.0f, 64), mat20.id);
    HRMeshRef sph3 = HRMeshFromSimpleMesh(L"sph3", CreateSphere(2.0f, 64), mat40.id);
    HRMeshRef boxBG = HRMeshFromSimpleMesh(L"boxBG", CreateCube(1.0f), matBG.id);
    HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), matGray.id);

    ////////////////////
    // Light
    ////////////////////

    HRLightRef rectLight = hrLightCreate(L"my_area_light");

    hrLightOpen(rectLight, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(rectLight);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"rect");
      lightNode.attribute(L"distribution").set_value(L"diffuse");

      auto sizeNode = lightNode.append_child(L"size");

      sizeNode.append_attribute(L"half_length").set_value(L"10.0");
      sizeNode.append_attribute(L"half_width").set_value(L"10.0");

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(2.0f*IRRADIANCE_TO_RADIANCE);
			VERIFY_XML(lightNode);
    }
    hrLightClose(rectLight);

    ////////////////////
    // Camera
    ////////////////////

    HRCameraRef camRef = hrCameraCreate(L"my camera");

    hrCameraOpen(camRef, HR_WRITE_DISCARD);
    {
      auto camNode = hrCameraParamNode(camRef);

      camNode.append_child(L"fov").text().set(L"45");
      camNode.append_child(L"nearClipPlane").text().set(L"0.01");
      camNode.append_child(L"farClipPlane").text().set(L"100.0");

      camNode.append_child(L"up").text().set(L"0 1 0");
      camNode.append_child(L"position").text().set(L"0 1 16");
      camNode.append_child(L"look_at").text().set(L"0 1 0");
    }
    hrCameraClose(camRef);

    ////////////////////
    // Render settings
    ////////////////////

    HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 1024, 768, 512, 4096);


    ////////////////////
    // Create scene
    ////////////////////

    HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

    using namespace LiteMath;

    float4x4 mRot;
    float4x4 mTranslate;
    float4x4 mScale;
    float4x4 mRes;

    hrSceneOpen(scnRef, HR_WRITE_DISCARD);

    ///////////
    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0.0f, -1.0f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, planeRef, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(-4.25f, 1.25f, 4.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph1, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0.0f, 1.25f, 4.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph2, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(4.25f, 1.25f, 4.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph3, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();
    mRot.identity();

    const float DEG_TO_RAD = 0.01745329251f; // float(3.14159265358979323846f) / 180.0f;

    mTranslate = translate4x4(float3(0.0f, 1.0f, -4.0f));
    mScale = scale4x4(float3(16.0f, 8.0f, 0.5f));
    mRes = mul(mScale, mRes);
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, boxBG, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0, 15.0f, 0.0));
    mRes = mul(mTranslate, mRes);

    hrLightInstance(scnRef, rectLight, mRes.L());

    ///////////

    hrSceneClose(scnRef);

    hrFlush(scnRef, renderRef);

    ////////////////////
    // Rendering, save and check image
    ////////////////////

    RenderProgress(renderRef);

    std::filesystem::create_directories(saveRenderFile.parent_path());
    hrRenderSaveFrameBufferLDR(renderRef, saveRenderFile.wstring().c_str());

    return check_images(ws2s(nameTest).c_str(), 1, 75);
  }


  bool test_116_transparency_thin()
  {
    std::wstring nameTest                = L"test_116";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);

    ////////////////////
    // Materials
    ////////////////////

    HRMaterialRef matGlass = hrMaterialCreate(L"matGlass");
    HRMaterialRef matThinGlass = hrMaterialCreate(L"matThinGlass");
    HRMaterialRef matThinGlassFog = hrMaterialCreate(L"matThinGlassFog");
    HRMaterialRef matBG = hrMaterialCreate(L"matBG");
    HRMaterialRef matGray = hrMaterialCreate(L"matGray");

    HRTextureNodeRef texChecker = hrTexture2DCreateFromFile(L"data/textures/chess_white.bmp");

    hrMaterialOpen(matGlass, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGlass);

      auto refl = matNode.append_child(L"reflectivity");

      refl.append_attribute(L"brdf_type").set_value(L"phong");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(1.0f);
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(2.41f);

      auto transp = matNode.append_child(L"transparency");

      transp.append_attribute(L"brdf_type").set_value(L"phong");
      transp.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      transp.append_child(L"glossiness").append_attribute(L"val").set_value(1.0);
      transp.append_child(L"thin_walled").append_attribute(L"val").set_value(0);
      transp.append_child(L"fog_color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      transp.append_child(L"fog_multiplier").append_attribute(L"val").set_value(0.0f);
      transp.append_child(L"ior").append_attribute(L"val").set_value(2.41f);

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matGlass);

    hrMaterialOpen(matThinGlass, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matThinGlass);

      auto refl = matNode.append_child(L"reflectivity");

      refl.append_attribute(L"brdf_type").set_value(L"phong");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(1.0f);
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(2.41f);

      auto transp = matNode.append_child(L"transparency");

      transp.append_attribute(L"brdf_type").set_value(L"phong");
      transp.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      transp.append_child(L"glossiness").append_attribute(L"val").set_value(1.0);
      transp.append_child(L"thin_walled").append_attribute(L"val").set_value(1);
      transp.append_child(L"fog_color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      transp.append_child(L"fog_multiplier").append_attribute(L"val").set_value(0.0f);
      transp.append_child(L"ior").append_attribute(L"val").set_value(2.41f);

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matThinGlass);

    hrMaterialOpen(matThinGlassFog, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matThinGlassFog);

      auto refl = matNode.append_child(L"reflectivity");

      refl.append_attribute(L"brdf_type").set_value(L"phong");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(1.0f);
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(2.41f);

      auto transp = matNode.append_child(L"transparency");

      transp.append_attribute(L"brdf_type").set_value(L"phong");
      transp.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      transp.append_child(L"glossiness").append_attribute(L"val").set_value(1.0);
      transp.append_child(L"thin_walled").append_attribute(L"val").set_value(1);
      transp.append_child(L"fog_color").append_attribute(L"val").set_value(L"0.5 1.0 0.5");
      transp.append_child(L"fog_multiplier").append_attribute(L"val").set_value(2.0f);
      transp.append_child(L"ior").append_attribute(L"val").set_value(2.41f);

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matThinGlassFog);

    hrMaterialOpen(matBG, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matBG);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.25 0.25 1.0");
      color.append_attribute(L"tex_apply_mode").set_value(L"multiply");

			auto texNode = hrTextureBind(texChecker, color);

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 32, 0, 0, 0,
                                  0, 16, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };
      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matBG);

    hrMaterialOpen(matGray, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGray);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      color.append_attribute(L"tex_apply_mode").set_value(L"replace");

			auto texNode = hrTextureBind(texChecker, color);

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 16, 0, 0, 0,
                                  0, 16, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };
      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matGray);

    ////////////////////
    // Meshes
    ////////////////////
    HRMeshRef sph1 = HRMeshFromSimpleMesh(L"sph1", CreateSphere(2.0f, 64), matGlass.id);
    HRMeshRef sph2 = HRMeshFromSimpleMesh(L"sph2", CreateSphere(2.0f, 64), matThinGlass.id);
    HRMeshRef sph3 = HRMeshFromSimpleMesh(L"sph3", CreateSphere(2.0f, 64), matThinGlassFog.id);
    HRMeshRef boxBG = HRMeshFromSimpleMesh(L"boxBG", CreateCube(1.0f), matBG.id);
    HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), matGray.id);

    ////////////////////
    // Light
    ////////////////////

    HRLightRef rectLight = hrLightCreate(L"my_area_light");

    hrLightOpen(rectLight, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(rectLight);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"rect");
      lightNode.attribute(L"distribution").set_value(L"diffuse");

      auto sizeNode = lightNode.append_child(L"size");

      sizeNode.append_attribute(L"half_length").set_value(L"10.0");
      sizeNode.append_attribute(L"half_width").set_value(L"10.0");

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(2.0f*IRRADIANCE_TO_RADIANCE);
			VERIFY_XML(lightNode);
    }
    hrLightClose(rectLight);

    ////////////////////
    // Camera
    ////////////////////

    HRCameraRef camRef = hrCameraCreate(L"my camera");

    hrCameraOpen(camRef, HR_WRITE_DISCARD);
    {
      auto camNode = hrCameraParamNode(camRef);

      camNode.append_child(L"fov").text().set(L"45");
      camNode.append_child(L"nearClipPlane").text().set(L"0.01");
      camNode.append_child(L"farClipPlane").text().set(L"100.0");

      camNode.append_child(L"up").text().set(L"0 1 0");
      camNode.append_child(L"position").text().set(L"0 1 16");
      camNode.append_child(L"look_at").text().set(L"0 1 0");
    }
    hrCameraClose(camRef);

    ////////////////////
    // Render settings
    ////////////////////

    HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 1024, 768, 256, 2048);


    ////////////////////
    // Create scene
    ////////////////////

    HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

    using namespace LiteMath;

    float4x4 mRot;
    float4x4 mTranslate;
    float4x4 mScale;
    float4x4 mRes;

    hrSceneOpen(scnRef, HR_WRITE_DISCARD);

    ///////////
    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0.0f, -1.0f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, planeRef, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(-4.25f, 1.25f, 4.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph1, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0.0f, 1.25f, 4.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph2, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(4.25f, 1.25f, 4.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph3, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();
    mRot.identity();

    const float DEG_TO_RAD = 0.01745329251f; // float(3.14159265358979323846f) / 180.0f;

    mTranslate = translate4x4(float3(0.0f, 1.0f, -4.0f));
    mScale = scale4x4(float3(16.0f, 8.0f, 0.5f));
    mRes = mul(mScale, mRes);
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, boxBG, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0, 15.0f, 0.0));
    mRes = mul(mTranslate, mRes);

    hrLightInstance(scnRef, rectLight, mRes.L());

    ///////////

    hrSceneClose(scnRef);

    hrFlush(scnRef, renderRef);
    
    ////////////////////
    // Rendering, save and check image
    ////////////////////

    RenderProgress(renderRef);

    std::filesystem::create_directories(saveRenderFile.parent_path());
    hrRenderSaveFrameBufferLDR(renderRef, saveRenderFile.wstring().c_str());

    return check_images(ws2s(nameTest).c_str(), 1, 100);
  }


  bool test_117_transparency_texture()
  {
    std::wstring nameTest                = L"test_117";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);

    ////////////////////
    // Materials
    ////////////////////

    HRMaterialRef matR = hrMaterialCreate(L"matR");
    HRMaterialRef matG = hrMaterialCreate(L"matG");
    HRMaterialRef matB = hrMaterialCreate(L"matB");
    HRMaterialRef matBG = hrMaterialCreate(L"matBG");
    HRMaterialRef matGray = hrMaterialCreate(L"matGray");

    HRTextureNodeRef texChecker = hrTexture2DCreateFromFile(L"data/textures/chess_white.bmp");
    HRTextureNodeRef texPattern = hrTexture2DCreateFromFile(L"data/textures/ornament_inv.png");

    hrMaterialOpen(matR, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matR);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_child(L"color").append_attribute(L"val").set_value(L"1.0 0.5 0.5");

      auto transp = matNode.append_child(L"transparency");
      transp.append_attribute(L"brdf_type").set_value(L"phong");

      auto color = transp.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.9 0.7 0.7");
      color.append_attribute(L"tex_apply_mode").set_value(L"multiply");

      transp.append_child(L"glossiness").append_attribute(L"val").set_value(1.0);
      transp.append_child(L"thin_walled").append_attribute(L"val").set_value(0);
      transp.append_child(L"fog_color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      transp.append_child(L"fog_multiplier").append_attribute(L"val").set_value(1.0f);
      transp.append_child(L"ior").append_attribute(L"val").set_value(1.05f);

			auto texNode = hrTextureBind(texPattern, transp.child(L"color"));

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 4, 0, 0, 0,
                                  0, 4, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };
      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matR);

    hrMaterialOpen(matG, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matG);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_child(L"color").append_attribute(L"val").set_value(L"0.5 1.0 0.5");

      auto transp = matNode.append_child(L"transparency");
      transp.append_attribute(L"brdf_type").set_value(L"phong");

      auto color = transp.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.7 0.9 0.7");
      color.append_attribute(L"tex_apply_mode").set_value(L"multiply");

      transp.append_child(L"glossiness").append_attribute(L"val").set_value(1.0);
      transp.append_child(L"thin_walled").append_attribute(L"val").set_value(0);
      transp.append_child(L"fog_color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      transp.append_child(L"fog_multiplier").append_attribute(L"val").set_value(1.0f);
      transp.append_child(L"ior").append_attribute(L"val").set_value(1.05f);

			auto texNode = hrTextureBind(texPattern, transp.child(L"color"));

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 4, 0, 0, 0,
                                  0, 4, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };
      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matG);

    hrMaterialOpen(matB, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matB);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 1.0");

      auto transp = matNode.append_child(L"transparency");
      transp.append_attribute(L"brdf_type").set_value(L"phong");

      auto color = transp.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.7 0.7 0.9");
      color.append_attribute(L"tex_apply_mode").set_value(L"multiply");

      transp.append_child(L"glossiness").append_attribute(L"val").set_value(1.0);
      transp.append_child(L"thin_walled").append_attribute(L"val").set_value(0);
      transp.append_child(L"fog_color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      transp.append_child(L"fog_multiplier").append_attribute(L"val").set_value(1.0f);
      transp.append_child(L"ior").append_attribute(L"val").set_value(1.05f);

			auto texNode = hrTextureBind(texPattern, transp.child(L"color"));

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 4, 0, 0, 0,
                                  0, 4, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };
      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matB);

    hrMaterialOpen(matBG, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matBG);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.25 0.25 1.0");
      color.append_attribute(L"tex_apply_mode").set_value(L"multiply");

			auto texNode = hrTextureBind(texChecker, color);

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 32, 0, 0, 0,
                                  0, 16, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };
      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matBG);

    hrMaterialOpen(matGray, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGray);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      color.append_attribute(L"tex_apply_mode").set_value(L"replace");

			auto texNode = hrTextureBind(texChecker, color);

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 16, 0, 0, 0,
                                  0, 16, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };
      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matGray);

    ////////////////////
    // Meshes
    ////////////////////
    HRMeshRef sph1 = HRMeshFromSimpleMesh(L"sph1", CreateSphere(2.0f, 64), matR.id);
    HRMeshRef sph2 = HRMeshFromSimpleMesh(L"sph2", CreateSphere(2.0f, 64), matG.id);
    HRMeshRef sph3 = HRMeshFromSimpleMesh(L"sph3", CreateSphere(2.0f, 64), matB.id);
    HRMeshRef boxBG = HRMeshFromSimpleMesh(L"boxBG", CreateCube(1.0f), matBG.id);
    HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), matGray.id);

    ////////////////////
    // Light
    ////////////////////

    HRLightRef rectLight = hrLightCreate(L"my_area_light");

    hrLightOpen(rectLight, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(rectLight);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"rect");
      lightNode.attribute(L"distribution").set_value(L"diffuse");

      auto sizeNode = lightNode.append_child(L"size");

      sizeNode.append_attribute(L"half_length").set_value(L"10.0");
      sizeNode.append_attribute(L"half_width").set_value(L"10.0");

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(2.0*IRRADIANCE_TO_RADIANCE);
			VERIFY_XML(lightNode);
    }
    hrLightClose(rectLight);

    ////////////////////
    // Camera
    ////////////////////

    HRCameraRef camRef = hrCameraCreate(L"my camera");

    hrCameraOpen(camRef, HR_WRITE_DISCARD);
    {
      auto camNode = hrCameraParamNode(camRef);

      camNode.append_child(L"fov").text().set(L"45");
      camNode.append_child(L"nearClipPlane").text().set(L"0.01");
      camNode.append_child(L"farClipPlane").text().set(L"100.0");

      camNode.append_child(L"up").text().set(L"0 1 0");
      camNode.append_child(L"position").text().set(L"0 1 16");
      camNode.append_child(L"look_at").text().set(L"0 1 0");
    }
    hrCameraClose(camRef);

    ////////////////////
    // Render settings
    ////////////////////

    HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 1024, 768, 256, 2048);


    ////////////////////
    // Create scene
    ////////////////////

    HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

    using namespace LiteMath;

    float4x4 mRot;
    float4x4 mTranslate;
    float4x4 mScale;
    float4x4 mRes;

    hrSceneOpen(scnRef, HR_WRITE_DISCARD);

    ///////////
    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0.0f, -1.0f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, planeRef, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(-4.25f, 1.25f, 4.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph1, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0.0f, 1.25f, 4.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph2, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(4.25f, 1.25f, 4.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph3, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();
    mRot.identity();

    const float DEG_TO_RAD = 0.01745329251f; // float(3.14159265358979323846f) / 180.0f;

    mTranslate = translate4x4(float3(0.0f, 1.0f, -4.0f));
    mScale = scale4x4(float3(16.0f, 8.0f, 0.5f));
    mRes = mul(mScale, mRes);
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, boxBG, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0, 15.0f, 0.0));
    mRes = mul(mTranslate, mRes);

    hrLightInstance(scnRef, rectLight, mRes.L());

    ///////////

    hrSceneClose(scnRef);

    hrFlush(scnRef, renderRef);
    
    ////////////////////
    // Rendering, save and check image
    ////////////////////

    RenderProgress(renderRef);

    std::filesystem::create_directories(saveRenderFile.parent_path());
    hrRenderSaveFrameBufferLDR(renderRef, saveRenderFile.wstring().c_str());

    return check_images(ws2s(nameTest).c_str(), 1, 120);
  }


  bool test_118_transparency_glossiness_texture()
  {
    std::wstring nameTest                = L"test_118";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);

    ////////////////////
    // Materials
    ////////////////////

    HRMaterialRef matR = hrMaterialCreate(L"matR");
    HRMaterialRef matG = hrMaterialCreate(L"matG");
    HRMaterialRef matB = hrMaterialCreate(L"matB");
    HRMaterialRef matBG = hrMaterialCreate(L"matBG");
    HRMaterialRef matGray = hrMaterialCreate(L"matGray");

    HRTextureNodeRef texChecker = hrTexture2DCreateFromFile(L"data/textures/chess_white.bmp");
    HRTextureNodeRef texPattern = hrTexture2DCreateFromFile(L"data/textures/blur_pattern.bmp");

    hrMaterialOpen(matR, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matR);

      auto transp = matNode.append_child(L"transparency");

      transp.append_attribute(L"brdf_type").set_value(L"phong");
      transp.append_child(L"color").append_attribute(L"val").set_value(L"1.0 0.7 0.7");
      transp.append_child(L"glossiness").append_attribute(L"val").set_value(2.0f);
      transp.append_child(L"thin_walled").append_attribute(L"val").set_value(0);
      transp.append_child(L"fog_color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      transp.append_child(L"fog_multiplier").append_attribute(L"val").set_value(1.0f);
      transp.append_child(L"ior").append_attribute(L"val").set_value(1.05f);

			auto texNode = hrTextureBind(texPattern, transp.child(L"glossiness"));

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 2, 0, 0, 0,
                                  0, 2, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };
      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matR);

    hrMaterialOpen(matG, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matG);

      auto transp = matNode.append_child(L"transparency");

      transp.append_attribute(L"brdf_type").set_value(L"phong");
      transp.append_child(L"color").append_attribute(L"val").set_value(L"0.7 1.0 0.7");
      transp.append_child(L"glossiness").append_attribute(L"val").set_value(0.9);
      transp.append_child(L"thin_walled").append_attribute(L"val").set_value(0);
      transp.append_child(L"fog_color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      transp.append_child(L"fog_multiplier").append_attribute(L"val").set_value(1.0f);
      transp.append_child(L"ior").append_attribute(L"val").set_value(1.05f);

			auto texNode = hrTextureBind(texPattern, transp.child(L"glossiness"));

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 2, 0, 0, 0,
                                  0, 2, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };
      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matG);

    hrMaterialOpen(matB, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matB);

      auto transp = matNode.append_child(L"transparency");

      transp.append_attribute(L"brdf_type").set_value(L"phong");
      transp.append_child(L"color").append_attribute(L"val").set_value(L"0.7 0.7 1.0");
      transp.append_child(L"glossiness").append_attribute(L"val").set_value(0.8);
      transp.append_child(L"thin_walled").append_attribute(L"val").set_value(0);
      transp.append_child(L"fog_color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      transp.append_child(L"fog_multiplier").append_attribute(L"val").set_value(1.0f);
      transp.append_child(L"ior").append_attribute(L"val").set_value(1.05f);

			auto texNode = hrTextureBind(texPattern, transp.child(L"glossiness"));

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 2, 0, 0, 0,
                                  0, 2, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };
      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matB);

    hrMaterialOpen(matBG, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matBG);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.25 0.25 1.0");
      color.append_attribute(L"tex_apply_mode").set_value(L"multiply");

			auto texNode = hrTextureBind(texChecker, color);

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 32, 0, 0, 0,
                                  0, 16, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };
      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matBG);

    hrMaterialOpen(matGray, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGray);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      color.append_attribute(L"tex_apply_mode").set_value(L"replace");

			auto texNode = hrTextureBind(texChecker, color);

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 16, 0, 0, 0,
                                  0, 16, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };
      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matGray);

    ////////////////////
    // Meshes
    ////////////////////
    HRMeshRef sph1 = HRMeshFromSimpleMesh(L"sph1", CreateSphere(2.0f, 64), matR.id);
    HRMeshRef sph2 = HRMeshFromSimpleMesh(L"sph2", CreateSphere(2.0f, 64), matG.id);
    HRMeshRef sph3 = HRMeshFromSimpleMesh(L"sph3", CreateSphere(2.0f, 64), matB.id);
    HRMeshRef boxBG = HRMeshFromSimpleMesh(L"boxBG", CreateCube(1.0f), matBG.id);
    HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), matGray.id);

    ////////////////////
    // Light
    ////////////////////

    HRLightRef rectLight = hrLightCreate(L"my_area_light");

    hrLightOpen(rectLight, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(rectLight);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"rect");
      lightNode.attribute(L"distribution").set_value(L"diffuse");

      auto sizeNode = lightNode.append_child(L"size");

      sizeNode.append_attribute(L"half_length").set_value(L"5.0");
      sizeNode.append_attribute(L"half_width").set_value(L"5.0");

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(8.0f*IRRADIANCE_TO_RADIANCE);
			VERIFY_XML(lightNode);
    }
    hrLightClose(rectLight);

    ////////////////////
    // Camera
    ////////////////////

    HRCameraRef camRef = hrCameraCreate(L"my camera");

    hrCameraOpen(camRef, HR_WRITE_DISCARD);
    {
      auto camNode = hrCameraParamNode(camRef);

      camNode.append_child(L"fov").text().set(L"45");
      camNode.append_child(L"nearClipPlane").text().set(L"0.01");
      camNode.append_child(L"farClipPlane").text().set(L"100.0");

      camNode.append_child(L"up").text().set(L"0 1 0");
      camNode.append_child(L"position").text().set(L"0 1 16");
      camNode.append_child(L"look_at").text().set(L"0 1 0");
    }
    hrCameraClose(camRef);

    ////////////////////
    // Render settings
    ////////////////////

    HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 1024, 768, 256, 2048);


    ////////////////////
    // Create scene
    ////////////////////

    HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

    using namespace LiteMath;

    float4x4 mRot;
    float4x4 mTranslate;
    float4x4 mScale;
    float4x4 mRes;

    hrSceneOpen(scnRef, HR_WRITE_DISCARD);

    ///////////
    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0.0f, -1.0f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, planeRef, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(-4.25f, 1.25f, 4.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph1, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0.0f, 1.25f, 4.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph2, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(4.25f, 1.25f, 4.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph3, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();
    mRot.identity();

    const float DEG_TO_RAD = 0.01745329251f; // float(3.14159265358979323846f) / 180.0f;

    mTranslate = translate4x4(float3(0.0f, 1.0f, -4.0f));
    mScale = scale4x4(float3(16.0f, 8.0f, 0.5f));
    mRes = mul(mScale, mRes);
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, boxBG, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0, 15.0f, 0.0));
    mRes = mul(mTranslate, mRes);

    hrLightInstance(scnRef, rectLight, mRes.L());

    ///////////

    hrSceneClose(scnRef);

    hrFlush(scnRef, renderRef);
    
    ////////////////////
    // Rendering, save and check image
    ////////////////////

    RenderProgress(renderRef);

    std::filesystem::create_directories(saveRenderFile.parent_path());
    hrRenderSaveFrameBufferLDR(renderRef, saveRenderFile.wstring().c_str());

    return check_images(ws2s(nameTest).c_str(), 1, 150);
  }


  bool test_119_opacity_texture()
  {
    std::wstring nameTest                = L"test_119";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);

    ////////////////////
    // Materials
    ////////////////////

    HRMaterialRef matR = hrMaterialCreate(L"matR");
    HRMaterialRef matG = hrMaterialCreate(L"matG");
    HRMaterialRef matB = hrMaterialCreate(L"matB");
    HRMaterialRef matBG = hrMaterialCreate(L"matBG");
    HRMaterialRef matGray = hrMaterialCreate(L"matGray");

    HRTextureNodeRef texChecker = hrTexture2DCreateFromFile(L"data/textures/chess_white.bmp");
    HRTextureNodeRef texPattern = hrTexture2DCreateFromFile(L"data/textures/bigleaf3.tga");

    hrMaterialOpen(matR, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matR);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_child(L"color").append_attribute(L"val").set_value(L"1.0 0.5 0.5");

      auto opacity = matNode.append_child(L"opacity");
      opacity.append_child(L"skip_shadow").append_attribute(L"val").set_value(0);

			auto texNode = hrTextureBind(texPattern, opacity);

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 3, 0, 0, 0,
                                  0, 3, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };
      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matR);

    hrMaterialOpen(matG, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matG);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_child(L"color").append_attribute(L"val").set_value(L"0.5 1.0 0.5");

      auto opacity = matNode.append_child(L"opacity");
      opacity.append_child(L"skip_shadow").append_attribute(L"val").set_value(1);

			auto texNode = hrTextureBind(texPattern, opacity);

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 3, 0, 0, 0,
                                  0, 3, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };
      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matG);

    hrMaterialOpen(matB, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matB);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 1.0");

      auto opacity = matNode.append_child(L"opacity");
      opacity.append_child(L"skip_shadow").append_attribute(L"val").set_value(1);

			// auto texNode = hrTextureBind(texPattern, opacity);
      // 
      // texNode.append_attribute(L"matrix");
      // float samplerMatrix[16] = { 3, 0, 0, 0,
      //                             0, 3, 0, 0,
      //                             0, 0, 1, 0,
      //                             0, 0, 0, 1 };
      // texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      // texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      // texNode.append_attribute(L"input_gamma").set_value(2.2f);
      // texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      // HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matB);

    hrMaterialOpen(matBG, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matBG);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.25 0.25 1.0");
      color.append_attribute(L"tex_apply_mode").set_value(L"multiply");

			auto texNode = hrTextureBind(texChecker, color);

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 32, 0, 0, 0,
                                  0, 16, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };
      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matBG);

    hrMaterialOpen(matGray, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGray);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      color.append_attribute(L"tex_apply_mode").set_value(L"replace");

			auto texNode = hrTextureBind(texChecker, color);

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 16, 0, 0, 0,
                                  0, 16, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };
      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matGray);

    ////////////////////
    // Meshes
    ////////////////////
    HRMeshRef sph1 = HRMeshFromSimpleMesh(L"sph1", CreateSphere(2.0f, 64), matR.id);
    HRMeshRef sph2 = HRMeshFromSimpleMesh(L"sph2", CreateSphere(2.0f, 64), matG.id);
    HRMeshRef sph3 = HRMeshFromSimpleMesh(L"sph3", CreateSphere(2.0f, 64), matB.id);
    HRMeshRef boxBG = HRMeshFromSimpleMesh(L"boxBG", CreateCube(1.0f), matBG.id);
    HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), matGray.id);

    ////////////////////
    // Light
    ////////////////////

    HRLightRef rectLight = hrLightCreate(L"my_area_light");

    hrLightOpen(rectLight, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(rectLight);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"rect");
      lightNode.attribute(L"distribution").set_value(L"diffuse");

      auto sizeNode = lightNode.append_child(L"size");

      sizeNode.append_attribute(L"half_length").set_value(L"1.0");
      sizeNode.append_attribute(L"half_width").set_value(L"1.0");

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(200.0f*IRRADIANCE_TO_RADIANCE);
			VERIFY_XML(lightNode);
    }
    hrLightClose(rectLight);

    ////////////////////
    // Camera
    ////////////////////

    HRCameraRef camRef = hrCameraCreate(L"my camera");

    hrCameraOpen(camRef, HR_WRITE_DISCARD);
    {
      auto camNode = hrCameraParamNode(camRef);

      camNode.append_child(L"fov").text().set(L"45");
      camNode.append_child(L"nearClipPlane").text().set(L"0.01");
      camNode.append_child(L"farClipPlane").text().set(L"100.0");

      camNode.append_child(L"up").text().set(L"0 1 0");
      camNode.append_child(L"position").text().set(L"0 1 16");
      camNode.append_child(L"look_at").text().set(L"0 1 0");
    }
    hrCameraClose(camRef);

    ////////////////////
    // Render settings
    ////////////////////

    HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 1024, 768, 256, 2048);


    ////////////////////
    // Create scene
    ////////////////////

    HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

    using namespace LiteMath;

    float4x4 mRot;
    float4x4 mTranslate;
    float4x4 mScale;
    float4x4 mRes;

    hrSceneOpen(scnRef, HR_WRITE_DISCARD);

    ///////////
    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0.0f, -1.0f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, planeRef, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(-4.25f, 1.25f, 4.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph1, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0.0f, 1.25f, 4.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph2, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(4.25f, 1.25f, 4.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph3, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();
    mRot.identity();

    const float DEG_TO_RAD = 0.01745329251f; // float(3.14159265358979323846f) / 180.0f;

    mTranslate = translate4x4(float3(0.0f, 1.0f, -4.0f));
    mScale = scale4x4(float3(16.0f, 8.0f, 0.5f));
    mRes = mul(mScale, mRes);
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, boxBG, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0, 20.0f, 0.0));
    mRes = mul(mTranslate, mRes);

    hrLightInstance(scnRef, rectLight, mRes.L());

    ///////////

    hrSceneClose(scnRef);

    hrFlush(scnRef, renderRef);
    
    ////////////////////
    // Rendering, save and check image
    ////////////////////

    RenderProgress(renderRef);

    std::filesystem::create_directories(saveRenderFile.parent_path());
    hrRenderSaveFrameBufferLDR(renderRef, saveRenderFile.wstring().c_str());

    return check_images(ws2s(nameTest).c_str(), 1, 50);
  }


  bool test_120_opacity_shadow_matte()
  {
    std::wstring nameTest                 = L"test_120";
    std::filesystem::path libraryPath     = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile  = L"tests_images/" + nameTest + L"/z_out.png";
    std::filesystem::path saveRenderFile2 = L"tests_images/" + nameTest + L"/z_out2.png";
    std::filesystem::path saveRenderFile3 = L"tests_images/" + nameTest + L"/z_out3.png";
    std::filesystem::path saveRenderFile4 = L"tests_images/" + nameTest + L"/z_out4.png";
    std::filesystem::path saveRenderFile5 = L"tests_images/" + nameTest + L"/z_out5.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);

    ////////////////////
    // Materials
    ////////////////////

    HRMaterialRef matR    = hrMaterialCreate(L"matR");
    HRMaterialRef matG    = hrMaterialCreate(L"matG");
    HRMaterialRef matB    = hrMaterialCreate(L"matB");
    HRMaterialRef matBG   = hrMaterialCreate(L"matBG");
    HRMaterialRef matGray = hrMaterialCreate(L"matGray");

    HRTextureNodeRef texChecker = hrTexture2DCreateFromFile(L"data/textures/chess_white.bmp");
    HRTextureNodeRef texPattern = hrTexture2DCreateFromFile(L"data/textures/blur_pattern.bmp");

    hrMaterialOpen(matR, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matR);

      auto diff = matNode.append_child(L"diffuse");

      diff.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.15 0.15");

      auto opacity = matNode.append_child(L"opacity");
      opacity.append_child(L"skip_shadow").append_attribute(L"val").set_value(0);
      opacity.append_child(L"shadow_matte").append_attribute(L"val").set_value(0);

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matR);

    hrMaterialOpen(matG, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matG);
			matNode.attribute(L"type").set_value(L"shadow_catcher");
			VERIFY_XML(matNode);
    }
    hrMaterialClose(matG);

    hrMaterialOpen(matB, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matB);

      auto diff = matNode.append_child(L"diffuse");

      diff.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 1.0");

      auto opacity = matNode.append_child(L"opacity");
      opacity.append_child(L"skip_shadow").append_attribute(L"val").set_value(0);
      opacity.append_child(L"shadow_matte").append_attribute(L"val").set_value(0);

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matB);

    hrMaterialOpen(matBG, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matBG);

      auto diff = matNode.append_child(L"diffuse");

      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.25 0.25 1.0");

			auto texNode = hrTextureBind(texChecker, color);

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 32, 0, 0, 0,
                                  0, 16, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };
      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matBG);

    hrMaterialOpen(matGray, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGray);

      auto diff = matNode.append_child(L"diffuse");

      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"1.0 1.0 1.0");

			auto texNode = hrTextureBind(texChecker, color);

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 16, 0, 0, 0,
                                  0, 16, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };
      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matGray);

    ////////////////////
    // Meshes
    ////////////////////
    HRMeshRef sph1      = HRMeshFromSimpleMesh(L"sph1", CreateSphere(1.0f, 64), matR.id);
    HRMeshRef sph2      = HRMeshFromSimpleMesh(L"sph2", CreateSphere(1.0f, 64), matR.id);
    HRMeshRef sph3      = HRMeshFromSimpleMesh(L"sph3", CreateSphere(1.0f, 64), matR.id);
    HRMeshRef boxBG     = HRMeshFromSimpleMesh(L"boxBG", CreateCube(1.0f), matBG.id);
    HRMeshRef planeRef  = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), matGray.id);
		HRMeshRef planeRef2 = HRMeshFromSimpleMesh(L"my_plane2", CreatePlane(4.0f), matG.id);

    ////////////////////
    // Light
    ////////////////////

    HRLightRef rectLight = hrLightCreate(L"my_area_light");

    hrLightOpen(rectLight, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(rectLight);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"rect");
      lightNode.attribute(L"distribution").set_value(L"diffuse");

      auto sizeNode = lightNode.append_child(L"size");

      sizeNode.append_attribute(L"half_length").set_value(L"2.0");
      sizeNode.append_attribute(L"half_width").set_value(L"2.0");

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(20.0*IRRADIANCE_TO_RADIANCE);
			VERIFY_XML(lightNode);
    }
    hrLightClose(rectLight);

		HRLightRef sky = hrLightCreate(L"sky");

		hrLightOpen(sky, HR_WRITE_DISCARD);
		{
			auto lightNode = hrLightParamNode(sky);

			lightNode.attribute(L"type").set_value(L"sky");

			auto intensityNode = lightNode.append_child(L"intensity");

			intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"0.05 0.05 0.25");
			intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(1.0f);

			VERIFY_XML(lightNode);
		}
		hrLightClose(sky);

    ////////////////////
    // Camera
    ////////////////////

    HRCameraRef camRef = hrCameraCreate(L"my camera");

    hrCameraOpen(camRef, HR_WRITE_DISCARD);
    {
      auto camNode = hrCameraParamNode(camRef);

      camNode.append_child(L"fov").text().set(L"45");
      camNode.append_child(L"nearClipPlane").text().set(L"0.01");
      camNode.append_child(L"farClipPlane").text().set(L"100.0");

      camNode.append_child(L"up").text().set(L"0 1 0");
      camNode.append_child(L"position").text().set(L"0 1 11");
      camNode.append_child(L"look_at").text().set(L"0 -0.5 0");
    }
    hrCameraClose(camRef);

    ////////////////////
    // Render settings
    ////////////////////

    HRRenderRef renderRef = CreateBasicTestRenderPTNoCaust(CURR_RENDER_DEVICE, 512, 512, 256, 2048);

    hrRenderOpen(renderRef, HR_OPEN_EXISTING);
    {
      auto camNode = hrRenderParamNode(renderRef);
      camNode.force_child(L"evalgbuffer").text() = 1;
    }
    hrRenderClose(renderRef);


    ////////////////////
    // Create scene
    ////////////////////

    HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

    using namespace LiteMath;

    float4x4 mRot;
    float4x4 mTranslate;
    float4x4 mScale;
    float4x4 mRes;

    hrSceneOpen(scnRef, HR_WRITE_DISCARD);

    ///////////

    //mTranslate = translate4x4(float3(0.0f, -1.0f, 0.0f));
    //hrMeshInstance(scnRef, planeRef, mTranslate.L());

		mTranslate = translate4x4(float3(0.0f, -0.75f, 5.0f));
		hrMeshInstance(scnRef, planeRef2, mTranslate.L());

    mTranslate.identity();
    mTranslate = translate4x4(float3(0.0f, 0.25f, 5.0f));
    hrMeshInstance(scnRef, sph2, mTranslate.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0, 15.0f, 0.0));
    mRes = mul(mTranslate, mRes);

    hrLightInstance(scnRef, rectLight, mRes.L());
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
    hrRenderSaveGBufferLayerLDR(renderRef, saveRenderFile2.wstring().c_str(), L"depth");
    hrRenderSaveGBufferLayerLDR(renderRef, saveRenderFile3.wstring().c_str(), L"diffcolor");
    hrRenderSaveGBufferLayerLDR(renderRef, saveRenderFile4.wstring().c_str(), L"alpha");
    hrRenderSaveGBufferLayerLDR(renderRef, saveRenderFile5.wstring().c_str(), L"shadow");

    return check_images(ws2s(nameTest).c_str(), 5, 25);    
  }


  bool test_121_translucency()
  {
    std::wstring nameTest                = L"test_121";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);

    ////////////////////
    // Materials
    ////////////////////

    HRMaterialRef matR = hrMaterialCreate(L"matR");
    HRMaterialRef matG = hrMaterialCreate(L"matG");
    HRMaterialRef matB = hrMaterialCreate(L"matB");
    HRMaterialRef matGray = hrMaterialCreate(L"matGray");

    HRTextureNodeRef texChecker = hrTexture2DCreateFromFile(L"data/textures/chess_white.bmp");

    hrMaterialOpen(matR, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matR);

      auto transl = matNode.append_child(L"translucency");
      transl.append_child(L"color").append_attribute(L"val").set_value(L"1.0 0.15 0.15");
			VERIFY_XML(matNode);
    }
    hrMaterialClose(matR);

    hrMaterialOpen(matG, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matG);

      auto transl = matNode.append_child(L"translucency");
      transl.append_child(L"color").append_attribute(L"val").set_value(L"0.15 1.0 0.15");
			VERIFY_XML(matNode);
    }
    hrMaterialClose(matG);

    hrMaterialOpen(matB, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matB);

      auto transl = matNode.append_child(L"translucency");
      transl.append_child(L"color").append_attribute(L"val").set_value(L"0.15 0.15 1.0");
			VERIFY_XML(matNode);
    }
    hrMaterialClose(matB);

    hrMaterialOpen(matGray, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGray);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      color.append_attribute(L"tex_apply_mode").set_value(L"replace");

			auto texNode = hrTextureBind(texChecker, color);

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 16, 0, 0, 0,
                                  0, 16, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };
      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

			VERIFY_XML(matNode);
    }
    hrMaterialClose(matGray);

    ////////////////////
    // Meshes
    ////////////////////
    HRMeshRef box1 = HRMeshFromSimpleMesh(L"box1", CreateCube(1.0f), matR.id);
    HRMeshRef box2 = HRMeshFromSimpleMesh(L"box2", CreateCube(1.0f), matG.id);
    HRMeshRef box3 = HRMeshFromSimpleMesh(L"box3", CreateCube(1.0f), matB.id);
    HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), matGray.id);

    ////////////////////
    // Light
    ////////////////////

    HRLightRef rectLight = hrLightCreate(L"my_area_light");

    hrLightOpen(rectLight, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(rectLight);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"rect");
      lightNode.attribute(L"distribution").set_value(L"diffuse");

      auto sizeNode = lightNode.append_child(L"size");

      sizeNode.append_attribute(L"half_length").set_value(L"10.0");
      sizeNode.append_attribute(L"half_width").set_value(L"10.0");

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(2.0f*IRRADIANCE_TO_RADIANCE);
			VERIFY_XML(lightNode);
    }
    hrLightClose(rectLight);

    ////////////////////
    // Camera
    ////////////////////

    HRCameraRef camRef = hrCameraCreate(L"my camera");

    hrCameraOpen(camRef, HR_WRITE_DISCARD);
    {
      auto camNode = hrCameraParamNode(camRef);

      camNode.append_child(L"fov").text().set(L"45");
      camNode.append_child(L"nearClipPlane").text().set(L"0.01");
      camNode.append_child(L"farClipPlane").text().set(L"100.0");

      camNode.append_child(L"up").text().set(L"0 1 0");
      camNode.append_child(L"position").text().set(L"0 10 14");
      camNode.append_child(L"look_at").text().set(L"0 1 5");
    }
    hrCameraClose(camRef);

    ////////////////////
    // Render settings
    ////////////////////

    HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 512, 512, 256, 2048);


    ////////////////////
    // Create scene
    ////////////////////

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

    mTranslate = translate4x4(float3(0.0f, -1.0f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, planeRef, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();
    mRot.identity();

    mTranslate = translate4x4(float3(-4.25f, 1.25f, 4.0f));
    mScale = scale4x4(float3(2.0f, 4.0f, 0.02f));
    mRot = rotate4x4X(90.f*DEG_TO_RAD);
    mRes = mul(mScale, mRes);
    mRes = mul(mRot, mRes);
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, box1, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();
    mRot.identity();

    mTranslate = translate4x4(float3(0.0f, 1.25f, 4.0f));
    mScale = scale4x4(float3(2.0f, 4.0f, 0.02f));
    mRot = rotate4x4X(90.f*DEG_TO_RAD);
    mRes = mul(mScale, mRes);
    mRes = mul(mRot, mRes);
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, box2, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();
    mRot.identity();

    mTranslate = translate4x4(float3(4.25f, 1.25f, 4.0f));
    mScale = scale4x4(float3(2.0f, 4.0f, 0.02f));
    mRot = rotate4x4X(90.f*DEG_TO_RAD);
    mRes = mul(mScale, mRes);
    mRes = mul(mRot, mRes);
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, box3, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();
    mRot.identity();
    
    mTranslate = translate4x4(float3(0, 15.0f, 00.0));
    //mRot = rotate4x4X(45.f*DEG_TO_RAD);
    //mRes = mul(mRot, mRes);
    mRes = mul(mTranslate, mRes);

    hrLightInstance(scnRef, rectLight, mRes.L());

    ///////////

    hrSceneClose(scnRef);

    hrFlush(scnRef, renderRef);
    
    ////////////////////
    // Rendering, save and check image
    ////////////////////

    RenderProgress(renderRef);

    std::filesystem::create_directories(saveRenderFile.parent_path());
    hrRenderSaveFrameBufferLDR(renderRef, saveRenderFile.wstring().c_str());

    return check_images(ws2s(nameTest).c_str(), 1, 20);
  }


  bool test_122_translucency_texture()
  {
    std::wstring nameTest                = L"test_122";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);

    ////////////////////
    // Materials
    ////////////////////

    HRMaterialRef matR = hrMaterialCreate(L"matR");
    HRMaterialRef matG = hrMaterialCreate(L"matG");
    HRMaterialRef matB = hrMaterialCreate(L"matB");
    HRMaterialRef matGray = hrMaterialCreate(L"matGray");

    HRTextureNodeRef texChecker = hrTexture2DCreateFromFile(L"data/textures/chess_white.bmp");

    hrMaterialOpen(matR, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matR);

      auto transl = matNode.append_child(L"translucency");

      auto color = transl.append_child(L"color");
      color.append_attribute(L"val").set_value(L"1.0 0.15 0.15");
      color.append_attribute(L"tex_apply_mode").set_value(L"multiply");

			auto texNode = hrTextureBind(texChecker, transl.child(L"color"));

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 4, 0, 0, 0,
                                  0, 4, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };
      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);
			VERIFY_XML(matNode);
    }
    hrMaterialClose(matR);

    hrMaterialOpen(matG, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matG);

      auto transl = matNode.append_child(L"translucency");

      auto color = transl.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.15 1.0 0.15");
      color.append_attribute(L"tex_apply_mode").set_value(L"multiply");

			auto texNode = hrTextureBind(texChecker, transl.child(L"color"));

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 8, 0, 0, 0,
                                  0, 8, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };
      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);
			VERIFY_XML(matNode);
    }
    hrMaterialClose(matG);

    hrMaterialOpen(matB, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matB);

      auto transl = matNode.append_child(L"translucency");

      auto color = transl.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.15 0.15 1.0");
      color.append_attribute(L"tex_apply_mode").set_value(L"multiply");

			auto texNode = hrTextureBind(texChecker, transl.child(L"color"));

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 4, 0, 0, 0,
                                  0, 4, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };
      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);
			VERIFY_XML(matNode);
    }
    hrMaterialClose(matB);

    hrMaterialOpen(matGray, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGray);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      color.append_attribute(L"tex_apply_mode").set_value(L"replace");

			auto texNode = hrTextureBind(texChecker, color);

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 16, 0, 0, 0,
                                  0, 16, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };
      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);
			VERIFY_XML(matNode);
    }
    hrMaterialClose(matGray);

    ////////////////////
    // Meshes
    ////////////////////
    HRMeshRef box1 = HRMeshFromSimpleMesh(L"box1", CreateCube(1.0f), matR.id);
    HRMeshRef box2 = HRMeshFromSimpleMesh(L"box2", CreateCube(1.0f), matG.id);
    HRMeshRef box3 = HRMeshFromSimpleMesh(L"box3", CreateCube(1.0f), matB.id);
    HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), matGray.id);

    ////////////////////
    // Light
    ////////////////////

    HRLightRef rectLight = hrLightCreate(L"my_area_light");

    hrLightOpen(rectLight, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(rectLight);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"rect");
      lightNode.attribute(L"distribution").set_value(L"diffuse");

      auto sizeNode = lightNode.append_child(L"size");

      sizeNode.append_attribute(L"half_length").set_value(L"10.0");
      sizeNode.append_attribute(L"half_width").set_value(L"10.0");

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(2.0f*IRRADIANCE_TO_RADIANCE);
			VERIFY_XML(lightNode);
    }
    hrLightClose(rectLight);

    ////////////////////
    // Camera
    ////////////////////

    HRCameraRef camRef = hrCameraCreate(L"my camera");

    hrCameraOpen(camRef, HR_WRITE_DISCARD);
    {
      auto camNode = hrCameraParamNode(camRef);

      camNode.append_child(L"fov").text().set(L"45");
      camNode.append_child(L"nearClipPlane").text().set(L"0.01");
      camNode.append_child(L"farClipPlane").text().set(L"100.0");

      camNode.append_child(L"up").text().set(L"0 1 0");
      camNode.append_child(L"position").text().set(L"0 10 14");
      camNode.append_child(L"look_at").text().set(L"0 1 5");
    }
    hrCameraClose(camRef);

    ////////////////////
    // Render settings
    ////////////////////

    HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 512, 512, 256, 2048);


    ////////////////////
    // Create scene
    ////////////////////

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

    mTranslate = translate4x4(float3(0.0f, -1.0f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, planeRef, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();
    mRot.identity();

    mTranslate = translate4x4(float3(-4.25f, 1.25f, 4.0f));
    mScale = scale4x4(float3(2.0f, 4.0f, 0.02f));
    mRot = rotate4x4X(90.f*DEG_TO_RAD);
    mRes = mul(mScale, mRes);
    mRes = mul(mRot, mRes);
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, box1, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();
    mRot.identity();

    mTranslate = translate4x4(float3(0.0f, 1.25f, 4.0f));
    mScale = scale4x4(float3(2.0f, 4.0f, 0.02f));
    mRot = rotate4x4X(90.f*DEG_TO_RAD);
    mRes = mul(mScale, mRes);
    mRes = mul(mRot, mRes);
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, box2, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();
    mRot.identity();

    mTranslate = translate4x4(float3(4.25f, 1.25f, 4.0f));
    mScale = scale4x4(float3(2.0f, 4.0f, 0.02f));
    mRot = rotate4x4X(90.f*DEG_TO_RAD);
    mRes = mul(mScale, mRes);
    mRes = mul(mRot, mRes);
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, box3, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();
    mRot.identity();



    mTranslate = translate4x4(float3(0, 15.0f, 00.0));
    //mRot = rotate4x4X(45.f*DEG_TO_RAD);
    //mRes = mul(mRot, mRes);
    mRes = mul(mTranslate, mRes);

    hrLightInstance(scnRef, rectLight, mRes.L());

    ///////////

    hrSceneClose(scnRef);

    hrFlush(scnRef, renderRef);
    
    ////////////////////
    // Rendering, save and check image
    ////////////////////

    RenderProgress(renderRef);

    std::filesystem::create_directories(saveRenderFile.parent_path());
    hrRenderSaveFrameBufferLDR(renderRef, saveRenderFile.wstring().c_str());

    return check_images(ws2s(nameTest).c_str(), 1, 20);
  }


  bool test_123_emission()
  {
    std::wstring nameTest                 = L"test_123";
    std::filesystem::path libraryPath     = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile  = L"tests_images/" + nameTest + L"/z_out.png";
    std::filesystem::path saveRenderFile2 = L"tests_images/" + nameTest + L"/z_out2.png";
    std::filesystem::path saveRenderFile3 = L"tests_images/" + nameTest + L"/z_out3.png";

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

    HRMaterialRef matR = hrMaterialCreate(L"red_emission");
    HRMaterialRef matG = hrMaterialCreate(L"green_emission");
    HRMaterialRef matB = hrMaterialCreate(L"blue_emission");

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

    ////////////////////
    ////////////////////

    hrMaterialOpen(matR, HR_WRITE_DISCARD);
    {
      auto matNode  = hrMaterialParamNode(matR);
      auto emission = matNode.append_child(L"emission");

      emission.append_child(L"multiplier").append_attribute(L"val").set_value(1.0f);
      emission.append_child(L"color").append_attribute(L"val").set_value(L"1.0 0.0 0.0");

  	  VERIFY_XML(matNode);
    }
    hrMaterialClose(matR);

    hrMaterialOpen(matG, HR_WRITE_DISCARD);
    {
      auto matNode  = hrMaterialParamNode(matG);
      auto emission = matNode.append_child(L"emission");

      emission.append_child(L"multiplier").append_attribute(L"val").set_value(1.0f);
      emission.append_child(L"color").append_attribute(L"val").set_value(L"0.0 1.0 0.0");

     VERIFY_XML(matNode);
    }
    hrMaterialClose(matG);

    hrMaterialOpen(matB, HR_WRITE_DISCARD);
    {
      auto matNode  = hrMaterialParamNode(matB);
      auto emission = matNode.append_child(L"emission");

      emission.append_child(L"multiplier").append_attribute(L"val").set_value(1.0f);
      emission.append_child(L"color").append_attribute(L"val").set_value(L"0.0 0.0 1.0");

     VERIFY_XML(matNode);
    }
    hrMaterialClose(matB);

    ////////////////////
    ////////////////////
    ////////////////////
    ////////////////////

    HRMeshRef cubeOpenRef = hrMeshCreate(L"my_box");
    HRMeshRef sphereRef[3];
    {
      sphereRef[0] = hrMeshCreate(L"my_sphere1");
      sphereRef[1] = hrMeshCreate(L"my_sphere2");
      sphereRef[2] = hrMeshCreate(L"my_sphere3");
    }

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

    int32_t matsId[3] = {matR.id, matG.id, matB.id};
    for(int i=0;i<3;i++)
    {
      hrMeshOpen(sphereRef[i], HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
      {
        hrMeshVertexAttribPointer4f(sphereRef[i], L"pos",      &sphere.vPos[0]);
        hrMeshVertexAttribPointer4f(sphereRef[i], L"norm",     &sphere.vNorm[0]);
        hrMeshVertexAttribPointer2f(sphereRef[i], L"texcoord", &sphere.vTexCoord[0]);

        hrMeshMaterialId(sphereRef[i], matsId[i]);
        hrMeshAppendTriangles3(sphereRef[i], int32_t(sphere.triIndices.size()), &sphere.triIndices[0]);
      }
      hrMeshClose(sphereRef[i]);
    }
    ////////////////////
    ////////////////////

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

    ////////////////////
    ////////////////////
    ////////////////////
    ////////////////////

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

    bool checkReqRed   = false;
    bool checkReqGreen = false;
    bool checkReqBlue  = false;

    for (int sceneId = 0; sceneId < 3; sceneId++) {

      hrSceneOpen(scnRef, HR_WRITE_DISCARD);
      {
        // instance sphere and cornell box
        //
        auto mtranslate = hlm::translate4x4(hlm::float3(0.0f, -1.5f, 1.0f));
        hrMeshInstance(scnRef, sphereRef[sceneId], mtranslate.L());

        auto mrot = hlm::rotate4x4Y(180.0f * DEG_TO_RAD);
        hrMeshInstance(scnRef, cubeOpenRef, mrot.L());

        mtranslate = hlm::translate4x4(hlm::float3(0, 3.85f, 0));
        hrLightInstance(scnRef, rectLight, mtranslate.L());
      }
      hrSceneClose(scnRef);

      hrFlush(scnRef, renderRef, camRef);

      ////////////////////
      // Rendering, save and check image
      ////////////////////

      RenderProgress(renderRef);

      std::filesystem::create_directories(saveRenderFile.parent_path());

      if (sceneId == 0)
        hrRenderSaveFrameBufferLDR(renderRef, saveRenderFile.wstring().c_str());        
      else if (sceneId == 1)
        hrRenderSaveFrameBufferLDR(renderRef, saveRenderFile2.wstring().c_str());
      else
        hrRenderSaveFrameBufferLDR(renderRef, saveRenderFile3.wstring().c_str());


      if (sceneId == 0)
      {
        std::vector<float> imageHDR(512 * 512 * 4);
        hrRenderGetFrameBufferHDR4f(renderRef, 512, 512, imageHDR.data());

        float rgba[3] = { 0,0,0 };
        rgba[0] = imageHDR[4 * (220 * 512 + 256) + 0];
        rgba[1] = imageHDR[4 * (220 * 512 + 256) + 1];
        rgba[2] = imageHDR[4 * (220 * 512 + 256) + 2];

        checkReqRed = (fabs(rgba[0] - 1.0f) < 1e-5f && fabs(rgba[1]) < 1.0e-5f && fabs(rgba[2]) < 1.0e-5f);
      }
      else if (sceneId == 1)
      {
        std::vector<float> imageHDR(512 * 512 * 4);
        hrRenderGetFrameBufferHDR4f(renderRef, 512, 512, imageHDR.data());

        float rgba[3] = { 0,0,0 };
        rgba[0] = imageHDR[4 * (220 * 512 + 256) + 0];
        rgba[1] = imageHDR[4 * (220 * 512 + 256) + 1];
        rgba[2] = imageHDR[4 * (220 * 512 + 256) + 2];

        checkReqGreen = (fabs(rgba[0]) < 1e-5f && fabs(rgba[1] - 1.0f) < 1.0e-5f && fabs(rgba[2]) < 1.0e-5f);
      }
      else if (sceneId == 2)
      {
        std::vector<float> imageHDR(512 * 512 * 4);
        hrRenderGetFrameBufferHDR4f(renderRef, 512, 512, imageHDR.data());

        float rgba[3] = { 0,0,0 };
        rgba[0] = imageHDR[4 * (220 * 512 + 256) + 0];
        rgba[1] = imageHDR[4 * (220 * 512 + 256) + 1];
        rgba[2] = imageHDR[4 * (220 * 512 + 256) + 2];

        checkReqBlue = (fabs(rgba[0]) < 1e-5f && fabs(rgba[1]) < 1.0e-5f && fabs(rgba[2] - 1.0f) < 1.0e-5f);
      }
    }

    return check_images(ws2s(nameTest).c_str(), 3, 10) && checkReqRed && checkReqGreen && checkReqBlue;
  }


  bool test_124_emission_texture()
  {
    std::wstring nameTest                = L"test_124";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);

    ////////////////////
    // Materials
    ////////////////////

    HRMaterialRef matR = hrMaterialCreate(L"matR");
    HRMaterialRef matG = hrMaterialCreate(L"matG");
    HRMaterialRef matB = hrMaterialCreate(L"matB");
    HRMaterialRef matBG = hrMaterialCreate(L"matBG");
    HRMaterialRef matGray = hrMaterialCreate(L"matGray");

    HRTextureNodeRef texChecker = hrTexture2DCreateFromFile(L"data/textures/chess_white.bmp");
    HRTextureNodeRef tex = hrTexture2DCreateFromFile(L"data/textures/blur_pattern.bmp");

    hrMaterialOpen(matR, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matR);

      auto emission = matNode.append_child(L"emission");
      emission.append_child(L"multiplier").append_attribute(L"val").set_value(1.0f);

      auto color = emission.append_child(L"color");
      color.append_attribute(L"val").set_value(L"1.0 0.0 0.0");
      color.append_attribute(L"tex_apply_mode ").set_value(L"multiply");

			auto texNode = hrTextureBind(tex, emission.child(L"color"));

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 2, 0, 0, 0,
                                  0, 2, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };
      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);
			VERIFY_XML(matNode);
    }
    hrMaterialClose(matR);

    hrMaterialOpen(matG, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matG);

      auto emission = matNode.append_child(L"emission");
      emission.append_child(L"multiplier").append_attribute(L"val").set_value(1.0f);

      auto color = emission.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.0 1.0 0.0");
      color.append_attribute(L"tex_apply_mode ").set_value(L"multiply");

			auto texNode = hrTextureBind(texChecker, emission.child(L"color"));

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 8, 0, 0, 0,
                                  0, 8, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };
      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);
			VERIFY_XML(matNode);
    }
    hrMaterialClose(matG);

    hrMaterialOpen(matB, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matB);

      auto emission = matNode.append_child(L"emission");
      emission.append_child(L"multiplier").append_attribute(L"val").set_value(1.0f);

      auto color = emission.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.0 0.0 1.0");
      color.append_attribute(L"tex_apply_mode ").set_value(L"multiply");

			auto texNode = hrTextureBind(tex, emission.child(L"color"));

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 2, 0, 0, 0,
                                  0, 2, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };
      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);
			VERIFY_XML(matNode);
    }
    hrMaterialClose(matB);

    hrMaterialOpen(matBG, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matBG);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.25 0.25 1.0");
      color.append_attribute(L"tex_apply_mode ").set_value(L"multiply");

			auto texNode = hrTextureBind(texChecker, color);

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 32, 0, 0, 0,
                                  0, 16, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };
      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);
			VERIFY_XML(matNode);
    }
    hrMaterialClose(matBG);

    hrMaterialOpen(matGray, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGray);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      color.append_attribute(L"tex_apply_mode ").set_value(L"replace");

			auto texNode = hrTextureBind(texChecker, color);

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 16, 0, 0, 0,
                                  0, 16, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };
      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);
			VERIFY_XML(matNode);
    }
    hrMaterialClose(matGray);

    ////////////////////
    // Meshes
    ////////////////////
    HRMeshRef sph1 = HRMeshFromSimpleMesh(L"sph1", CreateSphere(2.0f, 64), matR.id);
    HRMeshRef sph2 = HRMeshFromSimpleMesh(L"sph2", CreateSphere(2.0f, 64), matG.id);
    HRMeshRef sph3 = HRMeshFromSimpleMesh(L"sph3", CreateSphere(2.0f, 64), matB.id);
    HRMeshRef boxBG = HRMeshFromSimpleMesh(L"boxBG", CreateCube(1.0f), matBG.id);
    HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), matGray.id);

    ////////////////////
    // Light
    ////////////////////

    HRLightRef rectLight = hrLightCreate(L"my_area_light");

    hrLightOpen(rectLight, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(rectLight);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"rect");
      lightNode.attribute(L"distribution").set_value(L"diffuse");

      auto sizeNode = lightNode.append_child(L"size");

      sizeNode.append_attribute(L"half_length").set_value(L"10.0");
      sizeNode.append_attribute(L"half_width").set_value(L"10.0");

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(2.0f*IRRADIANCE_TO_RADIANCE);
			VERIFY_XML(lightNode);
    }
    hrLightClose(rectLight);

    ////////////////////
    // Camera
    ////////////////////

    HRCameraRef camRef = hrCameraCreate(L"my camera");

    hrCameraOpen(camRef, HR_WRITE_DISCARD);
    {
      auto camNode = hrCameraParamNode(camRef);

      camNode.append_child(L"fov").text().set(L"45");
      camNode.append_child(L"nearClipPlane").text().set(L"0.01");
      camNode.append_child(L"farClipPlane").text().set(L"100.0");

      camNode.append_child(L"up").text().set(L"0 1 0");
      camNode.append_child(L"position").text().set(L"0 1 16");
      camNode.append_child(L"look_at").text().set(L"0 1 0");
    }
    hrCameraClose(camRef);

    ////////////////////
    // Render settings
    ////////////////////

    HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 512, 512, 256, 2048);


    ////////////////////
    // Create scene
    ////////////////////

    HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

    using namespace LiteMath;

    float4x4 mRot;
    float4x4 mTranslate;
    float4x4 mScale;
    float4x4 mRes;

    hrSceneOpen(scnRef, HR_WRITE_DISCARD);

    ///////////
    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0.0f, -1.0f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, planeRef, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(-4.25f, 1.25f, 4.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph1, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0.0f, 1.25f, 4.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph2, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(4.25f, 1.25f, 4.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph3, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();
    mRot.identity();

    const float DEG_TO_RAD = 0.01745329251f; // float(3.14159265358979323846f) / 180.0f;

    mTranslate = translate4x4(float3(0.0f, 1.0f, -4.0f));
    mScale = scale4x4(float3(16.0f, 8.0f, 0.5f));
    mRes = mul(mScale, mRes);
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, boxBG, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0, 15.0f, 0.0));
    mRes = mul(mTranslate, mRes);

    hrLightInstance(scnRef, rectLight, mRes.L());

    ///////////

    hrSceneClose(scnRef);

    hrFlush(scnRef, renderRef);
    
    ////////////////////
    // Rendering, save and check image
    ////////////////////

    RenderProgress(renderRef);

    std::filesystem::create_directories(saveRenderFile.parent_path());
    hrRenderSaveFrameBufferLDR(renderRef, saveRenderFile.wstring().c_str());

    return check_images(ws2s(nameTest).c_str(), 1, 10);
  }


  bool test_125_emission_cast_gi()
  {
    std::wstring nameTest                = L"test_125";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);

    ////////////////////
    // Materials
    ////////////////////

    HRMaterialRef matR = hrMaterialCreate(L"matR");
    HRMaterialRef matG = hrMaterialCreate(L"matG");
    HRMaterialRef matB = hrMaterialCreate(L"matB");
    HRMaterialRef matBG = hrMaterialCreate(L"matBG");
    HRMaterialRef matGray = hrMaterialCreate(L"matGray");

    HRTextureNodeRef texChecker = hrTexture2DCreateFromFile(L"data/textures/chess_white.bmp");
    HRTextureNodeRef tex = hrTexture2DCreateFromFile(L"data/textures/blur_pattern.bmp");

    hrMaterialOpen(matR, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matR);

      auto emission = matNode.append_child(L"emission");
      emission.append_child(L"multiplier").append_attribute(L"val").set_value(1.0f);
      emission.append_child(L"cast_gi").append_attribute(L"val").set_value(1);

      auto color = emission.append_child(L"color");
      color.append_attribute(L"val").set_value(L"1.0 0.0 0.0");
      color.append_attribute(L"tex_apply_mode ").set_value(L"multiply");

			auto texNode = hrTextureBind(tex, emission.child(L"color"));

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 2, 0, 0, 0,
                                  0, 2, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };
      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);
			VERIFY_XML(matNode);
    }
    hrMaterialClose(matR);

    hrMaterialOpen(matG, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matG);

      auto refl = matNode.append_child(L"reflectivity");

      refl.append_attribute(L"brdf_type").set_value(L"phong");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(1.0f);
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(16.0f);
			VERIFY_XML(matNode);
    }
    hrMaterialClose(matG);

    hrMaterialOpen(matB, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matB);

      auto emission = matNode.append_child(L"emission");
      emission.append_child(L"multiplier").append_attribute(L"val").set_value(4.0f);
      emission.append_child(L"cast_gi").append_attribute(L"val").set_value(0);

      auto color = emission.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.0 0.7 1.0");
      color.append_attribute(L"tex_apply_mode ").set_value(L"multiply");

			auto texNode = hrTextureBind(tex, emission.child(L"color"));

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 2, 0, 0, 0,
                                  0, 2, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };
      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);
			VERIFY_XML(matNode);
    }
    hrMaterialClose(matB);

    hrMaterialOpen(matBG, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matBG);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.25 0.25 1.0");
      color.append_attribute(L"tex_apply_mode ").set_value(L"multiply");

			auto texNode = hrTextureBind(texChecker, color);

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 32, 0, 0, 0,
                                  0, 16, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };
      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);
			VERIFY_XML(matNode);
    }
    hrMaterialClose(matBG);

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

    ////////////////////
    // Meshes
    ////////////////////
    HRMeshRef sph1 = HRMeshFromSimpleMesh(L"sph1", CreateSphere(2.0f, 64), matR.id);
    HRMeshRef sph2 = HRMeshFromSimpleMesh(L"sph2", CreateSphere(2.0f, 64), matG.id);
    HRMeshRef sph3 = HRMeshFromSimpleMesh(L"sph3", CreateSphere(2.0f, 64), matB.id);
    HRMeshRef boxBG = HRMeshFromSimpleMesh(L"boxBG", CreateCube(1.0f), matBG.id);
    HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), matGray.id);

    ////////////////////
    // Light
    ////////////////////

    HRLightRef rectLight = hrLightCreate(L"my_area_light");

    hrLightOpen(rectLight, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(rectLight);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"rect");
      lightNode.attribute(L"distribution").set_value(L"diffuse");

      auto sizeNode = lightNode.append_child(L"size");

      sizeNode.append_attribute(L"half_length").set_value(L"5.0");
      sizeNode.append_attribute(L"half_width").set_value(L"5.0");

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(2.0*IRRADIANCE_TO_RADIANCE);
			VERIFY_XML(lightNode);
    }
    hrLightClose(rectLight);

    ////////////////////
    // Camera
    ////////////////////

    HRCameraRef camRef = hrCameraCreate(L"my camera");

    hrCameraOpen(camRef, HR_WRITE_DISCARD);
    {
      auto camNode = hrCameraParamNode(camRef);

      camNode.append_child(L"fov").text().set(L"45");
      camNode.append_child(L"nearClipPlane").text().set(L"0.01");
      camNode.append_child(L"farClipPlane").text().set(L"100.0");

      camNode.append_child(L"up").text().set(L"0 1 0");
      camNode.append_child(L"position").text().set(L"0 1 16");
      camNode.append_child(L"look_at").text().set(L"0 1 0");
    }
    hrCameraClose(camRef);

    ////////////////////
    // Render settings
    ////////////////////

    HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 1024, 768, 256, 2048);


    ////////////////////
    // Create scene
    ////////////////////

    HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

    using namespace LiteMath;

    float4x4 mRot;
    float4x4 mTranslate;
    float4x4 mScale;
    float4x4 mRes;

    hrSceneOpen(scnRef, HR_WRITE_DISCARD);

    ///////////
    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0.0f, -1.0f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, planeRef, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(-4.25f, 1.25f, 4.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph1, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0.0f, 2.25f, 4.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph2, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(4.25f, 1.25f, 4.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph3, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();
    mRot.identity();

    const float DEG_TO_RAD = 0.01745329251f; // float(3.14159265358979323846f) / 180.0f;

    mTranslate = translate4x4(float3(0.0f, 1.0f, -4.0f));
    mScale = scale4x4(float3(16.0f, 8.0f, 0.5f));
    mRes = mul(mScale, mRes);
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, boxBG, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0, 15.0f, 0.0));
    mRes = mul(mTranslate, mRes);

    hrLightInstance(scnRef, rectLight, mRes.L());

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


  bool test_126_bump_amount()
  {
    std::wstring nameTest                = L"test_126";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);

    ////////////////////
    // Materials
    ////////////////////

    auto mat025     = hrMaterialCreate(L"mat025");
    auto mat050     = hrMaterialCreate(L"mat050");
    auto mat090     = hrMaterialCreate(L"mat090");
    auto matGray    = hrMaterialCreate(L"matGray");
    auto texChecker = hrTexture2DCreateFromFile(L"data/textures/chess_white.bmp");
    auto texBump    = hrTexture2DCreateFromFile(L"data/textures/ornament.jpg");

    

    hrMaterialOpen(mat025, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(mat025);
      AddDiffuseNode(matNode, L"0.5 0.5 0.5");      
      AddReliefNode(matNode, L"height_bump", 0.25F, texBump, L"wrap", L"wrap", 2, 2);
    }
    hrMaterialClose(mat025);

    hrMaterialOpen(mat050, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(mat050);
      AddDiffuseNode(matNode, L"0.5 0.5 0.5");
      AddReliefNode(matNode, L"height_bump", 0.5F, texBump, L"wrap", L"wrap", 2, 2);
    }
    hrMaterialClose(mat050);

    hrMaterialOpen(mat090, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(mat090);
      AddDiffuseNode(matNode, L"0.5 0.5 0.5");
      AddReliefNode(matNode, L"height_bump", 0.9F, texBump, L"wrap", L"wrap", 2, 2);
    }
    hrMaterialClose(mat090);

    hrMaterialOpen(matGray, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGray);
      AddDiffuseNode(matNode, L"0.5 0.5 0.5", L"Lambert", 0, texChecker, L"wrap", L"wrap", 16, 16);
    }
    hrMaterialClose(matGray);

    ////////////////////
    // Meshes
    ////////////////////

    auto sph1Ref  = HRMeshFromSimpleMesh(L"sph1", CreateSphere(2, 64), mat025.id);
    auto sph2Ref  = HRMeshFromSimpleMesh(L"sph2", CreateSphere(2, 64), mat050.id);
    auto sph3Ref  = HRMeshFromSimpleMesh(L"sph3", CreateSphere(2, 64), mat090.id);
    auto planeRef = HRMeshFromSimpleMesh(L"plane", CreatePlane(20), matGray.id);

    ////////////////////
    // Light
    ////////////////////

    auto rectLight = CreateLight(L"Light01", L"area", L"rect", L"diffuse", 10, 10, L"1 1 1", 2.0f * IRRADIANCE_TO_RADIANCE);

    ////////////////////
    // Camera
    ////////////////////

    CreateCamera(45, L"0 10 8", L"0 0 0");

    ////////////////////
    // Render settings
    ////////////////////

    auto renderRef = CreateBasicTestRenderPTNoCaust(CURR_RENDER_DEVICE, 1024, 768, 64, 64);

    ////////////////////
    // Create scene
    ////////////////////

    auto scnRef = hrSceneCreate((L"scene_" + nameTest).c_str());

    hrSceneOpen(scnRef, HR_WRITE_DISCARD);

    AddMeshToScene(scnRef, planeRef, float3(0, -1, 0));
    AddMeshToScene(scnRef, sph1Ref, float3(-4.25f, 1.25f, 0));
    AddMeshToScene(scnRef, sph2Ref, float3(0, 1.25f, 0));
    AddMeshToScene(scnRef, sph3Ref, float3(4.25f, 1.25f, 0));
    AddLightToScene(scnRef, rectLight, float3(0, 17, 0));

    hrSceneClose(scnRef);
    hrFlush(scnRef, renderRef);

    ////////////////////
    // Rendering, save and check image
    ////////////////////

    RenderProgress(renderRef);

    std::filesystem::create_directories(saveRenderFile.parent_path());
    hrRenderSaveFrameBufferLDR(renderRef, saveRenderFile.wstring().c_str());

    hrRenderLogDir(renderRef, L"C:/[Hydra]/logs/", true);

    return check_images(ws2s(nameTest).c_str(), 1);
  }


  bool test_127_normal_map_height()
  {
    std::wstring nameTest                = L"test_127";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);

    ////////////////////
    // Materials
    ////////////////////

    HRMaterialRef mat025 = hrMaterialCreate(L"mat025");
    HRMaterialRef mat050 = hrMaterialCreate(L"mat050");
    HRMaterialRef mat090 = hrMaterialCreate(L"mat090");
    HRMaterialRef matGray = hrMaterialCreate(L"matGray");

    HRTextureNodeRef texChecker = hrTexture2DCreateFromFile(L"data/textures/chess_white.bmp");
    HRTextureNodeRef tex        = hrTexture2DCreateFromFile(L"data/textures/normal_map.jpg");
    HRTextureNodeRef tex2       = hrTexture2DCreateFromFile(L"data/textures/Scifi_normal.jpg");

    hrMaterialOpen(mat025, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(mat025);

      auto diffuse = matNode.append_child(L"diffuse");
      diffuse.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 0.5");

      auto displacement = matNode.append_child(L"displacement");
      auto heightNode   = displacement.append_child(L"normal_map");

      displacement.append_attribute(L"type").set_value(L"normal_bump");
      
      auto invert = heightNode.append_child(L"invert");
      invert.append_attribute(L"x").set_value(0);
      invert.append_attribute(L"y").set_value(0);

			auto texNode = hrTextureBind(tex, heightNode);

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 2, 0, 0, 0,
                                  0, 2, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };

      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(1.0f);            // !!! this is important for normalmap !!!
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);
			VERIFY_XML(matNode);
    }
    hrMaterialClose(mat025);

    hrMaterialOpen(mat050, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(mat050);

      auto diffuse = matNode.append_child(L"diffuse");
      diffuse.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 0.5");

      auto displacement = matNode.append_child(L"displacement");
      auto heightNode = displacement.append_child(L"normal_map");

      displacement.append_attribute(L"type").set_value(L"normal_bump");

      auto invert = heightNode.append_child(L"invert");
      invert.append_attribute(L"x").set_value(0);
      invert.append_attribute(L"y").set_value(0);

      auto texNode = hrTextureBind(tex2, heightNode);

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 2, 0, 0, 0,
                                  0, 2, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };

      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(1.0f);              // !!! this is important for normalmap !!!
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);
			VERIFY_XML(matNode);
    }
    hrMaterialClose(mat050);

    hrMaterialOpen(mat090, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(mat090);

      auto diffuse = matNode.append_child(L"diffuse");
      diffuse.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 0.5");

      auto displacement = matNode.append_child(L"displacement");
      auto heightNode = displacement.append_child(L"normal_map");

      displacement.append_attribute(L"type").set_value(L"normal_bump");

      auto invert = heightNode.append_child(L"invert");
      invert.append_attribute(L"x").set_value(0);
      invert.append_attribute(L"y").set_value(0);


      auto texNode = hrTextureBind(tex, heightNode);

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 2, 0, 0, 0,
                                  0, 2, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);
			VERIFY_XML(matNode);
    }
    hrMaterialClose(mat090);

    hrMaterialOpen(matGray, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGray);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.5 0.5 0.5");
      color.append_attribute(L"tex_apply_mode ").set_value(L"multiply");

			auto texNode = hrTextureBind(texChecker, color);

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 16, 0, 0, 0,
                                   0, 16, 0, 0,
                                   0, 0, 1, 0,
                                   0, 0, 0, 1 };

      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f); 
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);
			VERIFY_XML(matNode);
    }
    hrMaterialClose(matGray);

    ////////////////////
    // Meshes
    ////////////////////
    HRMeshRef sph1 = HRMeshFromSimpleMesh(L"sph1", CreateSphere(2.0f, 64), mat025.id);
    HRMeshRef sph2 = HRMeshFromSimpleMesh(L"sph2", CreateSphere(2.0f, 64), mat050.id);
    HRMeshRef sph3 = HRMeshFromSimpleMesh(L"sph3", CreateSphere(2.0f, 64), mat090.id);
    HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), matGray.id);

    ////////////////////
    // Light
    ////////////////////

    HRLightRef rectLight = hrLightCreate(L"my_area_light");

    hrLightOpen(rectLight, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(rectLight);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"rect");
      lightNode.attribute(L"distribution").set_value(L"diffuse");

      auto sizeNode = lightNode.append_child(L"size");

      sizeNode.append_attribute(L"half_length").set_value(L"10.0");
      sizeNode.append_attribute(L"half_width").set_value(L"10.0");

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(2.0*IRRADIANCE_TO_RADIANCE);
			VERIFY_XML(lightNode);
    }
    hrLightClose(rectLight);

    ////////////////////
    // Camera
    ////////////////////

    HRCameraRef camRef = hrCameraCreate(L"my camera");

    hrCameraOpen(camRef, HR_WRITE_DISCARD);
    {
      auto camNode = hrCameraParamNode(camRef);

      camNode.append_child(L"fov").text().set(L"45");
      camNode.append_child(L"nearClipPlane").text().set(L"0.01");
      camNode.append_child(L"farClipPlane").text().set(L"100.0");

      camNode.append_child(L"up").text().set(L"0 1 0");
      camNode.append_child(L"position").text().set(L"0 10 8");
      camNode.append_child(L"look_at").text().set(L"0 0 0");
    }
    hrCameraClose(camRef);

    ////////////////////
    // Render settings
    ////////////////////

    HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 1024, 768, 256, 2048);


    ////////////////////
    // Create scene
    ////////////////////

    HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

    using namespace LiteMath;

    float4x4 mRot;
    float4x4 mTranslate;
    float4x4 mScale;
    float4x4 mRes;

    hrSceneOpen(scnRef, HR_WRITE_DISCARD);

    ///////////
    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0.0f, -1.0f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, planeRef, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(-4.25f, 1.25f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph1, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0.0f, 1.25f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph2, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(4.25f, 1.25f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph3, mRes.L());


    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0, 17.0f, 0.0));
    mRes = mul(mTranslate, mRes);

    hrLightInstance(scnRef, rectLight, mRes.L());

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


  bool test_128_bump_radius()
  {
    std::wstring nameTest                = L"test_128";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);

    ////////////////////
    // Materials
    ////////////////////

    HRMaterialRef mat10 = hrMaterialCreate(L"mat10");
    HRMaterialRef mat25 = hrMaterialCreate(L"mat25");
    HRMaterialRef mat50 = hrMaterialCreate(L"mat50");
    HRMaterialRef matGray = hrMaterialCreate(L"matGray");

    HRTextureNodeRef texChecker = hrTexture2DCreateFromFile(L"data/textures/chess_white.bmp");
    HRTextureNodeRef tex = hrTexture2DCreateFromFile(L"data/textures/300px-Bump2.jpg");

    hrMaterialOpen(mat10, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(mat10);

       auto diffuse = matNode.append_child(L"diffuse");
      diffuse.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 0.5");

      auto displacement = matNode.append_child(L"displacement");
      auto heightNode   = displacement.append_child(L"height_map");

      displacement.append_attribute(L"type").set_value(L"height_bump");
      heightNode.append_attribute(L"amount").set_value(1.0f);
      heightNode.append_attribute(L"smooth_lvl").set_value(0.1f);

			auto texNode = hrTextureBind(tex, heightNode);

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 4, 0, 0, 0,
                                  0, 4, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };
      
      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);
			VERIFY_XML(matNode);
    }
    hrMaterialClose(mat10);

    hrMaterialOpen(mat25, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(mat25);

      auto diffuse = matNode.append_child(L"diffuse");
      diffuse.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 0.5");

      auto displacement = matNode.append_child(L"displacement");
      auto heightNode   = displacement.append_child(L"height_map");

      displacement.append_attribute(L"type").set_value(L"height_bump");
      heightNode.append_attribute(L"amount").set_value(1.0f);
      heightNode.append_attribute(L"smooth_lvl").set_value(0.2f);

			auto texNode = hrTextureBind(tex, heightNode);

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 4, 0, 0, 0,
                                  0, 4, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };
      
      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);
			VERIFY_XML(matNode);
    }
    hrMaterialClose(mat25);

    hrMaterialOpen(mat50, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(mat50);

      auto diffuse = matNode.append_child(L"diffuse");
      diffuse.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 0.5");

      auto displacement = matNode.append_child(L"displacement");
      auto heightNode   = displacement.append_child(L"height_map");

      displacement.append_attribute(L"type").set_value(L"height_bump");
      heightNode.append_attribute(L"amount").set_value(1.0f);
      heightNode.append_attribute(L"smooth_lvl").set_value(0.5f);

			auto texNode = hrTextureBind(tex, heightNode);

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 4, 0, 0, 0,
                                  0, 4, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };
      
      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);
			VERIFY_XML(matNode);
    }
    hrMaterialClose(mat50);

    hrMaterialOpen(matGray, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGray);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.5 0.5 0.5");
      color.append_attribute(L"tex_apply_mode ").set_value(L"multiply");

			auto texNode = hrTextureBind(texChecker, color);

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 16, 0, 0, 0,
                                  0, 16, 0, 0,
                                  0, 0,  1, 0,
                                  0, 0,  0, 1 };

      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);
			VERIFY_XML(matNode);
    }
    hrMaterialClose(matGray);

    ////////////////////
    // Meshes
    ////////////////////
    HRMeshRef sph1 = HRMeshFromSimpleMesh(L"sph1", CreateSphere(2.0f, 64), mat10.id);
    HRMeshRef sph2 = HRMeshFromSimpleMesh(L"sph2", CreateSphere(2.0f, 64), mat25.id);
    HRMeshRef sph3 = HRMeshFromSimpleMesh(L"sph3", CreateSphere(2.0f, 64), mat50.id);
    HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), matGray.id);

    ////////////////////
    // Light
    ////////////////////

    HRLightRef rectLight = hrLightCreate(L"my_area_light");

    hrLightOpen(rectLight, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(rectLight);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"rect");
      lightNode.attribute(L"distribution").set_value(L"diffuse");

      auto sizeNode = lightNode.append_child(L"size");

      sizeNode.append_attribute(L"half_length").set_value(L"10.0");
      sizeNode.append_attribute(L"half_width").set_value(L"10.0");

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(1.0*IRRADIANCE_TO_RADIANCE);
			VERIFY_XML(lightNode);
    }
    hrLightClose(rectLight);

    ////////////////////
    // Camera
    ////////////////////

    HRCameraRef camRef = hrCameraCreate(L"my camera");

    hrCameraOpen(camRef, HR_WRITE_DISCARD);
    {
      auto camNode = hrCameraParamNode(camRef);

      camNode.append_child(L"fov").text().set(L"45");
      camNode.append_child(L"nearClipPlane").text().set(L"0.01");
      camNode.append_child(L"farClipPlane").text().set(L"100.0");

      camNode.append_child(L"up").text().set(L"0 1 0");
      camNode.append_child(L"position").text().set(L"0 10 8");
      camNode.append_child(L"look_at").text().set(L"0 0 0");
    }
    hrCameraClose(camRef);

    ////////////////////
    // Render settings
    ////////////////////

    HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 1024, 768, 256, 2048);


    ////////////////////
    // Create scene
    ////////////////////

    HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

    using namespace LiteMath;

    float4x4 mRot;
    float4x4 mTranslate;
    float4x4 mScale;
    float4x4 mRes;

    hrSceneOpen(scnRef, HR_WRITE_DISCARD);

    ///////////
    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0.0f, -1.0f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, planeRef, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(-4.25f, 1.25f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph1, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0.0f, 1.25f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph2, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(4.25f, 1.25f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph3, mRes.L());


    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0, 17.0f, 0.0));
    mRes = mul(mTranslate, mRes);

    hrLightInstance(scnRef, rectLight, mRes.L());

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


  bool test_129_phong_energy_fix()
  {
    std::wstring nameTest                = L"test_129";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

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

    hrMaterialOpen(mat0, HR_WRITE_DISCARD);
    {
      xml_node matNode = hrMaterialParamNode(mat0);
      auto refl        = matNode.append_child(L"reflectivity");

      refl.append_attribute(L"brdf_type").set_value(L"phong");
      refl.append_child(L"color").append_attribute(L"val")      = L"0.95 0.95 0.95";
      refl.append_child(L"glossiness").append_attribute(L"val") = 0.8f;
      refl.append_child(L"extrusion").append_attribute(L"val")  = L"maxcolor";
      refl.append_child(L"fresnel").append_attribute(L"val")    = 0;
      refl.append_child(L"energy_fix").append_attribute(L"val") = 1;

      VERIFY_XML(matNode);
    }
    hrMaterialClose(mat0);

    //std::cout << "cosPower(0.8) = " << testphong::cosPowerFromGlosiness(0.8f) << std::endl;

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

    ////////////////////
    ////////////////////
    ////////////////////
    ////////////////////

    HRMeshRef cubeOpenRef = hrMeshCreate(L"my_box");
    HRMeshRef planeRef    = hrMeshCreate(L"my_plane");
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
        sphere.matIndices[i] = mat0.id;

      hrMeshPrimitiveAttribPointer1i(sphereRef, L"mind", &sphere.matIndices[0]);
      hrMeshAppendTriangles3(sphereRef, int32_t(sphere.triIndices.size()), &sphere.triIndices[0]);
    }
    hrMeshClose(sphereRef);

    ////////////////////
    ////////////////////

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

    ////////////////////
    ////////////////////
    ////////////////////
    ////////////////////

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
    HRRenderRef renderRef = hrRenderCreate(L"HydraModern");
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

      node.append_child(L"trace_depth").text()      = 8;
      node.append_child(L"diff_trace_depth").text() = 4;
      node.append_child(L"maxRaysPerPixel").text()  = 2048;

      node.append_child(L"qmc_variant").text()      = 7;
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
      auto mtranslate = hlm::translate4x4(hlm::float3(0, -1.5, 1));
      hrMeshInstance(scnRef, sphereRef, mtranslate.L());

      auto mrot = hlm::rotate4x4Y(180.0f*DEG_TO_RAD);
      hrMeshInstance(scnRef, cubeOpenRef, mrot.L());

      //// instance light (!!!)
      //
      mtranslate = hlm::translate4x4(hlm::float3(0, 3.85f, 0));
      hrLightInstance(scnRef, rectLight, mtranslate.L());
    }
    hrSceneClose(scnRef);

    float glossValues[3]     = {0.5f, 0.6f, 0.7f};
    std::wstring outNames[3] = {L"tests_images/test_129/z_out.png",
                                L"tests_images/test_129/z_out2.png",
                                L"tests_images/test_129/z_out3.png"};

    for (int j = 0; j < 3; j++)
    {
      hrMaterialOpen(mat0, HR_OPEN_EXISTING);
      {
        xml_node matNode = hrMaterialParamNode(mat0);
        matNode.child(L"reflectivity").child(L"glossiness").attribute(L"val") = glossValues[j];
      }
      hrMaterialClose(mat0);

      std::cout << "cosPower(" << glossValues[j] << ") = " << testphong::cosPowerFromGlosiness(glossValues[j]) << std::endl;

      hrFlush(scnRef, renderRef, camRef);

      ////////////////////
      // Rendering, save and check image
      ////////////////////

      RenderProgress(renderRef);

      std::filesystem::create_directories(saveRenderFile.parent_path());
      hrRenderSaveFrameBufferLDR(renderRef, outNames[j].c_str());
    } // for (int j = 0; j < 3; j++)
    
    return check_images(ws2s(nameTest).c_str(), 1, 25);    
  }


  bool test_130_bump_invert_normalY()
  {
    std::wstring nameTest                = L"test_130";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);

    ////////////////////
    // Materials
    ////////////////////

    HRMaterialRef mat1 = hrMaterialCreate(L"mat1");
    HRMaterialRef mat2 = hrMaterialCreate(L"mat2");
    HRMaterialRef mat3 = hrMaterialCreate(L"mat3");
    HRMaterialRef matGray = hrMaterialCreate(L"matGray");

    HRTextureNodeRef texChecker = hrTexture2DCreateFromFile(L"data/textures/chess_white.bmp");
    HRTextureNodeRef tex = hrTexture2DCreateFromFile(L"data/textures/normal_map.jpg");

    hrMaterialOpen(mat1, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(mat1);

      auto diffuse = matNode.append_child(L"diffuse");
      diffuse.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 0.5");

      auto refl = matNode.append_child(L"reflectivity");
      refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.9");
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(8.0f);

      auto displacement = matNode.append_child(L"displacement");
      auto heightNode   = displacement.append_child(L"normal_map");

      displacement.append_attribute(L"type").set_value(L"normal_bump");
      
      auto invert = heightNode.append_child(L"invert");
      invert.append_attribute(L"x").set_value(0);
      invert.append_attribute(L"y").set_value(0);

			auto texNode = hrTextureBind(tex, heightNode);

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 2, 0, 0, 0,
                                  0, 2, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };

      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(1.0f);               // important for normal maps!!
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);
			VERIFY_XML(matNode);
    }
    hrMaterialClose(mat1);

    hrMaterialOpen(mat2, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(mat2);

      auto diffuse = matNode.append_child(L"diffuse");
      diffuse.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 0.5");

      auto refl = matNode.append_child(L"reflectivity");
      refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.9");
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(8.0f);

      auto displacement = matNode.append_child(L"displacement");
      auto heightNode = displacement.append_child(L"normal_map");

      displacement.append_attribute(L"type").set_value(L"normal_bump");

      auto invert = heightNode.append_child(L"invert");
      invert.append_attribute(L"x").set_value(1);
      invert.append_attribute(L"y").set_value(0);

      auto texNode = hrTextureBind(tex, heightNode);

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 2, 0, 0, 0,
                                  0, 2, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };

      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(1.0f);                // important for normal maps!!
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);
      VERIFY_XML(matNode);
    }
    hrMaterialClose(mat2);

    hrMaterialOpen(mat3, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(mat3);

      auto diffuse = matNode.append_child(L"diffuse");
      diffuse.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 0.5");

      auto refl = matNode.append_child(L"reflectivity");
      refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.9");
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(8.0f);

      auto displacement = matNode.append_child(L"displacement");
      auto heightNode   = displacement.append_child(L"normal_map");

      displacement.append_attribute(L"type").set_value(L"normal_bump");
      
      auto invert = heightNode.append_child(L"invert");
      invert.append_attribute(L"x").set_value(0);
      invert.append_attribute(L"y").set_value(1);

			auto texNode = hrTextureBind(tex, heightNode);

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 2, 0, 0, 0,
                                  0, 2, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };

      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(1.0f);                                 // important for normal maps!!
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);
			VERIFY_XML(matNode);
    }
    hrMaterialClose(mat3);

    hrMaterialOpen(matGray, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGray);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.5 0.5 0.5");
      color.append_attribute(L"tex_apply_mode ").set_value(L"multiply");

			auto texNode = hrTextureBind(texChecker, color);

      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 16, 0, 0, 0,
                                   0, 16, 0, 0,
                                   0, 0, 1, 0,
                                   0, 0, 0, 1 };

      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(2.2f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);
			VERIFY_XML(matNode);
    }
    hrMaterialClose(matGray);

    ////////////////////
    // Meshes
    ////////////////////
    HRMeshRef sph1 = HRMeshFromSimpleMesh(L"sph1", CreateSphere(2.0f, 64), mat1.id);
    HRMeshRef sph2 = HRMeshFromSimpleMesh(L"sph2", CreateSphere(2.0f, 64), mat2.id);
    HRMeshRef sph3 = HRMeshFromSimpleMesh(L"sph3", CreateSphere(2.0f, 64), mat3.id);
    HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), matGray.id);

    ////////////////////
    // Light
    ////////////////////

    HRLightRef rectLight = hrLightCreate(L"my_area_light");

    hrLightOpen(rectLight, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(rectLight);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"rect");
      lightNode.attribute(L"distribution").set_value(L"diffuse");

      auto sizeNode = lightNode.append_child(L"size");

      sizeNode.append_attribute(L"half_length").set_value(L"10.0");
      sizeNode.append_attribute(L"half_width").set_value(L"10.0");

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(1.0*IRRADIANCE_TO_RADIANCE);
			VERIFY_XML(lightNode);
    }
    hrLightClose(rectLight);

    ////////////////////
    // Camera
    ////////////////////

    HRCameraRef camRef = hrCameraCreate(L"my camera");

    hrCameraOpen(camRef, HR_WRITE_DISCARD);
    {
      auto camNode = hrCameraParamNode(camRef);

      camNode.append_child(L"fov").text().set(L"45");
      camNode.append_child(L"nearClipPlane").text().set(L"0.01");
      camNode.append_child(L"farClipPlane").text().set(L"100.0");

      camNode.append_child(L"up").text().set(L"0 1 0");
      camNode.append_child(L"position").text().set(L"0 10 8");
      camNode.append_child(L"look_at").text().set(L"0 0 0");
    }
    hrCameraClose(camRef);

    ////////////////////
    // Render settings
    ////////////////////

    HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 1024, 768, 256, 2048);


    ////////////////////
    // Create scene
    ////////////////////

    HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

    using namespace LiteMath;

    float4x4 mRot;
    float4x4 mTranslate;
    float4x4 mScale;
    float4x4 mRes;

    hrSceneOpen(scnRef, HR_WRITE_DISCARD);

    ///////////
    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0.0f, -1.0f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, planeRef, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(-4.25f, 1.25f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph1, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0.0f, 1.25f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph2, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(4.25f, 1.25f, 0.0f));
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, sph3, mRes.L());


    ///////////

    mTranslate.identity();
    mRes.identity();

    mTranslate = translate4x4(float3(0, 17.0f, 0.0));
    mRes = mul(mTranslate, mRes);

    hrLightInstance(scnRef, rectLight, mRes.L());

    ///////////

    hrSceneClose(scnRef);

    hrFlush(scnRef, renderRef);

    ////////////////////
    // Rendering, save and check image
    ////////////////////

    RenderProgress(renderRef);

    std::filesystem::create_directories(saveRenderFile.parent_path());
    hrRenderSaveFrameBufferLDR(renderRef, saveRenderFile.wstring().c_str());

    return check_images(ws2s(nameTest).c_str(), 1, 70);
  }
}