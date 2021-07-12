#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec2 texCoords;

layout(location = 0) out vec4 outColor;

layout (location = 8) in vec3 position;

// UNIFORM BUFFER
layout(binding = 5) uniform GlobalCameraUniformBuffer {
    mat4 worldProjection;
    mat4 viewMatrix;
	vec3 cameraLocation;
} ubo;

// SAMPLERS
//layout (binding = 6) uniform sampler2D image;
//layout (binding = 7) uniform sampler2D shadowMap;

// PUSH CONSTANTS
layout(push_constant) uniform PushConstant_STR {
	mat4 model;
} primitive;

void main() {
	outColor = vec4(mod(position.x / 20, 1), mod(position.y / 20, 1), mod(position.z / 20, 1), 0);//texture(colorMap, texCoords);	
}