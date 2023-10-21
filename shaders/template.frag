#version 420 core

layout (location = 0) out vec4 outColor;

uniform vec2 viewPortSize;
uniform float currentTime;

void main()
{
    const vec2 uv = gl_FragCoord.xy / viewPortSize.xy;     // [0, 1]^2
    const float aspect = viewPortSize.x / viewPortSize.y;  // aspect ratio
    
    outColor = vec4(0.0, 0.0, 0.0, 1.0);
}
