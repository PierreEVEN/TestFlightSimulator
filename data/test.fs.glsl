#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D colorMap;
layout(binding = 2) uniform sampler2D colorMap2;

layout (location = 0) in vec2 texCoords;

layout(location = 0) out vec4 outColor;


void main() {
	outColor = texture(colorMap, texCoords);
	outColor = texture(colorMap2, texCoords);
	
}