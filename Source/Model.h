#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
	#pragma comment(lib, "libnoise.lib")
#endif
#pragma once
#include "tiny_obj_loader.h"


#include <GDT/OpenGL.h>
#include <GDT/Vector2f.h>
#include <GDT/Vector3f.h>

#include <vector>
#include <string>
#include <noise/noise.h> // used for the Perlin noise generation

class Model
{
public:
    std::vector<Vector3f> vertices;
    std::vector<Vector3f> normals;
    std::vector<Vector2f> texCoords;
    std::vector<Vector3f> colors;
    std::vector<Vector3f> diffuseColors;
    std::vector<Vector3f> ambientColors;
    std::vector<Vector3f> specularColors;
    std::vector<float> shininessValues;
    
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

    GLuint vao;
};

Model loadModel(std::string path);
Model loadModelWithMaterials(std::string path);
Model makeMap(noise::module::Perlin perlinGenerator, float perlinSize, int resolution, float heightMult, float scale);
float getHeightMapPoint(Vector3f point, noise::module::Perlin perlinGenerator, float scale, float heightMult);
Model loadCube();
Model makeQuad();
Vector3f getColor(float height);
