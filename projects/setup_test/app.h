#include "window.h"

class App_SetupTest
{
public:
	Engine::Window window;

	App_SetupTest();

	void Init();
	void UpdateLoop();
	void Deinit();
};