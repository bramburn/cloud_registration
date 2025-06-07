#version 330 core

uniform vec3 color;
out vec4 fragColor;

void main()
{
    // Create circular point shape with smooth edges
    vec2 coord = gl_PointCoord - vec2(0.5);
    float distance = length(coord);
    if (distance > 0.5) {
        discard;
    }
    
    float alpha = 1.0 - smoothstep(0.3, 0.5, distance);
    fragColor = vec4(color, alpha);
}
