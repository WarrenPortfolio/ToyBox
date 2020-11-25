#include "Application.h"
#include <Graphics/Renderer.h>

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
	// create graphics
	Renderer* renderer = new Renderer();
	renderer->Startup();

	// application loop
	while (mShouldExit == false)
	{
		renderer->DrawFrame();
	}

	// destroy graphics
	renderer->Shutdown();
	delete renderer;
}