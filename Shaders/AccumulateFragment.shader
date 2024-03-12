#version 430 core

out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D accumulate_texture;
uniform sampler2D sample_texture;

layout(std140, binding = 4) uniform renderData
{
	mat4 camera;
	vec3 camera_pos;
	int num_nodes;
	int curr_frame;
};

void main()
{
	vec3 A = texture(sample_texture, TexCoord).rgb;
	vec3 B = texture(accumulate_texture, TexCoord).rgb;

	float inv_frame = 1.0 / float(curr_frame);
	vec3 result = (inv_frame) * A + (1 - inv_frame) * B;
	FragColor = vec4(result, 1.0);
}