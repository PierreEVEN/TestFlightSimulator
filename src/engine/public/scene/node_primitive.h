#pragma once
#include "node_base.h"
#include "scene.h"
#include "rendering/window.h"

class Scene;

class PrimitiveNode : public Node
{
  public:
    PrimitiveNode() = default;
    virtual ~PrimitiveNode() = default;

    void set_visible(bool b_visible);

    virtual void render(RenderContext render_context) = 0;
    
  private:
    glm::dmat4   render_matrix = glm::dmat4();
    bool         is_visible    = false;
};