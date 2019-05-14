#include "Model.h"
#include "Image.h"

#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include <glm/glm/gtc/type_ptr.hpp>

#include <GDT/Window.h>
#include <GDT/Input.h>
#include <GDT/Shader.h>
#include <GDT/Matrix4f.h>
#include <GDT/Vector3f.h>
#include <GDT/Math.h>
#include <GDT/OpenGL.h>

#include <vector>
#include <iostream>

// Rudimentary function for drawing models, feel free to replace or change it with your own logic
// Just make sure you let the shader know whether the model has texture coordinates
void drawModel(ShaderProgram& shader, const Model& model, Vector3f position, Vector3f rotation = Vector3f(0), float scale = 1)
{
    Matrix4f modelMatrix;
    modelMatrix.translate(position);
    modelMatrix.rotate(rotation);
    modelMatrix.scale(scale);
    
    //std::cout << "Model matrix: " << std::endl;
    //std::cout << modelMatrix.str() << std::endl;
    
    shader.uniformMatrix4f("modelMatrix", modelMatrix);
    //shader.uniform1i("hasTexCoords", model.texCoords.size() > 0);
    shader.uniform1i("hasTexCoords", false); // NEW

    glBindVertexArray(model.vao);
    glDrawArrays(GL_TRIANGLES, 0, model.vertices.size());
    
}

Matrix4f getGDTMatrix(glm::mat4 glmMatrix){
    Matrix4f tempMat = Matrix4f();
    const float *pSource = (const float*) glm::value_ptr(glmMatrix);
    for (int i = 0; i < 16; i++) tempMat[i] = pSource[i];
    return tempMat;
}

class Application : KeyListener, MouseMoveListener, MouseClickListener
{
public:
    void init()
    {
        window.setGlVersion(3, 3, true);
        window.create("Final Project", 1024, 1024);

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
        //defaultShader.uniformMatrix4f("projMatrix", projMatrix);

        glEnable(GL_DEPTH_TEST);
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        
        // Nicolas
        
        glGetIntegerv(GL_VIEWPORT, viewport);
        
        std::cout << "Viewport: " << std::endl;
        std::cout << "x: " << viewport[0] << std::endl;
        std::cout << "y: " << viewport[1] << std::endl;
        std::cout << "w: " << viewport[2] << std::endl;
        std::cout << "h: " << viewport[3] << std::endl;
        
        scaleValue = 1;
        translateZ = 0;
        
        model = loadModel("Resources/cube_normals.obj");
        std::vector<Vector3f> myvector = model.vertices; // NEW
        for (std::vector<Vector3f>::iterator it = myvector.begin() ; it != myvector.end(); ++it) { // NEW
            std::cout << ' ' << *it;
            std::cout << '\n';
        }
        //projMatrix[0] = 1 / tan(fov);
        //glGenVertexArrays(1, 0);
        
        
    }

    void update()
    {
        // This is your game loop
        // Put your real-time logic and rendering in here
        
        
        while (!window.shouldClose())
        {
            defaultShader.bind();

            // Clear the screen
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // ...
            
            glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 0.0f);
            glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
            glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
            cameraFront = glm::normalize(cameraFront);
            glm::mat4 viewMatrix_glm =  glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
            
            //std::cout << "View GLM matrix: " << std::endl;
            viewMatrix = getGDTMatrix(viewMatrix_glm);
            //std::cout << viewMatrix.str() << std::endl;
            
            glm::mat4 projMatrix_glm = glm::perspective(glm::radians(45.0f), (float)viewport[2]/(float)viewport[3], 0.1f, 100.0f);
            //std::cout << "Projection GLM matrix: " << std::endl;
            projMatrix = getGDTMatrix(projMatrix_glm);
            //std::cout << projMatrix.str() << std::endl;
            
            defaultShader.uniformMatrix4f("projMatrix", projMatrix); // NEW
            defaultShader.uniformMatrix4f("viewMatrix", viewMatrix); // NEW
            
            Vector3f modelPosition = Vector3f(0, 0, translateZ);
            Vector3f rotation = Vector3f(0);
            float scale = scaleValue;
            drawModel(defaultShader, model, modelPosition, rotation, scale);

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
        if (key == 263) { // NEW: LEFT
            scaleValue--;
            std::cout << "Scale value: " << scaleValue << std::endl;
        } else if (key == 262){ // NEW: RIGHT
            scaleValue++;
            std::cout << "Scale value: " << scaleValue << std::endl;
        } else if (key == 264){ // NEW: DOWN
            translateZ--;
            std::cout << "Translate value: " << translateZ << std::endl;
        } else if (key == 265){ // NEW: UP
            translateZ++;
            std::cout << "Translate value: " << translateZ << std::endl;
        }
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
    
    GLint viewport[4]; // NEW
    int scaleValue; // NEW
    int translateZ; // NEW
    
    Model model;
};

int main()
{
    Application app;
    app.init();
    app.update();

    return 0;
}
