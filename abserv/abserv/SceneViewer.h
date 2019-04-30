#pragma once

#if defined(SCENE_VIEWER)

extern "C" {
#include <GL/glew.h>
}
#pragma warning(push)
#pragma warning(disable: 4505)
#include <GL/freeglut.h>
#pragma warning(pop)

#include "Vector3.h"
#include "Game.h"
#include "Point.h"

namespace Debug {

class Camera
{
public:
    Camera();

    void Resize(int w, int h);
    Math::Matrix4 GetMatrix();

    Math::Transformation transformation_;
    Math::Quaternion rotation_;
    float near_;
    float far_;
    float fov_;
    float zoom_;
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
    float ratio_;
    float yaw_;
    float pitch_;
    float cameraDistance_;
    bool mouseLook_;
    Math::Point<int> mousePos_;
    GLuint VAO_;
    static void StaticRenderScene();
    static void StaticChangeSize(GLsizei w, GLsizei h);
    static void StaticMouse(int button, int state, int x, int y);
    static void StaticMouseMove(int x, int y);
    static void StaticMouseWheel(int button, int dir, int x, int y);
    static void StaticMenu(int id);
    static void StaticKeyboard(unsigned char key, int x, int y);
    void Update();
    void UpdateMenu();
    void InternalInitialize();
    void DrawScene();
    void DrawObject(const std::shared_ptr<Game::GameObject>& object);
    void Mouse(int button, int state, int x, int y);
    void MouseMove(int x, int y);
    void MouseWheel(int button, int dir, int x, int y);
    void Menu(int id);
    void Keyboard(unsigned char key, int x, int y);
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

#endif
