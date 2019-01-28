#include "stdafx.h"
#include "SceneViewer.h"
#include "UuidUtils.h"
#include "Subsystems.h"
#include "GameManager.h"
#include "Scheduler.h"
#include "Dispatcher.h"
#include <cassert>
#include "Player.h"
#include "MathUtils.h"

// http://www.alecjacobson.com/weblog/?p=4307

namespace Debug {

const float CAMERA_MIN_DIST = 0.0f;
const float CAMERA_INITIAL_DIST = 1.0f;
const float CAMERA_MAX_DIST = 40.0f;

SceneViewer* SceneViewer::instance_ = nullptr;

// Shader sources
const char* vertexShaderString = R"glsl(
#version 330 core
uniform mat4 proj;
uniform mat4 model;
in vec3 position;
void main()
{
  gl_Position = proj * model * vec4(position, 1.0);
}
)glsl";
const char* fragmentShaderString = R"glsl(
#version 330 core
out vec4 color;
void main()
{
  color = vec4(1.0, 1.0, 1.0, 1.0);
}
)glsl";

SceneViewer::SceneViewer() :
    running_(false),
    initialized_(false),
    menuId_(0),
    yaw_(0.0f),
    pitch_(0.0f),
    cameraDistance_(CAMERA_INITIAL_DIST),
    mouseLook_(false),
    mousePos_({0, 0})
{
    assert(SceneViewer::instance_ == nullptr);
    SceneViewer::instance_ = this;
}

SceneViewer::~SceneViewer()
{
    Stop();
    SceneViewer::instance_ = nullptr;
}

void SceneViewer::StaticRenderScene()
{
    assert(SceneViewer::instance_ != nullptr);
    SceneViewer::instance_->Render();
}

void SceneViewer::StaticChangeSize(GLsizei w, GLsizei h)
{
    assert(SceneViewer::instance_ != nullptr);
    SceneViewer::instance_->ChangeSize(w, h);
}

void SceneViewer::StaticMouse(int button, int state, int x, int y)
{
    assert(SceneViewer::instance_ != nullptr);
    SceneViewer::instance_->Mouse(button, state, x, y);
}

void SceneViewer::StaticMouseMove(int x, int y)
{
    assert(SceneViewer::instance_ != nullptr);
    SceneViewer::instance_->MouseMove(x, y);
}

void SceneViewer::StaticMouseWheel(int button, int dir, int x, int y)
{
    assert(SceneViewer::instance_ != nullptr);
    SceneViewer::instance_->MouseWheel(button, dir, x, y);
}

void SceneViewer::StaticMenu(int id)
{
    assert(SceneViewer::instance_ != nullptr);
    SceneViewer::instance_->Menu(id);
}

void SceneViewer::StaticKeyboard(unsigned char key, int x, int y)
{
    assert(SceneViewer::instance_ != nullptr);
    SceneViewer::instance_->Keyboard(key, x, y);
}

void SceneViewer::Update()
{
    if (!running_)
        return;

    if (initialized_)
    {
        if (GetSubsystem<Game::GameManager>()->GetGameCount() != 0)
        {
            auto games = GetSubsystem<Game::GameManager>()->GetGames();
            game_ = (*games.begin()).second;
        }

        if (std::shared_ptr<Game::Game> g = game_.lock())
        {
            if (g->GetPlayerCount() != 0)
            {
                auto p = g->GetPlayers().begin()->second;

                // Get camera look at dir from character yaw + pitch
                Math::Quaternion rot = Math::Quaternion(Math::Vector3::UnitY, yaw_);
                Math::Quaternion dir = rot * Math::Quaternion(Math::Vector3::UnitX, pitch_);
                Math::Vector3 aimPoint;
                static const Math::Vector3 CAM_POS(0.0f, 1.0f, 1.5f);
                aimPoint = p->transformation_.position_ + CAM_POS;// +rot * CAM_POS;
                Math::Vector3 rayDir = dir * Math::Vector3::Back;

                camera_.transformation_.position_ = (aimPoint + rayDir * cameraDistance_);
                camera_.rotation_ = dir;
            }
        }
        glutMainLoopStep();
        Render();
    }

    GetSubsystem<Asynch::Scheduler>()->Add(
        Asynch::CreateScheduledTask(20, std::bind(&SceneViewer::Update, shared_from_this()))
    );
}

void SceneViewer::UpdateMenu()
{
    if (GetSubsystem<Game::GameManager>()->GetGameCount() == 0)
        return;

    auto games = GetSubsystem<Game::GameManager>()->GetGames();
    for (const auto& game : games)
    {
        std::string name = game.second->GetName() + " (" + game.second->instanceData_.uuid + ")";
        glutAddMenuEntry(name.c_str(), game.first);
    }
}

void SceneViewer::InternalInitialize()
{

    int argc = 1;
    char *argv[1] = { (char*)"Something" };
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitContextVersion(3, 2);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);

    glutInitWindowSize(768, 480);
    glutCreateWindow("Scene Viewer");
    ratio_ = 768.0f / 480.0f;
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    glewInit();

    // Compile each shader
    const auto& compile_shader = [](const GLint type, const char* str) -> GLuint
    {
        GLuint id = glCreateShader(type);
        glShaderSource(id, 1, &str, NULL);
        glCompileShader(id);
        return id;
    };
    GLuint vid = compile_shader(GL_VERTEX_SHADER, vertexShaderString);
    GLuint fid = compile_shader(GL_FRAGMENT_SHADER, fragmentShaderString);
    // attach shaders and link
    shaderProgram_ = glCreateProgram();
    glAttachShader(shaderProgram_, vid);
    glAttachShader(shaderProgram_, fid);
    glLinkProgram(shaderProgram_);
    GLint status;
    glGetProgramiv(shaderProgram_, GL_LINK_STATUS, &status);
    glDeleteShader(vid);
    glDeleteShader(fid);

    glutDisplayFunc(SceneViewer::StaticRenderScene);
    glutReshapeFunc(SceneViewer::StaticChangeSize);
    glutMouseFunc(SceneViewer::StaticMouse);
    glutMouseWheelFunc(SceneViewer::StaticMouseWheel);
    glutKeyboardFunc(SceneViewer::StaticKeyboard);
    glutMotionFunc(SceneViewer::StaticMouseMove);

    menuId_ = glutCreateMenu(SceneViewer::StaticMenu);
    glutSetMenu(menuId_);
    glutAttachMenu(GLUT_RIGHT_BUTTON);

    initialized_ = true;
}

void SceneViewer::DrawScene()
{
    if (auto g = game_.lock())
    {
        glUseProgram(shaderProgram_);
        // select program and attach uniforms
        GLint proj_loc = glGetUniformLocation(shaderProgram_, "proj");
        Math::Matrix4 matrix = camera_.GetMatrix();
        // https://www.gamedev.net/forums/topic/698812-leftright-coordinate-system-and-rotation/?tab=comments#comment-5390101
        matrix.Scale(Math::Vector3(1.0f, 1.0f, 2.0f));
        matrix.Translate(Math::Vector3(0.0f, 0.0f, -1.0f));
        glUniformMatrix4fv(proj_loc, 1, GL_FALSE, matrix.Transpose().Data());

        auto objects = g->GetObjects();
        for (const auto& object : objects)
        {
            DrawObject(object);
        }
    }
}

void SceneViewer::DrawObject(const std::shared_ptr<Game::GameObject>& object)
{
    auto collShape = object->GetCollisionShape();
    if (!collShape)
        return;
    const Math::Transformation& trans = object->transformation_;
    Math::Shape s = collShape->GetShape();
    if (s.indexCount_ == 0)
        return;

    Math::Matrix4 matrix;
    if (collShape->shapeType_ == Math::ShapeTypeBoundingBox)
    {
        // There is 1 special case, an oriented BB
        using BBoxShape = Math::CollisionShapeImpl<Math::BoundingBox>;
        BBoxShape* shape = static_cast<BBoxShape*>(collShape);
        if (shape->shape_->IsOriented())
            matrix = trans.GetMatrix(shape->shape_->orientation_);
        else
            matrix = trans.GetMatrix();
    }
    else
        matrix = trans.GetMatrix();
    // https://www.gamedev.net/forums/topic/698812-leftright-coordinate-system-and-rotation/?tab=comments#comment-5390101
    matrix.Scale(Math::Vector3(-1.0f, 1.0f, 1.0f));

    // Generate and attach buffers to vertex array
    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    GLuint VBO, EBO;
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * s.VertexDataSize(), s.VertexData(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * s.indexCount_, s.indexData_.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    GLint model_loc = glGetUniformLocation(shaderProgram_, "model");
    glUniformMatrix4fv(model_loc, 1, GL_FALSE, matrix.Transpose().Data());

    // Draw mesh as wireframe
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, s.indexCount_, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);
}

void SceneViewer::Mouse(int button, int state, int x, int y)
{
    switch (button)
    {
    case GLUT_LEFT_BUTTON:
        if (state == GLUT_UP)
        {
            mouseLook_ = false;
        }
        else if (state == GLUT_DOWN)
        {
            mousePos_.x_ = x;
            mousePos_.y_ = y;
            mouseLook_ = true;
        }
        break;
    case GLUT_RIGHT_BUTTON:
        if (state == GLUT_DOWN)
        {
            yaw_ = 0.0f;
            pitch_ = 0.0f;
        }
        break;
    default:
        break;
    }
}

void SceneViewer::MouseMove(int x, int y)
{
    if (mouseLook_)
    {
        yaw_ += (float)(mousePos_.x_ - x) * 0.001f;
        pitch_ += (float)(mousePos_.y_ - y) * 0.001f;
        mousePos_.x_ = x;
        mousePos_.y_ = y;
    }
}

void SceneViewer::MouseWheel(int, int dir, int, int)
{
    if (dir > 0)
        cameraDistance_ -= 0.1f;
    else
        cameraDistance_ += 0.1f;
    cameraDistance_ = Math::Clamp(cameraDistance_, CAMERA_MIN_DIST, CAMERA_MAX_DIST);
}

void SceneViewer::Menu(int id)
{
    if (id == menuId_)
        UpdateMenu();
}

void SceneViewer::Keyboard(unsigned char /* key */, int, int)
{
/*    switch (key)
    {
    case 'w':
    case 'W':
        camera_.transformation_.position_.z_ += 0.5f;
        break;
    case 's':
    case 'S':
        camera_.transformation_.position_.z_ -= 0.5f;
        break;
    case 'a':
    case 'A':
        camera_.transformation_.position_.x_ -= 0.5f;
        break;
    case 'd':
    case 'D':
        camera_.transformation_.position_.x_ += 0.5f;
        break;
    case 'q':
    case 'Q':
        camera_.transformation_.position_.y_ += 0.5f;
        break;
    case 'y':
    case 'Y':
        camera_.transformation_.position_.y_ -= 0.5f;
        break;
    }*/
}

bool SceneViewer::Initialize()
{
    //  Create the window on the thread you want to process its messages.
    GetSubsystem<Asynch::Dispatcher>()->Add(
        Asynch::CreateTask(std::bind(&SceneViewer::InternalInitialize, shared_from_this()))
    );
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(500ms);

    return true;
}

void SceneViewer::Run()
{
    running_ = true;

    glutMainLoopEnter();
    GetSubsystem<Asynch::Scheduler>()->Add(
        Asynch::CreateScheduledTask(20, std::bind(&SceneViewer::Update, shared_from_this()))
    );
}

void SceneViewer::Stop()
{
    initialized_ = false;
    glutMainLoopExit();
    glDeleteProgram(shaderProgram_);
    running_ = false;
}

void SceneViewer::Render()
{
    if (!initialized_)
        return;

    //    auto gm = GetSubsystem<Game::GameManager>();
    //    std::shared_ptr<Game::Game> g = gm->GetInstance(SceneViewer::instanceUuid);
    //    if (!g)
    //        return;
    glClearColor(0.0, 0.0, 0.0, 1.0);
    //Clear all the buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    DrawScene();

    glutSwapBuffers();
    glutPostRedisplay();
}

void SceneViewer::ChangeSize(GLsizei w, GLsizei h)
{
    camera_.Resize(w, h);
    ratio_ = static_cast<float>(w) / static_cast<float>(h);
}

Camera::Camera() :
    near_(0.1f),
    far_(20000.f),
    fov_(90.0f),
    zoom_(1.0f),
    rotation_(Math::Quaternion::Identity)
{ }

void Camera::Resize(int w, int h)
{
    glViewport(0, 0, w, h);
}

Math::Matrix4 Camera::GetMatrix()
{
    if (rotation_ != Math::Quaternion::Identity)
        return XMath::XMMatrixInverse(nullptr, transformation_.GetMatrix(rotation_));
    return XMath::XMMatrixInverse(nullptr, transformation_.GetMatrix());
}

}
