###########################
#          RAIN           #
###########################

set(INCLUDE_DIRECTORIES src/other/include)

set(SOURCE_FILES "")
set(FILES_PATH "src/other/src/")#file path from root
#list(APPEND SOURCE_FILES "${FILES_PATH}particle.cpp")

#headers
set(HEADER_FILES "")
set(FILES_PATH "src/other/include/")#file path from root
list(APPEND HEADER_FILES "${FILES_PATH}sol.hpp")
list(APPEND HEADER_FILES "${FILES_PATH}tiny_obj_loader.h")
list(APPEND HEADER_FILES "${FILES_PATH}tiny_obj_loader.cpp")
