#pragma once
#include <string>
#include <unordered_set>
#include <vector>

class IEngineInterface;
class Window;
class WindowBase;

class WindowManager final
{
    friend Window;
    friend WindowBase;

  public:
    ~WindowManager();
    void draw();

  private:

    void add_window(WindowBase* window);
    void remove_window(WindowBase* window);

    std::vector<WindowBase*>   windows;
    std::unordered_set<size_t> window_ids;
};

class WindowBase
{
    friend WindowManager;

  public:
    WindowBase(IEngineInterface* context, const std::string& name, WindowBase* parent = nullptr);

    void close();

    [[nodiscard]] IEngineInterface* get_context() const { return window_context; }

  protected:
    virtual ~WindowBase();

    virtual void draw_content() = 0;

  private:
    void draw();
    bool open;

    std::string window_name;
    size_t      window_id;

    IEngineInterface*        window_context = nullptr;
    WindowBase*              parent_window  = nullptr;
    std::vector<WindowBase*> children;
};

class DemoWindow : public WindowBase
{
  public:
    using WindowBase::WindowBase;

  protected:
    void draw_content() override;
};