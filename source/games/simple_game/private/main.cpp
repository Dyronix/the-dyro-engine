#include "simple_game.h"

#include "application/engine.h"

//--------------------------------------------------------------
// The entry point of the game. A regular main() (instead of the win32
// WinMain) keeps a console window around, which is where all engine log
// messages appear.
//
// This is the smallest possible main(): the window size and title are set
// right here in code. The demo game (source/games/buz_game) shows how to
// override these from the command line as well; you do not need that to start.
//--------------------------------------------------------------
int main()
{
	buz::engine_settings settings;
	settings.window_width = 1280;
	settings.window_height = 720;
	settings.window_title = L"BuzEngine - simple game";

	// Keep pixel art crisp instead of blurring it when scaled.
	settings.sampler_filter = buz::texture_filter::nearest;

	simple_game game;

	buz::engine engine;
	return engine.run(game, settings);
}
