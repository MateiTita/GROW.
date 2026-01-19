#include "Graphics\window.h"
#include "Camera\camera.h"
#include "Shaders\shader.h"
#include "Model Loading\mesh.h"
#include "Model Loading\texture.h"
#include "Model Loading\meshLoaderObj.h"
#include "ImGUI/imgui.h"
#include "ImGUI/imgui_impl_glfw.h"
#include "ImGUI/imgui_impl_opengl3.h"
#include "Player/Player.h"

void processKeyboardInput();

float deltaTime = 0.0f; // time between current frame and last frame
float lastFrame = 0.0f;
bool lowHealthMode = false;

Window window("Game Engine", 800, 800);
Camera camera;
Player* player;

// Underwater lighting
glm::vec3 lightColor = glm::vec3(0.8f, 0.9f, 1.0f);
glm::vec3 lightPos = glm::vec3(0.0f, 200.0f, 0.0f);

// Spotlight
glm::vec3 spotlightPos = glm::vec3(0.0f, 0.0f, 0.0f);           // Position of spotlight
glm::vec3 spotlightDir = glm::vec3(0.0f, -1.0f, 0.0f);          // Direction (pointing down)
glm::vec3 spotlightColor = glm::vec3(1.0f, 1.0f, 0.8f);         // Warm yellow color
float spotlightCutOff = glm::cos(glm::radians(15.0f));          // Inner cone angle
float spotlightOuterCutOff = glm::cos(glm::radians(25.0f));     // Outer cone angle

// Underwater fog 
glm::vec3 waterColor = glm::vec3(0.0f, 0.3f, 0.5f);  // Deep blue
float fogDensity = 0.004f;  // How fast things fade (lower = see farther)

// Third-person camera offset
glm::vec3 cameraOffset = glm::vec3(0.0f, 10.0f, 30.0f);

int currentTask = 0;
bool pressedW = false;
bool pressedA = false;
bool pressedS = false;
bool pressedD = false;
bool usedDash = false;
const int totalTasks = 2;


int main()
{
	glClearColor(0.0f, 0.2f, 0.4f, 1.0f);

	// building and compiling shader program
	Shader shader("Shaders/vertex_shader.glsl", "Shaders/fragment_shader.glsl");
	Shader sunShader("Shaders/sun_vertex_shader.glsl", "Shaders/sun_fragment_shader.glsl");

	// Textures
	GLuint tex = loadBMP("Resources/Textures/wood.bmp");
	GLuint tex2 = loadBMP("Resources/Textures/rock.bmp");
	GLuint tex3 = loadBMP("Resources/Textures/orange.bmp");
	GLuint texGround = loadBMP("Resources/Textures/Ground.bmp");

	glEnable(GL_DEPTH_TEST);

	// ImGui Setup
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window.getWindow(), true);
	ImGui_ImplOpenGL3_Init("#version 330");

	// Test custom mesh loading
	std::vector<Vertex> vert;
	vert.push_back(Vertex());
	vert[0].pos = glm::vec3(10.5f, 10.5f, 0.0f);
	vert[0].textureCoords = glm::vec2(1.0f, 1.0f);

	vert.push_back(Vertex());
	vert[1].pos = glm::vec3(10.5f, -10.5f, 0.0f);
	vert[1].textureCoords = glm::vec2(1.0f, 0.0f);

	vert.push_back(Vertex());
	vert[2].pos = glm::vec3(-10.5f, -10.5f, 0.0f);
	vert[2].textureCoords = glm::vec2(0.0f, 0.0f);

	vert.push_back(Vertex());
	vert[3].pos = glm::vec3(-10.5f, 10.5f, 0.0f);
	vert[3].textureCoords = glm::vec2(0.0f, 1.0f);

	vert[0].normals = glm::normalize(glm::cross(vert[1].pos - vert[0].pos, vert[3].pos - vert[0].pos));
	vert[1].normals = glm::normalize(glm::cross(vert[2].pos - vert[1].pos, vert[0].pos - vert[1].pos));
	vert[2].normals = glm::normalize(glm::cross(vert[3].pos - vert[2].pos, vert[1].pos - vert[2].pos));
	vert[3].normals = glm::normalize(glm::cross(vert[0].pos - vert[3].pos, vert[2].pos - vert[3].pos));

	std::vector<int> ind = {0, 1, 3,
							1, 2, 3};

	std::vector<Texture> textures;
	textures.push_back(Texture());
	textures[0].id = tex;
	textures[0].type = "texture_diffuse";

	std::vector<Texture> texturesGround;
	texturesGround.push_back(Texture());
	texturesGround[0].id = texGround;
	texturesGround[0].type = "texture_diffuse";

	std::vector<Texture> textures2;
	textures2.push_back(Texture());
	textures2[0].id = tex2;
	textures2[0].type = "texture_diffuse";

	std::vector<Texture> textures3;
	textures3.push_back(Texture());
	textures3[0].id = tex3;
	textures3[0].type = "texture_diffuse";

	Mesh mesh(vert, ind, textures3);

	// Create Obj files - easier :)
	// we can add here our textures :)
	MeshLoaderObj loader;
	Mesh sun = loader.loadObj("Resources/Models/sphere.obj");
	Mesh rock = loader.loadObj("Resources/Models/cube.obj", textures2);   // Rock texture
	Mesh coral = loader.loadObj("Resources/Models/cube.obj", textures3);  // Orange texture
	Mesh plane = loader.loadObj("Resources/Models/plane.obj", texturesGround);
	Mesh fish = loader.loadObj("Resources/Models/fish.obj", textures3);

	player = new Player(&fish, glm::vec3(0.0f, -40.0f, 0.0f));



	// check if we close the window or press the escape button
	while (!window.isPressed(GLFW_KEY_ESCAPE) &&
		   glfwWindowShouldClose(window.getWindow()) == 0)
	{
		window.clear();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// GUI Window
		ImGui::Begin("GROW - Game Settings");

		ImGui::Text("Fog Density: %.3f", fogDensity);

		ImGui::Separator();

		if (ImGui::Button("Increase Fog")) {
			fogDensity += 0.002f;
			if (fogDensity > 0.05f) fogDensity = 0.05f;
		}
		ImGui::SameLine();
		if (ImGui::Button("Decrease Fog")) {
			fogDensity -= 0.002f;
			if (fogDensity < 0.001f) fogDensity = 0.001f;
		}

		if (ImGui::Button("Low Health Mode")) {
			lowHealthMode = !lowHealthMode;
			if (lowHealthMode) {
				waterColor = glm::vec3(0.5f, 0.0f, 0.0f);
			}
			else {
				waterColor = glm::vec3(0.0f, 0.3f, 0.5f);
			}
		}

		if (lowHealthMode) {
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "ACTIVE");
		}

		// Task System Display
		ImGui::Separator();
		ImGui::Text("=== TASKS ===");

		if (currentTask == 0)
		{
			ImGui::Text("Task 1: Learn to swim!");
			ImGui::Text("Press W, A, S, D to move.");
			ImGui::Text("Progress: %s %s %s %s",
				pressedW ? "[W]" : "[ ]",
				pressedA ? "[A]" : "[ ]",
				pressedS ? "[S]" : "[ ]",
				pressedD ? "[D]" : "[ ]");
		}
		else if (currentTask == 1)
		{
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Task 1: COMPLETE!");
			ImGui::Text("Task 2: Use your dash!");
			ImGui::Text("Press SHIFT to dash.");
		}
		else if (currentTask >= totalTasks)
		{
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Task 1: COMPLETE!");
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Task 2: COMPLETE!");
		}

		ImGui::End();

		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processKeyboardInput();


		player->Update(deltaTime);

		glm::vec3 newCameraPos = player->position + cameraOffset;
		camera.setCameraPosition(newCameraPos);
		// Make camera look at player
		glm::vec3 directionToPlayer = player->position - camera.getCameraPosition();
		camera.setViewDirection(directionToPlayer);

		// test mouse input
		if (window.isMousePressed(GLFW_MOUSE_BUTTON_LEFT))
		{
			std::cout << "Pressing mouse button" << std::endl;
		}
		//// Code for the light ////

		sunShader.use();

		glm::mat4 ProjectionMatrix = glm::perspective(90.0f, window.getWidth() * 1.0f / window.getHeight(), 0.1f, 10000.0f);
		glm::mat4 ViewMatrix = glm::lookAt(camera.getCameraPosition(), camera.getCameraPosition() + camera.getCameraViewDirection(), camera.getCameraUp());

		GLuint MatrixID = glGetUniformLocation(sunShader.getId(), "MVP");

		// Test for one Obj loading = light source

		glm::mat4 ModelMatrix = glm::mat4(1.0);
		ModelMatrix = glm::translate(ModelMatrix, lightPos);
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

		sun.draw(sunShader);

		//// End code for the light ////

		shader.use();


		GLuint MatrixID2 = glGetUniformLocation(shader.getId(), "MVP");
		GLuint ModelMatrixID = glGetUniformLocation(shader.getId(), "model");

		
		glUniform3f(glGetUniformLocation(shader.getId(), "lightColor"), lightColor.x, lightColor.y, lightColor.z);
		glUniform3f(glGetUniformLocation(shader.getId(), "lightPos"), lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(glGetUniformLocation(shader.getId(), "viewPos"), camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);
		glUniform3f(glGetUniformLocation(shader.getId(), "waterColor"), waterColor.x, waterColor.y, waterColor.z);
		glUniform1f(glGetUniformLocation(shader.getId(), "fogDensity"), fogDensity);

		glUniform3f(glGetUniformLocation(shader.getId(), "spotlightPos"), spotlightPos.x, spotlightPos.y, spotlightPos.z);
		glUniform3f(glGetUniformLocation(shader.getId(), "spotlightDir"), spotlightDir.x, spotlightDir.y, spotlightDir.z);
		glUniform3f(glGetUniformLocation(shader.getId(), "spotlightColor"), spotlightColor.x, spotlightColor.y, spotlightColor.z);
		glUniform1f(glGetUniformLocation(shader.getId(), "spotlightCutOff"), spotlightCutOff);
		glUniform1f(glGetUniformLocation(shader.getId(), "spotlightOuterCutOff"), spotlightOuterCutOff);

		// UNDERWATER SCENE OBJECTS

		//SEAFLOOR 
		ModelMatrix = glm::mat4(1.0);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, -50.0f, 0.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(20.0f, 1.0f, 20.0f));
		MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
		glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		plane.draw(shader);

		// ROCK 1 
		ModelMatrix = glm::mat4(1.0);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-30.0f, -45.0f, -20.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(8.0f, 5.0f, 8.0f));
		MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
		glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		rock.draw(shader);

		//  ROCK 2 
		ModelMatrix = glm::mat4(1.0);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(40.0f, -47.0f, 30.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(6.0f, 4.0f, 7.0f));
		MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
		glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		rock.draw(shader);

		// ROCK 3
		ModelMatrix = glm::mat4(1.0);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(15.0f, -48.0f, -50.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(5.0f, 3.0f, 5.0f));
		MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
		glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		rock.draw(shader);

		//  CORAL 1 (tall) 
		ModelMatrix = glm::mat4(1.0);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-20.0f, -35.0f, -40.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(3.0f, 15.0f, 3.0f));
		MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
		glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		coral.draw(shader);

		//  CORAL 2 
		ModelMatrix = glm::mat4(1.0);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(25.0f, -40.0f, -10.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(4.0f, 10.0f, 4.0f));
		MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
		glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		coral.draw(shader);

		//  CORAL 3 
		ModelMatrix = glm::mat4(1.0);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-45.0f, -38.0f, 15.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(2.0f, 12.0f, 2.0f));
		MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
		glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		coral.draw(shader);

		//  CORAL 4 
		ModelMatrix = glm::mat4(1.0);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(50.0f, -42.0f, -30.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(3.0f, 8.0f, 3.0f));
		MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
		glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		coral.draw(shader);

		ModelMatrix = glm::mat4(1.0);
		ModelMatrix = glm::translate(ModelMatrix, player->position);
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(4.0f));

		MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
		glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

		player->Draw(shader);


		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		window.update();

	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	return 0;
}


void processKeyboardInput()
{
	
	float playerSpeed = 30.0f * deltaTime;

	if (window.isPressed(GLFW_KEY_LEFT_SHIFT))
	{
		player->Dash();
		if (player->isDashing)
		{
			usedDash = true;
		}
	}

	if (player->isDashing)
	{
		playerSpeed *= 5.0f;
	}

	glm::vec3 forward = camera.getCameraViewDirection();

	// We use the camera's Up vector to ensure it's relative to the camera
	glm::vec3 right = glm::normalize(glm::cross(forward, camera.getCameraUp()));

	// Player Movement
	if (window.isPressed(GLFW_KEY_W))
	{
		player->position += forward * playerSpeed;
		pressedW = true;
	}
	if (window.isPressed(GLFW_KEY_S))
	{
		player->position -= forward * playerSpeed;
		pressedS = true;
	}
	if (window.isPressed(GLFW_KEY_A))
	{
		player->position -= right * playerSpeed;
		pressedA = true;
	}
	if (window.isPressed(GLFW_KEY_D))
	{
		player->position += right * playerSpeed;
		pressedD = true;
	}

	// 4. Swim Up/Down
	if (window.isPressed(GLFW_KEY_SPACE))
		player->position.y += playerSpeed;
	if (window.isPressed(GLFW_KEY_LEFT_CONTROL))
		player->position.y -= playerSpeed;

	// Camera Rotation Controls (Keep these to look around)
	float rotSpeed = 30.0f * deltaTime;
	if (window.isPressed(GLFW_KEY_LEFT)) camera.rotateOy(rotSpeed);
	if (window.isPressed(GLFW_KEY_RIGHT)) camera.rotateOy(-rotSpeed);
	if (window.isPressed(GLFW_KEY_UP)) camera.rotateOx(rotSpeed);
	if (window.isPressed(GLFW_KEY_DOWN)) camera.rotateOx(-rotSpeed);

	float groundLevel = -45.0f;  // above -50 
	glm::vec3 pos = camera.getCameraPosition();
	if (player->position.y < groundLevel)
	{
		player->position.y = groundLevel;
	}

	if (currentTask == 0)
	{
		// Task 1: Check if all WASD keys have been pressed
		if (pressedW && pressedA && pressedS && pressedD)
		{
			currentTask = 1;
		}
	}
	else if (currentTask == 1)
	{
		// Task 2: Check if dash was used
		if (usedDash)
		{
			currentTask = 2;
		}
	}

}
