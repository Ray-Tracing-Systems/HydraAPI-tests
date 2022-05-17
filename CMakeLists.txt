cmake_minimum_required(VERSION 3.7)
project(hydra_api_tests)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES
        Camera.h
        input.h input.cpp
        Timer.h Timer.cpp
        linmath.h
        simplerandom.h
        utils.cpp
        mesh_utils.h mesh_utils.cpp
        main.cpp
        tests.h
        test_mtl_adv.cpp
        test_image_processing.cpp
        tests1.cpp
        tests2.cpp
        tests3.cpp
        tests4.cpp
        tests5.cpp
        tests6.cpp
        tests7.cpp
        tests8.cpp
        tests9.cpp
        tests10.cpp
        testsA.cpp
        testsB.cpp
        tests_geo.cpp
        tests_lights.cpp
        tests_lights2.cpp
        test_lights3.cpp
        test_lights4.cpp
        tests_mtl.cpp
        tests_mtl2.cpp
        tests_mtl3.cpp
        tests_mtl4.cpp
        tests_mtl5.cpp
        tests_mtl6.cpp
        tests_external.cpp
        tests_algr.cpp)

set(SOURCE_FILES_GL
        opengl_init.cpp
        RenderDrivers.cpp
        RenderDriverOpenGL1.h RenderDriverOpenGL1.cpp
        RenderDriverOpenGL1_DelayedLoad.cpp
        RenderDriverOpenGL1_DrawBvhLevels.cpp
        RenderDriverOpenGL1_TestCustomAttributes.cpp
        RenderDriverOpenGL1_TestSplit.cpp
        RenderDriverOpenGL1Debug.cpp
        RenderDriverOpenGL1DebugDrawRays.cpp tests_mtl6.cpp)
        
set(SOURCE_FILES_EXTENSIONS
        testsExtensions.cpp)

#set (CMAKE_CXX_FLAGS_DEBUG  "${CMAKE_CXX_FLAGS_DEBUG}")

set(HYDRA_API_ROOT_DIR      ${CMAKE_SOURCE_DIR}/../HydraAPI CACHE PATH "Path to HydraAPI") #
set(HYDRA_API_SRC_DIR       ${HYDRA_API_ROOT_DIR}/hydra_api)
set(HYDRA_API_LIB_DIR       ${HYDRA_API_ROOT_DIR}/bin)
set(HYDRA_API_UTILS_SRC_DIR ${HYDRA_API_ROOT_DIR}/utils)

message(STATUS "HYDRA_API_ROOT_DIR is ${HYDRA_API_ROOT_DIR}")

set(ADDITIONAL_INCLUDE_DIRS
        ${HYDRA_API_ROOT_DIR}/dependencies/include CACHE STRING "Path to HydraAPI dependencies (freeimage and, if OpenGL is used, glfw)")
if(WIN32)
  set(ADDITIONAL_LIBRARY_DIRS
        ${HYDRA_API_ROOT_DIR}/dependencies/lib_x64_win CACHE STRING "Path to HydraAPI dependencies (freeimage and, if OpenGL is used, glfw)")
else()
  set(ADDITIONAL_LIBRARY_DIRS
        ${HYDRA_API_ROOT_DIR}/dependencies/lib_x64_linux CACHE STRING "Path to HydraAPI dependencies (freeimage and, if OpenGL is used, glfw)")
endif()

find_library(FREEIMAGE_LIBRARY freeimage REQUIRED HINTS ${ADDITIONAL_LIBRARY_DIRS})
find_path   (FREEIMAGE_INCLUDE_DIR "FreeImage.h" REQUIRED HINTS ${ADDITIONAL_INCLUDE_DIRS})
include_directories(${FREEIMAGE_INCLUDE_DIR})
# message(STATUS "FREEIMAGE_LIB is ${FREEIMAGE_LIB}")

find_library(GLFW3_LIBRARY NAMES glfw glfw3 glfw3dll REQUIRED HINTS ${ADDITIONAL_LIBRARY_DIRS})
find_path   (GLFW3_INCLUDE_DIR GLFW REQUIRED HINTS ${ADDITIONAL_INCLUDE_DIRS})
include_directories(${GLFW3_INCLUDE_DIR})
# message(STATUS "GLFW3_LIBRARY is ${GLFW3_LIBRARY}")

include_directories(${ADDITIONAL_INCLUDE_DIRS} ${HYDRA_API_SRC_DIR} ${HYDRA_API_UTILS_SRC_DIR})
link_directories(${ADDITIONAL_LIBRARY_DIRS})

find_package(OpenGL REQUIRED)
find_package(OpenMP REQUIRED)

find_package (Threads REQUIRED)
set(THREADS_PREFER_PTHREAD_FLAG ON)

set(VECTOR_TEX_SUPPORT ON CACHE BOOL "Build with vector textures support extension")
if(VECTOR_TEX_SUPPORT)
  set(SOURCE_FILES ${SOURCE_FILES_EXTENSIONS} ${SOURCE_FILES})
	add_compile_definitions(VECTOR_TEX_SUPPORT)
endif()

add_executable(hydra_api_tests ${SOURCE_FILES} ${SOURCE_FILES_GL})

find_library(HYDRA_API_LIB_DEBUG            hydra_api  HINTS ${HYDRA_API_LIB_DIR}/Debug ${HYDRA_API_LIB_DIR} NO_DEFAULT_PATH)
find_library(HYDRA_API_IES_PARSER_LIB_DEBUG ies_parser HINTS ${HYDRA_API_LIB_DIR}/Debug ${HYDRA_API_LIB_DIR} NO_DEFAULT_PATH)
find_library(HYDRA_API_MIKKTSPACE_DEBUG     mikktspace HINTS ${HYDRA_API_LIB_DIR}/Debug ${HYDRA_API_LIB_DIR} NO_DEFAULT_PATH)
find_library(HYDRA_API_CORTO_DEBUG          corto      HINTS ${HYDRA_API_LIB_DIR}/Debug ${HYDRA_API_LIB_DIR} NO_DEFAULT_PATH)

find_library(HYDRA_API_LIB_RELEASE            hydra_api  HINTS ${HYDRA_API_LIB_DIR}/Release ${HYDRA_API_LIB_DIR} NO_DEFAULT_PATH)
find_library(HYDRA_API_IES_PARSER_LIB_RELEASE ies_parser HINTS ${HYDRA_API_LIB_DIR}/Release ${HYDRA_API_LIB_DIR} NO_DEFAULT_PATH)
find_library(HYDRA_API_MIKKTSPACE_RELEASE     mikktspace HINTS ${HYDRA_API_LIB_DIR}/Release ${HYDRA_API_LIB_DIR} NO_DEFAULT_PATH)
find_library(HYDRA_API_CORTO_RELEASE          corto      HINTS ${HYDRA_API_LIB_DIR}/Release ${HYDRA_API_LIB_DIR} NO_DEFAULT_PATH)

if(WIN32)
  find_library(HYDRA_API_CLEW_DEBUG   clew HINTS ${HYDRA_API_LIB_DIR}/Debug   ${HYDRA_API_LIB_DIR} NO_DEFAULT_PATH)
  find_library(HYDRA_API_CLEW_RELEASE clew HINTS ${HYDRA_API_LIB_DIR}/Release ${HYDRA_API_LIB_DIR} NO_DEFAULT_PATH)
else()
  find_package(OpenCL REQUIRED)  
endif()

target_link_libraries(hydra_api_tests LINK_PUBLIC debug     
	${HYDRA_API_LIB_DEBUG}
	${HYDRA_API_IES_PARSER_LIB_DEBUG}
	${HYDRA_API_MIKKTSPACE_DEBUG}
	${HYDRA_API_CORTO_DEBUG})
target_link_libraries(hydra_api_tests LINK_PUBLIC optimized 
	${HYDRA_API_LIB_RELEASE}
	${HYDRA_API_IES_PARSER_LIB_RELEASE}
	${HYDRA_API_MIKKTSPACE_RELEASE}
	${HYDRA_API_CORTO_RELEASE})
  
if(WIN32)
  target_link_libraries(hydra_api_tests LINK_PUBLIC debug     ${HYDRA_API_CLEW_DEBUG})
  target_link_libraries(hydra_api_tests LINK_PUBLIC optimized ${HYDRA_API_CLEW_RELEASE})
else()
  target_link_libraries(hydra_api_tests LINK_PUBLIC OpenCL::OpenCL)
endif()

target_link_libraries(hydra_api_tests LINK_PUBLIC OpenMP::OpenMP_CXX)
target_link_libraries(hydra_api_tests LINK_PUBLIC ${OPENGL_gl_LIBRARY} ${OPENGL_glu_LIBRARY}
		                                          ${FREEIMAGE_LIBRARY} ${GLFW3_LIBRARY})
  
if(VECTOR_TEX_SUPPORT)
	include_directories(${HYDRA_API_ROOT_DIR}/dependencies/optional/msdfgen/include)
	include_directories(${HYDRA_API_ROOT_DIR}/dependencies/optional/nanosvg)
	# include_directories(${HYDRA_API_ROOT_DIR}/dependencies/optional/skia/headers/third_party)
	# include_directories(${HYDRA_API_ROOT_DIR}/dependencies/optional/skia/headers)
	# include_directories(${HYDRA_API_ROOT_DIR}/dependencies/optional/skia/headers/include)
	# include_directories(${HYDRA_API_ROOT_DIR}/dependencies/optional/skia/headers/modules)
	# include_directories(${HYDRA_API_ROOT_DIR}/dependencies/optional/skia/headers/src)

	if(WIN32)
		set(MSDF_LIB_DIR_DEBUG   ${HYDRA_API_ROOT_DIR}/dependencies/optional/msdfgen/lib/win_x64_debug)
		set(MSDF_LIB_DIR_RELEASE ${HYDRA_API_ROOT_DIR}/dependencies/optional/msdfgen/lib/win_x64_release)
		set(SKIA_LIB_DIR         ${HYDRA_API_ROOT_DIR}/dependencies/optional/skia/win_x64)

		find_library(MSDF_LIB_DEBUG   NAMES msdfgen  REQUIRED HINTS ${MSDF_LIB_DIR_DEBUG})
		find_library(MSDF_LIB_RELEASE NAMES msdfgen REQUIRED HINTS ${MSDF_LIB_DIR_RELEASE})
		find_library(SKIA_LIB         NAMES skia.dll skia REQUIRED HINTS ${SKIA_LIB_DIR})
	else()
		set(MSDF_LIB_DIR_DEBUG   ${HYDRA_API_ROOT_DIR}/dependencies/optional/msdfgen/lib/linux_x64_debug)
		set(MSDF_LIB_DIR_RELEASE ${HYDRA_API_ROOT_DIR}/dependencies/optional/msdfgen/lib/linux_x64_release)
		set(SKIA_LIB_DIR         ${HYDRA_API_ROOT_DIR}/dependencies/optional/skia/linux_x64)

		find_library(MSDF_LIB_CORE_DEBUG   NAMES msdfgen-core REQUIRED HINTS ${MSDF_LIB_DIR_DEBUG})
		find_library(MSDF_LIB_EXT_DEBUG    NAMES msdfgen-ext  REQUIRED HINTS ${MSDF_LIB_DIR_DEBUG})
		find_library(MSDF_LIB_CORE_RELEASE NAMES msdfgen-core REQUIRED HINTS ${MSDF_LIB_DIR_RELEASE})
		find_library(MSDF_LIB_EXT_RELEASE  NAMES msdfgen-ext  REQUIRED HINTS ${MSDF_LIB_DIR_RELEASE})
		set(MSDF_LIB_DEBUG   ${MSDF_LIB_CORE_DEBUG}   ${MSDF_LIB_EXT_DEBUG})
		set(MSDF_LIB_RELEASE ${MSDF_LIB_CORE_RELEASE} ${MSDF_LIB_EXT_RELEASE})
	endif()

	find_library(SKIA_LIB         NAMES skia.dll skia REQUIRED HINTS ${SKIA_LIB_DIR})

	target_link_libraries(hydra_api_tests LINK_PUBLIC debug     ${MSDF_LIB_DEBUG})  
	target_link_libraries(hydra_api_tests LINK_PUBLIC optimized ${MSDF_LIB_RELEASE})  
	target_link_libraries(hydra_api_tests LINK_PUBLIC ${SKIA_LIB})  
endif()

  
if(WIN32)
  add_definitions(-DUNICODE -D_UNICODE)
  add_definitions(-DNEED_DIR_CHANGE)
else()
  target_compile_options(hydra_api_tests PRIVATE -fpermissive -Wnarrowing)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.1")

  if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=memory -fno-omit-frame-pointer -fsanitize=leak)
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize-address-use-after-scope -fno-omit-frame-pointer -fsanitize=leak -fsanitize=undefined")
    MESSAGE( STATUS "WE USE CLANG(main);")
  else()
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize-address-use-after-scope -fno-omit-frame-pointer -fsanitize=leak -fsanitize=undefined -fsanitize=bounds-strict")
    MESSAGE( STATUS "WE USE GCC  (main);")
  endif()

  target_link_libraries(hydra_api_tests LINK_PUBLIC Threads::Threads rt dl)
endif()


