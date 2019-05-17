#include "Model.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include <iostream>

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

Model loadMap(int width, int depth, int resolution)
{
    
    Model model;
    
    
//    // Loop over shapes
//    for (size_t s = 0; s < shapes.size(); s++) {
//        // Loop over faces(polygon)
//        size_t index_offset = 0;
//        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
//            int fv = shapes[s].mesh.num_face_vertices[f];
//
//            // Loop over vertices in the face.
//            for (size_t v = 0; v < fv; v++) {
//                // access to vertex
//                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
//                tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
//                tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
//                tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];
//                model.vertices.push_back(Vector3f(vx, vy, vz));
//                tinyobj::real_t nx = attrib.normals[3 * idx.normal_index + 0];
//                tinyobj::real_t ny = attrib.normals[3 * idx.normal_index + 1];
//                tinyobj::real_t nz = attrib.normals[3 * idx.normal_index + 2];
//                model.normals.push_back(Vector3f(nx, ny, nz));
//
//                if (attrib.texcoords.size() > 0) {
//                    tinyobj::real_t tx = attrib.texcoords[2 * idx.texcoord_index + 0];
//                    tinyobj::real_t ty = attrib.texcoords[2 * idx.texcoord_index + 1];
//                    model.texCoords.push_back(Vector2f(tx, 1-ty));
//                }
//
//                // Optional: vertex colors
//                // tinyobj::real_t red = attrib.colors[3*idx.vertex_index+0];
//                // tinyobj::real_t green = attrib.colors[3*idx.vertex_index+1];
//                // tinyobj::real_t blue = attrib.colors[3*idx.vertex_index+2];
//            }
//            index_offset += fv;
//
//            // per-face material
//            shapes[s].mesh.material_ids[f];
//        }
//    }
    
    float stepX = (float) width/resolution;
    float stepZ = (float) depth/resolution;
    
    unsigned int gridArea = resolution * resolution;
    
    for (int i = 0; i < resolution; i++) {
        for (int j = 0; j < resolution; j++) {
            
            float pos1x = i * stepX;
            float pos2x = (i + 1) * stepX;
            
            float pos1z = j * stepZ;
            float pos2z = (j + 1) * stepZ;
            
            float height = sin((pos1x+pos2x)/2) * sin((pos1z+pos2z)/2);
            
            float height11 = sin(pos1x) * sin(pos1z);
            Vector3f point11 = Vector3f(pos1x, height11, pos1z);
            
            float height12 = sin(pos1x) * sin(pos2z);
            Vector3f point12 = Vector3f(pos1x, height12, pos2z);
            
            float height21 = sin(pos2x) * sin(pos1z);
            Vector3f point21 = Vector3f(pos2x, height21, pos1z);
            
            float height22 = sin(pos2x) * sin(pos2z);
            Vector3f point22 = Vector3f(pos2x, height22, pos2z);
            
            
            // First triangle
            model.vertices.push_back(point11);
            model.vertices.push_back(point21);
            model.vertices.push_back(point12);
            
            model.normals.push_back(cross(point11, point21));
            
            // Complementary triangle
            model.vertices.push_back(point21);
            model.vertices.push_back(point22);
            model.vertices.push_back(point12);
            
            model.normals.push_back(cross(point22, point12));
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
//    GLuint nbo;
//    glGenBuffers(1, &nbo);
//    glBindBuffer(GL_ARRAY_BUFFER, nbo);
//    glBufferData(GL_ARRAY_BUFFER, model.normals.size() * sizeof(Vector3f), model.normals.data(), GL_STATIC_DRAW);
//    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
//    glEnableVertexAttribArray(1);
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
