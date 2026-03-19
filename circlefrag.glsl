#version 330 core
in vec2 FragPos;
out vec4 FragColor;

uniform vec2 circleCenter;
uniform float radius;
uniform vec3 colorCenter;
uniform vec3 colorEdge;

void main()
{
    float dist = distance(FragPos, circleCenter);
    
    // Ha a távolság nagyobb a sugárnál, eldobjuk (így lesz kör a négyzetbõl)
    if(dist > radius) {
        discard;
    }
    
    // Színátmenet (interpoláció)
    float t = dist / radius;
    vec3 finalColor = mix(colorCenter, colorEdge, t);
    
    FragColor = vec4(finalColor, 1.0);
}