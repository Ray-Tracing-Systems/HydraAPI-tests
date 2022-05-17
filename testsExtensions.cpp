#include "tests.h"

#include "HRExtensions_VectorTex.h"
#include "HydraXMLHelpers.h"
#include "LiteMath.h"

namespace EXTENSIONS_TESTS
{
  namespace hlm = LiteMath;


  // basic MSDF 
  bool test_ext_vtex_1()
  {
    hrErrorCallerPlace(L"test_ext_vtex_1");

    hrSceneLibraryOpen(L"tests_ext/test_ext_vtex_1", HR_WRITE_DISCARD);

    SimpleMesh plane = CreatePlane(4.0f);

    
    hr_vtex::VectorTexCreateInfo vtex_info{};
    vtex_info.mode        = hr_vtex::VTEX_MODE::VTEX_MSDF;
    vtex_info.dpi         = 96;
    vtex_info.sdfCombine  = false;
    vtex_info.sdfAngThres = 3.14;
    vtex_info.drawOutline = false;
    vtex_info.bgColor[0]  = 0.5f; vtex_info.bgColor[1] = 0.5f; vtex_info.bgColor[2] = 0.5f; vtex_info.bgColor[3] = 1.0f;
    
    pugi::xml_node vtex_node;
    HRTextureNodeRef texSDF = hr_vtex::hrTextureVector2DCreateFromFile(L"data/textures/non-intersecting.svg", &vtex_info, &vtex_node);

    HRMaterialRef mat0 = hrMaterialCreate(L"mysimplemat");
    hrMaterialOpen(mat0, HR_WRITE_DISCARD);
    {
      xml_node matNode = hrMaterialParamNode(mat0);
      xml_node diff = matNode.append_child(L"diffuse");

      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      auto colorNode = diff.append_child(L"color");
      colorNode.append_attribute(L"tex_apply_mode") = L"replace";

      HydraXMLHelpers::WriteFloat4(colorNode.append_attribute(L"val"), vtex_info.bgColor);
      
      auto texNode = hrTextureBind(texSDF, colorNode);

      // add procedural texture arguments
      HydraXMLHelpers::moveChildNodes(vtex_node, texNode);
    }
    hrMaterialClose(mat0);


    HRMeshRef planeRef = hrMeshCreate(L"my_plane");
    hrMeshOpen(planeRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
    {
      hrMeshVertexAttribPointer4f(planeRef, L"pos", &plane.vPos[0]);
      hrMeshVertexAttribPointer4f(planeRef, L"norm", &plane.vNorm[0]);
      hrMeshVertexAttribPointer2f(planeRef, L"texcoord", &plane.vTexCoord[0]);
      hrMeshMaterialId(planeRef, 0);
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

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"0.75 0.75 1");
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

      camNode.append_child(L"fov").text().set(L"45");
      camNode.append_child(L"nearClipPlane").text().set(L"0.01");
      camNode.append_child(L"farClipPlane").text().set(L"100.0");

      camNode.append_child(L"up").text().set(L"0 1 0");
      //camNode.append_child(L"position").text().set(L"0 0 3");
      camNode.append_child(L"position").text().set(L"0 0 10");
      camNode.append_child(L"look_at").text().set(L"0 0 0");
    }
    hrCameraClose(camRef);


    HRRenderRef renderRef = hrRenderCreate(L"HydraModern"); 
    hrRenderEnableDevice(renderRef, CURR_RENDER_DEVICE, true);

    hrRenderOpen(renderRef, HR_WRITE_DISCARD);
    {
      pugi::xml_node node = hrRenderParamNode(renderRef);

      node.append_child(L"width").text() = L"1024";
      node.append_child(L"height").text() = L"1024";

      node.append_child(L"method_primary").text() = L"pathtracing";
      node.append_child(L"method_secondary").text() = L"pathtracing";
      node.append_child(L"method_tertiary").text() = L"pathtracing";
      node.append_child(L"method_caustic").text() = L"pathtracing";
      node.append_child(L"shadows").text() = L"1";

      node.append_child(L"trace_depth").text() = L"4";
      node.append_child(L"diff_trace_depth").text() = L"2";
      node.append_child(L"pt_error").text() = L"2.0";
      node.append_child(L"minRaysPerPixel").text() = L"512";
      node.append_child(L"maxRaysPerPixel").text() = L"512";
    }
    hrRenderClose(renderRef);

    
    HRSceneInstRef scnRef = hrSceneCreate(L"my scene");
    const float DEG_TO_RAD = float(3.14159265358979323846f) / 180.0f;
    hrSceneOpen(scnRef, HR_WRITE_DISCARD);
    {

      auto mtranslate = hlm::translate4x4(hlm::float3(0, -4.0f, 1));
      auto mscale = hlm::scale4x4(hlm::float3(0.75f, 0.75f, 0.75f));
      auto mrot = hlm::rotate4x4X(-DEG_TO_RAD * 90.0f);
      //auto mres = mtranslate * mscale;
      float4x4 identity;
      hrMeshInstance(scnRef, planeRef, mrot.L());

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

    hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_ext_vtex_1/z_out.png");

    return check_images("test_ext_vtex_1", 1, 10);
  }

  // texture matrix
  bool test_ext_vtex_2()
  {
    hrErrorCallerPlace(L"test_ext_vtex_2");

    hrSceneLibraryOpen(L"tests_ext/test_ext_vtex_2", HR_WRITE_DISCARD);

    SimpleMesh plane = CreatePlane(4.0f);


    hr_vtex::VectorTexCreateInfo vtex_info{};
    vtex_info.mode = hr_vtex::VTEX_MODE::VTEX_MSDF;
    vtex_info.dpi = 96;
    vtex_info.sdfCombine = false;
    vtex_info.sdfAngThres = 3.14;
    vtex_info.drawOutline = false;
    vtex_info.bgColor[0] = 0.5f; vtex_info.bgColor[1] = 0.5f; vtex_info.bgColor[2] = 0.5f; vtex_info.bgColor[3] = 1.0f;
    vtex_info.textureMatrix[0] = 4.0f; vtex_info.textureMatrix[1] = 0.0f; vtex_info.textureMatrix[2] = 0.5f;
    vtex_info.textureMatrix[3] = 0.0f; vtex_info.textureMatrix[4] = 8.0f; vtex_info.textureMatrix[5] = -0.5f;
    vtex_info.textureFlags     = hr_vtex::TEX_WRAP;//hr_vtex::TEX_CLAMP_U | hr_vtex::TEX_CLAMP_V;
    pugi::xml_node vtex_node;
    HRTextureNodeRef texSDF = hr_vtex::hrTextureVector2DCreateFromFile(L"data/textures/figure-text.svg", &vtex_info, &vtex_node);

    HRMaterialRef mat0 = hrMaterialCreate(L"mysimplemat");
    hrMaterialOpen(mat0, HR_WRITE_DISCARD);
    {
      xml_node matNode = hrMaterialParamNode(mat0);
      xml_node diff = matNode.append_child(L"diffuse");

      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      auto colorNode = diff.append_child(L"color");
      colorNode.append_attribute(L"tex_apply_mode") = L"replace";

      HydraXMLHelpers::WriteFloat4(colorNode.append_attribute(L"val"), vtex_info.bgColor);

      auto texNode = hrTextureBind(texSDF, colorNode);

      // add procedural texture arguments
      HydraXMLHelpers::moveChildNodes(vtex_node, texNode);
    }
    hrMaterialClose(mat0);


    HRMeshRef planeRef = hrMeshCreate(L"my_plane");
    hrMeshOpen(planeRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
    {
      hrMeshVertexAttribPointer4f(planeRef, L"pos", &plane.vPos[0]);
      hrMeshVertexAttribPointer4f(planeRef, L"norm", &plane.vNorm[0]);
      hrMeshVertexAttribPointer2f(planeRef, L"texcoord", &plane.vTexCoord[0]);
      hrMeshMaterialId(planeRef, 0);
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

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"0.75 0.75 1");
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

      camNode.append_child(L"fov").text().set(L"45");
      camNode.append_child(L"nearClipPlane").text().set(L"0.01");
      camNode.append_child(L"farClipPlane").text().set(L"100.0");

      camNode.append_child(L"up").text().set(L"0 1 0");
      camNode.append_child(L"position").text().set(L"0 0 10");
      camNode.append_child(L"look_at").text().set(L"0 0 0");
    }
    hrCameraClose(camRef);


    HRRenderRef renderRef = hrRenderCreate(L"HydraModern");
    hrRenderEnableDevice(renderRef, CURR_RENDER_DEVICE, true);

    hrRenderOpen(renderRef, HR_WRITE_DISCARD);
    {
      pugi::xml_node node = hrRenderParamNode(renderRef);

      node.append_child(L"width").text() = L"1024";
      node.append_child(L"height").text() = L"1024";

      node.append_child(L"method_primary").text() = L"pathtracing";
      node.append_child(L"method_secondary").text() = L"pathtracing";
      node.append_child(L"method_tertiary").text() = L"pathtracing";
      node.append_child(L"method_caustic").text() = L"pathtracing";
      node.append_child(L"shadows").text() = L"1";

      node.append_child(L"trace_depth").text() = L"4";
      node.append_child(L"diff_trace_depth").text() = L"2";
      node.append_child(L"pt_error").text() = L"2.0";
      node.append_child(L"minRaysPerPixel").text() = L"512";
      node.append_child(L"maxRaysPerPixel").text() = L"512";
    }
    hrRenderClose(renderRef);


    HRSceneInstRef scnRef = hrSceneCreate(L"my scene");
    const float DEG_TO_RAD = float(3.14159265358979323846f) / 180.0f;
    hrSceneOpen(scnRef, HR_WRITE_DISCARD);
    {

      auto mtranslate = hlm::translate4x4(hlm::float3(0, -4.0f, 1));
      auto mscale = hlm::scale4x4(hlm::float3(0.75f, 0.75f, 0.75f));
      auto mrot = hlm::rotate4x4X(-DEG_TO_RAD * 90.0f);
      //auto mres = mtranslate * mscale;
      float4x4 identity;
      hrMeshInstance(scnRef, planeRef, mrot.L());

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

    hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_ext_vtex_2/z_out.png");

    return check_images("test_ext_vtex_2", 1, 10);
  }

  // draw outlines
  bool test_ext_vtex_3()
  {
    hrErrorCallerPlace(L"test_ext_vtex_3");

    hrSceneLibraryOpen(L"tests_ext/test_ext_vtex_3", HR_WRITE_DISCARD);

    SimpleMesh plane = CreatePlane(4.0f);


    hr_vtex::VectorTexCreateInfo vtex_info{};
    vtex_info.mode = hr_vtex::VTEX_MODE::VTEX_MSDF;
    vtex_info.dpi  = 256;
    vtex_info.sdfCombine    = false;
    vtex_info.sdfAngThres   = 3.14;
    vtex_info.drawOutline   = true;
    vtex_info.distThreshold = 1.0f / 128.0f;
    vtex_info.smoothFac     = 1.0f / 256.0f;
    vtex_info.bgColor[0]    = 0.5f; vtex_info.bgColor[1] = 0.5f; vtex_info.bgColor[2] = 0.5f; vtex_info.bgColor[3] = 1.0f;
    vtex_info.textureFlags  = hr_vtex::TEX_CLAMP_U | hr_vtex::TEX_CLAMP_V;
    pugi::xml_node vtex_node;
    HRTextureNodeRef texSDF = hr_vtex::hrTextureVector2DCreateFromFile(L"data/textures/stars.svg", &vtex_info, &vtex_node);

    HRMaterialRef mat0 = hrMaterialCreate(L"mysimplemat");
    hrMaterialOpen(mat0, HR_WRITE_DISCARD);
    {
      xml_node matNode = hrMaterialParamNode(mat0);
      xml_node diff = matNode.append_child(L"diffuse");

      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      auto colorNode = diff.append_child(L"color");
      colorNode.append_attribute(L"tex_apply_mode") = L"replace";

      HydraXMLHelpers::WriteFloat4(colorNode.append_attribute(L"val"), vtex_info.bgColor);

      auto texNode = hrTextureBind(texSDF, colorNode);

      // add procedural texture arguments
      HydraXMLHelpers::moveChildNodes(vtex_node, texNode);
    }
    hrMaterialClose(mat0);


    HRMeshRef planeRef = hrMeshCreate(L"my_plane");
    hrMeshOpen(planeRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
    {
      hrMeshVertexAttribPointer4f(planeRef, L"pos", &plane.vPos[0]);
      hrMeshVertexAttribPointer4f(planeRef, L"norm", &plane.vNorm[0]);
      hrMeshVertexAttribPointer2f(planeRef, L"texcoord", &plane.vTexCoord[0]);
      hrMeshMaterialId(planeRef, 0);
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

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"0.75 0.75 1");
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

      camNode.append_child(L"fov").text().set(L"45");
      camNode.append_child(L"nearClipPlane").text().set(L"0.01");
      camNode.append_child(L"farClipPlane").text().set(L"100.0");

      camNode.append_child(L"up").text().set(L"0 1 0");
      camNode.append_child(L"position").text().set(L"0 0 10");
      camNode.append_child(L"look_at").text().set(L"0 0 0");
    }
    hrCameraClose(camRef);


    HRRenderRef renderRef = hrRenderCreate(L"HydraModern");
    hrRenderEnableDevice(renderRef, CURR_RENDER_DEVICE, true);

    hrRenderOpen(renderRef, HR_WRITE_DISCARD);
    {
      pugi::xml_node node = hrRenderParamNode(renderRef);

      node.append_child(L"width").text() = L"1024";
      node.append_child(L"height").text() = L"1024";

      node.append_child(L"method_primary").text() = L"pathtracing";
      node.append_child(L"method_secondary").text() = L"pathtracing";
      node.append_child(L"method_tertiary").text() = L"pathtracing";
      node.append_child(L"method_caustic").text() = L"pathtracing";
      node.append_child(L"shadows").text() = L"1";

      node.append_child(L"trace_depth").text() = L"4";
      node.append_child(L"diff_trace_depth").text() = L"2";
      node.append_child(L"pt_error").text() = L"2.0";
      node.append_child(L"minRaysPerPixel").text() = L"512";
      node.append_child(L"maxRaysPerPixel").text() = L"512";
    }
    hrRenderClose(renderRef);


    HRSceneInstRef scnRef = hrSceneCreate(L"my scene");
    const float DEG_TO_RAD = float(3.14159265358979323846f) / 180.0f;
    hrSceneOpen(scnRef, HR_WRITE_DISCARD);
    {

      auto mtranslate = hlm::translate4x4(hlm::float3(0, -4.0f, 1));
      auto mscale = hlm::scale4x4(hlm::float3(0.75f, 0.75f, 0.75f));
      auto mrot = hlm::rotate4x4X(-DEG_TO_RAD * 90.0f);
      //auto mres = mtranslate * mscale;
      float4x4 identity;
      hrMeshInstance(scnRef, planeRef, mrot.L());

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

    hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_ext_vtex_3/z_out.png");

    return check_images("test_ext_vtex_3", 1, 10);
  }

  // combine
  bool test_ext_vtex_4()
  {
    hrErrorCallerPlace(L"test_ext_vtex_4");

    hrSceneLibraryOpen(L"tests_ext/test_ext_vtex_4", HR_WRITE_DISCARD);

    SimpleMesh plane = CreatePlane(4.0f);

    hr_vtex::VectorTexCreateInfo vtex_info{};
    vtex_info.mode = hr_vtex::VTEX_MODE::VTEX_MSDF;
    vtex_info.dpi = 96;
    vtex_info.sdfCombine  = true;
    vtex_info.sdfAngThres = 3.14;
    vtex_info.drawOutline = false;
    vtex_info.bgColor[0] = 0.5f; vtex_info.bgColor[1] = 0.5f; vtex_info.bgColor[2] = 0.5f; vtex_info.bgColor[3] = 1.0f;
    vtex_info.overrideShapeColor[0] = 1.0f;
    vtex_info.overrideShapeColor[1] = 1.0f;
    vtex_info.overrideShapeColor[2] = 0.0f;
    vtex_info.overrideShapeColor[3] = 1.0f;
    vtex_info.textureFlags = hr_vtex::TEX_CLAMP_U | hr_vtex::TEX_CLAMP_V;
    pugi::xml_node vtex_node;
    HRTextureNodeRef texSDF = hr_vtex::hrTextureVector2DCreateFromFile(L"data/textures/sample-text.svg", &vtex_info, &vtex_node);

    HRMaterialRef mat0 = hrMaterialCreate(L"mysimplemat");
    hrMaterialOpen(mat0, HR_WRITE_DISCARD);
    {
      xml_node matNode = hrMaterialParamNode(mat0);
      xml_node diff = matNode.append_child(L"diffuse");

      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      auto colorNode = diff.append_child(L"color");
      colorNode.append_attribute(L"tex_apply_mode") = L"replace";

      HydraXMLHelpers::WriteFloat4(colorNode.append_attribute(L"val"), vtex_info.bgColor);

      auto texNode = hrTextureBind(texSDF, colorNode);

      // add procedural texture arguments
      HydraXMLHelpers::moveChildNodes(vtex_node, texNode);
    }
    hrMaterialClose(mat0);


    HRMeshRef planeRef = hrMeshCreate(L"my_plane");
    hrMeshOpen(planeRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
    {
      hrMeshVertexAttribPointer4f(planeRef, L"pos", &plane.vPos[0]);
      hrMeshVertexAttribPointer4f(planeRef, L"norm", &plane.vNorm[0]);
      hrMeshVertexAttribPointer2f(planeRef, L"texcoord", &plane.vTexCoord[0]);
      hrMeshMaterialId(planeRef, 0);
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

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"0.75 0.75 1");
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

      camNode.append_child(L"fov").text().set(L"45");
      camNode.append_child(L"nearClipPlane").text().set(L"0.01");
      camNode.append_child(L"farClipPlane").text().set(L"100.0");

      camNode.append_child(L"up").text().set(L"0 1 0");
      camNode.append_child(L"position").text().set(L"0 0 10");
      camNode.append_child(L"look_at").text().set(L"0 0 0");
    }
    hrCameraClose(camRef);


    HRRenderRef renderRef = hrRenderCreate(L"HydraModern");
    hrRenderEnableDevice(renderRef, CURR_RENDER_DEVICE, true);

    hrRenderOpen(renderRef, HR_WRITE_DISCARD);
    {
      pugi::xml_node node = hrRenderParamNode(renderRef);

      node.append_child(L"width").text() = L"1024";
      node.append_child(L"height").text() = L"1024";

      node.append_child(L"method_primary").text() = L"pathtracing";
      node.append_child(L"method_secondary").text() = L"pathtracing";
      node.append_child(L"method_tertiary").text() = L"pathtracing";
      node.append_child(L"method_caustic").text() = L"pathtracing";
      node.append_child(L"shadows").text() = L"1";

      node.append_child(L"trace_depth").text() = L"4";
      node.append_child(L"diff_trace_depth").text() = L"2";
      node.append_child(L"pt_error").text() = L"2.0";
      node.append_child(L"minRaysPerPixel").text() = L"512";
      node.append_child(L"maxRaysPerPixel").text() = L"512";
    }
    hrRenderClose(renderRef);


    HRSceneInstRef scnRef = hrSceneCreate(L"my scene");
    const float DEG_TO_RAD = float(3.14159265358979323846f) / 180.0f;
    hrSceneOpen(scnRef, HR_WRITE_DISCARD);
    {

      auto mtranslate = hlm::translate4x4(hlm::float3(0, -4.0f, 1));
      auto mscale = hlm::scale4x4(hlm::float3(0.75f, 0.75f, 0.75f));
      auto mrot = hlm::rotate4x4X(-DEG_TO_RAD * 90.0f);
      //auto mres = mtranslate * mscale;
      float4x4 identity;
      hrMeshInstance(scnRef, planeRef, mrot.L());

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

    hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_ext_vtex_4/z_out.png");

    return check_images("test_ext_vtex_4", 1, 10);
  }

  // rasterize
  bool test_ext_vtex_5()
  {
    hrErrorCallerPlace(L"test_ext_vtex_5");

    hrSceneLibraryOpen(L"tests_ext/test_ext_vtex_5", HR_WRITE_DISCARD);

    SimpleMesh plane = CreatePlane(4.0f);

    hr_vtex::VectorTexCreateInfo vtex_info{};
    vtex_info.mode = hr_vtex::VTEX_MODE::VTEX_RASTERIZE;
    vtex_info.dpi = 200;
    vtex_info.bgColor[0] = 0.5f; vtex_info.bgColor[1] = 0.5f; vtex_info.bgColor[2] = 0.5f; vtex_info.bgColor[3] = 1.0f;
    vtex_info.textureMatrix[0] = 4.0f * 0.866f; vtex_info.textureMatrix[1] = -0.5f;         vtex_info.textureMatrix[2] = 0.0f;
    vtex_info.textureMatrix[3] = 0.5f;          vtex_info.textureMatrix[4] = 4.0f * 0.866f; vtex_info.textureMatrix[5] = 0.0f;
    vtex_info.textureFlags = hr_vtex::TEX_WRAP;//hr_vtex::TEX_CLAMP_U | hr_vtex::TEX_CLAMP_V;
    pugi::xml_node vtex_node;
    HRTextureNodeRef texSDF = hr_vtex::hrTextureVector2DCreateFromFile(L"data/textures/tiger.svg", &vtex_info, &vtex_node);

    HRMaterialRef mat0 = hrMaterialCreate(L"mysimplemat");
    hrMaterialOpen(mat0, HR_WRITE_DISCARD);
    {
      xml_node matNode = hrMaterialParamNode(mat0);
      xml_node diff = matNode.append_child(L"diffuse");

      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      auto colorNode = diff.append_child(L"color");
      colorNode.append_attribute(L"tex_apply_mode") = L"replace";

      HydraXMLHelpers::WriteFloat4(colorNode.append_attribute(L"val"), vtex_info.bgColor);

      auto texNode = hrTextureBind(texSDF, colorNode);

      // add procedural texture arguments
      HydraXMLHelpers::moveChildNodes(vtex_node, texNode);
    }
    hrMaterialClose(mat0);


    HRMeshRef planeRef = hrMeshCreate(L"my_plane");
    hrMeshOpen(planeRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
    {
      hrMeshVertexAttribPointer4f(planeRef, L"pos", &plane.vPos[0]);
      hrMeshVertexAttribPointer4f(planeRef, L"norm", &plane.vNorm[0]);
      hrMeshVertexAttribPointer2f(planeRef, L"texcoord", &plane.vTexCoord[0]);
      hrMeshMaterialId(planeRef, 0);
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

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"0.75 0.75 1");
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

      camNode.append_child(L"fov").text().set(L"45");
      camNode.append_child(L"nearClipPlane").text().set(L"0.01");
      camNode.append_child(L"farClipPlane").text().set(L"100.0");

      camNode.append_child(L"up").text().set(L"0 1 0");
      camNode.append_child(L"position").text().set(L"0 0 10");
      camNode.append_child(L"look_at").text().set(L"0 0 0");
    }
    hrCameraClose(camRef);


    HRRenderRef renderRef = hrRenderCreate(L"HydraModern");
    hrRenderEnableDevice(renderRef, CURR_RENDER_DEVICE, true);

    hrRenderOpen(renderRef, HR_WRITE_DISCARD);
    {
      pugi::xml_node node = hrRenderParamNode(renderRef);

      node.append_child(L"width").text() = L"1024";
      node.append_child(L"height").text() = L"1024";

      node.append_child(L"method_primary").text() = L"pathtracing";
      node.append_child(L"method_secondary").text() = L"pathtracing";
      node.append_child(L"method_tertiary").text() = L"pathtracing";
      node.append_child(L"method_caustic").text() = L"pathtracing";
      node.append_child(L"shadows").text() = L"1";

      node.append_child(L"trace_depth").text() = L"4";
      node.append_child(L"diff_trace_depth").text() = L"2";
      node.append_child(L"pt_error").text() = L"2.0";
      node.append_child(L"minRaysPerPixel").text() = L"512";
      node.append_child(L"maxRaysPerPixel").text() = L"512";
    }
    hrRenderClose(renderRef);


    HRSceneInstRef scnRef = hrSceneCreate(L"my scene");
    const float DEG_TO_RAD = float(3.14159265358979323846f) / 180.0f;
    hrSceneOpen(scnRef, HR_WRITE_DISCARD);
    {

      auto mtranslate = hlm::translate4x4(hlm::float3(0, -4.0f, 1));
      auto mscale = hlm::scale4x4(hlm::float3(0.75f, 0.75f, 0.75f));
      auto mrot = hlm::rotate4x4X(-DEG_TO_RAD * 90.0f);
      //auto mres = mtranslate * mscale;
      float4x4 identity;
      hrMeshInstance(scnRef, planeRef, mrot.L());

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

    hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_ext_vtex_5/z_out.png");

    return check_images("test_ext_vtex_5", 1, 10);
  }

  // overlapping shapes
  bool test_ext_vtex_6()
  {
    hrErrorCallerPlace(L"test_ext_vtex_6");

    hrSceneLibraryOpen(L"tests_ext/test_ext_vtex_6", HR_WRITE_DISCARD);

    SimpleMesh plane = CreatePlane(4.0f);

    hr_vtex::VectorTexCreateInfo vtex_info{};
    vtex_info.mode = hr_vtex::VTEX_MODE::VTEX_MSDF;
    vtex_info.dpi  = 96;
    vtex_info.sdfCombine  = false;
    vtex_info.sdfAngThres = 3.14;
    vtex_info.drawOutline = false;
    vtex_info.bgColor[0] = 0.5f; vtex_info.bgColor[1] = 0.5f; vtex_info.bgColor[2] = 0.5f; vtex_info.bgColor[3] = 1.0f;

    pugi::xml_node vtex_node;
    HRTextureNodeRef texSDF = hr_vtex::hrTextureVector2DCreateFromFile(L"data/textures/figures4.svg", &vtex_info, &vtex_node);

    HRMaterialRef mat0 = hrMaterialCreate(L"mysimplemat");
    hrMaterialOpen(mat0, HR_WRITE_DISCARD);
    {
      xml_node matNode = hrMaterialParamNode(mat0);
      xml_node diff = matNode.append_child(L"diffuse");

      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      auto colorNode = diff.append_child(L"color");
      colorNode.append_attribute(L"tex_apply_mode") = L"replace";

      HydraXMLHelpers::WriteFloat4(colorNode.append_attribute(L"val"), vtex_info.bgColor);

      auto texNode = hrTextureBind(texSDF, colorNode);

      // add procedural texture arguments
      HydraXMLHelpers::moveChildNodes(vtex_node, texNode);
    }
    hrMaterialClose(mat0);


    HRMeshRef planeRef = hrMeshCreate(L"my_plane");
    hrMeshOpen(planeRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
    {
      hrMeshVertexAttribPointer4f(planeRef, L"pos", &plane.vPos[0]);
      hrMeshVertexAttribPointer4f(planeRef, L"norm", &plane.vNorm[0]);
      hrMeshVertexAttribPointer2f(planeRef, L"texcoord", &plane.vTexCoord[0]);
      hrMeshMaterialId(planeRef, 0);
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

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"0.75 0.75 1");
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

      camNode.append_child(L"fov").text().set(L"45");
      camNode.append_child(L"nearClipPlane").text().set(L"0.01");
      camNode.append_child(L"farClipPlane").text().set(L"100.0");

      camNode.append_child(L"up").text().set(L"0 1 0");
      //camNode.append_child(L"position").text().set(L"0 0 3");
      camNode.append_child(L"position").text().set(L"0 0 10");
      camNode.append_child(L"look_at").text().set(L"0 0 0");
    }
    hrCameraClose(camRef);


    HRRenderRef renderRef = hrRenderCreate(L"HydraModern");
    hrRenderEnableDevice(renderRef, CURR_RENDER_DEVICE, true);

    hrRenderOpen(renderRef, HR_WRITE_DISCARD);
    {
      pugi::xml_node node = hrRenderParamNode(renderRef);

      node.append_child(L"width").text() = L"1024";
      node.append_child(L"height").text() = L"1024";

      node.append_child(L"method_primary").text() = L"pathtracing";
      node.append_child(L"method_secondary").text() = L"pathtracing";
      node.append_child(L"method_tertiary").text() = L"pathtracing";
      node.append_child(L"method_caustic").text() = L"pathtracing";
      node.append_child(L"shadows").text() = L"1";

      node.append_child(L"trace_depth").text() = L"4";
      node.append_child(L"diff_trace_depth").text() = L"2";
      node.append_child(L"pt_error").text() = L"2.0";
      node.append_child(L"minRaysPerPixel").text() = L"512";
      node.append_child(L"maxRaysPerPixel").text() = L"512";
    }
    hrRenderClose(renderRef);


    HRSceneInstRef scnRef = hrSceneCreate(L"my scene");
    const float DEG_TO_RAD = float(3.14159265358979323846f) / 180.0f;
    hrSceneOpen(scnRef, HR_WRITE_DISCARD);
    {

      auto mtranslate = hlm::translate4x4(hlm::float3(0, -4.0f, 1));
      auto mscale = hlm::scale4x4(hlm::float3(0.75f, 0.75f, 0.75f));
      auto mrot = hlm::rotate4x4X(-DEG_TO_RAD * 90.0f);
      //auto mres = mtranslate * mscale;
      float4x4 identity;
      hrMeshInstance(scnRef, planeRef, mrot.L());

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

    hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_ext_vtex_6/z_out.png");

    return check_images("test_ext_vtex_6", 1, 10);
  }

  bool test_ext_vtex_7()
  {
    hrErrorCallerPlace(L"test_ext_vtex_7");

    hrSceneLibraryOpen(L"tests_ext/test_ext_vtex_7", HR_WRITE_DISCARD);

    SimpleMesh plane = CreatePlane(4.0f);

    hr_vtex::VectorTexCreateInfo vtex_info{};
    vtex_info.mode = hr_vtex::VTEX_MODE::VTEX_MSDF;
    vtex_info.dpi  = 96;
    vtex_info.sdfCombine  = false;
    vtex_info.sdfAngThres = 3.14;
    vtex_info.drawOutline = false;
    vtex_info.generateMask = true;
    vtex_info.bgColor[0] = 0.0f; vtex_info.bgColor[1] = 0.0f; vtex_info.bgColor[2] = 0.0f; vtex_info.bgColor[3] = 0.0f;

    pugi::xml_node vtex_node;
    HRTextureNodeRef texSDF = hr_vtex::hrTextureVector2DCreateFromFile(L"data/textures/figure-text.svg", &vtex_info, &vtex_node);

    HRMaterialRef matRed   = hrMaterialCreate(L"red");
    HRMaterialRef matGreen = hrMaterialCreate(L"green");
    hrMaterialOpen(matRed, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matRed);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").append_attribute(L"val").set_value(L"0.75 0.0 0.0");

      VERIFY_XML(matNode);
    }
    hrMaterialClose(matRed);

    hrMaterialOpen(matGreen, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGreen);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").append_attribute(L"val").set_value(L"0.0 0.75 0.0");

      VERIFY_XML(matNode);
    }
    hrMaterialClose(matGreen);

    HRMaterialRef matBlend1 = hrMaterialCreateBlend(L"matBlend1", matRed, matGreen);
    hrMaterialOpen(matBlend1, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matBlend1);

      auto blend = matNode.append_child(L"blend");
      blend.append_attribute(L"type").set_value(L"mask_blend");

      auto mask = blend.append_child(L"mask");
      mask.append_attribute(L"val").set_value(1.0f);
      auto texNode = hrTextureBind(texSDF, mask);

      // add procedural texture arguments
      HydraXMLHelpers::moveChildNodes(vtex_node, texNode);
      VERIFY_XML(matNode);
    }
    hrMaterialClose(matBlend1);


    HRMeshRef planeRef = hrMeshCreate(L"my_plane");
    hrMeshOpen(planeRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
    {
      hrMeshVertexAttribPointer4f(planeRef, L"pos", &plane.vPos[0]);
      hrMeshVertexAttribPointer4f(planeRef, L"norm", &plane.vNorm[0]);
      hrMeshVertexAttribPointer2f(planeRef, L"texcoord", &plane.vTexCoord[0]);
      hrMeshMaterialId(planeRef, matBlend1.id);
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

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"0.75 0.75 1");
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

      camNode.append_child(L"fov").text().set(L"45");
      camNode.append_child(L"nearClipPlane").text().set(L"0.01");
      camNode.append_child(L"farClipPlane").text().set(L"100.0");

      camNode.append_child(L"up").text().set(L"0 1 0");
      //camNode.append_child(L"position").text().set(L"0 0 3");
      camNode.append_child(L"position").text().set(L"0 0 10");
      camNode.append_child(L"look_at").text().set(L"0 0 0");
    }
    hrCameraClose(camRef);


    HRRenderRef renderRef = hrRenderCreate(L"HydraModern");
    hrRenderEnableDevice(renderRef, CURR_RENDER_DEVICE, true);

    hrRenderOpen(renderRef, HR_WRITE_DISCARD);
    {
      pugi::xml_node node = hrRenderParamNode(renderRef);

      node.append_child(L"width").text() = L"1024";
      node.append_child(L"height").text() = L"1024";

      node.append_child(L"method_primary").text() = L"pathtracing";
      node.append_child(L"method_secondary").text() = L"pathtracing";
      node.append_child(L"method_tertiary").text() = L"pathtracing";
      node.append_child(L"method_caustic").text() = L"pathtracing";
      node.append_child(L"shadows").text() = L"1";

      node.append_child(L"trace_depth").text() = L"4";
      node.append_child(L"diff_trace_depth").text() = L"2";
      node.append_child(L"pt_error").text() = L"2.0";
      node.append_child(L"minRaysPerPixel").text() = L"512";
      node.append_child(L"maxRaysPerPixel").text() = L"512";
    }
    hrRenderClose(renderRef);


    HRSceneInstRef scnRef = hrSceneCreate(L"my scene");
    const float DEG_TO_RAD = float(3.14159265358979323846f) / 180.0f;
    hrSceneOpen(scnRef, HR_WRITE_DISCARD);
    {

      auto mtranslate = hlm::translate4x4(hlm::float3(0, -4.0f, 1));
      auto mscale = hlm::scale4x4(hlm::float3(0.75f, 0.75f, 0.75f));
      auto mrot = hlm::rotate4x4X(-DEG_TO_RAD * 90.0f);
      //auto mres = mtranslate * mscale;
      float4x4 identity;
      hrMeshInstance(scnRef, planeRef, mrot.L());

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

    hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_ext_vtex_7/z_out.png");

    return check_images("test_ext_vtex_7", 1, 10);
  }

  bool test_ext_vtex_8()
  {
    hrErrorCallerPlace(L"test_ext_vtex_8");

    hrSceneLibraryOpen(L"tests_ext/test_ext_vtex_8", HR_WRITE_DISCARD);

    SimpleMesh plane = CreatePlane(4.0f);

    hr_vtex::VectorTexCreateInfo vtex_mask{};
    vtex_mask.mode = hr_vtex::VTEX_MODE::VTEX_MSDF;
    vtex_mask.dpi  = 64;
    vtex_mask.sdfCombine  = false;
    vtex_mask.sdfAngThres = 3.14;
    vtex_mask.drawOutline = false;
    vtex_mask.generateMask = true;
    vtex_mask.bgColor[0] = 0.0f; vtex_mask.bgColor[1] = 0.0f; vtex_mask.bgColor[2] = 0.0f; vtex_mask.bgColor[3] = 0.0f;
    pugi::xml_node vtex_node_mask;
    HRTextureNodeRef texSDF_mask = hr_vtex::hrTextureVector2DCreateFromFile(L"data/textures/stars.svg", &vtex_mask, &vtex_node_mask);

    hr_vtex::VectorTexCreateInfo vtex_info{};
    vtex_info.mode = hr_vtex::VTEX_MODE::VTEX_MSDF;
    vtex_info.dpi  = 256;
    vtex_info.sdfCombine    = false;
    vtex_info.sdfAngThres   = 3.14;
    vtex_info.drawOutline   = true;
    vtex_info.distThreshold = 1.0f / 128.0f;
    vtex_info.smoothFac     = 1.0f / 256.0f;
    vtex_info.bgColor[0]    = 1.0f; vtex_info.bgColor[1] = 1.0f; vtex_info.bgColor[2] = 0.0f; vtex_info.bgColor[3] = 1.0f;
    vtex_info.textureFlags  = hr_vtex::TEX_CLAMP_U | hr_vtex::TEX_CLAMP_V;
    pugi::xml_node vtex_node_color;
    HRTextureNodeRef texSDF_color = hr_vtex::hrTextureVector2DCreateFromFile(L"data/textures/stars.svg", &vtex_info, &vtex_node_color);

    HRMaterialRef mat0 = hrMaterialCreate(L"mysimplemat");
    hrMaterialOpen(mat0, HR_WRITE_DISCARD);
    {
      xml_node matNode = hrMaterialParamNode(mat0);
      xml_node diff = matNode.append_child(L"diffuse");

      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      auto colorNode = diff.append_child(L"color");
      colorNode.append_attribute(L"tex_apply_mode") = L"replace";
      HydraXMLHelpers::WriteFloat4(colorNode.append_attribute(L"val"), vtex_info.bgColor);
      auto texNode = hrTextureBind(texSDF_color, colorNode);

      // add procedural texture arguments
      HydraXMLHelpers::moveChildNodes(vtex_node_color, texNode);
    }
    hrMaterialClose(mat0);

    HRMaterialRef matRed   = hrMaterialCreate(L"red");
    hrMaterialOpen(matRed, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matRed);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");
      diff.append_child(L"color").append_attribute(L"val").set_value(L"0.75 0.0 0.0");

      VERIFY_XML(matNode);
    }
    hrMaterialClose(matRed);


    HRMaterialRef matBlend1 = hrMaterialCreateBlend(L"matBlend1", mat0, matRed);
    hrMaterialOpen(matBlend1, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matBlend1);

      auto blend = matNode.append_child(L"blend");
      blend.append_attribute(L"type").set_value(L"mask_blend");

      auto mask = blend.append_child(L"mask");
      mask.append_attribute(L"val").set_value(1.0f);
      auto texNode = hrTextureBind(texSDF_mask, mask);

      // add procedural texture arguments
      HydraXMLHelpers::moveChildNodes(vtex_node_mask, texNode);
      VERIFY_XML(matNode);
    }
    hrMaterialClose(matBlend1);


    HRMeshRef planeRef = hrMeshCreate(L"my_plane");
    hrMeshOpen(planeRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
    {
      hrMeshVertexAttribPointer4f(planeRef, L"pos", &plane.vPos[0]);
      hrMeshVertexAttribPointer4f(planeRef, L"norm", &plane.vNorm[0]);
      hrMeshVertexAttribPointer2f(planeRef, L"texcoord", &plane.vTexCoord[0]);
      hrMeshMaterialId(planeRef, matBlend1.id);
//      hrMeshMaterialId(planeRef, mat0.id);
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

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"0.75 0.75 1");
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

      camNode.append_child(L"fov").text().set(L"45");
      camNode.append_child(L"nearClipPlane").text().set(L"0.01");
      camNode.append_child(L"farClipPlane").text().set(L"100.0");

      camNode.append_child(L"up").text().set(L"0 1 0");
      //camNode.append_child(L"position").text().set(L"0 0 3");
      camNode.append_child(L"position").text().set(L"0 0 10");
      camNode.append_child(L"look_at").text().set(L"0 0 0");
    }
    hrCameraClose(camRef);


    HRRenderRef renderRef = hrRenderCreate(L"HydraModern");
    hrRenderEnableDevice(renderRef, CURR_RENDER_DEVICE, true);

    hrRenderOpen(renderRef, HR_WRITE_DISCARD);
    {
      pugi::xml_node node = hrRenderParamNode(renderRef);

      node.append_child(L"width").text() = L"1024";
      node.append_child(L"height").text() = L"1024";

      node.append_child(L"method_primary").text() = L"pathtracing";
      node.append_child(L"method_secondary").text() = L"pathtracing";
      node.append_child(L"method_tertiary").text() = L"pathtracing";
      node.append_child(L"method_caustic").text() = L"pathtracing";
      node.append_child(L"shadows").text() = L"1";

      node.append_child(L"trace_depth").text() = L"4";
      node.append_child(L"diff_trace_depth").text() = L"2";
      node.append_child(L"pt_error").text() = L"2.0";
      node.append_child(L"minRaysPerPixel").text() = L"512";
      node.append_child(L"maxRaysPerPixel").text() = L"512";
    }
    hrRenderClose(renderRef);


    HRSceneInstRef scnRef = hrSceneCreate(L"my scene");
    const float DEG_TO_RAD = float(3.14159265358979323846f) / 180.0f;
    hrSceneOpen(scnRef, HR_WRITE_DISCARD);
    {

      auto mtranslate = hlm::translate4x4(hlm::float3(0, -4.0f, 1));
      auto mscale = hlm::scale4x4(hlm::float3(0.75f, 0.75f, 0.75f));
      auto mrot = hlm::rotate4x4X(-DEG_TO_RAD * 90.0f);
      //auto mres = mtranslate * mscale;
      float4x4 identity;
      hrMeshInstance(scnRef, planeRef, mrot.L());

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

    hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_ext_vtex_8/z_out.png");

    return check_images("test_ext_vtex_8", 1, 10);
  }
}