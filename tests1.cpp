#include "tests.h"
#include <math.h>
#include <iomanip>
#include <fstream>
#include <set>
#include <algorithm>

#if defined(WIN32)
  #include <GLFW/glfw3.h>
  #pragma comment(lib, "glfw3dll.lib")
#else
  #include <GLFW/glfw3.h>
#endif

#include <wchar.h>
#include <wctype.h>

#include "../hydra_api/HR_HDRImageTool.h"

///////////////////////////////////////////////////////////////////////////////////


bool check_dups(pugi::xml_node a_lib) // check that each object with some id have only one xml node.
{
  std::set<int32_t> idSet; 

  for (pugi::xml_node child = a_lib.first_child(); child != nullptr; child = child.next_sibling())
  {
    int32_t objId = child.attribute(L"id").as_int();
    if (idSet.find(objId) == idSet.end())
      idSet.insert(objId);
    else
      return false;
  }

  return true;
}


bool check_dups_attr(pugi::xml_node a_lib) // check that each object with some id have only one xml node.
{
  for (pugi::xml_node child = a_lib.first_child(); child != nullptr; child = child.next_sibling())
  {
    std::set< std::wstring > attrSet;
    for (auto attr : child.attributes())
    {
      const std::wstring name = attr.name();
      if(attrSet.find(name) == attrSet.end())
        attrSet.insert(name);
      else
      {
        std::wcout << L"duplicate attribute found for node " << child.name() << L", name = " \
                   << child.attribute(L"name").as_string() << L", id = " \
                   << child.attribute(L"id").as_int() << std::endl;
        return false;
      }
    }
    
    if(!check_dups_attr(child))
      return false;
  }
  
  return true;
}


bool check_all_duplicates(const std::wstring& a_fileName)
{
  pugi::xml_document doc;
  doc.load_file(a_fileName.c_str());
  if (doc == nullptr)
    return false;

  pugi::xml_node lib0 = doc.child(L"textures_lib");
  pugi::xml_node lib1 = doc.child(L"materials_lib");
  pugi::xml_node lib2 = doc.child(L"lights_lib");
  pugi::xml_node lib3 = doc.child(L"geometry_lib");
  pugi::xml_node lib4 = doc.child(L"cam_lib");
  pugi::xml_node lib5 = doc.child(L"render_lib");
  pugi::xml_node lib6 = doc.child(L"scenes");

  if (lib0 == nullptr || lib1 == nullptr || lib2 == nullptr || lib3 == nullptr)
    return false;

  if (lib4 == nullptr || lib5 == nullptr || lib6 == nullptr)
    return false;

  bool thistest = check_dups(lib0) && check_dups(lib1) && check_dups(lib2) &&
                  check_dups(lib3) && check_dups(lib4) && check_dups(lib5) && check_dups(lib6);
  
  bool othertest = check_dups_attr(lib0) && check_dups_attr(lib1) && check_dups_attr(lib2) &&
                   check_dups_attr(lib3) && check_dups_attr(lib4) && check_dups_attr(lib5) &&
                   check_dups_attr(lib6);

  return thistest && othertest;
}


bool check_test_01(const std::wstring a_fileName)
{
  pugi::xml_document doc;
  doc.load_file(a_fileName.c_str());

  pugi::xml_node libMat = doc.child(L"materials_lib");

  pugi::xml_node m1 = libMat.find_child_by_attribute(L"id", L"0");
  pugi::xml_node m2 = libMat.find_child_by_attribute(L"id", L"1");

  bool name1_ok      = std::wstring(m1.attribute(L"name").value()) == L"MyRedMaterialName";
  bool name2_ok      = std::wstring(m2.attribute(L"name").value()) == L"MyGreenMaterialName";
  bool diffColor1_ok = (std::wstring(m1.child(L"diffuse").child(L"color").text().get()) == L"0.35 0.02 0.02");
  bool diffColor2_ok = (std::wstring(m2.child(L"diffuse").child(L"color").text().get()) == L"0.1 0.9 0.1");
  bool reflBrdf_ok   = std::wstring(m1.child(L"reflectivity").attribute(L"brdf_type").value()) == L"phong";

  return name1_ok && name2_ok && diffColor1_ok && diffColor2_ok && reflBrdf_ok;
}


bool test01_materials_add()
{
  hrSceneLibraryOpen(L"tests/test_01", HR_WRITE_DISCARD);

  // material definition
  //
  HRMaterialRef mat  = hrMaterialCreate(L"MyRedMaterialName"); // create new HydraMaterial
  HRMaterialRef mat2 = hrMaterialCreate(L"MyGreenMaterialName"); // create new material

  hrMaterialOpen(mat, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat);

    xml_node diff = matNode.append_child(L"diffuse");
    xml_node refl = matNode.append_child(L"reflectivity");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").text().set(L"0.35 0.02 0.02");
    diff.append_child(L"roughness").text().set(L"0");

    refl.append_attribute(L"brdf_type").set_value(L"phong");
    refl.append_child(L"color").text().set(L"0.1 0.1 0.1");
    refl.append_child(L"resnel_IOR").text().set(L"1.5");
    refl.append_child(L"glosiness").text().set(L"0.75");
  }
  hrMaterialClose(mat); // HRCheckErrorsMacro; ... 


  hrMaterialOpen(mat2, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat2);

    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").text().set(L"0.1 0.9 0.1");
    diff.append_child(L"roughness").text().set(L"0");
  }
  hrMaterialClose(mat2);

  hrFlush();

  return check_test_01(L"tests/test_01/statex_00001.xml") && check_test_01(L"tests/test_01/change_00000.xml");
}


bool check_test_02(const std::wstring a_fileName)
{
  pugi::xml_document doc;
  doc.load_file(a_fileName.c_str());

  pugi::xml_node libMat = doc.child(L"materials_lib");

  pugi::xml_node m1 = libMat.find_child_by_attribute(L"id", L"0");
  pugi::xml_node m2 = libMat.find_child_by_attribute(L"id", L"1");

  bool name1_ok = std::wstring(m1.attribute(L"name").value()) == L"MyRedMaterialName";
  bool name2_ok = std::wstring(m2.attribute(L"name").value()) == L"MyGreenMaterialName";

  bool diff1_ok = (m1.child(L"diffuse")      == nullptr);
  bool reft1_ok = (m1.child(L"reflectivity") == nullptr);

  bool diffColor2_ok = (std::wstring(m2.child(L"diffuse").child(L"color").text().get()) == L"0.952941 0.0196078 0.0196078");
  bool reflBrdf_ok   = (std::wstring(m2.child(L"diffuse").attribute(L"brdf_type").value()) == L"oren_nayar");
  bool roughness_ok  = (std::wstring(m2.child(L"diffuse").child(L"roughness").text().get()) == L"1");

  return name1_ok && name2_ok && diff1_ok && reft1_ok && diffColor2_ok && reflBrdf_ok && roughness_ok;
}


bool test02_materials_changes_open_mode()
{
  hrSceneLibraryOpen(L"tests/test_02", HR_WRITE_DISCARD);

  // material definition
  //
  HRMaterialRef mat  = hrMaterialCreate(L"MyRedMaterialName");   // create new HydraMaterial
  HRMaterialRef mat2 = hrMaterialCreate(L"MyGreenMaterialName"); // create new material

  hrMaterialOpen(mat, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat);

    xml_node diff = matNode.append_child(L"diffuse");
    xml_node refl = matNode.append_child(L"reflectivity");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").text().set(L"0.352941 0.0196078 0.0196078");
    diff.append_child(L"roughness").text().set(L"0");

    refl.append_attribute(L"brdf_type").set_value(L"phong");
    refl.append_child(L"color").text().set(L"0 0 0 ");
    refl.append_child(L"resnel_IOR").text().set(L"1.5");
    refl.append_child(L"glosiness").text().set(L"0.75");
  }
  hrMaterialClose(mat); // HRCheckErrorsMacro; ... 


  hrMaterialOpen(mat2, HR_WRITE_DISCARD);
  {
    xml_node matNode = hrMaterialParamNode(mat2);

    xml_node diff = matNode.append_child(L"diffuse");

    diff.append_attribute(L"brdf_type").set_value(L"lambert");
    diff.append_child(L"color").text().set(L"0.952941 0.0196078 0.0196078");
    diff.append_child(L"roughness").text().set(L"0");
  }
  hrMaterialClose(mat2);

  hrFlush();

  // make changes
  //
  hrMaterialOpen(mat, HR_WRITE_DISCARD);
  hrMaterialClose(mat); 
  
  hrMaterialOpen(mat2, HR_OPEN_EXISTING);
  {
    xml_node matNode = hrMaterialParamNode(mat2);
  
    xml_node diff = matNode.child(L"diffuse");
  
    diff.attribute(L"brdf_type").set_value(L"oren_nayar");
    diff.child(L"roughness").text().set(L"1");
  }
  hrMaterialClose(mat2);
  
  hrFlush();

  // now check xml by parsing it and find essential attributes!
  bool noDups1 = check_all_duplicates(L"tests/test_02/statex_00001.xml");
  bool noDups2 = check_all_duplicates(L"tests/test_02/statex_00002.xml");

  return check_test_02(L"tests/test_02/statex_00002.xml") && check_test_02(L"tests/test_02/change_00001.xml") && noDups1 && noDups2;
}


bool check_test_03()
{
  pugi::xml_document doc;
  doc.load_file(L"tests/test_03/statex_00001.xml");

  pugi::xml_node libLights = doc.child(L"lights_lib");

  pugi::xml_node l1 = libLights.find_child_by_attribute(L"id", L"0");
  pugi::xml_node l2 = libLights.find_child_by_attribute(L"id", L"1");

  bool color1_ok = std::wstring(l1.child(L"intensity").child(L"color").attribute(L"val").as_string()) == L"10 10 10";
  bool color2_ok = std::wstring(l2.child(L"intensity").child(L"color").attribute(L"val").as_string()) == L"1 1 1";

  return color1_ok && color2_ok;
}


bool test03_lights_add()
{
  hrSceneLibraryOpen(L"tests/test_03", HR_WRITE_DISCARD);

  HRLightRef light  = hrLightCreate(L"testPointLight1");
  HRLightRef light2 = hrLightCreate(L"testSpotLight2");
  HRLightRef light3 = hrLightCreate(L"testPointLight3");

  // simple light example; the container will be translated to point light;
  //
  hrLightOpen(light, HR_WRITE_DISCARD);
  {
    xml_node lightNode = hrLightParamNode(light);

    lightNode.attribute(L"shape").set_value(L"point");                // you don't have to add attributes, only modify them
    lightNode.attribute(L"distribution").set_value(L"onmi");          // you don't have to add attributes, only modify them

    lightNode.append_child(L"position").append_attribute(L"val") = L"0 10 0";

    xml_node intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").append_attribute(L"val")      = L"10 10 10";
    intensityNode.append_child(L"multiplier").append_attribute(L"val") = 2.0f*IRRADIANCE_TO_RADIANCE;
  }
  hrLightClose(light);

  hrLightOpen(light2, HR_WRITE_DISCARD);
  {
    xml_node lightNode = hrLightParamNode(light2);

    lightNode.append_child(L"position").append_attribute(L"val") = L"0 10 0";

    xml_node intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").append_attribute(L"val")      = L"1 1 1";
    intensityNode.append_child(L"multiplier").append_attribute(L"val") = 1.5f*IRRADIANCE_TO_RADIANCE;
  }
  hrLightClose(light2);

  hrFlush();

  return check_test_03();
}


bool check_test_04()
{
  pugi::xml_document doc;
  doc.load_file(L"tests/test_04/statex_00002.xml");

  pugi::xml_node libLights = doc.child(L"lights_lib");

  pugi::xml_node l1 = libLights.find_child_by_attribute(L"id", L"0");
  pugi::xml_node l2 = libLights.find_child_by_attribute(L"id", L"1");

  bool color1_ok     = (std::wstring(l1.child(L"intensity").child(L"color").attribute(L"val").as_string()) == L"10 10 10");
  bool color2_not_ok = (l2.child(L"intensity").child(L"color") == nullptr);

  bool pos1_ok       = (std::wstring(l1.child(L"position").attribute(L"val").as_string()) == L"0 10 0");
  bool pos2_ok       = (std::wstring(l2.child(L"position").attribute(L"val").as_string()) == L"100 0 100");

  return color1_ok && color2_not_ok && pos1_ok && pos2_ok;
}

bool check_test_04_changes()
{
  pugi::xml_document doc;
  doc.load_file(L"tests/test_04/change_00001.xml");

  pugi::xml_node libLights = doc.child(L"lights_lib");

  pugi::xml_node l1 = libLights.find_child_by_attribute(L"id", L"0");
  pugi::xml_node l2 = libLights.find_child_by_attribute(L"id", L"1");
  pugi::xml_node l3 = libLights.find_child_by_attribute(L"id", L"2");

  return (l1 != nullptr) && (l2 != nullptr) && (l3 == nullptr);
}


bool test04_lights_add_change()
{
  hrSceneLibraryOpen(L"tests/test_04", HR_WRITE_DISCARD);

  HRLightRef light  = hrLightCreate(L"testPointLight1");
  HRLightRef light2 = hrLightCreate(L"testSpotLight2");
  HRLightRef light3 = hrLightCreate(L"testSpotLight3");

  // simple light example; the container will be translated to point light;
  //
  hrLightOpen(light, HR_WRITE_DISCARD);
  {
    xml_node lightNode = hrLightParamNode(light);

    lightNode.attribute(L"shape").set_value(L"point");                // you don't have to add attributes, only modify them
    lightNode.attribute(L"distribution").set_value(L"onmi");          // you don't have to add attributes, only modify them

    lightNode.append_child(L"position").append_attribute(L"val") = L"20 20 20";

    xml_node intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").append_attribute(L"val")      = L"10 10 10";
    intensityNode.append_child(L"multiplier").append_attribute(L"val") = 2.0f*IRRADIANCE_TO_RADIANCE;
  }
  hrLightClose(light);

  hrLightOpen(light2, HR_WRITE_DISCARD);
  {
    xml_node lightNode = hrLightParamNode(light2);

    lightNode.append_child(L"position").append_attribute(L"val") = L"0 10 0";

    xml_node intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").append_attribute(L"val")      = L"1 1 1";
    intensityNode.append_child(L"multiplier").append_attribute(L"val") = 1.5f*IRRADIANCE_TO_RADIANCE;
  }
  hrLightClose(light2);

  hrFlush();

  hrLightOpen(light, HR_OPEN_EXISTING);
  {
    xml_node lightNode = hrLightParamNode(light);
    lightNode.child(L"position").attribute(L"val") = L"0 10 0";
  }
  hrLightClose(light);

  hrLightOpen(light2, HR_WRITE_DISCARD);
  {
    xml_node lightNode = hrLightParamNode(light2);
    lightNode.append_child(L"position").append_attribute(L"val") = L"100 0 100";
  }
  hrLightClose(light2);

  hrFlush();

  return check_test_04() && check_test_04_changes();
}

////////////////////////////////////////////////

bool check_05_06_mesh_cube(const wchar_t* a_path) // L"tests/test_64/statex_00003.xml"
{
  pugi::xml_document doc;
  doc.load_file(a_path);

  pugi::xml_node libLights = doc.child(L"geometry_lib");
  pugi::xml_node mesh      = libLights.find_child_by_attribute(L"id", L"0");

  pugi::xml_node pos       = mesh.child(L"positions");
  pugi::xml_node norm      = mesh.child(L"normals");
  pugi::xml_node texcoords = mesh.child(L"texcoords");

  pugi::xml_node ind       = mesh.child(L"indices");
  pugi::xml_node mind      = mesh.child(L"matindices");

  int vertNum = pos.attribute(L"bytesize").as_int() / (sizeof(float)*4);
  int indNum  = ind.attribute(L"bytesize").as_int() / sizeof(int);
  int triNum  = mind.attribute(L"bytesize").as_int() / sizeof(int);

  return (vertNum == 24) && (indNum == 36) && (triNum == 12) && (norm != nullptr) && (texcoords != nullptr);
}


bool check_test_05(const wchar_t* path)
{
  bool cubeIsOk = check_05_06_mesh_cube(path);
  if (!cubeIsOk)
    return false;

  pugi::xml_document doc;
  doc.load_file(path);

  pugi::xml_node scene = doc.child(L"scenes").child(L"scene");

  int childCount = 0;
  for (auto p = scene.first_child(); p != nullptr; p = p.next_sibling())
    childCount++;

  if (childCount != 1)
    return false;

  std::wstring matStr = scene.child(L"instance").attribute(L"matrix").as_string();
  bool matrixIsOk = (matStr == L"3 0 0 0 0 3 0 0 0 0 3 0 0 0 0 3 ");

  return matrixIsOk;
}


bool check_test_05_changes()
{
  bool cubeIsOk = check_05_06_mesh_cube(L"tests/test_05/change_00000.xml");
  if (!cubeIsOk)
    return false;

  pugi::xml_document doc;
  doc.load_file("tests/test_05/change_00000.xml");

  pugi::xml_node scene = doc.child(L"scenes").child(L"scene");

  int childCount = 0;
  for (auto p = scene.first_child(); p != nullptr; p = p.next_sibling())
    childCount++;

  if (childCount != 1)
    return false;

  std::wstring matStr = scene.child(L"instance").attribute(L"matrix").as_string();
  bool matrixIsOk = (matStr == L"1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1 ");

  return matrixIsOk;
}

bool check_test_05_changes1()
{
  pugi::xml_document doc;
  doc.load_file(L"tests/test_05/change_00001.xml");

  pugi::xml_node libLights = doc.child(L"geometry_lib");
  pugi::xml_node mesh      = libLights.find_child_by_attribute(L"id", L"0");
  pugi::xml_node scene     = doc.child(L"scenes").child(L"scene");

  int childCount = 0;
  for (auto p = scene.first_child(); p != nullptr; p = p.next_sibling())
    childCount++;

  if (childCount != 1)
    return false;

  std::wstring matStr = scene.child(L"instance").attribute(L"matrix").as_string();
  bool matrixIsOk = (matStr == L"2 0 0 0 0 2 0 0 0 0 2 0 0 0 0 2 ");

  const bool res = (matrixIsOk && (mesh == nullptr));
  return res;
}


bool test05_instances_write_discard()
{
  hrSceneLibraryOpen(L"tests/test_05", HR_WRITE_DISCARD);

  uint32_t numberVertices = 24;
  uint32_t numberIndices  = 36;

  float cubeVertices[] =
  {
    -1.0f, -1.0f, -1.0f, +1.0f,
    -1.0f, -1.0f, +1.0f, +1.0f,
    +1.0f, -1.0f, +1.0f, +1.0f,
    +1.0f, -1.0f, -1.0f, +1.0f,
    -1.0f, +1.0f, -1.0f, +1.0f,
    -1.0f, +1.0f, +1.0f, +1.0f,
    +1.0f, +1.0f, +1.0f, +1.0f,
    +1.0f, +1.0f, -1.0f, +1.0f,
    -1.0f, -1.0f, -1.0f, +1.0f,
    -1.0f, +1.0f, -1.0f, +1.0f,
    +1.0f, +1.0f, -1.0f, +1.0f,
    +1.0f, -1.0f, -1.0f, +1.0f,
    -1.0f, -1.0f, +1.0f, +1.0f,
    -1.0f, +1.0f, +1.0f, +1.0f,
    +1.0f, +1.0f, +1.0f, +1.0f,
    +1.0f, -1.0f, +1.0f, +1.0f,
    -1.0f, -1.0f, -1.0f, +1.0f,
    -1.0f, -1.0f, +1.0f, +1.0f,
    -1.0f, +1.0f, +1.0f, +1.0f,
    -1.0f, +1.0f, -1.0f, +1.0f,
    +1.0f, -1.0f, -1.0f, +1.0f,
    +1.0f, -1.0f, +1.0f, +1.0f,
    +1.0f, +1.0f, +1.0f, +1.0f,
    +1.0f, +1.0f, -1.0f, +1.0f
  };

  float cubeNormals[] =
  {
    0.0f, -1.0f, 0.0f, +1.0f,
    0.0f, -1.0f, 0.0f, +1.0f,
    0.0f, -1.0f, 0.0f, +1.0f,
    0.0f, -1.0f, 0.0f, +1.0f,
    0.0f, +1.0f, 0.0f, +1.0f,
    0.0f, +1.0f, 0.0f, +1.0f,
    0.0f, +1.0f, 0.0f, +1.0f,
    0.0f, +1.0f, 0.0f, +1.0f,
    0.0f, 0.0f, -1.0f, +1.0f,
    0.0f, 0.0f, -1.0f, +1.0f,
    0.0f, 0.0f, -1.0f, +1.0f,
    0.0f, 0.0f, -1.0f, +1.0f,
    0.0f, 0.0f, +1.0f, +1.0f,
    0.0f, 0.0f, +1.0f, +1.0f,
    0.0f, 0.0f, +1.0f, +1.0f,
    0.0f, 0.0f, +1.0f, +1.0f,
    -1.0f, 0.0f, 0.0f, +1.0f,
    -1.0f, 0.0f, 0.0f, +1.0f,
    -1.0f, 0.0f, 0.0f, +1.0f,
    -1.0f, 0.0f, 0.0f, +1.0f,
    +1.0f, 0.0f, 0.0f, +1.0f,
    +1.0f, 0.0f, 0.0f, +1.0f,
    +1.0f, 0.0f, 0.0f, +1.0f,
    +1.0f, 0.0f, 0.0f, +1.0f
  };

  float cubeTexCoords[] =
  {
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,
    0.0f, 0.0f,
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
  };

  uint32_t cubeIndices[] =
  {
    0, 2, 1,
    0, 3, 2,
    4, 5, 6,
    4, 6, 7,
    8, 9, 10,
    8, 10, 11,
    12, 15, 14,
    12, 14, 13,
    16, 17, 18,
    16, 18, 19,
    20, 23, 22,
    20, 22, 21
  };

  // glEnableClientState(GL_VERTEX_ARRAY);
  // glVertexPointer(4, GL_FLOAT, 0, cubeVertices);
  // glDrawElements(GL_TRIANGLES, numberIndices, GL_UNSIGNED_INT, cubeIndices);

  auto cubeRef = hrMeshCreate(L"cube");

  hrMeshOpen(cubeRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
  {
    hrMeshVertexAttribPointer4f(cubeRef, L"pos", cubeVertices);
    hrMeshVertexAttribPointer4f(cubeRef, L"norm", cubeNormals);
    hrMeshVertexAttribPointer2f(cubeRef, L"texcoord", cubeTexCoords);
    

    hrMeshMaterialId(cubeRef, 0);
    hrMeshAppendTriangles3(cubeRef, numberIndices, (int*)cubeIndices);
  }
  hrMeshClose(cubeRef);

  auto scnRef = hrSceneCreate(L"my scene");

  // draw scene (1)
  //
  hrSceneOpen(scnRef, HR_WRITE_DISCARD);
  {
    float matrixRot[16] = { 1, 0, 0, 0,
                            0, 1, 0, 0,
                            0, 0, 1, 0,
                            0, 0, 0, 1 };

    hrMeshInstance(scnRef, cubeRef, matrixRot);
  }
  hrSceneClose(scnRef);

  hrFlush();
  
  // draw scene (2)
  //
  hrSceneOpen(scnRef, HR_WRITE_DISCARD);
  {
    float matrixRot[16] = { 2, 0, 0, 0,
                            0, 2, 0, 0,
                            0, 0, 2, 0,
                            0, 0, 0, 2 };

    hrMeshInstance(scnRef, cubeRef, matrixRot);
  }
  hrSceneClose(scnRef);

  hrFlush();

  // draw scene (3)
  //
  hrSceneOpen(scnRef, HR_WRITE_DISCARD);
  {
    float matrixRot[16] = { 3, 0, 0, 0,
                            0, 3, 0, 0,
                            0, 0, 3, 0,
                            0, 0, 0, 3 };

    hrMeshInstance(scnRef, cubeRef, matrixRot);
  }
  hrSceneClose(scnRef);

  hrFlush();

  bool noDups1 = check_all_duplicates(L"tests/test_05/statex_00002.xml");
  bool noDups2 = check_all_duplicates(L"tests/test_05/statex_00003.xml");

  return check_test_05(L"tests/test_05/statex_00003.xml") && check_test_05_changes() && check_test_05_changes1() && noDups1 && noDups2;
}


bool check_test_06()
{
  const wchar_t* path = L"tests/test_06/statex_00003.xml";

  bool cubeIsOk = check_05_06_mesh_cube(path);
  if (!cubeIsOk)
    return false;

  pugi::xml_document doc;
  doc.load_file(path);

  pugi::xml_node scene = doc.child(L"scenes").child(L"scene");

  int childCount = 0;
  for (auto p = scene.first_child(); p != nullptr; p = p.next_sibling())
    childCount++;

  if (childCount != 3)
    return false;

  pugi::xml_node i1 = scene.child(L"instance");
  pugi::xml_node i2 = i1.next_sibling();
  pugi::xml_node i3 = i2.next_sibling();

  bool matrix1IsOk = (std::wstring(i1.attribute(L"matrix").as_string()) == L"1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1 ");
  bool matrix2IsOk = (std::wstring(i2.attribute(L"matrix").as_string()) == L"2 0 0 0 0 2 0 0 0 0 2 0 0 0 0 2 ");
  bool matrix3IsOk = (std::wstring(i3.attribute(L"matrix").as_string()) == L"3 0 0 0 0 3 0 0 0 0 3 0 0 0 0 3 ");

  return matrix1IsOk && matrix2IsOk && matrix3IsOk;
}


bool test06_instances_open_existent()
{
  hrSceneLibraryOpen(L"tests/test_06", HR_WRITE_DISCARD);

  uint32_t numberVertices = 24;
  uint32_t numberIndices = 36;

  float cubeVertices[] =
  {
    -1.0f, -1.0f, -1.0f, +1.0f,
    -1.0f, -1.0f, +1.0f, +1.0f,
    +1.0f, -1.0f, +1.0f, +1.0f,
    +1.0f, -1.0f, -1.0f, +1.0f,
    -1.0f, +1.0f, -1.0f, +1.0f,
    -1.0f, +1.0f, +1.0f, +1.0f,
    +1.0f, +1.0f, +1.0f, +1.0f,
    +1.0f, +1.0f, -1.0f, +1.0f,
    -1.0f, -1.0f, -1.0f, +1.0f,
    -1.0f, +1.0f, -1.0f, +1.0f,
    +1.0f, +1.0f, -1.0f, +1.0f,
    +1.0f, -1.0f, -1.0f, +1.0f,
    -1.0f, -1.0f, +1.0f, +1.0f,
    -1.0f, +1.0f, +1.0f, +1.0f,
    +1.0f, +1.0f, +1.0f, +1.0f,
    +1.0f, -1.0f, +1.0f, +1.0f,
    -1.0f, -1.0f, -1.0f, +1.0f,
    -1.0f, -1.0f, +1.0f, +1.0f,
    -1.0f, +1.0f, +1.0f, +1.0f,
    -1.0f, +1.0f, -1.0f, +1.0f,
    +1.0f, -1.0f, -1.0f, +1.0f,
    +1.0f, -1.0f, +1.0f, +1.0f,
    +1.0f, +1.0f, +1.0f, +1.0f,
    +1.0f, +1.0f, -1.0f, +1.0f
  };

  float cubeNormals[] =
  {
    0.0f, -1.0f, 0.0f, +1.0f,
    0.0f, -1.0f, 0.0f, +1.0f,
    0.0f, -1.0f, 0.0f, +1.0f,
    0.0f, -1.0f, 0.0f, +1.0f,
    0.0f, +1.0f, 0.0f, +1.0f,
    0.0f, +1.0f, 0.0f, +1.0f,
    0.0f, +1.0f, 0.0f, +1.0f,
    0.0f, +1.0f, 0.0f, +1.0f,
    0.0f, 0.0f, -1.0f, +1.0f,
    0.0f, 0.0f, -1.0f, +1.0f,
    0.0f, 0.0f, -1.0f, +1.0f,
    0.0f, 0.0f, -1.0f, +1.0f,
    0.0f, 0.0f, +1.0f, +1.0f,
    0.0f, 0.0f, +1.0f, +1.0f,
    0.0f, 0.0f, +1.0f, +1.0f,
    0.0f, 0.0f, +1.0f, +1.0f,
    -1.0f, 0.0f, 0.0f, +1.0f,
    -1.0f, 0.0f, 0.0f, +1.0f,
    -1.0f, 0.0f, 0.0f, +1.0f,
    -1.0f, 0.0f, 0.0f, +1.0f,
    +1.0f, 0.0f, 0.0f, +1.0f,
    +1.0f, 0.0f, 0.0f, +1.0f,
    +1.0f, 0.0f, 0.0f, +1.0f,
    +1.0f, 0.0f, 0.0f, +1.0f
  };

  float cubeTexCoords[] =
  {
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,
    0.0f, 0.0f,
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
  };

  uint32_t cubeIndices[] =
  {
    0, 2, 1,
    0, 3, 2,
    4, 5, 6,
    4, 6, 7,
    8, 9, 10,
    8, 10, 11,
    12, 15, 14,
    12, 14, 13,
    16, 17, 18,
    16, 18, 19,
    20, 23, 22,
    20, 22, 21
  };

  // glEnableClientState(GL_VERTEX_ARRAY);
  // glVertexPointer(4, GL_FLOAT, 0, cubeVertices);
  // glDrawElements(GL_TRIANGLES, numberIndices, GL_UNSIGNED_INT, cubeIndices);

  auto cubeRef = hrMeshCreate(L"cube");

  hrMeshOpen(cubeRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
  {
    hrMeshVertexAttribPointer4f(cubeRef, L"pos", cubeVertices);
    hrMeshVertexAttribPointer4f(cubeRef, L"norm", cubeNormals);
    hrMeshVertexAttribPointer2f(cubeRef, L"texcoord", cubeTexCoords);

    hrMeshMaterialId(cubeRef, 0);
    hrMeshAppendTriangles3(cubeRef, numberIndices, (int*)cubeIndices);
  }
  hrMeshClose(cubeRef);

  auto scnRef = hrSceneCreate(L"my scene");

  // draw scene (1)
  //
  hrSceneOpen(scnRef, HR_WRITE_DISCARD);
  {
    float matrixRot[16] = { 1, 0, 0, 0,
                            0, 1, 0, 0,
                            0, 0, 1, 0,
                            0, 0, 0, 1 };

    hrMeshInstance(scnRef, cubeRef, matrixRot);
  }
  hrSceneClose(scnRef);

  hrFlush();


  // draw scene (2)
  //
  hrSceneOpen(scnRef, HR_OPEN_EXISTING);
  {
    float matrixRot[16] = { 2, 0, 0, 0,
                            0, 2, 0, 0,
                            0, 0, 2, 0,
                            0, 0, 0, 2 };

    hrMeshInstance(scnRef, cubeRef, matrixRot);
  }
  hrSceneClose(scnRef);

  hrFlush();

  // draw scene (3)
  //
  hrSceneOpen(scnRef, HR_OPEN_EXISTING);
  {
    float matrixRot[16] = { 3, 0, 0, 0,
                            0, 3, 0, 0,
                            0, 0, 3, 0,
                            0, 0, 0, 3 };

    hrMeshInstance(scnRef, cubeRef, matrixRot);
  }
  hrSceneClose(scnRef);

  hrFlush();

  bool noDups1 = check_all_duplicates(L"tests/test_06/statex_00002.xml");
  bool noDups2 = check_all_duplicates(L"tests/test_06/statex_00003.xml");

  return check_test_06() && noDups1 && noDups2;
}


bool check_test_07(const wchar_t* a_path)
{
  pugi::xml_document doc;
  doc.load_file(a_path);

  pugi::xml_node libLights = doc.child(L"cam_lib");

  pugi::xml_node c1 = libLights.find_child_by_attribute(L"id", L"0");

  bool ok1 = std::wstring(c1.child(L"position").text().as_string()) == L"0 0 4.50035";
  bool ok2 = std::wstring(c1.child(L"look_at").text().as_string())  == L"0 1.62921e-005 -95.4996";

  return ok1 && ok2;
}


bool test07_camera_add()
{
  hrSceneLibraryOpen(L"tests/test_07", HR_WRITE_DISCARD);

  HRLightRef  light = hrLightCreate(L"testPointLight1");

  // simple light example; the container will be translated to point light;
  //
  hrLightOpen(light, HR_WRITE_DISCARD);
  {
    xml_node lightNode = hrLightParamNode(light);

    lightNode.attribute(L"shape").set_value(L"point");                // you don't have to add attributes, only modify them
    lightNode.attribute(L"distribution").set_value(L"onmi");          // you don't have to add attributes, only modify them

    lightNode.append_child(L"position").append_attribute(L"val") = L"0 10 0";

    xml_node intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").append_attribute(L"val")      = L"10 10 10";
    intensityNode.append_child(L"multiplier").append_attribute(L"val") = 2.0f*IRRADIANCE_TO_RADIANCE;
  }
  hrLightClose(light);

  HRCameraRef cam = hrCameraCreate(L"MyCamera");

  hrCameraOpen(cam, HR_WRITE_DISCARD);
  {
    xml_node camNode = hrCameraParamNode(cam);

    camNode.append_child(L"fov").text().set(L"45");
    camNode.append_child(L"nearClipPlane").text().set(L"0.0254");
    camNode.append_child(L"farClipPlane").text().set(L"25.4");

    camNode.append_child(L"up").text().set(L"0 1 0");
    camNode.append_child(L"position").text().set(L"0 0 4.50035");
    camNode.append_child(L"look_at").text().set(L"0 1.62921e-005 -95.4996");
  }
  hrCameraClose(cam);

  hrFlush();

  return check_test_07(L"tests/test_07/statex_00001.xml") && check_test_07(L"tests/test_07/change_00000.xml");
}


bool check_test_08_1(const wchar_t* a_fileName)
{
  pugi::xml_document doc;
  doc.load_file(a_fileName);

  pugi::xml_node libLights = doc.child(L"cam_lib");

  pugi::xml_node c1 = libLights.find_child_by_attribute(L"id", L"0");

  bool ok1 = std::wstring(c1.child(L"position").text().get()) == L"0 0 4.50035";
  bool ok2 = std::wstring(c1.child(L"look_at").text().get()) == L"0 1.62921e-005 -95.4996";

  return ok1 && ok2;
}


bool check_test_08_2(const wchar_t* a_fileName)
{
  pugi::xml_document doc;
  doc.load_file(a_fileName);

  pugi::xml_node libLights = doc.child(L"cam_lib");

  pugi::xml_node c1 = libLights.find_child_by_attribute(L"id", L"0");

  bool ok1 = std::wstring(c1.child(L"position").text().get()) == L"1 1 1";
  bool ok2 = std::wstring(c1.child(L"look_at").text().get()) == L"0 -1 0";

  return ok1 && ok2;
}


bool test08_camera_add_change()
{
  hrSceneLibraryOpen(L"tests/test_08", HR_WRITE_DISCARD);

  HRLightRef  light = hrLightCreate(L"testPointLight1");

  // simple light example; the container will be translated to point light;
  //
  hrLightOpen(light, HR_WRITE_DISCARD);
  {
    xml_node lightNode = hrLightParamNode(light);

    lightNode.attribute(L"shape").set_value(L"point");                // you don't have to add attributes, only modify them
    lightNode.attribute(L"distribution").set_value(L"onmi");          // you don't have to add attributes, only modify them

    lightNode.append_child(L"position").text().set(L"0 10 0");

    xml_node intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").text().set(L"10 10 10");
    intensityNode.append_child(L"multiplier").text().set(L"2");
  }
  hrLightClose(light);

  HRCameraRef cam = hrCameraCreate(L"MyCamera");

  hrCameraOpen(cam, HR_WRITE_DISCARD);
  {
    xml_node camNode = hrCameraParamNode(cam);

    camNode.append_child(L"fov").text().set(L"45");
    camNode.append_child(L"nearClipPlane").text().set(L"0.0254");
    camNode.append_child(L"farClipPlane").text().set(L"25.4");

    camNode.append_child(L"up").text().set(L"0 1 0");
    camNode.append_child(L"position").text().set(L"0 0 4.50035");
    camNode.append_child(L"look_at").text().set(L"0 1.62921e-005 -95.4996");
  }
  hrCameraClose(cam);

  hrFlush();

  hrCameraOpen(cam, HR_OPEN_EXISTING);
  {
    xml_node camNode = hrCameraParamNode(cam);

    camNode.child(L"up").text().set(L"0 1 0");
    camNode.child(L"position").text().set(L"10 10 10");
    camNode.child(L"look_at").text().set(L"20 20 20");
  }
  hrCameraClose(cam);

  hrCameraOpen(cam, HR_OPEN_EXISTING);
  {
    xml_node camNode = hrCameraParamNode(cam);

    camNode.child(L"up").text().set(L"0 1 0");
    camNode.child(L"position").text().set(L"1 1 1");
    camNode.child(L"look_at").text().set(L"0 -1 0");
  }
  hrCameraClose(cam);

  hrFlush();

  bool noDups1 = check_all_duplicates(L"tests/test_08/statex_00001.xml");
  bool noDups2 = check_all_duplicates(L"tests/test_08/statex_00002.xml");

  return check_test_08_1(L"tests/test_08/change_00000.xml") && check_test_08_1(L"tests/test_08/statex_00001.xml") &&
         check_test_08_2(L"tests/test_08/change_00001.xml") && check_test_08_2(L"tests/test_08/statex_00002.xml") && noDups1 && noDups2;
}


bool check_test_09()
{

  bool ok3 = false, ok4 = false;
  {
    pugi::xml_document doc;
    doc.load_file(L"tests/test_09/statex_00001.xml");

    pugi::xml_node libLights = doc.child(L"render_lib");

    pugi::xml_node c1 = libLights.find_child_by_attribute(L"id", L"0");

    ok3 = std::wstring(c1.child(L"width").text().get()) == L"1024";
    ok4 = std::wstring(c1.child(L"height").text().get()) == L"768";
  }

  pugi::xml_document doc;
  doc.load_file(L"tests/test_09/statex_00002.xml");

  pugi::xml_node libLights = doc.child(L"render_lib");

  pugi::xml_node c1 = libLights.find_child_by_attribute(L"id", L"0");

  bool ok1 = std::wstring(c1.child(L"width").text().get()) == L"768";
  bool ok2 = std::wstring(c1.child(L"height").text().get()) == L"768";

  return ok1 && ok2 && ok3 && ok4;
}


bool test09_render_ogl()
{
  hrSceneLibraryOpen(L"tests/test_09", HR_WRITE_DISCARD);

  HRLightRef  light = hrLightCreate(L"testPointLight1");

  // simple light example; the container will be translated to point light;
  //
  hrLightOpen(light, HR_WRITE_DISCARD);
  {
    xml_node lightNode = hrLightParamNode(light);

    lightNode.attribute(L"shape").set_value(L"point");                // you don't have to add attributes, only modify them
    lightNode.attribute(L"distribution").set_value(L"onmi");          // you don't have to add attributes, only modify them

    lightNode.append_child(L"position").text().set(L"0 10 0");

    xml_node intensityNode = lightNode.append_child(L"intensity");

    intensityNode.append_child(L"color").text().set(L"10 10 10");
    intensityNode.append_child(L"multiplier").text().set(L"2");
  }
  hrLightClose(light);

  // camera
  //
  HRCameraRef cam = hrCameraCreate(L"MyCamera");

  hrCameraOpen(cam, HR_WRITE_DISCARD);
  {
    xml_node camNode = hrCameraParamNode(cam);

    camNode.append_child(L"fov").text().set(L"45");
    camNode.append_child(L"nearClipPlane").text().set(L"0.0254");
    camNode.append_child(L"farClipPlane").text().set(L"25.4");

    camNode.append_child(L"up").text().set(L"0 1 0");
    camNode.append_child(L"position").text().set(L"0 0 4.50035");
    camNode.append_child(L"look_at").text().set(L"0 1.62921e-005 -95.4996");
  }
  hrCameraClose(cam);

  // render
  //
  HRRenderRef settingsRef = hrRenderCreate(L"opengl1");

  hrRenderOpen(settingsRef, HR_WRITE_DISCARD);
  {
    pugi::xml_node node = hrRenderParamNode(settingsRef);

    node.append_child(L"width").text().set(L"1024");
    node.append_child(L"height").text().set(L"768");
  }
  hrRenderClose(settingsRef);

  //
  //
  hrFlush();

  hrCameraOpen(cam, HR_OPEN_EXISTING);
  {
    xml_node camNode = hrCameraParamNode(cam);

    camNode.child(L"up").text().set(L"0 1 0");
    camNode.child(L"position").text().set(L"10 10 10");
    camNode.child(L"look_at").text().set(L"20 20 20");
  }
  hrCameraClose(cam);

  hrCameraOpen(cam, HR_OPEN_EXISTING);
  {
    xml_node camNode = hrCameraParamNode(cam);

    camNode.child(L"up").text().set(L"0 1 0");
    camNode.child(L"position").text().set(L"1 1 1");
    camNode.child(L"look_at").text().set(L"0 -1 0");
  }
  hrCameraClose(cam);

  hrRenderOpen(settingsRef, HR_OPEN_EXISTING);
  {
    pugi::xml_node node = hrRenderParamNode(settingsRef);
    node.child(L"width").text().set(L"768");
  }
  hrRenderClose(settingsRef);

  hrFlush();

  return check_test_09();
}


bool check_test_16()
{
	pugi::xml_document doc;
	doc.load_file(L"tests/test_16/statex_00002.xml");

	pugi::xml_node libMat = doc.child(L"materials_lib");
	pugi::xml_node libTex1 = doc.child(L"textures_lib");

	pugi::xml_node m1 = libMat.find_child_by_attribute(L"id", L"0");
	pugi::xml_node m2 = libMat.find_child_by_attribute(L"id", L"1");

	bool tex1_id_ok    = std::wstring(m1.child(L"diffuse").child(L"texture").attribute(L"id").value()) == L"2";
	bool tex2_id_ok    = std::wstring(m2.child(L"reflect").child(L"texture").attribute(L"id").value()) == L"1";
	bool del_tex_id_ok = std::wstring(m2.child(L"diffuse").child(L"texture").attribute(L"id").value()) == L"";

	pugi::xml_node ts1 = libTex1.find_child_by_attribute(L"id", L"1");
	pugi::xml_node ts2 = libTex1.find_child_by_attribute(L"id", L"2");

	bool tex1_state_name_ok = std::wstring(ts1.attribute(L"name").value()) == L"data/textures/texture1.bmp";
	bool tex2_state_name_ok = std::wstring(ts2.attribute(L"name").value()) == L"data/textures/163.jpg";
	
	pugi::xml_document doc2;
	doc2.load_file(L"tests/test_16/change_00001.xml");

	pugi::xml_node libTex2 = doc2.child(L"textures_lib");

	pugi::xml_node t1 = libTex2.find_child_by_attribute(L"id", L"2");
	pugi::xml_node t2 = libTex2.find_child_by_attribute(L"id", L"1");
	
	bool tex1_name_ok = std::wstring(t1.attribute(L"name").value()) == L"data/textures/163.jpg";
	//bool tex2_name_ok = std::wstring(t2.attribute(L"name").value()) == L"data/textures/texture1.bmp";
	
	return tex1_id_ok && tex2_id_ok && tex1_name_ok && tex1_state_name_ok && tex2_state_name_ok;
}


bool test16_texture_add_change()
{
	hrSceneLibraryOpen(L"tests/test_16", HR_WRITE_DISCARD);
	
	HRMaterialRef mat = hrMaterialCreate(L"MyTestMaterial_1");   
	HRMaterialRef mat2 = hrMaterialCreate(L"MyTestMaterial_2"); 

	hrMaterialOpen(mat, HR_WRITE_DISCARD);
	{
		xml_node matNode = hrMaterialParamNode(mat);

		xml_node diff = matNode.append_child(L"diffuse");

		diff.append_attribute(L"brdf_type").set_value(L"lambert");
		diff.append_child(L"color").text().set(L"0.5 0.5 0.001");
		diff.append_child(L"roughness").text().set(L"0.88");

	}
	hrMaterialClose(mat); 
 
	hrMaterialOpen(mat2, HR_WRITE_DISCARD);
	{
		xml_node matNode = hrMaterialParamNode(mat2);

		xml_node diff = matNode.append_child(L"diffuse");
		xml_node refl = matNode.append_child(L"reflect");

		diff.append_attribute(L"brdf_type").set_value(L"lambert");
		diff.append_child(L"color").text().set(L"0.01 0.33 0.55");
		diff.append_child(L"roughness").text().set(L"0");

		refl.append_attribute(L"brdf_type").set_value(L"microfacet");
		refl.append_child(L"color").text().set(L"0.25 0.25 0.25");
		refl.append_child(L"resnel_IOR").text().set(L"1.5");
		refl.append_child(L"glosiness").text().set(L"0.88");

		HRTextureNodeRef testTex = hrTexture2DCreateFromFile(L"data/textures/texture1.bmp");
		hrTextureBind(testTex, refl);
		hrTextureBind(testTex, diff);
	}
	hrMaterialClose(mat2);

	hrFlush();

	// make changes
	//
	hrMaterialOpen(mat, HR_OPEN_EXISTING);
	{
		xml_node matNode = hrMaterialParamNode(mat);
		xml_node diff = matNode.child(L"diffuse");

		HRTextureNodeRef testTex = hrTexture2DCreateFromFile(L"data/textures/163.jpg");
		hrTextureBind(testTex, diff);
	}
	hrMaterialClose(mat);


	hrMaterialOpen(mat2, HR_WRITE_DISCARD);
	{
		xml_node matNode = hrMaterialParamNode(mat2);

		xml_node diff = matNode.append_child(L"diffuse");
		xml_node refl = matNode.append_child(L"reflect");

		diff.append_attribute(L"brdf_type").set_value(L"lambert");
		diff.append_child(L"color").text().set(L"0.01 0.33 0.55");
		diff.append_child(L"roughness").text().set(L"0");

		refl.append_attribute(L"brdf_type").set_value(L"microfacet");
		refl.append_child(L"color").text().set(L"0.25 0.25 0.25");
		refl.append_child(L"resnel_IOR").text().set(L"1.5");
		refl.append_child(L"glosiness").text().set(L"0.88");

		HRTextureNodeRef testTex = hrTexture2DCreateFromFile(L"data/textures/texture1.bmp");

		hrTextureBind(testTex, refl);
	}
	hrMaterialClose(mat2);

	hrFlush();

  bool noDups1 = check_all_duplicates(L"tests/test_16/statex_00001.xml");
  bool noDups2 = check_all_duplicates(L"tests/test_16/statex_00002.xml");

	return check_test_16() && noDups1 && noDups2;
}


bool check_test_17()
{
	pugi::xml_document doc;
	doc.load_file(L"tests/test_17/change_00000.xml");

	pugi::xml_node libTex = doc.child(L"textures_lib");

	pugi::xml_node t1 = libTex.find_child_by_attribute(L"id", L"1");

	bool tex1_name_ok = std::wstring(t1.attribute(L"name").value()) == L"fu_1";
	bool tex1_type_ok = std::wstring(t1.attribute(L"type").value()) == L"falloff";

	bool colorA_ok = std::wstring(t1.child(L"color_A").text().get()) == L"0.0 1.0 0.0";
	bool colorB_ok = std::wstring(t1.child(L"color_B").text().get()) == L"0.0 0.0 1.0";
	bool someparam1_ok = std::wstring(t1.child(L"some_param1").text().get()) == L"some_value";


	pugi::xml_document doc2;
	doc.load_file(L"tests/test_17/change_00001.xml");

	pugi::xml_node libTex2 = doc.child(L"textures_lib");

	pugi::xml_node t2 = libTex2.find_child_by_attribute(L"id", L"2");

	bool tex2_name_ok = std::wstring(t2.attribute(L"name").value()) == L"fu_1";
	bool tex2_type_ok = std::wstring(t2.attribute(L"type").value()) == L"falloff";

	bool colorA2_ok = std::wstring(t2.child(L"color_A").text().get()) == L"1.0 1.0 0.0";
	bool colorB2_ok = std::wstring(t2.child(L"color_B").text().get()) == L"0.0 1.0 1.0";
	bool someparam2_1_ok = std::wstring(t2.child(L"some_param1").text().get()) == L"some_new_value";
	bool someparam2_2_ok = std::wstring(t2.child(L"some_param2").text().get()) == L"some_new_value";

	return tex1_name_ok && tex1_type_ok && colorA_ok && colorB_ok && someparam1_ok && colorA2_ok && colorB2_ok && someparam2_1_ok && someparam2_2_ok;
}


bool test17_falloff()
{
	hrSceneLibraryOpen(L"tests/test_17", HR_WRITE_DISCARD);


	HRMaterialRef mat = hrMaterialCreate(L"MyTestMaterial_1");

	hrMaterialOpen(mat, HR_WRITE_DISCARD);
	{
		xml_node matNode = hrMaterialParamNode(mat);

		xml_node diff = matNode.append_child(L"diffuse");

		diff.append_attribute(L"brdf_type").set_value(L"lambert");
		diff.append_child(L"color").text().set(L"0.5 0.5 0.001");
		diff.append_child(L"roughness").text().set(L"0.88");

		HRTextureNodeRef testTex = hrTextureCreateAdvanced(L"falloff", L"fu_1");
		hrTextureBind(testTex, diff);

		hrTextureNodeOpen(testTex, HR_WRITE_DISCARD);
		{
			xml_node texNode = hrTextureParamNode(testTex);

			texNode.append_child(L"color_A").text().set(L"0.0 1.0 0.0");
			texNode.append_child(L"color_B").text().set(L"0.0 0.0 1.0");

			texNode.append_child(L"some_param1").text().set(L"some_value");

		}
		hrTextureNodeClose(testTex);

	}
	hrMaterialClose(mat);

	hrFlush();

	// make changes
	//
	hrMaterialOpen(mat, HR_OPEN_EXISTING);
	{
		xml_node matNode = hrMaterialParamNode(mat);
		xml_node diff = matNode.child(L"diffuse");

		HRTextureNodeRef testTex = hrTextureCreateAdvanced(L"falloff", L"fu_1");
		hrTextureBind(testTex, diff);

		hrTextureNodeOpen(testTex, HR_WRITE_DISCARD);
		{
			xml_node texNode = hrTextureParamNode(testTex);

			texNode.append_child(L"color_A").text().set(L"1.0 1.0 0.0");
			texNode.append_child(L"color_B").text().set(L"0.0 1.0 1.0");

			texNode.append_child(L"some_param1").text().set(L"some_new_value");
			texNode.append_child(L"some_param2").text().set(L"some_new_value");
		}
		hrTextureNodeClose(testTex);

	}
	hrMaterialClose(mat);

	hrFlush();

	return check_test_17();
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



void test03_open_scene()
{
  HRMeshRef      cubeRef;
  HRSceneInstRef scnRef;

  uint32_t numberVertices = 24;
  uint32_t numberIndices  = 36;

  float cubeVertices[] =
  {
    -1.0f, -1.0f, -1.0f, +1.0f,
    -1.0f, -1.0f, +1.0f, +1.0f,
    +1.0f, -1.0f, +1.0f, +1.0f,
    +1.0f, -1.0f, -1.0f, +1.0f,
    -1.0f, +1.0f, -1.0f, +1.0f,
    -1.0f, +1.0f, +1.0f, +1.0f,
    +1.0f, +1.0f, +1.0f, +1.0f,
    +1.0f, +1.0f, -1.0f, +1.0f,
    -1.0f, -1.0f, -1.0f, +1.0f,
    -1.0f, +1.0f, -1.0f, +1.0f,
    +1.0f, +1.0f, -1.0f, +1.0f,
    +1.0f, -1.0f, -1.0f, +1.0f,
    -1.0f, -1.0f, +1.0f, +1.0f,
    -1.0f, +1.0f, +1.0f, +1.0f,
    +1.0f, +1.0f, +1.0f, +1.0f,
    +1.0f, -1.0f, +1.0f, +1.0f,
    -1.0f, -1.0f, -1.0f, +1.0f,
    -1.0f, -1.0f, +1.0f, +1.0f,
    -1.0f, +1.0f, +1.0f, +1.0f,
    -1.0f, +1.0f, -1.0f, +1.0f,
    +1.0f, -1.0f, -1.0f, +1.0f,
    +1.0f, -1.0f, +1.0f, +1.0f,
    +1.0f, +1.0f, +1.0f, +1.0f,
    +1.0f, +1.0f, -1.0f, +1.0f
  };

  float cubeNormals[] =
  {
    0.0f, -1.0f, 0.0f, +1.0f,
    0.0f, -1.0f, 0.0f, +1.0f,
    0.0f, -1.0f, 0.0f, +1.0f,
    0.0f, -1.0f, 0.0f, +1.0f,
    0.0f, +1.0f, 0.0f, +1.0f,
    0.0f, +1.0f, 0.0f, +1.0f,
    0.0f, +1.0f, 0.0f, +1.0f,
    0.0f, +1.0f, 0.0f, +1.0f,
    0.0f, 0.0f, -1.0f, +1.0f,
    0.0f, 0.0f, -1.0f, +1.0f,
    0.0f, 0.0f, -1.0f, +1.0f,
    0.0f, 0.0f, -1.0f, +1.0f,
    0.0f, 0.0f, +1.0f, +1.0f,
    0.0f, 0.0f, +1.0f, +1.0f,
    0.0f, 0.0f, +1.0f, +1.0f,
    0.0f, 0.0f, +1.0f, +1.0f,
    -1.0f, 0.0f, 0.0f, +1.0f,
    -1.0f, 0.0f, 0.0f, +1.0f,
    -1.0f, 0.0f, 0.0f, +1.0f,
    -1.0f, 0.0f, 0.0f, +1.0f,
    +1.0f, 0.0f, 0.0f, +1.0f,
    +1.0f, 0.0f, 0.0f, +1.0f,
    +1.0f, 0.0f, 0.0f, +1.0f,
    +1.0f, 0.0f, 0.0f, +1.0f
  };

  float cubeTexCoords[] =
  {
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,
    0.0f, 0.0f,
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
  };

  uint32_t cubeIndices[] =
  {
    0, 2, 1,
    0, 3, 2,
    4, 5, 6,
    4, 6, 7,
    8, 9, 10,
    8, 10, 11,
    12, 15, 14,
    12, 14, 13,
    16, 17, 18,
    16, 18, 19,
    20, 23, 22,
    20, 22, 21
  };

  hrSceneLibraryOpen(L"OpenGL1", HR_WRITE_DISCARD);
  
  cubeRef = hrMeshCreate(L"cube");

  hrMeshOpen(cubeRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
  {
    hrMeshVertexAttribPointer4f(cubeRef, L"pos", cubeVertices);
    hrMeshVertexAttribPointer4f(cubeRef, L"norm", cubeNormals);
    hrMeshVertexAttribPointer2f(cubeRef, L"texcoord", cubeTexCoords);

    hrMeshMaterialId(cubeRef, 0);
    hrMeshAppendTriangles3(cubeRef, numberIndices, (int*)cubeIndices);
  }
  hrMeshClose(cubeRef);

  scnRef = hrSceneCreate(L"my scene");

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  hrSceneOpen(scnRef, HR_WRITE_DISCARD); 
  {
    float matrixRot[16] = {1,0,0,0,
                           0,1,0,0,
                           0,0,1,0,
                           0,0,0,1};

    hrMeshInstance(scnRef, cubeRef, matrixRot); 
  }
  hrSceneClose(scnRef);                         

  hrSceneOpen(scnRef, HR_WRITE_DISCARD); 
  {
    float matrixRot[16] = {1,1,1,1,
                           0,1,0,0,
                           0,0,1,0,
                           0,0,0,1};

    hrMeshInstance(scnRef, cubeRef, matrixRot); 
  }
  hrSceneClose(scnRef);       

  hrSceneOpen(scnRef, HR_OPEN_EXISTING);
  {
    float matrixRot[16] = { 2, 2, 2, 2,
                            0, 1, 0, 0,
                            0, 0, 1, 0,
                            0, 0, 0, 1 };

     float matrixRot2[16] = { 3, 3, 3, 3,
                              0, 1, 0, 0,
                              0, 0, 1, 0,
                              0, 0, 0, 1 };

    hrMeshInstance(scnRef, cubeRef, matrixRot);
    hrMeshInstance(scnRef, cubeRef, matrixRot2);
  }

  hrSceneClose(scnRef);

  hrFlush();

  /*
  hrSceneOpen(scnRef, HR_WRITE_DISCARD);
  {
    float matrixRot[16] = { 1, 0, 0, 0,
      0, 1, 0, 0,
      0, 0, 1, 0,
      0, 0, 0, 1 };

    hrMeshInstance(scnRef, cubeRef, matrixRot);
  }
  hrSceneClose(scnRef);

  hrFlush();
  */
}


float g_MSEOutput = 0.0f;
bool g_testWasIgnored = false;

bool dummy_test() 
{ 
  g_testWasIgnored = true;  
  return false; 
}

void PrintResultSingleTest(const bool a_res, std::ofstream& a_fout, std::string a_nameTest,
  const int a_renderTimeSeconds)
{
  std::cout << "                                      \r";

  std::ostringstream outBuff;

  if (a_res)
  {
    std::string str = "mse = " + std::to_string((int)(g_MSEOutput));
    outBuff << std::setw(60) << std::left << a_nameTest;
    outBuff << std::setw(15) << std::left << "ok.";
    outBuff << std::setw(15) << str;
    outBuff << "Render time: " << a_renderTimeSeconds << " sec.";
  }
  else if (g_testWasIgnored)  
    outBuff << std::setw(60) << std::left << a_nameTest << "skipped.";
  else
  {
    std::string str = "MSE = " + std::to_string((int)(g_MSEOutput));
    outBuff << std::setw(60) << std::left << a_nameTest;
    outBuff << std::setw(15) << std::left << "FAILED!";
    outBuff << std::setw(15) << str;
    outBuff << "Render time: " << a_renderTimeSeconds << " sec.";
  }
  
  std::cout << outBuff.str() << std::endl << std::endl;
  a_fout    << outBuff.str() << std::endl;
  
  g_testWasIgnored = false;
}


struct TestFunc
{
  bool(*func)();
  std::string name;

  TestFunc(bool(*a_func)(), std::string a_name) 
  {
    func = a_func;
    name = a_name;
  }
};


void PrintResultTest(const int a_startTestsId, std::vector<TestFunc>& a_tests, std::ofstream& a_fout,
  std::string a_nameGroupTests)
{  
  using time      = std::chrono::system_clock;
  using timeFloat = std::chrono::duration<float>;

  int failedTests    = 0;
  std::ostringstream outBuff;
  auto now           = time::now();
  auto currData      = time::to_time_t(now);
  int totalRenderSec = 0;

  outBuff << a_nameGroupTests << std::endl;  
  outBuff << std::ctime(&currData);
    
  std::cout << outBuff.str() << std::endl;
  a_fout    << outBuff.str() << std::endl;

  for (int i = a_startTestsId; i < a_tests.size(); ++i)
  {       
    const auto start         = time::now();
    const bool res           = a_tests[i].func();    
    const auto end           = time::now();

    failedTests             += (res == true) ? 0 : 1;
    const timeFloat rendTime = end - start;
    totalRenderSec          += rendTime.count();
    PrintResultSingleTest(res, a_fout, a_tests[i].name, rendTime.count());    
  }
      
  const float rendTimeMinutes     = totalRenderSec / 60.0F;
  const int   rendTimeSec         = (int)((rendTimeMinutes - (int)(rendTimeMinutes)) * 60.0F);

  std::ostringstream outBuff2;

  outBuff2 << std::endl;
  outBuff2 << std::setw(20) << std::left << "Tests complete."   << std::endl;
  outBuff2 << std::setw(20) << std::left << "All render time: " << (int)(rendTimeMinutes) << " min. " << rendTimeSec    << " sec." << std::endl;
  outBuff2 << std::setw(20) << std::left << "Failed tests: "    << failedTests            << " of " << a_tests.size() << " tests." << std::endl;
  outBuff2 << std::setw(20) << std::left << "Failed tests: "    << (int)(failedTests * 100.0F / max(a_tests.size(), 1)) << " %";

  std::cout << outBuff2.str() << std::endl;
  a_fout    << outBuff2.str() << std::endl;
}


void run_all_api_tests(const int startTestId)
{
  std::vector<TestFunc> tests = 
  {
    { &test01_materials_add                               ,"test01_materials_add" },
    { &test02_materials_changes_open_mode                 ,"test02_materials_changes_open_mode" },
    { &test03_lights_add                                  ,"test03_lights_add" },
    { &test04_lights_add_change                           ,"test04_lights_add_change" },
    { &test05_instances_write_discard                     ,"test05_instances_write_discard" },
    { &test06_instances_open_existent                     ,"test06_instances_open_existent" },
    { &test07_camera_add                                  ,"test07_camera_add" },
    { &test08_camera_add_change                           ,"test08_camera_add_change" },
    { &test09_render_ogl                                  ,"test09_render_ogl" },
    { &test10_render_ogl_cube                             ,"test10_render_ogl_cube" },
    { &test11_render_ogl_some_figures                     ,"test11_render_ogl_some_figures" },
    { &test12_render_ogl_100_random_figures               ,"test12_render_ogl_100_random_figures" },
    { &test13_render_ogl_some_figures_diff_mats_prom_ptr  ,"test13_render_ogl_some_figures_diff_mats_prom_ptr" },
    { &test14_bad_material_indices                        ,"test14_bad_material_indices" },
    { &test15_main_scene_and_mat_editor                   ,"test15_main_scene_and_mat_editor" },
    { &test16_texture_add_change                          ,"test16_texture_add_change" },
    { &test17_falloff                                     ,"test17_falloff" },
    { &test18_camera_move                                 ,"test18_camera_move" },
    { &test19_material_change                             ,"test19_material_change" },
    { &test20_mesh_change                                 ,"test20_mesh_change" },
    { &test21_add_same_textures_from_file                 ,"test21_add_same_textures_from_file" },
    { &test22_can_not_load_texture                        ,"test22_can_not_load_texture" },
    { &test23_texture_from_memory                         ,"test23_texture_from_memory" },
    { &test24_many_textures_big_data                      ,"test24_many_textures_big_data" },
    { &test25_many_textures_big_data                      ,"test25_many_textures_big_data" },
    { &test26_many_textures_big_data                      ,"test26_many_textures_big_data" },
    { &test27_many_textures_big_data_from_mem             ,"test27_many_textures_big_data_from_mem" },
    { &test28_compute_normals                             ,"test28_compute_normals" },
    { &test29_many_textures_and_meshes                    ,"test29_many_textures_and_meshes" },
    { &test30_many_textures_and_meshes                    ,"test30_many_textures_and_meshes" },
    { &test31_procedural_texture_LDR                      ,"test31_procedural_texture_LDR" },
    { &test32_procedural_texture_HDR                      ,"test32_procedural_texture_HDR" },
    { &test33_update_from_file                            ,"test33_update_from_file" },
    { &test34_delayed_textures_does_not_exists            ,"test34_delayed_textures_does_not_exists" },
    { &test35_cornell_with_light                          ,"test35_cornell_with_light" },
    { &test36_update_from_memory                          ,"test36_update_from_memory" },
    { &test37_cornell_with_light_different_image_layers   ,"test37_cornell_with_light_different_image_layers" },
    { &test38_save_mesh_and_delayed_load                  ,"test38_save_mesh_and_delayed_load" },
    { &test39_mmlt_or_ibpt                                ,"test39_mmlt_or_ibpt" },
    { &test40_several_changes                             ,"test40_several_changes" },
    { &test41_load_library_basic                          ,"test41_load_library_basic" },
    { &test42_load_mesh_compressed                        ,"test42_load_mesh_compressed" },
    { &test43_test_direct_light                           ,"test43_test_direct_light" },
    { &test44_four_lights_and_compressed_mesh             ,"test44_four_lights_and_compressed_mesh" },
    { &test45_mesh_from_vsgf_opengl_bug_teapot            ,"test45_mesh_from_vsgf_opengl_bug_teapot" },
    { &test46_light_geom_rect                             ,"test46_light_geom_rect" },
    { &test47_light_geom_disk                             ,"test47_light_geom_disk" },
    { &test48_light_geom_sphere                           ,"test48_light_geom_sphere" },
    { &test49_light_geom_disk                             ,"test49_light_geom_disk" },
    { &test50_open_library_several_times                  ,"test50_open_library_several_times" },
    { &dummy_test                                         ,"dummy_test" },                                          // 51
    { &dummy_test                                         ,"dummy_test" },                                          // 52
    { &dummy_test                                         ,"dummy_test" },                                          // 53
    { &dummy_test                                         ,"dummy_test" },                                          // 54
    { &test55_clear_scene                                 ,"test55_clear_scene" },
    { &test56_mesh_change_open_existing                   ,"test56_mesh_change_open_existing" },
    { &test57_single_instance                             ,"test57_single_instance" },
    { &dummy_test                                         ,"dummy_test" },                                          // 58
    { &test59_cornell_water_mlt                           ,"test59_cornell_water_mlt" },
    { &test60_debug_print_and_cant_load_mesh              ,"test60_debug_print_and_cant_load_mesh" },               // 60
    { &test61_cornell_with_light_near_wall_and_glossy_wall,"test61_cornell_with_light_near_wall_and_glossy_wall" },
    { &dummy_test                                         ,"dummy_test" },                                          // 62
    { &dummy_test                                         ,"dummy_test" },                                          // 63
    { &test64_several_changes_light_area                  ,"test64_several_changes_light_area" },
    { &test65_several_changes_light_rect                  ,"test65_several_changes_light_rect" },
    { &test66_fast_render_no_final_update                 ,"test66_fast_render_no_final_update" },
    { &test67_fast_empty_scene                            ,"test67_fast_empty_scene" },
    { &test68_scene_library_file_info                     ,"test68_scene_library_file_info" },
    { &dummy_test                                         ,"dummy_test" },                                          // 69
    { &test70_area_lights16                               ,"test70_area_lights16" },
    { &test71_out_of_memory                               ,"test71_out_of_memory" },
    { &dummy_test                                         ,"dummy_test" },                                          // 72
    { &dummy_test                                         ,"dummy_test" },                                          // 73 
    { &test74_frame_buffer_line                           ,"test74_frame_buffer_line" },
    { &test75_repeated_render                             ,"test75_repeated_render" },
    { &test76_empty_mesh                                  ,"test76_empty_mesh" },
    { &test77_save_gbuffer_layers                         ,"test77_save_gbuffer_layers" },
    { &test78_material_remap_list1                        ,"test78_material_remap_list1" },
    { &test79_material_remap_list2                        ,"test79_material_remap_list2" },
    { &test80_lt_rect_image                               ,"test80_lt_rect_image" },
    { &test81_custom_attributes                           ,"test81_custom_attributes" },
    { &test82_proc_texture                                ,"test82_proc_texture" },
    { &test83_proc_texture2                               ,"test83_proc_texture2" },
    { &test84_proc_texture2                               ,"test84_proc_texture2" },
    { &test85_proc_texture_ao                             ,"test85_proc_texture_ao" },
    { &test86_proc_texture_ao_dirt                        ,"test86_proc_texture_ao_dirt" },
    { &test87_proc_texture_reflect                        ,"test87_proc_texture_reflect" },
    { &test88_proc_texture_convex_rust                    ,"test88_proc_texture_convex_rust" },
    { &test89_proc_texture_dirty                          ,"test89_proc_texture_dirty" },
    { &test90_proc_tex_normalmap                          ,"test90_proc_tex_normalmap" },
    { &test91_proc_tex_bump                               ,"test91_proc_tex_bump" },
    { &test92_proc_tex_bump2                              ,"test92_proc_tex_bump2" },
    { &dummy_test                                         ,"dummy_test" },
    { &dummy_test                                         ,"dummy_test" },
    { &test95_bump                                        ,"test95_bump" },
    { &test96_hexaplanar                                  ,"test96_hexaplanar" },
    { &test97_camera_from_matrices                        ,"test97_camera_from_matrices" },
    { &dummy_test                                         ,"dummy_test" },  // correct implementation of motion blur is not yet finished
    { &test99_triplanar                                   ,"test99_triplanar" }
  };

  int startTestId2 = startTestId - 1;
  if (startTestId2 < 0) 
    startTestId2 = 0;

  std::ofstream fout("z_test_api.txt");

  PrintResultTest(startTestId2, tests, fout, "API tests.");

  fout.close();
  glfwTerminate();
}


void run_all_geo_tests()
{
	using namespace GEO_TESTS;
  std::vector<TestFunc> tests =
  {
    { &test_001_mesh_from_memory,     "test_001_mesh_from_memory" },
    { &test_002_mesh_from_vsgf,       "test_002_mesh_from_vsgf" },
    { &test_003_compute_normals,      "test_003_compute_normals" },
    { &test_004_dof,                  "test_004_dof" },
    { &test_005_instancing,           "test_005_instancing" },
    { &test_006_points_on_mesh,       "test_006_points_on_mesh" },
    { &test_007_import_obj,           "test_007_import_obj" },
    { &test_008_import_obj_w_mtl,     "test_008_import_obj_w_mtl" },
    { &test_009_import_obj_fullscale, "test_009_import_obj_fullscale" }
  };

  std::ofstream fout("z_test_geometry.txt");

  PrintResultTest(0, tests, fout, "Geo tests.");

  fout.close();
}


void run_all_mtl_tests(int a_start)
{
	using namespace MTL_TESTS;
  std::vector<TestFunc> tests =
  {
    { &test_100_diffuse_orennayar               , "test_100_diffuse_orennayar" },
    { &test_101_diffuse_lambert_orbspec_mat01   , "test_101_diffuse_lambert_orbspec_mat01" },
    { &test_102_mirror_orbspec_mat02            , "test_102_mirror_orbspec_mat02" },
    { &test_103_diffuse_texture                 , "test_103_diffuse_texture" },
    { &test_104_reflect_phong_orbspec_mat03     , "test_104_reflect_phong_orbspec_mat03" },
    { &test_105_reflect_torranse_sparrow        , "test_105_reflect_torranse_sparrow" },
    { &test_106_reflect_fresnel_ior             , "test_106_reflect_fresnel_ior" },
    { &test_107_reflect_extrusion               , "test_107_reflect_extrusion" },
    { &test_108_reflect_texture                 , "test_108_reflect_texture" },
    { &test_109_reflect_glossiness_texture      , "test_109_reflect_glossiness_texture" },
    { &test_110_texture_sampler                 , "test_110_texture_sampler" },
    { &test_111_glossiness_texture_sampler      , "test_111_glossiness_texture_sampler" },
    { &test_112_transparency                    , "test_112_transparency" },
    { &test_113_transparency_ior                , "test_113_transparency_ior" },
    { &test_114_transparency_fog                , "test_114_transparency_fog" },
    { &test_115_transparency_fog_mult           , "test_115_transparency_fog_mult" },
    { &test_116_transparency_thin               , "test_116_transparency_thin" },
    { &test_117_transparency_texture            , "test_117_transparency_texture" },
    { &test_118_transparency_glossiness_texture , "test_118_transparency_glossiness_texture" },
    { &test_119_opacity_texture                 , "test_119_opacity_texture" },
    { &test_120_opacity_shadow_matte            , "test_120_opacity_shadow_matte" },
    { &test_121_translucency                    , "test_121_translucency" },
    { &test_122_translucency_texture            , "test_122_translucency_texture" },
    { &test_123_emission                        , "test_123_emission" },
    { &test_124_emission_texture                , "test_124_emission_texture" },
    { &test_125_emission_cast_gi                , "test_125_emission_cast_gi" },
    { &test_126_bump_amount                     , "test_126_bump_amount" },
    { &test_127_normal_map_height               , "test_127_normal_map_height" },
    { &test_128_bump_radius                     , "test_128_bump_radius" },
    { &test_129_phong_energy_fix                , "test_129_phong_energy_fix" },
    { &test_130_bump_invert_normalY             , "test_130_bump_invert_normalY" },
    { &test_131_blend_simple                    , "test_131_blend_simple" },
    { &test_132_blend_recursive                 , "test_132_blend_recursive" },
    { &test_133_emissive_and_diffuse            , "test_133_emissive_and_diffuse" },
    { &test_134_diff_refl_transp                , "test_134_diff_refl_transp" },
    { &test_135_opacity_metal                   , "test_135_opacity_metal" },
    { &test_136_opacity_glass                   , "test_136_opacity_glass" },
    { &test_137_cam_mapped_diffuse              , "test_137_cam_mapped_diffuse" },
    { &test_138_translucency_and_diffuse        , "test_138_translucency_and_diffuse" },
    { &test_139_glass_and_bump                  , "test_139_glass_and_bump" },
    { &test_140_blend_emission                  , "test_140_blend_emission" },
    { &test_141_opacity_smooth                  , "test_141_opacity_smooth" },
    { &dummy_test                               , "dummy_test" },              // 142
    { &dummy_test                               , "dummy_test" },              // 143
    { &dummy_test                               , "dummy_test" },              // 144
    { &dummy_test                               , "dummy_test" },              // 145
    { &dummy_test                               , "dummy_test" },              // 146
    { &dummy_test                               , "dummy_test" },              // 147
    { &dummy_test                               , "dummy_test" },              // 148
    { &dummy_test                               , "dummy_test" },              // 149
    { &test_150_gloss_mirror_cos_div            , "test_150_gloss_mirror_cos_div" },
    { &test_151_gloss_mirror_cos_div2           , "test_151_gloss_mirror_cos_div2" },
    { &test_152_texture_color_replace_mode      , "test_152_texture_color_replace_mode" },
    { &test_153_opacity_shadow_matte_opacity    , "test_153_opacity_shadow_matte_opacity" },
    { &test_154_baked_checker_precomp           , "test_154_baked_checker_precomp" },
    { &test_155_baked_checker_HDR_precomp       , "test_155_baked_checker_HDR_precomp" },
    { &test_156_baked_checker_precomp_update    , "test_156_baked_checker_precomp_update" },
    { &test_157_baked_checker_precomp_remap     , "test_157_baked_checker_precomp_remap" },
    { &test_158_proc_dirt1                      , "test_158_proc_dirt1" },
    { &test_159_proc_dirt2                      , "test_159_proc_dirt2" },
    { &test_160_proc_dirt3                      , "test_160_proc_dirt3" },
    { &test_161_simple_displacement             , "test_161_simple_displacement" },
    { &test_162_shadow_matte_back1              , "test_162_shadow_matte_back1" },
    { &test_163_diffuse_texture_recommended_res , "test_163_diffuse_texture_recommended_res" },
    { &test_164_simple_displacement_proctex     , "test_164_simple_displacement_proctex" },
    { &test_165_simple_displacement_mesh        , "test_165_simple_displacement_mesh" },
    { &test_166_displace_by_noise               , "test_166_displace_by_noise" },
    { &test_167_subdiv                          , "test_167_subdiv" },
    { &test_168_diffuse_texture_recommended_res2, "test_168_diffuse_texture_recommended_res2" },
    { &test_169_displace_custom_callback        , "test_169_displace_custom_callback" },
    { &test_170_fresnel_blend                   , "test_170_fresnel_blend" },
    { &test_171_simple_displacement_triplanar   , "test_171_simple_displacement_triplanar" },
    { &test_172_glossy_dark_edges_phong         , "test_172_glossy_dark_edges_phong" },
    { &test_173_glossy_dark_edges_microfacet    , "test_173_glossy_dark_edges_microfacet" },
    { &dummy_test                               , "dummy_test" },
    { &test_175_beckman_isotropic               , "test_175_beckman_isotropic" },
    { &test_176_beckman_anisotropic             , "test_176_beckman_anisotropic" },
    { &test_177_beckman_aniso_rot               , "test_177_beckman_aniso_rot" },
    { &test_178_trggx_isotropic                 , "test_178_trggx_isotropic" },
    { &test_179_trggx_anisotropic               , "test_179_trggx_anisotropic" },
    { &test_180_trggx_aniso_rot                 , "test_180_trggx_aniso_rot" },
  };

  std::ofstream fout("z_test_materials.txt");

  PrintResultTest(a_start, tests, fout, "Materials tests.");

  fout.close();	
}


void run_all_lgt_tests(int a_start)
{
  using namespace LGHT_TESTS;
  std::vector<TestFunc> tests =
  {
    { &test_200_spot                              , "test_200_spot" },
    { &test_201_sphere                            , "test_201_sphere" },
    { &test_202_sky_color                         , "test_202_sky_color" },
    { &test_203_sky_hdr                           , "test_203_sky_hdr" },
    { &test_204_sky_hdr_rotate                    , "test_204_sky_hdr_rotate" },
    { &test_205_sky_and_directional_sun           , "test_205_sky_and_directional_sun" },
    { &test_206_ies1                              , "test_206_ies1" },
    { &test_207_ies2                              , "test_207_ies2" },
    { &test_208_ies3                              , "test_208_ies3" },
    { &test_209_skyportal                         , "test_209_skyportal" },
    { &test_210_skyportal_hdr                     , "test_210_skyportal_hdr" },
    { &test_211_sky_and_sun_perez                 , "test_211_sky_and_sun_perez" },
    { &test_212_skyportal_sun                     , "test_212_skyportal_sun" },
    { &test_213_point_omni                        , "test_213_point_omni" },
    { &test_214_sky_ldr                           , "test_214_sky_ldr" },
    { &test_215_light_scale_intensity             , "test_215_light_scale_intensity" },
    { &test_216_ies4                              , "test_216_ies4" },
    { &test_217_cylinder                          , "test_217_cylinder" },
    { &test_218_cylinder2                         , "test_218_cylinder2" },
    { &test_219_cylinder_tex                      , "test_219_cylinder_tex" },
    { &test_220_cylinder_tex2                     , "test_220_cylinder_tex2" },
    { &test_221_cylinder_tex3                     , "test_221_cylinder_tex3" },
    { &test_222_cylinder_with_end_face            , "test_222_cylinder_with_end_face" },
    { &test_223_rotated_area_light                , "test_223_rotated_area_light" },
    { &test_224_rotated_area_light2               , "test_224_rotated_area_light2" },
    { &test_225_point_spot_simple                 , "test_225_point_spot_simple" },
    { &test_226_area_spot_simple                  , "test_226_area_spot_simple" },
    { &test_227_point_spot_glossy_wall            , "test_227_point_spot_glossy_wall" },
    { &test_228_point_ies_for_bpt                 , "test_228_point_ies_for_bpt" },
    { &test_229_point_ies_for_bpt                 , "test_229_point_ies_for_bpt" },
    { &test_230_area_ies_for_bpt                  , "test_230_area_ies_for_bpt" },
    { &test_231_direct_soft_shadow                , "test_231_direct_soft_shadow" },
    { &test_232_point_area_ies                    , "test_232_point_area_ies" },
    { &test_233_light_group_point_area_ies        , "test_233_light_group_point_area_ies" },
    { &test_234_light_group_light_inst_cust_params, "test_234_light_group_light_inst_cust_params" },
    { &test_235_stadium                           , "test_235_stadium" },
    { &test_236_light_group_point_area_ies2       , "test_236_light_group_point_area_ies2" },
    { &test_237_cubemap_ldr                       , "test_237_cubemap_ldr" },
    { &test_238_mesh_light_one_triangle           , "test_238_mesh_light_one_triangle" },
    { &test_239_mesh_light_two_triangle           , "test_239_mesh_light_two_triangle" },
    { &test_240_mesh_light_torus                  , "test_240_mesh_light_torus" },
    { &test_241_mesh_light_torus_texture_ldr      , "test_241_mesh_light_torus_texture_ldr" },
    { &test_242_mesh_light_torus_texture_hdr      , "test_242_mesh_light_torus_texture_hdr" },
    { &test_243_mesh_light_do_not_sample_me       , "test_243_mesh_light_do_not_sample_me" },
    { &test_244_do_not_sample_me                  , "test_244_do_not_sample_me" },
    { &test_245_cylinder_tex_nearest              , "test_245_cylinder_tex_nearest" },
  };

  std::ofstream fout("z_test_lights.txt");
  
  PrintResultTest(a_start, tests, fout, "Lights tests.");

  fout.close();
}


void run_all_alg_tests(int a_start)
{
  using namespace ALGR_TESTS;
  std::vector<TestFunc> tests =
  {
    { &test_401_ibpt_and_glossy_glass       , "test_401_ibpt_and_glossy_glass" },
    { &test_402_ibpt_and_glossy_double_glass, "test_402_ibpt_and_glossy_double_glass" },
    { &test_403_light_inside_double_glass   , "test_403_light_inside_double_glass" },
    { &test_404_cornell_glossy              , "test_404_cornell_glossy" },
    { &test_405_cornell_with_mirror         , "test_405_cornell_with_mirror" },
    { &test_406_env_glass_ball_caustic      , "test_406_env_glass_ball_caustic" }
  };
  
  std::ofstream fout("z_test_ralgs.txt");  
  
  PrintResultTest(a_start, tests, fout, "Render algorithm tests.");
  
  fout.close();  
}


void run_all_microfacet_torrance_sparrow()
{
  using namespace LGHT_TESTS;
  using namespace MTL_TESTS;
  
  std::vector<TestFunc> tests =
  {
    { &test_202_sky_color                 , "test_202_sky_color" },
    { &test_203_sky_hdr                   , "test_203_sky_hdr" },
    { &test_204_sky_hdr_rotate            , "test_204_sky_hdr_rotate" },
    { &test_105_reflect_torranse_sparrow  , "test_105_reflect_torranse_sparrow" },
    { &test_107_reflect_extrusion         , "test_107_reflect_extrusion" },
    { &test_108_reflect_texture           , "test_108_reflect_texture" },
    { &test_109_reflect_glossiness_texture, "test_109_reflect_glossiness_texture" },
    { &test_111_glossiness_texture_sampler, "test_111_glossiness_texture_sampler" },
    { &test_130_bump_invert_normalY       , "test_130_bump_invert_normalY" },
    { &test_135_opacity_metal             , "test_135_opacity_metal" },
    { &test_151_gloss_mirror_cos_div2     , "test_151_gloss_mirror_cos_div2" },
    { &test_166_displace_by_noise         , "test_166_displace_by_noise" },
    { &test_169_displace_custom_callback  , "test_169_displace_custom_callback" },
  };
        
  std::ofstream fout("z_microfacet.txt");    

  PrintResultTest(0, tests, fout, "Microfacet Torrance-Sparrow tests.");

  fout.close();
}


void run_all_vector_tex_tests()
{
  using namespace EXTENSIONS_TESTS;
  std::vector<TestFunc> tests =
  {
    { &test_ext_vtex_1, "test_ext_vtex_1" },
    { &test_ext_vtex_2, "test_ext_vtex_2" },
    { &test_ext_vtex_3, "test_ext_vtex_3" },
    { &test_ext_vtex_4, "test_ext_vtex_4" },
    { &test_ext_vtex_5, "test_ext_vtex_5" },
    { &test_ext_vtex_6, "test_ext_vtex_6" },
    { &test_ext_vtex_7, "test_ext_vtex_7" },
    { &test_ext_vtex_8, "test_ext_vtex_8" }
  };

  std::ofstream fout("z_vector_textures.txt");

  PrintResultTest(0, tests, fout, "Vector textures tests.");

  fout.close();
}


bool run_single_3dsmax_test(const std::wstring& a_path)
{
  std::wstring errorPlace = std::wstring(L"run_single_3dsmax_test__") + a_path;
  std::wstring outPath    = std::wstring(L"3dsMaxTests/rendered/")    + a_path.substr(12, a_path.size()) + L".png";
  std::wstring refPath    = std::wstring(L"3dsMaxTests/Reference/")   + a_path.substr(12, a_path.size()) + L".png";

  hrErrorCallerPlace(errorPlace.c_str());
  hrSceneLibraryOpen(a_path.c_str(), HR_OPEN_EXISTING);

  /////////////////////////////////////////////////////////
  HRRenderRef renderRef;
  HRSceneInstRef scnRef;
  renderRef.id = 0;
  scnRef.id    = 0;
  /////////////////////////////////////////////////////////

  hrRenderEnableDevice(renderRef, CURR_RENDER_DEVICE, true);

  hrFlush(scnRef, renderRef);

  TEST_UTILS::RenderProgress(renderRef);

  hrRenderSaveFrameBufferLDR(renderRef, outPath.c_str());

  const std::string outPathS = ws2s(outPath);
  const std::string refPathS = ws2s(refPath);

  int w1, h1, w2, h2;
  std::vector<int32_t> image1, image2;

  bool loaded1 = HydraRender::LoadLDRImageFromFile(outPathS.c_str(), &w1, &h1, image1);
  bool loaded2 = HydraRender::LoadLDRImageFromFile(refPathS.c_str(), &w2, &h2, image2);

  if (w1 != w2 || h1 != h2 || !loaded1 || !loaded2)
  {
    g_MSEOutput = 1000000.0f;
    return false;
  }

  const float mseVal = HydraRender::MSE_RGB_LDR(image1, image2);
  g_MSEOutput        = mseVal;

  //#TODO: clean generated state files !!!
  
  std::vector<std::wstring> files = hr_listfiles(a_path.c_str(), true);
  
  for(const auto& file : files)
  {
    if(file.find(L"statex_00001.xml") == std::wstring::npos)
      hr_fs::deletefile(file.c_str());
  }
  
  return (mseVal <= 50.0f);
}


void run_all_3dsmax_tests(int a_start)
{
  std::vector<std::wstring> files = hr_listfiles(L"3dsMaxTests", false);
  std::vector<std::wstring> filesFiltered; 

  filesFiltered.reserve(files.size());
  std::copy_if(files.begin(), files.end(), std::back_inserter(filesFiltered),
               [](const std::wstring& f) { return (f.size() >= 15 && f.find(L"3dsMaxTests") != std::wstring::npos && f.find(L".max") == std::wstring::npos && iswdigit(f[12 + 0]) && iswdigit(f[12 + 1]) && iswdigit(f[12 + 2])); });
    
  
  std::ofstream fout("z_3dsmax_tests.txt");

  std::ostringstream outBuff;

  auto now      = std::chrono::system_clock::now();
  auto currData = std::chrono::system_clock::to_time_t(now);

  outBuff << "3DS Max tests." << std::endl;
  outBuff << std::ctime(&currData);

  std::cout << outBuff.str() << std::endl;
  fout      << outBuff.str() << std::endl;

  for (int i = a_start; i < filesFiltered.size(); ++i)
  {    
    const auto start = std::chrono::system_clock::now();        

    const std::string name = ws2s(filesFiltered[i]);
    const bool res         = run_single_3dsmax_test(filesFiltered[i]);
    
    const auto end   = std::chrono::system_clock::now();    
    std::chrono::duration<float> rendTime = end - start;
    
    PrintResultSingleTest(res, fout, name.c_str(), rendTime.count());
  }

  fout.close();
}


void terminate_opengl()
{
	glfwTerminate();
}
