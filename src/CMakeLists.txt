###########################
#         vulkan         #
###########################

set(INCLUDE_DIRECTORIES src/include)

set(SOURCE_FILES "")
set(FILES_PATH "src/src/")#file path from root
list(APPEND SOURCE_FILES "${FILES_PATH}main.cpp")
list(APPEND SOURCE_FILES "${FILES_PATH}sdl_init.cpp")
list(APPEND SOURCE_FILES "${FILES_PATH}vulkan_context.cpp")
list(APPEND SOURCE_FILES "${FILES_PATH}utils.cpp")
list(APPEND SOURCE_FILES "${FILES_PATH}texture.cpp")
list(APPEND SOURCE_FILES "${FILES_PATH}camera.cpp")
list(APPEND SOURCE_FILES "${FILES_PATH}mesh.cpp")

set(HEADER_FILES "")
set(FILES_PATH "src/include/")#file path from root
list(APPEND HEADER_FILES "${FILES_PATH}sdl_init.hpp")
list(APPEND HEADER_FILES "${FILES_PATH}vulkan_context.hpp")
list(APPEND HEADER_FILES "${FILES_PATH}vulkan_structs.hpp")
list(APPEND HEADER_FILES "${FILES_PATH}utils.hpp")
list(APPEND HEADER_FILES "${FILES_PATH}texture.hpp")
list(APPEND HEADER_FILES "${FILES_PATH}camera.hpp")
list(APPEND HEADER_FILES "${FILES_PATH}mesh.hpp")

#Other folder
list(APPEND INCLUDE_DIRECTORIES src/other/include)

set(FILES_PATH "src/other/src/")#file path from root
list(APPEND SOURCE_FILES "${FILES_PATH}tiny_obj_loader.cpp")

set(FILES_PATH "src/other/include/")#file path from root
list(APPEND HEADER_FILES "${FILES_PATH}tiny_obj_loader.h")
