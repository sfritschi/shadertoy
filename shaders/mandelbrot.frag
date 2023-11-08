#version 420 core

layout (location = 0) out vec4 outColor;

uniform vec2 viewPortSize;
uniform float currentTime;

const float convergenceRadius = 2.0;

void main()
{
    const vec2 uv = gl_FragCoord.xy / viewPortSize.xy;     // [0, 1]^2
    const float aspect = viewPortSize.x / viewPortSize.y;  // aspect ratio
    
    vec2 c = 2.0 * uv - 1.0;
    c.x = (c.x - 0.5) * aspect;
    
    vec2 a = vec2(0.0, 0.0);  // initial guess
    
    bool isDiverged = false;
    const int n = 100;
    for (int i = 0; i < n; ++i)
    {
        // a_{n} = a_{n-1}^2 + c
        a = vec2(a.x*a.x - a.y*a.y, 2.0*a.x*a.y) + c;
        
        if (length(a) > convergenceRadius)
        {
            isDiverged = true;
            break;
        }
    }
    
    if (isDiverged)
        outColor = vec4(1.0, 1.0, 1.0, 1.0);  // white
    else
        outColor = vec4(0.0, 0.0, 0.0, 1.0);  // black
}
