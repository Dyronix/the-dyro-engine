#pragma once

#include "core/math.h"

namespace dyx
{
	//--------------------------------------------------------------
	/// @brief 2D axis aligned rectangle, the workhorse for collision checks.
	///
	/// Stored as two corners: min is the top-left, max is the bottom-right
	/// (remember: in screen coordinates the y axis points down).
	struct rect
	{
		vec2 min = { 0.0f, 0.0f };
		vec2 max = { 0.0f, 0.0f };

		//----------------------------------------------------------
		/// @brief Compares both corners; exact float equality, no epsilon.
		bool operator==(const rect&) const = default;

		//----------------------------------------------------------
		/// @brief Builds a rect from its center and size, matching how
		/// sprites are positioned in draw_sprite.
		/// @param center Position of the rect center in pixels.
		/// @param size Width and height in pixels.
		static rect from_center_size(vec2 center, vec2 size)
		{
			const vec2 half_size = size * 0.5f;
			return { center - half_size, center + half_size };
		}

		//----------------------------------------------------------
		/// @brief Returns the position of the rect center.
		vec2 center() const { return (min + max) * 0.5f; }

		//----------------------------------------------------------
		/// @brief Returns the width and height of the rect.
		vec2 size() const { return max - min; }

		//----------------------------------------------------------
		/// @brief Returns true when the point lies inside the rect.
		bool contains(vec2 point) const
		{
			return point.x >= min.x && point.x <= max.x &&
				point.y >= min.y && point.y <= max.y;
		}

		//----------------------------------------------------------
		/// @brief Returns true when the two rects overlap.
		bool intersects(const rect& other) const
		{
			return min.x <= other.max.x && other.min.x <= max.x &&
				min.y <= other.max.y && other.min.y <= max.y;
		}

		//----------------------------------------------------------
		/// @brief Expands the rect so it also contains the given point.
		void grow(vec2 point)
		{
			min = glm::min(min, point);
			max = glm::max(max, point);
		}

		//----------------------------------------------------------
		/// @brief Expands the rect so it also contains the given rect.
		void grow(const rect& other)
		{
			min = glm::min(min, other.min);
			max = glm::max(max, other.max);
		}
	};
}
