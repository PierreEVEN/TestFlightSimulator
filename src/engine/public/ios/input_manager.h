#pragma once
#include "GLFW/glfw3.h"

#include <cpputils/eventmanager.hpp>
#include <string>

class InputAction;
DECLARE_DELEGATE_MULTICAST(InputCall, const InputAction&, const double);

enum class EInputType
{
    keyboard,
    mouse_button,
    mouse_axis,
    scroll_axis,
    joystick_axis
};

struct InputActionReference
{
    int        code;
    EInputType input_type;
};

class InputAction
{
    friend class InputManager;

  public:
    InputAction(const std::string& name, const std::vector<InputActionReference> in_key_combination) : input_name(name), key_combination(in_key_combination)
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
    bool                              b_was_just_pressed  = false;
    bool                              b_was_just_released = false;
    bool                              b_is_pressed        = false;
    std::string                       input_name;
    std::vector<InputActionReference> key_combination;
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
    void        handle_key_input(InputAction& action, const double delta_time);
    void        poll_events(const double delta_time);

  private:
    std::vector<InputAction> input_actions;
};

namespace keyboard
{
constexpr InputActionReference key_0{.code = GLFW_KEY_0, .input_type = EInputType::keyboard};
constexpr InputActionReference key_1{.code = GLFW_KEY_1, .input_type = EInputType::keyboard};
constexpr InputActionReference key_2{.code = GLFW_KEY_2, .input_type = EInputType::keyboard};
constexpr InputActionReference key_3{.code = GLFW_KEY_3, .input_type = EInputType::keyboard};
constexpr InputActionReference key_4{.code = GLFW_KEY_4, .input_type = EInputType::keyboard};
constexpr InputActionReference key_5{.code = GLFW_KEY_5, .input_type = EInputType::keyboard};
constexpr InputActionReference key_6{.code = GLFW_KEY_6, .input_type = EInputType::keyboard};
constexpr InputActionReference key_7{.code = GLFW_KEY_7, .input_type = EInputType::keyboard};
constexpr InputActionReference key_8{.code = GLFW_KEY_8, .input_type = EInputType::keyboard};
constexpr InputActionReference key_9{.code = GLFW_KEY_9, .input_type = EInputType::keyboard};
constexpr InputActionReference key_a{.code = GLFW_KEY_A, .input_type = EInputType::keyboard};
constexpr InputActionReference key_b{.code = GLFW_KEY_B, .input_type = EInputType::keyboard};
constexpr InputActionReference key_c{.code = GLFW_KEY_C, .input_type = EInputType::keyboard};
constexpr InputActionReference key_d{.code = GLFW_KEY_D, .input_type = EInputType::keyboard};
constexpr InputActionReference key_e{.code = GLFW_KEY_E, .input_type = EInputType::keyboard};
constexpr InputActionReference key_f{.code = GLFW_KEY_F, .input_type = EInputType::keyboard};
constexpr InputActionReference key_g{.code = GLFW_KEY_G, .input_type = EInputType::keyboard};
constexpr InputActionReference key_h{.code = GLFW_KEY_H, .input_type = EInputType::keyboard};
constexpr InputActionReference key_i{.code = GLFW_KEY_I, .input_type = EInputType::keyboard};
constexpr InputActionReference key_j{.code = GLFW_KEY_J, .input_type = EInputType::keyboard};
constexpr InputActionReference key_k{.code = GLFW_KEY_K, .input_type = EInputType::keyboard};
constexpr InputActionReference key_l{.code = GLFW_KEY_L, .input_type = EInputType::keyboard};
constexpr InputActionReference key_m{.code = GLFW_KEY_M, .input_type = EInputType::keyboard};
constexpr InputActionReference key_n{.code = GLFW_KEY_N, .input_type = EInputType::keyboard};
constexpr InputActionReference key_o{.code = GLFW_KEY_O, .input_type = EInputType::keyboard};
constexpr InputActionReference key_p{.code = GLFW_KEY_P, .input_type = EInputType::keyboard};
constexpr InputActionReference key_q{.code = GLFW_KEY_Q, .input_type = EInputType::keyboard};
constexpr InputActionReference key_r{.code = GLFW_KEY_R, .input_type = EInputType::keyboard};
constexpr InputActionReference key_s{.code = GLFW_KEY_S, .input_type = EInputType::keyboard};
constexpr InputActionReference key_t{.code = GLFW_KEY_T, .input_type = EInputType::keyboard};
constexpr InputActionReference key_u{.code = GLFW_KEY_U, .input_type = EInputType::keyboard};
constexpr InputActionReference key_v{.code = GLFW_KEY_V, .input_type = EInputType::keyboard};
constexpr InputActionReference key_w{.code = GLFW_KEY_W, .input_type = EInputType::keyboard};
constexpr InputActionReference key_x{.code = GLFW_KEY_X, .input_type = EInputType::keyboard};
constexpr InputActionReference key_y{.code = GLFW_KEY_Y, .input_type = EInputType::keyboard};
constexpr InputActionReference key_z{.code = GLFW_KEY_Z, .input_type = EInputType::keyboard};
constexpr InputActionReference key_left_shift{.code = GLFW_KEY_LEFT_SHIFT, .input_type = EInputType::keyboard};
constexpr InputActionReference key_right_shift{.code = GLFW_KEY_RIGHT_SHIFT, .input_type = EInputType::keyboard};
constexpr InputActionReference key_space{.code = GLFW_KEY_SPACE, .input_type = EInputType::keyboard};
}; // namespace keyboard