#pragma once
#include "ios/input_manager.h"

#include <memory>

class Camera;

class CameraBasicController
{
  public:
    CameraBasicController(const std::shared_ptr<Camera>& in_camera, InputManager* input_manager);

  private:
    std::shared_ptr<Camera> controlled_camera;
    double                  movement_speed = 1000.0;

    double pitch = 0.0;
    double yaw   = 0.0;
};