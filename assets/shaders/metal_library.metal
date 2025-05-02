#include <metal_stdlib>

using namespace metal;

struct vertex_attributes
{
	packed_float3 position;
	packed_float3 normal;
	packed_float2 uv;
	packed_float4 color;
};

struct uniforms
{
	simd::float4x4 model;
	simd::float4x4 view;
	simd::float4x4 projection;
};

struct vertex_varyings
{
	simd::float4 position [[position]];
	simd::float2 uv;
};

vertex vertex_varyings vertex_shader(uint vertex_id [[vertex_id]],                             //
                                     device const vertex_attributes *attributes [[buffer(0)]], //
                                     constant uniforms &uniforms [[buffer(1)]])
{
	vertex_varyings varyings;
	varyings.position =
	    uniforms.projection * uniforms.view * uniforms.model * simd::float4(attributes[vertex_id].position, 1.0f);
	varyings.uv = attributes[vertex_id].uv;
	return varyings;
}

fragment half4 fragment_shader(vertex_varyings varyings [[stage_in]], //
                               texture2d<half, access::sample> texture [[texture(0)]])
{
	constexpr sampler s(address::repeat, filter::linear);
	half4 color = texture.sample(s, varyings.uv).rgba;
	if (color.x == 0.0f && color.y == 0.0f && color.z == 0.0f)
	{
		color.x = varyings.uv.x;
		color.y = varyings.uv.y;
	}
	return color;
}
