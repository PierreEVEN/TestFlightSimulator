#pragma once
#include "ui/window/windowBase.h"


class ContentBrowser : public WindowBase
{
public:
	using WindowBase::WindowBase;
protected:
	void draw_content() override;	
};
