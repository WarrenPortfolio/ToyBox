#include "Application.h"
#include <Graphics/Renderer.h>

#define GLFW_INCLUDE_NONE
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <chrono>

constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 800;

Application& Application::Current()
{
	static Application sApplcation;
	return sApplcation;
}

void Application::Shutdown()
{
	mShouldExit = true;
}

void Application::Run()
{
	// create window
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	mMainWindow = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "ToyBox - Vulkan Renderer", nullptr, nullptr);

	// create graphics
	Renderer* renderer = new Renderer();
	renderer->Startup();

	// On Windows, steady_clock is now based on QueryPerformanceCounter()
	// https://docs.microsoft.com/en-us/cpp/standard-library/chrono
	using TimePoint = std::chrono::steady_clock::time_point;
	const TimePoint startTime = std::chrono::steady_clock::now();
	TimePoint previosFrameTime = startTime;
	TimePoint currentFrameTime = startTime;

	// application loop
	while (mShouldExit == false && !glfwWindowShouldClose(mMainWindow))
	{
		// Poll and handle events (inputs, window resize, etc.)
		// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
		// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
		// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
		// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
		glfwPollEvents();

		// get current and previous frame time
		previosFrameTime = currentFrameTime;
		currentFrameTime = std::chrono::steady_clock::now();

		// calculate delta time in seconds
		float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentFrameTime - previosFrameTime).count();

		renderer->FrameUpdate(deltaTime);
		renderer->FrameRender();
		renderer->FramePresent();
	}

	// destroy graphics
	renderer->Shutdown();
	delete renderer;

	// destroy window
	glfwDestroyWindow(mMainWindow);
	glfwTerminate();
}