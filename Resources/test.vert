
#version 330 core

// Per-vertex attributes
layout(location = 0) in vec3 pos; // World-space position
layout(location = 1) in vec3 texCoords;

// Data to pass to fragment shader
out vec3 fragPos;
out vec3 outTexCoords;

void main() {
    // Transform 3D position into on-screen position
    gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);
    
    // Pass position and normal through to fragment shader
    fragPos = pos;
    outTexCoords = texCoords;
}
