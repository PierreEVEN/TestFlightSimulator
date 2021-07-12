#version 460
#extension GL_ARB_separate_shader_objects : enable

layout (location = 8) in vec3 position;
layout (location = 9) in vec3 normal;
layout (location = 0) in vec2 texCoords;

layout(location = 0) out vec4 outColor;

// UNIFORM BUFFER
layout(binding = 5) uniform GlobalCameraUniformBuffer {
    mat4 worldProjection;
    mat4 viewMatrix;
	vec3 cameraLocation;
} ubo;

// SAMPLERS
//layout (binding = 6) uniform sampler2D image;
//layout (binding = 7) uniform sampler2D shadowMap;


void main() {
	vec3 sun = normalize(vec3(-1.0, 1.0, 1.0));

	float ambiant = 0.1;

	float light_power = max(0, dot(sun, normal)) * (1 - ambiant) + ambiant;

	outColor = vec4(light_power, light_power, light_power, 0); //vec4(mod(position.x / 20, 1), mod(position.y / 20, 1), mod(position.z / 20, 1), 0);//texture(colorMap, texCoords);	
}