#pragma once

#include "core/math.h"

#include <cstdint>

namespace dyro
{
	//--------------------------------------------------------------
	/// @brief The keys the engine understands.
	///
	/// Every value equals the win32 virtual key code of that key, so
	/// translating an incoming key message is a simple cast.
	enum class key : uint8_t
	{
		backspace = 0x08, // VK_BACK
		tab = 0x09,       // VK_TAB
		enter = 0x0d,     // VK_RETURN
		shift = 0x10,     // VK_SHIFT
		control = 0x11,   // VK_CONTROL
		escape = 0x1b,    // VK_ESCAPE
		space = 0x20,     // VK_SPACE

		left = 0x25,  // VK_LEFT
		up = 0x26,    // VK_UP
		right = 0x27, // VK_RIGHT
		down = 0x28,  // VK_DOWN

		// The digit and letter keys match their ascii characters
		num_0 = '0', num_1 = '1', num_2 = '2', num_3 = '3', num_4 = '4',
		num_5 = '5', num_6 = '6', num_7 = '7', num_8 = '8', num_9 = '9',

		a = 'A', b = 'B', c = 'C', d = 'D', e = 'E', f = 'F', g = 'G',
		h = 'H', i = 'I', j = 'J', k = 'K', l = 'L', m = 'M', n = 'N',
		o = 'O', p = 'P', q = 'Q', r = 'R', s = 'S', t = 'T', u = 'U',
		v = 'V', w = 'W', x = 'X', y = 'Y', z = 'Z',
	};

	//--------------------------------------------------------------
	/// @brief The mouse buttons the engine understands.
	enum class mouse_button : uint8_t
	{
		left = 0,
		right = 1,
		middle = 2,
	};

	//--------------------------------------------------------------
	/// @brief Keeps track of the keyboard and mouse state.
	///
	/// The engine owns one instance (engine::get_input) and the window feeds
	/// it every input message. Game code just asks questions:
	///
	///     if (input.is_key_down(dyro::key::a))            { /* every frame while held */ }
	///     if (input.was_key_pressed(dyro::key::space))    { /* only the frame it went down */ }
	///     if (input.was_key_released(dyro::key::escape))  { /* only the frame it came up */ }
	class input
	{
	public:
		//----------------------------------------------------------
		/// @brief Returns true while the key is held down.
		bool is_key_down(key value) const { return m_current_keys[static_cast<uint8_t>(value)]; }

		//----------------------------------------------------------
		/// @brief Returns true only in the frame the key went down.
		bool was_key_pressed(key value) const
		{
			return m_current_keys[static_cast<uint8_t>(value)] && !m_previous_keys[static_cast<uint8_t>(value)];
		}

		//----------------------------------------------------------
		/// @brief Returns true only in the frame the key came up.
		bool was_key_released(key value) const
		{
			return !m_current_keys[static_cast<uint8_t>(value)] && m_previous_keys[static_cast<uint8_t>(value)];
		}

		//----------------------------------------------------------
		/// @brief Returns true while the mouse button is held down.
		bool is_mouse_button_down(mouse_button button) const { return m_current_buttons[static_cast<uint8_t>(button)]; }

		//----------------------------------------------------------
		/// @brief Returns true only in the frame the mouse button went down.
		bool was_mouse_button_pressed(mouse_button button) const
		{
			return m_current_buttons[static_cast<uint8_t>(button)] && !m_previous_buttons[static_cast<uint8_t>(button)];
		}

		//----------------------------------------------------------
		/// @brief Returns true only in the frame the mouse button came up.
		bool was_mouse_button_released(mouse_button button) const
		{
			return !m_current_buttons[static_cast<uint8_t>(button)] && m_previous_buttons[static_cast<uint8_t>(button)];
		}

		//----------------------------------------------------------
		/// @brief Returns the mouse position in pixels, relative to the
		/// top-left corner of the drawable area.
		vec2 get_mouse_position() const { return m_mouse_position; }

		//----------------------------------------------------------
		/// @brief Returns how far the mouse wheel turned this frame:
		/// positive away from you, negative towards you, one "click" is 1.
		float get_mouse_wheel_delta() const { return m_wheel_delta; }

		// The functions below are called by the engine and window, game code
		// never needs them.

		//----------------------------------------------------------
		/// @brief Starts a new frame: what was current becomes the previous
		/// state that pressed/released compare against.
		void new_frame();

		//----------------------------------------------------------
		/// @brief Records that a key went down or up.
		void on_key_event(uint32_t virtual_key, bool is_down);

		//----------------------------------------------------------
		/// @brief Records that a mouse button went down or up.
		void on_mouse_button_event(mouse_button button, bool is_down);

		//----------------------------------------------------------
		/// @brief Records the new mouse position.
		void on_mouse_move(float x, float y);

		//----------------------------------------------------------
		/// @brief Records a mouse wheel turn.
		void on_mouse_wheel(float delta);

		//----------------------------------------------------------
		/// @brief Releases everything; called when the window loses focus so
		/// no key stays stuck down while the game is in the background.
		void on_focus_lost();

	private:
		static constexpr uint32_t k_key_count = 256;
		static constexpr uint32_t k_button_count = 3;

		bool m_current_keys[k_key_count] = {};
		bool m_previous_keys[k_key_count] = {};

		bool m_current_buttons[k_button_count] = {};
		bool m_previous_buttons[k_button_count] = {};

		vec2 m_mouse_position = { 0.0f, 0.0f };
		float m_wheel_delta = 0.0f;
	};
}
