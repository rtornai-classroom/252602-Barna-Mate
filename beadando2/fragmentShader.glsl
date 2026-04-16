#version 330 core

out vec4 FragColor;
uniform vec3 objectColor;
uniform bool isPoint;

void main()
{
    // Ha pontot rajzolunk, levágjuk a négyzet sarkait, hogy kör legyen
    if (isPoint) {
        vec2 coord = gl_PointCoord - vec2(0.5);
        if (length(coord) > 0.5) {
            discard; // Eldobjuk a pixelt a körön kívül
        }
    }
    
    FragColor = vec4(objectColor, 1.0);
}