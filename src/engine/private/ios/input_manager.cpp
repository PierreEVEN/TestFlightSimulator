

#include "ios/input_manager.h"

#include "cpputils/logger.hpp"

static int key_state[GLFW_KEY_LAST];

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
    glfwSetKeyCallback(handle, key_callback);
}

void InputManager::poll_events()
{
    glfwPollEvents();

    for (auto& input : input_actions)
    {
        handle_key_input(input);
    }
}

void InputManager::handle_key_input(InputAction& action)
{
    bool b_pressed = true;
    for (const auto& key : action.key_combination)
    {
        if (!key_state[key])
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
        action.pressed_event.Execute(action);
    }
    if (action.b_was_just_released)
    {
        action.released_event.Execute(action);
    }
    if (action.b_is_pressed)
    {
        action.press_event.Execute(action);
    }
}