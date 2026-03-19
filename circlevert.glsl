#version 330 core
layout (location = 0) in vec2 aPos;

uniform mat4 model;
uniform mat4 projection;

out vec2 FragPos;

void main()
{
    // Kiszámoljuk a tényleges világkoordinátát
    vec4 worldPos = model * vec4(aPos, 0.0, 1.0);
    FragPos = worldPos.xy; // Átadjuk a Fragment Shadernek
    
    gl_Position = projection * worldPos;
}