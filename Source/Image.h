#pragma once

#include <string>

class Image
{
public:
    int width, height;
    unsigned char* data;

    unsigned int handle;
};

Image loadImage(std::string path);
