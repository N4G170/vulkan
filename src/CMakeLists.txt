###########################
#         vulkan         #
###########################

set(INCLUDE_DIRECTORIES src/include src/include/vk src/include/managers)

set(HEADER_FILES "")
set(FILES_PATH "src/include/managers/")#file path from root
list(APPEND HEADER_FILES "${FILES_PATH}resource_manager.hpp")

set(FILES_PATH "src/include/vk/")#file path from root
list(APPEND HEADER_FILES "${FILES_PATH}commandbuffer.hpp")
list(APPEND HEADER_FILES "${FILES_PATH}framebuffer.hpp")
list(APPEND HEADER_FILES "${FILES_PATH}pipelines.hpp")
list(APPEND HEADER_FILES "${FILES_PATH}renderpass.hpp")
list(APPEND HEADER_FILES "${FILES_PATH}swapchain.hpp")
list(APPEND HEADER_FILES "${FILES_PATH}vulkan_context.hpp")
list(APPEND SOURCE_FILES "${FILES_PATH}vulkan_memory_block.hpp")
list(APPEND SOURCE_FILES "${FILES_PATH}vulkan_memory_manager.hpp")
list(APPEND HEADER_FILES "${FILES_PATH}vulkan_pointers.hpp")
list(APPEND HEADER_FILES "${FILES_PATH}vulkan_resources.hpp")
list(APPEND HEADER_FILES "${FILES_PATH}vulkan_structs.hpp")
list(APPEND HEADER_FILES "${FILES_PATH}vulkan_utils.hpp")

set(FILES_PATH "src/include/")#file path from root
list(APPEND HEADER_FILES "${FILES_PATH}camera.hpp")
list(APPEND HEADER_FILES "${FILES_PATH}config.hpp")
list(APPEND HEADER_FILES "${FILES_PATH}constants.hpp")
list(APPEND HEADER_FILES "${FILES_PATH}material.hpp")
list(APPEND HEADER_FILES "${FILES_PATH}mesh.hpp")
list(APPEND HEADER_FILES "${FILES_PATH}model.hpp")
list(APPEND HEADER_FILES "${FILES_PATH}sdl_init.hpp")
list(APPEND HEADER_FILES "${FILES_PATH}structs.hpp")
list(APPEND HEADER_FILES "${FILES_PATH}test_scene.hpp")
list(APPEND HEADER_FILES "${FILES_PATH}texture.hpp")
list(APPEND HEADER_FILES "${FILES_PATH}utils.hpp")


###########################################

set(SOURCE_FILES "")
set(FILES_PATH "src/src/managers/")#file path from root
list(APPEND SOURCE_FILES "${FILES_PATH}resource_manager.cpp")

set(FILES_PATH "src/src/vk/")#file path from root
list(APPEND SOURCE_FILES "${FILES_PATH}commandbuffer.cpp")
list(APPEND SOURCE_FILES "${FILES_PATH}framebuffer.cpp")
list(APPEND SOURCE_FILES "${FILES_PATH}pipelines.cpp")
list(APPEND SOURCE_FILES "${FILES_PATH}renderpass.cpp")
list(APPEND SOURCE_FILES "${FILES_PATH}swapchain.cpp")
list(APPEND SOURCE_FILES "${FILES_PATH}vulkan_context.cpp")
list(APPEND SOURCE_FILES "${FILES_PATH}vulkan_memory_block.cpp")
list(APPEND SOURCE_FILES "${FILES_PATH}vulkan_memory_manager.cpp")
list(APPEND SOURCE_FILES "${FILES_PATH}vulkan_resources.cpp")

set(FILES_PATH "src/src/")#file path from root
list(APPEND SOURCE_FILES "${FILES_PATH}camera.cpp")
list(APPEND SOURCE_FILES "${FILES_PATH}main.cpp")
list(APPEND SOURCE_FILES "${FILES_PATH}material.cpp")
list(APPEND SOURCE_FILES "${FILES_PATH}mesh.cpp")
list(APPEND SOURCE_FILES "${FILES_PATH}model.cpp")
list(APPEND SOURCE_FILES "${FILES_PATH}sdl_init.cpp")
list(APPEND SOURCE_FILES "${FILES_PATH}structs.cpp")
list(APPEND SOURCE_FILES "${FILES_PATH}texture.cpp")
list(APPEND SOURCE_FILES "${FILES_PATH}utils.cpp")
