#pragma once
#include "node_base.h"

class Scene;

class PrimitiveNode : public Node
{
  public:
    PrimitiveNode() = default;
    virtual ~PrimitiveNode() = default;

    void set_visible(bool b_visible);

    virtual void render(VkCommandBuffer& command_buffer, size_t image_index) = 0;
    
  private:
    glm::dmat4   render_matrix = glm::dmat4();
    bool         is_visible    = false;
};