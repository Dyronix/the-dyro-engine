//--------------------------------------------------------------
// Sprite pixel shader
//
// Samples the sprite texture and multiplies it with the tint color.
//--------------------------------------------------------------

struct pixel_input
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float4 tint : COLOR0;
};

Texture2D sprite_texture : register(t0);
SamplerState linear_sampler : register(s0);

float4 main(pixel_input input) : SV_TARGET
{
    return sprite_texture.Sample(linear_sampler, input.uv) * input.tint;
}
