#version 330

uniform sampler2D colorMap;

in vec3 passNormal;
in vec2 passTexCoord;

out vec4 fragColor;


void main()
{
    //fragColor = vec4(texture(colorMap, passTexCoord).rgb, 1.0);
    fragColor = vec4(1.f, 0.f, 0.f, 1.0);
}
