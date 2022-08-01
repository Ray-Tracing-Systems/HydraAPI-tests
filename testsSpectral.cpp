#include <cassert>
#include <filesystem>
#include <algorithm>
#include <numeric>
#include "tests.h"

#include "HydraXMLHelpers.h"
#include "LiteMath.h"
#include "mesh_utils.h"
#include "HRExtensions_Spectral.h"


namespace SPECTRAL_TESTS
{
  namespace hlm = LiteMath;

  bool test_cornell_RGB()
  {
    hrErrorCallerPlace(L"test_cornell_RGB");

    hrSceneLibraryOpen(L"tests/test_cornell_RGB", HR_WRITE_DISCARD);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Materials
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRMaterialRef matR = hrMaterialCreate(L"matR");
    HRMaterialRef matG = hrMaterialCreate(L"matG");
    HRMaterialRef matGray = hrMaterialCreate(L"matGray");


    hrMaterialOpen(matR, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matR);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.63 0.065 0.05");
//      color.append_attribute(L"val").set_value(L"0.539043 0.539043 0.539043");

      VERIFY_XML(matNode);
    }
    hrMaterialClose(matR);

    hrMaterialOpen(matG, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matG);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.14 0.45 0.091");
//      color.append_attribute(L"val").set_value(L"0.00949657 0.00949657 0.00949657");

      VERIFY_XML(matNode);
    }
    hrMaterialClose(matG);

    hrMaterialOpen(matGray, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGray);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.725 0.71 0.68");
//      color.append_attribute(L"val").set_value(L"0.8 0.8 0.8");

      VERIFY_XML(matNode);
    }
    hrMaterialClose(matGray);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Meshes
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    auto simpleBox = CreateCubeOpen(1.0f);
    HRMeshRef box = TEST_UTILS::HRMeshFromSimpleMesh(L"box", simpleBox, matG.id);
    //hrMeshCreateFromFile(L"data/meshes/floor.obj");

    hrMeshOpen(box, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
    {
      hrMeshVertexAttribPointer4f(box, L"pos", &simpleBox.vPos[0]);
      hrMeshVertexAttribPointer4f(box, L"norm", &simpleBox.vNorm[0]);
      hrMeshVertexAttribPointer2f(box, L"texcoord", &simpleBox.vTexCoord[0]);

      int cubeMatIndices[10] = { matGray.id, matGray.id, matGray.id, matGray.id, matGray.id, matGray.id, matG.id, matG.id, matR.id, matR.id };

      hrMeshPrimitiveAttribPointer1i(box, L"mind", cubeMatIndices);

      hrMeshAppendTriangles3(box, int(simpleBox.triIndices.size()), &simpleBox.triIndices[0]);
    }
    hrMeshClose(box);

    auto tallBox = hrMeshCreateFromFile(L"data/meshes/tallbox.obj");
    hrMeshOpen(tallBox, HR_TRIANGLE_IND3, HR_OPEN_EXISTING);
    {
      hrMeshMaterialId(tallBox, matGray.id);
    }
    hrMeshClose(tallBox);

    auto smallBox = hrMeshCreateFromFile(L"data/meshes/smallbox.obj");
    hrMeshOpen(smallBox, HR_TRIANGLE_IND3, HR_OPEN_EXISTING);
    {
      hrMeshMaterialId(smallBox, matGray.id);
    }
    hrMeshClose(smallBox);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Light
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRLightRef rectLight = hrLightCreate(L"my_area_light");

    hrLightOpen(rectLight, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(rectLight);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"rect");
      lightNode.attribute(L"distribution").set_value(L"diffuse");

      auto sizeNode = lightNode.append_child(L"size");

      sizeNode.append_attribute(L"half_length").set_value(L"0.25");
      sizeNode.append_attribute(L"half_width").set_value(L"0.19");

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 0.70588 0.23529");
//      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"0.404987 0.404987 0.404987");

      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(17.0f);
      VERIFY_XML(lightNode);
    }
    hrLightClose(rectLight);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Camera
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRCameraRef camRef = hrCameraCreate(L"my camera");

    hrCameraOpen(camRef, HR_WRITE_DISCARD);
    {
      auto camNode = hrCameraParamNode(camRef);

      camNode.append_child(L"fov").text().set(L"19.5");
      camNode.append_child(L"nearClipPlane").text().set(L"0.01");
      camNode.append_child(L"farClipPlane").text().set(L"100.0");

      camNode.append_child(L"up").text().set(L"0 1 0");
      camNode.append_child(L"position").text().set(L"0 1 6.8");
      camNode.append_child(L"look_at").text().set(L"0 1 0");
    }
    hrCameraClose(camRef);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Render settings
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRRenderRef renderRef = TEST_UTILS::CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 512, 512, 256, 1024);
    hrRenderOpen(renderRef, HR_OPEN_EXISTING);
    {
      auto node = hrRenderParamNode(renderRef);
//      node.append_child(L"framebuffer_channels").text() = 4;

      node.force_child(L"diff_trace_depth").text() = 5;
    }
    hrRenderClose(renderRef);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Create scene
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

    const float DEG_TO_RAD = 0.01745329251f; // float(3.14159265358979323846f) / 180.0f;

    hrSceneOpen(scnRef, HR_WRITE_DISCARD);

    hlm::float4x4 mScale, mRotX, mRotY, mT, mRes;

    ///////////

    mRotY = hlm::rotate4x4Y(180.0f * DEG_TO_RAD);
    mT = hlm::translate4x4({0.0f, 1.0f, 0.0f});
    mRes = mT * mRotY;

    hrMeshInstance(scnRef, box, mRes.L());

    mRes = {};
    hrMeshInstance(scnRef, tallBox, mRes.L());

    hrMeshInstance(scnRef, smallBox, mRes.L());

    ///////////

    mT = hlm::translate4x4({0.0f, 1.98f, -0.03f});
//    mRotX = hlm::rotate4x4X(-90.0f * DEG_TO_RAD);
    mRes = mT * mRotX;

    hrLightInstance(scnRef, rectLight, mRes.L());

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
        std::cout << "rendering progress = " << info.progress << "% \r"; std::cout.flush();
        std::cout.precision(pres);
      }

      if (info.finalUpdate)
        break;
    }

    hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_cornell_RGB/z_out.png");
    hrRenderSaveFrameBufferHDR(renderRef, L"tests_images/test_cornell_RGB/z_out.exr");

    return check_images("test_cornell_RGB", 1, 25);
  }

  bool test_cornell_spectral_2()
  {
    hrErrorCallerPlace(L"test_cornell_spectral_2");

    hrSceneLibraryOpen(L"tests/test_cornell_spectral_2", HR_WRITE_DISCARD);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Materials
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    std::vector<int> wavelengths = {400,        440,       480,      520,      560,      600,      640,        680};
    std::vector<float> red       = {0.00800717, 0.125122,  1e-05,    1e-05,    1e-05,    0.685583, 0.539043,   0.0597818};
    std::vector<float> green     = {0.00168552, 0.0579974, 0.137609, 0.504929, 0.566779, 0.179425, 0.00949657, 6.65987e-05};
    std::vector<float> light     = {0.0117894,  0.296203,  0.158988, 0.419806, 0.939172, 1.07416,  0.404987,   0.0416185};

    auto greenMaterials = hr_spectral::CreateSpectralDiffuseMaterials(wavelengths, green, L"green");
    auto redMaterials = hr_spectral::CreateSpectralDiffuseMaterials(wavelengths, red, L"red");

    HRMaterialRef matGray = hrMaterialCreate(L"matGray");
    hrMaterialOpen(matGray, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGray);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
//      color.append_attribute(L"val").set_value(L"0.725 0.71 0.68");
      color.append_attribute(L"val").set_value(L"0.8 0.8 0.8");

      VERIFY_XML(matNode);
    }
    hrMaterialClose(matGray);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Meshes
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    auto simpleBox = CreateCubeOpen(1.0f);
    HRMeshRef box = TEST_UTILS::HRMeshFromSimpleMesh(L"box", simpleBox, matGray.id);
    hrMeshOpen(box, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
    {
      hrMeshVertexAttribPointer4f(box, L"pos", &simpleBox.vPos[0]);
      hrMeshVertexAttribPointer4f(box, L"norm", &simpleBox.vNorm[0]);
      hrMeshVertexAttribPointer2f(box, L"texcoord", &simpleBox.vTexCoord[0]);

      int cubeMatIndices[10] = { matGray.id, matGray.id, matGray.id, matGray.id, matGray.id, matGray.id,
                                 greenMaterials[0].id, greenMaterials[0].id, redMaterials[0].id, redMaterials[0].id };

      hrMeshPrimitiveAttribPointer1i(box, L"mind", cubeMatIndices);

      hrMeshAppendTriangles3(box, int(simpleBox.triIndices.size()), &simpleBox.triIndices[0]);
    }
    hrMeshClose(box);

    auto tallBox = hrMeshCreateFromFile(L"data/meshes/tallbox.obj");
    hrMeshOpen(tallBox, HR_TRIANGLE_IND3, HR_OPEN_EXISTING);
    {
      hrMeshMaterialId(tallBox, matGray.id);
    }
    hrMeshClose(tallBox);

    auto smallBox = hrMeshCreateFromFile(L"data/meshes/smallbox.obj");
    hrMeshOpen(smallBox, HR_TRIANGLE_IND3, HR_OPEN_EXISTING);
    {
      hrMeshMaterialId(smallBox, matGray.id);
    }
    hrMeshClose(smallBox);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Light
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRLightRef rectLight = hrLightCreate(L"my_area_light");

    hrLightOpen(rectLight, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(rectLight);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"rect");
      lightNode.attribute(L"distribution").set_value(L"diffuse");

      auto sizeNode = lightNode.append_child(L"size");

      sizeNode.append_attribute(L"half_length").set_value(L"0.25");
      sizeNode.append_attribute(L"half_width").set_value(L"0.19");

      auto intensityNode = lightNode.append_child(L"intensity");
      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"0.404987 0.404987 0.404987");

      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(17.0f);
      VERIFY_XML(lightNode);
    }
    hrLightClose(rectLight);

    auto lights = hr_spectral::CreateSpectralLights(rectLight, wavelengths, light, L"area_light");

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Camera
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRCameraRef camRef = hrCameraCreate(L"my camera");

    hrCameraOpen(camRef, HR_WRITE_DISCARD);
    {
      auto camNode = hrCameraParamNode(camRef);

      camNode.append_child(L"fov").text().set(L"19.5");
      camNode.append_child(L"nearClipPlane").text().set(L"0.01");
      camNode.append_child(L"farClipPlane").text().set(L"100.0");

      camNode.append_child(L"up").text().set(L"0 1 0");
      camNode.append_child(L"position").text().set(L"0 1 6.8");
      camNode.append_child(L"look_at").text().set(L"0 1 0");
    }
    hrCameraClose(camRef);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Render settings
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRRenderRef renderRef = TEST_UTILS::CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 512, 512, 256, 512);
    hrRenderOpen(renderRef, HR_OPEN_EXISTING);
    {
      auto node = hrRenderParamNode(renderRef);
      node.append_child(L"framebuffer_channels").text() = 1;
      node.force_child(L"diff_trace_depth").text() = 5;
    }
    hrRenderClose(renderRef);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Create scene
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    const float DEG_TO_RAD = 0.01745329251f; // float(3.14159265358979323846f) / 180.0f;
    hlm::float4x4 mScale, mRotX, mRotY, mT, mRes;
    hlm::float4x4 identity{};

    mRotY = hlm::rotate4x4Y(180.0f * DEG_TO_RAD);
    mT = hlm::translate4x4({0.0f, 1.0f, 0.0f});
    auto openboxMatrix = mT * mRotY;

    mT = hlm::translate4x4({0.0f, 1.98f, -0.03f});
    auto lightMatrix = mT;

    std::vector<std::filesystem::path> savedImages;

    HRSceneInstRef scnRef = hrSceneCreate(L"scene");
    for(int i = 0; i < wavelengths.size(); ++i)
    {
      hrSceneOpen(scnRef, HR_WRITE_DISCARD);

      std::vector<int> openboxRemapList = {matGray.id, matGray.id,
                                           greenMaterials[0].id, greenMaterials[i].id,
                                           redMaterials[0].id, redMaterials[i].id};
      hrMeshInstance(scnRef, box, openboxMatrix.L(), openboxRemapList.data(), openboxRemapList.size());

      hrMeshInstance(scnRef, tallBox, identity.L());
      hrMeshInstance(scnRef, smallBox, identity.L());
      hrLightInstance(scnRef, lights[i], lightMatrix.L());

      ///////////

      hrSceneClose(scnRef);

      hrFlush(scnRef, renderRef, camRef);

      while (true)
      {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);

        if (info.haveUpdateFB)
        {
          auto pres = std::cout.precision(2);
          std::cout << "rendering progress = " << info.progress << "% \r"; std::cout.flush();
          std::cout.precision(pres);
        }

        if (info.finalUpdate)
          break;
      }

      std::wstringstream imgName;
      imgName << wavelengths[i] << "nm";
      std::wstring basePath = L"tests_images/test_cornell_spectral_2/";
      const std::wstring ldrName = basePath + imgName.str() + std::wstring(L".png");
      const std::wstring hdrName = basePath + imgName.str() + std::wstring(L".exr");

      hrRenderSaveFrameBufferLDR(renderRef, ldrName.c_str());
      hrRenderSaveFrameBufferHDR(renderRef, hdrName.c_str());
      savedImages.push_back(hdrName);
    }

    std::filesystem::path rgbPath {"tests_images/test_cornell_spectral_2/z_out.exr"};
    hr_spectral::SpectralImagesToRGB(rgbPath, savedImages, wavelengths);

    return check_images("test_cornell_spectral_2", 1, 25);
  }

  bool test_macbeth()
  {
    hrErrorCallerPlace(L"test_macbeth");

    hrSceneLibraryOpen(L"tests/test_macbeth", HR_WRITE_DISCARD);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Materials
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    std::vector<int> wavelengths = {400, 440, 480, 520, 560, 600, 640, 680};
    std::vector<std::vector<HRMaterialRef>> materials(24);
    materials[0]  = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth-1.spd", wavelengths, L"m1");
    materials[1]  = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth-2.spd", wavelengths, L"m2");
    materials[2]  = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth-3.spd", wavelengths, L"m3");
    materials[3]  = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth-4.spd", wavelengths, L"m4");
    materials[4]  = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth-5.spd", wavelengths, L"m5");
    materials[5]  = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth-6.spd", wavelengths, L"m6");
    materials[6]  = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth-7.spd", wavelengths, L"m7");
    materials[7]  = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth-8.spd", wavelengths, L"m8");
    materials[8]  = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth-9.spd", wavelengths, L"m9");
    materials[9]  = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth-10.spd", wavelengths, L"m10");
    materials[10] = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth-11.spd", wavelengths, L"m11");
    materials[11] = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth-12.spd", wavelengths, L"m12");
    materials[12] = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth-13.spd", wavelengths, L"m13");
    materials[13] = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth-14.spd", wavelengths, L"m14");
    materials[14] = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth-15.spd", wavelengths, L"m15");
    materials[15] = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth-16.spd", wavelengths, L"m16");
    materials[16] = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth-17.spd", wavelengths, L"m17");
    materials[17] = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth-18.spd", wavelengths, L"m18");
    materials[18] = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth-19.spd", wavelengths, L"m19");
    materials[19] = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth-20.spd", wavelengths, L"m20");
    materials[20] = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth-21.spd", wavelengths, L"m21");
    materials[21] = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth-22.spd", wavelengths, L"m22");
    materials[22] = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth-23.spd", wavelengths, L"m23");
    materials[23] = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth-24.spd", wavelengths, L"m24");

    HRMaterialRef matBlack = hrMaterialCreate(L"matBlack");
    hrMaterialOpen(matBlack, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matBlack);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
//      color.append_attribute(L"val").set_value(L"0.725 0.71 0.68");
      color.append_attribute(L"val").set_value(L"0.01 0.01 0.01");

      VERIFY_XML(matNode);
    }
    hrMaterialClose(matBlack);

    HRMaterialRef matTest = hrMaterialCreate(L"matTest");
    hrMaterialOpen(matTest, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matTest);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
//      color.append_attribute(L"val").set_value(L"0.725 0.71 0.68");
      color.append_attribute(L"val").set_value(L"0.8 0.0 0.8");

      VERIFY_XML(matNode);
    }
    hrMaterialClose(matTest);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Meshes
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    HRMeshRef chip = TEST_UTILS::HRMeshFromSimpleMesh(L"macbeth_chip", CreatePlane(0.5f), matBlack.id);


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Light
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    HRLightRef sky = hrLightCreate(L"sky");

    hrLightOpen(sky, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(sky);

      lightNode.attribute(L"type").set_value(L"sky");

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"0.75 0.75 0.75");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(1.0f);
      VERIFY_XML(lightNode);
    }
    hrLightClose(sky);

    HRLightRef sun = hrLightCreate(L"sun");

    hrLightOpen(sun, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(sun);

      lightNode.attribute(L"type").set_value(L"directional");
      lightNode.attribute(L"shape").set_value(L"point");
      lightNode.attribute(L"distribution").set_value(L"directional");

      auto sizeNode = lightNode.append_child(L"size");
      sizeNode.append_attribute(L"inner_radius").set_value(L"0.0");
      sizeNode.append_attribute(L"outer_radius").set_value(L"1000.0");

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(4.0);

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

      camNode.append_child(L"fov").text().set(L"30");
      camNode.append_child(L"nearClipPlane").text().set(L"0.01");
      camNode.append_child(L"farClipPlane").text().set(L"100.0");

      camNode.append_child(L"up").text().set(L"0 1 0");
      camNode.append_child(L"position").text().set(L"0 0 10");
      camNode.append_child(L"look_at").text().set(L"0 0 -1");
    }
    hrCameraClose(camRef);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Render settings
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRRenderRef renderRef = TEST_UTILS::CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 1280, 720, 256, 256);
    hrRenderOpen(renderRef, HR_OPEN_EXISTING);
    {
      auto node = hrRenderParamNode(renderRef);
      node.append_child(L"framebuffer_channels").text() = 1;
      node.force_child(L"diff_trace_depth").text() = 5;
    }
    hrRenderClose(renderRef);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Create scene
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    const float DEG_TO_RAD = 0.01745329251f; // float(3.14159265358979323846f) / 180.0f;
    hlm::float4x4 mScale, mRotX, mRotY, mT, mRes;
    hlm::float4x4 identity{};

    std::vector<std::filesystem::path> savedImages;

    HRSceneInstRef scnRef = hrSceneCreate(L"scene");
    for(int i = 0; i < wavelengths.size(); ++i)
    {
      hrSceneOpen(scnRef, HR_WRITE_DISCARD);

      mRotX = hlm::rotate4x4X(90.0f * DEG_TO_RAD);
      constexpr uint32_t N_CHIPS_X   = 6;
      constexpr uint32_t N_CHIPS_Y   = 4;
      constexpr float    CHIP_SIZE   = 1.0f;
      constexpr float    BORDER_SIZE = 0.1f;
      constexpr float    PLATE_X     = N_CHIPS_X * CHIP_SIZE + (N_CHIPS_X + 1) * BORDER_SIZE;
      constexpr float    PLATE_Y     = N_CHIPS_Y * CHIP_SIZE + (N_CHIPS_Y + 1) * BORDER_SIZE;

      mScale = hlm::scale4x4({PLATE_X, 1.0, PLATE_Y});
      mT = hlm::translate4x4({0.0f, 0.0f, -0.001f});
      mRes = mT * mRotX * mScale;
      hrMeshInstance(scnRef, chip, mRes.L());
//      hrLightInstance(scnRef, sky, identity.L());

      mT = translate4x4(float3(0.0f, 0.0f, 100.0f));
      mRotX = rotate4x4X(90.0f*DEG_TO_RAD);
      mRes = mT * mRotX;
      hrLightInstance(scnRef, sun, mRes.L());

      for (int x = 0; x < N_CHIPS_X; ++x)
      {
        for (int y = 0; y < N_CHIPS_Y; ++y)
        {
          float xx = -PLATE_X * 0.5f + float(x) * (CHIP_SIZE + BORDER_SIZE) + CHIP_SIZE * 0.5f + BORDER_SIZE;
          float yy = +PLATE_Y * 0.5f - float(y) * (CHIP_SIZE + BORDER_SIZE) - CHIP_SIZE * 0.5f - BORDER_SIZE;
          mT = hlm::translate4x4({xx, yy, 0.0f});
          mRes = mT * mRotX;

          uint32_t chipId = x + y * N_CHIPS_X;
          std::vector<int> remapList = {matBlack.id, materials[chipId][i].id};
          hrMeshInstance(scnRef, chip, mRes.L(), remapList.data(), remapList.size());
        }
      }

      ///////////

      hrSceneClose(scnRef);

      hrFlush(scnRef, renderRef, camRef);

      while (true)
      {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);

        if (info.haveUpdateFB)
        {
          auto pres = std::cout.precision(2);
          std::cout << "rendering progress = " << info.progress << "% \r"; std::cout.flush();
          std::cout.precision(pres);
        }

        if (info.finalUpdate)
          break;
      }

      std::wstringstream imgName;
      imgName << wavelengths[i] << "nm";
//      imgName << "_";
      std::wstring basePath = L"tests_images/test_macbeth/";
      const std::wstring ldrName = basePath + imgName.str() + std::wstring(L".png");
      const std::wstring hdrName = basePath + imgName.str() + std::wstring(L".exr");

      hrRenderSaveFrameBufferLDR(renderRef, ldrName.c_str());
      hrRenderSaveFrameBufferHDR(renderRef, hdrName.c_str());
      savedImages.push_back(hdrName);
    }

    std::filesystem::path rgbPath {"tests_images/test_macbeth/z_out.exr"};
    hr_spectral::SpectralImagesToRGB(rgbPath, savedImages, wavelengths);

    return check_images("test_macbeth", 1, 25);
  }

  bool test_texture_1()
  {
    hrErrorCallerPlace(L"test_texture_1");

    hrSceneLibraryOpen(L"test_texture_1", HR_WRITE_DISCARD);

    constexpr int TOTAL_WAVELENGTHS = 31;
    std::vector<int> wavelengths(TOTAL_WAVELENGTHS);
    std::generate(wavelengths.begin(), wavelengths.end(), [n = 390] () mutable { return n = n + 10; });

    const std::filesystem::path basePath {"data/spectral/glass_tiles"};
    const std::string baseTexName {"glass_tiles_ms"};
    std::vector<std::filesystem::path> texPaths(TOTAL_WAVELENGTHS);
    for(int n = 0; n < TOTAL_WAVELENGTHS ; ++n)
    {
      std::stringstream ss;
      ss << baseTexName << "_" << std::setfill('0') << std::setw(2) << n + 1 << ".png";
      auto path = basePath;
      path.append(ss.str());
      texPaths[n] = path;
    }

    hlm::float4x4 texMatrix;
    auto materials = hr_spectral::CreateSpectralTexturedDiffuseMaterials(wavelengths, texPaths, texMatrix);

    SimpleMesh plane = CreatePlane(4.0f);
    HRMeshRef planeRef = hrMeshCreate(L"my_plane");
    hrMeshOpen(planeRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
    {
      hrMeshVertexAttribPointer4f(planeRef, L"pos", &plane.vPos[0]);
      hrMeshVertexAttribPointer4f(planeRef, L"norm", &plane.vNorm[0]);
      hrMeshVertexAttribPointer2f(planeRef, L"texcoord", &plane.vTexCoord[0]);
      hrMeshMaterialId(planeRef, materials[0].id);
      hrMeshAppendTriangles3(planeRef, int(plane.triIndices.size()), &plane.triIndices[0]);
    }
    hrMeshClose(planeRef);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRLightRef sky = hrLightCreate(L"sky");

    hrLightOpen(sky, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(sky);

      lightNode.attribute(L"type").set_value(L"sky");

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(1.0f);

      VERIFY_XML(lightNode);
    }
    hrLightClose(sky);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // camera
    //
    HRCameraRef camRef = hrCameraCreate(L"my camera");

    hrCameraOpen(camRef, HR_WRITE_DISCARD);
    {
      xml_node camNode = hrCameraParamNode(camRef);

      camNode.append_child(L"fov").text().set(L"30");
      camNode.append_child(L"nearClipPlane").text().set(L"0.01");
      camNode.append_child(L"farClipPlane").text().set(L"100.0");

      camNode.append_child(L"up").text().set(L"0 1 0");
      camNode.append_child(L"position").text().set(L"0 0 14.9");
      camNode.append_child(L"look_at").text().set(L"0 0 0");
    }
    hrCameraClose(camRef);


    HRRenderRef renderRef = TEST_UTILS::CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 512, 512, 256, 512);
    hrRenderOpen(renderRef, HR_OPEN_EXISTING);
    {
      auto node = hrRenderParamNode(renderRef);
      node.append_child(L"framebuffer_channels").text() = 1;
    }
    hrRenderClose(renderRef);
    hrRenderEnableDevice(renderRef, CURR_RENDER_DEVICE, true);


    auto mT = translate4x4(float3(0.0f, 0.0f, 100.0f));
    auto mRotX = rotate4x4X(90.0f*DEG_TO_RAD);
    hlm::float4x4 identity;
    auto planeMatrix = hlm::rotate4x4X(-DEG_TO_RAD * 90.0f);

    std::vector<std::filesystem::path> savedImages;
    HRSceneInstRef scnRef = hrSceneCreate(L"scene");
    for(int i = 0; i < wavelengths.size(); ++i)
    {
      hrSceneOpen(scnRef, HR_WRITE_DISCARD);
      {
        std::vector<int> remapList = {materials[0].id, materials[i].id};
        hrMeshInstance(scnRef, planeRef, planeMatrix.L(), remapList.data(), remapList.size());

        hrLightInstance(scnRef, sky, identity.L());
      }
      hrSceneClose(scnRef);

      hrFlush(scnRef, renderRef, camRef);

      while (true)
      {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);

        if (info.haveUpdateFB)
        {
          auto pres = std::cout.precision(2);
          std::cout << "rendering progress = " << info.progress << "% \r"; std::cout.flush();
          std::cout.precision(pres);
        }

        if (info.finalUpdate)
          break;
      }

      std::wstringstream imgName;
      imgName << wavelengths[i] << "nm";
      const std::wstring outBasePath = L"tests_images/test_texture_1/";
      const std::wstring ldrName = outBasePath + imgName.str() + std::wstring(L".png");
      const std::wstring hdrName = outBasePath + imgName.str() + std::wstring(L".exr");

      hrRenderSaveFrameBufferLDR(renderRef, ldrName.c_str());
      hrRenderSaveFrameBufferHDR(renderRef, hdrName.c_str());
      savedImages.push_back(hdrName);
    }

    std::filesystem::path rgbPath {"tests_images/test_texture_1/z_out.exr"};
    hr_spectral::SpectralImagesToRGB(rgbPath, savedImages, wavelengths);

    return check_images("test_texture_1", 1, 25);
  }
}