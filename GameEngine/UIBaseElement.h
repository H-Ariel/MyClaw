#pragma once

#include "GameEngineFramework.hpp"


/// <summary>
/// Represents a basic UI element with position and rendering logic.
/// </summary>
class UIBaseElement
{
public:
	/// <summary>
	/// Constructs a UI element with an optional position.
	/// </summary>
	/// <param name="position">The position of the element (default is (0, 0)).</param>
	UIBaseElement(D2D1_POINT_2F position = {});

	/// <summary>
	/// Destructor for cleaning up any resources.
	/// </summary>
	virtual ~UIBaseElement();

	/// <summary>
	/// Updates the logic of the UI element, typically used for animations or state changes.
	/// </summary>
	/// <param name="elapsedTime">The time elapsed since the last update, in milliseconds.</param>
	virtual void Logic(uint32_t elapsedTime); // in milliseconds

	/// <summary>
	/// Renders the UI element on the screen.
	/// </summary>
	virtual void Draw();

	/// <summary>
	/// Gets the rectangle that represents the bounds of the UI element.
	/// </summary>
	/// <returns>The rectangle that defines the bounds of the element.</returns>
	virtual Rectangle2D GetRect();


	/// <summary>
	/// The position of the element (x, y coordinates).
	/// </summary>
	D2D1_POINT_2F position;
};
