/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#if defined(SCENE_VIEWER)

#include <sa/Compiler.h>
extern "C" {
#include <GL/glew.h>
}
PRAGMA_WARNING_PUSH
PRAGMA_WARNING_DISABLE_MSVC(4505)
#include <GL/freeglut.h>
PRAGMA_WARNING_POP

#include <absmath/Vector3.h>
#include "Game.h"
#include <absmath/Point.h>
#include <eastl.hpp>

namespace Debug {

class Camera
{
public:
    Camera();

    void Resize(int w, int h);
    Math::Matrix4 GetMatrix();

    Math::Transformation transformation_;
    Math::Quaternion rotation_;
    float near_{ 0.1f };
    float far_{ 20000.0f };
    float fov_{ 90.0f };
    float zoom_{ 1.0f };
};

class SceneViewer : public std::enable_shared_from_this<SceneViewer>
{
private:
    bool running_{ false };
    bool initialized_{ false };
    static SceneViewer* instance_;
    bool mouseLook_{ false };
    Math::Point<int> mousePos_{ 0, 0 };
    float cameraDistance_;
    Camera camera_;
    ea::weak_ptr<Game::Game> game_;
    int menuId_{ 0 };
    GLuint vertexShader_;
    GLuint fragmentShader_;
    GLuint shaderProgram_;
    float ratio_;
    float yaw_;
    float pitch_;
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
    void DrawObject(const Game::GameObject& object);
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
