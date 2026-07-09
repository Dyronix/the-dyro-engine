#pragma once

namespace dyx
{
	class engine;
	class renderer_2d;

	//--------------------------------------------------------------
	/// @brief Base class every game derives from.
	///
	/// The engine owns the main loop and calls these functions at the right
	/// time. Override the ones you need:
	/// @code{.cpp}
	/// class my_game : public dyx::game
	/// {
	/// public:
	///     void initialize(dyx::engine& engine) override;              // load textures here
	///     void update(dyx::engine& engine, float delta_seconds) override; // game logic here
	///     void draw(dyx::engine& engine, dyx::renderer_2d& renderer) override; // draw sprites here
	/// };
	/// @endcode
	class game
	{
	public:
		virtual ~game() = default;

		//----------------------------------------------------------
		/// @brief Called once after the engine started, before the first frame.
		/// @param engine Engine instance, use it to load textures.
		virtual void initialize(engine& engine) {}

		//----------------------------------------------------------
		/// @brief Called every frame before drawing.
		/// @param engine Engine instance, use it to read input and reach other systems.
		/// @param delta_seconds Time the previous frame took, in seconds.
		virtual void update(engine& engine, float delta_seconds) {}

		//----------------------------------------------------------
		/// @brief Called every frame to draw the game.
		/// @param engine Engine instance, use it to read input and reach other systems.
		/// @param renderer Renderer used to draw sprites.
		virtual void draw(engine& engine, renderer_2d& renderer) {}

		//----------------------------------------------------------
		/// @brief Called once when the game is closing.
		virtual void shutdown() {}
	};
}
