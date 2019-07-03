#ifndef RENDERDRIVERS_H
#define RENDERDRIVERS_H

#include "RenderDriverOpenGL1.h"

std::unique_ptr<IHRRenderDriver> CreateOpenGL1_RenderDriver();
std::unique_ptr<IHRRenderDriver> CreateOpenGL1_DelayedLoad_RenderDriver();
std::unique_ptr<IHRRenderDriver> CreateOpenGL1_DelayedLoad_RenderDriver2();
std::unique_ptr<IHRRenderDriver> CreateOpenGL1DrawBVH_RenderDriver();
std::unique_ptr<IHRRenderDriver> CreateOpenGL1Debug_TestCustomAttributes();
std::unique_ptr<IHRRenderDriver> CreateOpenGL1TestSplit_RenderDriver();
std::unique_ptr<IHRRenderDriver> CreateOpenGL1Debug_RenderDriver();
std::unique_ptr<IHRRenderDriver> CreateOpenGL1DrawRays_RenderDriver();

void registerAllGL1Drivers();
void printAllAvailableDrivers();

#endif //RENDERDRIVERS_H