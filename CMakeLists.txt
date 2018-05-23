cmake_minimum_required(VERSION 3.8.2)

#project name and cpp standard (c++11)
project("Vulkan" CXX)
# set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD 14)
# set(CMAKE_CXX_STANDARD 17)

if(NOT CMAKE_BUILD_TYPE)
# set(CMAKE_BUILD_TYPE Debug)
  set(CMAKE_BUILD_TYPE Release)
endif()

if(CMAKE_BUILD_TYPE MATCHES Release)
    add_definitions(-DNDEBUG)
endif()

if (UNIX)
# set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -fno-pie -no-pie")
endif (UNIX)

set(app_name "vulkan")

#include_directories(src)#will search inside all directories inside src
add_subdirectory(src)


#Get include directories from sub folders
set(INCLUDE_DIRECTORIES "")
get_directory_property(INCLUDE_DIRECTORIES DIRECTORY src DEFINITION INCLUDE_DIRECTORIES)
#Add include directories
include_directories(${INCLUDE_DIRECTORIES})

#Get Source files paths from subfolders
set(SOURCE_FILES "")
get_directory_property(SOURCE_FILES DIRECTORY src DEFINITION SOURCE_FILES)

#Get Header files paths
set(HEADER_FILES "")
get_directory_property(HEADER_FILES DIRECTORY src DEFINITION HEADER_FILES)


if (UNIX)
# includes cmake_modules/FindSDL2*.cmake
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
#SDL2
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIR})
# SDL2 image
find_package(SDL2_image REQUIRED)
include_directories(${SDL2_IMAGE_INCLUDE_DIR})
#Lua
find_package(Lua REQUIRED)
include_directories(${LUA_INCLUDE_DIR})
#sol
set(SOL_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/external/include/sol2/)
include_directories(${SOL_INCLUDE_DIR})
#Vulkan
find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIR})
find_package(GLM REQUIRED)
include_directories(${GLM_INCLUDE_DIR})
find_package(ASSIMP REQUIRED)
include_directories(${ASSIMP_INCLUDE_DIRS})
endif (UNIX)

if(WIN32)
set(SDL2_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/external/include/sdl2/)
include_directories(${SDL2_INCLUDE_DIR})
set(SDL2_LIBRARY ${CMAKE_SOURCE_DIR}/external/lib/sdl2.lib ${CMAKE_SOURCE_DIR}/external/lib/sdl2main.lib)

set(SDL2_IMAGE_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/external/include/sdl2/)
include_directories(${SDL2_IMAGE_INCLUDE_DIR})
set(SDL2_IMAGE_LIBRARY ${CMAKE_SOURCE_DIR}/external/lib/sdl2_image.lib)

set(Vulkan_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/external/include/)
include_directories(${Vulkan_INCLUDE_DIR})
set(Vulkan_LIBRARY ${CMAKE_SOURCE_DIR}/external/lib/vulkan-1.lib)

set(GLM_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/external/include/)
include_directories(${GLM_INCLUDE_DIR})

set(ASSIMP_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/external/include/assimp)
include_directories(${ASSIMP_INCLUDE_DIRS})
set(ASSIMP_LIBRARIES ${CMAKE_SOURCE_DIR}/external/lib/assimp-vc140-mt.lib)

set(LUA_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/external/include/lua/)
include_directories(${LUA_INCLUDE_DIR})
set(LUA_LIBRARY ${CMAKE_SOURCE_DIR}/external/lib/lua53.lib)

set(SOL_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/external/include/sol2/)
include_directories(${SOL_INCLUDE_DIR})
endif(WIN32)

#define exe folder
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin )

add_executable(${app_name} ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(${app_name} ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARY} ${ASSIMP_LIBRARIES} ${Vulkan_LIBRARY} ${GLM_LIBRARY} ${LUA_LIBRARY})

#copy contents of root folder "data" to folder "data" next to executable, if build is succeeds(POST_BUILD)
add_custom_command(TARGET ${app_name} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${CMAKE_SOURCE_DIR}/data $<TARGET_FILE_DIR:${app_name}>/data)

if (WIN32)
set(DLLS
    assimp-vc140-mt.dll
    libjpeg-9.dll
    libpng16-16.dll
    libtiff-5.dll
    libwebp-7.dll
    lua53.dll
    SDL2.dll
    SDL2_image.dll
    vulkan-1.dll
    zlib1.dll
)

foreach(dll ${DLLS})
  add_custom_command(TARGET ${app_name} POST_BUILD
                 COMMAND ${CMAKE_COMMAND} -E copy_if_different
                 "${CMAKE_SOURCE_DIR}/external/dll/${dll}"
                 $<TARGET_FILE_DIR:${app_name}>)
endforeach(dll)
endif (WIN32)
