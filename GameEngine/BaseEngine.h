#pragma once

#include "UIBaseElement.h"

/// <summary>
/// Base class for engine management, handling events and rendering logic for a game or UI.
/// </summary>
class BaseEngine
{
public:
    /// <summary>
    /// Constructor for initializing the engine state.
    /// </summary>
    BaseEngine();

    /// <summary>
    /// Destructor for cleaning up resources used by the engine.
    /// </summary>
    virtual ~BaseEngine();

    /// <summary>
    /// Called when a key is released.
    /// </summary>
    /// <param name="key">The key code of the key that was released.</param>
    virtual void OnKeyUp(int key);

    /// <summary>
    /// Called when a key is pressed.
    /// </summary>
    /// <param name="key">The key code of the key that was pressed.</param>
    virtual void OnKeyDown(int key);

    /// <summary>
    /// Called when a mouse button is released.
    /// </summary>
    /// <param name="btn">The mouse button that was released (e.g., left or right button).</param>
    virtual void OnMouseButtonUp(MouseButtons btn);

    /// <summary>
    /// Called when a mouse button is pressed.
    /// </summary>
    /// <param name="btn">The mouse button that was pressed (e.g., left or right button).</param>
    virtual void OnMouseButtonDown(MouseButtons btn);

    /// <summary>
    /// Called when the window is resized.
    /// </summary>
    virtual void OnResize();

    /// <summary>
    /// Performs the logic for this frame. Typically used for updating game state, animations, etc.
    /// </summary>
    /// <param name="elapsedTime">Time elapsed since the last frame, in milliseconds.</param>
    virtual void Logic(uint32_t elapsedTime);

    /// <summary>
    /// Renders the current state of the engine to the screen.
    /// </summary>
    virtual void Draw();

    /// <summary>
    /// Gets the next engine to transition to.
    /// </summary>
    /// <returns>A shared pointer to the next engine.</returns>
    shared_ptr<BaseEngine> getNextEngine();

    /// <summary>
    /// The current mouse position, in screen coordinates (x, y).
    /// </summary>
    D2D1_POINT_2U mousePosition;

    /// <summary>
    /// If true, the engine will stop execution after the current frame.
    /// </summary>
    bool stopEngine;

    /// <summary>
    /// If true, the screen will be cleared before each frame is drawn.
    /// </summary>
    bool clearScreen;

protected:
    /// <summary>
    /// Changes the current engine to a new one, transitioning with optional arguments.
    /// </summary>
    /// <typeparam name="NextEngine">The type of the next engine.</typeparam>
    /// <typeparam name="Args">The types of arguments required by the new engine's constructor.</typeparam>
    /// <param name="args">Arguments to pass to the next engine's constructor.</param>
    template <class NextEngine, class... Args>
    void changeEngine(Args... args)
    {
        _nextEngine = make_shared<NextEngine>(args...);
        stopEngine = true;
    }

    /// <summary>
    /// A list of UI elements that are part of the engine's current state.
    /// </summary>
    vector<UIBaseElement*> _elementsList;

    /// <summary>
    /// A shared pointer to the next engine that will be used after the current one ends.
    /// </summary>
    shared_ptr<BaseEngine> _nextEngine;
};
