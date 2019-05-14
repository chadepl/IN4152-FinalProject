#version 330

uniform sampler2D colorMap;
uniform sampler2D shadowMap;

uniform bool hasTexCoords;

in vec3 passPosition;
in vec3 passNormal;
in vec2 passTexCoord;
in vec4 passShadowCoord;

out vec4 fragColor;

void main()
{
    vec3 normal = normalize(passNormal);
    
    vec3 color = vec3(1, 1, 1);
    if (hasTexCoords)
        color = texture(colorMap, passTexCoord).rgb;

    // Output color value, change from (1, 0, 0) to something else
    fragColor = vec4(0, 0, 0, 1);
}
