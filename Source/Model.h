#pragma comment(lib, "libnoise.lib")
#pragma once

#include <GDT/OpenGL.h>
#include <GDT/Vector2f.h>
#include <GDT/Vector3f.h>

#include <vector>
#include <string>

class Model
{
public:
    std::vector<Vector3f> vertices;
    std::vector<Vector3f> normals;
    std::vector<Vector2f> texCoords;
    std::vector<Vector3f> colors;

    GLuint vao;
};

Model loadModel(std::string path);
Model loadMap(float width, float depth, int resolution);
Model loadCube();
Vector3f getColor(float height);
