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
#include <ctime>
#include <cmath>

#include <noise/noise.h> // used for the Perlin noise generation

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
void drawModel(ShaderProgram& shader, const Model& model, Vector3f position, Vector3f rotation = Vector3f(0), float scale = 1, bool spacecraft = false)
{
    Matrix4f modelMatrix;
    modelMatrix.translate(position);
	if (spacecraft) {
		modelMatrix.rotate(rotation.y, 0, 1.f, 0);
		modelMatrix.rotate(rotation.x, 1.f, 0, 0);
		modelMatrix.rotate(rotation.z, 0, 0, 1.f);
	}
	else {
		modelMatrix.rotate(rotation);
	}
	
    modelMatrix.scale(scale);

	
    shader.uniformMatrix4f("modelMatrix", modelMatrix);
    shader.uniform1i("hasTexCoords", model.texCoords.size() > 0);
    
	//glActiveTexture(1);
	//glUniform1i(0, 1);
	if (model.texCoords.size() > 0) {
		int nMaterial = model.materials.size();

		// Bind texture of the model
        glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureHandles[model.materials[0].diffuse_texname]);
        shader.uniform1i("colorMap", 0);
	}

    glBindVertexArray(model.vao);
    glDrawArrays(GL_TRIANGLES, 0, model.vertices.size());
    
}

// Rudimentary function for drawing models, feel free to replace or change it with your own logic
// Just make sure you let the shader know whether the model has texture coordinates
void drawPlanet(ShaderProgram& shader, const Model& model, Vector3f& position, Vector3f rotation = Vector3f(0), float scale = 1, Vector3f rotationPoint = Vector3f(0), float distance=1.f)
{
	Matrix4f modelMatrix;
	
	float newX = distance * cos(degToRad(rotation.y)) + rotationPoint.x;
	float newZ = distance * sin(degToRad(rotation.y)) + rotationPoint.z;
	position = Vector3f(newX, position.y, newZ);
	
	modelMatrix.translate(position);
	modelMatrix.rotate(rotation.y, 0, 1.f, 0);

	modelMatrix.scale(scale);

	shader.uniformMatrix4f("modelMatrix", modelMatrix);
	shader.uniform1i("hasTexCoords", model.texCoords.size() > 0);

	if (model.texCoords.size() > 0) {
		int nMaterial = model.materials.size();

		// Bind texture of the model
        glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureHandles[model.materials[0].diffuse_texname]);
        shader.uniform1i("colorMap", 0);
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
		window.create("Grand Theft Spacecraft", 1024, 1024);
        
        // Viewport for camera calculations
        glViewport(0, 0, WIDTH, HEIGHT);
        glGetIntegerv(GL_VIEWPORT, m_viewport);
		
		// Capture mouse pointer to look around (sneakily get real glfwWindow)
		glfwSetInputMode(window.windowPointer(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        
        
        // INIT GAME STATE
        srand(time(0));
        
        // -- light
        
        light.position = Vector3f(0.f, 10.f, 0.f);
        light.viewMatrix = lookAtMatrix(light.position, Vector3f(0.f), Vector3f(0.f, 1.f, 0.f));
        light.projectionMatrix = projectionProjectiveMatrix(60, 1, 0.1, 100);
        
        light.ambientColor = Vector3f(0.2f, 0.2f, 0.2f);
        light.diffuseColor = Vector3f(0.5f, 0.5f, 0.5f);
        light.specularColor = Vector3f(1.0f, 1.0f, 1.0f);
        
        // -- general game state
        
        initGameState();
        
        // -- loading models
        
        map.center = Vector3f(0.f);
        map.model = makeTerrain(map.perlinGenerator, map.perlinSize, map.resolution, map.heightMult, map.scale, false);
        
        ocean.center = Vector3f(0.f);
        //ocean.resolution = 100;
        ocean.perlinSize = 3;
        ocean.heightMult = 1.f;
        ocean.model = makeTerrain(ocean.perlinGenerator, ocean.perlinSize, ocean.resolution, ocean.heightMult, ocean.scale, true);
        
		
        spacecraft = loadModelWithMaterials("Resources/spacecraft.obj", "Resources/");
        
        explosion.numFrames = 9;
        for (int i = explosion.firstFrame; i < explosion.numFrames + 1; ++i) {
            explosion.frames.push_back(loadModelWithMaterials("Resources/spacecraftExplosion/spacecraftExplosion_00000"+ std::to_string(i)+".obj", "Resources/spacecraftExplosion/"));
            
            //std::cout << "Loading obstacle: " << i << std::endl;
            //std::cout << "Position: (" << newObstacle.position.x << ", " << newObstacle.position.y << ", " << newObstacle.position.z << ")" << std::endl;
        }
        
        skybox = loadModel("Resources/skysphereblue.obj");
        skybox_texture = loadImage("Resources/" + skybox.materials[0].diffuse_texname);
		skyboxBH = loadModel("Resources/skysphereblueBH.obj");
		skybox_BH_texture = loadImage("Resources/" + skyboxBH.materials[0].diffuse_texname);

		starSkybox = loadModel("Resources/skybox.obj");
		starsky_texture = loadImage("Resources/" + starSkybox.materials[0].diffuse_texname);

        earth = loadModelWithMaterials("Resources/gijsEarth.obj", "Resources/");
        earth_texture = loadImage("Resources/"+earth.materials[0].diffuse_texname);
		mars = loadModelWithMaterials("Resources/mars.obj", "Resources/");
		mars_texture = loadImage("Resources/" + mars.materials[0].diffuse_texname);
		pinkplanet = loadModelWithMaterials("Resources/pink.obj", "Resources/");
		pink_texture = loadImage("Resources/" + pinkplanet.materials[0].diffuse_texname);

		pEarth.position = Vector3f(0.f, 30.f, 0.f);
		pEarth.rotationAngle = 0.f;
		pMars.position = Vector3f(20.f, 30.f, 2.f);
		pMars.rotationAngle = 0.f;
		pTest.position = Vector3f(10.f, 30.f, 12.f);
		pTest.rotationAngle = 0.f;

        hangar = loadModelWithMaterials("Resources/Hangar2.obj", "Resources/");
        hangar_roof = loadImage("Resources/" + hangar.materials[0].diffuse_texname);

		textureHandles.insert(std::pair<std::string, int>(earth.materials[0].diffuse_texname, earth_texture.handle));
		textureHandles.insert(std::pair<std::string, int>(hangar.materials[0].diffuse_texname, hangar_roof.handle));
		textureHandles.insert(std::pair<std::string, int>(mars.materials[0].diffuse_texname, mars_texture.handle));
		textureHandles.insert(std::pair<std::string, int>(pinkplanet.materials[0].diffuse_texname, pink_texture.handle));
		textureHandles.insert(std::pair<std::string, int>(skybox.materials[0].diffuse_texname, skybox_texture.handle));
		textureHandles.insert(std::pair<std::string, int>(skyboxBH.materials[0].diffuse_texname, skybox_BH_texture.handle));

		textureHandles.insert(std::pair<std::string, int>(starSkybox.materials[0].diffuse_texname, starsky_texture.handle));

        // testing models
        testingQuad = makeQuad();
        //testingQuad_tex = loadImage("Resources/earth-square.jpg");
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
            
            skySphereShader.create();
            skySphereShader.addShader(VERTEX, "Resources/shaderSkySphere.vert");
            skySphereShader.addShader(FRAGMENT, "Resources/shaderSkySphere.frag");
            skySphereShader.build();
            
            testShader.create();
            testShader.addShader(VERTEX, "Resources/test.vert");
            testShader.addShader(FRAGMENT, "Resources/test.frag");
            testShader.build();
        }
        catch (ShaderLoadingException e)
        {
            std::cerr << e.what() << std::endl;
        }

        defaultShader.bind();

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
            
            // SHADOWS
            drawScene(true);
            
            
            glClearColor(0.f, 0.f, 0.f, 1.f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            // NORMAL SCENE
            drawScene(false);
            
            // SKY SPHERES
            skySphereShader.bind();
            #if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
            glViewport(0, 0, WIDTH, HEIGHT);
            #else
            glViewport(0, 0, WIDTH * 2, HEIGHT * 2);
            #endif
            
            skySphereShader.uniformMatrix4f("projMatrix", game.projMatrix);
            skySphereShader.uniformMatrix4f("viewMatrix", game.characterViewMatrix);

			if (true) { //TODO If not all arcs are crossed
				drawModel(skySphereShader, skybox, Vector3f(0.f), Vector3f(0.f), map.scale / 2, false);
			}
			else {
				drawModel(skySphereShader, skyboxBH, Vector3f(0.f), Vector3f(0.f), map.scale / 2, false);
				drawModel(skySphereShader, starSkybox, Vector3f(-95.f, 60.f, 140.f), Vector3f(0.f), 75.f, false);
			}

            
            // TESTING
            
//            testShader.bind();
//            glViewport(0, 0, WIDTH * 2, HEIGHT *2);
//            glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
//            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//            testShader.uniform1i("texShadow", 1);
//
//            Model quad = makeQuad();
//            glBindVertexArray(quad.vao);
//            glDrawArrays(GL_TRIANGLES, 0, quad.vertices.size());

            
            // Processes input and swaps the window buffer
            window.update();
        }
    }
    
    // Method for drawing all the elements of the game
    void drawScene(bool forComputingShadows) {
        
        if(!forComputingShadows){
            defaultShader.bind();
			#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
			glViewport(0, 0, WIDTH, HEIGHT);
			#else
			glViewport(0, 0, WIDTH * 2, HEIGHT * 2);
			#endif

            
            // Bind the shadow map to texture slot 0            
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, texShadow);
            defaultShader.uniform1i("shadowMap", 1);
            
            defaultShader.uniformMatrix4f("projMatrix", game.projMatrix);
            defaultShader.uniformMatrix4f("viewMatrix", game.characterViewMatrix);
            
            defaultShader.uniformMatrix4f("light.projectionMatrix", light.projectionMatrix);
            defaultShader.uniformMatrix4f("light.viewMatrix", light.viewMatrix);
            defaultShader.uniform3f("light.position", light.position);
            defaultShader.uniform3f("light.ambientColor", light.ambientColor);
            defaultShader.uniform3f("light.diffuseColor", light.diffuseColor);
            defaultShader.uniform3f("light.specularColor", light.specularColor);
            defaultShader.uniform1f("light.lightPower", light.power);
            
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
            
        }
        
        if(!forComputingShadows){
            // 1. Draw map
            defaultShader.uniform1i("forTesting", false); // REMOVE at the end
            drawModel(defaultShader, map.model, Vector3f(-0.5f, 0.f, 0.f),Vector3f(rotateAngle, rotateAngle*2, rotateAngle * 0.8), 1.f);
            
            defaultShader.uniform1i("forTesting", false); // REMOVE at the end
            drawModel(defaultShader, ocean.model, Vector3f(-0.5f, 0.f, 0.f),Vector3f(rotateAngle, rotateAngle*2, rotateAngle * 0.8), 1.f);
            
            
            // 2. Draw hangar
            defaultShader.uniform1i("forTesting", false);
            drawModel(defaultShader, hangar, game.hangarPosition, Vector3f(0, 0, 0), game.hangarScalingFactor);
            
            // 3. Draw spacecraft
            defaultShader.uniform1i("forTesting", false); // REMOVE at the end
			
            drawModel(defaultShader, spacecraft, game.characterPosition, Vector3f(-pitch, -yaw + 90.f,game.characterRoll), game.characterScalingFactor, true);
            
            // 4. Draw moving planets
//            drawModel(defaultShader, earth, pEarth.position, Vector3f(0, pEarth.rotationAngle, 0), 4.f);
//            drawPlanet(defaultShader, mars, pMars.position, Vector3f(0, pEarth.rotationAngle * 5, 0), 5.f, pEarth.position, 25.f);
//            drawPlanet(defaultShader, pinkplanet, pTest.position, Vector3f(0, pTest.rotationAngle , 0), 1.5f, pMars.position, 12.f);


            // 5. Draw arcs
            for (std::vector<Obstacle>::iterator it = obstacles.begin() ; it != obstacles.end(); ++it){
                Obstacle obs = *it;
                if(obs.crossed) defaultShader.uniform1i("forTesting", true); // REMOVE at the end
                else defaultShader.uniform1i("forTesting", false); // REMOVE at the end
                drawModel(defaultShader, obs.model, obs.position, obs.rotation, obs.scaling);
            }
            
            // 6. Skyboxes
            
            //defaultShader.uniform1i("forTesting", 0); // REMOVE at the end
            //drawModel(defaultShader, skybox, Vector3f(0.f), Vector3f(0.f), map.scale/2);
            
            
            // 7. Draw OTHER stuff
                //drawModel(defaultShader, testingQuad, Vector3f(0, 5, 0));
            if(explosion.on){
                drawModel(defaultShader, explosion.frames.front(), game.characterPosition, Vector3f(-pitch, -yaw + 90.f,game.characterRoll), game.characterScalingFactor, true);
            }
            

            defaultShader.uniform1i("forTesting", false); // REMOVE at the end
            //drawModel(defaultShader, arcTest, Vector3f(0.f, 10.f, 5.f), Vector3f(0.f,0.f,0.f), 1);
            
            defaultShader.uniform1i("forTesting", true); // REMOVE at the end
            //drawModel(defaultShader, lightCube, light.position, Vector3f(0.f,0.f,0.f), 0.5);
            
            defaultShader.uniform1i("forTesting", true); // REMOVE at the end
            drawModel(defaultShader, lightCube, light.position);
            
            //drawModel(defaultShader, dragon, Vector3f(0.f, 0.f, 0.f));
            //drawModel(defaultShader, davidHead, Vector3f(0.f, 0.f, 0.f));
        }
        
        if(forComputingShadows){
            
            // 1. Draw map
            drawModel(shadowShader, map.model, Vector3f(-0.5f, 0.f, 0.f),Vector3f(rotateAngle, rotateAngle*2, rotateAngle * 0.8), 1.f);
            
            // 2. Draw hangar
            drawModel(shadowShader, hangar, game.hangarPosition, Vector3f(0, 0, 0), game.hangarScalingFactor);            
            
            // 3. Draw spacecraft
            drawModel(shadowShader, spacecraft, game.characterPosition, Vector3f(-pitch, -yaw + 90.f, game.characterRoll), game.characterScalingFactor, true);
            
            // 4. Draw moving planets
            
            // 5. Draw arcs
            for (std::vector<Obstacle>::iterator it = obstacles.begin() ; it != obstacles.end(); ++it){
                Obstacle obs = *it;
                drawModel(shadowShader, obs.model, obs.position, obs.rotation, obs.scaling);
            }
            
            // 6. Draw OTHER stuff
            
            //drawModel(shadowShader, arcTest, Vector3f(0.f, 10.f, 5.f), Vector3f(0.f,0.f,0.f), 1);
            
            //drawModel(shadowShader, lightCube, light.position, Vector3f(0.f,0.f,0.f), 0.5);
            
            //drawModel(shadowShader, lightCube, Vector3f(0.f, 15.f, 0.f));
            
            //drawModel(defaultShader, dragon, Vector3f(0.f, 0.f, 0.f));
            //drawModel(defaultShader, davidHead, Vector3f(0.f, 0.f, 0.f));
        }
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
    }
    
    void initGameState(){
        game.characterPosition = Vector3f(0.f, 2.f, 0.f); //3.5f, 2.f, -3.f
        cameraPos = game.characterPosition + -cameraTarget * .2f;
        std::cout << game.characterPosition << std::endl;
        std::cout << cameraPos << std::endl;
        game.characterScalingFactor = .1f;
        game.characterRoll = 0.f;
        game.characterRollSensitivity = 0.5f;
        game.characterTurboModeOn = false;
        game.characterIsNotExploded = true;
        
        explosion.currentFrame = 1;
        explosion.on = false;
        
        game.hangarPosition = Vector3f(0.f, 1.f, 0.f);
        game.hangarScalingFactor = 1.f;
        game.turboModeOn = false;
        game.gameStart = false;
        
        if(!obstacles.empty()){
            for (auto &obs : obstacles){
                obs.crossed = false;
            }
        }else{
            for (int i = 0; i < 10; ++i) {
                Obstacle newObstacle;
                
                float randomPositionX = rand()%(int)round(map.scale) - (int)round(map.scale/2);
                float randomPositionZ = rand()%(int)round(map.scale) - (int)round(map.scale/2);
                float height = getHeightMapPoint(Vector3f(randomPositionX, 0.f, randomPositionZ), map.perlinGenerator, map.perlinSize, map.scale, map.heightMult);
                
                newObstacle.position = Vector3f(randomPositionX, height + 10, randomPositionZ);
                newObstacle.model = loadModelWithMaterials("Resources/obstacleArcSimplified.obj", "Resources/");
                newObstacle.scaling = 1.f;
                newObstacle.rotation = Vector3f(0.f, 90.f, 0.f);
                obstacles.push_back(newObstacle);
                //std::cout << "Loading obstacle: " << i << std::endl;
                //std::cout << "Position: (" << newObstacle.position.x << ", " << newObstacle.position.y << ", " << newObstacle.position.z << ")" << std::endl;
            }
        }
    }
    
    
    // Method for updating the game state after each frame
    void updateGameState(){
		// Move character forward if user has pressed any key
		if(game.gameStart && game.characterIsNotExploded)
			game.characterPosition += cameraTarget.normalize() * movementSpeed;
        
        float currentGroundHeight = getHeightMapPoint(game.characterPosition, map.perlinGenerator, map.perlinSize, map.scale, map.heightMult);
        
        // check collisions with ceiling and floor
    //    if(game.characterPosition.length() > map.scale/2 || game.characterPosition.y <= currentGroundHeight){
    //        
    //        if(!explosion.on){
    //            explosion.on = true;
    //            game.characterIsNotExploded = false;
    //            explosion.startTime = clock();
    //        } else {
    //            clock_t elapsed = clock();
    //            double elapsed_secs = double(elapsed - explosion.startTime) / CLOCKS_PER_SEC;
    //            //std::cout << "Elapsed: " << elapsed_secs << std::endl;
				//if (elapsed_secs > 10) {
				//	mKeyPressed.clear();
				//	initGameState();
				//}
    //        }
    //        
    //    }
        
        // check if has crosed and arc
        
        for (auto &obs : obstacles){
            Vector3f distanceVector = (obs.position - game.characterPosition);
            float distance = std::abs(distanceVector.length());
            if(distance < 1) obs.crossed = true;
        }
        
        
        
        // Matrices updates
		if (!game.cameraFirstPerson) {
			cameraPos = game.characterPosition + -(cameraTarget + Vector3f(0, -0.5f, 0))  *2.f;
		}
		else {
			cameraPos = game.characterPosition + -cameraTarget * .2f;
		}
		game.characterViewMatrix = lookAtMatrix(cameraPos, game.characterPosition, cameraUp); // depends on processKeyboardInput();
		game.projMatrix = projectionProjectiveMatrix(45, m_viewport[2] / m_viewport[3], 0.1, 100);

        
        // Light updates
        
        light.position = Vector3f(3.f, 40.f, 8.f);
        light.viewMatrix = lookAtMatrix(light.position, game.characterPosition, Vector3f(0.f, 1.f, 0.f));
        light.projectionMatrix = projectionProjectiveMatrix(60, 1, 0.1, 100);
        
        light.ambientColor = Vector3f(0.2f, 0.2f, 0.2f);
        light.diffuseColor = Vector3f(0.5f, 0.5f, 0.5f);
        light.specularColor = Vector3f(1.0f, 1.0f, 1.0f);
        
        if (game.turboModeOn){
            movementSpeed = 0.5f;
        } else {
            movementSpeed = 0.05f;
        }
        
        pEarth.rotationAngle += 0.1f;
        pMars.rotationAngle += 0.05f;
        pTest.rotationAngle += 0.01f;
        
        // Check, for all the arcs if the spaceship has traversed them and change something if thats the case.
        
        
        
    }

	// Apply camera transformations according to keyboard input
	void processKeyboardInput() {
		if (mKeyPressed.empty()) {
			game.characterRoll /= 1.25f;
			return;
		}

		game.gameStart = true;
        
        if(game.characterIsNotExploded){
            if (mKeyPressed[GLFW_KEY_W]) {
                // Move forward using unit direction vector
                //cameraPos += cameraTarget.normalize() * movementSpeed;
                game.characterPosition += cameraTarget.normalize() * movementSpeed;
				std::cout << game.characterPosition << std::endl;
                //std::cout << "X: " << cameraPos.x << " Y: " << cameraPos.y << " Z: " << cameraPos.z << std::endl;
            }
            if (mKeyPressed[GLFW_KEY_S]) {
                // Move backward using unit direction vector
                game.characterPosition -= cameraTarget.normalize() * movementSpeed;
                // cameraPos -= cameraTarget.normalize() * movementSpeed;
                //std::cout << "X: " << cameraPos.x << " Y: " << cameraPos.y << " Z: " << cameraPos.z << std::endl;
            }
            if (mKeyPressed[GLFW_KEY_A]) {
                // cameraPos -= normalize(cross(cameraTarget, cameraUp)) * movementSpeed;
                //game.characterPosition -= normalize(cross(cameraTarget, cameraUp)) * movementSpeed;
                game.characterRoll -= 1.f;
            }
            if (mKeyPressed[GLFW_KEY_D]) {
                //game.characterPosition += normalize(cross(cameraTarget, cameraUp)) * movementSpeed;
                // cameraPos += normalize(cross(cameraTarget, cameraUp)) * movementSpeed;
                game.characterRoll += 1.f;
            }
            if(!mKeyPressed[GLFW_KEY_A] && !mKeyPressed[GLFW_KEY_D]){
                game.characterRoll /= 1.25f;
            }
            if (mKeyPressed[GLFW_KEY_T]) {
                game.turboModeOn = true;
                // cameraPos += normalize(cross(cameraTarget, cameraUp)) * movementSpeed;
            }
			if (mKeyPressed[GLFW_KEY_N]) {
				game.turboModeOn = false;
				// cameraPos += normalize(cross(cameraTarget, cameraUp)) * movementSpeed;
			}
			if (mKeyPressed[GLFW_KEY_1]) {
				game.cameraFirstPerson = true;
			}
			if (mKeyPressed[GLFW_KEY_3]) {
				game.cameraFirstPerson = false;
			}
            //std::cout << "Camera pos x: " << cameraPos.x << cameraPos.y << cameraPos.z << std::endl;
        }


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
        
		// Roll spacecraft on yaw movement to mimic banked turn
		game.characterRoll += x_off * game.characterRollSensitivity;

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
		bool cameraFirstPerson = true;
        
        Vector3f characterPosition;
        float characterScalingFactor;
		float characterRoll;
		float characterRollSensitivity;
        Matrix4f characterViewMatrix;
        Matrix4f projMatrix;
        bool characterIsNotExploded = true;
        
        Vector3f hangarPosition;
        float hangarScalingFactor;
        
        bool turboModeOn;
		bool gameStart;
    } game;
    
    // Computed in a square from 0 to 1
    // Use world coordinates to compare it to objects
    struct Map {
        int resolution = 100;
        float perlinSize = 2.f;
        Vector3f center;
        Model model;
        float heightMult = 5.f;
        float scale = 200.f;
        noise::module::Perlin perlinGenerator;
    };
    
    Map map;
    Map ocean;

	struct Planet {
		Vector3f position;
		float rotationAngle;
	};
    
    struct Obstacle {
        Vector3f position;
        float scaling;
        Vector3f rotation;
        Model model;
        bool crossed = false;
    };
    
    std::vector<Obstacle> obstacles;
    
    struct Animation{
        clock_t startTime = clock();
        bool on = false;
        int firstFrame = 1;
        int currentFrame = 1;
        int numFrames = 10;
        std::vector<Model> frames;
    };
    
    Animation explosion;
    
    
    
    // Light
    struct Light {
        Vector3f position;
        Matrix4f viewMatrix;
        Matrix4f projectionMatrix;
        Vector3f ambientColor;
        Vector3f diffuseColor;
        Vector3f specularColor;
        float power;
    } light;

    // Shader for default rendering and for depth rendering
    ShaderProgram defaultShader;
    ShaderProgram testShader;
    ShaderProgram shadowShader;
    ShaderProgram skySphereShader;

    // Projection and view matrices for you to fill in and use
    Matrix4f projMatrix;
    Matrix4f viewMatrix;
	Model model;
	float rotateAngle = 0.f;

	// Key and mouse input variables and parameters
	std::map<int, bool> mKeyPressed;
	float movementSpeed = 0.15f;
	float mouseSensitivity = 0.1f;
	bool mouseCaptured = false;
	float last_x, last_y;
	float pitch, yaw = 45.f;
    
    // Terrain
    
    Model sphere;
    Model dragon;
    Model davidHead;
	Model earth;
	Model mars;
	Model pinkplanet;
	Model hangar;
	Model spacecraft;
	Model skybox;
	Model skyboxBH;
	Model starSkybox;

	Planet pEarth;
	Planet pMars;
	Planet pTest;

	Image earth_texture;
	Image mars_texture;
	Image pink_texture;
	Image hangar_roof;
	Image skybox_texture;
	Image skybox_BH_texture;
	Image starsky_texture;
    
    GLint m_viewport[4];
    
    // Testing models
    Model cube1;
    Model cube2;
    Model cubeNormals;
    Model testingQuad;
    Image testingQuad_tex;
    Model lightCube;
    Model arcTest;
    
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
