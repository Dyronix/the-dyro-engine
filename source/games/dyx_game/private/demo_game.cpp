#include "demo_game.h"

#include "application/engine.h"
#include "application/input.h"
#include "core/log.h"
#include "core/noise.h"
#include "core/paths.h"
#include "core/random.h"
#include "core/rect.h"

#define FMT_HEADER_ONLY
#include <fmt/core.h>

#include <cmath>
#include <vector>

namespace
{
	//--------------------------------------------------------------
	/// @brief Generates a grayscale texture from smooth noise, to show off
	/// dyx::noise_2d and texture_loader::create_from_pixels.
	std::shared_ptr<dyx::texture> make_noise_texture(dyx::texture_loader& loader, uint32_t size)
	{
		std::vector<uint8_t> pixels(static_cast<size_t>(size) * size * 4);

		for (uint32_t y = 0; y < size; ++y)
		{
			for (uint32_t x = 0; x < size; ++x)
			{
				// noise_2d returns roughly [-1, 1]; remap to a 0..255 gray
				const float sample = dyx::noise_2d(static_cast<float>(x) * 0.05f, static_cast<float>(y) * 0.05f);
				const auto gray = static_cast<uint8_t>((sample * 0.5f + 0.5f) * 255.0f);

				uint8_t* pixel = &pixels[(static_cast<size_t>(y) * size + x) * 4];
				pixel[0] = gray;
				pixel[1] = gray;
				pixel[2] = gray;
				pixel[3] = 255;
			}
		}

		return loader.create_from_pixels(size, size, pixels.data());
	}

	//--------------------------------------------------------------
	/// @brief Generates a small sprite sheet: four frames of a growing
	/// circle, laid out next to each other in one texture.
	std::shared_ptr<dyx::texture> make_circle_sheet(dyx::texture_loader& loader, uint32_t frame_size, uint32_t frame_count)
	{
		const uint32_t sheet_width = frame_size * frame_count;
		std::vector<uint8_t> pixels(static_cast<size_t>(sheet_width) * frame_size * 4); // all zero = transparent

		const float center = static_cast<float>(frame_size) * 0.5f;

		for (uint32_t frame = 0; frame < frame_count; ++frame)
		{
			const float radius = center * (0.35f + 0.6f * static_cast<float>(frame) / static_cast<float>(frame_count - 1));

			for (uint32_t y = 0; y < frame_size; ++y)
			{
				for (uint32_t x = 0; x < frame_size; ++x)
				{
					const dyx::vec2 offset = { static_cast<float>(x) - center, static_cast<float>(y) - center };
					if (dyx::length(offset) > radius)
					{
						continue;
					}

					uint8_t* pixel = &pixels[(static_cast<size_t>(y) * sheet_width + frame * frame_size + x) * 4];
					pixel[0] = 255;
					pixel[1] = 255;
					pixel[2] = 255;
					pixel[3] = 255;
				}
			}
		}

		return loader.create_from_pixels(sheet_width, frame_size, pixels.data());
	}
}

//--------------------------------------------------------------
void demo_game::initialize(dyx::engine& engine)
{
	m_engine = &engine;

	const std::filesystem::path content_directory = dyx::paths::get_executable_directory() / "content";

	m_checker_texture = engine.get_texture_loader().load_from_file(content_directory / "textures" / "checker.png");
	m_ball_texture = engine.get_texture_loader().load_from_file(content_directory / "textures" / "ball.png");

	m_font.atlas = engine.get_texture_loader().load_from_file(content_directory / "fonts" / "font_8x8.png");

	m_noise_texture = make_noise_texture(engine.get_texture_loader(), 256);
	m_circle_sheet = make_circle_sheet(engine.get_texture_loader(), 64, 4);

	dyx::set_random_seed(1234);
}

//--------------------------------------------------------------
void demo_game::update(float delta_seconds)
{
	m_time += delta_seconds;
	m_rotation += delta_seconds; // one radian per second

	dyx::input& input = m_engine->get_input();

	// Move the ball with wasd or the arrow keys
	dyx::vec2 direction = { 0.0f, 0.0f };
	if (input.is_key_down(dyx::key::a) || input.is_key_down(dyx::key::left)) { direction.x -= 1.0f; }
	if (input.is_key_down(dyx::key::d) || input.is_key_down(dyx::key::right)) { direction.x += 1.0f; }
	if (input.is_key_down(dyx::key::w) || input.is_key_down(dyx::key::up)) { direction.y -= 1.0f; }
	if (input.is_key_down(dyx::key::s) || input.is_key_down(dyx::key::down)) { direction.y += 1.0f; }

	// Normalize so diagonal movement is not faster than straight movement
	if (dyx::length(direction) > 0.0f)
	{
		direction = dyx::normalize(direction);
	}

	constexpr float ball_speed = 400.0f; // pixels per second
	m_ball_position += direction * ball_speed * delta_seconds;
	m_ball_position = dyx::clamp(m_ball_position, dyx::vec2(60.0f, 60.0f), dyx::vec2(1220.0f, 660.0f));

	// The mouse wheel resizes the ball
	m_ball_size = dyx::clamp(m_ball_size + input.get_mouse_wheel_delta() * 10.0f, 40.0f, 300.0f);

	// Space gives the ball a new random tint
	if (input.was_key_pressed(dyx::key::space))
	{
		m_ball_tint = { dyx::random_float(), dyx::random_float(), dyx::random_float(), 1.0f };
		dyx::log::info("New ball tint: ({:.2f}, {:.2f}, {:.2f})", m_ball_tint.r, m_ball_tint.g, m_ball_tint.b);
	}

	// Smooth the fps so the hud number does not flicker
	if (delta_seconds > 0.0f)
	{
		m_smoothed_fps = dyx::lerp(m_smoothed_fps, 1.0f / delta_seconds, 0.05f);
	}
}

//--------------------------------------------------------------
void demo_game::draw(dyx::renderer_2d& renderer)
{
	dyx::input& input = m_engine->get_input();
	const dyx::vec2 mouse = input.get_mouse_position();

	// A big background quad
	renderer.draw_sprite(*m_checker_texture, { 640.0f, 360.0f }, { 500.0f, 500.0f });

	// The procedurally generated noise texture
	renderer.draw_sprite(*m_noise_texture, { 150.0f, 550.0f }, { 220.0f, 220.0f });

	// A rotating quad, tinted red
	renderer.draw_sprite(*m_checker_texture, { 250.0f, 360.0f }, { 150.0f, 150.0f }, m_rotation, { 1.0f, 0.3f, 0.3f, 1.0f });

	// Sprite sheet animation: pick one of the four frames of the sheet
	const auto frame = static_cast<uint32_t>(m_time * 8.0f) % 4;
	const dyx::rect frame_rect = { { static_cast<float>(frame) * 64.0f, 0.0f }, { static_cast<float>(frame + 1) * 64.0f, 64.0f } };
	renderer.draw_sprite(*m_circle_sheet, frame_rect, { 1030.0f, 200.0f }, { 100.0f, 100.0f }, 0.0f, { 1.0f, 0.6f, 0.1f, 1.0f });

	// A bouncing ball with transparency (alpha blending)
	const float bounce_height = std::abs(std::sin(m_time * 2.0f)) * 250.0f;
	renderer.draw_sprite(*m_ball_texture, { 1030.0f, 560.0f - bounce_height }, { 120.0f, 120.0f });

	// The player ball, with a debug outline that lights up when the mouse
	// is inside its bounds
	renderer.draw_sprite(*m_ball_texture, m_ball_position, { m_ball_size, m_ball_size }, 0.0f, m_ball_tint);

	const dyx::rect ball_bounds = dyx::rect::from_center_size(m_ball_position, { m_ball_size, m_ball_size });
	const bool mouse_on_ball = ball_bounds.contains(mouse);
	const dyx::color outline_color = mouse_on_ball ? dyx::color{ 1.0f, 0.2f, 0.2f, 1.0f } : dyx::color{ 0.2f, 1.0f, 0.2f, 0.6f };
	renderer.draw_rect_outline(ball_bounds, 2.0f, outline_color);

	// A line from the rotating quad to the player ball
	renderer.draw_line({ 250.0f, 360.0f }, m_ball_position, 2.0f, { 1.0f, 1.0f, 0.2f, 0.4f });

	// The hud, drawn last so it sits on top of everything
	renderer.draw_text(m_font, "wasd moves the ball - space tints it - wheel resizes it", { 20.0f, 20.0f }, 16.0f);
	renderer.draw_text(m_font,
		fmt::format("fps {:.0f}\nmouse {:.0f} {:.0f}", m_smoothed_fps, mouse.x, mouse.y),
		{ 20.0f, 48.0f }, 16.0f, { 0.6f, 0.8f, 1.0f, 1.0f });
}
