#ifndef RENDERDRIVERS_H
#define RENDERDRIVERS_H

#include "RenderDriverOpenGL1.h"

IHRRenderDriver* CreateOpenGL1_RenderDriver();
IHRRenderDriver* CreateOpenGL1_DelayedLoad_RenderDriver();
IHRRenderDriver* CreateOpenGL1_DelayedLoad_RenderDriver2();
IHRRenderDriver* CreateOpenGL1DrawBVH_RenderDriver();
IHRRenderDriver* CreateOpenGL1Debug_TestCustomAttributes();
IHRRenderDriver* CreateOpenGL1TestSplit_RenderDriver();
IHRRenderDriver* CreateOpenGL1Debug_RenderDriver();
IHRRenderDriver* CreateOpenGL1DrawRays_RenderDriver();

void registerAllGL1Drivers();
void printAllAvailableDrivers();

#endif //RENDERDRIVERS_H