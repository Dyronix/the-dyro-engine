#pragma once

#include <cmath>

namespace dyro
{
	//--------------------------------------------------------------
	/// @brief Two component vector used for 2D positions, sizes and texture coordinates.
	struct float2
	{
		float x = 0.0f;
		float y = 0.0f;
	};

	//--------------------------------------------------------------
	/// @brief RGBA color with components in the [0, 1] range.
	struct color
	{
		float r = 1.0f;
		float g = 1.0f;
		float b = 1.0f;
		float a = 1.0f;
	};

	//--------------------------------------------------------------
	/// @brief 4x4 matrix stored in row-major order.
	///
	/// The engine uses the row-vector convention: a position is transformed
	/// by multiplying it on the left of the matrix (v' = v * M). This matches
	/// the "row_major" matrices declared in our HLSL shaders.
	struct float4x4
	{
		float m[4][4] = {};
	};

	//--------------------------------------------------------------
	/// @brief Returns the identity matrix.
	inline float4x4 make_identity()
	{
		float4x4 identity;
		identity.m[0][0] = 1.0f;
		identity.m[1][1] = 1.0f;
		identity.m[2][2] = 1.0f;
		identity.m[3][3] = 1.0f;

		return identity;
	}

	//--------------------------------------------------------------
	/// @brief Multiplies two matrices (result = a * b).
	/// @param a Left hand side matrix, applied first in the row-vector convention.
	/// @param b Right hand side matrix, applied second in the row-vector convention.
	/// @return Combined transformation matrix.
	inline float4x4 multiply(const float4x4& a, const float4x4& b)
	{
		float4x4 result;
		for (int row = 0; row < 4; ++row)
		{
			for (int column = 0; column < 4; ++column)
			{
				float sum = 0.0f;
				for (int i = 0; i < 4; ++i)
				{
					sum += a.m[row][i] * b.m[i][column];
				}

				result.m[row][column] = sum;
			}
		}

		return result;
	}

	//--------------------------------------------------------------
	/// @brief Builds an orthographic projection for 2D rendering.
	///
	/// Maps pixel coordinates to normalized device coordinates, with the
	/// origin (0, 0) in the top-left corner of the screen and the y axis
	/// pointing down (like most 2D drawing APIs).
	///
	/// @param width Width of the render area in pixels.
	/// @param height Height of the render area in pixels.
	/// @return Orthographic projection matrix.
	inline float4x4 make_orthographic_projection(float width, float height)
	{
		float4x4 projection;
		projection.m[0][0] = 2.0f / width;
		projection.m[1][1] = -2.0f / height;
		projection.m[2][2] = 1.0f;
		projection.m[3][0] = -1.0f;
		projection.m[3][1] = 1.0f;
		projection.m[3][3] = 1.0f;

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
	inline float4x4 make_sprite_transform(float2 position, float2 size, float rotation_radians)
	{
		const float cosine = std::cos(rotation_radians);
		const float sine = std::sin(rotation_radians);

		// Scale, rotation and translation combined into a single matrix
		// (row-vector convention: scale first, then rotate, then translate).
		float4x4 transform;
		transform.m[0][0] = size.x * cosine;
		transform.m[0][1] = size.x * sine;
		transform.m[1][0] = size.y * -sine;
		transform.m[1][1] = size.y * cosine;
		transform.m[2][2] = 1.0f;
		transform.m[3][0] = position.x;
		transform.m[3][1] = position.y;
		transform.m[3][3] = 1.0f;

		return transform;
	}
}
