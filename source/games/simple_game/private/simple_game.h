#pragma once

#include "application/game.h"
#include "core/math.h"
#include "graphics/font.h"
#include "graphics/texture.h"

#include <memory>

//--------------------------------------------------------------
/// @brief A tiny "catch the ball" game, meant as a first project.
///
/// A ball falls from the top of the screen; slide the paddle left and right
/// to catch it and the score goes up. Read update() and draw() in
/// simple_game.cpp from top to bottom - that is the whole game.
///
/// This is the simplest game in the repo. When you want to see more of what
/// the engine can do (procedural textures, sprite sheets, rotation, ...),
/// look at source/games/dyx_game.
class simple_game : public dyx::game
{
public:
	//----------------------------------------------------------
	/// @brief Loads the ball texture and the font.
	void initialize(dyx::engine& engine) override;

	//----------------------------------------------------------
	/// @brief Moves the paddle, drops the ball, and checks for a catch.
	void update(dyx::engine& engine, float delta_seconds) override;

	//----------------------------------------------------------
	/// @brief Draws the ball, the paddle and the score.
	void draw(dyx::engine& engine, dyx::renderer_2d& renderer) override;

private:
	//----------------------------------------------------------
	/// @brief Drops a fresh ball from just above the top at a random x.
	void respawn_ball();

	// The engine hands textures back as a shared_ptr; you never new/delete
	// them yourself. Just store it and pass it to draw_sprite every frame.
	std::shared_ptr<dyx::texture> m_ball;
	dyx::font m_font;

	dyx::vec2 m_ball_position = { 640.0f, 0.0f };
	float m_ball_speed = 300.0f;  // pixels per second the ball falls
	float m_paddle_x = 640.0f;    // center x of the paddle (it sits near the bottom)
	int m_score = 0;
};
