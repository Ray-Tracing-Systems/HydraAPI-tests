//bool test_sdf()
//{
//
//  hrErrorCallerPlace(L"test_sdf");
//
//  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  hrSceneLibraryOpen(L"tests/test_sdf", HR_WRITE_DISCARD);
//
//  //SimpleMesh sphere = CreateSphere(2.0f, 128);
//  SimpleMesh cubeOpen = CreateCubeOpen(4.0f);
//
//  // textures
//  //
//  HRTextureNodeRef texSDF1 = hrTexture2DCreateFromFile(L"data/textures/kotoriy-chas.png");
//  HRTextureNodeRef texSDF2 = hrTexture2DCreateFromFile(L"data/textures/sdf_font.png");
//  HRTextureNodeRef texProc = hrTextureCreateAdvanced(L"proc", L"sdf1");
//  HRTextureNodeRef texProc2 = hrTextureCreateAdvanced(L"proc", L"sdf2");
//
//  hrTextureNodeOpen(texProc, HR_WRITE_DISCARD);
//  {
//    xml_node texNode = hrTextureParamNode(texProc);
//
//    xml_node code_node = texNode.append_child(L"code");
//    code_node.append_attribute(L"file") = L"data/code/sdf_texture.c";
//    code_node.append_attribute(L"main") = L"main";
//  }
//  hrTextureNodeClose(texProc);
//
//  hrTextureNodeOpen(texProc2, HR_WRITE_DISCARD);
//  {
//    xml_node texNode = hrTextureParamNode(texProc2);
//
//    xml_node code_node = texNode.append_child(L"code");
//    code_node.append_attribute(L"file") = L"data/code/sdf_texture.c";
//    code_node.append_attribute(L"main") = L"main";
//  }
//  hrTextureNodeClose(texProc2);
//
//
//  // other as usual in this test
//  //
//  HRMaterialRef mat0 = hrMaterialCreate(L"mysimplemat");
//  HRMaterialRef mat1 = hrMaterialCreate(L"mesh");
//  HRMaterialRef mat2 = hrMaterialCreate(L"green");
//  HRMaterialRef mat3 = hrMaterialCreate(L"white");
//
//  hrMaterialOpen(mat0, HR_WRITE_DISCARD);
//  {
//    xml_node matNode = hrMaterialParamNode(mat0);
//    xml_node diff = matNode.append_child(L"diffuse");
//
//    diff.append_attribute(L"brdf_type").set_value(L"lambert");
//    auto colorNode = diff.append_child(L"color");
//
//    colorNode.append_attribute(L"val") = L"0.5 0.5 0.5";
//    colorNode.append_attribute(L"tex_apply_mode") = L"multiply";
//
//    auto texNode = hrTextureBind(texProc2, colorNode);
//
//    // not used currently #TODO: figure out what of theese we needed!
//    //
//    texNode.append_attribute(L"matrix");
//    float samplerMatrix[16] = { 1, 0, 0, 0,
//                                0, 1, 0, 0,
//                                0, 0, 1, 0,
//                                0, 0, 0, 1 };
//
//    texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
//    texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
//    texNode.append_attribute(L"input_gamma").set_value(2.2f);
//    texNode.append_attribute(L"input_alpha").set_value(L"rgb");
//
//    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);
//
//    // proc texture sampler settings
//   //
//    xml_node p1 = texNode.append_child(L"arg");
//    xml_node p2 = texNode.append_child(L"arg");
//    xml_node p3 = texNode.append_child(L"arg");
//
//    std::wstringstream strOut;
//    strOut << texSDF1.id; // << L" " << texBitmap2.id;
//    auto val = strOut.str();
//
//    p1.append_attribute(L"id") = 0;
//    p1.append_attribute(L"name") = L"sdfTexture";
//    p1.append_attribute(L"type") = L"sampler2D";
//    p1.append_attribute(L"size") = 1;
//    p1.append_attribute(L"val") = val.c_str();
//
//    p2.append_attribute(L"id") = 1;
//    p2.append_attribute(L"name") = L"texScale";
//    p2.append_attribute(L"type") = L"float2";
//    p2.append_attribute(L"size") = 1;
//    p2.append_attribute(L"val") = L"1 1";
//
//    p3.append_attribute(L"id") = 2;
//    p3.append_attribute(L"name") = L"inColor";
//    p3.append_attribute(L"type") = L"float4";
//    p3.append_attribute(L"size") = 1;
//    p3.append_attribute(L"val") = L"1 0 1 1";
//  }
//  hrMaterialClose(mat0);
//
//  hrMaterialOpen(mat1, HR_WRITE_DISCARD);
//  {
//    xml_node matNode = hrMaterialParamNode(mat1);
//
//    xml_node refl = matNode.append_child(L"reflectivity");
//
//    refl.append_attribute(L"brdf_type").set_value(L"phong");
//    refl.append_child(L"color").text().set(L"0.367059 0.345882 0");
//    refl.append_child(L"glossiness").text().set(L"0.5");
//
//    xml_node diff = matNode.append_child(L"diffuse");
//
//    diff.append_attribute(L"brdf_type").set_value(L"lambert");
//    auto colorNode = diff.append_child(L"color");
//
//    colorNode.append_attribute(L"val") = L"0.5 0.5 0.0";
//    colorNode.append_attribute(L"tex_apply_mode") = L"multiply";
//
//    auto texNode = hrTextureBind(texProc, colorNode);
//
//    texNode.append_attribute(L"matrix");
//    float samplerMatrix[16] = { 1, 0, 0, 0,
//                                0, 1, 0, 0,
//                                0, 0, 1, 0,
//                                0, 0, 0, 1 };
//
//    texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
//    texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
//    texNode.append_attribute(L"input_gamma").set_value(2.2f);
//    texNode.append_attribute(L"input_alpha").set_value(L"rgb");
//
//    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);
//
//    // proc texture sampler settings
//    //
//    xml_node p1 = texNode.append_child(L"arg");
//    xml_node p2 = texNode.append_child(L"arg");
//    xml_node p3 = texNode.append_child(L"arg");
//
//    std::wstringstream strOut;
//    strOut << texSDF2.id; // << L" " << texBitmap2.id;
//    auto val = strOut.str();
//
//    p1.append_attribute(L"id") = 0;
//    p1.append_attribute(L"name") = L"sdfTexture";
//    p1.append_attribute(L"type") = L"sampler2D";
//    p1.append_attribute(L"size") = 1;
//    p1.append_attribute(L"val") = val.c_str();
//
//    p2.append_attribute(L"id") = 1;
//    p2.append_attribute(L"name") = L"texScale";
//    p2.append_attribute(L"type") = L"float2";
//    p2.append_attribute(L"size") = 1;
//    p2.append_attribute(L"val") = L"4 4";
//
//    p3.append_attribute(L"id") = 2;
//    p3.append_attribute(L"name") = L"inColor";
//    p3.append_attribute(L"type") = L"float4";
//    p3.append_attribute(L"size") = 1;
//    p3.append_attribute(L"val") = L"1 0 1 1";
//  }
//  hrMaterialClose(mat1);
//
//  hrMaterialOpen(mat2, HR_WRITE_DISCARD);
//  {
//    xml_node matNode = hrMaterialParamNode(mat2);
//    xml_node diff = matNode.append_child(L"diffuse");
//
//    diff.append_attribute(L"brdf_type").set_value(L"lambert");
//    diff.append_child(L"color").append_attribute(L"val").set_value(L"0.0 0.5 0.0");
//  }
//  hrMaterialClose(mat2);
//
//  hrMaterialOpen(mat3, HR_WRITE_DISCARD);
//  {
//    xml_node matNode = hrMaterialParamNode(mat3);
//    xml_node diff = matNode.append_child(L"diffuse");
//
//    diff.append_attribute(L"brdf_type").set_value(L"lambert");
//    diff.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 0.5");
//  }
//  hrMaterialClose(mat3);
//
//  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  HRMeshRef cubeOpenRef = hrMeshCreate(L"my_box");
//  HRMeshRef planeRef = hrMeshCreate(L"my_plane");
//  //HRMeshRef sphereRef = hrMeshCreate(L"my_sphere");
//
//  HRMeshRef meshRef = hrMeshCreateFromFileDL(L"data/meshes/lucy.vsgf");
//
//  hrMeshOpen(cubeOpenRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
//  {
//    hrMeshVertexAttribPointer4f(cubeOpenRef, L"pos", &cubeOpen.vPos[0]);
//    hrMeshVertexAttribPointer4f(cubeOpenRef, L"norm", &cubeOpen.vNorm[0]);
//    hrMeshVertexAttribPointer2f(cubeOpenRef, L"texcoord", &cubeOpen.vTexCoord[0]);
//
//    int cubeMatIndices[10] = { mat0.id, mat0.id, mat0.id, mat0.id, mat0.id, mat0.id, mat2.id, mat2.id, mat2.id, mat2.id };
//
//    //hrMeshMaterialId(cubeRef, 0);
//    hrMeshPrimitiveAttribPointer1i(cubeOpenRef, L"mind", cubeMatIndices);
//    hrMeshAppendTriangles3(cubeOpenRef, int(cubeOpen.triIndices.size()), &cubeOpen.triIndices[0]);
//  }
//  hrMeshClose(cubeOpenRef);
//
//  hrMeshOpen(meshRef, HR_TRIANGLE_IND3, HR_OPEN_EXISTING);
//  {
//    hrMeshMaterialId(meshRef, mat1.id);
//  }
//  hrMeshClose(meshRef);
//
//  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  HRLightRef rectLight = hrLightCreate(L"my_area_light");
//
//  hrLightOpen(rectLight, HR_WRITE_DISCARD);
//  {
//    pugi::xml_node lightNode = hrLightParamNode(rectLight);
//
//    lightNode.attribute(L"type").set_value(L"area");
//    lightNode.attribute(L"shape").set_value(L"rect");
//    lightNode.attribute(L"distribution").set_value(L"diffuse");
//
//    pugi::xml_node sizeNode = lightNode.append_child(L"size");
//
//    sizeNode.append_attribute(L"half_length") = 1.0f;
//    sizeNode.append_attribute(L"half_width") = 1.0f;
//
//    pugi::xml_node intensityNode = lightNode.append_child(L"intensity");
//
//    intensityNode.append_child(L"color").append_attribute(L"val") = L"1 1 1";
//    intensityNode.append_child(L"multiplier").append_attribute(L"val") = 8.0 * IRRADIANCE_TO_RADIANCE;
//  }
//  hrLightClose(rectLight);
//
//  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  // camera
//  //
//  HRCameraRef camRef = hrCameraCreate(L"my camera");
//
//  hrCameraOpen(camRef, HR_WRITE_DISCARD);
//  {
//    xml_node camNode = hrCameraParamNode(camRef);
//
//    camNode.append_child(L"fov").text().set(L"45");
//    camNode.append_child(L"nearClipPlane").text().set(L"0.01");
//    camNode.append_child(L"farClipPlane").text().set(L"100.0");
//
//    camNode.append_child(L"up").text().set(L"0 1 0");
//    //camNode.append_child(L"position").text().set(L"0 0 3");
//    camNode.append_child(L"position").text().set(L"0 0 14");
//    camNode.append_child(L"look_at").text().set(L"0 0 0");
//  }
//  hrCameraClose(camRef);
//
//  // set up render settings
//  //
//  HRRenderRef renderRef = hrRenderCreate(L"HydraModern"); // opengl1
//  hrRenderEnableDevice(renderRef, CURR_RENDER_DEVICE, true);
//  //hrRenderLogDir(renderRef, L"C:/[Hydra]/logs/", false);
//
//  hrRenderOpen(renderRef, HR_WRITE_DISCARD);
//  {
//    pugi::xml_node node = hrRenderParamNode(renderRef);
//
//    node.append_child(L"width").text() = L"1024";
//    node.append_child(L"height").text() = L"1024";
//
//    node.append_child(L"method_primary").text() = L"pathtracing";
//    node.append_child(L"method_secondary").text() = L"pathtracing";
//    node.append_child(L"method_tertiary").text() = L"pathtracing";
//    node.append_child(L"method_caustic").text() = L"pathtracing";
//    node.append_child(L"shadows").text() = L"1";
//
//    node.append_child(L"trace_depth").text() = L"8";
//    node.append_child(L"diff_trace_depth").text() = L"4";
//    node.append_child(L"pt_error").text() = L"2.0";
//    node.append_child(L"minRaysPerPixel").text() = L"512";
//    node.append_child(L"maxRaysPerPixel").text() = L"512";
//  }
//  hrRenderClose(renderRef);
//
//  // create scene
//  //
//  HRSceneInstRef scnRef = hrSceneCreate(L"my scene");
//
//  const float DEG_TO_RAD = float(3.14159265358979323846f) / 180.0f;
//
//  hrSceneOpen(scnRef, HR_WRITE_DISCARD);
//  {
//
//    auto mtranslate = hlm::translate4x4(hlm::float3(0, -4.0f, 1));
//    auto mscale = hlm::scale4x4(hlm::float3(0.75f, 0.75f, 0.75f));
//    auto mres = mtranslate * mscale;
//    hrMeshInstance(scnRef, meshRef, mres.L());
//
//    auto mrot = hlm::rotate4x4Y(180.0f * DEG_TO_RAD);
//    hrMeshInstance(scnRef, cubeOpenRef, mrot.L());
//
//    mtranslate = hlm::translate4x4(hlm::float3(0, 3.85f, 0));
//    hrLightInstance(scnRef, rectLight, mtranslate.L());
//
//    mtranslate = hlm::translate4x4(hlm::float3(-2.0f, 3.85f, 2.0f));
//    auto mrot1 = hlm::rotate4x4X(DEG_TO_RAD * 30.0f);
//    auto mrot2 = hlm::rotate4x4Y(DEG_TO_RAD * -30.0f);
//    mres = mtranslate * mrot2 * mrot1;
//    hrLightInstance(scnRef, rectLight, mres.L());
//  }
//  hrSceneClose(scnRef);
//
//  hrFlush(scnRef, renderRef, camRef);
//
//  while (true)
//  {
//    std::this_thread::sleep_for(std::chrono::milliseconds(500));
//
//    HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);
//
//    if (info.haveUpdateFB)
//    {
//      auto pres = std::cout.precision(2);
//      std::cout << "rendering progress = " << info.progress << "% \r"; std::cout.flush();
//      std::cout.precision(pres);
//
//
//
//    }
//
//    if (info.finalUpdate)
//      break;
//  }
//
//  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_sdf/z_out.png");
//
//  return check_images("test_sdf", 1, 10);
//}
//
//bool test_sdf2()
//{
//
//  hrErrorCallerPlace(L"test_sdf2");
//
//  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  hrSceneLibraryOpen(L"tests/test_sdf2", HR_WRITE_DISCARD);
//
//  //SimpleMesh sphere = CreateSphere(2.0f, 128);
//  SimpleMesh plane = CreatePlane(4.0f);
//
//  // textures
//  //
//  HRTextureNodeRef texSDF = hrTexture2DCreateFromFile(L"data/textures/MSDF.png");
//  HRTextureNodeRef texProc = hrTextureCreateAdvanced(L"proc", L"sdf1");
//
//  hrTextureNodeOpen(texProc, HR_WRITE_DISCARD);
//  {
//    xml_node texNode = hrTextureParamNode(texProc);
//
//    xml_node code_node = texNode.append_child(L"code");
//    code_node.append_attribute(L"file") = L"data/code/msdf_texture.c";
//    code_node.append_attribute(L"main") = L"main";
//  }
//  hrTextureNodeClose(texProc);
//
//
//  // other as usual in this test
//  //
//  HRMaterialRef mat0 = hrMaterialCreate(L"mysimplemat");
//
//  hrMaterialOpen(mat0, HR_WRITE_DISCARD);
//  {
//    xml_node matNode = hrMaterialParamNode(mat0);
//    xml_node diff = matNode.append_child(L"diffuse");
//
//    diff.append_attribute(L"brdf_type").set_value(L"lambert");
//    auto colorNode = diff.append_child(L"color");
//
//    colorNode.append_attribute(L"val") = L"0.5 0.5 0.5";
//    colorNode.append_attribute(L"tex_apply_mode") = L"multiply";
//
//    auto texNode = hrTextureBind(texProc, colorNode);
//
//    // not used currently #TODO: figure out what of theese we needed!
//    //
//    texNode.append_attribute(L"matrix");
//    float samplerMatrix[16] = { 1, 0, 0, 0,
//                                0, 1, 0, 0,
//                                0, 0, 1, 0,
//                                0, 0, 0, 1 };
//
//    texNode.append_attribute(L"addressing_mode_u").set_value(L"wrap");
//    texNode.append_attribute(L"addressing_mode_v").set_value(L"wrap");
//    texNode.append_attribute(L"input_gamma").set_value(2.2f);
//    texNode.append_attribute(L"input_alpha").set_value(L"rgb");
//
//    HydraXMLHelpers::WriteMatrix4x4(texNode, L"matrix", samplerMatrix);
//
//    // proc texture sampler settings
//   //
//    xml_node p1 = texNode.append_child(L"arg");
//    xml_node p2 = texNode.append_child(L"arg");
//    xml_node p3 = texNode.append_child(L"arg");
//
//    std::wstringstream strOut;
//    strOut << texSDF.id; // << L" " << texBitmap2.id;
//    auto val = strOut.str();
//
//    p1.append_attribute(L"id") = 0;
//    p1.append_attribute(L"name") = L"sdfTexture";
//    p1.append_attribute(L"type") = L"sampler2D";
//    p1.append_attribute(L"size") = 1;
//    p1.append_attribute(L"val") = val.c_str();
//
//    p2.append_attribute(L"id") = 1;
//    p2.append_attribute(L"name") = L"texScale";
//    p2.append_attribute(L"type") = L"float2";
//    p2.append_attribute(L"size") = 1;
//    p2.append_attribute(L"val") = L"1 1";
//
//    p3.append_attribute(L"id") = 2;
//    p3.append_attribute(L"name") = L"inColor";
//    p3.append_attribute(L"type") = L"float4";
//    p3.append_attribute(L"size") = 1;
//    p3.append_attribute(L"val") = L"1 0 1 1";
//  }
//  hrMaterialClose(mat0);
//
//
//  HRMeshRef planeRef = hrMeshCreate(L"my_plane");
//  hrMeshOpen(planeRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
//  {
//    hrMeshVertexAttribPointer4f(planeRef, L"pos", &plane.vPos[0]);
//    hrMeshVertexAttribPointer4f(planeRef, L"norm", &plane.vNorm[0]);
//    hrMeshVertexAttribPointer2f(planeRef, L"texcoord", &plane.vTexCoord[0]);
//    hrMeshMaterialId(planeRef, 0);
//    hrMeshAppendTriangles3(planeRef, int(plane.triIndices.size()), &plane.triIndices[0]);
//  }
//  hrMeshClose(planeRef);
//
//
//  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  HRLightRef rectLight = hrLightCreate(L"my_area_light");
//
//  hrLightOpen(rectLight, HR_WRITE_DISCARD);
//  {
//    pugi::xml_node lightNode = hrLightParamNode(rectLight);
//
//    lightNode.attribute(L"type").set_value(L"area");
//    lightNode.attribute(L"shape").set_value(L"rect");
//    lightNode.attribute(L"distribution").set_value(L"diffuse");
//
//    pugi::xml_node sizeNode = lightNode.append_child(L"size");
//
//    sizeNode.append_attribute(L"half_length") = 1.0f;
//    sizeNode.append_attribute(L"half_width") = 1.0f;
//
//    pugi::xml_node intensityNode = lightNode.append_child(L"intensity");
//
//    intensityNode.append_child(L"color").append_attribute(L"val") = L"1 1 1";
//    intensityNode.append_child(L"multiplier").append_attribute(L"val") = 8.0 * IRRADIANCE_TO_RADIANCE;
//  }
//  hrLightClose(rectLight);
//
//  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  // camera
//  //
//  HRCameraRef camRef = hrCameraCreate(L"my camera");
//
//  hrCameraOpen(camRef, HR_WRITE_DISCARD);
//  {
//    xml_node camNode = hrCameraParamNode(camRef);
//
//    camNode.append_child(L"fov").text().set(L"45");
//    camNode.append_child(L"nearClipPlane").text().set(L"0.01");
//    camNode.append_child(L"farClipPlane").text().set(L"100.0");
//
//    camNode.append_child(L"up").text().set(L"0 1 0");
//    //camNode.append_child(L"position").text().set(L"0 0 3");
//    camNode.append_child(L"position").text().set(L"0 0 10");
//    camNode.append_child(L"look_at").text().set(L"0 0 0");
//  }
//  hrCameraClose(camRef);
//
//  // set up render settings
//  //
//  HRRenderRef renderRef = hrRenderCreate(L"HydraModern"); // opengl1
//  hrRenderEnableDevice(renderRef, CURR_RENDER_DEVICE, true);
//  //hrRenderLogDir(renderRef, L"C:/[Hydra]/logs/", false);
//
//  hrRenderOpen(renderRef, HR_WRITE_DISCARD);
//  {
//    pugi::xml_node node = hrRenderParamNode(renderRef);
//
//    node.append_child(L"width").text() = L"1024";
//    node.append_child(L"height").text() = L"1024";
//
//    node.append_child(L"method_primary").text() = L"pathtracing";
//    node.append_child(L"method_secondary").text() = L"pathtracing";
//    node.append_child(L"method_tertiary").text() = L"pathtracing";
//    node.append_child(L"method_caustic").text() = L"pathtracing";
//    node.append_child(L"shadows").text() = L"1";
//
//    node.append_child(L"trace_depth").text() = L"8";
//    node.append_child(L"diff_trace_depth").text() = L"4";
//    node.append_child(L"pt_error").text() = L"2.0";
//    node.append_child(L"minRaysPerPixel").text() = L"512";
//    node.append_child(L"maxRaysPerPixel").text() = L"512";
//  }
//  hrRenderClose(renderRef);
//
//  // create scene
//  //
//  HRSceneInstRef scnRef = hrSceneCreate(L"my scene");
//
//  const float DEG_TO_RAD = float(3.14159265358979323846f) / 180.0f;
//
//  hrSceneOpen(scnRef, HR_WRITE_DISCARD);
//  {
//
//    auto mtranslate = hlm::translate4x4(hlm::float3(0, -4.0f, 1));
//    auto mscale = hlm::scale4x4(hlm::float3(0.75f, 0.75f, 0.75f));
//    auto mrot = hlm::rotate4x4X(-DEG_TO_RAD * 90.0f);
//    //auto mres = mtranslate * mscale;
//    float4x4 identity;
//    hrMeshInstance(scnRef, planeRef, mrot.L());
//
//    mrot = hlm::rotate4x4X(-DEG_TO_RAD * 90.0f);
//    mtranslate = hlm::translate4x4(hlm::float3(0, 1.5f, 12.0f));
//    auto mres = mtranslate * mrot;
//    hrLightInstance(scnRef, rectLight, mres.L());
//
//    mtranslate = hlm::translate4x4(hlm::float3(-2.0f, 3.85f, 2.0f));
//    auto mrot1 = hlm::rotate4x4X(DEG_TO_RAD * 30.0f);
//    auto mrot2 = hlm::rotate4x4Y(DEG_TO_RAD * -30.0f);
//    mres = mtranslate * mrot2 * mrot1;
//    hrLightInstance(scnRef, rectLight, mres.L());
//  }
//  hrSceneClose(scnRef);
//
//  hrFlush(scnRef, renderRef, camRef);
//
//  while (true)
//  {
//    std::this_thread::sleep_for(std::chrono::milliseconds(500));
//
//    HRRenderUpdateInfo info = hrRenderHaveUpdate(renderRef);
//
//    if (info.haveUpdateFB)
//    {
//      auto pres = std::cout.precision(2);
//      std::cout << "rendering progress = " << info.progress << "% \r"; std::cout.flush();
//      std::cout.precision(pres);
//    }
//
//    if (info.finalUpdate)
//      break;
//  }
//
//  hrRenderSaveFrameBufferLDR(renderRef, L"tests_images/test_sdf2/z_out.png");
//
//  return check_images("test_sdf2", 1, 10);
//}
