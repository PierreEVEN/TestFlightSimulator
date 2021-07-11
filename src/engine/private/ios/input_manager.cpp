

#include "ios/input_manager.h"

#include "cpputils/logger.hpp"

static int    key_state[GLFW_KEY_LAST];
static int    mouse_button_state[GLFW_MOUSE_BUTTON_LAST];
static double mouse_position[2];
static double mouse_movement[2];
static double mouse_position_last[2];

static void key_callback(GLFWwindow* handle, int key_code, int scan_code, int action, int modifiers)
{
    switch (action)
    {
    case GLFW_PRESS:
        key_state[key_code] = 1;
        break;
    case GLFW_RELEASE:
        key_state[key_code] = 0;
        break;
    case GLFW_REPEAT: // not used
        break;
    default:
        LOG_ERROR("unhandled input action type");
    }
}

static void cursor_position_callback(GLFWwindow* handle, double pos_x, double pos_y)
{
    mouse_position[0] = pos_x;
    mouse_position[1] = pos_y;

    mouse_movement[0] += mouse_position[0] - mouse_position_last[0];
    mouse_movement[1] += mouse_position[1] - mouse_position_last[1];

    mouse_position_last[0] = mouse_position[0];
    mouse_position_last[1] = mouse_position[1];
}

static void mouse_button_callback(GLFWwindow* handle, int button_code, int action, int modifiers)
{
    switch (action)
    {
    case GLFW_PRESS:
        mouse_button_state[button_code] = 1;
        break;
    case GLFW_RELEASE:
        mouse_button_state[button_code] = 0;
        break;
    case GLFW_REPEAT: // not used
        break;
    default:
        LOG_ERROR("unhandled input action type");
    }
}

void InputManager::add_input(const InputAction& new_input)
{
    input_actions.emplace_back(new_input);
}

InputAction* InputManager::get_input(const std::string& input_name)
{
    for (auto& input : input_actions)
    {
        if (input_name == input.name())
            return &input;
    }
    return nullptr;
}

void InputManager::configure(GLFWwindow* handle)
{
    for (int i = 0; i < GLFW_KEY_LAST; ++i)
        key_state[i] = 0;

    for (int i = 0; i < GLFW_MOUSE_BUTTON_LAST; ++i)
        mouse_button_state[i] = 0;

    mouse_position_last[0] = 0.0;
    mouse_position_last[1] = 0.0;
    mouse_movement[0]      = 0.0;
    mouse_movement[1]      = 0.0;
    mouse_position[0]      = 0.0;
    mouse_position[1]      = 0.0;

    glfwSetKeyCallback(handle, key_callback);
    glfwSetCursorPosCallback(handle, cursor_position_callback);
    glfwSetMouseButtonCallback(handle, mouse_button_callback);
    //glfwSetScrollCallback(handle, scroll_callback);
}

void InputManager::poll_events(const double delta_time)
{
    glfwPollEvents();

    for (auto& input : input_actions)
    {
        handle_key_input(input, delta_time);
    }
}

void InputManager::handle_key_input(InputAction& action, const double delta_time)
{
    bool b_pressed = true;
    for (const auto& key : action.key_combination)
    {
        if (!key_state[key.code])
        {
            b_pressed = false;
        }
    }

    if (b_pressed)
    {
        action.b_was_just_released = false;
        if (!action.b_is_pressed)
        {
            action.b_is_pressed       = true;
            action.b_was_just_pressed = true;
        }
        else
        {
            action.b_was_just_pressed = false;
        }
    }
    else
    {
        action.b_was_just_pressed = false;
        if (action.b_is_pressed)
        {
            action.b_is_pressed        = false;
            action.b_was_just_released = true;
        }
        else
        {
            action.b_was_just_released = false;
        }
    }

    if (action.b_was_just_pressed)
    {
        action.pressed_event.Execute(action, delta_time);
    }
    if (action.b_was_just_released)
    {
        action.released_event.Execute(action, delta_time);
    }
    if (action.b_is_pressed)
    {
        action.press_event.Execute(action, delta_time);
    }
}