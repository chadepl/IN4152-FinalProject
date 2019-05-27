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
#define _USE_MATH_DEFINES
#include <math.h>
#include <map>

#ifndef M_PI
	#define M_PI 3.14159265358979323846
#endif
#define degToRad(angleInDegrees) ((angleInDegrees) * M_PI / 180.0)


// Produces a projection matrix for perspective projection
// http://www.songho.ca/opengl/gl_projectionmatrix.html
Matrix4f projectionProjectiveMatrix(float fov, float aspect, float nnear, float ffar){
    
    Matrix4f projectionMatrix;
    float fov_degrees = fov * M_PI / 180;
    
    projectionMatrix[0] = 1/(aspect * tan(fov_degrees/2));
    projectionMatrix[5] = 1/tan(fov_degrees/2);
    projectionMatrix[10] = -(ffar + nnear) / (ffar - nnear);
    projectionMatrix[11] = -1;
    projectionMatrix[14] = -2 * ffar * nnear / (ffar - nnear);
    
    return projectionMatrix;
}

Matrix4f projectionOrthographicMatric(float nnear, float ffar, float left, float right, float top, float bottom){
    
    Matrix4f projectionMatrix;
    
    projectionMatrix[0] = 2/(right-left);
    projectionMatrix[5] = 2/(top-bottom);
    projectionMatrix[10] = -2/(ffar - nnear);
    projectionMatrix[12] = -(right+left)/(right-left);
    projectionMatrix[13] = -(top+bottom)/(top-bottom);
    projectionMatrix[14] = -(ffar + nnear)/(ffar - nnear);
    
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
    shader.uniform1i("hasColor", model.colors.size() > 0);

    glBindVertexArray(model.vao);
    glDrawArrays(GL_TRIANGLES, 0, model.vertices.size());
    
}



class Application : KeyListener, MouseMoveListener, MouseClickListener
{
public:
	Vector3f cameraPos = Vector3f(0.f, 1.f, 3.f);
	Vector3f cameraTarget = Vector3f(0, 0, -1.0f);
	Vector3f cameraUp = Vector3f(0.f, 1.f, 0.f);

    void init()
    {
        window.setGlVersion(3, 3, true);
		window.create("Final Project", 1024, 1024);
        
        // Viewport for camera calculations
        glGetIntegerv( GL_VIEWPORT, m_viewport);
		
		// Capture mouse pointer to look around (sneakily get real glfwWindow)
		glfwSetInputMode(window.windowPointer(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

				//Load cube model
//        model = loadModel("Resources/cube_normals.obj");
        mapWidth = 5;
        mapDepth = 5;
        map = loadMap(mapWidth, mapDepth, 100);
        cube1 = loadCube();
        cube2 = loadCube();
        //dragon = loadModel("Resources/dragon.obj");
        //davidHead = loadModel("Resources/DavidHeadCleanMax.obj");

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
            

            
            Matrix4f projMatrix = projectionProjectiveMatrix(45, m_viewport[2]/m_viewport[3], 0.1, 100);
           // Matrix4f projMatrix = projectionOrthographicMatric(0.1, 5, -1, 1, 1, -1);
            
			processKeyboardInput();
            Matrix4f viewMatrix = lookAtMatrix(cameraPos, cameraPos + cameraTarget, cameraUp);
            
			
            defaultShader.uniformMatrix4f("projMatrix", projMatrix);
            defaultShader.uniformMatrix4f("viewMatrix", viewMatrix);
            
			Vector3f lightPos = Vector3f(0.f, 10.f, 0.f);

            //defaultShader.uniform3f("viewPos", cameraPos);
            //defaultShader.uniform3fv("viewPos", 3, &cameraPos);
            
//            drawModel(defaultShader, map, Vector3f(-0.5f, 0.f, 0.f),Vector3f(rotateAngle, rotateAngle*2, rotateAngle * 0.8), 1.f);
            
            drawModel(defaultShader, map, Vector3f(-mapWidth/2, 0.f, -mapDepth/2),Vector3f(0, 0, 0), 1.f);
//            drawModel(defaultShader, cube1, Vector3f(0.f, 0.f, 0.f),Vector3f(rotateAngle, rotateAngle, 0), 1.f);
            drawModel(defaultShader, cube2, Vector3f(1.5f, 0.f, -2.f),Vector3f(rotateAngle, rotateAngle, 0), 1.f);
            //drawModel(defaultShader, dragon, Vector3f(0.f, 0.f, 0.f));
            //drawModel(defaultShader, davidHead, Vector3f(0.f, 0.f, 0.f));
            
			rotateAngle = rotateAngle + 0.25f;
            // Processes input and swaps the window buffer
            window.update();
        }
    }

	// Apply camera transformations according to keyboard input
	void processKeyboardInput() {
		if (mKeyPressed.empty()) return;


		if (mKeyPressed[GLFW_KEY_W]) {
			//cameraPos += cameraPos * movementSpeed;
			//cameraPos += dot(movementSpeed, cameraTarget);
			
			Vector3f forwardVector = cameraTarget;

			forwardVector.normalize(); // normalise your vector

			//moving forward :
			cameraPos += forwardVector * movementSpeed;
			//cameraTarget += forwardVector * movementSpeed;
			std::cout << "X: " << cameraPos.x << " Y: " << cameraPos.y << " Z: " << cameraPos.z << std::endl;
		}
		if (mKeyPressed[GLFW_KEY_S]) {
			//cameraPos -= cameraPos * movementSpeed;
			Vector3f forwardVector = cameraTarget;

			forwardVector.normalize(); // normalise your vector

			//moving forward :
			cameraPos -= forwardVector * movementSpeed;
			//cameraTarget -= forwardVector * movementSpeed;


			std::cout << "X: " << cameraPos.x << " Y: " << cameraPos.y << " Z: " << cameraPos.z << std::endl;
			//cameraPos -= dot(movementSpeed, cameraTarget);
		}
		if (mKeyPressed[GLFW_KEY_A]) {
			cameraPos -= normalize(cross(cameraTarget, cameraUp)) * movementSpeed;
		}
		if (mKeyPressed[GLFW_KEY_D]) {
			cameraPos += normalize(cross(cameraTarget, cameraUp)) * movementSpeed;
			
		}
		//std::cout << "Camera pos x: " << cameraPos.x << cameraPos.y << cameraPos.z << std::endl;

	}

    // In here you can handle key presses
    // key - Integer that corresponds to numbers in https://www.glfw.org/docs/latest/group__keys.html
    // mods - Any modifier keys pressed, like shift or control
	void onKeyPressed(int key, int mods)
	{
		mKeyPressed[key] = true;
	}

    // In here you can handle key releases
    // key - Integer that corresponds to numbers in https://www.glfw.org/docs/latest/group__keys.html
    // mods - Any modifier keys pressed, like shift or control
    void onKeyReleased(int key, int mods)
    {
		mKeyPressed[key] = false;
    }

    // If the mouse is moved this function will be called with the x, y screen-coordinates of the mouse
    void onMouseMove(float x, float y)
    {
		if (!mouseCaptured) {
			last_x = x;
			last_y = y;
			mouseCaptured = true;
			return;
		}

		// Calculate offset between last mousepointer andnow
		float x_off = x - last_x;
		float y_off = y - last_y;

		last_x = x;
		last_y = y;
        
		x_off = x_off * mouseSensitivity;
		y_off = y_off * mouseSensitivity;

		pitch += y_off;
		if (pitch > 90.f) {
			pitch = 89.9f;
		}
		else if (pitch < -90.f) {
			pitch = -89.9f;
		}
		yaw = std::fmod((yaw + x_off), (GLfloat)360.0f);

		//std::cout << "PITCH: " << pitch << " YAW: " << yaw << std::endl;
		
		cameraTarget = Vector3f(cos(degToRad(pitch)) * cos(degToRad(yaw)), sin(degToRad(pitch)), cos(degToRad(pitch)) * sin(degToRad(yaw)));
		cameraTarget.normalize();
		//std::cout << "Target X: " << cameraTarget.x << "Y: " << cameraTarget.y << "Z: " << cameraTarget.z << std::endl;
		//std::cout << "Mouse at position: " << x << " " << y << std::endl;
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

	// Key and mouse input variables and parameters
	std::map<int, bool> mKeyPressed;
	float movementSpeed = 0.05f;
	float mouseSensitivity = 0.1f;
	bool mouseCaptured = false;
	float last_x, last_y;
	float pitch, yaw = 0.f;
    
    // Terrain
    Model map;
    double mapWidth, mapDepth;
    Model cube1;
    Model cube2;
    Model cubeNormals;
    Model sphere;
    Model dragon;
    Model davidHead;
    
    
    GLint m_viewport[4];
};

int main()
{
    Application app;
    app.init();
    app.update();

    return 0;
}
