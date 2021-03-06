#version 450

layout (points) in;
layout (line_strip, max_vertices = 2) out;

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec4 col;
layout(location = 3) in vec3 norm;
layout(location = 4) in vec3 tang;
layout(location = 5) in vec3 bitang;

void main() {    
    gl_Position = gl_in[0].gl_Position + vec4(-0.1, 0.0, 0.0, 0.0); 
    EmitVertex();

    gl_Position = gl_in[0].gl_Position + vec4( 0.1, 0.0, 0.0, 0.0);
    EmitVertex();
    
    EndPrimitive();
}  