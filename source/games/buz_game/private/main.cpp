#include "demo_game.h"

#include "application/engine.h"
#include "core/log.h"

#include <args/args.h>

#include <string>

//--------------------------------------------------------------
// The entry point of the game. A regular main() (instead of the win32
// WinMain) keeps a console window around, which is where all engine log
// messages appear.
//
// Every engine setting can be overridden at launch, e.g.
//   run.bat -game=buz_game -window_width=1920 -window_height=1080
// Settings that are not passed keep the value set here in code, which is why
// each line below hands its own value to args::parser as the fallback.
//--------------------------------------------------------------
int main(int argc, char** argv)
{
	args::parser arguments(argc, argv);

	buz::engine_settings settings;
	settings.window_width = arguments.get("window_width", settings.window_width);
	settings.window_height = arguments.get("window_height", settings.window_height);
	settings.window_title = arguments.get("window_title", std::wstring(L"BuzEngine - demo game"));

	// The remaining settings are configured in code (they rarely need changing
	// at launch, so they are not exposed on the command line).

	// Change this to adapter_preference::lowest_score to test the game on
	// the weakest graphics card in your machine.
	settings.gpu_preference = buz::adapter_preference::highest_score;

	// Change this to texture_filter::nearest for a crisp pixel art look
	// instead of the default smooth/blurred scaling.
	settings.sampler_filter = buz::texture_filter::nearest;

	// Point out mistyped or unsupported settings instead of silently ignoring them.
	for (const std::string& name : arguments.unused_keys())
	{
		buz::log::warn("unknown or invalid setting '{}' ignored", name);
	}

	demo_game game;

	buz::engine engine;
	return engine.run(game, settings);
}
