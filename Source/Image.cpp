#include "Image.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <GDT/OpenGL.h>

#include <iostream>

Image loadImage(std::string path)
{
    int comp;
    int width, height;

    Image image;
    image.data = stbi_load(path.c_str(), &image.width, &image.height, &comp, 4);

    if (!image.data) {
        std::cout << "Failed to load image at: " << path << std::endl;
        exit(0);
    }

    glGenTextures(1, &image.handle);
    glBindTexture(GL_TEXTURE_2D, image.handle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, image.width, image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    return image;
}
