#include "application/input.h"

#include <cstring>

namespace buz
{
	//--------------------------------------------------------------
	void input::new_frame()
	{
		std::memcpy(m_previous_keys, m_current_keys, sizeof(m_current_keys));
		std::memcpy(m_previous_buttons, m_current_buttons, sizeof(m_current_buttons));

		// The wheel delta is per frame, so it starts at zero and the wheel
		// messages of the coming frame add to it.
		m_wheel_delta = 0.0f;
	}

	//--------------------------------------------------------------
	void input::on_key_event(uint32_t virtual_key, bool is_down)
	{
		if (virtual_key < k_key_count)
		{
			m_current_keys[virtual_key] = is_down;
		}
	}

	//--------------------------------------------------------------
	void input::on_mouse_button_event(mouse_button button, bool is_down)
	{
		m_current_buttons[static_cast<uint8_t>(button)] = is_down;
	}

	//--------------------------------------------------------------
	void input::on_mouse_move(float x, float y)
	{
		m_mouse_position = { x, y };
	}

	//--------------------------------------------------------------
	void input::on_mouse_wheel(float delta)
	{
		m_wheel_delta += delta;
	}

	//--------------------------------------------------------------
	void input::on_focus_lost()
	{
		std::memset(m_current_keys, 0, sizeof(m_current_keys));
		std::memset(m_current_buttons, 0, sizeof(m_current_buttons));
	}
}
