#ifndef hCamera
#define hCamera

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include <map>

namespace vk
{// TODO:
    // Clean up this file
    struct Camera {
        Camera();
    public:
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 proj;
        alignas(16) glm::vec3 position;

        void update(float FOVdeg = 45.f, float nearPlane = 0.01f, float farPlane = 1000.f);
    protected:
        glm::vec3 Up;
        glm::vec3 Orientation;
    };
    struct Player : Camera {
        Player() {
            player = this;
        }
        void update();
    public:
        using pfunc = void(*)();
        std::map<int, pfunc> keyboard_map = {
            {GLFW_KEY_W, forward},
            {GLFW_KEY_S, backward},
            {GLFW_KEY_A, left},
            {GLFW_KEY_D, right},
            {GLFW_KEY_SPACE, up},
            {GLFW_KEY_LEFT_CONTROL, down},
            {GLFW_KEY_LEFT_SHIFT, sprint}
        };
    protected:
        float velocity = 0.05f;
        float sensitivity = 1.75f;

        bool firstClick = true;
        inline const static float default_velocity = 0.05f;
    private:
        inline static Player* player;
        static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void forward();
        static void backward();
        static void left();
        static void right();
        static void up();
        static void down();
        static void sprint();


        void mouse();
        void buttons();
        void sticks();

    };
}

#endif