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

#include "HR_HDRImageTool.h"
#include "HydraXMLHelpers.h"

#pragma warning(disable:4996)
#pragma warning(disable:4838)
#pragma warning(disable:4244)

extern GLFWwindow* g_window;
using namespace TEST_UTILS;


namespace MTL_TESTS
{
  namespace hlm = LiteMath;
  
	bool test_133_emissive_and_diffuse()
	{
    std::wstring nameTest                = L"test_133";
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

			matNode.append_child(L"diffuse").append_child(L"color").append_attribute(L"val").set_value(L"0.0 0.25 0.0");

			auto emission = matNode.append_child(L"emission");
			emission.append_child(L"multiplier").append_attribute(L"val").set_value(0.75f);

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

			matNode.append_child(L"diffuse").append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 0.0");

			auto emission = matNode.append_child(L"emission");
			emission.append_child(L"multiplier").append_attribute(L"val").set_value(0.65f);

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

			matNode.append_child(L"diffuse").append_child(L"color").append_attribute(L"val").set_value(L"0.4 0.4 0.4");

			auto emission = matNode.append_child(L"emission");
			emission.append_child(L"multiplier").append_attribute(L"val").set_value(0.5f);

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

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Meshes
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		HRMeshRef sph1 = HRMeshFromSimpleMesh(L"sph1", CreateSphere(2.0f, 64), matR.id);
		HRMeshRef sph2 = HRMeshFromSimpleMesh(L"sph2", CreateSphere(2.0f, 64), matG.id);
		HRMeshRef sph3 = HRMeshFromSimpleMesh(L"sph3", CreateSphere(2.0f, 64), matB.id);
		HRMeshRef boxBG = HRMeshFromSimpleMesh(L"boxBG", CreateCube(1.0f), matBG.id);
		HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), matGray.id);

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

			sizeNode.append_attribute(L"half_length").set_value(L"5.0");
			sizeNode.append_attribute(L"half_width").set_value(L"5.0");

			auto intensityNode = lightNode.append_child(L"intensity");

			intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
			intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(2.0*IRRADIANCE_TO_RADIANCE);
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

			camNode.append_child(L"fov").text().set(L"45");
			camNode.append_child(L"nearClipPlane").text().set(L"0.01");
			camNode.append_child(L"farClipPlane").text().set(L"100.0");

			camNode.append_child(L"up").text().set(L"0 1 0");
			camNode.append_child(L"position").text().set(L"0 1 16");
			camNode.append_child(L"look_at").text().set(L"0 1 0");
		}
		hrCameraClose(camRef);

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Render settings
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 512, 512, 256, 2048);


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

		const float DEG_TO_RAD = float(3.14159265358979323846f) / 180.0f;

		mTranslate = translate4x4(float3(0.0f, 1.0f, -4.0f));
		mScale = scale4x4(float3(16.0f, 8.0f, 0.5f));
		mRes = mul(mScale, mRes);
		// mRes = mul(mRot, mRes);
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

    return check_images(ws2s(nameTest).c_str());
  }


	bool test_134_diff_refl_transp()
	{
    std::wstring nameTest                = L"test_134";
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
			diff.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 0.0");

			auto refl = matNode.append_child(L"reflectivity");
			
			refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
			refl.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
			refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.95");
			refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
			refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
			refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(2.5);

			auto transp = matNode.append_child(L"transparency");

      transp.append_attribute(L"brdf_type").set_value(L"phong");
      transp.append_child(L"color").append_attribute(L"val").set_value(L"0.75 0.75 0.75");
      transp.append_child(L"glossiness").append_attribute(L"val").set_value(1.0);
      transp.append_child(L"thin_walled").append_attribute(L"val").set_value(0);
      transp.append_child(L"fog_color").append_attribute(L"val").set_value(L"1.0 0.0 0.0");
      transp.append_child(L"fog_multiplier").append_attribute(L"val").set_value(1.0f);
      transp.append_child(L"ior").append_attribute(L"val").set_value(1.75f);

			VERIFY_XML(matNode);
		}
		hrMaterialClose(matMaxcolor);

		hrMaterialOpen(matLuminance, HR_WRITE_DISCARD);
		{
			auto matNode = hrMaterialParamNode(matLuminance);

			auto refl = matNode.append_child(L"reflectivity");

			refl.append_attribute(L"brdf_type").set_value(L"phong");
			refl.append_child(L"color").append_attribute(L"val").set_value(L"0.75 0.75 0.75");
			refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"1.0");
			refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
			refl.append_child(L"fresnel").append_attribute(L"val").set_value(0);

			VERIFY_XML(matNode);
		}
		hrMaterialClose(matLuminance);

		hrMaterialOpen(matColored, HR_WRITE_DISCARD);
		{
			auto matNode = hrMaterialParamNode(matColored);

			auto diff = matNode.append_child(L"diffuse");

			diff.append_attribute(L"brdf_type").set_value(L"lambert");
			diff.append_child(L"color").append_attribute(L"val").set_value(L"0.0 0.0 0.65");

			auto refl = matNode.append_child(L"reflectivity");

			refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
			refl.append_child(L"color").append_attribute(L"val").set_value(L"0.0 0.25 0.0");
			refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"1.0");
			refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
			refl.append_child(L"fresnel").append_attribute(L"val").set_value(0);

			auto transp = matNode.append_child(L"transparency");

			transp.append_attribute(L"brdf_type").set_value(L"phong");
			transp.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 0.5");
			transp.append_child(L"glossiness").append_attribute(L"val").set_value(1.0);
			transp.append_child(L"thin_walled").append_attribute(L"val").set_value(0);
			transp.append_child(L"fog_color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
			transp.append_child(L"fog_multiplier").append_attribute(L"val").set_value(1.0f);
			transp.append_child(L"ior").append_attribute(L"val").set_value(1.75f);

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

		HRTextureNodeRef texChecker = hrTexture2DCreateFromFile(L"data/textures/chess_white.bmp");
		HRMaterialRef matBG = hrMaterialCreate(L"matBG");

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

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Meshes
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		HRMeshRef sphereMaxcolor = HRMeshFromSimpleMesh(L"sphereMaxcolor", CreateSphere(2.0f, 64), matMaxcolor.id);
		HRMeshRef sphereLum      = HRMeshFromSimpleMesh(L"sphereLum",      CreateSphere(2.0f, 64), matLuminance.id);
		HRMeshRef sphereColored  = HRMeshFromSimpleMesh(L"sphereColored",  CreateSphere(2.0f, 64), matColored.id);
		HRMeshRef planeRef       = HRMeshFromSimpleMesh(L"my_plane",       CreatePlane(10.0f),     matGray.id);
		HRMeshRef boxBG          = HRMeshFromSimpleMesh(L"boxBG",          CreateCube(1.0f),       matBG.id);

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

			sizeNode.append_attribute(L"half_length").set_value(L"5");
			sizeNode.append_attribute(L"half_width").set_value(L"5");

			auto intensityNode = lightNode.append_child(L"intensity");

			intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
			intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(4.0*IRRADIANCE_TO_RADIANCE);
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

			camNode.append_child(L"fov").text().set(L"45");
			camNode.append_child(L"nearClipPlane").text().set(L"0.01");
			camNode.append_child(L"farClipPlane").text().set(L"100.0");

			camNode.append_child(L"up").text().set(L"0 1 0");
			camNode.append_child(L"position").text().set(L"0 0 12");
			camNode.append_child(L"look_at").text().set(L"0 0 0");
		}
		hrCameraClose(camRef);

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Render settings
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 1024, 768, 256, 2048);

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Create scene
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
		{
			LiteMath::float4x4 mTranslate, mRes, mRot, mScale;

			const float DEG_TO_RAD = float(3.14159265358979323846f) / 180.0f;

			mTranslate = LiteMath::translate4x4(LiteMath::float3(0.0f, 1.0f, -4.0f));
			mScale     = LiteMath::scale4x4(LiteMath::float3(16.0f, 8.0f, 0.5f));
			mRes       = mul(mScale, mRes);
			mRes       = mul(mTranslate, mRes);

			hrMeshInstance(scnRef, boxBG, mRes.L());
		}

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


  bool test_135_opacity_metal()
  {
    std::wstring nameTest                = L"test_135";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);
    
    ////////////////////
    // Materials
    ////////////////////

    HRMaterialRef matMirror     = hrMaterialCreate(L"matMirror");
    HRMaterialRef matMetalPhong = hrMaterialCreate(L"matMetalPhong");
    HRMaterialRef matMetalMF    = hrMaterialCreate(L"matMetalMF");
    HRMaterialRef matBG         = hrMaterialCreate(L"matBG");
    HRMaterialRef matGray       = hrMaterialCreate(L"matGray");

    HRTextureNodeRef texChecker = hrTexture2DCreateFromFile(L"data/textures/chess_white.bmp");
    HRTextureNodeRef texPattern = hrTexture2DCreateFromFile(L"data/textures/blur_pattern.bmp");

    hrMaterialOpen(matMirror, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matMirror);

      auto refl = matNode.append_child(L"reflectivity");

      refl.append_attribute(L"brdf_type").set_value(L"phong");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"1.0");
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(0);
      refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(1.5);

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
    hrMaterialClose(matMirror);

    hrMaterialOpen(matMetalPhong, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matMetalPhong);

      auto refl = matNode.append_child(L"reflectivity");

      refl.append_attribute(L"brdf_type").set_value(L"phong");
      refl.append_child(L"color").append_attribute(L"val")       = L"0.8 0.8 0.8";
      refl.append_child(L"glossiness").append_attribute(L"val")  = L"0.8";
      refl.append_child(L"extrusion").append_attribute(L"val")   = L"maxcolor";
      refl.append_child(L"fresnel").append_attribute(L"val")     = 1;
      refl.append_child(L"fresnel_ior").append_attribute(L"val") = 50;
      refl.append_child(L"energy_fix").append_attribute(L"val")  = 1;

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
    hrMaterialClose(matMetalPhong);

    hrMaterialOpen(matMetalMF, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matMetalMF);

      auto refl = matNode.append_child(L"reflectivity");

      refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"0.8 0.8 0.8");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.8");
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(50);

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
    hrMaterialClose(matMetalMF);

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

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Meshes
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    HRMeshRef sph1 = HRMeshFromSimpleMesh(L"sph1", CreateSphere(2.0f, 64), matMirror.id);
    HRMeshRef sph2 = HRMeshFromSimpleMesh(L"sph2", CreateSphere(2.0f, 64), matMetalPhong.id);
    HRMeshRef sph3 = HRMeshFromSimpleMesh(L"sph3", CreateSphere(2.0f, 64), matMetalMF.id);
    HRMeshRef boxBG = HRMeshFromSimpleMesh(L"boxBG", CreateCube(1.0f), matBG.id);
    HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), matGray.id);

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

      sizeNode.append_attribute(L"half_length").set_value(L"10.0");
      sizeNode.append_attribute(L"half_width").set_value(L"10.0");

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(2.0f*IRRADIANCE_TO_RADIANCE);
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

      camNode.append_child(L"fov").text().set(L"45");
      camNode.append_child(L"nearClipPlane").text().set(L"0.01");
      camNode.append_child(L"farClipPlane").text().set(L"100.0");

      camNode.append_child(L"up").text().set(L"0 1 0");
      camNode.append_child(L"position").text().set(L"0 1 16");
      camNode.append_child(L"look_at").text().set(L"0 1 0");
    }
    hrCameraClose(camRef);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Render settings
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 512, 512, 256, 2048);
    hrRenderOpen(renderRef, HR_OPEN_EXISTING);
    {
      auto settingsNode = hrRenderParamNode(renderRef);
      settingsNode.force_child(L"method_caustic").text() = L"none";
    }
    hrRenderClose(renderRef);

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

    return check_images(ws2s(nameTest).c_str(), 1, 20);
  }


  bool test_136_opacity_glass()
  {
    std::wstring nameTest                = L"test_136";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);
    
    ////////////////////
    // Materials
    ////////////////////

    HRMaterialRef matGlass                 = hrMaterialCreate(L"matGlass");
    HRMaterialRef matGlassTransluc         = hrMaterialCreate(L"matGlassTransluc");
    HRMaterialRef matDiffGlassTranslucBump = hrMaterialCreate(L"matDiffGlassTranslucBump");
    HRMaterialRef matBG                    = hrMaterialCreate(L"matBG");
    HRMaterialRef matGray                  = hrMaterialCreate(L"matGray");

    HRTextureNodeRef texChecker  = hrTexture2DCreateFromFile(L"data/textures/chess_white.bmp");
    HRTextureNodeRef texPattern  = hrTexture2DCreateFromFile(L"data/textures/blur_pattern.bmp");
    HRTextureNodeRef texOrnament = hrTexture2DCreateFromFile(L"data/textures/ornament.jpg");


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
      transp.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      transp.append_child(L"glossiness").append_attribute(L"val").set_value(1.0f);
      transp.append_child(L"thin_walled").append_attribute(L"val").set_value(0);
      transp.append_child(L"fog_color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      transp.append_child(L"fog_multiplier").append_attribute(L"val").set_value(1.0f);
      transp.append_child(L"ior").append_attribute(L"val").set_value(1.5f);

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
    hrMaterialClose(matGlass);

    hrMaterialOpen(matGlassTransluc, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGlassTransluc);

      auto refl = matNode.append_child(L"reflectivity");

      refl.append_attribute(L"brdf_type").set_value(L"phong");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(1.0f);
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(1.5f);

      auto transp = matNode.append_child(L"transparency");

      transp.append_attribute(L"brdf_type").set_value(L"phong");
      transp.append_child(L"color").append_attribute(L"val").set_value(L"0.6 0.7 0.8");
      transp.append_child(L"glossiness").append_attribute(L"val").set_value(1.0f);
      transp.append_child(L"thin_walled").append_attribute(L"val").set_value(0);
      transp.append_child(L"fog_color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      transp.append_child(L"fog_multiplier").append_attribute(L"val").set_value(1.0f);
      transp.append_child(L"ior").append_attribute(L"val").set_value(1.5f);

      auto transl = matNode.append_child(L"translucency");
      transl.append_child(L"color").append_attribute(L"val").set_value(L"0.2 0.3 0.5");

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
    hrMaterialClose(matGlassTransluc);

    hrMaterialOpen(matDiffGlassTranslucBump, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matDiffGlassTranslucBump);

      auto diffuse = matNode.append_child(L"diffuse");
      diffuse.append_child(L"color").append_attribute(L"val").set_value(L"0.2 0.3 0.5");

      auto refl = matNode.append_child(L"reflectivity");

      refl.append_attribute(L"brdf_type").set_value(L"phong");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(1.0f);
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(1.5f);

      auto transp = matNode.append_child(L"transparency");

      transp.append_attribute(L"brdf_type").set_value(L"phong");
      transp.append_child(L"color").append_attribute(L"val").set_value(L"0.6 0.7 0.8");
      transp.append_child(L"glossiness").append_attribute(L"val").set_value(1.0f);
      transp.append_child(L"thin_walled").append_attribute(L"val").set_value(0);
      transp.append_child(L"fog_color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      transp.append_child(L"fog_multiplier").append_attribute(L"val").set_value(1.0f);
      transp.append_child(L"ior").append_attribute(L"val").set_value(1.5f);

      auto transl = matNode.append_child(L"translucency");
      transl.append_child(L"color").append_attribute(L"val").set_value(L"0.2 0.3 0.5");

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

      auto displacement = matNode.append_child(L"displacement");
      auto heightNode = displacement.append_child(L"height_map");

      displacement.append_attribute(L"type").set_value(L"height_bump");
      heightNode.append_attribute(L"amount").set_value(1.0f);

      auto texNode2 = hrTextureBind(texOrnament, heightNode);

      texNode2.append_attribute(L"matrix");
      float samplerMatrix2[16] = { 2, 0, 0, 0,
                                   0, 2, 0, 0,
                                   0, 0, 1, 0,
                                   0, 0, 0, 1 };

      texNode2.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode2.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode2.append_attribute(L"input_gamma").set_value(2.2f);
      texNode2.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode2, L"matrix", samplerMatrix2);

      VERIFY_XML(matNode);
    }
    hrMaterialClose(matDiffGlassTranslucBump);

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

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Meshes
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    HRMeshRef sph1 = HRMeshFromSimpleMesh(L"sph1", CreateSphere(2.0f, 64), matGlass.id);
    HRMeshRef sph2 = HRMeshFromSimpleMesh(L"sph2", CreateSphere(2.0f, 64), matGlassTransluc.id);
    HRMeshRef sph3 = HRMeshFromSimpleMesh(L"sph3", CreateSphere(2.0f, 64), matDiffGlassTranslucBump.id);
    HRMeshRef boxBG = HRMeshFromSimpleMesh(L"boxBG", CreateCube(1.0f), matBG.id);
    HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), matGray.id);

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

      sizeNode.append_attribute(L"half_length").set_value(L"10.0");
      sizeNode.append_attribute(L"half_width").set_value(L"10.0");

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(2.0f*IRRADIANCE_TO_RADIANCE);
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

      camNode.append_child(L"fov").text().set(L"45");
      camNode.append_child(L"nearClipPlane").text().set(L"0.01");
      camNode.append_child(L"farClipPlane").text().set(L"100.0");

      camNode.append_child(L"up").text().set(L"0 1 0");
      camNode.append_child(L"position").text().set(L"0 1 16");
      camNode.append_child(L"look_at").text().set(L"0 1 0");
    }
    hrCameraClose(camRef);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Render settings
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 512, 512, 256, 2048);

    hrRenderOpen(renderRef, HR_OPEN_EXISTING);
    {
      auto xmlNode = hrRenderParamNode(renderRef);
      xmlNode.force_child(L"method_caustic").text() = L"none";
    }
    hrRenderClose(renderRef);

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

    return check_images(ws2s(nameTest).c_str(), 1, 20);
  }


  bool test_137_cam_mapped_diffuse()
  {
    std::wstring nameTest                = L"test_137";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);
    
    ////////////////////
    // Materials
    ////////////////////
  
    SimpleMesh cube     = CreateCube(1.5f);
    SimpleMesh cubeOpen = CreateCubeOpen(4.0f);
  
    HRMaterialRef mat0 = hrMaterialCreate(L"mysimplemat");
    HRMaterialRef mat1 = hrMaterialCreate(L"red");
    HRMaterialRef mat2 = hrMaterialCreate(L"green");
    HRMaterialRef mat3 = hrMaterialCreate(L"white");
  
    HRMaterialRef mat4 = hrMaterialCreate(L"difftex");
    HRMaterialRef mat5 = hrMaterialCreate(L"difftex_projected");
  
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
  
    HRTextureNodeRef texChecker = hrTexture2DCreateFromFile(L"data/textures/texture1.bmp");
    
    hrMaterialOpen(mat4, HR_WRITE_DISCARD);
    {
      xml_node matNode = hrMaterialParamNode(mat4);
  
      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");
  
      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"1 1 1");
      color.append_attribute(L"tex_apply_mode").set_value(L"multiply");
  
      hrTextureBind(texChecker, color);
  
      auto texNode = color.child(L"texture");
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
    hrMaterialClose(mat4);
  
    hrMaterialOpen(mat5, HR_WRITE_DISCARD);
    {
      xml_node matNode = hrMaterialParamNode(mat5);
    
      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");
    
      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"1 1 1");
      color.append_attribute(L"tex_apply_mode").set_value(L"multiply");
    
      hrTextureBind(texChecker, color);
    
      auto texNode = color.child(L"texture");
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
  
      texNode.append_attribute(L"channel") = L"camera_mapped";
      
      VERIFY_XML(matNode);
    }
    hrMaterialClose(mat5);
  
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  
    HRMeshRef cubeOpenRef = hrMeshCreate(L"my_box");
    HRMeshRef cubeRef     = hrMeshCreate(L"my_cube1");
    HRMeshRef cubeRef2    = hrMeshCreate(L"my_cube2");
  
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
  
    hrMeshOpen(cubeRef, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
    {
      hrMeshVertexAttribPointer4f(cubeRef, L"pos", &cube.vPos[0]);
      hrMeshVertexAttribPointer4f(cubeRef, L"norm", &cube.vNorm[0]);
      hrMeshVertexAttribPointer2f(cubeRef, L"texcoord", &cube.vTexCoord[0]);
    
      for (size_t i = 0; i < cube.matIndices.size(); i++)
        cube.matIndices[i] = mat4.id;
    
      hrMeshPrimitiveAttribPointer1i(cubeRef, L"mind", &cube.matIndices[0]);
      hrMeshAppendTriangles3(cubeRef, int32_t(cube.triIndices.size()), &cube.triIndices[0]);
    }
    hrMeshClose(cubeRef);
  
    hrMeshOpen(cubeRef2, HR_TRIANGLE_IND3, HR_WRITE_DISCARD);
    {
      hrMeshVertexAttribPointer4f(cubeRef2, L"pos", &cube.vPos[0]);
      hrMeshVertexAttribPointer4f(cubeRef2, L"norm", &cube.vNorm[0]);
      hrMeshVertexAttribPointer2f(cubeRef2, L"texcoord", &cube.vTexCoord[0]);
    
      for (size_t i = 0; i < cube.matIndices.size(); i++)
        cube.matIndices[i] = mat5.id;
    
      hrMeshPrimitiveAttribPointer1i(cubeRef2, L"mind", &cube.matIndices[0]);
      hrMeshAppendTriangles3(cubeRef2, int32_t(cube.triIndices.size()), &cube.triIndices[0]);
    }
    hrMeshClose(cubeRef2);
  
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  
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
  
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
  
    hrSceneOpen(scnRef, HR_WRITE_DISCARD);
    {
      // instance sphere and cornell box
      //
      auto mrot1      = hlm::rotate4x4Y(30.0f*DEG_TO_RAD);
      auto mtranslate = hlm::translate4x4(hlm::float3(-1.5f, -2.5f, -1.5f));
      auto mres       = hlm::mul(mtranslate, mrot1);
      hrMeshInstance(scnRef, cubeRef, mres.L());
  
      mrot1      = hlm::rotate4x4Y(-20.0f*DEG_TO_RAD);
      mtranslate = hlm::translate4x4(hlm::float3(2.0f, -2.5f, 1.0f));
      mres       = hlm::mul(mtranslate, mrot1);
      hrMeshInstance(scnRef, cubeRef2, mres.L());
    
      auto mrot = hlm::rotate4x4Y(180.0f*DEG_TO_RAD);
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
    hrRenderSaveFrameBufferLDR(renderRef, saveRenderFile.wstring().c_str());

    return check_images(ws2s(nameTest).c_str(), 1, 25);
  }


  bool test_138_translucency_and_diffuse()
  {
    std::wstring nameTest                = L"test_138";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);
    
    ////////////////////
    // Materials
    ////////////////////

    HRMaterialRef matDiffTrans         = hrMaterialCreate(L"matDiffTrans");
    HRMaterialRef matDiffReflTrans     = hrMaterialCreate(L"matDiffReflTrans");
    HRMaterialRef matDiffReflTransOpac = hrMaterialCreate(L"matDiffReflTransOpac");
    HRMaterialRef matBG                = hrMaterialCreate(L"matBG");
    HRMaterialRef matGray              = hrMaterialCreate(L"matGray");

    HRTextureNodeRef texChecker = hrTexture2DCreateFromFile(L"data/textures/chess_white.bmp");
    HRTextureNodeRef texPattern = hrTexture2DCreateFromFile(L"data/textures/blur_pattern.bmp");
    HRTextureNodeRef texEnv     = hrTexture2DCreateFromFile(L"data/textures/building.hdr");


    hrMaterialOpen(matDiffTrans, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matDiffTrans);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.4 0.4 0.4");

      auto transl = matNode.append_child(L"translucency");
      transl.append_child(L"color").append_attribute(L"val").set_value(L"0.4 0.4 0.4");

      VERIFY_XML(matNode);
    }
    hrMaterialClose(matDiffTrans);

    hrMaterialOpen(matDiffReflTrans, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matDiffReflTrans);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.2 0.3 0.4");

      auto refl = matNode.append_child(L"reflectivity");

      refl.append_attribute(L"brdf_type").set_value(L"phong");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"0.8 0.8 0.8");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.9");
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"luminance");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(1.5);

      auto transl = matNode.append_child(L"translucency");
      transl.append_child(L"color").append_attribute(L"val").set_value(L"0.2 0.3 0.4");

      VERIFY_XML(matNode);
    }
    hrMaterialClose(matDiffReflTrans);

    hrMaterialOpen(matDiffReflTransOpac, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matDiffReflTransOpac);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_attribute(L"brdf_type").set_value(L"lambert");

      auto color = diff.append_child(L"color");
      color.append_attribute(L"val").set_value(L"0.4 0.1 0.1");

      auto refl = matNode.append_child(L"reflectivity");

      refl.append_attribute(L"brdf_type").set_value(L"torranse_sparrow");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"0.8 0.8 0.8");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(L"0.8");
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(1.5);

      auto transl = matNode.append_child(L"translucency");
      transl.append_child(L"color").append_attribute(L"val").set_value(L"0.1 0.4 0.1");

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
    hrMaterialClose(matDiffReflTransOpac);

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
      color.append_attribute(L"val").set_value(L"0.7 0.7 0.7");
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

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Meshes
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    HRMeshRef sph1     = HRMeshFromSimpleMesh(L"sph1", CreateSphere(2.0f, 64), matDiffTrans.id);
    HRMeshRef sph2     = HRMeshFromSimpleMesh(L"sph2", CreateSphere(2.0f, 64), matDiffReflTrans.id);
    HRMeshRef sph3     = HRMeshFromSimpleMesh(L"sph3", CreateSphere(2.0f, 64), matDiffReflTransOpac.id);
    HRMeshRef boxBG    = HRMeshFromSimpleMesh(L"boxBG", CreateCube(1.0f), matBG.id);
    HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), matGray.id);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Light
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    HRLightRef sky = hrLightCreate(L"sky");

    hrLightOpen(sky, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(sky);

      lightNode.attribute(L"type").set_value(L"sky");
      lightNode.attribute(L"distribution").set_value(L"map");

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"0.1");

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

      sizeNode.append_attribute(L"half_length").set_value(L"1.0");
      sizeNode.append_attribute(L"half_width").set_value(L"1.0");

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(1000.0f*IRRADIANCE_TO_RADIANCE);

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

      camNode.append_child(L"fov").text().set(L"45");
      camNode.append_child(L"nearClipPlane").text().set(L"0.01");
      camNode.append_child(L"farClipPlane").text().set(L"100.0");

      camNode.append_child(L"up").text().set(L"0 1 0");
      camNode.append_child(L"position").text().set(L"0 1 16");
      camNode.append_child(L"look_at").text().set(L"0 1 0");
    }
    hrCameraClose(camRef);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Render settings
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 512, 512, 256, 2048);
    hrRenderOpen(renderRef, HR_OPEN_EXISTING);
    {
      auto node = hrRenderParamNode(renderRef);
      node.force_child(L"method_caustic").text() = L"none";
    }
    hrRenderClose(renderRef);

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

    mTranslate = translate4x4(float3(0.0f, -6.0f, 3.0f));
    mScale = scale4x4(float3(16.0f, 8.0f, 0.5f));
    mRot = rotate4x4Y(25.f * DEG_TO_RAD);
    mRes = mul(mScale, mRes);
    mRes = mul(mRot, mRes);
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, boxBG, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();
    mRot.identity();

    mTranslate = translate4x4(float3(10, 15.0f, -20.0));
    mRes = mul(mTranslate, mRes);

    hrLightInstance(scnRef, rectLight, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

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

    return check_images(ws2s(nameTest).c_str(), 1, 15);
  }


  bool test_139_glass_and_bump()
  {
    std::wstring nameTest                = L"test_139";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

    hrErrorCallerPlace(nameTest.c_str());
    hrSceneLibraryOpen(libraryPath.wstring().c_str(), HR_WRITE_DISCARD);
    
    ////////////////////
    // Materials
    ////////////////////

    HRMaterialRef matGlass = hrMaterialCreate(L"matGlass");
    HRMaterialRef matGlassBump = hrMaterialCreate(L"matGlassBump");
    HRMaterialRef matGlassNormalBump = hrMaterialCreate(L"matDiffGlassTranslucBump");
    HRMaterialRef matBG = hrMaterialCreate(L"matBG");
    HRMaterialRef matGray = hrMaterialCreate(L"matGray");

    HRTextureNodeRef texChecker = hrTexture2DCreateFromFile(L"data/textures/chess_white.bmp");
    HRTextureNodeRef texNormal = hrTexture2DCreateFromFile(L"data/textures/normal_map.jpg");
    HRTextureNodeRef texOrnament = hrTexture2DCreateFromFile(L"data/textures/ornament.jpg");
    HRTextureNodeRef texEnv = hrTexture2DCreateFromFile(L"data/textures/building.hdr");


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
      transp.append_child(L"color").append_attribute(L"val").set_value(L"0.6 0.7 0.8");
      transp.append_child(L"glossiness").append_attribute(L"val").set_value(1.0f);
      transp.append_child(L"thin_walled").append_attribute(L"val").set_value(0);
      transp.append_child(L"fog_color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      transp.append_child(L"fog_multiplier").append_attribute(L"val").set_value(1.0f);
      transp.append_child(L"ior").append_attribute(L"val").set_value(1.5f);

      VERIFY_XML(matNode);
    }
    hrMaterialClose(matGlass);

    hrMaterialOpen(matGlassBump, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGlassBump);

      auto refl = matNode.append_child(L"reflectivity");

      refl.append_attribute(L"brdf_type").set_value(L"phong");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(1.0f);
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(1.5f);

      auto transp = matNode.append_child(L"transparency");

      transp.append_attribute(L"brdf_type").set_value(L"phong");
      transp.append_child(L"color").append_attribute(L"val").set_value(L"0.6 0.7 0.8");
      transp.append_child(L"glossiness").append_attribute(L"val").set_value(1.0f);
      transp.append_child(L"thin_walled").append_attribute(L"val").set_value(0);
      transp.append_child(L"fog_color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      transp.append_child(L"fog_multiplier").append_attribute(L"val").set_value(1.0f);
      transp.append_child(L"ior").append_attribute(L"val").set_value(1.5f);

      auto displacement = matNode.append_child(L"displacement");
      auto heightNode = displacement.append_child(L"height_map");

      displacement.append_attribute(L"type").set_value(L"height_bump");
      heightNode.append_attribute(L"amount").set_value(0.5f);

      auto texNode2 = hrTextureBind(texOrnament, heightNode);

      texNode2.append_attribute(L"matrix");
      float samplerMatrix2[16] = { 2, 0, 0, 0,
                                   0, 2, 0, 0,
                                   0, 0, 1, 0,
                                   0, 0, 0, 1 };

      texNode2.append_attribute(L"addressing_mode_u").set_value(L"wrap");
      texNode2.append_attribute(L"addressing_mode_v").set_value(L"wrap");
      texNode2.append_attribute(L"input_gamma").set_value(2.2f);
      texNode2.append_attribute(L"input_alpha").set_value(L"rgb");

      HydraXMLHelpers::WriteMatrix4x4(texNode2, L"matrix", samplerMatrix2);

      VERIFY_XML(matNode);
    }
    hrMaterialClose(matGlassBump);

    hrMaterialOpen(matGlassNormalBump, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matGlassNormalBump);

      auto refl = matNode.append_child(L"reflectivity");

      refl.append_attribute(L"brdf_type").set_value(L"phong");
      refl.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      refl.append_child(L"glossiness").append_attribute(L"val").set_value(1.0f);
      refl.append_child(L"extrusion").append_attribute(L"val").set_value(L"maxcolor");
      refl.append_child(L"fresnel").append_attribute(L"val").set_value(1);
      refl.append_child(L"fresnel_ior").append_attribute(L"val").set_value(1.5f);

      auto transp = matNode.append_child(L"transparency");

      transp.append_attribute(L"brdf_type").set_value(L"phong");
      transp.append_child(L"color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      transp.append_child(L"glossiness").append_attribute(L"val").set_value(1.0f);
      transp.append_child(L"thin_walled").append_attribute(L"val").set_value(0);
      transp.append_child(L"fog_color").append_attribute(L"val").set_value(L"1.0 1.0 1.0");
      transp.append_child(L"fog_multiplier").append_attribute(L"val").set_value(1.0f);
      transp.append_child(L"ior").append_attribute(L"val").set_value(1.5f);

      auto displacement = matNode.append_child(L"displacement");
      auto heightNode = displacement.append_child(L"normal_map");

      displacement.append_attribute(L"type").set_value(L"normal_bump");

      auto invert = heightNode.append_child(L"invert");
      invert.append_attribute(L"x").set_value(0);
      invert.append_attribute(L"y").set_value(0);

      auto texNode = hrTextureBind(texNormal, heightNode);

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
    hrMaterialClose(matGlassNormalBump);

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
      color.append_attribute(L"val").set_value(L"0.7 0.7 0.7");
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

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Meshes
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    HRMeshRef sph1 = HRMeshFromSimpleMesh(L"sph1", CreateSphere(2.0f, 64), matGlass.id);
    HRMeshRef sph2 = HRMeshFromSimpleMesh(L"sph2", CreateSphere(2.0f, 64), matGlassBump.id);
    HRMeshRef sph3 = HRMeshFromSimpleMesh(L"sph3", CreateSphere(2.0f, 64), matGlassNormalBump.id);
    HRMeshRef boxBG = HRMeshFromSimpleMesh(L"boxBG", CreateCube(1.0f), matBG.id);
    HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), matGray.id);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Light
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    HRLightRef sky = hrLightCreate(L"sky");

    hrLightOpen(sky, HR_WRITE_DISCARD);
    {
      auto lightNode = hrLightParamNode(sky);

      lightNode.attribute(L"type").set_value(L"sky");
      lightNode.attribute(L"distribution").set_value(L"map");

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(L"0.1");

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
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(2.0*IRRADIANCE_TO_RADIANCE);

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

    HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 1024, 768, 512, 1024);

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

    mTranslate = translate4x4(float3(0.0f, 0.0f, 1.0f));
    mScale = scale4x4(float3(16.0f, 8.0f, 0.5f));
    mRes = mul(mScale, mRes);
    mRes = mul(mTranslate, mRes);

    hrMeshInstance(scnRef, boxBG, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();
    mRot.identity();

    mTranslate = translate4x4(float3(0, 10.0f, 0.0));
    mRes = mul(mTranslate, mRes);

    hrLightInstance(scnRef, rectLight, mRes.L());

    ///////////

    mTranslate.identity();
    mRes.identity();

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

    return check_images(ws2s(nameTest).c_str(), 1, 65);
  }


  bool test_141_opacity_smooth()
  {
    std::wstring nameTest                = L"test_141";
    std::filesystem::path libraryPath    = L"tests_f/"      + nameTest;
    std::filesystem::path saveRenderFile = L"tests_images/" + nameTest + L"/z_out.png";

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
      diff.append_child(L"color").append_attribute(L"val").set_value(L"1.0 0.5 0.5");

      auto opacity = matNode.append_child(L"opacity");
      opacity.append_attribute(L"smooth").set_value(0);
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
      opacity.append_attribute(L"smooth").set_value(1);
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
    hrMaterialClose(matG);

    hrMaterialOpen(matB, HR_WRITE_DISCARD);
    {
      auto matNode = hrMaterialParamNode(matB);

      auto diff = matNode.append_child(L"diffuse");
      diff.append_child(L"color").append_attribute(L"val").set_value(L"0.5 0.5 1.0");

      auto opacity = matNode.append_child(L"opacity");
      opacity.append_attribute(L"smooth").set_value(1);
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

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Meshes
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    HRMeshRef sph1 = HRMeshFromSimpleMesh(L"sph1", CreateSphere(2.0f, 64), matR.id);
    HRMeshRef sph2 = HRMeshFromSimpleMesh(L"sph2", CreateSphere(2.0f, 64), matG.id);
    HRMeshRef sph3 = HRMeshFromSimpleMesh(L"sph3", CreateSphere(2.0f, 64), matB.id);
    HRMeshRef boxBG = HRMeshFromSimpleMesh(L"boxBG", CreateCube(1.0f), matBG.id);
    HRMeshRef planeRef = HRMeshFromSimpleMesh(L"my_plane", CreatePlane(20.0f), matGray.id);

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

      sizeNode.append_attribute(L"half_length").set_value(L"0.5");
      sizeNode.append_attribute(L"half_width").set_value(L"0.5");

      auto intensityNode = lightNode.append_child(L"intensity");

      intensityNode.append_child(L"color").append_attribute(L"val").set_value(L"1 1 1");
      intensityNode.append_child(L"multiplier").append_attribute(L"val").set_value(400.0*IRRADIANCE_TO_RADIANCE);
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

      camNode.append_child(L"fov").text().set(L"45");
      camNode.append_child(L"nearClipPlane").text().set(L"0.01");
      camNode.append_child(L"farClipPlane").text().set(L"100.0");

      camNode.append_child(L"up").text().set(L"0 1 0");
      camNode.append_child(L"position").text().set(L"0 1 16");
      camNode.append_child(L"look_at").text().set(L"0 1 0");
    }
    hrCameraClose(camRef);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Render settings
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    HRRenderRef renderRef = CreateBasicTestRenderPT(CURR_RENDER_DEVICE, 512, 512, 256, 2048);


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

    return check_images(ws2s(nameTest).c_str());
  }
};