#pragma once
#include "ios/input_manager.h"

#include <memory>

class Camera;

class CameraBasicController
{
  public:

      CameraBasicController(const std::shared_ptr<Camera>& in_camera, InputManager* input_manager);


      void move_forward(const InputAction& input_action);
      void move_backward(const InputAction& input_action);
      void move_right(const InputAction& input_action);
      void move_left(const InputAction& input_action);
      void move_up(const InputAction& input_action);
      void move_down(const InputAction& input_action);

  private:
    std::shared_ptr<Camera> controlled_camera;
};