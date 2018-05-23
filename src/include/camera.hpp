#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera
{
    public:
        //<f> Constructors & operator=
        /** brief Default constructor */
        Camera();
        /** brief Default destructor */
        virtual ~Camera() noexcept;

        /** brief Copy constructor */
        Camera(const Camera& other);
        /** brief Move constructor */
        Camera(Camera&& other) noexcept;

        /** brief Copy operator */
        Camera& operator= (const Camera& other);
        /** brief Move operator */
        Camera& operator= (Camera&& other) noexcept;
        //</f> /Constructors & operator=

        //<f> Methods
        void Init(const glm::vec3& position, const glm::vec3& camera_direction);
        void ZoomIn();
        void ZoomOut();

        void Update();

        void MoveForward(float);
        void MoveBackward(float);
        void MoveLeft(float);
        void MoveRight(float);

        void LookUp(float);
        void LookDown(float);
        void LookLeft(float);
        void LookRight(float);

        //</f> /Methods

        //<f> Getters/Setters
        void Position(const glm::vec3&);
        glm::vec3 Position() const;

        void Rotation(const glm::vec3&);
        glm::vec3 Rotation() const;

        void Direction(const glm::vec3&);
        glm::vec3 Direction() const;

        void Up(const glm::vec3& up);
        glm::vec3 Up() const;

        glm::vec3 Cross() const;

        void FOVDegrees(float fov);
        float FOVDegrees() const;
        void FOVRadians(float fov);
        float FOVRadians() const;

        /**
         * \brief Set min fov degrees value. If min > max, min = max;
         */
        void MinFOVDegrees(float min_fov);
        float MinFOVDegrees() const;
        /**
         * \brief Set min fov degrees value. If max < min, max = min;
         */
        void MaxFOVDegrees(float max_fov);
        float MaxFOVDegrees() const;

        void AspectRatio(float aspect);
        float AspectRatio() const;

        void ZNear(float z_near);
        float ZNear() const;
        void ZFar(float z_far);
        float ZFar() const;

        glm::mat4 View() const;
        glm::mat4 Projection() const;
        //</f> /Getters/Setters

    protected:
        // vars and stuff

    private:
        glm::vec3 m_position;
        glm::vec3 m_direction;
        glm::vec3 m_direction_cross;
        glm::vec3 m_rotation;
        glm::vec3 m_up;

        float m_fov_deg;
        float m_min_fov_deg;
        float m_max_fov_deg;
        float m_aspect_ratio;
        float m_z_near;
        float m_z_far;

        float m_zoom_step;

        struct
        {
            glm::mat4 view;
            glm::mat4 projection;
        } m_matrices;

        //dirty Flags
        bool m_update_direction;
        bool m_update_cross;
        bool m_update_view;
        bool m_update_projection;

        //<f> Update Methods
        void UpdateView();
        void UpdateProjection();
        void UpdateCameraDirection();
        void UpdateCameraDirectionCross();
        //</f> /Update Methods
};

#endif //CAMERA_HPP
