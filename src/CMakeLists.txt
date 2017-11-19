###########################
#         sdl_gui         #
###########################

set(INCLUDE_DIRECTORIES src/include)

set(SOURCE_FILES "")
set(FILES_PATH "src/src/")#file path from root
list(APPEND SOURCE_FILES "${FILES_PATH}main.cpp")
list(APPEND SOURCE_FILES "${FILES_PATH}sdl_init.cpp")
list(APPEND SOURCE_FILES "${FILES_PATH}vulkan_context.cpp")
list(APPEND SOURCE_FILES "${FILES_PATH}utils.cpp")
list(APPEND SOURCE_FILES "${FILES_PATH}texture.cpp")

set(HEADER_FILES "")
set(FILES_PATH "src/include/")#file path from root
list(APPEND HEADER_FILES "${FILES_PATH}sdl_init.hpp")
list(APPEND HEADER_FILES "${FILES_PATH}vulkan_context.hpp")
list(APPEND HEADER_FILES "${FILES_PATH}vulkan_structs.hpp")
list(APPEND HEADER_FILES "${FILES_PATH}utils.hpp")
list(APPEND HEADER_FILES "${FILES_PATH}texture.hpp")
