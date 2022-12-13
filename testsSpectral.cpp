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


  bool test_virtual_room()
  {
    hrErrorCallerPlace(L"test_virtual_room");

    hrSceneLibraryOpen(L"tests/test_virtual_room", HR_WRITE_DISCARD);


    // GEO
  //  std::vector<std::vector<float>> OBJECT_MATRICES = { {0.985574, 0.0121051, 0.168814, -0.0103193,
  //                             -0.0290729, 0.994722, 0.0984061, -0.0210492,
  //                             -0.166732, -0.101894, 0.980723, 0.0181473,
  //                              0, 0, 0, 1},
  //    {0.942972, 0.0104333, 0.332707, -0.0113168,
  //    -0.043626, 0.994761, 0.0924521, -0.0210557,
  //    -0.33, -0.101694, 0.938487, 0.0179416,
  //    0, 0, 0, 1},
  //    {0.870858, 0.00877298, 0.491457, -0.0123649,
  //-0.0573394, 0.994828, 0.0838464, -0.0210457,
  //-0.488179, -0.101198, 0.866856, 0.017908,
  //0, 0, 0, 1},
  //    {0.772781, 0.00723181, 0.634632, -0.013398,
  //-0.0692975, 0.994918, 0.0730451, -0.0210191,
  //-0.630878, -0.100426, 0.769355, 0.0180552,
  //0, 0, 0, 1},
  //    {0.64988, 0.00583309, 0.760014, -0.014401,
  //-0.0793103, 0.995032, 0.0601806, -0.0209763,
  //-0.755887, -0.0993871, 0.647114, 0.0183828,
  //0, 0, 0, 1},
  //    {0.508418, 0.00464927, 0.861098, -0.0153221,
  //-0.0868567, 0.995162, 0.0459097, -0.0209193,
  //-0.856719, -0.0981334, 0.506362, 0.0188746,
  //0, 0, 0, 1},
  //    {0.353404, 0.00371083, 0.935463, -0.0161351,
  //-0.0917764, 0.995305, 0.0307235, -0.0208505,
  //-0.930958, -0.0967112, 0.352086, 0.0195099,
  //0, 0, 0, 1},
  //    {0.189931, 0.00303752, 0.981793, -0.0168192,
  //-0.0940111, 0.995456, 0.015107, -0.0207721,
  //-0.977286, -0.0951687, 0.189353, 0.020265,
  //0, 0, 0, 1},
  //    {0.0162642, 0.0026292, 0.999864, -0.0173779,
  //-0.0935191, 0.995617, -0.00109681, -0.0206834,
  //-0.995485, -0.0934885, 0.0164387, 0.0211497,
  //0, 0, 0, 1},
  //    {-0.158702, 0.0025209, 0.987323, -0.0177749,
  //-0.0901939, 0.995778, -0.0170402, -0.0205885,
  //-0.983198, -0.0917548, -0.157804, 0.0221226,
  //0, 0, 0, 1},
  //    {
  //      -0.331823, 0.00272496, 0.943338, -0.0179973,
  //-0.0839981, 0.995938, -0.0324235, -0.0204891,
  //-0.939595, -0.0899974, -0.330246, 0.0231689,
  //0, 0, 0, 1},
  //    {-0.491099, 0.00322587, 0.871098, -0.0180305,
  //-0.075374, 0.996085, -0.0461823, -0.0203919,
  //-0.867837, -0.0883382, -0.488933, 0.0242158,
  //0, 0, 0, 1},
  //    {-0.630508, 0.0039767, 0.776173, -0.0178885,
  //-0.064909, 0.996214, -0.0578315, -0.0203013,
  //-0.773464, -0.0868438, -0.627862, 0.0252161,
  //0, 0, 0, 1},
  //    {-0.753537, 0.00498442, 0.657387, -0.0175743,
  //-0.0524515, 0.996327, -0.0676774, -0.020215,
  //-0.65531, -0.0854784, -0.750508, 0.0261916,
  //0, 0, 0, 1},
  //    {-0.855559, 0.0062263, 0.517668, -0.0170913,
  //-0.0383285, 0.996422, -0.0753306, -0.0201362,
  //-0.516285, -0.0842912, -0.852259, 0.0271099,
  //0, 0, 0, 1},
  //    {-0.93298, 0.00767922, 0.359847, -0.0164453,
  //-0.0228448, 0.996493, -0.0804955, -0.0200673,
  //-0.359203, -0.0833213, -0.929532, 0.0279441,
  //0, 0, 0, 1},
  //    {-0.982325, 0.00931792, 0.186952, -0.0156433,
  //-0.00632256, 0.996539, -0.08289, -0.0200105,
  //-0.187077, -0.0826069, -0.978866, 0.0286674,
  //0, 0, 0, 1},
  //    {-0.999865, 0.0110173, 0.0121753, -0.0147468,
  //0.00997826, 0.996555, -0.0823347, -0.0199702,
  //-0.0130405, -0.0822021, -0.99653, 0.0292249,
  //0, 0, 0, 1}
  //  }; //f
  //  std::vector<std::vector<float>> OBJECT_MATRICES = {
  //    {0.947293, 0.00597393, 0.320314, 0.00349988,
  //     -0.0381051, 0.99483, 0.0941375, 0.0527648,
  //     -0.318096, -0.101381, 0.942622, 0.0470857,
  //     0, 0, 0, 1},
  //    {0.662143, 0.00512078, 0.74936, 0.000881813,
  //-0.0774985, 0.995083, 0.0616786, 0.0528749,
  //-0.745359, -0.0989143, 0.659284, 0.0479258,
  //0, 0, 0, 1},
  //    {
  //      0.198676, 0.00562117, 0.980049, -0.000973706,
  //-0.0955314, 0.995333, 0.0136573, 0.0530958,
  //-0.975398, -0.0963388, 0.198285, 0.0499643,
  //0, 0, 0, 1
  //},
  //    { -0.322904, 0.00737017, 0.946403, -0.00155382,
  //-0.0869207, 0.995513, -0.0374092, 0.0533708,
  //-0.942432, -0.0943416, -0.320814, 0.0526813,
  //0, 0, 0, 1},
  //    {-0.750005, 0.0098436, 0.661358, -0.000704709,
  //-0.0544907, 0.995571, -0.0766125, 0.0536192,
  //-0.659183, -0.0934976, -0.746147, 0.0552775,
  //0, 0, 0, 1},
  //    {-0.979987, 0.0124258, 0.198672, 0.00134253,
  //-0.00655262, 0.995495, -0.0945845, 0.0537809,
  //-0.198952, -0.0939934, -0.975491, 0.0571214,
  //0, 0, 0, 1},
  //    {
  //      -0.947307, 0.0144221, -0.320003, 0.00405696,
  //0.0442377, 0.995304, -0.0860998, 0.0538106,
  //0.317259, -0.0957192, -0.943496, 0.0577023,
  //0, 0, 0, 1}
  //  }; // c

    std::vector<std::vector<float>> OBJECT_MATRICES = {
      {-0.364119, 0.00821498, -0.931316, -0.0131309,
  0.0915598, 0.995433, -0.0270168, 0.036397,
  0.926841, -0.0951084, -0.363208, 0.0399366,
  0, 0, 0, 1},
      {0.477172, 0.0102979, -0.878749, -0.00316629,
  0.076538, 0.995645, 0.0532289, 0.0367696,
  0.875471, -0.092657, 0.474306, 0.0450288,
  0, 0, 0, 1},
      {0.9803, 0.0135381, -0.197048, 0.0072022,
  0.00504776, 0.995605, 0.0935149, 0.0362262,
  0.197448, -0.0926673, 0.975924, 0.0406866,
  0, 0, 0, 1},
      {0.781705, 0.015601, 0.623453, 0.0104976,
  -0.0714694, 0.995342, 0.0647036, 0.035157,
  -0.61954, -0.095137, 0.779179, 0.0300219,
  0, 0, 0, 1},
      {0.01865, 0.015012, 0.999713, 0.00433427,
  -0.0985955, 0.995041, -0.0131025, 0.0343196,
  -0.994953, -0.0983229, 0.0200377, 0.0205869,
  0, 0, 0, 1},
      {-0.753569, 0.0121977, 0.657256, -0.00680714,
  -0.0565265, 0.994922, -0.083274, 0.0343243,
  -0.654934, -0.0999051, -0.749053, 0.0192553,
  0, 0, 0, 1},
      {-0.988931, 0.00918997, -0.148088, -0.0149559,
  0.0236759, 0.995065, -0.0963563, 0.0351515,
  0.146472, -0.0987958, -0.984269, 0.026813,
  0, 0, 0, 1}
    }; //d
    std::wstring OBJECT_PATH(L"my_mesh.obj");

    float ANGLE_STEP = 50; // 30 // 10

    // MAT
    std::wstring DIFFUSE_TEX(L"diffuse.png");
    std::wstring GLOSSY_TEX(L"glossiness.png");

    // LIGHT
    float BASE_INTENSITY = 400.0f;
    float IOR = 1.504f; // 

    //CAMERA
    //float FOV = 18.77439302162933; // c
    float FOV = 18.536344118173652; //d
    //float FOV = 21; // f

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Materials
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRTextureNodeRef diffuseTex = hrTexture2DCreateFromFile(DIFFUSE_TEX.c_str());
    HRTextureNodeRef glossTex   = hrTexture2DCreateFromFile(GLOSSY_TEX.c_str());

    HRMaterialRef matDiffSpecRough = hrMaterialCreate(L"matDiffSpecRough");
    hrMaterialOpen(matDiffSpecRough, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matDiffSpecRough);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      hrTextureBind(diffuseTex, color);

      auto texNode = color.child(L"texture");
      texNode.append_attribute(L"matrix");
      float samplerMatrix[16] = { 1, 0, 0, 0,
                                  0, 1, 0, 0,
                                  0, 0, 1, 0,
                                  0, 0, 0, 1 };

      texNode.append_attribute(L"addressing_mode_u").set_value(L"clamp");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"clamp");
      texNode.append_attribute(L"input_gamma").set_value(1.0f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

      auto refl = matNode.append_child(L"reflectivity");
      refl.append_attribute(L"brdf_type").set_value(L"ggx");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      auto gloss = refl.append_child(L"glossiness");
      gloss.append_attribute(L"val").set_value(L"1.0");
      hrTextureBind(glossTex, gloss);
      texNode = gloss.child(L"texture");

      texNode.append_attribute(L"matrix");
      texNode.append_attribute(L"addressing_mode_u").set_value(L"clamp");
      texNode.append_attribute(L"addressing_mode_v").set_value(L"clamp");
      texNode.append_attribute(L"input_gamma").set_value(1.0f);
      texNode.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);

      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(IOR);

      VERIFY_XML(matNode);
    }
    hrMaterialClose(matDiffSpecRough);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Meshes
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRMeshRef objRef = hrMeshCreateFromFile(OBJECT_PATH.c_str());
    hrMeshOpen(objRef, HR_TRIANGLE_IND3, HR_OPEN_EXISTING);
    {
      hrMeshMaterialId(objRef, matDiffSpecRough.id);
    }
    hrMeshClose(objRef);


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Light
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRLightRef rectLight = hrLightCreate(L"softbox");
    hrLightOpen(rectLight, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(rectLight);

      lightNode.attribute(L"type").set_value(L"area");
      lightNode.attribute(L"shape").set_value(L"rect");
      lightNode.attribute(L"distribution").set_value(L"diffuse");

      auto sizeNode = lightNode.append_child(L"size");

      sizeNode.append_attribute(L"half_length").set_value(L"0.45");
      sizeNode.append_attribute(L"half_width").set_value(L"0.3");

      auto intensityNode = lightNode.append_child(L"intensity");
      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");

      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(5.5f);
      VERIFY_XML(lightNode);
    }
    hrLightClose(rectLight);

    HRLightRef sphereLight_right = hrLightCreate(L"led_right");
    hrLightOpen(sphereLight_right, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(sphereLight_right);

      lightNode.attribute(L"type").set_value(L"sphere");
      lightNode.attribute(L"shape").set_value(L"sphere");
      lightNode.attribute(L"distribution").set_value(L"diffuse");

      lightNode.append_child(L"size").append_attribute(L"radius").set_value(0.04f);

      auto intensityNode = lightNode.append_child(L"intensity");
      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");

      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(BASE_INTENSITY / 2.4f);
      VERIFY_XML(lightNode);
    }
    hrLightClose(sphereLight_right);

    HRLightRef sphereLight_left = hrLightCreate(L"led_left");
    hrLightOpen(sphereLight_left, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(sphereLight_left);

      lightNode.attribute(L"type").set_value(L"sphere");
      lightNode.attribute(L"shape").set_value(L"sphere");
      lightNode.attribute(L"distribution").set_value(L"diffuse");

      lightNode.append_child(L"size").append_attribute(L"radius").set_value(0.04f);

      auto intensityNode = lightNode.append_child(L"intensity");
      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");

      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(BASE_INTENSITY);
      VERIFY_XML(lightNode);
    }
    hrLightClose(sphereLight_left);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Camera
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRCameraRef camRef = hrCameraCreate(L"my camera");

    hrCameraOpen(camRef, HR_WRITE_DISCARD);
    {
      auto camNode = hrCameraParamNode(camRef);

      camNode.append_child(L"fov").text().set(FOV);
      camNode.append_child(L"nearClipPlane").text().set(L"0.01");
      camNode.append_child(L"farClipPlane").text().set(L"1000.0");

      camNode.append_child(L"up").text().set(L"0.0 1.0 0.0");
      camNode.append_child(L"position").text().set(L"0.0 0.5 0.5");
      camNode.append_child(L"look_at").text().set(L"0.0 0.0 0.0");
    }
    hrCameraClose(camRef);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Render settings
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRRenderRef renderRef = TEST_UTILS::CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 1600, 1067, 256, 512);
    hrRenderOpen(renderRef, HR_OPEN_EXISTING);
    {
      auto node = hrRenderParamNode(renderRef);
      node.append_child(L"framebuffer_channels").text() = 1;
      /*node.force_child(L"trace_depth").text() = 2;
      node.force_child(L"diff_trace_depth").text() = 2;*/
    }
    hrRenderClose(renderRef);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Create scene
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    const float DEG_TO_RAD = 0.01745329251f; // float(3.14159265358979323846f) / 180.0f;
    hlm::float4x4 objMatrix, lightLeftMatrix, lightRightMatrix, softboxMatrix;
    hlm::float4x4 identity{};


    auto mT = hlm::translate4x4({ -0.46, 0.51, 0.55 });
    lightLeftMatrix = mT;

    mT = hlm::translate4x4({ 0.46, 0.51, 0.55 });
    lightRightMatrix= mT;

    auto mRotX = hlm::rotate4x4X(90 * DEG_TO_RAD);
    auto mRotY = hlm::rotate4x4Y(-79.19 * DEG_TO_RAD);
    mT = hlm::translate4x4({ -1.1, 0.28, 0.21 });
    softboxMatrix = mT * mRotY * mRotX; //

 //   std::vector<std::filesystem::path> savedImages;
    HRSceneInstRef scnRef = hrSceneCreate(L"scene");
    int idx = 0;
    for (auto mat : OBJECT_MATRICES)
    {
      objMatrix = hlm::float4x4(mat.data());
      
      hrSceneOpen(scnRef, HR_WRITE_DISCARD);


      hrMeshInstance(scnRef, objRef, objMatrix.L());
      hrLightInstance(scnRef, sphereLight_left, lightLeftMatrix.L());
      hrLightInstance(scnRef, sphereLight_right, lightRightMatrix.L());
      // hrLightInstance(scnRef, rectLight, softboxMatrix.L());

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
      imgName << std::setfill(L'0') << std::setw(4) << idx * ANGLE_STEP;
      std::wstring basePath = L"tests_images/test_virtual_room/";
      const std::wstring ldrName = basePath + imgName.str() + std::wstring(L".png");
      const std::wstring hdrName = basePath + imgName.str() + std::wstring(L".exr");

      hrRenderSaveFrameBufferLDR(renderRef, ldrName.c_str());
      hrRenderSaveFrameBufferHDR(renderRef, hdrName.c_str());
      //savedImages.push_back(hdrName);
      idx++;
    }

    return check_images_HDR("test_virtual_room", 1, 25);
  }

}

