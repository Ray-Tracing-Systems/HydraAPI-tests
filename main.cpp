#include <iostream>
#include <vector>
//#include <zconf.h>

#include "HydraAPI.h"
#include "tests.h"

using pugi::xml_node;

/////////////////////// just leave this it as it is :)

#include "HydraRenderDriverAPI.h"
#include "RenderDrivers.h"

IHRRenderDriver* CreateDriverRTE(const wchar_t* a_cfg) { return nullptr; }

///////////////////////

#if defined WIN32
#include <windows.h> // for SetConsoleCtrlHandler
#else
#include <unistd.h>
#include <signal.h>
#endif

///////////////////////////////////////////////////////////////////////////////////

extern TEST_UTILS::SystemInfo g_systemInfo;



void InfoCallBack(const wchar_t* message, const wchar_t* callerPlace, HR_SEVERITY_LEVEL a_level)
{
  if (a_level >= HR_SEVERITY_WARNING)
  {
    if (a_level == HR_SEVERITY_WARNING)
      std::wcerr << L"WARNING: " << callerPlace << L": " << message; // << std::endl;
    else      
      std::wcerr << L"ERROR  : " << callerPlace << L": " << message; // << std::endl;
  }
}


void destroy()
{
  std::cout << "call destroy() --> hrSceneLibraryClose()" << std::endl;
  hrSceneLibraryClose();
}


#ifdef WIN32
BOOL WINAPI HandlerExit(_In_ DWORD fdwControl)
{
  exit(0);
  return TRUE;
}
#else
bool destroyedBySig = false;
void sig_handler(int signo)
{
  if(destroyedBySig)
    return;
  switch(signo)
  {
    case SIGINT : std::cerr << "\nmain_app, SIGINT";      break;
    case SIGABRT: std::cerr << "\nmain_app, SIGABRT";     break;
    case SIGILL : std::cerr << "\nmain_app, SIGINT";      break;
    case SIGTERM: std::cerr << "\nmain_app, SIGILL";      break;
    case SIGSEGV: std::cerr << "\nmain_app, SIGSEGV";     break;
    case SIGFPE : std::cerr << "\nmain_app, SIGFPE";      break;
    default     : std::cerr << "\nmain_app, SIG_UNKNOWN"; break;
    break;
  }
  std::cerr << " --> hrSceneLibraryClose()" << std::endl;
  hrSceneLibraryClose();
  destroyedBySig = true;
}
#endif



void init()
{
  registerAllGL1Drivers();
//  printAllAvailableDrivers();
}


int main(int argc, const char** argv)
{
  init();
  hrInfoCallback(&InfoCallBack);
  hrErrorCallerPlace(L"main");  // for debug needs only

  std::string workingDir = "..";
  if (argc > 1)
    workingDir = std::string(argv[1]);

  atexit(&destroy);                           // if application will terminated you have to call hrSceneLibraryClose to free all connections with hydra.exe
#if defined WIN32
  SetConsoleCtrlHandler(&HandlerExit, TRUE);  // if some one kill console :)
  wchar_t NPath[512];
  GetCurrentDirectoryW(512, NPath);
#ifdef NEED_DIR_CHANGE
  SetCurrentDirectoryW(L"../../main");
#endif
  std::wcout << L"[main]: curr_dir = " << NPath << std::endl;
#else

  if (chdir(workingDir.c_str()) != 0)
    std::cout << "[main]: chdir have failed for some reason ... " << std::endl;

  char cwd[1024];
  if (getcwd(cwd, sizeof(cwd)) != nullptr)
    std::cout << "[main]: curr_dir = " << cwd << std::endl;
  else
    std::cout << "getcwd() error" << std::endl;

  {
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = sig_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = SA_RESETHAND;
    sigaction(SIGINT, &sigIntHandler, NULL);
    sigaction(SIGSTOP, &sigIntHandler, NULL);
    sigaction(SIGABRT, &sigIntHandler, NULL);
    sigaction(SIGILL, &sigIntHandler, NULL);
    sigaction(SIGTERM, &sigIntHandler, NULL);
    sigaction(SIGSEGV, &sigIntHandler, NULL);
    sigaction(SIGFPE, &sigIntHandler, NULL);
  }
#endif

  std::cout << "sizeof(size_t) = " << sizeof(size_t) << std::endl;

  try
  {
    /////////////////////    
    // API_TESTS
    /////////////////////

    //test_055_clear_scene();
    //test_077_save_gbuffer_layers();
    //test_082_proc_texture(); // error
    //test_087_proc_texture_reflect();
    //test_090_proc_tex_normalmap();

    /////////////////////    
    // MTL_TESTS
    /////////////////////
    
    //MTL_TESTS::test_103_diffuse_texture();
    
    /////////////////////    
    // LGHT_TESTS
    /////////////////////

    //LGHT_TESTS::test_203_sky_hdr();

    /////////////////////    
    // GEO_TESTS
    /////////////////////

    //GEO_TESTS::test_308_import_obj_w_mtl(); // The bike is black.
    //GEO_TESTS::test_309_import_obj_fullscale(); // The bottle is black.

    /////////////////////    
    // ALGR_TESTS
    /////////////////////
    
    //ALGR_TESTS::test_404_cornell_glossy(); - IBPT and MMLT not equal with PT.
   
    /////////////////////    
    // 3DSMAX_TESTS
    /////////////////////
        
    //run_single_3dsmax_test(L"3dsMaxTests/327_Anisotropy_and_AreaLight_2") - crash?
    
    /////////////////////    
    // EXTENSIONS_TESTS
    /////////////////////

    //EXTENSIONS_TESTS::test_500_ext_vtex();
    //EXTENSIONS_TESTS::test_502_ext_vtex();
    //EXTENSIONS_TESTS::test_506_ext_vtex();

    /////////////////////    
    // SPECTRAL_TESTS
    /////////////////////

    //SPECTRAL_TESTS::test_601_cornell_spectral_2();
    //SPECTRAL_TESTS::test_602_macbeth();
    //SPECTRAL_TESTS::test_605_macbeth_2();
    //SPECTRAL_TESTS::test_606_macbeth_3();
    //SPECTRAL_TESTS::test_603_texture_1();

    /////////////////////
    // All tests
    /////////////////////
    
    // Specify the name of your folder for the report, based on system information.
#ifdef WIN32
    g_systemInfo = { L"Windows", L"NVIDIA_RTX2070_SUPER" };
#else
    g_systemInfo = { L"Linux", L"NVIDIA_RTX2070_SUPER" };
#endif 

    run_all_api_tests();
    run_all_mtl_tests();
    run_all_lgt_tests();
    run_all_geo_tests();
    run_all_alg_tests();
    run_all_vector_tex_tests(); //all wrong - need inv. gamma (pow 2.2) as textures for linear result.
    //run_all_3dsmax_tests(); 



    //run_all_microfacet_torrance_sparrow();
    //window_main_free_look(L"/home/frol/PROG/clsp/database/statex_00001.xml", L"opengl1");

    terminate_opengl();
  }
  catch (std::runtime_error& e)
  {
    std::cout << "std::runtime_error: " << e.what() << std::endl;
  }
  catch (...)
  {
    std::cout << "unknown exception" << std::endl;
  }

  hrErrorCallerPlace(L"main"); // for debug needs only
  hrSceneLibraryClose();
  return 0;
}
