#pragma once

// The engine uses glm (source/third_party/glm) for its math: a battle tested,
// header-only library that mirrors the vector math you write in shaders. This
// header gives the glm types dyx names and adds the handful of 2d helpers the
// engine itself needs. Game code only includes "core/math.h" and works with
// the dyx:: names.
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <concepts>

namespace dyx
{
	//--------------------------------------------------------------
	// The glm types under their dyx names. These are aliases, not wrappers:
	// dyx::vec2 IS glm::vec2, so every glm function works on them directly.
	using vec2 = glm::vec2;
	using vec3 = glm::vec3;
	using vec4 = glm::vec4;
	using ivec2 = glm::ivec2;
	using uvec2 = glm::uvec2;
	using mat4 = glm::mat4;

	//--------------------------------------------------------------
	// The most used glm functions, made available as dyx::... so game code
	// does not have to know where they come from.
	using glm::clamp;
	using glm::dot;
	using glm::length;
	using glm::distance;
	using glm::normalize;
	using glm::radians;
	using glm::degrees;

	//--------------------------------------------------------------
	inline constexpr float k_pi = 3.14159265358979323846f;

	//--------------------------------------------------------------
	/// @brief RGBA color with components in the [0, 1] range.
	struct color
	{
		float r = 1.0f;
		float g = 1.0f;
		float b = 1.0f;
		float a = 1.0f;

		/// @brief Compares all components; exact float equality, no epsilon.
		bool operator==(const color&) const = default;
	};

	//--------------------------------------------------------------
	/// @brief Satisfied by the types lerp supports: floating-point values
	/// and the glm vector types.
	///
	/// A concept describes what a template needs from its type parameter.
	/// Calling lerp with anything else fails right at the call site with
	/// "lerpable<T> evaluated to false" instead of pages of glm template
	/// errors from deep inside the implementation.
	template<typename T>
	concept lerpable = std::floating_point<T>
		|| std::same_as<T, vec2>
		|| std::same_as<T, vec3>
		|| std::same_as<T, vec4>;

	//--------------------------------------------------------------
	/// @brief Linearly interpolates between a and b.
	/// @param a Value returned at t = 0.
	/// @param b Value returned at t = 1.
	/// @param t Interpolation factor; values outside [0, 1] extrapolate.
	template<lerpable T>
	inline T lerp(const T& a, const T& b, float t)
	{
		return glm::mix(a, b, t);
	}

	//--------------------------------------------------------------
	/// @brief Builds an orthographic projection for 2D rendering.
	///
	/// Maps pixel coordinates to normalized device coordinates, with the
	/// origin (0, 0) in the top-left corner of the screen and the y axis
	/// pointing down (like most 2D drawing APIs).
	///
	/// Note on conventions: the engine follows glm, so matrices are stored
	/// column-major (m[column][row]) and positions are column vectors that
	/// are transformed as v' = M * v. The translation lives in the last
	/// column of the matrix.
	///
	/// @param width Width of the render area in pixels.
	/// @param height Height of the render area in pixels.
	/// @return Orthographic projection matrix.
	inline mat4 make_orthographic_projection(float width, float height)
	{
		mat4 projection(0.0f);
		projection[0][0] = 2.0f / width;   // x: [0, width] -> [-1, 1]
		projection[1][1] = -2.0f / height; // y: [0, height] -> [1, -1] (flip: screen y is down, clip space y is up)
		projection[2][2] = 1.0f;
		projection[3][0] = -1.0f;
		projection[3][1] = 1.0f;
		projection[3][3] = 1.0f;

		return projection;
	}

	//--------------------------------------------------------------
	/// @brief Builds the world transformation for a sprite quad.
	///
	/// The renderer draws a unit quad from (-0.5, -0.5) to (0.5, 0.5). This
	/// transform scales it to the requested size, rotates it around its
	/// center and then moves it so its center sits at the given position.
	///
	/// @param position Position of the sprite center in pixels.
	/// @param size Width and height of the sprite in pixels.
	/// @param rotation_radians Clockwise rotation around the sprite center.
	/// @return World transformation matrix for the sprite.
	inline mat4 make_sprite_transform(vec2 position, vec2 size, float rotation_radians)
	{
		// Column-vector convention: the transform written last is applied
		// first, so the quad is scaled, then rotated, then translated.
		mat4 transform = glm::translate(mat4(1.0f), vec3(position, 0.0f));
		transform = glm::rotate(transform, rotation_radians, vec3(0.0f, 0.0f, 1.0f));
		transform = glm::scale(transform, vec3(size, 1.0f));

		return transform;
	}
}
