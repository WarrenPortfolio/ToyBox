#pragma once

class Application
{
public:
	static Application& Current();

public:
	void Run();
	void Shutdown();

private:
	Application() = default;
	Application(const Application&) = delete;
	~Application() = default;

private:
	bool mShouldExit = false;
};
