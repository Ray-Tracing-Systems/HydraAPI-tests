#if defined(WIN32)
#define _USE_MATH_DEFINES // Make MS math.h define M_PI
#endif

#include "tests.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sstream>
#include <random>

#include <GLFW/glfw3.h>
#if defined(WIN32)
#pragma comment(lib, "glfw3dll.lib")
#else
#endif

#include "Timer.h"
#include "linmath.h"

#include "mesh_utils.h"
#include "input.h"
#include "Camera.h"

#include "HydraXMLHelpers.h"
#include <algorithm>

///////////////////////////////////////////////////////////////////////////////////

using namespace HydraXMLHelpers;

extern Input       g_input;
static GLFWwindow* g_window  = nullptr;
static int         g_width   = 1024;
static int         g_height  = 1024;
static int         g_filling = 0;


static void key(GLFWwindow* window, int k, int s, int action, int mods)
{
  if (action != GLFW_PRESS) return;

  switch (k)
  {
  case GLFW_KEY_Z:
    //if (mods & GLFW_MOD_SHIFT)
    //  view_rotz -= 5.0;
    //else
    //  view_rotz += 5.0;
    break;
  case GLFW_KEY_ESCAPE:
    glfwSetWindowShouldClose(window, GL_TRUE);
    break;
  case GLFW_KEY_UP:
    //view_rotx += 5.0;
    break;
  case GLFW_KEY_DOWN:
    //view_rotx -= 5.0;
    break;
  case GLFW_KEY_LEFT:
    //view_roty += 5.0;
    break;
  case GLFW_KEY_RIGHT:
    //view_roty -= 5.0;
    break;
  case GLFW_KEY_SPACE:
    if (g_filling == 0)
    {
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      g_filling = 1;
    }
    else
    {
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      g_filling = 0;
    }
  break;
  default:
    return;
  }
}


void initGLIfNeeded(int a_width, int a_height, const char* a_name)
{
  static bool firstCall = true;
  
  if (firstCall)
  {
    g_width  = a_width;
    g_height = a_height;
    
    if (!glfwInit())
    {
      fprintf(stderr, "Failed to initialize GLFW\n");
      exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_DEPTH_BITS, 24);

    g_window = glfwCreateWindow(g_width, g_height, a_name, NULL, NULL);
    if (!g_window)
    {
      fprintf(stderr, "Failed to open GLFW window\n");
      glfwTerminate();
      exit(EXIT_FAILURE);
    }

    // Set callback functions
    //glfwSetFramebufferSizeCallback(g_window, reshape);
    glfwSetKeyCallback(g_window, key);

    glfwMakeContextCurrent(g_window);
    //gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glfwSwapInterval(1);

    glfwGetFramebufferSize(g_window, &g_width, &g_height);
    //reshape(g_window, g_width, g_height);

    firstCall = false;
  }
  else
  {
    g_width  = a_width;
    g_height = a_height;
  
    glfwSetWindowSize(g_window, g_width, g_height);
    glfwSetWindowTitle(g_window, a_name);
  }
}
