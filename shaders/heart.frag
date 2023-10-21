#version 420 core

#define M_PI 3.1415926535897932384626433832795

layout (location = 0) out vec4 outColor;

uniform vec2 viewPortSize;
uniform float currentTime;

// Helper functions
float dot2(const float x, const float y)
{
    return x*x + y*y;    
}

vec3 alternatingColor(const vec2 uv, float a)
{
    return 0.5 * (1.0 + cos(a * currentTime + uv.xyx + vec3(0.0, 2.0, 4.0)));
}

// Signed distance functions (SDFs)
vec3 sdfCircle(const vec2 xy, const vec2 cx, const float r)
{
    const vec2 diff = xy - cx;
    const float d = dot(diff, diff) - r*r;  // negative if inside circle
    return d > 0.0 ? vec3(0.0) : alternatingColor(xy, 1.5);
}

// Adapted from:
// https://github.com/zranger1/PixelblazePatterns/blob/master/Toolkit/sdf2d.md
vec3 sdfHeart(const vec2 uv, float r) {
    float m, d;
    const float x = abs(uv.x);
    const float y = r - uv.y;
    r *= 2.0;
    if (x+y>r) {
        d = (sqrt(dot2(x - r*0.25,y - r*0.75)) - (r*0.3536));
    } else {
        m = 0.5 * max(x + y,0.0);
        d = sqrt(min(dot2(x,y-1),dot2(x-m,y-m)) * sign(x-y));
    }
    return d > 0.0 ? vec3(0.0) : alternatingColor(uv, 1.0);
}

void main()
{
    // Obtain [0,1]^2 normalized uv screen coords
    const vec2 uv = gl_FragCoord.xy / viewPortSize.xy;
    const float aspect = viewPortSize.x / viewPortSize.y;
    //const float color = clamp(cos(4.0 * M_PI * uv.x * uv.y), 0.0, 1.0);
    //const vec3 color = 0.5 * (1.0 + cos(currentTime + uv.xyx + vec3(0.0, 2.0, 4.0)));
    
    vec2 xy = uv - 0.5;
    xy.x *= aspect;  // aspect ratio (orthographic projection)
    xy.y *= -1.0;
    const float t = 2.0 * currentTime;  // speed up animation
    const vec3 col2 = sdfCircle(xy, vec2(0.2*cos(t), 0.2*sin(t)), 0.2);
    const vec3 col1 = sdfHeart(xy, 0.4);
    
    outColor = vec4(clamp(col1 + col2, 0.0, 1.0), 1.0);
}
