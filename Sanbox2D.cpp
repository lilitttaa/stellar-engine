#include <iostream>

#include "Application.h"
#include "PathManager.h"
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "Render/Light.h"
#include "Core/Render/Buffer.h"
#include "Core/Render/ShaderDataType.h"
#include "Core/Render/Texture2D.h"
#include "Core/Render/VertexArray.h"
#include "Event/Event.h"
#include "Render/Renderer2D.h"
#include "Source/Editor/UI/ImguiPanel.h"

using std::cout;
using std::endl;
using namespace ST;

class SandBox2D : public ST::Application {
public:
	SandBox2D(): ST::Application() {}

	~SandBox2D() override {}

	virtual void Init() override {
		Application::Init();

		//_render2D=ST_MAKE_REF<Renderer2D>(_window.get());
	}

	virtual void Tick(float deltaTime) override {
		Application::Tick(deltaTime);
		//ST_LOG("Tick sandbox! %f \n",deltaTime);
	}

	virtual void Render(float deltaTime) override {

		Application::Render(deltaTime);
	}

	virtual void Destroy() override {
		Application::Destroy();
	}

private:
	ST_REF<Renderer2D> _render2D;

};

ST::Application* CreateApplication() {
	return new SandBox2D();
}
