#include "Model.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include <iostream>

// For random number generators
#include <ctime>    // For time()
#include <cstdlib>  // For srand() and rand()

#include <noise/noise.h>

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

    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, &ifs);

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

                if (attrib.texcoords.size() > 0) {
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

float myNoise(float x, float z){
    return rand() % 2;
}


// Loads map
// Resolution refers to the number of squares (x2 number of triangles) per side
Model loadMap(float width, float depth, int resolution)
{
    
    Model model;
    
    float stepX = (float) width/resolution;
    float stepZ = (float) depth/resolution;
    
    srand(time(0));

    
    unsigned int gridArea = resolution * resolution;
    
    for (int i = 0; i < resolution; i++) {
        for (int j = 0; j < resolution; j++) {
            
            float pos1x = i * stepX;
            float pos2x = (i + 1) * stepX;
            
            float pos1z = j * stepZ;
            float pos2z = (j + 1) * stepZ;
            
            std::cout << "New pair of triangles: " << std::endl;
            std::cout << "PosT 1x: " << pos1x << std::endl;
            std::cout << "PosT 2x: " << pos2x << std::endl;
            std::cout << "PosT 1z: " << pos1z << std::endl;
            std::cout << "PosT 2z: " << pos2z << std::endl;
            
            float height11 = myNoise(pos1x, pos1z);
            Vector3f point11 = Vector3f(pos1x, height11, pos1z);
            
            float height12 = myNoise(pos1x, pos2z);
            Vector3f point12 = Vector3f(pos1x, height12, pos2z);
            
            float height21 = myNoise(pos2x, pos1z);
            Vector3f point21 = Vector3f(pos2x, height21, pos1z);
            
            float height22 = myNoise(pos2x, pos2z);
            Vector3f point22 = Vector3f(pos2x, height22, pos2z);
            
            std::cout << "Random: " << height11 << std::endl;
            
            Vector3f U, V;
            
            // First triangle
            model.vertices.push_back(point11);
            model.vertices.push_back(point21);
            model.vertices.push_back(point22);
            
            U = point21 - point11;
            V = point22 - point21;
            
            model.normals.push_back(cross(U, V));
            Vector3f normal = cross(U, V);
            
            // Complementary triangle
            model.vertices.push_back(point11);
            model.vertices.push_back(point22);
            model.vertices.push_back(point12);
            
            U = point12 - point22;
            V = point11 - point12;
            
            model.normals.push_back(cross(U, V));
        }
    }
    
    std::cout << "Size: " << model.vertices.size() << std::endl;
    
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

Model loadCube(){
    
    Model cube;
    
    Vector3f p1 = Vector3f(-0.5, -0.5, 0.5);
    Vector3f p2 = Vector3f(0.5, -0.5, 0.5);
    Vector3f p3 = Vector3f(0.5, 0.5, 0.5);
    Vector3f p4 = Vector3f(-0.5, 0.5, 0.5);
    Vector3f p5 = Vector3f(-0.5, -0.5, -0.5);
    Vector3f p6 = Vector3f(0.5, -0.5, -0.5);
    Vector3f p7 = Vector3f(0.5, 0.5, -0.5);
    Vector3f p8 = Vector3f(-0.5, 0.5, -0.5);
    Vector3f U, V;
    
    cube.vertices.push_back(p1); cube.vertices.push_back(p3); cube.vertices.push_back(p4);
    cube.vertices.push_back(p1); cube.vertices.push_back(p2); cube.vertices.push_back(p3);
    U = p4 - p3; V = p3 - p1; cube.normals.push_back(cross(U, V));
    U = p4 - p3; V = p3 - p1; cube.normals.push_back(cross(U, V));
    U = p4 - p3; V = p3 - p1; cube.normals.push_back(cross(U, V));
    U = p3 - p2; V = p2 - p1; cube.normals.push_back(cross(U, V));
    U = p3 - p2; V = p2 - p1; cube.normals.push_back(cross(U, V));
    U = p3 - p2; V = p2 - p1; cube.normals.push_back(cross(U, V));
    
    cube.vertices.push_back(p2); cube.vertices.push_back(p7); cube.vertices.push_back(p3);
    cube.vertices.push_back(p2); cube.vertices.push_back(p6); cube.vertices.push_back(p7);
    U = p3 - p7; V = p7 - p2; cube.normals.push_back(cross(U, V));
    U = p3 - p7; V = p7 - p2; cube.normals.push_back(cross(U, V));
    U = p3 - p7; V = p7 - p2; cube.normals.push_back(cross(U, V));
    U = p7 - p6; V = p6 - p2; cube.normals.push_back(cross(U, V));
    U = p7 - p6; V = p6 - p2; cube.normals.push_back(cross(U, V));
    U = p7 - p6; V = p6 - p2; cube.normals.push_back(cross(U, V));
    
    cube.vertices.push_back(p6); cube.vertices.push_back(p8); cube.vertices.push_back(p7);
    cube.vertices.push_back(p6); cube.vertices.push_back(p5); cube.vertices.push_back(p8);
    U = p7 - p8; V = p8 - p6; cube.normals.push_back(cross(U, V));
    U = p7 - p8; V = p8 - p6; cube.normals.push_back(cross(U, V));
    U = p7 - p8; V = p8 - p6; cube.normals.push_back(cross(U, V));
    U = p8 - p5; V = p5 - p6; cube.normals.push_back(cross(U, V));
    U = p8 - p5; V = p5 - p6; cube.normals.push_back(cross(U, V));
    U = p8 - p5; V = p5 - p6; cube.normals.push_back(cross(U, V));
    
    cube.vertices.push_back(p5); cube.vertices.push_back(p4); cube.vertices.push_back(p8);
    cube.vertices.push_back(p5); cube.vertices.push_back(p1); cube.vertices.push_back(p4);
    U = p8 - p4; V = p4 - p5; cube.normals.push_back(cross(U, V));
    U = p8 - p4; V = p4 - p5; cube.normals.push_back(cross(U, V));
    U = p8 - p4; V = p4 - p5; cube.normals.push_back(cross(U, V));
    U = p4 - p1; V = p1 - p5; cube.normals.push_back(cross(U, V));
    U = p4 - p1; V = p1 - p5; cube.normals.push_back(cross(U, V));
    U = p4 - p1; V = p1 - p5; cube.normals.push_back(cross(U, V));
    
    cube.vertices.push_back(p4); cube.vertices.push_back(p7); cube.vertices.push_back(p8);
    cube.vertices.push_back(p4); cube.vertices.push_back(p3); cube.vertices.push_back(p7);
    U = p8 - p7; V = p7 - p4; cube.normals.push_back(cross(U, V));
    U = p8 - p7; V = p7 - p4; cube.normals.push_back(cross(U, V));
    U = p8 - p7; V = p7 - p4; cube.normals.push_back(cross(U, V));
    U = p7 - p3; V = p3 - p4; cube.normals.push_back(cross(U, V));
    U = p7 - p3; V = p3 - p4; cube.normals.push_back(cross(U, V));
    U = p7 - p3; V = p3 - p4; cube.normals.push_back(cross(U, V));
    
    cube.vertices.push_back(p5); cube.vertices.push_back(p2); cube.vertices.push_back(p1);
    cube.vertices.push_back(p5); cube.vertices.push_back(p6); cube.vertices.push_back(p2);
    U = p1 - p2; V = p2 - p5; cube.normals.push_back(cross(U, V));
    U = p1 - p2; V = p2 - p5; cube.normals.push_back(cross(U, V));
    U = p1 - p2; V = p2 - p5; cube.normals.push_back(cross(U, V));
    U = p2 - p6; V = p6 - p5; cube.normals.push_back(cross(U, V));
    U = p2 - p6; V = p6 - p5; cube.normals.push_back(cross(U, V));
    U = p2 - p6; V = p6 - p5; cube.normals.push_back(cross(U, V));
    
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
