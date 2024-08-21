#include "GameMainApp.h"


// We can change it in: Project-Properties -> Linker -> System
#ifdef _DEBUG
int main(int argc, char* argv[])
#else
int APIENTRY wWinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int)
#endif
{
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF); // check for memory leaks
#endif

	try
	{
		GameMainApp app;
		app.init();
		app.run();
	}
	catch (const Exception& ex)
	{
		LogFile::logf(LogFile::Error, "%s", ex.message.c_str());
	}

	return 0;
}
