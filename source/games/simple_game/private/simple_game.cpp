#include "simple_game.h"

#include "application/engine.h"
#include "application/input.h"
#include "core/random.h"
#include "core/rect.h"

#include <format>

namespace
{
	// The screen and object sizes, kept as named constants so the numbers in
	// the logic below read as something you can understand at a glance.
	constexpr float k_screen_width = 1280.0f;
	constexpr float k_screen_height = 720.0f;

	constexpr float k_ball_size = 60.0f;    // the ball is 60x60 pixels
	constexpr float k_paddle_width = 160.0f;
	constexpr float k_paddle_height = 24.0f;
	constexpr float k_paddle_y = 680.0f;    // paddle center y, near the bottom
	constexpr float k_paddle_speed = 600.0f; // pixels per second the paddle moves

	constexpr float k_start_speed = 300.0f;  // ball fall speed at the start
	constexpr float k_speed_bump = 15.0f;    // ball gets this much faster per catch
}

//--------------------------------------------------------------
void simple_game::initialize(dyx::engine& engine)
{
	// Load the images once, here. Paths are relative to the content folder
	// that sits next to the executable.
	m_ball = engine.load_texture("textures/ball.png");
	m_font.atlas = engine.load_texture("fonts/font_8x8.png");

	// Seeding the random generator with a fixed value makes every run play out
	// the same way, which is handy while you are still finding bugs.
	dyx::set_random_seed(1234);

	m_ball_speed = k_start_speed;
	respawn_ball();
}

//--------------------------------------------------------------
void simple_game::update(dyx::engine& engine, float delta_seconds)
{
	dyx::input& input = engine.get_input();

	// Move the paddle with the arrow keys (or a/d). Multiplying by
	// delta_seconds keeps the speed the same no matter the frame rate.
	if (input.is_key_down(dyx::key::left) || input.is_key_down(dyx::key::a)) { m_paddle_x -= k_paddle_speed * delta_seconds; }
	if (input.is_key_down(dyx::key::right) || input.is_key_down(dyx::key::d)) { m_paddle_x += k_paddle_speed * delta_seconds; }

	// Keep the whole paddle on screen.
	m_paddle_x = dyx::clamp(m_paddle_x, k_paddle_width * 0.5f, k_screen_width - k_paddle_width * 0.5f);

	// The ball falls straight down.
	m_ball_position.y += m_ball_speed * delta_seconds;

	// Did the paddle catch the ball? Build a rectangle for each and ask the
	// engine whether they overlap.
	const dyx::rect ball_bounds = dyx::rect::from_center_size(m_ball_position, { k_ball_size, k_ball_size });
	const dyx::rect paddle_bounds = dyx::rect::from_center_size({ m_paddle_x, k_paddle_y }, { k_paddle_width, k_paddle_height });

	if (ball_bounds.intersects(paddle_bounds))
	{
		// Caught it: score a point, make the next ball a little faster.
		m_score += 1;
		m_ball_speed += k_speed_bump;
		respawn_ball();
	}
	else if (m_ball_position.y - k_ball_size * 0.5f > k_screen_height)
	{
		// The ball fell off the bottom: back to the start.
		m_score = 0;
		m_ball_speed = k_start_speed;
		respawn_ball();
	}
}

//--------------------------------------------------------------
void simple_game::draw(dyx::engine& engine, dyx::renderer_2d& renderer)
{
	// The falling ball.
	renderer.draw_sprite(*m_ball, m_ball_position, { k_ball_size, k_ball_size });

	// The paddle is just a filled rectangle.
	const dyx::rect paddle_bounds = dyx::rect::from_center_size({ m_paddle_x, k_paddle_y }, { k_paddle_width, k_paddle_height });
	renderer.draw_rect(paddle_bounds, { 0.3f, 0.7f, 1.0f, 1.0f });

	// A one-line hint and the score, drawn on top.
	renderer.draw_text(m_font, "catch the ball - move the paddle with the arrow keys", { 20.0f, 20.0f }, 16.0f);
	renderer.draw_text(m_font, std::format("score {}", m_score), { 20.0f, 44.0f }, 16.0f, { 0.6f, 0.8f, 1.0f, 1.0f });
}

//--------------------------------------------------------------
void simple_game::respawn_ball()
{
	// Start just above the top edge so the ball slides into view, at a random
	// horizontal position that keeps the whole ball on screen.
	const float x = dyx::random_range(k_ball_size, k_screen_width - k_ball_size);
	m_ball_position = { x, -k_ball_size };
}
