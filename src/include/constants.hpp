#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE//because vulkan depth goes from 0 to 1 and gl used -1 to 1
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

const glm::vec3 c_unit_vec_x{1.f, 0.f, 0.f};
const glm::vec3 c_unit_vec_y{0.f, 1.f, 0.f};
const glm::vec3 c_unit_vec_z{0.f, 0.f, 1.f};

#endif //CONSTANTS_HPP
