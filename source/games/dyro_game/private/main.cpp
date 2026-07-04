#include "demo_game.h"

#include "application/engine.h"

//--------------------------------------------------------------
// The entry point of the game. A regular main() (instead of the win32
// WinMain) keeps a console window around, which is where all engine log
// messages appear.
//--------------------------------------------------------------
int main()
{
	dyro::engine_settings settings;
	settings.window_width = 1280;
	settings.window_height = 720;
	settings.window_title = L"DyroEngine - demo game";

	// Change this to adapter_preference::lowest_score to test the game on
	// the weakest graphics card in your machine.
	settings.gpu_preference = dyro::adapter_preference::highest_score;

	// Change this to texture_filter::nearest for a crisp pixel art look
	// instead of the default smooth/blurred scaling.
	settings.sampler_filter = dyro::texture_filter::nearest;

	demo_game game;

	dyro::engine engine;
	return engine.run(game, settings);
}
