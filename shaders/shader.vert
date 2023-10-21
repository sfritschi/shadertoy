#version 420 core

// Quad in device coordinates (OpenGL)
const vec2 quadVertices[4] = {
    vec2(-1.0, -1.0),
    vec2(1.0, -1.0),
    vec2(1.0, 1.0),
    vec2(-1.0, 1.0)
};

void main()
{
    gl_Position = vec4(quadVertices[gl_VertexID].xy, 0.0, 1.0);
}
