

#include "camera_basic_controller.h"

#include "ios/input_manager.h"
#include "scene/node_camera.h"

CameraBasicController::CameraBasicController(const std::shared_ptr<Camera>& in_camera, InputManager* input_manager)
    : controlled_camera(in_camera)
{
    input_manager->add_input(InputAction("camera_move_forward", {GLFW_KEY_W}));
    input_manager->add_input(InputAction("camera_move_backward", {GLFW_KEY_S}));
    input_manager->add_input(InputAction("camera_move_right", {GLFW_KEY_D}));
    input_manager->add_input(InputAction("camera_move_left", {GLFW_KEY_A}));
    input_manager->add_input(InputAction("camera_move_up", {GLFW_KEY_SPACE}));
    input_manager->add_input(InputAction("camera_move_down", {GLFW_KEY_LEFT_SHIFT}));

    input_manager->get_input("camera_move_forward")->press_event.Add(this, &CameraBasicController::move_forward);
    input_manager->get_input("camera_move_backward")->press_event.Add(this, &CameraBasicController::move_backward);
    input_manager->get_input("camera_move_left")->press_event.Add(this, &CameraBasicController::move_right);
    input_manager->get_input("camera_move_right")->press_event.Add(this, &CameraBasicController::move_left);
    input_manager->get_input("camera_move_up")->press_event.Add(this, &CameraBasicController::move_up);
    input_manager->get_input("camera_move_down")->press_event.Add(this, &CameraBasicController::move_down);

}

double speed = 0.003;

void CameraBasicController::move_forward(const InputAction& input_action)
{
    controlled_camera->set_relative_position(controlled_camera->get_relative_position() + controlled_camera->get_forward_vector() * speed);

}

void CameraBasicController::move_backward(const InputAction& input_action)
{
    controlled_camera->set_relative_position(controlled_camera->get_relative_position() + controlled_camera->get_forward_vector() * -speed);
}

void CameraBasicController::move_right(const InputAction& input_action)
{
    controlled_camera->set_relative_position(controlled_camera->get_relative_position() + controlled_camera->get_right_vector() * speed);
}

void CameraBasicController::move_left(const InputAction& input_action)
{
    controlled_camera->set_relative_position(controlled_camera->get_relative_position() + controlled_camera->get_right_vector() * -speed);
}

void CameraBasicController::move_up(const InputAction& input_action)
{
    controlled_camera->set_relative_position(controlled_camera->get_relative_position() + controlled_camera->get_up_vector() * -speed);
}

void CameraBasicController::move_down(const InputAction& input_action)
{
    controlled_camera->set_relative_position(controlled_camera->get_relative_position() + controlled_camera->get_up_vector() * speed);
}
