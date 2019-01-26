#pragma once

extern "C" {
#include <GL/glew.h>
}
#pragma warning(push)
#pragma warning(disable: 4505)
#include <GL/freeglut.h>
#pragma warning(pop)

#include "Vector3.h"
#include "Game.h"

namespace Debug {

class Camera
{
public:
    Camera();

    void Zoom(int dir);
    void Resize(int w, int h);
    void Position();

    Math::Vector3 position_;
    Math::Vector3 rotation_;
    float zoom_;
    float ratio_;
};

class SceneViewer : public std::enable_shared_from_this<SceneViewer>
{
private:
    bool running_;
    bool initialized_;
    static SceneViewer* instance_;
    Camera camera_;
    std::weak_ptr<Game::Game> game_;
    int menuId_;
    GLuint vertexShader_;
    GLuint fragmentShader_;
    GLuint shaderProgram_;
    int projectionModelviewMatrixLoc_;
    static void StaticRenderScene();
    static void StaticChangeSize(GLsizei w, GLsizei h);
    static void StaticMouse(int button, int state, int x, int y);
    static void StaticMouseWheel(int button, int dir, int x, int y);
    static void StaticMenu(int id);
    void Update();
    void UpdateMenu();
    void InternalInitialize();
    void DrawScene();
    void DrawObject(const std::shared_ptr<Game::GameObject>& object);
    void Mouse(int button, int state, int x, int y);
    void MouseWheel(int button, int dir, int x, int y);
    void Menu(int id);
public:
    SceneViewer();
    ~SceneViewer();

    bool Initialize();
    void Run();
    void Stop();

    void Render();
    void ChangeSize(GLsizei w, GLsizei h);
};

}
