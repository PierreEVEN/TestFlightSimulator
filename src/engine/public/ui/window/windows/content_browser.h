#pragma once
#include "ui/window/window_base.h"

class ContentBrowser : public WindowBase
{
  public:
    using WindowBase::WindowBase;

  protected:
    void draw_content() override;
};
