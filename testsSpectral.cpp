#include <cassert>
#include "tests.h"

#include "HydraXMLHelpers.h"
#include "LiteMath.h"
#include "mesh_utils.h"

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

  std::vector<HRMaterialRef> createSpectralDiffuseMaterialSequence(const std::vector<int> &wavelengths,
                                                                   const std::vector<float> &spd,
                                                                   const std::wstring& name = L"mat")
  {
    assert(wavelengths.size() == spd.size());
    std::vector<HRMaterialRef> result;
    result.reserve(spd.size());

    for(int i = 0 ; i < spd.size(); ++i)
    {
      std::wstringstream ws;
      ws << name << L"_" << wavelengths[i];
      HRMaterialRef mat = hrMaterialCreate(ws.str().c_str());
      hrMaterialOpen(mat, HR_WRITE_DISCARD);
      {
        auto matNode = hrMaterialParamNode(mat);

        auto diff = matNode.append_child(L"diffuse");
        diff.append_attribute(L"brdf_type").set_value(L"lambert");

        auto color = diff.append_child(L"color");
        auto val = color.append_attribute(L"val");
        HydraXMLHelpers::WriteFloat3(val, {spd[i], spd[i], spd[i]});
        VERIFY_XML(matNode);
      }
      hrMaterialClose(mat);

      result.push_back(mat);
    }

    return result;
  }

  std::vector<HRLightRef> createSpectralLightSequence(const HRLightRef &baseLight, const std::vector<int> &wavelengths,
                                                      const std::vector<float> &spd, const std::wstring& name = L"light")
  {
    assert(wavelengths.size() == spd.size());
    std::vector<HRLightRef> result;
    result.reserve(spd.size());

    hrLightOpen(baseLight, HR_OPEN_READ_ONLY);
    auto baselightNode = hrLightParamNode(baseLight);

    for(int i = 0 ; i < spd.size(); ++i)
    {
      std::wstringstream ws;
      ws << name << L"_" << wavelengths[i];
      HRLightRef light = hrLightCreate(ws.str().c_str());
      hrLightOpen(light, HR_WRITE_DISCARD);
      {
        auto lightNode = hrLightParamNode(light);

        HydraXMLHelpers::forceAttributes(baselightNode, lightNode, {L"id", L"name", L"mat_id"});
        HydraXMLHelpers::copyChildNodes(baselightNode, lightNode);

        auto colorNode = lightNode.force_child(L"intensity").force_child(L"color");
        auto val = colorNode.force_attribute(L"val");
        HydraXMLHelpers::WriteFloat3(val, {spd[i], spd[i], spd[i]});

        VERIFY_XML(lightNode);
      }
      hrLightClose(light);

      result.push_back(light);
    }

    hrLightClose(baseLight);

    return result;
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

    auto greenMaterials = createSpectralDiffuseMaterialSequence(wavelengths, green, L"green");
    auto redMaterials = createSpectralDiffuseMaterialSequence(wavelengths, red, L"red");

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

    auto lights = createSpectralLightSequence(rectLight, wavelengths, light, L"area_light");

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
    }

    return true;
  }
}