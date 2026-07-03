#include "demo_game.h"

#include "application/engine.h"
#include "core/paths.h"

//--------------------------------------------------------------
void demo_game::initialize(dyro::engine& engine)
{
	const std::filesystem::path texture_directory = dyro::paths::get_executable_directory() / "content" / "textures";

	m_checker_texture = engine.get_texture_loader().load_from_file(texture_directory / "checker.png");
	m_ball_texture = engine.get_texture_loader().load_from_file(texture_directory / "ball.png");
}

//--------------------------------------------------------------
void demo_game::update(float delta_seconds)
{
	m_time += delta_seconds;
	m_rotation += delta_seconds; // one radian per second
}

//--------------------------------------------------------------
void demo_game::draw(dyro::renderer_2d& renderer)
{
	// A big background quad
	renderer.draw_sprite(*m_checker_texture, { 640.0f, 360.0f }, { 500.0f, 500.0f });

	// A rotating quad, tinted red
	renderer.draw_sprite(*m_checker_texture, { 250.0f, 360.0f }, { 150.0f, 150.0f }, m_rotation, { 1.0f, 0.3f, 0.3f, 1.0f });

	// A bouncing ball with transparency (alpha blending)
	const float bounce_height = std::abs(std::sin(m_time * 2.0f)) * 250.0f;
	renderer.draw_sprite(*m_ball_texture, { 1030.0f, 560.0f - bounce_height }, { 120.0f, 120.0f });

	// A half transparent ball on top of everything
	renderer.draw_sprite(*m_ball_texture, { 640.0f, 360.0f }, { 200.0f, 200.0f }, 0.0f, { 1.0f, 1.0f, 1.0f, 0.5f });
}
