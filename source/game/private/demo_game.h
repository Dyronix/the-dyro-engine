#pragma once

#include "application/game.h"
#include "core/math_2d.h"
#include "graphics/texture.h"

#include <memory>

//--------------------------------------------------------------
/// @brief Small example game: a few textured, tinted and rotating sprites.
///
/// Use this class as the starting point for your own game.
class demo_game : public dyro::game
{
public:
	//----------------------------------------------------------
	/// @brief Loads the textures the game draws.
	void initialize(dyro::engine& engine) override;

	//----------------------------------------------------------
	/// @brief Animates the sprites.
	void update(float delta_seconds) override;

	//----------------------------------------------------------
	/// @brief Draws the sprites.
	void draw(dyro::renderer_2d& renderer) override;

private:
	std::shared_ptr<dyro::texture> m_checker_texture;
	std::shared_ptr<dyro::texture> m_ball_texture;

	float m_rotation = 0.0f;
	float m_time = 0.0f;
};
