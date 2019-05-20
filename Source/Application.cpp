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
#include <GLFW/glfw3.h>

#include <vector>
#include <iostream>
#include <map>

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

	//glUniformMatrix4fv(1, 1, GL_FALSE, &View[0][0]);
	

    glBindVertexArray(model.vao);
    glDrawArrays(GL_TRIANGLES, 0, model.vertices.size());


}

void adjustView(Matrix4f viewMatrix, ShaderProgram& shader) {
	shader.uniformMatrix4f("viewMatrix", viewMatrix);
}

class Application : KeyListener, MouseMoveListener, MouseClickListener
{
public:
	// Camera parameters
	glm::vec3 cameraPos = glm::vec3(0.f, 0.f, 3.f);
	glm::vec3 cameraFront = glm::vec3(0.f, 0.f, -1.f);
	glm::vec3 cameraUp = glm::vec3(0.f, 1.f, 0.f);

    void init()
    {
        window.setGlVersion(3, 3, true);
        window.create("Final Project", 1024, 1024);

		//Load cube model
		model = loadModel("Resources/cube_normals.obj");

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

	Matrix4f cameraView() {
		Matrix4f view;
		
		glm::mat4 View = glm::lookAt(cameraPos, cameraFront, cameraUp);

		const float *pSource = (const float*)glm::value_ptr(View);
		for (int i = 0; i < 16; ++i)
			view[i] = pSource[i];

		return view;
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
			//Compute new camera location/view
			viewMatrix = cameraView();
			adjustView(viewMatrix, defaultShader);
			drawModel(defaultShader, model, Vector3f(0.f, 0.f, 0.f), Vector3f(rotateAngle, rotateAngle*2, rotateAngle * 0.8), 0.5f);
			rotateAngle = rotateAngle; //+ 0.25f;
            // Processes input and swaps the window buffer
            window.update();
        }
    }

    // In here you can handle key presses
    // key - Integer that corresponds to numbers in https://www.glfw.org/docs/latest/group__keys.html
    // mods - Any modifier keys pressed, like shift or control
    void onKeyPressed(int key, int mods)
    {
		if (key == GLFW_KEY_W) {
			cameraPos += 0.1f * cameraFront;
		}
		if (key == GLFW_KEY_S) {
			cameraPos -= 0.1f * cameraFront;
		}
		if (key == GLFW_KEY_A) {
			cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * 0.1f;
		}
		if (key == GLFW_KEY_D) {
			cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * 0.1f;
		}
		std::cout << "Camera pos x: " << cameraPos.x << cameraPos.y << cameraPos.z << std::endl;
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

	//Register keys and pressed state
	std::map<int, bool> mKeyPressed;

};

int main()
{
    Application app;
    app.init();
    app.update();

    return 0;
}
