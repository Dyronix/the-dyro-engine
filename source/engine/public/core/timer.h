#pragma once

#include <chrono>

namespace dyro
{
	//--------------------------------------------------------------
	/// @brief Measures elapsed wall clock time with high precision.
	///
	/// A freshly constructed timer starts measuring immediately:
	///
	///     dyro::timer stopwatch;
	///     expensive_work();
	///     dyro::log::info("took {} seconds", stopwatch.elapsed_seconds());
	class timer
	{
	public:
		//----------------------------------------------------------
		/// @brief Starts measuring on construction.
		timer() { reset(); }

		//----------------------------------------------------------
		/// @brief Restarts the measurement from now.
		void reset() { m_start = std::chrono::steady_clock::now(); }

		//----------------------------------------------------------
		/// @brief Returns the time passed since construction or the last reset.
		/// @return Elapsed time in seconds.
		float elapsed_seconds() const
		{
			return std::chrono::duration<float>(std::chrono::steady_clock::now() - m_start).count();
		}

	private:
		std::chrono::steady_clock::time_point m_start;
	};
}
