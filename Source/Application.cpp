#include "Model.h"
#include "Image.h"

#include <GDT/Window.h>
#include <GDT/Input.h>
#include <GDT/Shader.h>
#include <GDT/Matrix4f.h>
#include <GDT/Vector3f.h>
#include <GDT/Math.h>
#include <GDT/OpenGL.h>
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include <glm/glm/gtc/type_ptr.hpp>

#include <vector>
#include <iostream>

// Produces a projection matrix for perspective projection
// http://www.songho.ca/opengl/gl_projectionmatrix.html
Matrix4f projectionProjectiveMatrix(float near, float far, float left, float right, float top, float bottom){
    
    Matrix4f projectionMatrix;
    
    projectionMatrix[0] = 2*near/(right-left);
    projectionMatrix[5] = 2*near/(top-bottom);
    projectionMatrix[8] = (right+left)/(right-left);
    projectionMatrix[9] = (top+bottom)/(top-bottom);
    projectionMatrix[10] = -(far+near)/far-near;
    projectionMatrix[11] = -1;
    projectionMatrix[14] = -2*far*near/(far-near);
    projectionMatrix[15] = 0;
    
    return projectionMatrix;
}

Matrix4f projectionOrthographicMatric(float near, float far, float left, float right, float top, float bottom){
    
    Matrix4f projectionMatrix;
    
    projectionMatrix[0] = 2/(right-left);
    projectionMatrix[5] = 2/(top-bottom);
    projectionMatrix[10] = -2/(far-near);
    projectionMatrix[12] = -(right+left)/(right-left);
    projectionMatrix[13] = -(top+bottom)/(top-bottom);
    projectionMatrix[14] = -(far+near)/(far-near);
    
    return projectionMatrix;
    
}


// Produces a look-at matrix from the position of the camera (camera) facing the target position (target)
Matrix4f lookAtMatrix(Vector3f camera, Vector3f target, Vector3f up)
{
    Vector3f forward = normalize(target - camera);
    Vector3f right = normalize(cross(forward, up));
    up = cross(right, forward);
    
    Matrix4f lookAtMatrix;
    lookAtMatrix[0] = right.x; lookAtMatrix[4] = right.y; lookAtMatrix[8] = right.z;
    lookAtMatrix[1] = up.x; lookAtMatrix[5] = up.y; lookAtMatrix[9] = up.z;
    lookAtMatrix[2] = -forward.x; lookAtMatrix[6] = -forward.y; lookAtMatrix[10] = -forward.z;
    
    Matrix4f translateMatrix;
    translateMatrix[12] = -camera.x;
    translateMatrix[13] = -camera.y;
    translateMatrix[14] = -camera.z;
    
    return lookAtMatrix * translateMatrix;
}

// Rudimentary function for drawing models, feel free to replace or change it with your own logic
// Just make sure you let the shader know whether the model has texture coordinates
void drawModel(ShaderProgram& shader, const Model& model, Vector3f position, Vector3f rotation = Vector3f(0), float scale = 1)
{
    Matrix4f modelMatrix;
    modelMatrix.translate(position);
    modelMatrix.rotate(rotation);
    modelMatrix.scale(scale);

    shader.uniformMatrix4f("modelMatrix", modelMatrix);
    shader.uniform1i("hasTexCoords", model.texCoords.size() > 0);

    glBindVertexArray(model.vao);
    glDrawArrays(GL_TRIANGLES, 0, model.vertices.size());
    
}



class Application : KeyListener, MouseMoveListener, MouseClickListener
{
public:
    void init()
    {
        window.setGlVersion(3, 3, true);
        window.create("Final Project", 1024, 1024);

		//Load cube model
//        model = loadModel("Resources/cube_normals.obj");
        map = loadMap(1, 1, 10);

        window.addKeyListener(this);
        window.addMouseMoveListener(this);
        window.addMouseClickListener(this);

        try {
            defaultShader.create();
            defaultShader.addShader(VERTEX, "Resources/shader.vert");
            defaultShader.addShader(FRAGMENT, "Resources/shader.frag");
            defaultShader.build();

            shadowShader.create();
            shadowShader.addShader(VERTEX, "Resources/shadow.vert");
            shadowShader.build();

            // Any new shaders can be added below in similar fashion
            // ....
        }
        catch (ShaderLoadingException e)
        {
            std::cerr << e.what() << std::endl;
        }

        // Correspond the OpenGL texture units 0 and 1 with the
        // colorMap and shadowMap uniforms in the shader
        defaultShader.bind();
        defaultShader.uniform1i("colorMap", 0);
        defaultShader.uniform1i("shadowMap", 1);

        // Upload the projection matrix once, if it doesn't change
        // during the game we don't need to reupload it
        defaultShader.uniformMatrix4f("projMatrix", projMatrix);

        glEnable(GL_DEPTH_TEST);
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    }

    void update()
    {
        // This is your game loop
        // Put your real-time logic and rendering in here
        while (!window.shouldClose())
        {
            defaultShader.bind();

            // Clear the screen
            glClearColor(1.f, 1.f, 1.f, 1.f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // ...

//            drawModel(defaultShader, model, Vector3f(0.f, 0.f, 0.f), Vector3f(rotateAngle, rotateAngle*2, rotateAngle * 0.8), 0.2f);
            Vector3f observer = Vector3f(0.f,1.f,1.f);
            Vector3f target = Vector3f(0.f,0.f,-1.f);
            Matrix4f projMatrix;
            Matrix4f viewMatrix;
//            Matrix4f projMatrix = projectionProjectiveMatrix(-1, 1, -1, 1, 1, -1);
//            Matrix4f projMatrix = projectionOrthographicMatric(-1, 1, -1, 1, 1, -1);
            
//            Matrix4f viewMatrix = lookAtMatrix(observer, target, Vector3f(0.f, 0.f, 0.f));
            
            defaultShader.uniformMatrix4f("projMatrix", projMatrix);
            defaultShader.uniformMatrix4f("viewMatrix", viewMatrix);
            
            drawModel(defaultShader, map, Vector3f(-0.5f, 0.f, 0.f),Vector3f(rotateAngle, rotateAngle*2, rotateAngle * 0.8), 1.f);
            
			rotateAngle = rotateAngle + 0.25f;
            // Processes input and swaps the window buffer
            window.update();
        }
    }

    // In here you can handle key presses
    // key - Integer that corresponds to numbers in https://www.glfw.org/docs/latest/group__keys.html
    // mods - Any modifier keys pressed, like shift or control
    void onKeyPressed(int key, int mods)
    {
        std::cout << "Key pressed: " << key << std::endl;
    }

    // In here you can handle key releases
    // key - Integer that corresponds to numbers in https://www.glfw.org/docs/latest/group__keys.html
    // mods - Any modifier keys pressed, like shift or control
    void onKeyReleased(int key, int mods)
    {

    }

    // If the mouse is moved this function will be called with the x, y screen-coordinates of the mouse
    void onMouseMove(float x, float y)
    {
        std::cout << "Mouse at position: " << x << " " << y << std::endl;
    }

    // If one of the mouse buttons is pressed this function will be called
    // button - Integer that corresponds to numbers in https://www.glfw.org/docs/latest/group__buttons.html
    // mods - Any modifier buttons pressed
    void onMouseClicked(int button, int mods)
    {
        std::cout << "Pressed button: " << button << std::endl;
    }

    // If one of the mouse buttons is released this function will be called
    // button - Integer that corresponds to numbers in https://www.glfw.org/docs/latest/group__buttons.html
    // mods - Any modifier buttons pressed
    void onMouseReleased(int button, int mods)
    {

    }

private:
    Window window;

    // Shader for default rendering and for depth rendering
    ShaderProgram defaultShader;
    ShaderProgram shadowShader;

    // Projection and view matrices for you to fill in and use
    Matrix4f projMatrix;
    Matrix4f viewMatrix;
	Model model;
	float rotateAngle = 0.f;
    
    // Terrain
    Model map;
};

int main()
{
    Application app;
    app.init();
    app.update();

    return 0;
}
