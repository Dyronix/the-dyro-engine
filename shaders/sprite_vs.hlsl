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
    // Stored column-major (hlsl's default), which is exactly how glm stores
    // its matrices on the cpu, so the engine can upload them as-is.
    float4x4 transform;
    float4 tint_color;

    // Part of the texture this sprite shows: offset in xy, scale in zw
    // (0..1 texture space). (0, 0, 1, 1) shows the whole texture; sprite
    // sheets select a single frame here.
    float4 uv_rect;
};

pixel_input main(vertex_input input)
{
    pixel_input output;
    output.position = mul(transform, float4(input.position, 0.0f, 1.0f));
    output.uv = uv_rect.xy + input.uv * uv_rect.zw;
    output.tint = tint_color;

    return output;
}
