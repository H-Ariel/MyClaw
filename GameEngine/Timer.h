#pragma once

#include <functional>

/// <summary>
/// Timer class for scheduling a callback after a specified delay in milliseconds.
/// </summary>
class Timer {
public:
	/// <summary>
	/// Constructs a Timer with a duration and a callback function.
	/// </summary>
	/// <param name="cb">Callback function to execute when timer finishes.</param>
	Timer(std::function<void()> cb) : Timer(0, cb) {}

	/// <summary>
	/// Constructs a Timer with a duration and a callback function.
	/// </summary>
	/// <param name="ms">Duration in milliseconds before callback is invoked.</param>
	/// <param name="cb">Callback function to execute when timer finishes.</param>
	Timer(int ms = 0, std::function<void()> cb = nullptr)
		: firstTime(ms), callback(cb) { reset(); }


	/// <summary>
	/// Updates the timer by decreasing the remaining time.
	/// Calls the callback if time has elapsed.
	/// </summary>
	/// <param name="delta">Elapsed time in milliseconds since last update.</param>
	void update(int delta) {
		if (!finished) {
			timeLeft -= delta;
			if (timeLeft <= 0) {
				finished = true;
				if (callback)
					callback(); // we can reset timer in callback, so `finished=false`
			}
		}
	}

	/// <summary>
	/// Stops the timer immediately without calling the callback.
	/// </summary>
	void stop() {
		finished = true;
	}

	/// <summary>
	/// Resets the timer.
	/// </summary>
	/// <param name="ms">Duration in milliseconds before the callback is invoked. If `ms` is less than 0, the previous value will be used.</param>
	void reset(int ms = -1) {
		timeLeft = ms > 0 ? ms : firstTime;
		finished = !((bool)timeLeft); // if `timeLeft == 0` its finished
	}

	/// <summary>
	/// Checks if the timer has finished and callback was called.
	/// </summary>
	/// <returns>True if finished, false otherwise.</returns>
	bool isFinished() const { return finished; }

	/// <summary>
	/// Returns the remaining time in milliseconds before the timer finishes.
	/// </summary>
	/// <returns>Time left in milliseconds.</returns>
	int getTimeLeft() const { return timeLeft; }


private:
	std::function<void()> callback;
	const int firstTime; // in milliseconds
	int timeLeft; // in milliseconds
	bool finished;
};
