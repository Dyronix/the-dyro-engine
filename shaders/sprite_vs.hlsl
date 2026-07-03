//--------------------------------------------------------------
// Sprite vertex shader
//
// Transforms the unit quad to its position on screen and passes
// the texture coordinates and tint color on to the pixel shader.
//--------------------------------------------------------------

struct vertex_input
{
    float2 position : POSITION;
    float2 uv : TEXCOORD0;
};

struct pixel_input
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float4 tint : COLOR0;
};

// Root constants set by renderer_2d::draw_sprite (must match sprite_constants in renderer_2d.cpp)
cbuffer sprite_constants : register(b0)
{
    // Sprite transformation and screen projection combined into one matrix.
    // row_major matches the row-vector math the engine does on the cpu.
    row_major float4x4 transform;
    float4 tint_color;
};

pixel_input main(vertex_input input)
{
    pixel_input output;
    output.position = mul(float4(input.position, 0.0f, 1.0f), transform);
    output.uv = input.uv;
    output.tint = tint_color;

    return output;
}
