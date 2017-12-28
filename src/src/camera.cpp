#include "camera.hpp"
#include <utility>
#include <algorithm>

//<f> Constructors & operator=
Camera::Camera()
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
void Camera::ZoomIn()
{
    m_fov_deg -= m_zoom_step;

    // if()
}

void Camera::ZoomOut()
{
    
}
//</f> /Methods

//<f> Getters/Setters
void Camera::Position(glm::vec3 position) { m_position = position; }
glm::vec3 Camera::Position() const { return m_position; }

void Camera::LookPosition(glm::vec3 look_position) { m_look_position = look_position; }
glm::vec3 Camera::LookPosition() const { return m_look_position; }

void Camera::UpVector(glm::vec3 up) { m_up_vector = up; }
glm::vec3 Camera::UpVector() const { return m_up_vector; }

void Camera::FOVDegrees(float fov) { m_fov_deg = fov; }
float Camera::FOVDegrees() const { return m_fov_deg; }
void Camera::FOVRadians(float fov) { m_fov_deg = glm::degrees(fov); }
float Camera::FOVRadians() const { return glm::radians(m_fov_deg); }

/**
 * \brief Set min fov degrees value. If min > max, min = max;
 */
void Camera::MinFOVDegrees(float min_fov) { m_min_fov_deg = std::min(min_fov, m_max_fov_deg); }
float Camera::MinFOVDegrees() const { return m_min_fov_deg; }
/**
 * \brief Set min fov degrees value. If max < min, max = min;
 */
void Camera::MaxFOVDegrees(float max_fov) { }
float Camera::MaxFOVDegrees() const { }

void Camera::AspectRatio(float aspect){ m_aspect_ratio = aspect; }
float Camera::AspectRatio() const { return m_aspect_ratio; }

void Camera::ZNear(float z_near) { m_z_near = z_near; }
float Camera::ZNear() const { return m_z_near; }
void Camera::ZFar(float z_far) { m_z_far = z_far; }
float Camera::ZFar() const { return m_z_far; }

glm::mat4 Camera::View() const { return m_matrices.view; }
glm::mat4 Camera::Projection() const { return m_matrices.projection; }
//</f> /Getters/Setters

//<f> Update Methods
void Camera::UpdateView()
{
    //camera position, point looking at, up vector(what side is up)
    m_matrices.view = glm::lookAt(m_position, m_look_position, m_up_vector);
}
void Camera::UpdateProjection()
{
    m_matrices.projection = glm::perspective(glm::radians(m_fov_deg), m_aspect_ratio, m_z_near, m_z_far);
}
//</f> /Update Methods
