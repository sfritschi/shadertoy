#version 420 core

layout (location = 0) out vec4 outColor;

uniform vec2 viewPortSize;

uniform sampler2D currentState;

const float M_PI = 3.1415926535897932384626433832795;

void main()
{
    const vec4 alpha = texture(currentState, gl_FragCoord.xy / viewPortSize);
    
    // Map double pendulum angles in range [-pi, pi]^2 to a color in the
    // range of [0, 1]^3
    outColor = vec4((alpha.xy / M_PI + 1.0) * 0.5, 1.0, 1.0);
}
