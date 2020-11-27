#pragma once

struct GLFWwindow;

class Application
{
public:
	static Application& Current();

public:
	void Run();
	void Shutdown();

	GLFWwindow* MainWindow() const { return mMainWindow; }

private:
	Application() = default;
	Application(const Application&) = delete;
	~Application() = default;

private:
	bool mShouldExit = false;
	GLFWwindow* mMainWindow = nullptr;
};
