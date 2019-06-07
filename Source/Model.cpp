#include "Model.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include <iostream>

// For random number generators
#include <ctime>    // For time()
#include <cstdlib>  // For srand() and rand()


#include <array>
#include <vector>

#include <noise/noise.h>

Model loadModelWithMaterials(std::string path)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    
    std::string err;
    
    std::ifstream ifs(path.c_str());
    
    if (!ifs.is_open())
    {
        std::cerr << "Failed to find file: " << path << std::endl;
        exit(1);
    }
    
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, path.c_str(), "Resources/");
    
    if (!err.empty()) {
        std::cerr << err << std::endl;
    }
    
    if (!ret) {
        std::cerr << "Failed to load object: " << path << std::endl;
        exit(1);
    }
    
    Model model;
    
    if (attrib.normals.size() == 0)
    {
        std::cerr << "Model does not have normal vectors, please re-export with normals." << std::endl;
    }
    
    // Loop over shapes
    for (size_t s = 0; s < shapes.size(); s++) {
        // Loop over faces(polygon)
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            int fv = shapes[s].mesh.num_face_vertices[f];
            
            //if(materials.size() > 0) {
                tinyobj::material_t mat = materials[shapes[s].mesh.material_ids[f]];
                //std::cout << "Mesh - " << shapes[s].name << std::endl;
                //std::cout << "Diffuse: (" << mat.diffuse[0] << ", " << mat.diffuse[1] << ", " << mat.diffuse[2] << ") " << std::endl;
            //}
            // Materials to set
            
            
            
            // Loop over vertices in the face.
            for (size_t v = 0; v < fv; v++) {
                // access to vertex
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
                tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
                tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];
                model.vertices.push_back(Vector3f(vx, vy, vz));
                tinyobj::real_t nx = attrib.normals[3 * idx.normal_index + 0];
                tinyobj::real_t ny = attrib.normals[3 * idx.normal_index + 1];
                tinyobj::real_t nz = attrib.normals[3 * idx.normal_index + 2];
                model.normals.push_back(Vector3f(nx, ny, nz));
                
                model.diffuseColors.push_back(Vector3f(mat.diffuse[0], mat.diffuse[1], mat.diffuse[2]));
                model.ambientColors.push_back(Vector3f(mat.ambient[0], mat.ambient[1], mat.ambient[2]));
                model.specularColors.push_back(Vector3f(mat.specular[0], mat.specular[1], mat.specular[2]));
                model.shininessValues.push_back(mat.shininess);
                
                if (attrib.texcoords.size() > 0 && idx.texcoord_index >= 0) {
                    tinyobj::real_t tx = attrib.texcoords[2 * idx.texcoord_index + 0];
                    tinyobj::real_t ty = attrib.texcoords[2 * idx.texcoord_index + 1];
                    model.texCoords.push_back(Vector2f(tx, 1-ty));
                }
                
                // Optional: vertex colors
                // tinyobj::real_t red = attrib.colors[3*idx.vertex_index+0];
                // tinyobj::real_t green = attrib.colors[3*idx.vertex_index+1];
                // tinyobj::real_t blue = attrib.colors[3*idx.vertex_index+2];
            }
            index_offset += fv;
            
            
        }
    }
    
    model.shapes = shapes;
    model.materials = materials;
    
    glGenVertexArrays(1, &model.vao);
    glBindVertexArray(model.vao);
    
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, model.vertices.size() * sizeof(Vector3f), model.vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    GLuint nbo;
    glGenBuffers(1, &nbo);
    glBindBuffer(GL_ARRAY_BUFFER, nbo);
    glBufferData(GL_ARRAY_BUFFER, model.normals.size() * sizeof(Vector3f), model.normals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);
    GLuint diffuse_bo;
    glGenBuffers(1, &diffuse_bo);
    glBindBuffer(GL_ARRAY_BUFFER, diffuse_bo);
    glBufferData(GL_ARRAY_BUFFER, model.diffuseColors.size() * sizeof(Vector3f), model.diffuseColors.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);
    GLuint ambient_bo;
    glGenBuffers(1, &ambient_bo);
    glBindBuffer(GL_ARRAY_BUFFER, ambient_bo);
    glBufferData(GL_ARRAY_BUFFER, model.ambientColors.size() * sizeof(Vector3f), model.ambientColors.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(3);
    GLuint specular_bo;
    glGenBuffers(1, &specular_bo);
    glBindBuffer(GL_ARRAY_BUFFER, specular_bo);
    glBufferData(GL_ARRAY_BUFFER, model.specularColors.size() * sizeof(Vector3f), model.specularColors.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(4);
    GLuint shininess_bo;
    glGenBuffers(1, &shininess_bo);
    glBindBuffer(GL_ARRAY_BUFFER, shininess_bo);
    glBufferData(GL_ARRAY_BUFFER, model.shininessValues.size() * sizeof(Vector3f), model.shininessValues.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(5);
    
    
    if (model.texCoords.size() > 0)
    {
        GLuint tbo;
        glGenBuffers(1, &tbo);
        glBindBuffer(GL_ARRAY_BUFFER, tbo);
        glBufferData(GL_ARRAY_BUFFER, model.texCoords.size() * sizeof(Vector2f), model.texCoords.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(6, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(6);
    }
    
    return model;
}

Model loadModel(std::string path)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string err;

    std::ifstream ifs(path.c_str());

    if (!ifs.is_open())
    {
        std::cerr << "Failed to find file: " << path << std::endl;
        exit(1);
    }

    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, path.c_str(), "Resources/");
	
    if (!err.empty()) {
        std::cerr << err << std::endl;
    }

    if (!ret) {
        std::cerr << "Failed to load object: " << path << std::endl;
        exit(1);
    }
    
    Model model;

    if (attrib.normals.size() == 0)
    {
        std::cerr << "Model does not have normal vectors, please re-export with normals." << std::endl;
    }

    // Loop over shapes
    for (size_t s = 0; s < shapes.size(); s++) {
        // Loop over faces(polygon)
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            int fv = shapes[s].mesh.num_face_vertices[f];

            // Loop over vertices in the face.
            for (size_t v = 0; v < fv; v++) {
                // access to vertex
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
                tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
                tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];
                model.vertices.push_back(Vector3f(vx, vy, vz));
                tinyobj::real_t nx = attrib.normals[3 * idx.normal_index + 0];
                tinyobj::real_t ny = attrib.normals[3 * idx.normal_index + 1];
                tinyobj::real_t nz = attrib.normals[3 * idx.normal_index + 2];
                model.normals.push_back(Vector3f(nx, ny, nz));

                if (attrib.texcoords.size() > 0 && idx.texcoord_index >= 0) {
                    tinyobj::real_t tx = attrib.texcoords[2 * idx.texcoord_index + 0];
                    tinyobj::real_t ty = attrib.texcoords[2 * idx.texcoord_index + 1];
                    model.texCoords.push_back(Vector2f(tx, 1-ty));
                }
                
                // Optional: vertex colors
                // tinyobj::real_t red = attrib.colors[3*idx.vertex_index+0];
                // tinyobj::real_t green = attrib.colors[3*idx.vertex_index+1];
                // tinyobj::real_t blue = attrib.colors[3*idx.vertex_index+2];
            }
            index_offset += fv;

            // per-face material
            shapes[s].mesh.material_ids[f];
        }
    }

	model.shapes = shapes;
	model.materials = materials;

    glGenVertexArrays(1, &model.vao);
    glBindVertexArray(model.vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, model.vertices.size() * sizeof(Vector3f), model.vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    GLuint nbo;
    glGenBuffers(1, &nbo);
    glBindBuffer(GL_ARRAY_BUFFER, nbo);
    glBufferData(GL_ARRAY_BUFFER, model.normals.size() * sizeof(Vector3f), model.normals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);
    if (model.texCoords.size() > 0)
    {
        GLuint tbo;
        glGenBuffers(1, &tbo);
        glBindBuffer(GL_ARRAY_BUFFER, tbo);
        glBufferData(GL_ARRAY_BUFFER, model.texCoords.size() * sizeof(Vector2f), model.texCoords.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(2);
    }

    return model;
}

//float[][] buildHeightMap(float x, float z){
//    return rand() % 2;
//}

float getNoiseValue(noise::module::Perlin perlinGenerator, float posX, float posZ){
    float elevation = 1 * perlinGenerator.GetValue(1 * posX, 0, 1 * posZ)
        + 0.5 * perlinGenerator.GetValue(2 * posX, 0, 2 * posZ)
        + 0.25 * perlinGenerator.GetValue(4 * posX, 0, 4 * posZ);
    elevation = pow(elevation, 3);
    return elevation;
}


// Loads map
// Resolution refers to the number of squares (x2 number of triangles) per side
Model loadMap(float width, float depth, int resolution)
{
    
    Model model;
    srand(time(0));
    
    float stepX = (float) width/resolution;
    float stepZ = (float) depth/resolution;

    // Initialize noise
    noise::module::Perlin myModule;
    
    for (int i = 0; i < resolution; i++) {
        for (int j = 0; j < resolution; j++) {
            
            float pos1x = i * stepX;
            float pos2x = (i + 1) * stepX;
            
            float pos1z = j * stepZ;
            float pos2z = (j + 1) * stepZ;
            
            float height11 = getNoiseValue(myModule, pos1x, pos1z);
            Vector3f point11 = Vector3f(pos1x, height11, pos1z);
            
            float height12 = getNoiseValue(myModule, pos1x, pos2z);
            Vector3f point12 = Vector3f(pos1x, height12, pos2z);
            
            float height21 = getNoiseValue(myModule, pos2x, pos1z);
            Vector3f point21 = Vector3f(pos2x, height21, pos1z);
            
            float height22 = getNoiseValue(myModule, pos2x, pos2z);
            Vector3f point22 = Vector3f(pos2x, height22, pos2z);
            
            Vector3f P, Q;
            Vector3f normalVec;
            
            /*** First triangle ***/
            model.vertices.push_back(point11);
            model.vertices.push_back(point21);
            model.vertices.push_back(point22);
            
            P = point21 - point11;
            Q = point22 - point21;
            
            normalVec = cross(P, Q);
            
            model.normals.push_back(normalVec);
            model.normals.push_back(normalVec);
            model.normals.push_back(normalVec);
            
            model.colors.push_back(getColor(height11));
            model.colors.push_back(getColor(height21));
            model.colors.push_back(getColor(height22));
            
            /*** Second triangle ***/
            model.vertices.push_back(point11);
            model.vertices.push_back(point22);
            model.vertices.push_back(point12);
            
            P = point12 - point22;
            Q = point11 - point12;
            
            normalVec = cross(P, Q);
            
            model.normals.push_back(normalVec);
            model.normals.push_back(normalVec);
            model.normals.push_back(normalVec);
            
            model.colors.push_back(getColor(height11));
            model.colors.push_back(getColor(height22));
            model.colors.push_back(getColor(height12));
        }
    }
    
    glGenVertexArrays(1, &model.vao);
    glBindVertexArray(model.vao);
    
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, model.vertices.size() * sizeof(Vector3f), model.vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    
    GLuint nbo;
    glGenBuffers(1, &nbo);
    glBindBuffer(GL_ARRAY_BUFFER, nbo);
    glBufferData(GL_ARRAY_BUFFER, model.normals.size() * sizeof(Vector3f), model.normals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);
    
    if (model.texCoords.size() > 0)
    {
        GLuint tbo;
        glGenBuffers(1, &tbo);
        glBindBuffer(GL_ARRAY_BUFFER, tbo);
        glBufferData(GL_ARRAY_BUFFER, model.texCoords.size() * sizeof(Vector2f), model.texCoords.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(2);
    }
    
    GLuint cbo;
    glGenBuffers(1, &cbo);
    glBindBuffer(GL_ARRAY_BUFFER, cbo);
    glBufferData(GL_ARRAY_BUFFER, model.colors.size() * sizeof(Vector3f), model.colors.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(3);
    
    return model;
}

Vector3f WATER = Vector3f(0.2f, 0.6f, 1.f) * 0.5;
Vector3f BEACH = Vector3f(1.f, 0.8f, 0.4f) * 0.5;
Vector3f FOREST = Vector3f(0.f, 0.2f, 0.f) * 0.5;
Vector3f JUNGLE = Vector3f(0.2f, 0.8f, 0.2f) * 0.5;
Vector3f SAVANNAH = Vector3f(1.f, 0.8f, 0.f) * 0.5;
Vector3f DESERT = Vector3f(1.f, 0.4f, 0.f) * 0.5;
Vector3f SNOW = Vector3f(255.f, 255.f, 0.8f) * 0.5;

Vector3f getColor(float e){
    if (e < 0.1) return WATER;
    else if (e < 0.2) return BEACH;
    else if (e < 0.3) return FOREST;
    else if (e < 0.5) return JUNGLE;
    else if (e < 0.7) return SAVANNAH;
    else if (e < 0.9) return DESERT;
    else return SNOW;
}

Model loadCube(){
    
    Model cube;

    
    cube.vertices.push_back(Vector3f(-0.5f, -0.5f, -0.5f)); cube.vertices.push_back(Vector3f(0.5f, -0.5f, -0.5f)); cube.vertices.push_back(Vector3f(0.5f,  0.5f, -0.5f));
    cube.vertices.push_back(Vector3f(0.5f,  0.5f, -0.5f)); cube.vertices.push_back(Vector3f(-0.5f,  0.5f, -0.5f)); cube.vertices.push_back(Vector3f(-0.5f, -0.5f, -0.5f));
    cube.normals.push_back(Vector3f(0.0f,  0.0f, -1.0f));
    cube.normals.push_back(Vector3f(0.0f,  0.0f, -1.0f));
    cube.normals.push_back(Vector3f(0.0f,  0.0f, -1.0f));
    cube.normals.push_back(Vector3f(0.0f,  0.0f, -1.0f));
    cube.normals.push_back(Vector3f(0.0f,  0.0f, -1.0f));
    cube.normals.push_back(Vector3f(0.0f,  0.0f, -1.0f));

    
    cube.vertices.push_back(Vector3f(-0.5f, -0.5f,  0.5f)); cube.vertices.push_back(Vector3f(0.5f, -0.5f,  0.5f)); cube.vertices.push_back(Vector3f(0.5f,  0.5f,  0.5f));
    cube.vertices.push_back(Vector3f(0.5f,  0.5f,  0.5f)); cube.vertices.push_back(Vector3f(-0.5f,  0.5f,  0.5f)); cube.vertices.push_back(Vector3f(-0.5f, -0.5f,  0.5f));
    cube.normals.push_back(Vector3f(0.0f,  0.0f, 1.0f));
    cube.normals.push_back(Vector3f(0.0f,  0.0f, 1.0f));
    cube.normals.push_back(Vector3f(0.0f,  0.0f, 1.0f));
    cube.normals.push_back(Vector3f(0.0f,  0.0f, 1.0f));
    cube.normals.push_back(Vector3f(0.0f,  0.0f, 1.0f));
    cube.normals.push_back(Vector3f(0.0f,  0.0f, 1.0f));
    
    cube.vertices.push_back(Vector3f(-0.5f,  0.5f,  0.5f)); cube.vertices.push_back(Vector3f(-0.5f,  0.5f, -0.5f)); cube.vertices.push_back(Vector3f( -0.5f, -0.5f, -0.5f));
    cube.vertices.push_back(Vector3f(-0.5f, -0.5f, -0.5f)); cube.vertices.push_back(Vector3f(-0.5f, -0.5f,  0.5f)); cube.vertices.push_back(Vector3f(-0.5f,  0.5f,  0.5f));
    cube.normals.push_back(Vector3f(-1.0f,  0.0f,  0.0f));
    cube.normals.push_back(Vector3f(-1.0f,  0.0f,  0.0f));
    cube.normals.push_back(Vector3f(-1.0f,  0.0f,  0.0f));
    cube.normals.push_back(Vector3f(-1.0f,  0.0f,  0.0f));
    cube.normals.push_back(Vector3f(-1.0f,  0.0f,  0.0f));
    cube.normals.push_back(Vector3f(-1.0f,  0.0f,  0.0f));
    
    cube.vertices.push_back(Vector3f(0.5f,  0.5f,  0.5f)); cube.vertices.push_back(Vector3f(0.5f,  0.5f, -0.5f)); cube.vertices.push_back(Vector3f(0.5f, -0.5f, -0.5f));
    cube.vertices.push_back(Vector3f(0.5f, -0.5f, -0.5f)); cube.vertices.push_back(Vector3f(0.5f, -0.5f,  0.5f)); cube.vertices.push_back(Vector3f(0.5f,  0.5f,  0.5f));
    cube.normals.push_back(Vector3f(1.0f,  0.0f,  0.0f));
    cube.normals.push_back(Vector3f(1.0f,  0.0f,  0.0f));
    cube.normals.push_back(Vector3f(1.0f,  0.0f,  0.0f));
    cube.normals.push_back(Vector3f(1.0f,  0.0f,  0.0f));
    cube.normals.push_back(Vector3f(1.0f,  0.0f,  0.0f));
    cube.normals.push_back(Vector3f(1.0f,  0.0f,  0.0f));
    
    cube.vertices.push_back(Vector3f(-0.5f, -0.5f, -0.5f)); cube.vertices.push_back(Vector3f(0.5f, -0.5f, -0.5f)); cube.vertices.push_back(Vector3f(0.5f, -0.5f,  0.5f));
    cube.vertices.push_back(Vector3f(0.5f, -0.5f,  0.5f)); cube.vertices.push_back(Vector3f(-0.5f, -0.5f,  0.5f)); cube.vertices.push_back(Vector3f(-0.5f, -0.5f, -0.5f));
    cube.normals.push_back(Vector3f(0.0f, -1.0f,  0.0f));
    cube.normals.push_back(Vector3f(0.0f, -1.0f,  0.0f));
    cube.normals.push_back(Vector3f(0.0f, -1.0f,  0.0f));
    cube.normals.push_back(Vector3f(0.0f, -1.0f,  0.0f));
    cube.normals.push_back(Vector3f(0.0f, -1.0f,  0.0f));
    cube.normals.push_back(Vector3f(0.0f, -1.0f,  0.0f));
    
    cube.vertices.push_back(Vector3f(-0.5f,  0.5f, -0.5f)); cube.vertices.push_back(Vector3f(0.5f,  0.5f, -0.5f)); cube.vertices.push_back(Vector3f(0.5f,  0.5f,  0.5f));
    cube.vertices.push_back(Vector3f(0.5f,  0.5f,  0.5f)); cube.vertices.push_back(Vector3f(-0.5f,  0.5f,  0.5f)); cube.vertices.push_back(Vector3f(-0.5f,  0.5f, -0.5f));
    cube.normals.push_back(Vector3f(0.0f,  1.0f,  0.0f));
    cube.normals.push_back(Vector3f(0.0f,  1.0f,  0.0f));
    cube.normals.push_back(Vector3f(0.0f,  1.0f,  0.0f));
    cube.normals.push_back(Vector3f(0.0f,  1.0f,  0.0f));
    cube.normals.push_back(Vector3f(0.0f,  1.0f,  0.0f));
    cube.normals.push_back(Vector3f(0.0f,  1.0f,  0.0f));
    
//    Vector3f p1 = Vector3f(-0.5, -0.5, 0.5);
//    Vector3f p2 = Vector3f(0.5, -0.5, 0.5);
//    Vector3f p3 = Vector3f(0.5, 0.5, 0.5);
//    Vector3f p4 = Vector3f(-0.5, 0.5, 0.5);
//    Vector3f p5 = Vector3f(-0.5, -0.5, -0.5);
//    Vector3f p6 = Vector3f(0.5, -0.5, -0.5);
//    Vector3f p7 = Vector3f(0.5, 0.5, -0.5);
//    Vector3f p8 = Vector3f(-0.5, 0.5, -0.5);
//    Vector3f U, V;
    
//    cube.vertices.push_back(p5); cube.vertices.push_back(p6); cube.vertices.push_back(p7);
//    cube.vertices.push_back(p7); cube.vertices.push_back(p8); cube.vertices.push_back(p5);
//    cube.normals.push_back(Vector3f(0.0, 0.0, -1.0));
//    cube.normals.push_back(Vector3f(0.0, 0.0, -1.0));
//
//
//    cube.vertices.push_back(p1); cube.vertices.push_back(p2); cube.vertices.push_back(p3);
//    cube.vertices.push_back(p3); cube.vertices.push_back(p4); cube.vertices.push_back(p1);
//    cube.normals.push_back(Vector3f(0.0, 0.0, 1.0));
//    cube.normals.push_back(Vector3f(0.0, 0.0, 1.0));
//
//    cube.vertices.push_back(p4); cube.vertices.push_back(p8); cube.vertices.push_back(p5);
//    cube.vertices.push_back(p5); cube.vertices.push_back(p1); cube.vertices.push_back(p4);
//    cube.normals.push_back(Vector3f(-1.0, 0.0, 0.0));
//    cube.normals.push_back(Vector3f(-1.0, 0.0, 0.0));
//
//    cube.vertices.push_back(p3); cube.vertices.push_back(p7); cube.vertices.push_back(p6);
//    cube.vertices.push_back(p6); cube.vertices.push_back(p2); cube.vertices.push_back(p3);
//    cube.normals.push_back(Vector3f(1.0, 0.0, 0.0));
//    cube.normals.push_back(Vector3f(1.0, 0.0, 0.0));
//
//    cube.vertices.push_back(p5); cube.vertices.push_back(p6); cube.vertices.push_back(p2);
//    cube.vertices.push_back(p2); cube.vertices.push_back(p1); cube.vertices.push_back(p5);
//    cube.normals.push_back(Vector3f(0.0, -1.0, 0.0));
//    cube.normals.push_back(Vector3f(0.0, -1.0, 0.0));
//
//    cube.vertices.push_back(p8); cube.vertices.push_back(p7); cube.vertices.push_back(p3);
//    cube.vertices.push_back(p3); cube.vertices.push_back(p4); cube.vertices.push_back(p8);
//    cube.normals.push_back(Vector3f(0.0, 1.0, 0.0));
//    cube.normals.push_back(Vector3f(0.0, 1.0, 0.0));
    
    glGenVertexArrays(1, &cube.vao);
    glBindVertexArray(cube.vao);
    
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, cube.vertices.size() * sizeof(Vector3f), cube.vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    
    GLuint nbo;
    glGenBuffers(1, &nbo);
    glBindBuffer(GL_ARRAY_BUFFER, nbo);
    glBufferData(GL_ARRAY_BUFFER, cube.normals.size() * sizeof(Vector3f), cube.normals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);
    
    // My attempt to create a color buffer object
//    GLuint cbo;
//    glGenBuffers(1, &cbo);
//    glBindBuffer(GL_ARRAY_BUFFER, cbo);
//    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(Vector3f), colors.data(), GL_STATIC_DRAW);
//    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
//    glEnableVertexAttribArray(1);
    
    return cube;
    
}


Model makeQuad(){
    
    Model model;
    
    float size = 1.0;
    model.vertices.push_back(Vector3f(-size, size, 0.0));
    model.vertices.push_back(Vector3f(-size, -size, 0.0));
    model.vertices.push_back(Vector3f(size, -size, 0.0));
    
    model.vertices.push_back(Vector3f(-size, size, 0.0));
    model.vertices.push_back(Vector3f(size, -size, 0.0));
    model.vertices.push_back(Vector3f(size, size, 0.0));
    
    model.texCoords.push_back(Vector2f(0.0f, 1.0f));
    model.texCoords.push_back(Vector2f(0.0f, 0.0f));
    model.texCoords.push_back(Vector2f(1.0f, 0.0f));
    
    model.texCoords.push_back(Vector2f(0.0f, 1.0f));
    model.texCoords.push_back(Vector2f(1.0f, 0.0f));
    model.texCoords.push_back(Vector2f(1.0f, 1.0f));
    
    
    glGenVertexArrays(1, &model.vao);
    glBindVertexArray(model.vao);
    
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, model.vertices.size() * sizeof(Vector3f), model.vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    
    GLuint tbo;
    glGenBuffers(1, &tbo);
    glBindBuffer(GL_ARRAY_BUFFER, tbo);
    glBufferData(GL_ARRAY_BUFFER, model.texCoords.size() * sizeof(Vector2f), model.texCoords.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);
    
    return model;
    
}
