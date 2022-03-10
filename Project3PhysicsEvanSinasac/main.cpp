//Evan Sinasac - 1081418
//INFO6019 Physics and Simulation Project 3
//main.cpp description:
//					The main purpose of Project 3 is to learn and implement collision detection between particles and planes
//					as well as between two particles.  Most of the collision resolution was done in class with James Lucas
//					and I have built off of it to add "contact generators" for particle2plane and particle2particle collisions

#include <physics/cParticleWorld.h>
#include <physics/random_helpers.h>
#include <physics/particle_contact_generators.h>

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <fstream>
#include <algorithm>

#include "GLCommon.h"
#include "GLMCommon.h"
#include "globalThings.h"


//Globals;
float lastX = 600.f;
float lastY = 320.f;
bool firstMouse = true;
float cameraYaw = 90.f;
float cameraPitch = 0.f;

double deltaTime = 0.0;
double lastFrame = 0.0;


std::vector<std::string> modelLocations;

//Function signature for DrawObject()
void DrawObject(
    cMesh* pCurrentMesh,
    glm::mat4 matModel,
    GLint matModel_Location,
    GLint matModelInverseTranspose_Location,
    GLuint program,
    cVAOManager* pVAOManager);


//callbacks
static void mouse_callback(GLFWwindow* window, double xpos, double ypos);

// Decided not to use loadWorldFile for Project 3 but I didn't want to remove it completely just in case
//bool loadWorldFile();


int main(int argc, char** argv)
{
	GLFWwindow* pWindow;

	GLuint program = 0;		//0 means no shader program

	GLint mvp_location = -1;
	std::stringstream ss;

	// *******************************************
	// Physics Project 3 stuff
	//std::vector<nPhysics::cParticle*> projectiles;
	//nPhysics::cParticleWorld* world = new nPhysics::cParticleWorld(50);

	nPhysics::cPlaneParticleContactGenerator* groundGenerator = new nPhysics::cPlaneParticleContactGenerator(glm::vec3(0.0f, 1.0f, 0.0f), 2.0f);
	::g_world->AddContactGenerator(groundGenerator);

	//nPhysics::cPlaneParticleContactGenerator ceilWallGenerator(glm::vec3(0.0f, -1.0f, 0.0f), -5.0f);
	//::g_world->AddContactGenerator(&ceilWallGenerator);

	// For some reason the distance needs to be negative for the "walls" of the box, but positive works for the "ground"
	nPhysics::cPlaneParticleContactGenerator* leftWallGenerator = new nPhysics::cPlaneParticleContactGenerator(glm::vec3(-1.0f, 0.0f, 0.0f), -5.0f);
	::g_world->AddContactGenerator(leftWallGenerator);

	nPhysics::cPlaneParticleContactGenerator* rightWallGenerator = new nPhysics::cPlaneParticleContactGenerator(glm::vec3(1.0f, 0.0f, 0.0f), -5.0f);
	::g_world->AddContactGenerator(rightWallGenerator);

	nPhysics::cPlaneParticleContactGenerator* backWallGenerator = new nPhysics::cPlaneParticleContactGenerator(glm::vec3(0.0f, 0.0f, -1.0f), -5.0f);
	::g_world->AddContactGenerator(backWallGenerator);

	nPhysics::cPlaneParticleContactGenerator* frontWallGenerator = new nPhysics::cPlaneParticleContactGenerator(glm::vec3(0.0f, 0.0f, 1.0f), -5.0f);
	::g_world->AddContactGenerator(frontWallGenerator);

	// And the particle collision detection
	nPhysics::cParticle2ParticleContactGenerator* particleCollider = new nPhysics::cParticle2ParticleContactGenerator();
	::g_world->AddContactGenerator(particleCollider);

	// *******************************************

	glfwSetErrorCallback(GLFW_error_callback);

	if (!glfwInit())
	{
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	pWindow = glfwCreateWindow(1200, 640, "Physics Project 3", NULL, NULL);

	if (!pWindow)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwSetKeyCallback(pWindow, GLFW_key_callback);

	glfwSetCursorEnterCallback(pWindow, GLFW_cursor_enter_callback);
	glfwSetCursorPosCallback(pWindow, GLFW_cursor_position_callback);
	glfwSetScrollCallback(pWindow, GLFW_scroll_callback);
	glfwSetMouseButtonCallback(pWindow, GLFW_mouse_button_callback);
	glfwSetWindowSizeCallback(pWindow, GLFW_window_size_callback);


	glfwSetCursorPosCallback(pWindow, mouse_callback);

	glfwSetInputMode(pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwMakeContextCurrent(pWindow);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	glfwSwapInterval(1);


	GLint max_uniform_location = 0;
	GLint* p_max_uniform_location = NULL;
	p_max_uniform_location = &max_uniform_location;
	glGetIntegerv(GL_MAX_UNIFORM_LOCATIONS, p_max_uniform_location);

	std::cout << "GL_MAX_UNIFORM_LOCATIONS: " << max_uniform_location << std::endl;

	// Create global things
	::g_StartUp(pWindow);

	/*::g_pFlyCamera->setEye(glm::vec3(0.0f, 0.0f, 20.0f));
	std::cout << "Fly Camera At: " << ::g_pFlyCamera->getAt().x << " " << ::g_pFlyCamera->getAt().y << " " << ::g_pFlyCamera->getAt().z << std::endl;*/

	// Shaders with textures and lights, probably going to disable lights unless it looks OK
	cShaderManager::cShader vertShader;
	ss.str("");
	ss << SOLUTION_DIR << "common\\assets\\shaders\\vertShader_01.glsl";
	vertShader.fileName = ss.str();

	cShaderManager::cShader fragShader;
	ss.str("");
	ss << SOLUTION_DIR << "common\\assets\\shaders\\fragShader_01.glsl";
	fragShader.fileName = ss.str();

	if (::g_pShaderManager->createProgramFromFile("Shader#1", vertShader, fragShader))
	{
		std::cout << "Shader compiled OK" << std::endl;
		// 
		// Set the "program" variable to the one the Shader Manager used...
		program = ::g_pShaderManager->getIDFromFriendlyName("Shader#1");
	}
	else
	{
		std::cout << "Error making shader program: " << std::endl;
		std::cout << ::g_pShaderManager->getLastError() << std::endl;
	}


	// Select the shader program we want to use
	// (Note we only have one shader program at this point)
	glUseProgram(program);

	// *******************************************************
	// Now, I'm going to "load up" all the uniform locations
	// (This was to show how a map could be used)
	// Stuff we've started doing in graphics to make things faster 
	cShaderManager::cShaderProgram* pShaderProc = ::g_pShaderManager->pGetShaderProgramFromFriendlyName("Shader#1");
	int theUniformIDLoc = -1;
	theUniformIDLoc = glGetUniformLocation(program, "matModel");

	pShaderProc->mapUniformName_to_UniformLocation["matModel"] = theUniformIDLoc;

	// Or...
	pShaderProc->mapUniformName_to_UniformLocation["matModel"] = glGetUniformLocation(program, "matModel");

	pShaderProc->mapUniformName_to_UniformLocation["matView"] = glGetUniformLocation(program, "matView");
	pShaderProc->mapUniformName_to_UniformLocation["matProjection"] = glGetUniformLocation(program, "matProjection");
	pShaderProc->mapUniformName_to_UniformLocation["matModelInverseTranspose"] = glGetUniformLocation(program, "matModelInverseTranspose");

	// *******************************************************

	//GLint mvp_location = -1;
	mvp_location = glGetUniformLocation(program, "MVP");

	// Get "uniform locations" (aka the registers these are in)
	GLint matModel_Location = glGetUniformLocation(program, "matModel");
	//	GLint matView_Location = glGetUniformLocation(program, "matView");
	GLint matProjection_Location = glGetUniformLocation(program, "matProjection");
	GLint matModelInverseTranspose_Location = glGetUniformLocation(program, "matModelInverseTranspose");


	//Lights stuff here

	//    	vec4 param1;	// x = lightType, y = inner angle, z = outer angle, w = TBD
//	                // 0 = pointlight
//					// 1 = spot light
//					// 2 = directional light
	::g_pTheLights->theLights[0].position = glm::vec4(0.0f, 110.0f, 0.0f, 1.0f);
	//::g_pTheLights->theLights[0].diffuse = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	//::g_pTheLights->theLights[0].specular = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	::g_pTheLights->theLights[0].param1.x = 0.0f;	// Pointlight
	//::g_pTheLights->theLights[0].param1.y = 10.0f;   // Inner
	//::g_pTheLights->theLights[0].param1.z = 12.0f;   // Outer
	//::g_pTheLights->theLights[0].direction = glm::vec4(1.0f, -0.1f, 0.0f, 1.0f);
	::g_pTheLights->theLights[0].atten.x = 1.0f;
	::g_pTheLights->theLights[0].atten.y = 0.0000000000001f;
	::g_pTheLights->theLights[0].atten.z = 0.00000000000001f;
	::g_pTheLights->TurnOnLight(0);

	// Get the uniform locations of the light shader values
	::g_pTheLights->SetUpUniformLocations(program);

	ss.str("");
	ss << SOLUTION_DIR << "common\\assets\\models\\";

	::g_pVAOManager->setFilePath(ss.str());

	// Load ALL the models
	modelLocations.push_back("SM_Env_Floor_01_xyz_n_rgba_uv.ply");
	modelLocations.push_back("SM_Env_Floor_04_xyz_n_rgba_uv.ply");
	modelLocations.push_back("Isosphere_Smooth_Normals.ply");
	modelLocations.push_back("ISO_Sphere_flat_4div_xyz_n_rgba_uv.ply");
	modelLocations.push_back("Turret_Simple_01_xyz_n_rgba_uv.ply");
	modelLocations.push_back("SM_Env_Construction_Wall_01_xyz_n_rgba_uv.ply");

	unsigned int totalVerticesLoaded = 0;
	unsigned int totalTrianglesLoaded = 0;
	for (std::vector<std::string>::iterator itModel = modelLocations.begin(); itModel != modelLocations.end(); itModel++)
	{
		sModelDrawInfo theModel;
		std::string modelName = *itModel;
		std::cout << "Loading " << modelName << "...";
		if (!::g_pVAOManager->LoadModelIntoVAO(modelName, theModel, program))
		{
			std::cout << "didn't work because: " << std::endl;
			std::cout << ::g_pVAOManager->getLastError(true) << std::endl;
		}
		else
		{
			std::cout << "OK." << std::endl;
			std::cout << "\t" << theModel.numberOfVertices << " vertices and " << theModel.numberOfTriangles << " triangles loaded." << std::endl;
			totalTrianglesLoaded += theModel.numberOfTriangles;
			totalVerticesLoaded += theModel.numberOfVertices;
		}
	} //end of for (std::vector<std::string>::iterator itModel

	std::cout << "Done loading models." << std::endl;
	std::cout << "Total vertices loaded = " << totalVerticesLoaded << std::endl;
	std::cout << "Total triangles loaded = " << totalTrianglesLoaded << std::endl;

	// Make static meshes OR start loading the world file
	cMesh* cannonFloorMesh = new cMesh();
	cannonFloorMesh->meshName = "SM_Env_Floor_01_xyz_n_rgba_uv.ply";
	cannonFloorMesh->positionXYZ = glm::vec3(2.5f, 0.0f, -17.5f);
	cannonFloorMesh->orientationXYZ = glm::vec3(0.0f, 0.0f, 0.0f);
	cannonFloorMesh->scale = 1.0f;
	cannonFloorMesh->bUseWholeObjectDiffuseColour = false;
	cannonFloorMesh->wholeObjectDiffuseRGBA = glm::vec4(0.88f, 0.88f, 0.88f, 1.0f);
	cannonFloorMesh->alphaTransparency = 1.0f;
	cannonFloorMesh->bDontLight = false;

	cannonFloorMesh->clearTextureRatiosToZero();
	cannonFloorMesh->textureNames[1] = "SpaceInteriors_Texture.bmp";
	cannonFloorMesh->textureRatios[1] = 1.0f;

	::g_vec_pMeshes.push_back(cannonFloorMesh);


	cMesh* turretMesh = new cMesh();
	turretMesh->meshName = "Turret_Simple_01_xyz_n_rgba_uv.ply";
	turretMesh->positionXYZ = glm::vec3(0.0f, 1.0f, -15.0f);
	turretMesh->orientationXYZ = glm::vec3(::g_cannonPitch, ::g_cannonYaw, 0.0f);
	turretMesh->scale = 0.2f;
	turretMesh->bUseWholeObjectDiffuseColour = false;
	turretMesh->wholeObjectDiffuseRGBA = glm::vec4(0.88f, 0.88f, 0.88f, 1.0f);
	turretMesh->alphaTransparency = 1.0f;
	turretMesh->bDontLight = true;


	// If I want to use these models, they use multiple textures and put which texture to use at the vertex
	// Which means I'd be using different model formats and I'd either have to use only stuff from the creepy cat models I have and re-write the shaders
	// Or it'll look a little messy but match what I have for the shaders right now
	turretMesh->clearTextureRatiosToZero();
	turretMesh->textureNames[0] = "Wall_Atlas_01_ID.bmp";
	turretMesh->textureRatios[0] = 0.7f;
	turretMesh->textureNames[1] = "Wall_Atlas_03_ID.bmp";
	turretMesh->textureRatios[1] = 0.1f;
	turretMesh->textureNames[2] = "Wall_Atlas_08_ID.bmp";
	turretMesh->textureRatios[2] = 0.1f;
	turretMesh->textureNames[3] = "Wall_Atlas_11_ID.bmp";
	turretMesh->textureRatios[3] = 0.1f;

	::g_vec_pMeshes.push_back(turretMesh);


	// 5 models for "box", + 2 models for turret and base, offset of 7
	cMesh* pBoxFloor = new cMesh();
	pBoxFloor->meshName = "SM_Env_Floor_01_xyz_n_rgba_uv.ply";
	pBoxFloor->positionXYZ = glm::vec3(5.0f, 2.0f, -5.0f);
	pBoxFloor->orientationXYZ = glm::vec3(0.0f, 0.0f, 0.0f);
	pBoxFloor->scale = 2.0f;
	//pBoxFloor->bUseWholeObjectDiffuseColour = false;
	//pBoxFloor->wholeObjectDiffuseRGBA = glm::vec4(0.88f, 0.88f, 0.88f, 1.0f);
	pBoxFloor->alphaTransparency = 0.5f;
	pBoxFloor->bDontLight = true;
	pBoxFloor->clearTextureRatiosToZero();
	pBoxFloor->textureNames[1] = "SpaceInteriors_Texture.bmp";
	pBoxFloor->textureRatios[1] = 1.0f;
	::g_vec_pMeshes.push_back(pBoxFloor);

	cMesh* pBoxLeftWall = new cMesh();
	pBoxLeftWall->meshName = "SM_Env_Construction_Wall_01_xyz_n_rgba_uv.ply";
	pBoxLeftWall->positionXYZ = glm::vec3(5.0f, 2.0f, 0.0f);
	pBoxLeftWall->orientationXYZ = glm::vec3(0.0f, glm::radians(90.0f), 0.0f);
	pBoxLeftWall->scale = 0.5f;
	pBoxLeftWall->alphaTransparency = 1.0f;
	pBoxLeftWall->bDontLight = true;
	pBoxLeftWall->clearTextureRatiosToZero();
	pBoxLeftWall->textureNames[1] = "SpaceInteriors_Texture.bmp";
	pBoxLeftWall->textureRatios[1] = 1.0f;
	::g_vec_pMeshes.push_back(pBoxLeftWall);

	cMesh* pBoxRightWall = new cMesh();
	pBoxRightWall->meshName = "SM_Env_Construction_Wall_01_xyz_n_rgba_uv.ply";
	pBoxRightWall->positionXYZ = glm::vec3(-5.0f, 2.0f, 0.0f);
	pBoxRightWall->orientationXYZ = glm::vec3(0.0f, glm::radians(270.0f), 0.0f);
	pBoxRightWall->scale = 0.5f;
	pBoxRightWall->alphaTransparency = 1.0f;
	pBoxRightWall->bDontLight = true;
	pBoxRightWall->clearTextureRatiosToZero();
	pBoxRightWall->textureNames[1] = "SpaceInteriors_Texture.bmp";
	pBoxRightWall->textureRatios[1] = 1.0f;
	::g_vec_pMeshes.push_back(pBoxRightWall);

	cMesh* pBoxFrontWall = new cMesh();
	pBoxFrontWall->meshName = "SM_Env_Construction_Wall_01_xyz_n_rgba_uv.ply";
	pBoxFrontWall->positionXYZ = glm::vec3(0.0f, 2.0f, -5.0f);
	pBoxFrontWall->orientationXYZ = glm::vec3(0.0f, glm::radians(0.0f), 0.0f);
	pBoxFrontWall->scale = 0.5f;
	pBoxFrontWall->alphaTransparency = 1.0f;
	pBoxFrontWall->bDontLight = true;
	pBoxFrontWall->clearTextureRatiosToZero();
	pBoxFrontWall->textureNames[1] = "SpaceInteriors_Texture.bmp";
	pBoxFrontWall->textureRatios[1] = 1.0f;
	::g_vec_pMeshes.push_back(pBoxFrontWall);

	cMesh* pBoxBackWall = new cMesh();
	pBoxBackWall->meshName = "SM_Env_Construction_Wall_01_xyz_n_rgba_uv.ply";
	pBoxBackWall->positionXYZ = glm::vec3(0.0f, 2.0f, 5.0f);
	pBoxBackWall->orientationXYZ = glm::vec3(0.0f, glm::radians(180.0f), 0.0f);
	pBoxBackWall->scale = 0.5f;
	pBoxBackWall->alphaTransparency = 1.0f;
	pBoxBackWall->bDontLight = true;
	pBoxBackWall->clearTextureRatiosToZero();
	pBoxBackWall->textureNames[1] = "SpaceInteriors_Texture.bmp";
	pBoxBackWall->textureRatios[1] = 1.0f;
	::g_vec_pMeshes.push_back(pBoxBackWall);

	// Decided to not do world file since I need to use different textures so rather than redo the loadWorld, just not using it for this
	//World file stuff here
	//if (loadWorldFile())
	//{
	//	std::cout << "loadWorldFile finished OK" << std::endl;
	//}
	//else
	//{
	//	std::cout << "loadWorldFile did not finish OK, aborting" << std::endl;
	//	return -1;
	//}


	// Transparent objects get made last (and sorted before being drawn)


	//Michael Feeney's BMP texture mapping
	//// Load the textures
	ss.str("");
	ss << SOLUTION_DIR << "common\\assets\\textures\\";
	::g_pTextureManager->SetBasePath(ss.str());

	if (::g_pTextureManager->Create2DTextureFromBMPFile("SpaceInteriors_Texture.bmp", true))
		//if (::g_pTextureManager->Create2DTextureFromBMPFile("Pebbleswithquarzite.bmp", true))
	{
		std::cout << "Loaded the texture" << std::endl;
	}
	else
	{
		std::cout << "DIDN'T load the texture" << std::endl;
	}

	::g_pTextureManager->Create2DTextureFromBMPFile("SpaceInteriors_Emmision.bmp", true);
	::g_pTextureManager->Create2DTextureFromBMPFile("WorldMap.bmp", true);
	::g_pTextureManager->Create2DTextureFromBMPFile("2k_jupiter.bmp", true);

	::g_pTextureManager->Create2DTextureFromBMPFile("Wall_Atlas_01_ID.bmp", true);
	::g_pTextureManager->Create2DTextureFromBMPFile("Wall_Atlas_03_ID.bmp", true);
	::g_pTextureManager->Create2DTextureFromBMPFile("Wall_Atlas_08_ID.bmp", true);
	::g_pTextureManager->Create2DTextureFromBMPFile("Wall_Atlas_11_ID.bmp", true);

	// Default don't have a texture
	if (::g_pTextureManager->Create2DTextureFromBMPFile("BrightColouredUVMap.bmp", true))
		//if (::g_pTextureManager->Create2DTextureFromBMPFile("Pebbleswithquarzite.bmp", true))
	{
		std::cout << "Loaded the texture" << std::endl;
	}
	else
	{
		std::cout << "DIDN'T load the texture" << std::endl;
	}


	// Face camera towards origin
	if (::cameraEye.x > 0 && ::cameraEye.z > 0)
	{
		::cameraYaw = 180.f + (atan(::cameraEye.z / ::cameraEye.x) * 180.f / glm::pi<float>());
	}
	else if (::cameraEye.x > 0 && ::cameraEye.z < 0)
	{
		::cameraYaw = 90.f - (atan(::cameraEye.z / ::cameraEye.x) * 180.f / glm::pi<float>());
	}
	else if (::cameraEye.x < 0 && ::cameraEye.z > 0)
	{
		::cameraYaw = (atan(::cameraEye.z / ::cameraEye.x) * 180.f / glm::pi<float>());
	}
	else if (::cameraEye.x < 0 && ::cameraEye.z < 0)
	{
		::cameraYaw = (atan(::cameraEye.z / ::cameraEye.x) * 180.f / glm::pi<float>());
	}
	else if (::cameraEye.x == 0.f)
	{
		if (::cameraEye.z >= 0.f)
		{
			::cameraYaw = 270.f;
		}
		else
		{
			::cameraYaw = 90.f;
		}
	}
	else if (::cameraEye.z == 0.f)
	{
		if (::cameraEye.x <= 0)
		{
			::cameraYaw = 0.f;
		}
		else
		{
			::cameraYaw = 180.f;
		}
	}
	//anyways, after figuring out the yaw, we set the target at the negative of the xz of the ::camera position and y=0 (this faces the ::camera towards the origin)
	::cameraTarget = glm::vec3(-1.f * ::cameraEye.x, 0, -1.f * ::cameraEye.z);
	glm::normalize(::cameraTarget);

	const double MAX_DELTA_TIME = 0.1;	//100 ms
	double previousTime = glfwGetTime();

	while (!glfwWindowShouldClose(pWindow))
	{
		float ratio;
		int width, height;
		glm::mat4 matModel;				// used to be "m"; Sometimes it's called "world"
		glm::mat4 matProjection;        // used to be "p";
		glm::mat4 matView;              // used to be "v";

		double currentTime = glfwGetTime();
		deltaTime = currentTime - previousTime;
		deltaTime = (deltaTime > MAX_DELTA_TIME ? MAX_DELTA_TIME : deltaTime);
		previousTime = currentTime;

		glfwGetFramebufferSize(pWindow, &width, &height);
		ratio = width / (float)height;


		// ***************************************************
		// Physics
		::g_world->TimeStep((float)deltaTime);

		for (unsigned int index = 0; index < ::g_vec_pProjectiles.size(); index++)
		{
			// TODO: For some reason, whenever the projectile hits the sides of the "box" the position gets super fucked up.
			// Occasionnally happens when it hits the ground, but not usually
			// HACK fix before I implemented negative half space check, seems fine now that that's implemented
			/*if (::g_vec_pProjectiles[index]->GetPosition().x > 1000.0f || ::g_vec_pProjectiles[index]->GetPosition().x < -1000.0f ||
				::g_vec_pProjectiles[index]->GetPosition().y > 1000.0f || ::g_vec_pProjectiles[index]->GetPosition().y < -1000.0f ||
				::g_vec_pProjectiles[index]->GetPosition().z > 1000.0f || ::g_vec_pProjectiles[index]->GetPosition().z < -1000.0f)
			{
				std::cout << "WHAT" << std::endl;
				::g_vec_pProjectiles[index]->SetPosition(::g_vec_pMeshes[index + 7]->positionXYZ);
			}*/


			// Have 2 models (floor and cannon) so when we're adding particles we have to compare with a buffer
			// Correction, there is a total of 7 models before launching particles, so offset = 7 (probably should've made a variable I increment, but good enough for rock n roll [I'm absorbing Feeney's lingo lol])
			::g_vec_pMeshes[index + 7]->positionXYZ = ::g_vec_pProjectiles[index]->GetPosition();

			// For this project the projectiles don't die but I'm keeping this in just in case, removed the kill below 0 in the cParticle Integrate
			if (!g_vec_pProjectiles[index]->GetIsAlive())
			{
				::g_world->RemoveParticle(::g_vec_pProjectiles[index]);
				::g_vec_pProjectiles.erase(::g_vec_pProjectiles.begin() + index);
				::g_vec_pMeshes.erase(g_vec_pMeshes.begin() + index + 7);
				index--;
			}
		} //end of for


		// ***************************************************


		//Turn on the depth buffer
		glEnable(GL_DEPTH);         // Turns on the depth buffer
		glEnable(GL_DEPTH_TEST);    // Check if the pixel is already closer

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// *******************************************************
		// Screen is cleared and we are ready to draw the scene...
		// *******************************************************

		// Update the title text
		glfwSetWindowTitle(pWindow, ::g_TitleText.c_str());


		// Move the skybox "object" to match the camera
		//sphereSky->positionXYZ = ::cameraEye;

		// Copy the light information into the shader to draw the scene
		::g_pTheLights->CopyLightInfoToShader();

		/*matProjection = glm::perspective(0.6f,
			ratio,
			0.1f,
			1000.0f);*/

		matProjection = glm::perspective(0.6f,	// FOV variable later
			ratio,
			0.1f,								// Near plane
			1'000'000.0f);						// Far plane



		//matProjection = glm::perspective(
		//	::g_pFlyCamera->FOV,
		//	ratio,
		//	::g_pFlyCamera->nearPlane,      // Near plane (as large as possible)
		//	::g_pFlyCamera->farPlane);      // Far plane (as small as possible)


		//::g_pFlyCamera->Update(deltaTime);

		//glm::vec3 cameraEye = ::g_pFlyCamera->getEye();
		//glm::vec3 cameraAt = ::g_pFlyCamera->getAtInWorldSpace();
		//glm::vec3 cameraUp = ::g_pFlyCamera->getUpVector();


		matView = glm::mat4(1.0f);
		//matView = glm::lookAt(	cameraEye,   // "eye"
		//						cameraAt,    // "at"
		//						cameraUp);

		matView = glm::lookAt(::cameraEye,
			::cameraEye + ::cameraTarget,
			upVector);

		//	glUniformMatrix4fv(matView_Location, 1, GL_FALSE, glm::value_ptr(matView));

		glUniformMatrix4fv(pShaderProc->getUniformID_From_Name("matView"),
			1, GL_FALSE, glm::value_ptr(matView));


		glUniformMatrix4fv(matProjection_Location, 1, GL_FALSE, glm::value_ptr(matProjection));

		// **********************************************************************
		// Draw the "scene" of all objects.
		// i.e. go through the vector and draw each one...
		// **********************************************************************
		for (unsigned int index = 0; index != ::g_vec_pMeshes.size(); index++)
		{
			// So the code is a little easier...
			cMesh* pCurrentMesh = ::g_vec_pMeshes[index];

			matModel = glm::mat4(1.0f);  // "Identity" ("do nothing", like x1)
			//mat4x4_identity(m);

			DrawObject(pCurrentMesh,
				matModel,
				matModel_Location,
				matModelInverseTranspose_Location,
				program,
				::g_pVAOManager);


		}//for (unsigned int index
		// Scene is drawn
		// **********************************************************************

		// "Present" what we've drawn.
		glfwSwapBuffers(pWindow);        // Show what we've drawn

		// Process any events that have happened
		glfwPollEvents();

		// Handle OUR keyboard, mouse stuff
		handleAsyncKeyboard(pWindow, deltaTime);
		handleAsyncMouse(pWindow, deltaTime);
	} //while (!glfwWindowShouldClose(window))

	// All done, so delete things...
	::g_ShutDown(pWindow);


	glfwDestroyWindow(pWindow);

	glfwTerminate();
	exit(EXIT_SUCCESS);

} //end of main

//
//bool loadWorldFile()
//{
//	std::stringstream ss;
//	std::stringstream sFile;
//
//
//	ss << SOLUTION_DIR << "common\\assets\\worldFile.txt";
//
//	std::ifstream theFile(ss.str());
//
//	if (!theFile.is_open())
//	{
//		fprintf(stderr, "Could not open modelsToLoad.txr");
//		return false;
//	}
//
//	std::string nextToken;
//	ss.str("");
//
//	//Throwaway text describing the format of the file
//	theFile >> nextToken;       //ModelFileName(extension)
//	theFile >> nextToken;       //Position(x,y,z)
//	theFile >> nextToken;       //Orientation(x,y,z)
//	theFile >> nextToken;       //Scale
//	theFile >> nextToken;       //Colour(r,g,b)
//
//	//From here modify based on worldFile format
//	while (theFile >> nextToken)    //this should always be the name of the model to load or end.  Potential error check, add a check for "ply" in the mdoel name
//	{
//		cMesh* curMesh = new cMesh;
//		if (nextToken == "end")
//		{
//			break;
//		}
//		std::cout << nextToken << std::endl;        //Printing model names to console, just making sure we're loading ok.  Can be commented out whenever
//		//First is the file name of model
//		//ss << SOLUTION_DIR << "common\\assets\\models\\" << nextToken;
//		curMesh->meshName = nextToken;
//		//Next 3 are the position of the model
//		theFile >> nextToken;                                               //x position for the model
//		curMesh->positionXYZ.x = std::stof(nextToken);
//		theFile >> nextToken;                                               //y position for the model
//		curMesh->positionXYZ.y = std::stof(nextToken);
//		theFile >> nextToken;                                               //z position for the model
//		curMesh->positionXYZ.z = std::stof(nextToken);
//		//Next 3 are the orientation of the model
//		theFile >> nextToken;                                               //x orientation value
//		//curMesh.orientationXYZ.x = std::stof(nextToken);
//		curMesh->orientationXYZ.x = glm::radians(std::stof(nextToken));
//		theFile >> nextToken;                                               //y orientation value
//		//curMesh.orientationXYZ.y = std::stof(nextToken);
//		curMesh->orientationXYZ.y = glm::radians(std::stof(nextToken));
//		theFile >> nextToken;                                               //z orientation value
//		//curMesh.orientationXYZ.z = std::stof(nextToken);
//		curMesh->orientationXYZ.z = glm::radians(std::stof(nextToken));
//		//Next is the scale to multiply the model by
//		theFile >> nextToken;                                               //scale multiplier
//		curMesh->scale = std::stof(nextToken);
//		//Next 3 are the r, g, b values for the model
//		curMesh->bUseWholeObjectDiffuseColour = false;	//true for now for models without textures
//		theFile >> nextToken;													//RGB red value
//		curMesh->wholeObjectDiffuseRGBA.r = std::stof(nextToken) / 255.0f;		//convert to nice shader value (between 0 and 1)
//		theFile >> nextToken;													//RGB green value
//		curMesh->wholeObjectDiffuseRGBA.g = std::stof(nextToken) / 255.0f;		//convert to nice shader value (between 0 and 1)
//		theFile >> nextToken;													//RGB blue value
//		curMesh->wholeObjectDiffuseRGBA.b = std::stof(nextToken) / 255.0f;		//convert to nice shader value (between 0 and 1)
//		theFile >> nextToken;
//		curMesh->wholeObjectDiffuseRGBA.a = std::stof(nextToken);
//		curMesh->alphaTransparency = std::stof(nextToken);
//
//		curMesh->bDontLight = true;		// true to test texture stuff cause the lighting is still gawd aweful lmao
//
//		curMesh->clearTextureRatiosToZero();
//		curMesh->textureRatios[1] = 1.0f;
//		curMesh->textureNames[1] = "SpaceInteriors_Texture.bmp";
//
//		::g_vec_pMeshes.push_back(curMesh);     //push the model onto our vector of meshes
//		ss.str("");                         //reset the stringstream
//	} //end of while
//	theFile.close();
//	return true;
//}	//end of load world file


//Figured out the math for how to do this from https://learnopengl.com/Getting-started/Camera and http://www.opengl-tutorial.org/beginners-tutorials/tutorial-6-keyboard-and-mouse/
//Using the mouse position we calculate the direction that the camera will be facing
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	//if it's the start of the program this smooths out a potentially glitchy jump
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}
	//find the offset of where the mouse positions have moved
	float xOffset = xpos - lastX;
	float yOffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;
	//multiply by sensitivity so that it's not potentially crazy fast
	float sensitivity = 0.1f;
	xOffset *= sensitivity;
	yOffset *= sensitivity;

	::cameraYaw += xOffset;         // The yaw is the rotation around the ::camera's y-axis (so we want to add the xOffset to it)
	::cameraPitch += yOffset;       // The pitch is the rotation around the ::camera's x-axis (so we want to add the yOffset to it)
	//This limits the pitch so that we can't just spin the ::camera under/over itself
	if (::cameraPitch > 89.0f)
		::cameraPitch = 89.0f;
	if (::cameraPitch < -89.0f)
		::cameraPitch = -89.0f;
	//calculations for the new direction based on the mouse movements
	glm::vec3 direction;
	direction.x = cos(glm::radians(::cameraYaw)) * cos(glm::radians(::cameraPitch));
	direction.y = sin(glm::radians(::cameraPitch));
	direction.z = sin(glm::radians(::cameraYaw)) * cos(glm::radians(::cameraPitch));
	::cameraTarget = glm::normalize(direction);
} //fly ::camera