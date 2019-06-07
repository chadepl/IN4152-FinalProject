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
#include <string>

#ifndef M_PI
	#define M_PI 3.14159265358979323846
#endif
#define degToRad(angleInDegrees) ((angleInDegrees) * M_PI / 180.0)

std::map<std::string, int> textureHandles;


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
    
	//glActiveTexture(1);
	//glUniform1i(0, 1);
	if (model.texCoords.size() > 0) {
		int nMaterial = model.materials.size();

		// Bind texture of the model
		glBindTexture(GL_TEXTURE_2D, textureHandles[model.materials[0].diffuse_texname]);
	}

	
    glBindVertexArray(model.vao);
    glDrawArrays(GL_TRIANGLES, 0, model.vertices.size());
    
}



class Application : KeyListener, MouseMoveListener, MouseClickListener
{
public:
    Vector3f cameraPos = Vector3f(0.f, 10.f, -8.f); // before: 0.f, 1.f, 3.f
	Vector3f cameraTarget = Vector3f(0, 0, 1.0f); // 0, 0, -1.0f
	Vector3f cameraUp = Vector3f(0.f, 1.f, 0.f);

    void init()
    {
        window.setGlVersion(3, 3, true);
		window.create("Final Project", 1024, 1024);
        
        // Viewport for camera calculations
        glViewport(0, 0, WIDTH, HEIGHT);
        glGetIntegerv(GL_VIEWPORT, m_viewport);
		
		// Capture mouse pointer to look around (sneakily get real glfwWindow)
		glfwSetInputMode(window.windowPointer(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        
        // INIT GAME STATE
        
        game.characterPosition = Vector3f(0.f, 10.f, 0.f); //3.5f, 2.f, -3.f
        game.characterScalingFactor = .5f;
        game.characterTurboModeOn = false;
        
        game.hangarPosition = Vector3f(0.f, 1.f, 0.f);
        game.hangarScalingFactor = 1.f;
        game.turboModeOn = false;
        
        // -- light
        
        light.position = Vector3f(3.f, 20.f, 8.f);
        light.viewMatrix = lookAtMatrix(light.position, game.characterPosition, Vector3f(0.f, 1.f, 0.f));
        light.projectionMatrix = projectionProjectiveMatrix(60, 1, 0.1, 100);
        
        // LOADING MODELS
        
        // map
        map.resolution = 100;
        map.center = Vector3f(game.characterPosition.x, 0.f, game.characterPosition.z);
        map.model = makeMap(map.center, 100, 5.f, 50.f);

		//spacecraft = loadModel("Resources/spacecraft.obj");
        spacecraft = loadModelWithMaterials("Resources/spacecraft.obj");
        
        earth = loadModel("Resources/gijsEarth.obj");
        earth_texture = loadImage("Resources/"+earth.materials[0].diffuse_texname);
        std::cout << earth_texture.handle << std::endl;
        
        hangar = loadModel("Resources/Hangar2.obj");
        hangar_roof = loadImage("Resources/" + hangar.materials[0].diffuse_texname);
        std::cout << hangar_roof.handle << std::endl;

		textureHandles.insert(std::pair<std::string, int>(earth.materials[0].diffuse_texname, earth_texture.handle));
		textureHandles.insert(std::pair<std::string, int>(hangar.materials[0].diffuse_texname, hangar_roof.handle));
        
        // testing models
        testingQuad = makeQuad();
        testingQuad_tex = loadImage("Resources/earth-square.jpg");
        lightCube = loadCube();//loadModel("Resources/cube.obj");
        
        /////

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
            shadowShader.addShader(FRAGMENT, "Resources/shadow.frag");
            shadowShader.build();

            // Any new shaders can be added below in similar fashion
            // ....
            testShader.create();
            testShader.addShader(VERTEX, "Resources/test.vert");
            testShader.addShader(FRAGMENT, "Resources/test.frag");
            testShader.build();
        }
        catch (ShaderLoadingException e)
        {
            std::cerr << e.what() << std::endl;
        }

        // Correspond the OpenGL texture units 0 and 1 with the
        // colorMap and shadowMap uniforms in the shader
        defaultShader.bind();
        //defaultShader.uniform1i("colorMap", 0);
        //defaultShader.uniform1i("shadowMap", 1);

        // Upload the projection matrix once, if it doesn't change
        // during the game we don't need to reupload it
        defaultShader.uniformMatrix4f("projMatrix", projMatrix);

        glEnable(GL_DEPTH_TEST);
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        
        
        //////////////////// SHADOW MAP
        //// Create Shadow Texture
        glGenTextures(1, &texShadow);
        glBindTexture(GL_TEXTURE_2D, texShadow);
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, SHADOWTEX_WIDTH, SHADOWTEX_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        
        // Set behaviour for when texture coordinates are outside the [0, 1] range
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        // Set interpolation for texture sampling (GL_NEAREST for no interpolation)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // TODO: NEAREST
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        //// Create framebuffer for extra texture
        glGenFramebuffers(1, &framebuffer);
        
        //// Set shadow texure as depth buffer for this framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texShadow, 0);
        glClearDepth(1.0f);
        glClear(GL_DEPTH_BUFFER_BIT);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
    }

    void update()
    {
        // This is your game loop
        // Put your real-time logic and rendering in here
        while (!window.shouldClose())
        {
            
            processKeyboardInput();
            
            updateGameState();
            
            drawScene(true);
            
            drawScene(false);
            
            // TESTING
            
            //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
//            testShader.bind();
//            glViewport(0, 0, WIDTH * 2, HEIGHT *2);
//            glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
//            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//            // Bind the shadow map to texture slot 0
//            testShader.uniform1i("texShadow", 5);
//            //glActiveTexture(GL_TEXTURE0 + texture_unit);
//            //glBindTexture(GL_TEXTURE_2D, testingQuad_tex.handle);
//            //glBindTexture(GL_TEXTURE_2D, texShadow);
//
//
//            Model quad = makeQuad();
//            glBindVertexArray(quad.vao);
//            glDrawArrays(GL_TRIANGLES, 0, quad.vertices.size());
//
            
            // Processes input and swaps the window buffer
            window.update();
        }
    }
    
    // Method for drawing all the elements of the game
    void drawScene(bool forComputingShadows) {
        
        Matrix4f viewMatrix = lookAtMatrix(cameraPos, cameraPos + cameraTarget, cameraUp); // depends on processKeyboardInput();
        Matrix4f projMatrix = projectionProjectiveMatrix(45, m_viewport[2]/m_viewport[3], 0.1, 100);
        
        //ShaderProgram& shader = defaultShader;
        if(!forComputingShadows){
            defaultShader.bind();
            
            glViewport(0, 0, WIDTH * 2, HEIGHT *2);
            
            // Clear the screen
            glClearColor(0.94f, 1.f, 1.f, 1.f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            // Bind the shadow map to texture slot 0
            GLint texture_unit = 5;
            glActiveTexture(GL_TEXTURE0 + texture_unit);
            glBindTexture(GL_TEXTURE_2D, texShadow);
            defaultShader.uniform1i("shadowMap", texture_unit);
            
            defaultShader.uniformMatrix4f("projMatrix", projMatrix);
            defaultShader.uniformMatrix4f("viewMatrix", viewMatrix);
            
            defaultShader.uniformMatrix4f("light.projectionMatrix", light.projectionMatrix);
            defaultShader.uniformMatrix4f("light.viewMatrix", light.viewMatrix);
            defaultShader.uniform3f("light.position", light.position);
            defaultShader.uniform3f("light.ambientColor", light.ambientColor);
            defaultShader.uniform3f("light.diffuseColor", light.diffuseColor);
            defaultShader.uniform3f("light.specularColor", light.specularColor);
            defaultShader.uniform1f("light.lightPower", 20.f);
            
            defaultShader.uniform3f("viewPos", cameraPos);
            defaultShader.uniform1i("turboModeOn", game.turboModeOn);
            
        } else {
            
            // Bind the off-screen framebuffer
            glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
            
            // Clear the shadow map and set needed options
            glClearDepth(1.0f);
            glClear(GL_DEPTH_BUFFER_BIT);

            shadowShader.bind();
            
            // Set viewport size
            glViewport(0, 0, SHADOWTEX_WIDTH, SHADOWTEX_HEIGHT);
            
            shadowShader.uniformMatrix4f("projMatrix", light.projectionMatrix);
            shadowShader.uniformMatrix4f("viewMatrix", light.viewMatrix);
            shadowShader.uniform3f("viewPos", light.position);
            
            //shader = shadowShader;
            
        }
        
        if(!forComputingShadows){
            // 1. Draw map
            defaultShader.uniform1i("forTesting", 1); // REMOVE at the end
            drawModel(defaultShader, map.model, Vector3f(-0.5f, 0.f, 0.f),Vector3f(rotateAngle, rotateAngle*2, rotateAngle * 0.8), 1.f);
            
            
            // 2. Draw hangar
            // drawModel(defaultShader, hangar, game.hangarPosition, Vector3f(0, 0, 0), game.hangarScalingFactor);
            
            // 3. Draw spacecraft
            defaultShader.uniform1i("forTesting", 0); // REMOVE at the end
            drawModel(defaultShader, spacecraft, game.characterPosition, Vector3f(0, 0, 0), game.characterScalingFactor);
            
            // 4. Draw moving planets
            
            
            // 5. Draw arcs
            
            
            // 6. Draw OTHER stuff
                //drawModel(defaultShader, testingQuad, Vector3f(0, 5, 0));
            
            defaultShader.uniform1i("forTesting", 1); // REMOVE at the end
            //drawModel(defaultShader, lightCube, light.position, Vector3f(0.f,0.f,0.f), 0.5);
            
            defaultShader.uniform1i("forTesting", 1); // REMOVE at the end
            //drawModel(defaultShader, lightCube, Vector3f(0.f, 15.f, 0.f));
            
            //drawModel(defaultShader, dragon, Vector3f(0.f, 0.f, 0.f));
            //drawModel(defaultShader, davidHead, Vector3f(0.f, 0.f, 0.f));
        }
        
        if(forComputingShadows){
            
            // 1. Draw map
            drawModel(shadowShader, map.model, Vector3f(-0.5f, 0.f, 0.f),Vector3f(rotateAngle, rotateAngle*2, rotateAngle * 0.8), 1.f);
            
            // 2. Draw hangar
            // drawModel(defaultShader, hangar, game.hangarPosition, Vector3f(0, 0, 0), game.hangarScalingFactor);
            
            // 3. Draw spacecraft
            drawModel(shadowShader, spacecraft, game.characterPosition, Vector3f(0, 0, 0), game.characterScalingFactor);
            
            // 4. Draw moving planets
            
            // 5. Draw arcs
            
            // 6. Draw OTHER stuff
            
            //drawModel(shadowShader, lightCube, light.position, Vector3f(0.f,0.f,0.f), 0.5);
            
            //drawModel(shadowShader, lightCube, Vector3f(0.f, 15.f, 0.f));
            
            //drawModel(defaultShader, dragon, Vector3f(0.f, 0.f, 0.f));
            //drawModel(defaultShader, davidHead, Vector3f(0.f, 0.f, 0.f));
        }
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
    }
    
    // Method for updating the game state after each interaction
    void updateGameState(){
        
        light.ambientColor = Vector3f(0.2f, 0.2f, 0.2f);
        light.diffuseColor = Vector3f(0.5f, 0.5f, 0.5f);
        light.specularColor = Vector3f(1.0f, 1.0f, 1.0f);
        
        if(false){ // distance between player and center of the map            
            map.model = makeMap(map.center, 100, 1.f, 1.f);
        }
        
        
    }

	// Apply camera transformations according to keyboard input
	void processKeyboardInput() {
		if (mKeyPressed.empty()) return;


		if (mKeyPressed[GLFW_KEY_W]) {
			// Move forward using unit direction vector
			cameraPos += cameraTarget.normalize() * movementSpeed;
			
			//std::cout << "X: " << cameraPos.x << " Y: " << cameraPos.y << " Z: " << cameraPos.z << std::endl;
		}
		if (mKeyPressed[GLFW_KEY_S]) {
			// Move backward using unit direction vector
			cameraPos -= cameraTarget.normalize() * movementSpeed;

			//std::cout << "X: " << cameraPos.x << " Y: " << cameraPos.y << " Z: " << cameraPos.z << std::endl;		
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

		// Calculate offset between last and now
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

		cameraTarget = Vector3f(cos(degToRad(pitch)) * cos(degToRad(yaw)), sin(degToRad(pitch)), cos(degToRad(pitch)) * sin(degToRad(yaw)));
		cameraTarget.normalize();
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
    const int WIDTH = 1024;
    const int HEIGHT = 1024;
    
    // Game state
    struct Game {
        
        bool characterTurboModeOn;
        
        Vector3f characterPosition;
        float characterScalingFactor;
        
        Vector3f hangarPosition;
        float hangarScalingFactor;
        
        bool turboModeOn;
        
    } game;
    
    // Computed in a square from 0 to 1
    // Use world coordinates to compare it to objects
    struct Map {
        int resolution;
        Vector3f center;
        Model model;
    } map;
    
    // Light
    struct Light {
        Vector3f position;
        Matrix4f viewMatrix;
        Matrix4f projectionMatrix;
        Vector3f ambientColor;
        Vector3f diffuseColor;
        Vector3f specularColor;
    } light;

    // Shader for default rendering and for depth rendering
    ShaderProgram defaultShader;
    ShaderProgram testShader;
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
    
    Model sphere;
    Model dragon;
    Model davidHead;
	Model earth;
	Model hangar;
	Model spacecraft;

	Image earth_texture;
	Image hangar_roof;
    
    GLint m_viewport[4];
    
    // Testing models
    Model cube1;
    Model cube2;
    Model cubeNormals;
    Model testingQuad;
    Image testingQuad_tex;
    Model lightCube;
    
    // Shadow mapping
    const int SHADOWTEX_WIDTH  = 1024;
    const int SHADOWTEX_HEIGHT = 1024;
    GLuint texShadow;
    GLuint framebuffer;
};

int main()
{
    Application app;
    app.init();
    app.update();

    return 0;
}
