#version 450

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec4 col;
layout(location = 3) in vec3 norm;
layout(location = 4) in vec3 tang;
layout(location = 5) in vec3 bitang;

/*
layout(binding = 5) uniform UniformBufferObject {
    mat4 worldProjection;
    mat4 viewMatrix;
	vec3 cameraLocation;
	float time;
} ubo;
*/

layout(push_constant) uniform PushConsts {
	mat4 model;
} primitive;

layout (location = 0) out vec2 texCoords;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {

	texCoords = uv;
	//gl_Position = ubo.worldProjection * ubo.viewMatrix * primitive.model * vec4(pos.xyz, 1.0);

}