#version 420 core

layout (location = 0) out vec4 outColor;

uniform vec2 viewPortSize;
uniform float deltaTime;

uniform sampler2D input;

const float M_PI = 3.1415926535897932384626433832795;
const float g = 9.81; // m/s^2
const float L = 1.0;  // m
const float m = 2.0;  // kg

// Evaluate RHS of ODE \alpha^\prime = f(\alpha) at current timestep
vec4 rhs(const vec4 alpha)
{
    // alpha = (.x = \theta_1, .y = \theta_2, .z = \omega_1, .w = \omega_2)
    const float dtheta = alpha.x - alpha.y;
    const float sdtheta = sin(dtheta);
    const float cdtheta = cos(dtheta);
    const float stheta1 = sin(alpha.x);
    const float stheta2 = sin(alpha.y);
    const float r = g / L;
    const float omega12 = alpha.z * alpha.z;
    const float omega22 = alpha.w * alpha.w;
    
    const float f1 = r * stheta2 * cdtheta 
        - sdtheta * (omega12 * cdtheta + omega22)
        - 2.0 * r * stheta1;
    
    const float f2 = 2.0 * (omega12 * sdtheta - r * stheta2 
        + r * stheta1 * cdtheta) + omega22 * sdtheta * cdtheta;
    
    const float denom = 1.0 + sdtheta * sdtheta;
    
    return vec4(alpha.z, alpha.w, f1 / denom, f2 / denom);
}

// Performs 1 timestep of Runge-Kutta 4th order SSM
vec4 rk4(const vec4 alpha, const float dt)
{
    const vec4 k1 = rhs(alpha);
    const vec4 k2 = rhs(alpha + dt/2.0 * k1);
    const vec4 k3 = rhs(alpha + dt/2.0 * k2);
    const vec4 k4 = rhs(alpha + dt * k3);
    
    return alpha + dt/6.0 * (k1 + 2.0 * k2 + 2.0 * k3 + k4);
}

void main()
{
    const vec2 uv = gl_FragCoord.xy / viewPortSize.xy;     // [0, 1]^2
    const float aspect = viewPortSize.x / viewPortSize.y;  // aspect ratio
    
    const vec4 alphaInput = texture(input, uv);
    
    outColor = rk4(alphaInput, deltaTime);
}
