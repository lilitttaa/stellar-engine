#include <iostream>

#include "Application.h"
#include "PathManager.h"
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "Camera.h"
#include "Render/Light.h"
#include "Core/Render/Buffer.h"
#include "Core/Render/ShaderDataType.h"
#include "Core/Render/Texture2D.h"
#include "Core/Render/VertexArray.h"
#include "Source/Editor/UI/ImguiPanel.h"

using std::cout;
using std::endl;
using namespace ST;

#define PI 3.14159265359

const char* const logo[] =
{
    "eeeeeeeeeeeeeeee",
    "eeeeeeeeeeeeeeee",
    "ee............ee",
    "..00000.0000000.",
    "..0........0....",
    "..0........0....",
    "..0........0....",
    "..00000....0....",
    "......0....0....",
    "......0....0....",
    "......0....0....",
    "..00000....0....",
    "ee............ee",
    "ee............ee",
    "eeeeeeeeeeeeeeee",
    "eeeeeeeeeeeeeeee"
};

static std::ostream& operator<<(std::ostream& os, glm::vec3& vec){
    os << "X: " << vec.x << " Y: " << vec.y << " Z:" << vec.z << std::endl;
    return os;
}

const unsigned char icon_colors[5][4] =
{
    {0  ,0  ,0  ,255},      // black
    {255,0  ,0  ,255},    // red
    {0  ,255,0  ,255},    // green
    {0  ,0  ,255,255},    // blue
    {255,255,255,255} // white
};

static void SetIcon(GLFWwindow* window){
    unsigned char           pixels[16 * 16 * 4];
    unsigned char*          target            = pixels;
    constexpr unsigned char backgroundColor[] = {50,50,50,255};
    constexpr unsigned char iconColor[]       = {255,255,255,255};
    constexpr unsigned char edgeColor[]       = {50,50,50,255};
    const GLFWimage         img               = {16,16,pixels};
    for(int y = 0; y < img.width; y++){
        for(int x = 0; x < img.height; x++){
            if(logo[y][x] == '.') memcpy(target, backgroundColor, 4);
            else if(logo[y][x] == 'e') memcpy(target, edgeColor, 4);
            else memcpy(target, iconColor, 4);
            target += 4;
        }
    }
    glfwSetWindowIcon(window, 1, &img);
}

void ProcessInput(GLFWwindow* window, Camera& camera, float deltaTime){
    if(glfwGetKey(window,GLFW_KEY_W) == GLFW_PRESS){ camera.Move(deltaTime, 0); }
    if(glfwGetKey(window,GLFW_KEY_S) == GLFW_PRESS){ camera.Move(-deltaTime, 0); }
    if(glfwGetKey(window,GLFW_KEY_A) == GLFW_PRESS){ camera.Move(0, -deltaTime); }
    if(glfwGetKey(window,GLFW_KEY_D) == GLFW_PRESS){ camera.Move(0, deltaTime); }
}


class SandBox2D : public ST::Application{
public:
    SandBox2D(): ST::Application(){}

    virtual void Init() override{
        int m_width  = 1500;
        int m_height = 1000;
        glfwInit();
        glfwInitHint(GL_MAJOR_VERSION, 3);
        glfwInitHint(GL_MINOR_VERSION, 3);
        glfwWindowHint(GLFW_DECORATED, GL_FALSE);
        glfwInitHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
        GLFWmonitor*       monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode    = glfwGetVideoMode(monitor);
        m_width                    = mode->width;
        m_height                   = mode->height;
        GLFWwindow* window         = glfwCreateWindow(m_width, m_height, "Stellar", nullptr, nullptr);

        SetIcon(window);

        if(!window){
            std::cout << "Create Window Failed!" << std::endl;
            glfwTerminate();
            return;
        }
        glfwMakeContextCurrent(window);
        glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int length){
            glViewport(0, 0, width, length);
        });
        glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mode){
            if(key == GLFW_KEY_ESCAPE){ glfwSetWindowShouldClose(window,GL_TRUE); }
        });
        glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods){
            if(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS){
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }
            if(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE){
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
        });
        //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        ST::Camera camera(static_cast<float>(m_width) / static_cast<float>(m_height), 1.f, 100.f,
                          glm::vec3(10.0f, 0, 0), 45, glm::vec3(1.0f, 0, 0));
        struct WindowPointer{
            ST::Camera* camera{};

            float lastXPos = 0.0f;

            float lastYPos = 0.0f;

            bool first = true;
        };
        WindowPointer ptr{};
        ptr.camera = &camera;
        glfwSetWindowUserPointer(window, &ptr);
        glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset){
            auto* ptr = static_cast<WindowPointer*>(glfwGetWindowUserPointer(window));
            ptr->camera->SpeedUp(yoffset);
        });
        glfwSetCursorPosCallback(window, [](GLFWwindow* window, const double inXpos, const double inYpos){
            const auto xPos = static_cast<float>(inXpos);
            const auto yPos = static_cast<float>(inYpos);
            auto*      ptr  = static_cast<WindowPointer*>(glfwGetWindowUserPointer(window));

            if(glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS){
                if(ptr->first == false){
                    ptr->camera->Rotate(yPos - ptr->lastYPos, xPos - ptr->lastXPos, true);
                }
                else{ ptr->first = false; }
            }
            else{ ptr->first = true; }

            ptr->lastXPos = xPos;
            ptr->lastYPos = yPos;
        });

        if(!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))){
            std::cout << "Load Glad Failed!" << std::endl;
            return;
        }
        ST::ImguiPanel::Init(window);
        glViewport(0, 0, m_width, m_height);
        glClearColor(0.2f, 0.2f, 0.2f, 1);
        glEnable(GL_DEPTH_TEST);
    
    //  float verts[] = {
    //      // positions          // colors           // texture coords
    //      0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
    //      0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
    //     -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
    //     -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left 
    // };
    /*float verts[] = {
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
    
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    };*/
    
    float verts[] = {
        // positions          // normals           // texture coords
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
    
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
    
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
    
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
    
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
    
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
    };
    const glm::vec3 cubePositions[] = {
        glm::vec3( 0.0f,  0.0f,  0.0f),
        glm::vec3( 2.0f,  5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3( 2.4f, -0.4f, -3.5f),
        glm::vec3(-1.7f,  3.0f, -7.5f),
        glm::vec3( 1.3f, -2.0f, -2.5f),
        glm::vec3( 1.5f,  2.0f, -2.5f),
        glm::vec3( 1.5f,  0.2f, -1.5f),
        glm::vec3(-1.3f,  1.0f, -1.5f)
    };
    unsigned int indexs[] = {
        0,1,3,
        1,2,3
    };

    ST::VertexArray boxVAO;
    const auto      vertexBuffer = MAKE_REF<ST::VertexBuffer>(verts, sizeof(verts));
    //const auto indexBuffer = MakeRef<IndexBuffer>(indexs,sizeof(indexs));
    vertexBuffer->SetLayout({
        {ST::ShaderDataType::Float3,"v_Pos"},
        //{ShaderDataType::Float3,"v_Color"},
        {ST::ShaderDataType::Float3,"v_Normal"},
        {ST::ShaderDataType::Float2,"v_TexCoord"},

    });
    boxVAO.AddVertexBuffer(vertexBuffer);
    //vertexArray.SetIndexBuffer(indexBuffer);


    ST::Texture2D normalTexture(ST::PathManager::GetResourcePath() + "NoManSky.jpg", 0);
    Texture2D     specularTexture(PathManager::GetResourcePath() + "NoManSky.jpg", 1);

    Shader boxShader(PathManager::GetResourcePath() + R"(OpenGLShader/Shader.vt.glsl)",
                     PathManager::GetResourcePath() + R"(OpenGLShader/Shader.fg.glsl)");
    boxShader.UseShader();
    boxShader.SetInt("f_Texture", 0);
    glm::mat4 projTrans = camera.CalculatePrjMat();
    boxShader.SetMat4("v_Proj", projTrans);

    Shader lightShader(PathManager::GetResourcePath() + R"(OpenGLShader/Lighting.vt.glsl)",
                       PathManager::GetResourcePath() + R"(OpenGLShader/Lighting.fg.glsl)");
    lightShader.UseShader();
    lightShader.SetMat4("v_Proj", projTrans);

    PointLight pointLight(glm::vec3(-4.0f, 4.0f, 0), glm::vec3(0.2f), glm::vec3(0.5f),
                          glm::vec3(0.7f), 1.0f, 0.014f, 0.0007f);
    DirLight dirLight(glm::vec3(-0.2f, -1.0f, -0.3f), glm::vec3(0.2f), glm::vec3(0.5f),
                      glm::vec3(0.7f));

    double lastTime = 0.0, currentTime = 0.0;

    ImguiPanel::Close();
    }

    virtual void Tick(float deltaTime) override{ cout << "Tick sandbox! " << deltaTime << endl; }

    virtual void Render(float deltaTime) override{
        ImguiPanel::NewFrame();
        glClear(GL_COLOR_BUFFER_BIT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glfwPollEvents();
        boxShader.UseShader();
        boxVAO.Bind();

        ProcessInput(window, camera, static_cast<float>(deltaTime));


        ImguiPanel::CreatePointLightPanel("PointLight", pointLight);
        ImguiPanel::CreateDirLightPanel("DirLight", dirLight);

        //ImguiPanel::ShowDemoPanel();
        //ImGui::ShowDebugLogWindow();
        //ImGui::ShowStackToolWindow();

        const glm::mat4 viewTrans = camera.CalculateLookAtMat();
        boxShader.UseShader();
        boxShader.SetMat4("v_View", viewTrans);
        boxShader.SetVec3("f_EyePos", camera.GetPos());
        pointLight.SetUniform(boxShader);
        dirLight.SetUniform(boxShader);

        boxShader.SetInt("f_Material.f_Ka", 0);
        boxShader.SetInt("f_Material.f_Kd", 0);
        boxShader.SetInt("f_Material.f_Ks", 1);
        boxShader.SetFloat("f_Material.f_Shinness", 64);


        for(int i = 0; i < 10; ++i){
            glm::mat4 modelTrans(1.0);
            modelTrans = glm::rotate(modelTrans, glm::radians(static_cast<float>(glfwGetTime()) * 0.0f),
                                     glm::vec3(1.0f, 1.0f, 0));
            modelTrans = glm::translate(modelTrans, cubePositions[i]);
            boxShader.SetMat4("v_Model", modelTrans);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glm::mat4 modelTrans(1.0);
        modelTrans = glm::translate(modelTrans, pointLight.m_Pos);
        lightShader.UseShader();
        lightShader.SetMat4("v_Model", modelTrans);
        lightShader.SetMat4("v_View", viewTrans);
        lightShader.SetVec3("f_LightColor", glm::vec3(1, 1, 1));
        glDrawArrays(GL_TRIANGLES, 0, 36);
        // glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,0);


        ImguiPanel::Render();
        glfwSwapBuffers(window);
    }

    virtual void Destroy() override{}

};

ST::Application* CreateApplication(){ return new SandBox2D(); }
