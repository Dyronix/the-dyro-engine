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
class demo_game : public dyx::game
{
public:
	//----------------------------------------------------------
	/// @brief Loads the textures and the font, and generates a few textures
	/// procedurally.
	void initialize(dyx::engine& engine) override;

	//----------------------------------------------------------
	/// @brief Moves the ball with the keyboard and animates the sprites.
	void update(dyx::engine& engine, float delta_seconds) override;

	//----------------------------------------------------------
	/// @brief Draws the scene and the hud.
	void draw(dyx::engine& engine, dyx::renderer_2d& renderer) override;

private:
	std::shared_ptr<dyx::texture> m_checker_texture;
	std::shared_ptr<dyx::texture> m_ball_texture;
	std::shared_ptr<dyx::texture> m_noise_texture;
	std::shared_ptr<dyx::texture> m_circle_sheet;

	dyx::font m_font;

	// The ball the player moves around
	dyx::vec2 m_ball_position = { 640.0f, 360.0f };
	float m_ball_size = 120.0f;
	dyx::color m_ball_tint;

	float m_rotation = 0.0f;
	float m_time = 0.0f;
	float m_smoothed_fps = 0.0f;
};
