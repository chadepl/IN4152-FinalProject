#version 330 core

// Global variables for lighting calculations
uniform sampler2D texShadow;

// Output for on-screen color
layout(location = 0) out vec4 outColor;

// Interpolated output data from vertex shader
in vec3 fragPos;    // World-space position
in vec2 outTexCoords;


void main() {
    
    float color = texture(texShadow, outTexCoords.xy).x;
    //float color = 0.5;
    
    //    if(shadowMapDepth < fragLightDepth - 0.0001){ // is in shadow
    //        outColor = vec4(0.0, 0.0, 0.0, 1.0);
    //    } else {
    //        outColor = vec4(vec3(max(dot(fragNormal, lightDir), 0.0)) * multiplier, 1.0);
    //    }
    
//    if (outTexCoords.y < 0.5f){
//        outColor = vec4(0.f, 1.f, 0.f, 1.f);
//    }
    
    float linearDepth = color * 2 - 1;
    linearDepth = 2 * 0.1 * 100 / (100+0.1 - linearDepth * (100-0.1));
    float res =  linearDepth/10;
    
    outColor = vec4(vec3(linearDepth), 1.0f);
    
    //outColor = vec4(vec3(color), 1.0);
    
}
