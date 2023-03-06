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
#include "simplerandom.h"

#include "HR_HDRImageTool.h"
#include "HydraXMLHelpers.h"

///////////////////////////////////////////////////////////////////////////////////

#pragma warning(disable:4996)

extern GLFWwindow* g_window;
using namespace TEST_UTILS;
namespace hlm = LiteMath;

HAPI HRMeshRef hrMeshCreateFromFileDL_NoNormals(const wchar_t* a_fileName);


namespace GEO_TESTS
{  
  static const int TEST_IMG_SIZE = 512;
  
  bool test_001_mesh_from_memory()
  {
    std::wstring nameTest                = L"test_001";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);
    
    ////////////////////
    // Materials
    ////////////////////
    
    auto mat0 = hrMaterialCreate(L"mat0");
    auto mat1 = hrMaterialCreate(L"mat1");
    
    hrMaterialOpen(mat0, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(mat0);
      AddDiffuseNode(matNode, L"0.5 0.75 0.5");
    }
    hrMaterialClose(mat0);
    
    hrMaterialOpen(mat1, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(mat1);
      AddDiffuseNode(matNode, L"0.25 0.25 0.25");
    }
    hrMaterialClose(mat1);
    
    ////////////////////
    // Meshes
    ////////////////////
    
    auto cube     = CreateCube(0.75f);
    auto plane    = CreatePlane(4.0f);
    auto cubeRef  = hrMeshCreate(L"my_cube");
    auto planeRef = hrMeshCreate(L"my_plane");    
    
    hrMeshOpen(cubeRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
    {
      hrMeshVertexAttribPointer4f(cubeRef, L"pos", &cube.vPos[0]);
      hrMeshVertexAttribPointer4f(cubeRef, L"norm", &cube.vNorm[0]);
      hrMeshVertexAttribPointer2f(cubeRef, L"texcoord", &cube.vTexCoord[0]);
      
      hrMeshMaterialId(cubeRef, mat0.id);
      
      hrMeshAppendTriangles3(cubeRef, int(cube.triIndices.size()), &cube.triIndices[0]);
    }
    hrMeshClose(cubeRef);
    
    hrMeshOpen(planeRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
    {
      hrMeshVertexAttribPointer4f(planeRef, L"pos", &plane.vPos[0]);
      hrMeshVertexAttribPointer4f(planeRef, L"norm", &plane.vNorm[0]);
      hrMeshVertexAttribPointer2f(planeRef, L"texcoord", &plane.vTexCoord[0]);
      
      hrMeshMaterialId(planeRef, mat1.id);
      hrMeshAppendTriangles3(planeRef, int32_t(plane.triIndices.size()), &plane.triIndices[0]);
    }
    hrMeshClose(planeRef);
    
    ////////////////////
    // Light
    ////////////////////
    
    auto rectLight = CreateLight(L"Light01", L"area", L"rect", L"diffuse", 1.0f, 1.0f, L"1 1 1", 8.0f * IRRADIANCE_TO_RADIANCE);
        
    ////////////////////
    // Camera
    ////////////////////
    
    CreateCamera(30, L"0 1.5 15", L"0 1.5 0");        
    
    ////////////////////
    // Render settings
    ////////////////////
    
    auto renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, TEST_IMG_SIZE, TEST_IMG_SIZE, 256, 256);
    
    ////////////////////
    // Create scene
    ////////////////////
    
    auto scnRef = hrSceneCreate((L"scene_" + nameTest).c_str());
    
    hrSceneOpen(scnRef, HR_WRITE_DISCARD);
    
    AddMeshToScene(scnRef, planeRef, float3(0, -1, 0));
    AddMeshToScene(scnRef, cubeRef, float3(-2, 0, 0));
    AddMeshToScene(scnRef, cubeRef, float3(2, 0, 0));
    AddLightToScene(scnRef, rectLight, float3(0, 3.85f, 0));
    
    hrSceneClose(scnRef);    
    hrFlush(scnRef, renderRef);
    
    ////////////////////
    // Rendering, save and check image
    ////////////////////

    RenderProgress(renderRef);

    std::filesystem::create_directories(saveRenderFile.parent_path());
    hrRenderSaveFrameBufferLDR(renderRef, saveRenderFile.wstring().c_str());

    return check_images(ws2s(nameTest).c_str());
  }
  


  bool test_002_mesh_from_vsgf()
  {
    std::wstring nameTest                = L"test_002";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);
    
    ////////////////////
    // Materials
    ////////////////////
    
    //the mesh we're going to load has material id attribute set to 1
    //so we first need to add the material with id 0 and then add material for the mesh
    auto mat0_unused = hrMaterialCreate(L"mat0_unused");
    auto mat1        = hrMaterialCreate(L"mat1");
    
    hrMaterialOpen(mat1, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(mat1);
      AddDiffuseNode(matNode, L"0.3 0.3 0.7");
    }
    hrMaterialClose(mat1);
    
    ////////////////////
    // Meshes
    ////////////////////
    
    auto lucyRef = hrMeshCreateFromFileDL(L"data/meshes/lucy.vsgf");
        
    ////////////////////
    // Light
    ////////////////////

    auto rectLight = CreateLight(L"Light01", L"area", L"rect", L"diffuse", 5.0f, 5.0f, L"1 1 1", 4.0f * IRRADIANCE_TO_RADIANCE);
        
    ////////////////////
    // Camera
    ////////////////////
    
    CreateCamera(35, L"0 0 15", L"0 0 0");
    
    ////////////////////
    // Render settings
    ////////////////////
    
    auto renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, TEST_IMG_SIZE, TEST_IMG_SIZE, 256, 1024);
    
    ////////////////////
    // Create scene
    ////////////////////
    
    auto scnRef = hrSceneCreate((L"scene_" + nameTest).c_str());
        
    hrSceneOpen(scnRef, HR_WRITE_DISCARD);
    
    AddMeshToScene(scnRef, lucyRef, float3(-0.5f, -4.0f, 0.0f));
    AddLightToScene(scnRef, rectLight, float3(0, 10.0f, 0));
            
    hrSceneClose(scnRef);    
    hrFlush(scnRef, renderRef);
    
    ////////////////////
    // Rendering, save and check image
    ////////////////////

    RenderProgress(renderRef);

    std::filesystem::create_directories(saveRenderFile.parent_path());
    hrRenderSaveFrameBufferLDR(renderRef, saveRenderFile.wstring().c_str());

    return check_images(ws2s(nameTest).c_str());
  }
  


  bool test_003_compute_normals()
  {
    std::wstring nameTest                = L"test_003";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);

    ////////////////////
    // Materials
    ////////////////////
    
    auto mat0 = hrMaterialCreate(L"mat0");
    auto mat1 = hrMaterialCreate(L"mat1");
    
    hrMaterialOpen(mat0, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(mat0);
      AddDiffuseNode(matNode, L"0.5 0.5 0.5");      
    }
    hrMaterialClose(mat0);
    
    hrMaterialOpen(mat1, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(mat1);
      AddDiffuseNode(matNode, L"0.3 0.3 0.85");      
    }
    hrMaterialClose(mat1);
    
    ////////////////////
    // Meshes
    ////////////////////
    
    auto teapotRef = hrMeshCreateFromFileDL_NoNormals(L"data/meshes/teapot.vsgf");
    //auto lucyRef   = hrMeshCreateFromFileDL_NoNormals(L"data/meshes/lucy.vsgf");

    //auto teapotRef = hrMeshCreateFromFileDL(L"data/meshes/teapot.vsgf");
    auto lucyRef = hrMeshCreateFromFileDL(L"data/meshes/lucy.vsgf");
    
    ////////////////////
    // Light
    ////////////////////

    auto rectLight = CreateLight(L"Light01", L"area", L"rect", L"diffuse", 5, 5, L"1 1 1", 8.0f * IRRADIANCE_TO_RADIANCE);
        
    ////////////////////
    // Camera
    ////////////////////

    CreateCamera(45, L"0 0 15", L"0 0 0");
    
    ////////////////////
    // Render settings
    ////////////////////
    
    auto renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, TEST_IMG_SIZE, TEST_IMG_SIZE, 256, 1024);
    
    ////////////////////
    // Create scene
    ////////////////////
    
    auto scnRef = hrSceneCreate((L"scene_"+ nameTest).c_str());

    hrSceneOpen(scnRef, HR_WRITE_DISCARD);

    AddMeshToScene (scnRef, lucyRef,   float3(3, -5, 0));
    AddMeshToScene (scnRef, teapotRef,  float3(-3, -1, 0), float3(), float3(4,4,4));
    AddLightToScene(scnRef, rectLight, float3(0, 10, 0));
    
    hrSceneClose(scnRef);    
    hrFlush(scnRef, renderRef);
    
    ////////////////////
    // Rendering, save and check image
    ////////////////////

    RenderProgress(renderRef);
    std::filesystem::create_directories(saveRenderFile.parent_path());
    hrRenderSaveFrameBufferLDR(renderRef, saveRenderFile.wstring().c_str());

    return check_images(ws2s(nameTest).c_str());    
  }
  

  bool test_004_dof()
  {
    std::wstring nameTest                = L"test_004";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);
        
    ////////////////////
    // Materials
    ////////////////////

    HRTextureNodeRef testTex2 = hrTexture2DCreateFromFile(L"data/textures/chess_red.bmp");
    
    HRMaterialRef mat0 = hrMaterialCreate(L"mysimplemat");
    HRMaterialRef mat1 = hrMaterialCreate(L"mysimplemat2");
    HRMaterialRef mat2 = hrMaterialCreate(L"wood");
    HRMaterialRef mat3 = hrMaterialCreate(L"rock");
    
    hrMaterialOpen(mat0, HR_WRITE_DISCARD);
    {
      xml_node matNode = hrMaterialParamNode(mat0);
      
      xml_node diff = matNode.append_child(L"diffuse");
      
      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").text().set(L"0.75 0.75 0.5");
      
      HRTextureNodeRef testTex = hrTexture2DCreateFromFile(L"data/textures/texture1.bmp");
      hrTextureBind(testTex, diff);
    }
    hrMaterialClose(mat0);
    
    hrMaterialOpen(mat1, HR_WRITE_DISCARD);
    {
      xml_node matNode = hrMaterialParamNode(mat1);
      
      xml_node diff = matNode.append_child(L"diffuse");
      
      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").text().set(L"1 1 1");
      
      hrTextureBind(testTex2, diff);
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
      
      HRTextureNodeRef testTex = hrTexture2DCreateFromFile(L"data/textures/163.jpg");
      hrTextureBind(testTex, diff);
    }
    hrMaterialClose(mat3);
    
    
    HRLightRef directLight = hrLightCreate(L"my_direct_light");
    
    hrLightOpen(directLight, HR_WRITE_DISCARD);
    {
      pugi::xml_node lightNode = hrLightParamNode(directLight);
      
      lightNode.attribute(L"type").set_value(L"directional");
      lightNode.attribute(L"shape").set_value(L"point");
      
      pugi::xml_node sizeNode = lightNode.append_child(L"size");
      
      sizeNode.append_child(L"inner_radius").append_attribute(L"val") = 10.0f;
      sizeNode.append_child(L"outer_radius").append_attribute(L"val") = 20.0f;
      
      pugi::xml_node intensityNode = lightNode.append_child(L"intensity");
      
      intensityNode.append_child(L"color").append_attribute(L"val")      = L"1 1 1";
      intensityNode.append_child(L"multiplier").append_attribute(L"val") = 2.0f*IRRADIANCE_TO_RADIANCE;
    }
    hrLightClose(directLight);
    
    HRLightRef sky = hrLightCreate(L"sky");
    
    hrLightOpen(sky, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(sky);
      
      lightNode.attribute(L"type").set_value(L"sky");
      
      auto intensityNode = lightNode.append_child(L"intensity");
      
      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"0.25 0.25 0.5");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(1.0f);
      
      VERIFY_XML(lightNode);
    }
    hrLightClose(sky);
    
    // camera
    //
    auto camRef = hrCameraCreate(L"my camera");
    
    hrCameraOpen(camRef, HR_WRITE_DISCARD);
    {
      xml_node camNode = hrCameraParamNode(camRef);
      
      camNode.append_child(L"fov").text().set(L"45");
      camNode.append_child(L"nearClipPlane").text().set(L"0.01");
      camNode.append_child(L"farClipPlane").text().set(L"100.0");
      
      camNode.append_child(L"up").text().set(L"0 1 0");
      camNode.append_child(L"position").text().set(L"-8 2 15");
      camNode.append_child(L"look_at").text().set(L"0.1 0 0.1");
      
      camNode.append_child(L"enable_dof").text()      = 1;
      camNode.append_child(L"dof_lens_radius").text() = 0.06f;
    }
    hrCameraClose(camRef);
    
        
    // geometry
    //
    auto cubeRef  = HRMeshFromSimpleMesh(L"my_cube", CreateCube(0.25f), 1);
    auto planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), 2);
    auto sphRef   = HRMeshFromSimpleMesh(L"my_sphere", CreateSphere(0.5f, 32), 3);
    auto torRef   = HRMeshFromSimpleMesh(L"my_torus", CreateTorus(0.1f, 0.3f, 32, 32), 0);

    
    // create scene
    //
    auto scnRef = hrSceneCreate(L"my scene");
    
    float	rtri = 25.0f; // Angle For The Triangle ( NEW )
    float	rquad = 40.0f;
    float g_FPS = 60.0f;
    int   frameCounter = 0;
    
    const float DEG_TO_RAD = float(3.14159265358979323846f) / 180.0f;
    
    float matrixT[4][4], matrixT2[4][4];
    float mRot1[4][4], mTranslate[4][4], mRes[4][4];
    float mTranslateDown[4][4];
    
    mat4x4_identity(mRot1);
    mat4x4_identity(mTranslate);
    mat4x4_identity(mRes);
    
    mat4x4_identity(mRes);
    mat4x4_translate(mTranslateDown, 0.0f, -1.0f, -5.0f);
    mat4x4_mul(mRes, mTranslateDown, mRes);
    mat4x4_transpose(matrixT2, mRes);
    
    ////
    ////
    
    HRMeshRef refs[3] = { cubeRef, sphRef, torRef };
    
    hrSceneOpen(scnRef, HR_WRITE_DISCARD);
    
    hrMeshInstance(scnRef, planeRef, &matrixT2[0][0]);
    
    auto rgen = simplerandom::RandomGenInit(878976);
    
    for (int i = -5; i <= 5; i++)
    {
      for (int j = -5; j <= 5; j++)
      {
        float xCoord = 2.5f*float(i);
        float yCoord = 2.5f*float(j);
        
        mat4x4_identity(mRot1);
        mat4x4_identity(mTranslate);
        mat4x4_identity(mRes);
        
        mat4x4_translate(mTranslate, xCoord, 0.1f, yCoord);
        mat4x4_rotate_X(mRot1, mRot1, simplerandom::rnd(rgen, 0.0f, 360.0f)*DEG_TO_RAD);
        mat4x4_rotate_Y(mRot1, mRot1, simplerandom::rnd(rgen, 0.0f, 360.0f)*DEG_TO_RAD*0.5f);
        mat4x4_mul(mRes, mTranslate, mRot1);
        mat4x4_transpose(matrixT, mRes); // this fucking math library swap rows and columns
        
        hrMeshInstance(scnRef, refs[simplerandom::rand(rgen) % 3], &matrixT[0][0]);
      }
    }
    
    // add directional light
    //
    mat4x4_identity(mRot1);
    mat4x4_identity(mTranslate);
    mat4x4_identity(mRes);
    
    mat4x4_identity(mRes);
    
    mat4x4_rotate_Z(mRot1, mRot1, 1.0f*DEG_TO_RAD);
    //mat4x4_rotate_Z(mRot1, mRot1, 0.0f*DEG_TO_RAD);
    mat4x4_translate(mTranslateDown, 0.0f, 100.0f, 0.0f);
    
    mat4x4_mul(mRes, mRot1, mRes);
    mat4x4_mul(mRes, mTranslateDown, mRes);
    
    mat4x4_transpose(matrixT2, mRes);
    hrLightInstance(scnRef, directLight, &matrixT2[0][0]);
    hrLightInstance(scnRef, sky,         &matrixT2[0][0]);
    
    hrSceneClose(scnRef);
    
    // set up render settings
    //
    HRRenderRef renderRef = hrRenderCreate(L"HydraModern"); // opengl1
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    auto pList = hrRenderGetDeviceList(renderRef);
    
    while (pList != nullptr)
    {
      std::wcout << L"device id = " << pList->id << L", name = " << pList->name << L", driver = " << pList->driver << std::endl;
      pList = pList->next;
    }
    
    hrRenderEnableDevice(renderRef, 0, true);
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    hrRenderOpen(renderRef, HR_WRITE_DISCARD);
    {
      pugi::xml_node node = hrRenderParamNode(renderRef);
      
      node.append_child(L"width").text()  = TEST_IMG_SIZE;
      node.append_child(L"height").text() = TEST_IMG_SIZE;
      
      node.append_child(L"method_primary").text() = L"pathtracing";
      node.append_child(L"method_secondary").text() = L"pathtracing";
      node.append_child(L"method_tertiary").text() = L"pathtracing";
      node.append_child(L"method_caustic").text() = L"pathtracing";
      node.append_child(L"shadows").text() = L"1";
      
      node.append_child(L"trace_depth").text() = L"5";
      node.append_child(L"diff_trace_depth").text() = L"3";
      
      node.append_child(L"maxRaysPerPixel").text() = L"1024";
      node.append_child(L"offline_pt").text()      = L"1";
    }
    hrRenderClose(renderRef);
    
    
    hrFlush(scnRef, renderRef);
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    ////////////////////
    // Rendering, save and check image
    ////////////////////

    RenderProgress(renderRef);

    std::filesystem::create_directories(saveRenderFile.parent_path());
    hrRenderSaveFrameBufferLDR(renderRef, saveRenderFile.wstring().c_str());

    return check_images(ws2s(nameTest).c_str(), 1, 20);
  }
  

  bool test_005_instancing()
  {
    std::wstring nameTest                = L"test_005";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);
        
    ////////////////////
    // Materials
    ////////////////////    
    
    HRMaterialRef matGray     = hrMaterialCreate(L"matGray");
    HRMaterialRef matTrunk    = hrMaterialCreate(L"Trunk");
    HRMaterialRef matWigglers = hrMaterialCreate(L"Wigglers");
    HRMaterialRef matBranches = hrMaterialCreate(L"Branches");
    HRMaterialRef matPllarRts = hrMaterialCreate(L"PillarRoots");
    HRMaterialRef matLeaves   = hrMaterialCreate(L"Leaves");
    HRMaterialRef matCanopy   = hrMaterialCreate(L"Canopy");
    HRMaterialRef matCube     = hrMaterialCreate(L"CubeMap");
    
    HRTextureNodeRef texPattern = hrTexture2DCreateFromFile(L"data/textures/bigleaf3.tga");
    HRTextureNodeRef texture1   = hrTexture2DCreateFromFile(L"data/textures/texture1.bmp");

    hrMaterialOpen(matCube, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matCube);
      
      auto diff = matNode.append_child(L"diffuse");
      
      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 0.5");
      
      hrTextureBind(texture1, diff.child(L"color"));
    }
    hrMaterialClose(matCube);
    
    hrMaterialOpen(matGray, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGray);
      
      auto diff = matNode.append_child(L"diffuse");
      
      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 0.5");
      
    }
    hrMaterialClose(matGray);
    
    hrMaterialOpen(matTrunk, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matTrunk);
      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").append_attribute(L"val").set_value(L"0.345098 0.215686 0.0117647");
    }
    hrMaterialClose(matTrunk);
    
    hrMaterialOpen(matWigglers, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matWigglers);
      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").append_attribute(L"val").set_value(L"0.345098 0.215686 0.0117647");
    }
    hrMaterialClose(matWigglers);
    
    hrMaterialOpen(matBranches, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matBranches);
      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").append_attribute(L"val").set_value(L"0.345098 0.215686 0.0117647");
    }
    hrMaterialClose(matBranches);
    
    hrMaterialOpen(matPllarRts, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matPllarRts);
      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").append_attribute(L"val").set_value(L"0.345098 0.215686 0.0117647");
    }
    hrMaterialClose(matPllarRts);
    
    //////////////////////////////////
    
    hrMaterialOpen(matLeaves, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matLeaves);
      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").append_attribute(L"val").set_value(L"0.0533333 0.208627 0.00627451");
      
      auto opacity = matNode.append_child(L"opacity");
      auto texNode = hrTextureBind(texPattern, opacity);
      
    }
    hrMaterialClose(matLeaves);
    
    hrMaterialOpen(matCanopy, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matCanopy);
      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").append_attribute(L"val").set_value(L"0.0941176 0.4 0");
    }
    hrMaterialClose(matCanopy);
    
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Meshes
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    HRMeshRef cubeR    = HRMeshFromSimpleMesh(L"cubeR", CreateCube(2.0f), matCube.id);
    HRMeshRef pillar   = HRMeshFromSimpleMesh(L"pillar", CreateCube(1.0f), matGray.id);
    HRMeshRef sphereG  = HRMeshFromSimpleMesh(L"sphereG", CreateSphere(4.0f, 64), matTrunk.id);
    HRMeshRef torusB   = HRMeshFromSimpleMesh(L"torusB", CreateTorus(0.8f, 2.0f, 64, 64), matTrunk.id);
    HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(10000.0f), matGray.id);
    
    HRMeshRef treeRef  = hrMeshCreateFromFileDL(L"data/meshes/bigtree.vsgf");
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Light
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    HRLightRef sky = hrLightCreate(L"sky");
    HRLightRef sun = hrLightCreate(L"sun");
    
    hrLightOpen(sky, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(sky);
      
      lightNode.attribute(L"type").set_value(L"sky");
      lightNode.attribute(L"distribution").set_value(L"perez");
      
      auto intensityNode = lightNode.append_child(L"intensity");
      
      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(1.0f);
      
      auto sunModel = lightNode.append_child(L"perez");
      
      sunModel.append_attribute(L"sun_id").set_value(sun.id);
      sunModel.append_attribute(L"turbidity").set_value(2.0f);
      
      VERIFY_XML(lightNode);
    }
    hrLightClose(sky);
    
    hrLightOpen(sun, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(sun);
      
      lightNode.attribute(L"type").set_value(L"directional");
      lightNode.attribute(L"shape").set_value(L"point");
      lightNode.attribute(L"distribution").set_value(L"directional");
      
      auto sizeNode = lightNode.append_child(L"size");
      sizeNode.append_attribute(L"inner_radius").set_value(L"0.0");
      sizeNode.append_attribute(L"outer_radius").set_value(L"10000.0");
      
      auto intensityNode = lightNode.append_child(L"intensity");
      
      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1.0 0.85 0.64");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(2.0f*IRRADIANCE_TO_RADIANCE);
      
      lightNode.append_child(L"shadow_softness").append_attribute(L"val").set_value(1.0f);
      
      VERIFY_XML(lightNode);
    }
    hrLightClose(sun);
    
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
      camNode.append_child(L"position").text().set(L"0 15 25");
      camNode.append_child(L"look_at").text().set(L"5 10 0");
    }
    hrCameraClose(camRef);
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Render settings
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    //HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, TEST_IMG_SIZE, TEST_IMG_SIZE, 256, 2048);
    
    
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
    
    auto rgen = simplerandom::RandomGenInit(125);
    
    {
      const float dist1 = 40.0f;
      const int SQUARESIZE1 = 100;
      
      for (int i = -SQUARESIZE1; i < SQUARESIZE1; i++)
      {
        for (int j = -SQUARESIZE1; j < SQUARESIZE1; j++)
        {
          const float2 randOffset = float2(simplerandom::rnd(rgen, -1.0f, 1.0f), simplerandom::rnd(rgen, -1.0f, 1.0f));
          const float3 pos        = dist1*float3(float(i), 0.0f, float(j)) + dist1*1.0f*float3(randOffset.x, 0.0f, randOffset.y);
          
          mTranslate = translate4x4(float3(pos.x, 1.0f, pos.z));
          mRot       = rotate4x4Y(simplerandom::rnd(rgen, -180.0f*DEG_TO_RAD, +180.0f*DEG_TO_RAD));
          mRes       = mul(mTranslate, mRot);
          
          hrMeshInstance(scnRef, cubeR, mRes.L());
        }
      }
    }
    
    ///////////
    {
      const float dist = 40.0f;
      const int SQUARESIZE = 100;
      
      for (int i = -SQUARESIZE; i < SQUARESIZE; i++)
      {
        for (int j = -SQUARESIZE; j < SQUARESIZE; j++)
        {
          const float2 randOffset = float2(simplerandom::rnd(rgen, -1.0f, 1.0f), simplerandom::rnd(rgen, -1.0f, 1.0f));
          const float3 pos = dist*float3(float(i), 0.0f, float(j)) + dist*0.5f*float3(randOffset.x, 0.0f, randOffset.y);
          
          mTranslate = translate4x4(pos);
          mScale     = scale4x4(float3(5.0f, 5.0f, 5.0f));
          mRot       = rotate4x4Y(simplerandom::rnd(rgen, -180.0f*DEG_TO_RAD, +180.0f*DEG_TO_RAD));
          mRes       = mul(mTranslate, mul(mRot, mScale));
          
          if ((simplerandom::rnd(rgen, 0.0f, 1.0f) > 0.5f))
            hrMeshInstance(scnRef, treeRef, mRes.L());
        }
      }
    }
    
    ///////////
    ///////////
    
    mRes.identity();
    hrLightInstance(scnRef, sky, mRes.L());
    
    ///////////
    
    mTranslate.identity();
    mRes.identity();
    mRot.identity();
    mRot2.identity();
    
    mTranslate = translate4x4(float3(200.0f, 200.0f, -100.0f));
    mRot  = rotate4x4X(10.0f*DEG_TO_RAD);
    mRot2 = rotate4x4Z(30.f*DEG_TO_RAD);
    mRes  = mul(mRot2, mRot);
    mRes  = mul(mTranslate, mRes);
    
    hrLightInstance(scnRef, sun, mRes.L());
    
    
    hrSceneClose(scnRef);
    
    HRRenderRef renderRef = hrRenderCreate(L"HydraModern");
    
    hrRenderEnableDevice(renderRef, CURR_RENDER_DEVICE, true);
    
    hrRenderOpen(renderRef, HR_WRITE_DISCARD);
    {
      auto node = hrRenderParamNode(renderRef);
      
      node.append_child(L"width").text()  = TEST_IMG_SIZE;
      node.append_child(L"height").text() = TEST_IMG_SIZE;
      
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
    
    hrFlush(scnRef, renderRef);
    
    ////////////////////
    // Rendering, save and check image
    ////////////////////

    RenderProgress(renderRef);

    std::filesystem::create_directories(saveRenderFile.parent_path());
    hrRenderSaveFrameBufferLDR(renderRef, saveRenderFile.wstring().c_str());

    return check_images(ws2s(nameTest).c_str(), 1, 20);
  }
  

  bool test_006_points_on_mesh()
  {
    std::wstring nameTest                = L"test_006";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);
        
    ////////////////////
    // Materials
    ////////////////////    
    
    HRMaterialRef mat1 = hrMaterialCreate(L"mat1");
    HRMaterialRef mat2 = hrMaterialCreate(L"mat2");
    
    int rep = 8;    
    HRTextureNodeRef tex = hrTexture2DCreateBakedLDR(&procTexCheckerLDR, (void *) (&rep), sizeof(int), 32, 32);
    
    hrMaterialOpen(mat1, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(mat1);
      
      auto diffuse = matNode.append_child(L"diffuse");
      diffuse.append_child(L"color").append_attribute(L"val").set_value(L"0.2 0.2 0.120");
      
      auto texNode = hrTextureBind(tex, diffuse.child(L"color"));
      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 1, 0, 0, 0.0f,
                                  0, 1, 0, 0.0f,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };
      
      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode.append_attribute(L"input_gamma").set_value(1.0f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");
      
      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);
      
    }
    hrMaterialClose(mat1);
    
    hrMaterialOpen(mat2, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(mat2);
      
      auto refl = matNode.append_child(L"reflectivity");
      
      refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"0.99 0.6 0.0");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.999");
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(8.0f);
    }
    hrMaterialClose(mat2);
    
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Meshes
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    HRMeshRef tess = CreateTriStrip(64, 64, 100);//hrMeshCreateFromFileDL(L"data/meshes/teapot.vsgf");//
    
    HRMeshRef lucyRef = hrMeshCreateFromFileDL(L"data/meshes/lucy.vsgf");
    hrMeshOpen(lucyRef, HR_TRIANGLE_IND3, HR_OPEN_EXISTING);
    {
      hrMeshMaterialId(lucyRef, mat2.id);
    }
    hrMeshClose(lucyRef);
    
    uint32_t n_points = 100;
    std::vector<float> points(n_points * 3, 0.0f);
    
    hrMeshOpen(tess, HR_TRIANGLE_IND3, HR_OPEN_EXISTING);
    {
      hrMeshMaterialId(tess, mat1.id);
      HRUtils::getRandomPointsOnMesh(tess, points.data(), n_points, true);
    }
    hrMeshClose(tess);
    
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Light
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    HRLightRef sky = hrLightCreate(L"sky");
    
    hrLightOpen(sky, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(sky);
      
      lightNode.attribute(L"type").set_value(L"sky");
      
      auto intensityNode = lightNode.append_child(L"intensity");
      
      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"0.75 0.75 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(0.25f);
      
      VERIFY_XML(lightNode);
    }
    hrLightClose(sky);
    
    
    HRLightRef sphereLight = hrLightCreate(L"my_area_light");
    
    hrLightOpen(sphereLight, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(sphereLight);
      
      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"sphere");
      lightNode.attribute(L"distribution").set_value(L"uniform");
      
      auto intensityNode = lightNode.append_child(L"intensity");
      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(6.0f*IRRADIANCE_TO_RADIANCE);
      
      lightNode.append_child(L"size").append_attribute(L"radius").set_value(5.5f);
    }
    hrLightClose(sphereLight);
    
    
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
      camNode.append_child(L"position").text().set(L"0 50 100");
      camNode.append_child(L"look_at").text().set(L"0 0 0");
    }
    hrCameraClose(camRef);
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Render settings
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 512, 512, 256, 1024);
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Create scene
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
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
    mScale     = scale4x4(float3(0.51f, 0.51f, 0.51f));
    mRes       = mul(mTranslate, mScale);
    
    auto pt_matrix = mRes;
    
    hrMeshInstance(scnRef, tess, mRes.L());
    
    for(int i = 0; i < n_points; i += 3)
    {
      auto pt    = make_float3(points[i * 3], points[i * 3 + 1], points[i * 3 + 2]);
      pt         = pt_matrix*pt;
      mTranslate = translate4x4(pt);
      mScale     = scale4x4(float3(1.0f, 1.0f, 1.0f));
      mRes       = mul(mTranslate, mScale);
      
      hrMeshInstance(scnRef, lucyRef, mRes.L());
    }
    
    ///////////
    
    mTranslate.identity();
    mRes.identity();
    
    mTranslate = translate4x4(float3(0, 0.0f, 0.0));
    mRes = mul(mTranslate, mRes);
    
    hrLightInstance(scnRef, sky, mRes.L());
    
    mTranslate = translate4x4(float3(0, 40.0f, 0.0));
    hrLightInstance(scnRef, sphereLight, mTranslate.L());
    
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

  
  bool test_007_import_obj()
  {
    std::wstring nameTest                = L"test_007";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);
        
    ////////////////////
    // Materials
    ////////////////////    

    HRMaterialRef mat0 = hrMaterialCreate(L"default");
    HRMaterialRef mat1 = hrMaterialCreate(L"wood");
    
    // Default white diffuse material
    hrMaterialOpen(mat0, HR_WRITE_DISCARD);
    {
      xml_node matNode = hrMaterialParamNode(mat0);
      
      xml_node diff = matNode.append_child(L"diffuse");
      
      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").text().set(L"0.75 0.75 0.75");
      
    }
    hrMaterialClose(mat0);
    
    // Plane material
    hrMaterialOpen(mat1, HR_WRITE_DISCARD);
    {
      xml_node matNode = hrMaterialParamNode(mat1);
      
      xml_node diff = matNode.append_child(L"diffuse");
      
      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").text().set(L"0.75 0.75 0.75");
      
      HRTextureNodeRef testTex = hrTexture2DCreateFromFile(L"data/textures/relief_wood.jpg");
      hrTextureBind(testTex, diff);
    }
    hrMaterialClose(mat1);
    
    /***********************************************  Setting geometry  ***********************************************/
    
    // Geometry
    HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), 1); // The last argument is material's id
    
    
    // Importing mesh from .obj file
    HRModelLoadInfo objModelInfo;
    objModelInfo.useMaterial = false; // In this case the default material (id = 0) is used
    
    // You can set the path to .mtl files manually in case they are located in different folder (not with .obj file)
    //objModelInfo.mtlRelativePath = L"data/meshes/shapenet_test/test1";
    
    HRMeshRef objRef = hrMeshCreateFromFile(L"data/meshes/bunny.obj", objModelInfo);
    
    
    /*********************************************  Setting lightsources  *********************************************/
    
    HRLightRef directLight = hrLightCreate(L"my_direct_light");
    hrLightOpen(directLight, HR_WRITE_DISCARD);
    {
      pugi::xml_node lightNode = hrLightParamNode(directLight);
      
      lightNode.attribute(L"type").set_value(L"directional");
      lightNode.attribute(L"shape").set_value(L"point");
      
      pugi::xml_node sizeNode = lightNode.append_child(L"size");
      
      sizeNode.append_child(L"inner_radius").append_attribute(L"val") = 10.0f;
      sizeNode.append_child(L"outer_radius").append_attribute(L"val") = 20.0f;
      
      pugi::xml_node intensityNode = lightNode.append_child(L"intensity");
      
      intensityNode.append_child(L"color").append_attribute(L"val")      = L"1 1 1";
      intensityNode.append_child(L"multiplier").append_attribute(L"val") = 2.0f*IRRADIANCE_TO_RADIANCE;
    }
    hrLightClose(directLight);
    
    HRLightRef sky = hrLightCreate(L"sky");
    hrLightOpen(sky, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(sky);
      
      lightNode.attribute(L"type").set_value(L"sky");
      
      auto intensityNode = lightNode.append_child(L"intensity");
      
      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"0.25 0.25 0.5");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(1.0f);
      
      VERIFY_XML(lightNode);
    }
    hrLightClose(sky);
    
    
    /**********************************************  Setting the camera  **********************************************/
    
    auto camRef = hrCameraCreate(L"my camera");
    
    hrCameraOpen(camRef, HR_WRITE_DISCARD);
    {
      xml_node camNode = hrCameraParamNode(camRef);
      
      camNode.append_child(L"fov").text().set(L"45");
      camNode.append_child(L"nearClipPlane").text().set(L"0.01");
      camNode.append_child(L"farClipPlane").text().set(L"100.0");
      
      camNode.append_child(L"up").text().set(L"0 1 0");
      camNode.append_child(L"position").text().set(L"-8 10 25");
      camNode.append_child(L"look_at").text().set(L"0.1 0 0.1");
    }
    hrCameraClose(camRef);
    
    /***********************************************  Setting the scene  **********************************************/
    auto scnRef = hrSceneCreate(L"my scene");
    
    const float DEG_TO_RAD = float(3.14159265358979323846f) / 180.0f;
    
    hrSceneOpen(scnRef, HR_WRITE_DISCARD);
    {
      // Plane
      auto mres = hlm::translate4x4({0.0f, -1.0f, -5.0f});
      hrMeshInstance(scnRef, planeRef, mres.L());
      
      // Directional light and sky
      mres = hlm::mul(hlm::rotate4x4Z(1.0f * DEG_TO_RAD), hlm::translate4x4({0.0f, 100.0f, 0.0f}));
      hrLightInstance(scnRef, directLight, mres.L());
      hrLightInstance(scnRef, sky, mres.L());
      
      // Imported .obj file
      mres = hlm::mul(hlm::translate4x4({-1.0f, 0.5f, 5.5f}), hlm::scale4x4({5.0, 5.0, 5.0}));
      
      hrMeshInstance(scnRef, objRef, mres.L());
    }
    hrSceneClose(scnRef);
    
    // Setting up render settings
    //
    HRRenderRef renderRef = hrRenderCreate(L"HydraModern"); // opengl1
    
    /*********************************************  Setting the renderer  *********************************************/
    
    auto pList = hrRenderGetDeviceList(renderRef);
    
    while (pList != nullptr)
    {
      std::wcout << L"device id = " << pList->id << L", name = " << pList->name << L", driver = " << pList->driver << std::endl;
      pList = pList->next;
    }
    
    hrRenderEnableDevice(renderRef, 0, true);
    
    
    hrRenderOpen(renderRef, HR_WRITE_DISCARD);
    {
      pugi::xml_node node = hrRenderParamNode(renderRef);
      
      node.append_child(L"width").text()  = TEST_IMG_SIZE;
      node.append_child(L"height").text() = TEST_IMG_SIZE;
      
      node.append_child(L"method_primary").text() = L"pathtracing";
      node.append_child(L"method_secondary").text() = L"pathtracing";
      node.append_child(L"method_tertiary").text() = L"pathtracing";
      node.append_child(L"method_caustic").text() = L"pathtracing";
      node.append_child(L"shadows").text() = L"1";
      
      node.append_child(L"trace_depth").text() = L"5";
      node.append_child(L"diff_trace_depth").text() = L"3";
      
      node.append_child(L"maxRaysPerPixel").text() = L"1024";
      node.append_child(L"offline_pt").text()      = L"1";
    }
    hrRenderClose(renderRef);
    
    
    hrFlush(scnRef, renderRef);
    
    ////////////////////
    // Rendering, save and check image
    ////////////////////

    RenderProgress(renderRef);

    std::filesystem::create_directories(saveRenderFile.parent_path());
    hrRenderSaveFrameBufferLDR(renderRef, saveRenderFile.wstring().c_str());

    return check_images(ws2s(nameTest).c_str(), 1, 20);
  }
  

  bool test_008_import_obj_w_mtl()
  {
    std::wstring nameTest                = L"test_008";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);
        
    ////////////////////
    // Materials
    ////////////////////    
        
    HRMaterialRef mat0 = hrMaterialCreate(L"default");
    HRMaterialRef mat1 = hrMaterialCreate(L"wood");
    
    // Default white diffuse material
    hrMaterialOpen(mat0, HR_WRITE_DISCARD);
    {
      xml_node matNode = hrMaterialParamNode(mat0);
      
      xml_node diff = matNode.append_child(L"diffuse");
      
      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").text().set(L"0.75 0.75 0.75");
      
    }
    hrMaterialClose(mat0);
    
    // Plane material
    hrMaterialOpen(mat1, HR_WRITE_DISCARD);
    {
      xml_node matNode = hrMaterialParamNode(mat1);
      
      xml_node diff = matNode.append_child(L"diffuse");
      
      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").text().set(L"0.75 0.75 0.75");
      
      HRTextureNodeRef testTex = hrTexture2DCreateFromFile(L"data/textures/relief_wood.jpg");
      hrTextureBind(testTex, diff);
    }
    hrMaterialClose(mat1);
    
    /***********************************************  Setting geometry  ***********************************************/
    
    // Geometry
    HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), 1); // The last argument is material's id
    
    
    // Importing mesh from .obj file
    HRModelLoadInfo objModelInfo;
    objModelInfo.useMaterial = true; // In this case the default material (id = 0) is used
    
    // You can set the path to .mtl files manually in case they are located in different folder (not with .obj file)
    //objModelInfo.mtlRelativePath = L"data/meshes/shapenet_test/test1";
    
    HRMeshRef objRef = hrMeshCreateFromFile(L"data/meshes/shapenet/11/model.obj", objModelInfo);
    
    
    /*********************************************  Setting lightsources  *********************************************/
    
    HRLightRef directLight = hrLightCreate(L"my_direct_light");
    hrLightOpen(directLight, HR_WRITE_DISCARD);
    {
      pugi::xml_node lightNode = hrLightParamNode(directLight);
      
      lightNode.attribute(L"type").set_value(L"directional");
      lightNode.attribute(L"shape").set_value(L"point");
      
      pugi::xml_node sizeNode = lightNode.append_child(L"size");
      
      sizeNode.append_child(L"inner_radius").append_attribute(L"val") = 10.0f;
      sizeNode.append_child(L"outer_radius").append_attribute(L"val") = 20.0f;
      
      pugi::xml_node intensityNode = lightNode.append_child(L"intensity");
      
      intensityNode.append_child(L"color").append_attribute(L"val")      = L"1 1 1";
      intensityNode.append_child(L"multiplier").append_attribute(L"val") = 2.0f*IRRADIANCE_TO_RADIANCE;
    }
    hrLightClose(directLight);
    
    HRLightRef sky = hrLightCreate(L"sky");
    hrLightOpen(sky, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(sky);
      
      lightNode.attribute(L"type").set_value(L"sky");
      
      auto intensityNode = lightNode.append_child(L"intensity");
      
      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"0.25 0.25 0.5");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(1.0f);
      
      VERIFY_XML(lightNode);
    }
    hrLightClose(sky);
    
    
    /**********************************************  Setting the camera  **********************************************/
    
    auto camRef = hrCameraCreate(L"my camera");
    
    hrCameraOpen(camRef, HR_WRITE_DISCARD);
    {
      xml_node camNode = hrCameraParamNode(camRef);
      
      camNode.append_child(L"fov").text().set(L"45");
      camNode.append_child(L"nearClipPlane").text().set(L"0.01");
      camNode.append_child(L"farClipPlane").text().set(L"100.0");
      
      camNode.append_child(L"up").text().set(L"0 1 0");
      camNode.append_child(L"position").text().set(L"-8 10 25");
      camNode.append_child(L"look_at").text().set(L"0.1 0 0.1");
    }
    hrCameraClose(camRef);
    
    /***********************************************  Setting the scene  **********************************************/
    auto scnRef = hrSceneCreate(L"my scene");
    
    const float DEG_TO_RAD = float(3.14159265358979323846f) / 180.0f;
    
    hrSceneOpen(scnRef, HR_WRITE_DISCARD);
    {
      // Plane
      auto mres = hlm::translate4x4({0.0f, -1.0f, -5.0f});
      hrMeshInstance(scnRef, planeRef, mres.L());
      
      // Directional light and sky
      mres = hlm::mul(hlm::rotate4x4Z(1.0f * DEG_TO_RAD), hlm::translate4x4({0.0f, 100.0f, 0.0f}));
      hrLightInstance(scnRef, directLight, mres.L());
      hrLightInstance(scnRef, sky, mres.L());
      
      // Imported .obj file
      mres = hlm::mul(hlm::translate4x4({0.0f, 4.0f, 3.0f}), hlm::scale4x4(hlm::float3(20.0, 20.0, 20.0)));
      hrMeshInstance(scnRef, objRef, mres.L());
    }
    hrSceneClose(scnRef);
    
    // Setting up render settings
    //
    HRRenderRef renderRef = hrRenderCreate(L"HydraModern"); // opengl1
    
    /*********************************************  Setting the renderer  *********************************************/
    
    auto pList = hrRenderGetDeviceList(renderRef);
    
    while (pList != nullptr)
    {
      std::wcout << L"device id = " << pList->id << L", name = " << pList->name << L", driver = " << pList->driver << std::endl;
      pList = pList->next;
    }
    
    hrRenderEnableDevice(renderRef, 0, true);
    
    
    hrRenderOpen(renderRef, HR_WRITE_DISCARD);
    {
      pugi::xml_node node = hrRenderParamNode(renderRef);
      
      node.append_child(L"width").text()  = TEST_IMG_SIZE;
      node.append_child(L"height").text() = TEST_IMG_SIZE;
      
      node.append_child(L"method_primary").text() = L"pathtracing";
      node.append_child(L"method_secondary").text() = L"pathtracing";
      node.append_child(L"method_tertiary").text() = L"pathtracing";
      node.append_child(L"method_caustic").text() = L"pathtracing";
      node.append_child(L"shadows").text() = L"1";
      
      node.append_child(L"trace_depth").text() = L"5";
      node.append_child(L"diff_trace_depth").text() = L"3";
      
      node.append_child(L"maxRaysPerPixel").text() = L"1024";
      node.append_child(L"offline_pt").text()      = L"1";
    }
    hrRenderClose(renderRef);
    
    
    hrFlush(scnRef, renderRef);
    
    ////////////////////
    // Rendering, save and check image
    ////////////////////

    RenderProgress(renderRef);

    std::filesystem::create_directories(saveRenderFile.parent_path());
    hrRenderSaveFrameBufferLDR(renderRef, saveRenderFile.wstring().c_str());

    return check_images(ws2s(nameTest).c_str(), 1, 20);
  }
  

  bool test_009_import_obj_fullscale()
  {
    std::wstring nameTest                = L"test_009";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);
        
    ////////////////////
    // Materials
    ////////////////////    
    
    HRMaterialRef mat0 = hrMaterialCreate(L"default");
    HRMaterialRef mat1 = hrMaterialCreate(L"wood");
    
    // Default white diffuse material
    hrMaterialOpen(mat0, HR_WRITE_DISCARD);
    {
      xml_node matNode = hrMaterialParamNode(mat0);
      
      xml_node diff = matNode.append_child(L"diffuse");
      
      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").text().set(L"0.75 0.75 0.75");
      
    }
    hrMaterialClose(mat0);
    
    // Plane material
    hrMaterialOpen(mat1, HR_WRITE_DISCARD);
    {
      xml_node matNode = hrMaterialParamNode(mat1);
      
      xml_node diff = matNode.append_child(L"diffuse");
      
      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").text().set(L"0.75 0.75 0.75");
      
      HRTextureNodeRef testTex = hrTexture2DCreateFromFile(L"data/textures/relief_wood.jpg");
      hrTextureBind(testTex, diff);
    }
    hrMaterialClose(mat1);
    
    /***********************************************  Setting geometry  ***********************************************/
    
    // Geometry
    HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), 1); // The last argument is material's id
    
    
    // Importing mesh from .obj file
    HRModelLoadInfo objModelInfo;
    objModelInfo.useMaterial = true; // In this case the default material (id = 0) is used
    
    // You can set the path to .mtl files manually in case they are located in different folder (not with .obj file)
    //objModelInfo.mtlRelativePath = L"data/meshes/shapenet_test/test1";
    
    HRMeshRef objRef = hrMeshCreateFromFile(L"data/meshes/bottle/model.obj", objModelInfo);
    
    
    /*********************************************  Setting lightsources  *********************************************/
    
    HRLightRef directLight = hrLightCreate(L"my_direct_light");
    hrLightOpen(directLight, HR_WRITE_DISCARD);
    {
      pugi::xml_node lightNode = hrLightParamNode(directLight);
      
      lightNode.attribute(L"type").set_value(L"directional");
      lightNode.attribute(L"shape").set_value(L"point");
      
      pugi::xml_node sizeNode = lightNode.append_child(L"size");
      
      sizeNode.append_child(L"inner_radius").append_attribute(L"val") = 10.0f;
      sizeNode.append_child(L"outer_radius").append_attribute(L"val") = 20.0f;
      
      pugi::xml_node intensityNode = lightNode.append_child(L"intensity");
      
      intensityNode.append_child(L"color").append_attribute(L"val")      = L"1 1 1";
      intensityNode.append_child(L"multiplier").append_attribute(L"val") = 2.0f*IRRADIANCE_TO_RADIANCE;
    }
    hrLightClose(directLight);
    
    HRLightRef sky = hrLightCreate(L"sky");
    hrLightOpen(sky, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(sky);
      
      lightNode.attribute(L"type").set_value(L"sky");
      
      auto intensityNode = lightNode.append_child(L"intensity");
      
      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"0.25 0.25 0.5");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(1.0f);
      
      VERIFY_XML(lightNode);
    }
    hrLightClose(sky);
    
    
    /**********************************************  Setting the camera  **********************************************/
    
    auto camRef = hrCameraCreate(L"my camera");
    
    hrCameraOpen(camRef, HR_WRITE_DISCARD);
    {
      xml_node camNode = hrCameraParamNode(camRef);
      
      camNode.append_child(L"fov").text().set(L"45");
      camNode.append_child(L"nearClipPlane").text().set(L"0.01");
      camNode.append_child(L"farClipPlane").text().set(L"100.0");
      
      camNode.append_child(L"up").text().set(L"0 1 0");
      camNode.append_child(L"position").text().set(L"-8 10 25");
      camNode.append_child(L"look_at").text().set(L"0.1 0 0.1");
    }
    hrCameraClose(camRef);
    
    /***********************************************  Setting the scene  **********************************************/
    auto scnRef = hrSceneCreate(L"my scene");
    
    const float DEG_TO_RAD = float(3.14159265358979323846f) / 180.0f;
    
    hrSceneOpen(scnRef, HR_WRITE_DISCARD);
    {
      // Plane
      auto mres = hlm::translate4x4({0.0f, -1.0f, -5.0f});
      hrMeshInstance(scnRef, planeRef, mres.L());
      
      // Directional light and sky
      mres = hlm::mul(hlm::rotate4x4Z(1.0f * DEG_TO_RAD), hlm::translate4x4({0.0f, 100.0f, 0.0f}));
      hrLightInstance(scnRef, directLight, mres.L());
      hrLightInstance(scnRef, sky, mres.L());
      
      // Imported .obj file
      mres = hlm::mul(hlm::translate4x4({0.0f, 3.0f, 2.0f}), hlm::scale4x4({8.0f, 8.0f, 8.0f}));
      hrMeshInstance(scnRef, objRef, mres.L());
    }
    hrSceneClose(scnRef);
    
    // Setting up render settings
    //
    HRRenderRef renderRef = hrRenderCreate(L"HydraModern"); // opengl1
    
    /*********************************************  Setting the renderer  *********************************************/
    
    auto pList = hrRenderGetDeviceList(renderRef);
    
    while (pList != nullptr)
    {
      std::wcout << L"device id = " << pList->id << L", name = " << pList->name << L", driver = " << pList->driver << std::endl;
      pList = pList->next;
    }
    
    hrRenderEnableDevice(renderRef, 0, true);
    
    
    hrRenderOpen(renderRef, HR_WRITE_DISCARD);
    {
      pugi::xml_node node = hrRenderParamNode(renderRef);
      
      node.append_child(L"width").text()  = TEST_IMG_SIZE;
      node.append_child(L"height").text() = TEST_IMG_SIZE;
      
      node.append_child(L"method_primary").text() = L"pathtracing";
      node.append_child(L"method_secondary").text() = L"pathtracing";
      node.append_child(L"method_tertiary").text() = L"pathtracing";
      node.append_child(L"method_caustic").text() = L"pathtracing";
      node.append_child(L"shadows").text() = L"1";
      
      node.append_child(L"trace_depth").text() = L"5";
      node.append_child(L"diff_trace_depth").text() = L"3";
      
      node.append_child(L"maxRaysPerPixel").text() = L"1024";
      node.append_child(L"offline_pt").text()      = L"1";
    }
    hrRenderClose(renderRef);
    
    
    hrFlush(scnRef, renderRef);
    
    ////////////////////
    // Rendering, save and check image
    ////////////////////

    RenderProgress(renderRef);

    std::filesystem::create_directories(saveRenderFile.parent_path());
    hrRenderSaveFrameBufferLDR(renderRef, saveRenderFile.wstring().c_str());

    return check_images(ws2s(nameTest).c_str(), 1, 20);
  }
  
}
