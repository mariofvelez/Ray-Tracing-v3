#version 430 core

out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D result_texture;

void main()
{
	// hdr tone mapping
	// gamma correction
	FragColor = texture(result_texture, TexCoord);
}