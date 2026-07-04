#pragma once

#include "application/game.h"
#include "core/math.h"
#include "graphics/font.h"
#include "graphics/texture.h"

#include <memory>

//--------------------------------------------------------------
/// @brief Small example game touring the engine's features: textured,
/// tinted and rotating sprites, sprite sheet animation, procedural
/// textures, input, text and debug drawing.
///
/// Use this class as the starting point for your own game.
class demo_game : public dyro::game
{
public:
	//----------------------------------------------------------
	/// @brief Loads the textures and the font, and generates a few textures
	/// procedurally.
	void initialize(dyro::engine& engine) override;

	//----------------------------------------------------------
	/// @brief Moves the ball with the keyboard and animates the sprites.
	void update(float delta_seconds) override;

	//----------------------------------------------------------
	/// @brief Draws the scene and the hud.
	void draw(dyro::renderer_2d& renderer) override;

private:
	// Kept to reach the input state (and other engine systems) in update
	dyro::engine* m_engine = nullptr;

	std::shared_ptr<dyro::texture> m_checker_texture;
	std::shared_ptr<dyro::texture> m_ball_texture;
	std::shared_ptr<dyro::texture> m_noise_texture;
	std::shared_ptr<dyro::texture> m_circle_sheet;

	dyro::font m_font;

	// The ball the player moves around
	dyro::vec2 m_ball_position = { 640.0f, 360.0f };
	float m_ball_size = 120.0f;
	dyro::color m_ball_tint;

	float m_rotation = 0.0f;
	float m_time = 0.0f;
	float m_smoothed_fps = 0.0f;
};
