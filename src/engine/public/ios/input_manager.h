#pragma once
#include "GLFW/glfw3.h"

#include <cpputils/eventmanager.hpp>
#include <string>

class InputAction;
DECLARE_DELEGATE_MULTICAST(InputCall, const InputAction&);

class InputAction
{
    friend class InputManager;

  public:
    InputAction(const std::string& name, const std::vector<size_t> in_key_combination) : input_name(name), key_combination(in_key_combination)
    {
    }

    InputCall press_event;
    InputCall pressed_event;
    InputCall released_event;

    [[nodiscard]] bool is_pressed() const
    {
        return b_is_pressed;
    }

    [[nodiscard]] bool was_just_pressed() const
    {
        return b_was_just_pressed;
    }

    [[nodiscard]] bool was_just_released() const
    {
        return b_was_just_released;
    }

    [[nodiscard]] std::string name() const
    {
        return input_name;
    }

  private:
    bool                b_was_just_pressed  = false;
    bool                b_was_just_released = false;
    bool                b_is_pressed        = false;
    std::string         input_name;
    std::vector<size_t> key_combination;
};

class InputManager
{
  public:
    InputManager(GLFWwindow* window_handle)
    {
        configure(window_handle);
    }

    void add_input(const InputAction& new_input);

    [[nodiscard]] InputAction* get_input(const std::string& input_name);

    static void configure(GLFWwindow* handle);
    void        handle_key_input(InputAction& action);
    void        poll_events();

  private:
    std::vector<InputAction> input_actions;
};