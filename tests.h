#ifndef TESTS_H
#define TESTS_H

#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <filesystem>

#include "HydraAPI.h"
#include "HydraXMLVerify.h"
#include "../hydra_api/HydraXMLHelpers.h"
#include "linmath.h"
#include "LiteMath.h"

using namespace LiteMath;

#include "mesh_utils.h"
#include "simplerandom.h"

///////////////////////////////////////////////////////////////////////////////////

#define IRRADIANCE_TO_RADIANCE (3.14159265358979323846f)

using pugi::xml_node;

void InfoCallBack(const wchar_t* message, const wchar_t* callerPlace, HR_SEVERITY_LEVEL a_level);

inline xml_node force_child(xml_node a_parent, const wchar_t* a_name) ///< helper function
{
  xml_node child = a_parent.child(a_name);
  if (child != nullptr)
    return child;
  else
    return a_parent.append_child(a_name);
}


bool check_images(const char* a_path, const int a_numImages = 1, const float a_mse = 10.0f);
bool check_images_HDR(const char* a_path, const int a_numImages = 1, const float a_mse = 10.0f);
void initGLIfNeeded(int a_width = 512, int a_height = 512, const char* a_name = "glfw demo");

namespace TEST_UTILS
{
  struct SystemInfo
  {
    SystemInfo(const std::wstring& a_operationSystem, const std::wstring& a_videocard);      

    void SetOperSys  (const std::wstring& a_operationSystem);
    void SetVideocard(const std::wstring& a_videocard);

    std::wstring GetFolderNameFromInfo();
    std::wstring GetOsName();
    std::wstring GetVideocardName();

  private:
    std::wstring m_operationSystem;
    std::wstring m_videocard;
  }; 

  inline bool FileExists(const char* a_fileName);

  //images and textures
  void show_me_texture_ldr(const std::string& a_inFleName, const std::string& a_outFleName);
  void show_me_texture_hdr(const std::string& a_inFleName, const std::string& a_outFleName);
  void CreateTestBigTexturesFilesIfNeeded();
  HRTextureNodeRef AddRandomTextureFromMemory(size_t& memTotal, simplerandom::RandomGen& rgen);
  HRTextureNodeRef CreateRandomStrippedTextureFromMemory(size_t& a_byteSize, simplerandom::RandomGen& rgen);
  std::vector<unsigned int> CreateStripedImageData(unsigned int* a_colors, int a_stripsNum, int w, int h);
  void CreateStripedImageFile(const char* a_fileName, unsigned int* a_colors, int a_stripsNum, int w, int h);
  void procTexCheckerHDR(float* a_buffer, int w, int h, void* a_repeat);
  void procTexCheckerLDR(unsigned char* a_buffer, int w, int h, void* a_repeat);

  //displacement
  struct displace_data_1
  {
    float mult = 1.0f;
    float3 global_dir = float3(0.0f, 1.0f, 0.0f);
  };

  void customDisplacement1(const float* pos, const float* normal, const HRUtils::BBox& bbox, float displace_vec[3],
    void* a_customData, uint32_t a_customDataSize);
  void customDisplacementSpots(const float* pos, const float* normal, const HRUtils::BBox& bbox, float displace_vec[3],
    void* a_customData, uint32_t a_customDataSize);

  void customDisplacementFBM(const float* p, const float* normal, const HRUtils::BBox& bbox, float displace_vec[3],
    void* a_customData, uint32_t a_customDataSize);


  //geometry
  std::vector<HRMeshRef> CreateRandomMeshesArray(int a_size, simplerandom::RandomGen& rgen);
  HRMeshRef HRMeshFromSimpleMesh(const wchar_t* a_name, const SimpleMesh& a_mesh, int a_matId);
  HRMeshRef CreateCornellBox(const float a_size, HRMaterialRef a_leftWallMat, HRMaterialRef a_rightWallMat,
    HRMaterialRef a_ceilingMat, HRMaterialRef a_backWallMat, HRMaterialRef a_floorMat);

  //materials
  void AddDiffuseNode(HAPI pugi::xml_node& matNode, const wchar_t* a_diffuseColor,
    const wchar_t* a_brdfType = L"lambert", const float a_roughness = 0,
    HRTextureNodeRef a_texture = HRTextureNodeRef(), const wchar_t* a_addressingModeU = L"wrap",
    const wchar_t* a_addressingModeV = L"wrap", const float tileU = 1, const float tileV = 1,
    const float a_inputGamma = 2.2f, const wchar_t* a_inputAlpha = L"rgb");

  void AddReflectionNode(HAPI pugi::xml_node& matNode, const wchar_t* a_brdfType, const wchar_t* a_color,
    const float a_glossiness, const bool a_fresnel, const float a_ior = 1.5f, 
    const wchar_t* a_extrusion = L"maxcolor", const bool a_energyFix = false);

  void AddReliefNode(HAPI pugi::xml_node& matNode, const wchar_t* a_type, const float a_amount,
    HRTextureNodeRef a_texture = HRTextureNodeRef(), const wchar_t* a_addressingModeU = L"wrap",
    const wchar_t* a_addressingModeV = L"wrap", const float tileU = 1, const float tileV = 1,
    const float a_inputGamma = 2.2f, const wchar_t* a_inputAlpha = L"rgb");

  //light
  HRLightRef CreateLight(const wchar_t* a_name, const wchar_t* a_type, const wchar_t* a_shape,
    const wchar_t* a_distribution, const float a_halfLength, const float a_halfWidth,
    const wchar_t* a_color, const float a_multiplier, const bool a_spot = false, 
    const float a_innerRadius = 40, const float a_outerRadius = 60, const float a_shadowSoft = 0);

  HRLightRef CreateSky(const wchar_t* a_name, const wchar_t* a_color, const float a_multiplier,
    const wchar_t* a_distribution = L"uniform", const int sun_id = 0, const float a_turbidity = 2.0F);
    

  //camera
  void CreateCamera(const float a_fov, const wchar_t* a_position, const wchar_t* a_lookAt,
    const wchar_t* a_name = L"Camera01", const float a_nearClipPlane = 0.01F,
    const float a_farClipPlane = 100.0F, const wchar_t* a_up = L"0 1 0", const bool a_dof = false, 
    const float a_dofLensRadius = 0.1F);

  //scene  
  void AddMeshToScene(HRSceneInstRef& scnRef, HRMeshRef& a_meshRef, const float3 pos = float3(0, 0, 0),
    const float3 rot = float3(0, 0, 0), const float3 scale = float3(1, 1, 1), const int32_t* a_mmListm = nullptr, int32_t a_mmListSize = 0);
  void AddLightToScene(HRSceneInstRef& scnRef, HRLightRef& a_lightRef, const float3 a_pos = float3(0, 0, 0),
    const float3 a_rot = float3(0, 0, 0), const float3 a_scale = float3(1, 1, 1));

  //render
  HRRenderRef CreateBasicTestRenderPT(int a_deviceId, int a_w, int a_h, int a_minRays, int a_maxRays, 
    int a_rayBounce = 6, int a_diffBounce = 4, const wchar_t* a_drvName = L"HydraModern");
  HRRenderRef CreateBasicTestRenderPTNoCaust(int deviceId, int w, int h, int minRays, int maxRays);
  HRRenderRef CreateBasicTestRenderPTFastBackground(int deviceId, int w, int h, int minRays, int maxRays, const wchar_t* a_drvName = L"HydraModern");
  HRRenderRef CreateBasicGLRender(int w, int h);
  HRMeshRef CreateTriStrip(int rows, int cols, float size);

  void RenderProgress(HRRenderRef& a_renderRef);

  //Generate Report

  struct ResultTest
  {
  public:
    ResultTest(const std::wstring a_name, const std::vector<bool> a_res, const bool a_skip, const std::vector<float> a_mse,
      const float a_rendTime, const std::vector<std::wstring>& a_linkRefImg,
      const std::vector<std::wstring>& a_linkRenderImg);
      
    std::wstring GetName()          { return m_nameTest; }
    std::wstring GetMse()           { return m_mse; }
    std::wstring GetMseHtml()       { return m_mseHtml; }
    std::wstring GetStrResult()     { return m_result; }
    std::wstring GetStrResultHtml() { return m_resultHtml; }
    std::wstring GetRendTime()      { return m_renderTime; }
    std::wstring GetLinkRef()       { return m_linkRefImgs; }
    std::wstring GetLinkRend()      { return m_linkRendImgs; }

  private:
    std::wstring m_nameTest     = L"dummy test";
    std::wstring m_mse          = L"";
    std::wstring m_mseHtml      = L"";
    std::wstring m_result       = L"";
    std::wstring m_resultHtml   = L"";
    std::wstring m_renderTime   = L"";
    std::wstring m_linkRefImgs  = L"";
    std::wstring m_linkRendImgs = L"";
  };

  void SetReportFolder(const std::wstring& a_folder);

  void CreateHtml(std::wofstream& a_fileOut);
  void AddTextHtml(const std::wstring& a_text, std::wofstream& a_fileOut, const int a_size);
  void CreateHtmlHeaderTable(const std::vector<std::wstring>& a_header, std::wofstream& a_fileOut);
  void AddRowHtmlTable(ResultTest a_data, std::wofstream& a_fileOut);
  void CloseHtmlTable(std::wofstream& a_fileOut);
  void CloseHtml(std::wofstream& a_fileOut);
}

using DrawFuncType = void (*)();
using InitFuncType = void (*)();

void test02_simple_gl1_render(const wchar_t* a_driverName);
void window_main_free_look(const wchar_t* a_libPath, const wchar_t* a_renderName = L"opengl1", InitFuncType a_pInitFunc = nullptr, DrawFuncType a_pDrawFunc = nullptr);
void test_console_render(const wchar_t* a_libPath, const wchar_t* a_savePath);

void test_device_list();

bool check_all_duplicates(const std::wstring& a_fileName);
bool check_all_attrib_duplicates(const std::wstring& a_fileName);

bool test_001_materials_add();
bool test_002_materials_changes_open_mode();
bool test_003_lights_add();
bool test_004_lights_add_change();
bool test_005_instances_write_discard();
bool test_006_instances_open_existent();
bool test_007_camera_add();
bool test_008_camera_add_change();
bool test_009_render_ogl();

bool test_010_render_ogl_cube();
bool test_011_render_ogl_some_figures();
bool test_012_render_ogl_100_random_figures();
bool test_013_render_ogl_some_figures_diff_mats_prom_ptr();
bool test_014_bad_material_indices();

bool test_015_main_scene_and_mat_editor();
bool test_016_texture_add_change();
bool test_017_falloff();
bool test_018_camera_move();

bool test_019_material_change();
bool test_020_mesh_change();
bool test_021_add_same_textures_from_file();
bool test_022_can_not_load_texture();
bool test_023_texture_from_memory();
bool test_024_many_textures_big_data();
bool test_025_many_textures_big_data();
bool test_026_many_textures_big_data();
bool test_027_many_textures_big_data_from_mem();
bool test_028_compute_normals();
bool test_029_many_textures_and_meshes();
bool test_030_many_textures_and_meshes();
bool test_031_procedural_texture_LDR();
bool test_032_procedural_texture_HDR();
bool test_033_update_from_file();
bool test_034_delayed_textures_does_not_exists();
bool test_035_cornell_with_light();
bool test_036_update_from_memory();
bool test_037_cornell_with_light_different_image_layers();

bool test_038_save_mesh_and_delayed_load();
bool test_039_mmlt_or_ibpt();
bool test_040_several_changes();

bool test_041_load_library_basic();
bool test_042_load_mesh_compressed();

bool test_043_test_direct_light();
bool test_044_four_lights_and_compressed_mesh();
bool test_045_mesh_from_vsgf_opengl_bug_teapot();

bool test_046_light_geom_rect();
bool test_047_light_geom_disk();
bool test_048_light_geom_sphere();
bool test_049_light_geom_disk();
bool test_050_open_library_several_times();
bool test_051_instance_many_trees_and_opacity();
bool test_052_instance_perf_test();
bool test_053_crysponza_perf();
bool test_054_portalsroom_perf();
bool test_055_clear_scene();
bool test_056_mesh_change_open_existing();
bool test_057_single_instance();
bool test_058_crysponza_and_opacity1_perf();
bool test_059_cornell_water_mlt();
bool test_060_debug_print_and_cant_load_mesh();
bool test_061_cornell_with_light_near_wall_and_glossy_wall();

//bool test_062_bad_textures();
bool test_063_cornell_with_caustic_from_torus();
bool test_064_several_changes_light_area();
bool test_065_several_changes_light_rect();

bool test_066_fast_render_no_final_update();
bool test_067_fast_empty_scene();
bool test_068_scene_library_file_info();
bool test_069_obj_delayed_load();
bool test_070_area_lights16();
bool test_071_out_of_memory();
bool test_072_load_library_single_teapot_with_opacity(); // not used, to change it for something useful
bool test_073_big_resolution();                          // not used in automatic mode due to large image size
bool test_074_frame_buffer_line();
bool test_075_repeated_render();
bool test_076_empty_mesh();
bool test_077_save_gbuffer_layers();
bool test_078_material_remap_list1();
bool test_079_material_remap_list2();
bool test_080_lt_rect_image();
bool test_081_custom_attributes();

bool test_082_proc_texture();
bool test_083_proc_texture2();
bool test_084_proc_texture2();
bool test_085_proc_texture_ao();
bool test_086_proc_texture_ao_dirt();
bool test_087_proc_texture_reflect();
bool test_088_proc_texture_convex_rust();
bool test_089_proc_texture_dirty();

bool test_090_proc_tex_normalmap();
bool test_091_proc_tex_bump();
bool test_092_proc_tex_bump2();

bool test_093_proc_tex_recursive();

bool test_093_check_xml_fail_materials();   // not used in automatic mode
bool test_094_check_xml_fail_camera();      // not used in automatic mode

bool test_095_bump();
bool test_096_hexaplanar();
bool test_097_camera_from_matrices();
bool test_098_motion_blur();                // not implemented
bool test_099_triplanar();

bool test_motion_blur();
bool test_mono_framebuff();

bool test100_dummy_hydra_exec();          // not used

namespace GEO_TESTS
{
  bool test_301_mesh_from_memory();
  bool test_302_mesh_from_vsgf();
  bool test_303_compute_normals(); // bug - normals computation
  bool test_304_dof();
  bool test_305_instancing();
  bool test_306_points_on_mesh();
  
  bool test_307_import_obj();
  bool test_308_import_obj_w_mtl();
  bool test_309_import_obj_fullscale();
}

namespace MTL_TESTS
{
  bool test_100_diffuse_orennayar();
  bool test_101_diffuse_lambert_orbspec_mat01();
  bool test_102_mirror_orbspec_mat02();
  bool test_103_diffuse_texture();

  bool test_104_reflect_phong_orbspec_mat03();
  bool test_105_reflect_torranse_sparrow();
  bool test_106_reflect_fresnel_ior();
  bool test_107_reflect_extrusion();
  bool test_108_reflect_texture();
  bool test_109_reflect_glossiness_texture();

  bool test_110_texture_sampler();
  bool test_111_glossiness_texture_sampler();

  bool test_112_transparency();
  bool test_113_transparency_ior();
  bool test_114_transparency_fog();
  bool test_115_transparency_fog_mult();
  bool test_116_transparency_thin();
  bool test_117_transparency_texture();
  bool test_118_transparency_glossiness_texture();

  bool test_119_opacity_texture();
  bool test_120_opacity_shadow_matte(); // need to be redone with new material?

  bool test_121_translucency();
  bool test_122_translucency_texture(); // bug - samplers are ignored

  bool test_123_emission();
  bool test_124_emission_texture();
  bool test_125_emission_cast_gi();

  bool test_126_bump_amount();
  bool test_127_normal_map_height(); 
  bool test_128_bump_radius();
  bool test_129_phong_energy_fix();      
  bool test_130_bump_invert_normalY();

  bool test_131_blend_simple();
  bool test_132_blend_recursive();
  
  bool test_133_emissive_and_diffuse();
  bool test_134_diff_refl_transp();
  
  bool test_135_opacity_metal();         // not smooth backface
  bool test_136_opacity_glass();         // transparency work as thin;
  bool test_137_cam_mapped_diffuse();    //

  bool test_138_translucency_and_diffuse();
  bool test_139_glass_and_bump();
  bool test_140_blend_emission();       // 1) I suggest that the opacity from first mat always cut all the blends. 
                                        // 2) Not visible emission through glass.
  bool test_141_opacity_smooth();
  bool test_142_blend_normalmap_heightmap();

  bool test_150_gloss_mirror_cos_div();
  bool test_151_gloss_mirror_cos_div2();
  bool test_152_texture_color_replace_mode();
  bool test_153_opacity_shadow_matte_opacity();

  bool test_154_baked_checker_precomp();
  bool test_155_baked_checker_HDR_precomp();
  bool test_156_baked_checker_precomp_update();
  bool test_157_baked_checker_precomp_remap();

  bool test_158_proc_dirt1();
  bool test_159_proc_dirt2();
  bool test_160_proc_dirt3();

  bool test_161_simple_displacement();
  bool test_162_shadow_matte_back1();
  bool test_163_diffuse_texture_recommended_res();
  bool test_164_simple_displacement_proctex();
  bool test_165_simple_displacement_mesh();
  bool test_166_displace_by_noise();
  bool test_167_subdiv();
  bool test_168_diffuse_texture_recommended_res2();
  bool test_169_displace_custom_callback();
  bool test_170_fresnel_blend();
  bool test_171_simple_displacement_triplanar();
  bool test_172_glossy_dark_edges_phong();
  bool test_173_glossy_dark_edges_microfacet();
  
  bool test_175_beckman_isotropic();
  bool test_176_beckman_anisotropic();
  bool test_177_beckman_aniso_rot();
  
  bool test_178_trggx_isotropic();
  bool test_179_trggx_anisotropic();
  bool test_180_trggx_aniso_rot();
}

namespace LGHT_TESTS
{
  bool test_200_spot();
  bool test_201_sphere();
  bool test_202_sky_color();
  bool test_203_sky_hdr();
  bool test_204_sky_hdr_rotate();
  bool test_205_sky_and_directional_sun();

  bool test_206_ies1();
  bool test_207_ies2();
  bool test_208_ies3();

  bool test_209_skyportal();
  bool test_210_skyportal_hdr();
  bool test_211_sky_and_sun_perez();
  bool test_212_skyportal_sun();

  bool test_213_point_omni();
  bool test_214_sky_ldr();

  bool test_215_light_scale_intensity();
  bool test_216_ies4();

  bool test_217_cylinder();
  bool test_218_cylinder2();
  bool test_219_cylinder_tex();
  bool test_220_cylinder_tex2();
  bool test_221_cylinder_tex3();
  bool test_222_cylinder_with_end_face();

  bool test_223_rotated_area_light();
  bool test_224_rotated_area_light2();
  bool test_225_point_spot_simple();
  bool test_226_area_spot_simple();
  bool test_227_point_spot_glossy_wall();
  bool test_228_point_ies_for_bpt();
  bool test_229_point_ies_for_bpt();
  bool test_230_area_ies_for_bpt();
  bool test_231_direct_soft_shadow();

  bool test_232_point_area_ies();
  bool test_233_light_group_point_area_ies();
  bool test_234_light_group_light_inst_cust_params();
  bool test_235_stadium();
  bool test_236_light_group_point_area_ies2();
  bool test_237_cubemap_ldr();

  bool test_238_mesh_light_one_triangle();
  bool test_239_mesh_light_two_triangle();
  bool test_240_mesh_light_torus();
  bool test_241_mesh_light_torus_texture_ldr();
  bool test_242_mesh_light_torus_texture_hdr();
  bool test_243_mesh_light_do_not_sample_me();
  bool test_244_do_not_sample_me();
  bool test_245_cylinder_tex_nearest();
}


namespace ALGR_TESTS
{
  bool test_401_ibpt_and_glossy_glass();
  bool test_402_ibpt_and_glossy_double_glass();
  bool test_403_light_inside_double_glass();
  bool test_404_cornell_glossy();
  bool test_405_cornell_with_mirror();
  bool test_406_env_glass_ball_caustic();
}

namespace EXTENSIONS_TESTS
{
  bool test_500_ext_vtex(); // msdf basic
  bool test_501_ext_vtex(); // texture matrix
  bool test_502_ext_vtex(); // outlines
  bool test_503_ext_vtex(); // combine
  bool test_504_ext_vtex(); // rasterize
  bool test_505_ext_vtex(); // overlapping shapes
  bool test_506_ext_vtex(); // blend
  bool test_507_ext_vtex(); // blend 2
  bool test_508_ext_vtex(); // for trying out different configs
}

namespace SPECTRAL_TESTS
{
  bool test_600_cornell_RGB();
  bool test_601_cornell_spectral_2();
  bool test_602_macbeth();
  bool test_603_texture_1();
  bool test_604_tile();
  bool test_605_macbeth_2(); // render 3 wavelengths at a time;
  bool test_606_macbeth_3(); // use hydra api shared image to accumulate average spectrum
  bool test_607_generate_nerf_data();
  bool test_608_virtual_room();
  bool test_609_object_spectral();
  bool test_610_object_spectral_manyviews();
  void test_611_combine_images();
}

void run_all_vector_tex_tests();

//These tests need some scene library to exist in their respective folders
bool test_700_loadlibrary_and_edit();
bool test_701_loadlibrary_and_add_textures();
bool test_702_get_material_by_name_and_edit();
bool test_703_get_light_by_name_and_edit();
bool test_704_get_camera_by_name_and_edit();
bool test_705_transform_all_instances();
bool test_706_transform_all_instances_origin();


bool test_707_merge_library();      // run MTL_TESTS::test_131_blend_simple() first
bool test_708_merge_one_texture();  // run MTL_TESTS::test_131_blend_simple() first
bool test_709_merge_one_material(); // run MTL_TESTS::test_131_blend_simple() first
bool test_710_merge_one_mesh();     // run GEO_TESTS::test_305_instancing() and GEO_TESTS::test_302_mesh_from_vsgf() first
bool test_711_merge_scene();        // run MTL_TESTS::test_131_blend_simple() first
bool test_712_merge_one_light();    // run LGHT_TESTS::test_221_cylinder_tex3();


bool test_713_commit_without_render(); //needs scene library with at least 1 light and 2 materials
bool test_714_print_matlib_map();

bool test_715_merge_scene_with_remaps();
bool test_716_merge_scene_remap_override();
bool test_717_merge_scene_scene_id_mask();

bool test_718_displace_car_by_noise();
bool test_719_car_displacement_triplanar();
bool test_719_car_fresnel_ice();
bool test_720_car_triplanar(const int i);

bool test_721_depth_mesh();

void run_all_api_tests(const int startTestId = 0);
void run_all_geo_tests();
void run_all_mtl_tests(int a_start = 0);
void run_all_lgt_tests(int a_start = 0);
void run_all_alg_tests(int a_start = 0);
void terminate_opengl();

static const int CURR_RENDER_DEVICE = 0;

//void image_p_sandbox();

void run_all_microfacet_torrance_sparrow();
void run_all_3dsmax_tests(int a_start = 0);
bool run_single_3dsmax_test(const std::wstring& a_path);


#endif
