#include "RenderDrivers.h"

void registerGL1PlainDriver()
{
  HRDriverInfo plain_gl1_info;
  plain_gl1_info.supportHDRFrameBuffer        = false;
  plain_gl1_info.supportHDRTextures           = false;
  plain_gl1_info.supportMultiMaterialInstance = false;

  plain_gl1_info.supportImageLoadFromInternalFormat = false;
  plain_gl1_info.supportImageLoadFromExternalFormat = false;
  plain_gl1_info.supportMeshLoadFromInternalFormat  = false;
  plain_gl1_info.supportLighting                    = false;

  plain_gl1_info.memTotal = int64_t(8) * int64_t(1024 * 1024 * 1024);

  plain_gl1_info.driverName = L"opengl1";
  plain_gl1_info.createFunction = CreateOpenGL1_RenderDriver;

  RenderDriverFactory::Register(L"opengl1", plain_gl1_info);
}

void registerGL1DelayedLoadDrivers()
{
  HRDriverInfo driverInfo;
  driverInfo.supportHDRFrameBuffer        = false;
  driverInfo.supportHDRTextures           = false;
  driverInfo.supportMultiMaterialInstance = false;

  driverInfo.supportImageLoadFromInternalFormat = true;
  driverInfo.supportImageLoadFromExternalFormat = true;
  driverInfo.supportMeshLoadFromInternalFormat  = false;
  driverInfo.supportLighting                    = false;

  driverInfo.memTotal = int64_t(8) * int64_t(1024 * 1024 * 1024);

  driverInfo.driverName = L"opengl1DelayedLoad";
  driverInfo.createFunction = CreateOpenGL1_DelayedLoad_RenderDriver;
  RenderDriverFactory::Register(L"opengl1DelayedLoad", driverInfo);

  HRDriverInfo driverInfo2;
  driverInfo2.supportHDRFrameBuffer        = false;
  driverInfo2.supportHDRTextures           = false;
  driverInfo2.supportMultiMaterialInstance = false;

  driverInfo2.supportImageLoadFromInternalFormat = true;
  driverInfo2.supportImageLoadFromExternalFormat = true;
  driverInfo2.supportMeshLoadFromInternalFormat  = true;
  driverInfo2.supportLighting                    = false;

  driverInfo2.memTotal = int64_t(8) * int64_t(1024 * 1024 * 1024);

  driverInfo2.driverName = L"opengl1DelayedLoad2";
  driverInfo2.createFunction = CreateOpenGL1_DelayedLoad_RenderDriver2;
  RenderDriverFactory::Register(L"opengl1DelayedLoad2", driverInfo2);
}

void registerGL1DrawBVHDriver()
{
  HRDriverInfo driverInfo;
  driverInfo.driverName = L"opengl1DrawBvh";
  driverInfo.createFunction = CreateOpenGL1DrawBVH_RenderDriver;
  RenderDriverFactory::Register(L"opengl1DrawBvh", driverInfo);
}

void registerGL1TestCustomAttributesDriver()
{
  HRDriverInfo driverInfo;
  driverInfo.driverName = L"opengl1TestCustomAttributes";
  driverInfo.createFunction = CreateOpenGL1Debug_TestCustomAttributes;

  RenderDriverFactory::Register(L"opengl1TestCustomAttributes", driverInfo);
}

void registerGL1TestSplitDriver()
{
  HRDriverInfo driverInfo;
  driverInfo.driverName = L"opengl1TestSplit";
  driverInfo.createFunction = CreateOpenGL1TestSplit_RenderDriver;

  RenderDriverFactory::Register(L"opengl1TestSplit", driverInfo);
}

void registerGL1DebugDriver()
{
  HRDriverInfo driverInfo;
  driverInfo.driverName = L"opengl1Debug";
  driverInfo.createFunction = CreateOpenGL1Debug_RenderDriver;

  RenderDriverFactory::Register(L"opengl1Debug", driverInfo);
}

void registerGL1DrawRaysDriver()
{
  HRDriverInfo driverInfo;
  driverInfo.driverName = L"opengl1DrawRays";
  driverInfo.createFunction = CreateOpenGL1DrawRays_RenderDriver;

  RenderDriverFactory::Register(L"opengl1DrawRays", driverInfo);
}

void registerAllGL1Drivers()
{
  registerGL1PlainDriver();
  registerGL1DelayedLoadDrivers();
  registerGL1DrawBVHDriver();
  registerGL1TestCustomAttributesDriver();
  registerGL1TestSplitDriver();
  registerGL1DebugDriver();
  registerGL1DrawRaysDriver();
}

void printAllAvailableDrivers()
{
  auto drivers = RenderDriverFactory::GetListOfRegisteredDrivers();

  std::cout << "Render drivers currently registered in Hydra API: " << std::endl;
  for(const auto &d :drivers)
  {
    if(!d.empty())
    {
      auto tmp = ws2s(d);
      std::cout << tmp.c_str() << std::endl;
    }
  }

}




