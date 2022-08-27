#include <cassert>
#include <filesystem>
#include <algorithm>
#include <numeric>
#include <random>
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

    std::vector<float> wavelengths = {400,        440,       480,      520,      560,      600,      640,        680};
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
//      node.force_child(L"diff_trace_depth").text() = 5;
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

    std::filesystem::path avgPath {"tests_images/test_cornell_spectral_2/averageSpectrum.exr"};
    hr_spectral::AverageSpectralImages(avgPath, savedImages);

    std::filesystem::path avgPath2 {"tests_images/test_cornell_spectral_2/averageSpectrumV2.exr"};
    hr_spectral::AverageSpectralImagesV2(avgPath2, savedImages, wavelengths);

    std::filesystem::path yPath {"tests_images/test_cornell_spectral_2/Y.exr"};
    hr_spectral::SpectralImagesToY(yPath, savedImages, wavelengths);

    return check_images_HDR("test_cornell_spectral_2", 1, 25);
  }

  bool test_macbeth()
  {
    hrErrorCallerPlace(L"test_macbeth");

    hrSceneLibraryOpen(L"tests/test_macbeth", HR_WRITE_DISCARD);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Materials
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//    std::vector<int> wavelengths = {400, 440, 480, 520, 560, 600, 640, 680};
    std::vector<float> wavelengths;

    for(int w = 400; w <= 700; w += 10)
      wavelengths.push_back(float(w));

    std::vector<std::vector<HRMaterialRef>> materials(24);
    materials[0]  = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth_spectra/macbeth-1_new.spd", wavelengths, L"m1");
    materials[1]  = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth_spectra/macbeth-2_new.spd", wavelengths, L"m2");
    materials[2]  = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth_spectra/macbeth-3_new.spd", wavelengths, L"m3");
    materials[3]  = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth_spectra/macbeth-4_new.spd", wavelengths, L"m4");
    materials[4]  = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth_spectra/macbeth-5_new.spd", wavelengths, L"m5");
    materials[5]  = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth_spectra/macbeth-6_new.spd", wavelengths, L"m6");
    materials[6]  = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth_spectra/macbeth-7_new.spd", wavelengths, L"m7");
    materials[7]  = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth_spectra/macbeth-8_new.spd", wavelengths, L"m8");
    materials[8]  = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth_spectra/macbeth-9_new.spd", wavelengths, L"m9");
    materials[9]  = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth_spectra/macbeth-10_new.spd", wavelengths, L"m10");
    materials[10] = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth_spectra/macbeth-11_new.spd", wavelengths, L"m11");
    materials[11] = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth_spectra/macbeth-12_new.spd", wavelengths, L"m12");
    materials[12] = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth_spectra/macbeth-13_new.spd", wavelengths, L"m13");
    materials[13] = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth_spectra/macbeth-14_new.spd", wavelengths, L"m14");
    materials[14] = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth_spectra/macbeth-15_new.spd", wavelengths, L"m15");
    materials[15] = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth_spectra/macbeth-16_new.spd", wavelengths, L"m16");
    materials[16] = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth_spectra/macbeth-17_new.spd", wavelengths, L"m17");
    materials[17] = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth_spectra/macbeth-18_new.spd", wavelengths, L"m18");
    materials[18] = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth_spectra/macbeth-19_new.spd", wavelengths, L"m19");
    materials[19] = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth_spectra/macbeth-20_new.spd", wavelengths, L"m20");
    materials[20] = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth_spectra/macbeth-21_new.spd", wavelengths, L"m21");
    materials[21] = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth_spectra/macbeth-22_new.spd", wavelengths, L"m22");
    materials[22] = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth_spectra/macbeth-23_new.spd", wavelengths, L"m23");
    materials[23] = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth_spectra/macbeth-24_new.spd", wavelengths, L"m24");

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

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
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


//    auto d65_lights = hr_spectral::CreateSpectralLightsD65(sky, wavelengths);
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

//      hrLightInstance(scnRef, d65_lights[i], identity.L());

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

    std::filesystem::path avgPath {"tests_images/test_macbeth/averageSpectrum.exr"};
    hr_spectral::AverageSpectralImages(avgPath, savedImages);

    std::filesystem::path avgPath2 {"tests_images/test_macbeth/averageSpectrumV2.exr"};
    hr_spectral::AverageSpectralImagesV2(avgPath2, savedImages, wavelengths);

    std::filesystem::path yPath {"tests_images/test_macbeth/Y.exr"};
    hr_spectral::SpectralImagesToY(yPath, savedImages, wavelengths);

    return check_images_HDR("test_macbeth", 1, 25);
  }

  bool test_texture_1()
  {
    hrErrorCallerPlace(L"test_texture_1");

    hrSceneLibraryOpen(L"tests/test_texture_1", HR_WRITE_DISCARD);

    constexpr int TOTAL_WAVELENGTHS = 31;
    std::vector<float> wavelengths(TOTAL_WAVELENGTHS);
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

    std::filesystem::path avgPath {"tests_images/test_texture_1/averageSpectrum.exr"};
    hr_spectral::AverageSpectralImages(avgPath, savedImages);

    std::filesystem::path avgPath2 {"tests_images/test_texture_1/averageSpectrumV2.exr"};
    hr_spectral::AverageSpectralImagesV2(avgPath2, savedImages, wavelengths);

    std::filesystem::path yPath {"tests_images/test_texture_1/Y.exr"};
    hr_spectral::SpectralImagesToY(yPath, savedImages, wavelengths);

    return check_images_HDR("test_texture_1", 1, 25);
  }

  bool test_tile()
  {
    hrErrorCallerPlace(L"test_tile");

    hrSceneLibraryOpen(L"tests/test_tile", HR_WRITE_DISCARD);

    std::vector<float> wavelengths;
    for(int w = 400; w <= 700; w += 10)
      wavelengths.push_back(float(w));
    auto materials = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth_spectra/macbeth-21_new.spd", wavelengths, L"m21");

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
      const std::wstring outBasePath = L"tests_images/test_tile/";
      const std::wstring ldrName = outBasePath + imgName.str() + std::wstring(L".png");
      const std::wstring hdrName = outBasePath + imgName.str() + std::wstring(L".exr");

      hrRenderSaveFrameBufferLDR(renderRef, ldrName.c_str());
      hrRenderSaveFrameBufferHDR(renderRef, hdrName.c_str());
      savedImages.push_back(hdrName);
    }

    std::filesystem::path rgbPath {"tests_images/test_tile/z_out.exr"};
    hr_spectral::SpectralImagesToRGB(rgbPath, savedImages, wavelengths);

    std::filesystem::path avgPath {"tests_images/test_tile/averageSpectrum.exr"};
    hr_spectral::AverageSpectralImages(avgPath, savedImages);

    std::filesystem::path avgPath2 {"tests_images/test_tile/averageSpectrumV2.exr"};
    hr_spectral::AverageSpectralImagesV2(avgPath2, savedImages, wavelengths);

    std::filesystem::path yPath {"tests_images/test_tile/Y.exr"};
    hr_spectral::SpectralImagesToY(yPath, savedImages, wavelengths);

    return check_images_HDR("test_tile", 1, 25);
  }

  bool test_macbeth_2() // render 3 wavelengths at a time
  {
    hrErrorCallerPlace(L"test_macbeth_2");

    hrSceneLibraryOpen(L"tests/test_macbeth_2", HR_WRITE_DISCARD);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Materials
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//    std::vector<int> wavelengths = {400, 440, 480, 520, 560, 600, 640, 680};
    std::vector<float> wavelengths;

    for(int w = 400; w <= 700; w += 10)
      wavelengths.push_back(float(w));

    std::vector<std::vector<HRMaterialRef>> materials(24);
    materials[0]  = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile3channel("data/spectral/macbeth_spectra/macbeth-1_new.spd", wavelengths, L"m1");
    materials[1]  = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile3channel("data/spectral/macbeth_spectra/macbeth-2_new.spd", wavelengths, L"m2");
    materials[2]  = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile3channel("data/spectral/macbeth_spectra/macbeth-3_new.spd", wavelengths, L"m3");
    materials[3]  = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile3channel("data/spectral/macbeth_spectra/macbeth-4_new.spd", wavelengths, L"m4");
    materials[4]  = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile3channel("data/spectral/macbeth_spectra/macbeth-5_new.spd", wavelengths, L"m5");
    materials[5]  = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile3channel("data/spectral/macbeth_spectra/macbeth-6_new.spd", wavelengths, L"m6");
    materials[6]  = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile3channel("data/spectral/macbeth_spectra/macbeth-7_new.spd", wavelengths, L"m7");
    materials[7]  = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile3channel("data/spectral/macbeth_spectra/macbeth-8_new.spd", wavelengths, L"m8");
    materials[8]  = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile3channel("data/spectral/macbeth_spectra/macbeth-9_new.spd", wavelengths, L"m9");
    materials[9]  = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile3channel("data/spectral/macbeth_spectra/macbeth-10_new.spd", wavelengths, L"m10");
    materials[10] = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile3channel("data/spectral/macbeth_spectra/macbeth-11_new.spd", wavelengths, L"m11");
    materials[11] = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile3channel("data/spectral/macbeth_spectra/macbeth-12_new.spd", wavelengths, L"m12");
    materials[12] = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile3channel("data/spectral/macbeth_spectra/macbeth-13_new.spd", wavelengths, L"m13");
    materials[13] = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile3channel("data/spectral/macbeth_spectra/macbeth-14_new.spd", wavelengths, L"m14");
    materials[14] = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile3channel("data/spectral/macbeth_spectra/macbeth-15_new.spd", wavelengths, L"m15");
    materials[15] = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile3channel("data/spectral/macbeth_spectra/macbeth-16_new.spd", wavelengths, L"m16");
    materials[16] = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile3channel("data/spectral/macbeth_spectra/macbeth-17_new.spd", wavelengths, L"m17");
    materials[17] = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile3channel("data/spectral/macbeth_spectra/macbeth-18_new.spd", wavelengths, L"m18");
    materials[18] = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile3channel("data/spectral/macbeth_spectra/macbeth-19_new.spd", wavelengths, L"m19");
    materials[19] = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile3channel("data/spectral/macbeth_spectra/macbeth-20_new.spd", wavelengths, L"m20");
    materials[20] = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile3channel("data/spectral/macbeth_spectra/macbeth-21_new.spd", wavelengths, L"m21");
    materials[21] = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile3channel("data/spectral/macbeth_spectra/macbeth-22_new.spd", wavelengths, L"m22");
    materials[22] = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile3channel("data/spectral/macbeth_spectra/macbeth-23_new.spd", wavelengths, L"m23");
    materials[23] = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile3channel("data/spectral/macbeth_spectra/macbeth-24_new.spd", wavelengths, L"m24");

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


//    auto d65_lights = hr_spectral::CreateSpectralLightsD65(sky, wavelengths);
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
//    hrRenderOpen(renderRef, HR_OPEN_EXISTING);
//    {
//      auto node = hrRenderParamNode(renderRef);
//      node.append_child(L"framebuffer_channels").text() = 1;
//      node.force_child(L"diff_trace_depth").text() = 5;
//    }
//    hrRenderClose(renderRef);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Create scene
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    const float DEG_TO_RAD = 0.01745329251f; // float(3.14159265358979323846f) / 180.0f;
    hlm::float4x4 mScale, mRotX, mRotY, mT, mRes;
    hlm::float4x4 identity{};

    std::vector<std::filesystem::path> savedImages;

    HRSceneInstRef scnRef = hrSceneCreate(L"scene");
    int i = 0;
    uint32_t materialIdx = 0;
    while(i < wavelengths.size())
    {
      if(wavelengths.size() - i == 1 || wavelengths.size() - i == 2)
      {
        hrRenderOpen(renderRef, HR_OPEN_EXISTING);
        {
          auto node = hrRenderParamNode(renderRef);
          node.append_child(L"framebuffer_channels").text() = 1;
        }
        hrRenderClose(renderRef);
      }

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
          std::vector<int> remapList = {matBlack.id, materials[chipId][materialIdx].id};
          hrMeshInstance(scnRef, chip, mRes.L(), remapList.data(), remapList.size());
        }
      }
      materialIdx++;

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
      if(wavelengths.size() - i == 1 || wavelengths.size() - i == 2)
      {
        imgName << wavelengths[i] << "nm";
        i += 1;
      }
      else
      {
        imgName << wavelengths[i] << "-" << wavelengths[i + 1] << "-" << wavelengths[i + 2] << "nm";
        i += 3;
      }


      std::wstring basePath = L"tests_images/test_macbeth_2/";
      const std::wstring ldrName = basePath + imgName.str() + std::wstring(L".png");
      const std::wstring hdrName = basePath + imgName.str() + std::wstring(L".exr");

      hrRenderSaveFrameBufferLDR(renderRef, ldrName.c_str());
      hrRenderSaveFrameBufferHDR(renderRef, hdrName.c_str());
      savedImages.push_back(hdrName);

    }

    std::filesystem::path rgbPath {"tests_images/test_macbeth_2/z_out.exr"};
    hr_spectral::SpectralImagesToRGB(rgbPath, savedImages, wavelengths);

    std::filesystem::path avgPath {"tests_images/test_macbeth_2/averageSpectrum.exr"};
    hr_spectral::AverageSpectralImages(avgPath, savedImages);

    std::filesystem::path avgPath2 {"tests_images/test_macbeth_2/averageSpectrumV2.exr"};
    hr_spectral::AverageSpectralImagesV2(avgPath2, savedImages, wavelengths);

    std::filesystem::path yPath {"tests_images/test_macbeth_2/Y.exr"};
    hr_spectral::SpectralImagesToY(yPath, savedImages, wavelengths);

    return check_images_HDR("test_macbeth_2", 1, 25);
  }

  bool test_macbeth_3() // use hydra api shared framebuffer
  {
    hrErrorCallerPlace(L"test_macbeth_3");

    hrSceneLibraryOpen(L"tests/test_macbeth_3", HR_WRITE_DISCARD);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Materials
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    std::vector<float> wavelengths;

    for(int w = 400; w <= 700; w += 10)
      wavelengths.push_back(float(w));

    std::vector<std::vector<HRMaterialRef>> materials(24);
    materials[0]  = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth_spectra/macbeth-1_new.spd", wavelengths, L"m1");
    materials[1]  = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth_spectra/macbeth-2_new.spd", wavelengths, L"m2");
    materials[2]  = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth_spectra/macbeth-3_new.spd", wavelengths, L"m3");
    materials[3]  = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth_spectra/macbeth-4_new.spd", wavelengths, L"m4");
    materials[4]  = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth_spectra/macbeth-5_new.spd", wavelengths, L"m5");
    materials[5]  = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth_spectra/macbeth-6_new.spd", wavelengths, L"m6");
    materials[6]  = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth_spectra/macbeth-7_new.spd", wavelengths, L"m7");
    materials[7]  = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth_spectra/macbeth-8_new.spd", wavelengths, L"m8");
    materials[8]  = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth_spectra/macbeth-9_new.spd", wavelengths, L"m9");
    materials[9]  = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth_spectra/macbeth-10_new.spd", wavelengths, L"m10");
    materials[10] = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth_spectra/macbeth-11_new.spd", wavelengths, L"m11");
    materials[11] = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth_spectra/macbeth-12_new.spd", wavelengths, L"m12");
    materials[12] = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth_spectra/macbeth-13_new.spd", wavelengths, L"m13");
    materials[13] = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth_spectra/macbeth-14_new.spd", wavelengths, L"m14");
    materials[14] = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth_spectra/macbeth-15_new.spd", wavelengths, L"m15");
    materials[15] = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth_spectra/macbeth-16_new.spd", wavelengths, L"m16");
    materials[16] = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth_spectra/macbeth-17_new.spd", wavelengths, L"m17");
    materials[17] = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth_spectra/macbeth-18_new.spd", wavelengths, L"m18");
    materials[18] = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth_spectra/macbeth-19_new.spd", wavelengths, L"m19");
    materials[19] = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth_spectra/macbeth-20_new.spd", wavelengths, L"m20");
    materials[20] = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth_spectra/macbeth-21_new.spd", wavelengths, L"m21");
    materials[21] = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth_spectra/macbeth-22_new.spd", wavelengths, L"m22");
    materials[22] = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth_spectra/macbeth-23_new.spd", wavelengths, L"m23");
    materials[23] = hr_spectral::CreateSpectralDiffuseMaterialsFromSPDFile("data/spectral/macbeth_spectra/macbeth-24_new.spd", wavelengths, L"m24");

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


//    auto d65_lights = hr_spectral::CreateSpectralLightsD65(sky, wavelengths);
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
    const uint32_t samplesTotal = wavelengths.size() * 256;

    HRRenderRef renderRef = TEST_UTILS::CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 1280, 720, 256, 256);
    hrRenderOpen(renderRef, HR_OPEN_EXISTING);
    {
      auto node = hrRenderParamNode(renderRef);
      node.append_child(L"framebuffer_channels").text() = 1;
//      node.force_child(L"diff_trace_depth").text() = 5;
      node.force_child(L"maxRaysPerPixel").text() = samplesTotal;
      node.force_child(L"dont_run").text() = 1;
    }
    hrRenderClose(renderRef);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Create scenes
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    const float DEG_TO_RAD = 0.01745329251f; // float(3.14159265358979323846f) / 180.0f;
    hlm::float4x4 mScale, mRotX, mRotY, mT, mRes;
    hlm::float4x4 identity{};

    HRSceneInstRef scnRef = hrSceneCreate(L"scene");
    for(size_t i = 0; i < wavelengths.size(); ++i)
    {
      hrSceneOpen(scnRef, HR_WRITE_DISCARD);
      {
        mRotX = hlm::rotate4x4X(90.0f * DEG_TO_RAD);
        constexpr uint32_t N_CHIPS_X = 6;
        constexpr uint32_t N_CHIPS_Y = 4;
        constexpr float CHIP_SIZE = 1.0f;
        constexpr float BORDER_SIZE = 0.1f;
        constexpr float PLATE_X = N_CHIPS_X * CHIP_SIZE + (N_CHIPS_X + 1) * BORDER_SIZE;
        constexpr float PLATE_Y = N_CHIPS_Y * CHIP_SIZE + (N_CHIPS_Y + 1) * BORDER_SIZE;

        mScale = hlm::scale4x4({PLATE_X, 1.0, PLATE_Y});
        mT = hlm::translate4x4({0.0f, 0.0f, -0.001f});
        mRes = mT * mRotX * mScale;
        hrMeshInstance(scnRef, chip, mRes.L());

        mT = translate4x4(float3(0.0f, 0.0f, 100.0f));
        mRotX = rotate4x4X(90.0f * DEG_TO_RAD);
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
      }
      hrSceneClose(scnRef);

      hrFlush(scnRef, renderRef, camRef);
    }

    std::vector<std::wstring> states;
    for (uint32_t i = 0; i < wavelengths.size(); ++i)
    {
      std::wstringstream ws;
      ws << L"statex_" << std::fixed << std::setfill(L'0') << std::setw(5) << i + 1 << L".xml";
      states.emplace_back(ws.str());
    }

    const uint32_t numRenderPasses      = states.size();
    const uint32_t samplesPerWavelength = samplesTotal / numRenderPasses;
    float progressStep = 100.0f * float(1.0f) / float(numRenderPasses);
    float frameProgress = 0.0f;

    constexpr uint32_t waitIntervalMillisec = 500;
    int topState = 0;
    int passesDone = 0;
    while (topState < states.size())
    {
      std::wstringstream strOut;
      strOut << L"runhydra -cl_device_id " << 0 << L" -contribsamples " << samplesPerWavelength;
      strOut << L" -statefile " << states[topState].c_str();
      auto str = strOut.str();
      hrRenderCommand(renderRef, str.c_str());

      auto start = std::chrono::high_resolution_clock::now();
      while (true)
      {
        std::this_thread::sleep_for(std::chrono::milliseconds(waitIntervalMillisec / 8));

        HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);

        if (info.haveUpdateFB)
        {
          auto pres = std::cout.precision(2);
          std::cout << "rendering progress = " << info.progress << "% \r";
          std::cout.flush();
          std::cout.precision(pres);
        }
        else
        {
          auto currentTime = std::chrono::high_resolution_clock::now();
          std::chrono::duration<double> diff = currentTime - start;
          if(diff.count() > 180)
          {
            std::cout << "Render probably crashed - no updates for " << diff.count() << " seconds.";
            std::cout << "Trying the next state/scene..." << std::endl;
            break;
          }
        }

        if (info.finalUpdate || info.progress > 0.9995f * (frameProgress + progressStep))
        {
//          hrRenderCommand(a_render, L"exitnow");
          break;
        }
      }
      frameProgress += progressStep;

      std::this_thread::sleep_for(std::chrono::milliseconds(waitIntervalMillisec));
      passesDone++;
      topState++;
    }

    std::wstring basePath = L"tests_images/test_macbeth_3/";
    const std::wstring hdrName = basePath + std::wstring(L"z_out.exr");
    hrRenderSaveFrameBufferHDR(renderRef, hdrName.c_str());

    return check_images_HDR("test_macbeth_3", 1, 25);
  }

  bool generate_nerf_data()
  {
    hrErrorCallerPlace(L"generate_nerf_data");

    hrSceneLibraryOpen(L"tests/generate_nerf_data", HR_WRITE_DISCARD);

    HRMeshRef teapotRef = hrMeshCreateFromFileDL(L"data/meshes/bunny_origin.obj");

    HRTextureNodeRef texChecker = hrTexture2DCreateFromFileDL(L"data/textures/dirt001_diffuse.tif");

    HRMaterialRef mat0 = hrMaterialCreate(L"mysimplemat");
    hrMaterialOpen(mat0, HR_WRITE_DISCARD);
    {
      xml_node matNode = hrMaterialParamNode(mat0);

      xml_node diff = matNode.append_child(L"diffuse");

      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").text().set(L"1 1 1");

      xml_node refl = matNode.append_child(L"reflectivity");

      refl.append_attribute(L"brdf_type").set_value(L"phong");
      refl.append_child(L"color").text().set(L"0.15 0.15 0.15");
      refl.append_child(L"glossiness").text().set(L"0.9");

      auto texNode = hrTextureBind(texChecker, diff.child(L"color"));
      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 2, 0, 0, 0,
                                  0, 2, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };

      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

      auto displacement = matNode.append_child(L"displacement");
      auto heightNode   = displacement.append_child(L"height_map");

      displacement.append_attribute(L"type").set_value(L"height_bump");
      heightNode.append_attribute(L"amount").set_value(0.25f);

      auto texNode2 = hrTextureBind(texChecker, heightNode);

      texNode2.append_attribute(L"matrix");
      texNode2.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode2.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode2.append_attribute(L"input_gamma").set_value(2.2f);
      texNode2.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode2, L"matrix", samplerMatrix);
    }
    hrMaterialClose(mat0);

    HRMaterialRef mat1 = hrMaterialCreate(L"teapotmat");
    hrMaterialOpen(mat1, HR_WRITE_DISCARD);
    {
      xml_node matNode = hrMaterialParamNode(mat1);

      xml_node diff = matNode.append_child(L"diffuse");

      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").text().set(L"0.207843 0.188235 0");

      xml_node refl = matNode.append_child(L"reflectivity");

      refl.append_attribute(L"brdf_type").set_value(L"phong");
      refl.append_child(L"color").text().set(L"0.367059 0.345882 0");
      refl.append_child(L"glossiness").text().set(L"0.75");

      hrTextureBind(texChecker, refl.child(L"color"));
    }
    hrMaterialClose(mat1);

    HRTextureNodeRef texEnv = hrTexture2DCreateFromFile(L"data/textures/LA_Downtown_Afternoon_Fishing_3k.hdr");
    HRLightRef skyLight = hrLightCreate(L"my_sky_light");

    hrLightOpen(skyLight, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(skyLight);

      lightNode.attribute(L"type").set_value(L"sky");
      lightNode.attribute(L"distribution").set_value(L"map");

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"2.0");

//      auto texNode = hrTextureBind(texEnv, intensityNode.child(L"color"));
//
//      texNode.append_attribute(L"matrix");
//      float samplerMatrix[16] = { 1, 0, 0, 0,
//                                  0, 1, 0, 0,
//                                  0, 0, 1, 0,
//                                  0, 0, 0, 1 };
//
//      texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
//      texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
//
//      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

      VERIFY_XML(lightNode);
    }
    hrLightClose(skyLight);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRRenderRef renderRef = hrRenderCreate(L"HydraModern"); // opengl1 //
    hrRenderEnableDevice(renderRef, CURR_RENDER_DEVICE, true);
    hrRenderOpen(renderRef, HR_WRITE_DISCARD);
    {
      pugi::xml_node node = hrRenderParamNode(renderRef);

      node.append_child(L"width").text() = 400;
      node.append_child(L"height").text() = 400;

      node.append_child(L"method_primary").text() = L"pathtracing";
      node.append_child(L"trace_depth").text() = 5;
      node.append_child(L"diff_trace_depth").text() = 3;
      node.append_child(L"maxRaysPerPixel").text() = 512;
      node.append_child(L"evalgbuffer").text() = 1;
//      node.append_child(L"offline_pt").text()       = 0;
    }
    hrRenderClose(renderRef);

    // create scene
    //
    HRSceneInstRef scnRef = hrSceneCreate(L"my scene");

    hrSceneOpen(scnRef, HR_WRITE_DISCARD);
    {
      auto identity = LiteMath::float4x4{};
      auto teapotM = LiteMath::float4x4{};//LiteMath::translate4x4({0.25f, -0.75f, 0.0f});//
      hrMeshInstance(scnRef, teapotRef, teapotM.L());
      hrLightInstance(scnRef, skyLight, identity.L());
    }
    hrSceneClose(scnRef);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    float4x4 projMatrix = LiteMath::perspectiveMatrix(45.0f, 1.0f, 0.01f, 100.0f);

    uint32_t n_points = 150;
    constexpr float pi = 3.14159265358979323846f;
    float delta = (2.0f * pi) / ((float)n_points);
    float radius = 3.0f;
    std::vector<float4x4> viewMatrices;
    viewMatrices.reserve(n_points * n_points);

    std::ofstream matrixFile("tests_images/generate_nerf_data/matrices.txt");
    float3 up_init = float3(0, 1, 0);
    float3 eye_init = float3(0, 0, 3);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> phi_dist(-pi, pi);
    std::uniform_real_distribution<float> theta_dist(0, 2 * pi);
    for(int i = 0; i < n_points; ++i)
    {
      float phi = phi_dist(gen);
      float theta = theta_dist(gen);

      float3 eye {radius * sinf(phi) * sinf(theta),
                  radius * cosf(phi),
                  radius * sinf(phi) * cosf(theta)};
      if(1.0f - dot(up_init, eye) < 0.001f)
        up_init = float3{0, 0, 1};

      auto mat = LiteMath::lookAt(eye, float3(0, 0, 0), up_init);
      auto matInv = LiteMath::inverse4x4(mat);
      viewMatrices.push_back(mat);

      for(int x = 0; x < 4; ++x)
      {
        for(int y = 0; y < 4; ++y)
        {
          matrixFile << matInv(x, y) << ' ';
        }
      }
      matrixFile << std::endl;
    }
//    for(int i = 0; i < n_points; ++i)
//    {
//      float phi = -pi + delta * i;
////      float phi = phi_dist(gen);
//      for(int j = 0; j < n_points; ++j)
//      {
//        float theta = delta * j;
////        float theta = theta_dist(gen);
//        float3 eye {radius * sinf(phi) * sinf(theta),
//                    radius * cosf(phi),
//                    radius * sinf(phi) * cosf(theta),
//        };
//        if(1.0f - dot(up_init, eye) < 0.001f)
//          up_init = float3{0, 0, 1};
//        viewMatrices.push_back(LiteMath::lookAt(eye, float3(0, 0, 0), up_init));
//
//        for(int x = 0; x < 4; ++x)
//        {
//          for(int y = 0; y < 4; ++y)
//          {
//            matrixFile << viewMatrices.back()(x, y) << ' ';
//          }
//        }
//        matrixFile << std::endl;
//
//      }
//    }

    int idx = 0;
    HRCameraRef camRef = hrCameraCreate(L"my camera");
    for(const auto& view : viewMatrices)
    {
//      auto view = LiteMath::lookAt({0, 0, 3}, float3(0, 0, 0), {0, 1, 0});
      hrCameraOpen(camRef, HR_WRITE_DISCARD);
      {
        xml_node camNode = hrCameraParamNode(camRef);

        camNode.attribute(L"type") = L"two_matrices";

        float4x4 projMatrixInv = projMatrix;
        float4x4 lookAtMatrix = view;

        std::wstringstream mProj, mWorldView;
        for (int i = 0; i < 16; i++)
        {
          mProj << projMatrixInv.L()[i] << L" ";
          mWorldView << lookAtMatrix.L()[i] << L" ";
        }

        const std::wstring str1 = mProj.str();
        const std::wstring str2 = mWorldView.str();

        camNode.force_child(L"mProj").text() = str1.c_str();
        camNode.force_child(L"mWorldView").text() = str2.c_str();
      }
      hrCameraClose(camRef);

      hrFlush(scnRef, renderRef);

      while (true)
      {
        std::this_thread::sleep_for(std::chrono::milliseconds(250));

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

      std::wstringstream ws;
      ws << std::fixed << L"tests_images/generate_nerf_data/" << std::setfill(L'0') << std::setw(4) << idx << L"_rgb.png";

      hrRenderSaveFrameBufferLDR(renderRef, ws.str().c_str());

      std::wstringstream ws2;
      ws2 << std::fixed << L"tests_images/generate_nerf_data/" << std::setfill(L'0') << std::setw(4) << idx << L"_alpha.png";

      idx++;

      hrRenderSaveGBufferLayerLDR(renderRef, ws2.str().c_str(), L"alpha");
    }

    return true;
  }


}