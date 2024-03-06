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
  
  bool test_301_mesh_from_memory()
  {
    std::wstring nameTest                = L"test_301";
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
    
    auto renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, TEST_IMG_SIZE, TEST_IMG_SIZE, 64, 64);
    
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
  


  bool test_302_mesh_from_vsgf()
  {
    std::wstring nameTest                = L"test_302";
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
    
    auto renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, TEST_IMG_SIZE, TEST_IMG_SIZE, 256, 256);
    
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
  


  bool test_303_compute_normals()
  {
    std::wstring nameTest                = L"test_303";
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
    
    auto renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, TEST_IMG_SIZE, TEST_IMG_SIZE, 300, 300);
    
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
  

  bool test_304_dof()
  {
    std::wstring nameTest                = L"test_304";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);
        
    ////////////////////
    // Materials
    ////////////////////
        
    auto mat0 = hrMaterialCreate(L"mysimplemat");
    auto mat1 = hrMaterialCreate(L"mysimplemat2");
    auto mat2 = hrMaterialCreate(L"wood");
    auto mat3 = hrMaterialCreate(L"rock");
    
    auto testTex0  = hrTexture2DCreateFromFile(L"data/textures/texture1.bmp");
    auto testTex1 = hrTexture2DCreateFromFile(L"data/textures/chess_red.bmp");
    auto testTex2 = hrTexture2DCreateFromFile(L"data/textures/relief_wood.jpg");
    auto testTex3 = hrTexture2DCreateFromFile(L"data/textures/163.jpg");

    hrMaterialOpen(mat0, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(mat0);
      AddDiffuseNode(matNode, L"0.75 0.75 0.75", L"lambert", 0, testTex0);
    }
    hrMaterialClose(mat0);
    
    hrMaterialOpen(mat1, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(mat1);
      AddDiffuseNode(matNode, L"1 1 1", L"lambert", 0, testTex1);
    }
    hrMaterialClose(mat1);
    
    hrMaterialOpen(mat2, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(mat2);
      AddDiffuseNode(matNode, L"0.75 0.75 0.75", L"lambert", 0, testTex2);
    }
    hrMaterialClose(mat2);
    
    hrMaterialOpen(mat3, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(mat3);
      AddDiffuseNode(matNode, L"0.75 0.75 0.75", L"lambert", 0, testTex3);
    }
    hrMaterialClose(mat3);    
    
    ////////////////////
    // Light
    ////////////////////

    auto sky         = CreateSky(L"sky", L"0.25 0.25 0.5", 1);

    auto directLight = CreateLight(L"Light01", L"directional", L"point", L"diffuse", 0, 0, L"1 1 1",
                                    2.0f * IRRADIANCE_TO_RADIANCE, true, 100, 200); // 10 and 20 previously
    
    ////////////////////
    // Camera
    ////////////////////

    CreateCamera(30, L"-8 2 15", L"0.1 -3 0.1", L"Camera01", 0.01F, 100, L"0 1 0", true, 0.1F);
        
    ////////////////////
    // Render settings
    ////////////////////

    auto renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 512, 256, 256, 512);
            
    ////////////////////
    // Meshes
    ////////////////////

    auto cubeRef  = HRMeshFromSimpleMesh(L"my_cube", CreateCube(0.25f), 1);
    auto planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), 2);
    auto sphRef   = HRMeshFromSimpleMesh(L"my_sphere", CreateSphere(0.5f, 32), 3);
    auto torRef   = HRMeshFromSimpleMesh(L"my_torus", CreateTorus(0.1f, 0.3f, 32, 32), 0);
        
    ////////////////////
    // Create scene
    ////////////////////

    auto scnRef = hrSceneCreate((L"scene_" + nameTest).c_str());        
    
    hrSceneOpen(scnRef, HR_WRITE_DISCARD);
        
    auto rgen         = simplerandom::RandomGenInit(878976);
    HRMeshRef refs[3] = { cubeRef, sphRef, torRef };        
    
    for (int i = -5; i <= 5; i++)
    {
      for (int j = -5; j <= 5; j++)
      {        
        float matrixT[4][4], matrixT2[4][4];
        float mRot1[4][4], mTranslate[4][4], mRes[4][4];

        float xCoord = 2.5f * float(i);
        float yCoord = 2.5f * float(j);
        
        mat4x4_identity(mRot1);
        mat4x4_identity(mTranslate);
        mat4x4_identity(mRes);
        
        mat4x4_translate(mTranslate, xCoord, 0.1F, yCoord);
        mat4x4_rotate_X(mRot1, mRot1, simplerandom::rnd(rgen, 0, 360) * DEG_TO_RAD);
        mat4x4_rotate_Y(mRot1, mRot1, simplerandom::rnd(rgen, 0, 360) * DEG_TO_RAD * 0.5F);
        mat4x4_mul(mRes, mTranslate, mRot1);
        mat4x4_transpose(matrixT, mRes); // this fucking math library swap rows and columns
        
        hrMeshInstance(scnRef, refs[simplerandom::rand(rgen) % 3], &matrixT[0][0]);
        
        //AddMeshToScene(scnRef, refs[simplerandom::rand(rgen) % 3], float3(xCoord, 0.1F, yCoord), 
        //  float3(simplerandom::rnd(rgen, 0, 360), simplerandom::rnd(rgen, 0, 180), 0)); // Wrong. Why?

      }
    }
    
    AddMeshToScene(scnRef, planeRef, float3(0.0f, -1.0f, -5.0f));
    AddLightToScene(scnRef, directLight, float3(0, 100, 0), float3(0, 0, 1));
    AddLightToScene(scnRef, sky);

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
  

  bool test_305_instancing()
  {
    std::wstring nameTest                = L"test_305";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);
        
    ////////////////////
    // Materials
    ////////////////////    
    
    auto matGray     = hrMaterialCreate(L"matGray");
    auto matTrunk    = hrMaterialCreate(L"Trunk");
    auto matWigglers = hrMaterialCreate(L"Wigglers");
    auto matBranches = hrMaterialCreate(L"Branches");
    auto matPllarRts = hrMaterialCreate(L"PillarRoots");
    auto matLeaves   = hrMaterialCreate(L"Leaves");
    auto matCanopy   = hrMaterialCreate(L"Canopy");
    auto matCube     = hrMaterialCreate(L"CubeMap");
    
    auto texPattern  = hrTexture2DCreateFromFile(L"data/textures/bigleaf3.tga");
    auto texture1    = hrTexture2DCreateFromFile(L"data/textures/texture1.bmp");

    hrMaterialOpen(matCube, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matCube);
      AddDiffuseNode(matNode, L"0.5 0.5 0.5", L"lambert", 0, texture1);      
    }
    hrMaterialClose(matCube);
    
    hrMaterialOpen(matGray, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGray);
      AddDiffuseNode(matNode, L"0.5 0.5 0.5");      
    }
    hrMaterialClose(matGray);
    
    hrMaterialOpen(matTrunk, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matTrunk);
      AddDiffuseNode(matNode, L"0.345098 0.215686 0.0117647");
    }
    hrMaterialClose(matTrunk);
    
    hrMaterialOpen(matWigglers, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matWigglers);
      AddDiffuseNode(matNode, L"0.345098 0.215686 0.0117647");
    }
    hrMaterialClose(matWigglers);
    
    hrMaterialOpen(matBranches, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matBranches);
      AddDiffuseNode(matNode, L"0.345098 0.215686 0.0117647");
    }
    hrMaterialClose(matBranches);
    
    hrMaterialOpen(matPllarRts, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matPllarRts);
      AddDiffuseNode(matNode, L"0.345098 0.215686 0.0117647");
    }
    hrMaterialClose(matPllarRts);
    
    //////////////////////////////////
    
    hrMaterialOpen(matLeaves, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matLeaves);
      AddDiffuseNode(matNode, L"0.0533333 0.208627 0.00627451");      
      auto opacity = matNode.append_child(L"opacity");
      auto texNode = hrTextureBind(texPattern, opacity);
      
    }
    hrMaterialClose(matLeaves);
    
    hrMaterialOpen(matCanopy, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matCanopy);
      AddDiffuseNode(matNode, L"0.0941176 0.4 0");
    }
    hrMaterialClose(matCanopy);
    
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Meshes
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    auto cubeR    = HRMeshFromSimpleMesh(L"cubeR", CreateCube(2.0f), matCube.id);
    auto pillar   = HRMeshFromSimpleMesh(L"pillar", CreateCube(1.0f), matGray.id);
    auto sphereG  = HRMeshFromSimpleMesh(L"sphereG", CreateSphere(4.0f, 64), matTrunk.id);
    auto torusB   = HRMeshFromSimpleMesh(L"torusB", CreateTorus(0.8f, 2.0f, 64, 64), matTrunk.id);
    auto planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(10000.0f), matGray.id);
    
    auto treeRef  = hrMeshCreateFromFileDL(L"data/meshes/bigtree.vsgf");
    
    ////////////////////
    // Light
    ////////////////////

    auto sun = CreateLight(L"Light01", L"directional", L"point", L"directional", 0, 0, L"1.0 0.85 0.64",
      2.0f * IRRADIANCE_TO_RADIANCE, true, 0, 10000, 1);

    auto sky = CreateSky(L"sky", L"1 1 1", 1, L"perez", sun.id, 2);
    
    ////////////////////
    // Camera
    ////////////////////

    CreateCamera(45, L"0 15 25", L"5 10 0");
    
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
    
    float4x4 mRot, mRot2;
    float4x4 mTranslate;
    float4x4 mScale;
    float4x4 mRes;    
    
    mTranslate.identity();
    mRes.identity();
    mRot.identity();    
    mTranslate = translate4x4(float3(-4.75f, 1.0f, 5.0f));
    mRot       = rotate4x4Y(60.0f*DEG_TO_RAD);
    mRes       = mul(mTranslate, mRot);
    hrMeshInstance(scnRef, cubeR, mRes.L());

    //AddMeshToScene(scnRef, cubeR, float3(-4.75f, 1, 5), float3(0.0F, -60.0F, 0.0F)); // correct with rot. Y -60, why?
    
    auto rgen = simplerandom::RandomGenInit(125);
    
    {
      mTranslate.identity();
      mRes.identity();
      mRot.identity();

      const float dist1     = 40.0f;
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
          //AddMeshToScene(scnRef, cubeR, float3(pos.x, 1, pos.z), float3(0, simplerandom::rnd(rgen, -180.0f * DEG_TO_RAD, 180.0f * DEG_TO_RAD), 0)); // why move trees?
        }
      }
    }
    
    ///////////
    {
      mTranslate.identity();
      mRes.identity();
      mRot.identity();

      const float dist     = 40.0f;
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
            //AddMeshToScene(scnRef, treeRef, pos, float3(0, simplerandom::rnd(rgen, -180.0f * DEG_TO_RAD, +180.0f * DEG_TO_RAD), 0), float3(5, 5, 5)); // not equals with ref.
        }
      }
    }
    
    ///////////
    
    AddLightToScene(scnRef, sky);
    
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
    //AddLightToScene(scnRef, sun, float3(200, 200, -100), float3(10, 0, 30)); // small offset, why?!
        
    ///////////
    hrSceneClose(scnRef);        
    hrFlush(scnRef, renderRef);
    
    ////////////////////
    // Rendering, save and check image
    ////////////////////

    RenderProgress(renderRef);

    std::filesystem::create_directories(saveRenderFile.parent_path());
    hrRenderSaveFrameBufferLDR(renderRef, saveRenderFile.wstring().c_str());

    return check_images(ws2s(nameTest).c_str(), 1, 27);
  }
  

  bool test_306_points_on_mesh()
  {
    std::wstring nameTest                = L"test_306";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);
        
    ////////////////////
    // Materials
    ////////////////////    
    
    auto mat1 = hrMaterialCreate(L"mat1");
    auto mat2 = hrMaterialCreate(L"mat2");
    
    int rep   = 4;    
    auto tex  = hrTexture2DCreateBakedLDR(&procTexCheckerLDR, (void *) (&rep), sizeof(int), 512, 512);
    
    hrMaterialOpen(mat1, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(mat1);
      AddDiffuseNode(matNode, L"0.6 0.4 0.2", L"lambert", 0, tex);      
    }
    hrMaterialClose(mat1);
    
    hrMaterialOpen(mat2, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(mat2);
      AddDiffuseNode(matNode, L"0.7 0.7 0.7");

    }
    hrMaterialClose(mat2);
    
    
    ////////////////////
    // Meshes
    ////////////////////

    auto tessRef = CreateTriStrip(64, 64, 100);//hrMeshCreateFromFileDL(L"data/meshes/teapot.vsgf");    
    auto lucyRef = hrMeshCreateFromFileDL(L"data/meshes/lucy.vsgf");

    hrMeshOpen(lucyRef, HR_TRIANGLE_IND3, HR_OPEN_EXISTING);
    {
      hrMeshMaterialId(lucyRef, mat2.id);
    }
    hrMeshClose(lucyRef);
    
    uint32_t n_points = 100;
    std::vector<float> points(n_points * 3);
    
    hrMeshOpen(tessRef, HR_TRIANGLE_IND3, HR_OPEN_EXISTING);
    {
      hrMeshMaterialId(tessRef, mat1.id);
      HRUtils::getRandomPointsOnMesh(tessRef, points.data(), n_points, true);
    }
    hrMeshClose(tessRef);
    
    
    ////////////////////
    // Light
    ////////////////////

    auto sky         = CreateSky(L"sky", L"1 1 1", 0.05f);

    auto sphereLight = CreateLight(L"Light01", L"area", L"sphere", L"uniform", 3, 0, L"1 1 1",
      40 * IRRADIANCE_TO_RADIANCE);      
    
        
    ////////////////////
    // Camera
    ////////////////////

    CreateCamera(15, L"0 50 110", L"0 0 0");
    
    ////////////////////
    // Render settings
    ////////////////////

    auto renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 512, 256, 256, 1024);
    
    ////////////////////
    // Create scene
    ////////////////////

    auto scnRef = hrSceneCreate((L"scene_" + nameTest).c_str());

    hrSceneOpen(scnRef, HR_WRITE_DISCARD);

    using namespace LiteMath;
    
    float4x4 mRot;
    float4x4 mTranslate;
    float4x4 mScale;
    float4x4 mRes;    
    
    /////////
    mTranslate.identity();
    mRes.identity();
    
    mTranslate = translate4x4(float3(0, -1, 0));
    mScale     = scale4x4(float3(0.51f, 0.51f, 0.51f));
    mRes       = mul(mTranslate, mScale);
    
    auto pt_matrix = mRes;
    
    hrMeshInstance(scnRef, tessRef, mRes.L());
    
    for(int i = 0; i < n_points; i += 3)
    {
      auto pt    = make_float3(points[i * 3], points[i * 3 + 1], points[i * 3 + 2]);
      pt         = pt_matrix*pt;
      mTranslate = translate4x4(pt);
      mScale     = scale4x4(float3(1, 1, 1));
      mRes       = mul(mTranslate, mScale);  
      hrMeshInstance(scnRef, lucyRef, mRes.L());
    }    
    
    AddLightToScene(scnRef, sphereLight, float3(25, 25, 0));
    AddLightToScene(scnRef, sky);
    
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

  
  bool test_307_import_obj()
  {
    std::wstring nameTest                = L"test_307";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);
        
    ////////////////////
    // Materials
    ////////////////////    

    auto mat0    = hrMaterialCreate(L"default");
    auto mat1    = hrMaterialCreate(L"wood");    
    auto testTex = hrTexture2DCreateFromFile(L"data/textures/relief_wood.jpg");
        
    hrMaterialOpen(mat0, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(mat0);
      AddDiffuseNode(matNode, L"0.75 0.75 0.75");
    }
    hrMaterialClose(mat0);    

    hrMaterialOpen(mat1, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(mat1);
      AddDiffuseNode(matNode, L"0.75 0.75 0.75", L"lambert", 0, testTex);
    }
    hrMaterialClose(mat1);
    
    ////////////////////
    // Meshes
    ////////////////////

    auto planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), 1); // The last argument is material's id
        
    // Importing mesh from .obj file
    HRModelLoadInfo objModelInfo;
    objModelInfo.useMaterial = false; // In this case the default material (id = 0) is used
    
    // You can set the path to .mtl files manually in case they are located in different folder (not with .obj file)
    //objModelInfo.mtlRelativePath = L"data/meshes/shapenet_test/test1";
    
    auto objRef = hrMeshCreateFromFile(L"data/meshes/bunny.obj", objModelInfo);    
    
    ////////////////////
    // Light
    ////////////////////

    auto sky         = CreateSky(L"sky", L"0.25 0.25 0.5", 1);    

    auto directLight = CreateLight(L"Light01", L"directional", L"point", L"diffuse", 0, 0, L"1 1 1",
      1.0f * IRRADIANCE_TO_RADIANCE, true, 10, 20);
    
    ////////////////////
    // Camera
    ////////////////////

    CreateCamera(45, L"-8 10 25", L"0.1 0 0.1");
    
    ////////////////////
    // Render settings
    ////////////////////

    auto renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 512, 512, 64, 64);

    ////////////////////
    // Create scene
    ////////////////////

    auto scnRef = hrSceneCreate((L"scene_" + nameTest).c_str());
    
    hrSceneOpen(scnRef, HR_WRITE_DISCARD);

    AddMeshToScene(scnRef, planeRef, float3(0, -1, -5));
    AddMeshToScene(scnRef, objRef, float3(-1, 0.5F, 5.5F), float3(), float3(5, 5, 5));
    AddLightToScene(scnRef, directLight, float3(0, 100, 0), float3(0, 0, 1));
    AddLightToScene(scnRef, sky);

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
  

  bool test_308_import_obj_w_mtl()
  {
    std::wstring nameTest                = L"test_308";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);
        
    ////////////////////
    // Materials
    ////////////////////    

    auto mat0    = hrMaterialCreate(L"default");
    auto mat1    = hrMaterialCreate(L"wood");    
    auto testTex = hrTexture2DCreateFromFile(L"data/textures/relief_wood.jpg");
        
    hrMaterialOpen(mat0, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(mat0);
      AddDiffuseNode(matNode, L"0.75 0.75 0.75");
    }
    hrMaterialClose(mat0);    

    hrMaterialOpen(mat1, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(mat1);
      AddDiffuseNode(matNode, L"0.75 0.75 0.75", L"lambert", 0, testTex);
    }
    hrMaterialClose(mat1);
    
    ////////////////////
    // Meshes
    ////////////////////

    auto planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), 1); // The last argument is material's id
        
    // Importing mesh from .obj file
    HRModelLoadInfo objModelInfo;
    //objModelInfo.useMaterial = true; // In this case the default material (id = 0) is used
    
    // You can set the path to .mtl files manually in case they are located in different folder (not with .obj file)
    objModelInfo.mtlRelativePath = L"data/meshes/shapenet/11";
    
    auto objRef = hrMeshCreateFromFile(L"data/meshes/shapenet/11/model.obj", objModelInfo);
        
    ////////////////////
    // Light
    ////////////////////

    auto sky = CreateSky(L"sky", L"0.25 0.25 0.5", 1);

    auto directLight = CreateLight(L"Light01", L"directional", L"point", L"diffuse", 0, 0, L"1 1 1",
      1.0f * IRRADIANCE_TO_RADIANCE, true, 10, 20);    
    
    ////////////////////
    // Camera
    ////////////////////

    CreateCamera(45, L"-8 10 25", L"0.1 0 0.1");

    ////////////////////
    // Render settings
    ////////////////////

    auto renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 512, 512, 128, 128);
        
    ////////////////////
    // Create scene
    ////////////////////

    auto scnRef = hrSceneCreate((L"scene_" + nameTest).c_str());

    hrSceneOpen(scnRef, HR_WRITE_DISCARD);

    AddMeshToScene(scnRef, planeRef, float3(0, -1, -5));
    AddMeshToScene(scnRef, objRef, float3(0, 4, 3), float3(), float3(20, 20, 20));
    AddLightToScene(scnRef, directLight, float3(0, 100, 0), float3(0, 0, 1));
    AddLightToScene(scnRef, sky);

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
  

  bool test_309_import_obj_fullscale()
  {
    std::wstring nameTest                = L"test_309";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);
        
    ////////////////////
    // Materials
    ////////////////////    

    auto mat0    = hrMaterialCreate(L"default");
    auto mat1    = hrMaterialCreate(L"wood");    
    auto testTex = hrTexture2DCreateFromFile(L"data/textures/relief_wood.jpg");
        
    hrMaterialOpen(mat0, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(mat0);
      AddDiffuseNode(matNode, L"0.75 0.75 0.75");
    }
    hrMaterialClose(mat0);    

    hrMaterialOpen(mat1, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(mat1);
      AddDiffuseNode(matNode, L"0.75 0.75 0.75", L"lambert", 0, testTex);
    }
    hrMaterialClose(mat1);
    
    ////////////////////
    // Meshes
    ////////////////////
    
    auto planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), 1); // The last argument is material's id
        
    // Importing mesh from .obj file
    HRModelLoadInfo objModelInfo;
    //objModelInfo.useMaterial = true; // In this case the default material (id = 0) is used
    
    // You can set the path to .mtl files manually in case they are located in different folder (not with .obj file)
    //objModelInfo.mtlRelativePath = L"data/meshes/bottle/";
    
    auto objRef = hrMeshCreateFromFile(L"data/meshes/bottle/model.obj", objModelInfo);    
    
    ////////////////////
    // Light
    ////////////////////

    auto sky = CreateSky(L"sky", L"0.25 0.25 0.5", 1);

    auto directLight = CreateLight(L"Light01", L"directional", L"point", L"diffuse", 0, 0, L"1 1 1",
      1.0f * IRRADIANCE_TO_RADIANCE, true, 10, 20);

    ////////////////////
    // Camera
    ////////////////////

    CreateCamera(45, L"-8 10 25", L"0.1 0 0.1");

    ////////////////////
    // Render settings
    ////////////////////

    auto renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 512, 512, 128, 128);

    ////////////////////
    // Create scene
    ////////////////////

    auto scnRef = hrSceneCreate((L"scene_" + nameTest).c_str());

    hrSceneOpen(scnRef, HR_WRITE_DISCARD);

    AddMeshToScene(scnRef, planeRef, float3(0, -1, -5));    
    AddMeshToScene(scnRef, objRef, float3(0, 3, 2), float3(), float3(8, 8, 8));    
    AddLightToScene(scnRef, directLight, float3(0, 100, 0), float3(0, 0, 1));
    AddLightToScene(scnRef, sky);

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
}
