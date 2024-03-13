#version 430 core

out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D result_texture;

uniform float exposure;

void main()
{
	vec3 result = texture(result_texture, TexCoord).rgb;
	// hdr tone mapping
	result = vec3(1.0) - exp(-result * exposure);
	// gamma correction
	result = pow(result, vec3(1.0 / 2.2));

	FragColor = vec4(result, 1.0);
}