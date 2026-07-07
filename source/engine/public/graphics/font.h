#pragma once

#include "core/math.h"
#include "core/rect.h"
#include "graphics/texture.h"

#include <cstdint>
#include <memory>

namespace dyx
{
	//--------------------------------------------------------------
	/// @brief A fixed-grid bitmap font: one atlas texture holding every
	/// character in a regular grid, in ascii order.
	///
	/// All glyph data lives in the atlas image (content/fonts/), this struct
	/// only describes the grid so draw_text can find each character:
	/// @code{.cpp}
	/// dyx::font font;
	/// font.atlas = engine.get_texture_loader().load_from_file(path_to_atlas);
	/// renderer.draw_text(font, "hello", { 20.0f, 20.0f }, 16.0f);
	/// @endcode
	///
	/// The defaults match the engine's built-in atlas
	/// (content/fonts/font_8x8.png): 8x8 pixel glyphs, 16 per row, starting
	/// at the space character (ascii 32).
	struct font
	{
		std::shared_ptr<texture> atlas;

		uint32_t first_character = 32;
		uint32_t glyph_count = 96;
		uint32_t glyphs_per_row = 16;
		uvec2 glyph_size = { 8, 8 };

		//----------------------------------------------------------
		/// @brief Returns where a character's glyph sits in the atlas, in
		/// pixels. Characters the font does not have come out as the first
		/// glyph (usually space, so they show up as blank).
		rect get_glyph_source_rect(char character) const
		{
			// The cast via unsigned char keeps characters above 127 from
			// going negative and ending up as a huge index.
			uint32_t index = static_cast<unsigned char>(character) - first_character;
			if (index >= glyph_count)
			{
				index = 0;
			}

			const vec2 top_left = {
				static_cast<float>((index % glyphs_per_row) * glyph_size.x),
				static_cast<float>((index / glyphs_per_row) * glyph_size.y) };

			return { top_left, top_left + vec2(glyph_size) };
		}
	};
}
