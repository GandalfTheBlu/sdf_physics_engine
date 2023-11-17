#include "app.h"
#include "debug.h"

int main()
{
	App_SetupTest app;
	
	if (!TRY({
		app.Init();
		app.UpdateLoop();
		app.Deinit();
	}))
	{
		return 1;
	}

	return 0;
}