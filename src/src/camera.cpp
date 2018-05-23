#include "camera.hpp"
#include <utility>
#include <algorithm>
#include <iostream>

//<f> Constructors & operator=
Camera::Camera() : m_update_direction{false}, m_update_cross{false}, m_update_view{false}, m_update_projection{false}
{

}

Camera::~Camera() noexcept
{

}

Camera::Camera(const Camera& other)
{

}

Camera::Camera(Camera&& other) noexcept
{

}

Camera& Camera::operator=(const Camera& other)
{
    if(this != &other)//not same ref
    {
        auto tmp(other);
        *this = std::move(tmp);
    }

    return *this;
}

Camera& Camera::operator=(Camera&& other) noexcept
{
    if(this != &other)//not same ref
    {
        //move here
    }
    return *this;
}
//</f> /Constructors & operator=

//<f> Methods
void Camera::Init(const glm::vec3& position, const glm::vec3& direction)
{
    Position(position);
    Direction(direction);
}

void Camera::ZoomIn()
{
    m_fov_deg += m_zoom_step;
    m_fov_deg = std::min(m_max_fov_deg, m_fov_deg);
    m_update_projection = true;
}

void Camera::ZoomOut()
{
    m_fov_deg -= m_zoom_step;
    m_fov_deg = std::max(m_min_fov_deg, m_fov_deg);
    m_update_projection = true;
}

void Camera::Update()
{
    UpdateView();
    UpdateProjection();
    UpdateCameraDirection();
    UpdateCameraDirectionCross();
}

void Camera::MoveForward(float speed)
{
    m_position += m_direction * speed;
    m_update_view = true;
}

void Camera::MoveBackward(float speed)
{
    m_position -= m_direction * speed;
    m_update_view = true;
}

void Camera::MoveLeft(float speed)
{
    m_position -= m_direction_cross * speed;
    m_update_view = true;
}

void Camera::MoveRight(float speed)
{
    m_position += m_direction_cross * speed;
    m_update_view = true;
}

void Camera::LookUp(float angle)
{
    m_rotation.x -= angle;
    m_update_view = true;
}

void Camera::LookDown(float angle)
{
    m_rotation.x += angle;
    m_update_view = true;
}

void Camera::LookLeft(float angle)
{
    m_rotation.y -= angle;
    m_update_view = true;
}

void Camera::LookRight(float angle)
{
    m_rotation.y += angle;
    m_update_view = true;
}

//</f> /Methods

//<f> Getters/Setters
void Camera::Position(const glm::vec3& position) { m_position = position; m_update_view = true; }
glm::vec3 Camera::Position() const { return m_position; }
void Camera::Rotation(const glm::vec3& rotation) { m_rotation = rotation; m_update_view = true; }
glm::vec3 Camera::Rotation() const { return m_rotation; }

void Camera::Direction(const glm::vec3& direction) { m_direction = glm::normalize(direction); m_update_direction = true; }
glm::vec3 Camera::Direction() const { return m_direction; }

void Camera::Up(const glm::vec3& up) { m_up = glm::normalize(up); m_update_cross = true; }
glm::vec3 Camera::Up() const { return m_up; }

glm::vec3 Camera::Cross() const { return m_direction_cross; }

void Camera::FOVDegrees(float fov) { m_fov_deg = fov; m_update_projection = true; }
float Camera::FOVDegrees() const { return m_fov_deg; }
void Camera::FOVRadians(float fov) { m_fov_deg = glm::degrees(fov); m_update_projection = true; }
float Camera::FOVRadians() const { return glm::radians(m_fov_deg); }

/**
 * \brief Set min fov degrees value. If min > max, min = max;
 */
void Camera::MinFOVDegrees(float min_fov) { m_min_fov_deg = std::min(min_fov, m_max_fov_deg); m_update_projection = true; }
float Camera::MinFOVDegrees() const { return m_min_fov_deg; }
/**
 * \brief Set min fov degrees value. If max < min, max = min;
 */
void Camera::MaxFOVDegrees(float max_fov) { m_max_fov_deg = max_fov; m_update_projection = true; }
float Camera::MaxFOVDegrees() const { return m_max_fov_deg; }

void Camera::AspectRatio(float aspect){ m_aspect_ratio = aspect; m_update_projection = true; }
float Camera::AspectRatio() const { return m_aspect_ratio; }

void Camera::ZNear(float z_near) { m_z_near = z_near; m_update_projection = true; }
float Camera::ZNear() const { return m_z_near; }
void Camera::ZFar(float z_far) { m_z_far = z_far; m_update_projection = true; }
float Camera::ZFar() const { return m_z_far; }

glm::mat4 Camera::View() const { return m_matrices.view; }
glm::mat4 Camera::Projection() const { return m_matrices.projection; }
//</f> /Getters/Setters

//<f> Update Methods
void Camera::UpdateView()
{
    if(!m_update_view)
        return;
    m_update_view = false;

    glm::mat4 rotation_mat = glm::mat4(1.0f);
	glm::mat4 transform_mat = glm::translate(glm::mat4(1.0f), m_position);

	rotation_mat = glm::rotate(rotation_mat, glm::radians(m_rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	rotation_mat = glm::rotate(rotation_mat, glm::radians(m_rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	rotation_mat = glm::rotate(rotation_mat, glm::radians(m_rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

	m_matrices.view = rotation_mat * transform_mat;

    //we need to update the direction as it depends on the rotation
    m_update_direction = true;
}
void Camera::UpdateProjection()
{
    if(!m_update_projection)
        return;
    m_update_projection = false;
    m_matrices.projection = glm::perspective(glm::radians(m_fov_deg), m_aspect_ratio, m_z_near, m_z_far);
    m_matrices.projection[1][1] *= -1;
}

void Camera::UpdateCameraDirection()
{
    if(!m_update_direction)
        return;
    m_update_direction = false;

    glm::vec3 direction;
	direction.x = -cos(glm::radians(m_rotation.x)) * sin(glm::radians(m_rotation.y));
	direction.y = sin(glm::radians(m_rotation.x));
	direction.z = cos(glm::radians(m_rotation.x)) * cos(glm::radians(m_rotation.y));
    m_direction = glm::normalize(direction);
    //we need to update the cross as it depends on the direction
    m_update_cross = true;
}

void Camera::UpdateCameraDirectionCross()
{
    if(!m_update_cross)
        return;
    m_update_cross = false;
    m_direction_cross = glm::normalize(glm::cross(m_direction, m_up));
}
//</f> /Update Methods
