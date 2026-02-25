#version 420 core

layout (location = 0) out vec4 outColor;

uniform vec2 viewPortSize;

uniform sampler2D input;

const float M_PI = 3.1415926535897932384626433832795;

void main()
{
    const vec4 alpha = texture(input, gl_FragCoord.xy / viewPortSize);
    
    outColor = vec4((alpha.xy / M_PI + 1.0) * 0.5, 1.0, 1.0);
}
