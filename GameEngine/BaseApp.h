#pragma once

#include "BaseEngine.h"


/// <summary>
/// Represents the main application, responsible for initialization, window management, and running the engine.
/// </summary>
class BaseApp
{
public:
	/// <summary>
	/// Constructor for initializing the application with an optional window procedure and title.
	/// </summary>
	/// <param name="wndproc">The window procedure (optional). Can be overridden in derived classes.</param>
	/// <param name="title">The title of the application window (optional, default is an empty string).</param>
	BaseApp(WNDPROC wndproc = nullptr, const TCHAR title[] = L"");

	/// <summary>
	/// Destructor for cleaning up resources used by the application.
	/// </summary>
	virtual ~BaseApp();

	/// <summary>
	/// Initializes the application, typically creating and setting up the engine.
	/// Derived classes should override this method to initialize their specific engines.
	/// </summary>
	virtual void init();

	/// <summary>
	/// Starts the application and begins its main loop.
	/// </summary>
	void run();

protected:
	/// <summary>
	/// The main window procedure that processes window messages.
	/// Can be overridden in derived classes, but derived classes should call this method by default.
	/// </summary>
	/// <param name="hWnd">Handle to the window.</param>
	/// <param name="uMsg">The message being sent.</param>
	/// <param name="wParam">Additional message information.</param>
	/// <param name="lParam">Additional message information.</param>
	/// <returns>The result of the message processing (depends on the message type).</returns>
	static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

	/// <summary>
	/// A shared pointer to the engine currently running in the application.
	/// </summary>
	shared_ptr<BaseEngine> _pEngine;

	/// <summary>
	/// A flag indicating whether the application should keep running. Set to false to stop the application.
	/// </summary>
	bool _runApp;


private:
	/// <summary>
	/// Registers the window class for the application. This should be done before creating the window.
	/// </summary>
	/// <param name="wndproc">The window procedure to be used by the window class.</param>
	static void registerMyWindowClass(WNDPROC wndproc);
};
