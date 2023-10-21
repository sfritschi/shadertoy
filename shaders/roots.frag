#version 420 core

layout (location = 0) out vec4 outColor;

uniform vec2 viewPortSize;
uniform float currentTime;

// Roots of polynomial z^3 - 1. vec2.x real part vec2.y imaginary part
const vec2 roots[] = {
    vec2(1.0, 0.0),
    vec2(-0.5, sqrt(3.0)/2.0),
    vec2(-0.5, -sqrt(3.0)/2.0)
};

// Colors associated with individual roots
//const vec3 colors[] = {
//    vec3(0.0, 0.0, 1.0),  // blue
//    vec3(0.0, 1.0, 0.0),  // green
//    vec3(1.0, 0.0, 0.0)   // red
//};

const vec3 colors[] = {
    vec3(1.0, 1.0, 1.0),                                // white
    vec3(85.0 / 255.0, 205.0 / 255.0, 252.0 / 255.0),   // blue
    vec3(247.0 / 255.0, 168.0 / 255.0, 184.0 / 255.0)   // pink
};

const float circleRadius = 0.05;
const vec3 circleColor = vec3(0.0, 0.0, 0.0);

// Animation parameter
const int nSecondsRepeat = 20;  // repeat after every 20 seconds

void main()
{
    // Obtain [0,1]^2 normalized uv screen coords
    const vec2 uv = gl_FragCoord.xy / viewPortSize.xy;
    const float aspect = viewPortSize.x / viewPortSize.y;
    
    vec2 w = 4.0 * uv - 2.0;  // transform to [-2,2] range
    w.x *= aspect;  // aspect ratio (orthographic projection)
    
    bool isCircle = false;
    for (int i = 0; i < 3; ++i)
    {
        vec2 r = roots[i];
        r.x *= aspect;
        
        if (distance(w, r) < circleRadius)
        {
            isCircle = true;
            break;
        }
    }
    
    if (isCircle)
    {
        // Draw circle around root
        outColor = vec4(circleColor.rgb, 1.0);
        return;  // done
    }
    
    // Newton iteration using w as initial guess
    const int nIter = int(currentTime) % nSecondsRepeat;
    for (int i = 0; i < nIter; ++i)
    {
        // Evaluate real and complex sub-polynomials at current guess
        const vec2 p = vec2(
            w.x*w.x*w.x - 3.0*w.x*w.y*w.y - 1.0,
            (3.0*w.x*w.x - w.y*w.y)*w.y
        );
        // Evaluate inverse jacobian at current guess
        const float diag = w.x*w.x - w.y*w.y;
        const float off  = 2.0*w.x*w.y;
        float fac = w.x*w.x + w.y*w.y;
        fac *= fac;
        fac = 1.0 / (3.0 * fac);  // inverse determinant
        
        const mat2 invJac = fac * mat2(diag, -off, off, diag);
        
        // Carry out 1 Newton step
        w = w - invJac * p;
    }
    
    // Find index of closest root that this initial guess converged to
    int closest = 0;
    float minimum = 10000.0;  // arbitrary
    for (int i = 0; i < 3; ++i)
    {
        vec2 r = roots[i];
        r.x *= aspect;
        
        const float dist = distance(w, r);
        if (dist < minimum)
        {
            minimum = dist;
            closest = i;
        }
    }
    
    outColor = vec4(colors[closest], 1.0);
}
