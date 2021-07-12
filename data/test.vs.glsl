#version 460

// IN
layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec4 col;
layout(location = 3) in vec3 norm;
layout(location = 4) in vec3 tang;
layout(location = 5) in vec3 bitang;

layout (location = 8) out vec3 position;
layout (location = 9) out vec3 normal;

// UNIFORM BUFFER
layout(binding = 9) uniform GlobalCameraUniformBuffer {
    mat4 worldProjection;
    mat4 viewMatrix;
	vec3 cameraLocation;
} ubo;

// SAMPLERS
//layout (binding = 6) uniform sampler2D image;
//layout (binding = 7) uniform sampler2D shadowMap;


struct ObjectData{
	mat4 model;
};

layout(std140, binding = 0) readonly buffer ObjectBuffer{
	ObjectData objects[];
} objectBuffer;

// OUT
layout (location = 0) out vec2 texCoords;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
	position = pos;
	normal = norm;
	gl_Position = ubo.worldProjection * ubo.viewMatrix * objectBuffer.objects[gl_BaseInstance].model * vec4(pos.xyz, 1.0);
}