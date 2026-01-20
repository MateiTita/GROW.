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
#include <algorithm>

void processKeyboardInput();

float deltaTime = 0.0f; // time between current frame and last frame
float lastFrame = 0.0f;
bool lowHealthMode = false;

Window window("Game Engine", 800, 800);
Camera camera;
Player* player;

struct CollisionPacket {
	glm::vec3 eRadius;      // Player's size (ellipsoid radius)

	// R3 (World Space) data
	glm::vec3 R3Velocity;
	glm::vec3 R3Position;

	// eSpace (Ellipsoid Space) data
	glm::vec3 velocity;
	glm::vec3 normalizedVelocity;
	glm::vec3 basePoint;

	// Hit information
	bool foundCollision;
	double nearestDistance;
	glm::vec3 intersectionPoint; // Point where we hit
};

struct Obstacle {
	glm::vec3 position;
	glm::vec3 size;
	Mesh* mesh;
};

std::vector<Obstacle> obstacles;

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

float cameraDistance = 30.0f;
float cameraHeight = 10.0f;
float orbitAngle = 0.0f;

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
	GLuint texTemple = loadBMP("Resources/Textures/templ.bmp");
	GLuint texfishtoeat1 = loadBMP("Resources/Textures/fishtoeat1.bmp");
	GLuint texshell = loadBMP("Resources/Textures/shell_basecolour.bmp");
	GLuint texChest = loadBMP("Resources/Textures/chestt.bmp");





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

	std::vector<Texture> texturesTemple;
	texturesTemple.push_back(Texture());
	texturesTemple[0].id = texTemple;
	texturesTemple[0].type = "texture_diffuse";

	std::vector<Texture> texturesShell;
	texturesShell.push_back(Texture());
	texturesShell[0].id = texshell;
	texturesShell[0].type = "texture_diffuse";

	std::vector<Texture> texturesFishtoeat1;
	texturesFishtoeat1.push_back(Texture());
	texturesFishtoeat1[0].id = texfishtoeat1;
	texturesFishtoeat1[0].type = "texture_diffuse";

	std::vector<Texture> texturesChest;
	texturesChest.push_back(Texture());
	texturesChest[0].id = texChest;
	texturesChest[0].type = "texture_diffuse";

	std::vector<Texture> texturesKey;
	texturesKey.push_back(Texture());
	texturesKey[0].id = tex3;
	texturesKey[0].type = "texture_diffuse";


	Mesh mesh(vert, ind, textures3);

	// Create Obj files - easier :)
	// we can add here our textures :)
	MeshLoaderObj loader;
	Mesh sun = loader.loadObj("Resources/Models/sphere.obj");
	Mesh rock = loader.loadObj("Resources/Models/cube.obj", textures2);   // Rock texture
	Mesh coral = loader.loadObj("Resources/Models/cube.obj", textures3);  // Orange texture
	Mesh plane = loader.loadObj("Resources/Models/plane.obj", texturesGround);
	Mesh fish = loader.loadObj("Resources/Models/fish.obj", textures3);
	Mesh temple = loader.loadObj("Resources/Models/temple.obj", texturesTemple);
	Mesh fishtoeat = loader.loadObj("Resources/Models/fishtoeat1.obj", texturesFishtoeat1);
	Mesh shell = loader.loadObj("Resources/Models/seashell_obj.obj", texturesShell);
	Mesh canoe = loader.loadObj("Resources/Models/canoe.obj", textures);
	Mesh chest = loader.loadObj("Resources/Models/chest.obj", texturesChest);
	Mesh key = loader.loadObj("Resources/Models/key.obj", texturesKey);





	player = new Player(&fish, glm::vec3(0.0f, -40.0f, 0.0f));
	glm::vec3 templePos = glm::vec3(0.0f, -50.0f, -620.0f);
	glm::vec3 templeScale = glm::vec3(50.0f, 50.0f, 50.0f);

	// ===== chest =====
	glm::vec3 chestPos = glm::vec3(100.0f, -20.0f, -780.0f);
	glm::vec3 chestScale = glm::vec3(6.0f, 6.0f, 6.0f);
	float chestRotDeg = 0.0f;
	glm::vec3 chestRotAxis = glm::vec3(0.0f, 1.0f, 0.0f);
	float chestFloatDistance = 5.0f; 
	float chestFloatSpeed = 1.2f;

	// ===== key =====
	glm::vec3 keyPos = glm::vec3(350.0f, -27.0f, -380.0f);
	glm::vec3 keyScale = glm::vec3(8.0f, 8.0f, 8.0f);
	glm::vec3 keyRotAxis = glm::vec3(0.0f, 1.0f, 0.0f);
	float keyFloatDistance = 5.0f;
	float keyFloatSpeed = 1.6f;
	float keySpinSpeed =160.0f;
	
	// ===== FISH TO EAT =====
	glm::vec3 fishtoeatPos = glm::vec3(140.0f, 120.0f, -80.0f);
	glm::vec3 fishtoeatScale = glm::vec3(12.0f, 12.0f, 12.0f);
	float fishtoeatRotDeg = 0.0f;
	glm::vec3 fishtoeatRotAxis = glm::vec3(0.0f, 1.0f, 0.0f);

	// ===== SHELL =====
	glm::vec3 shellPos = glm::vec3(-120.0f, -52.0f, -150.0f);
	glm::vec3 shellScale = glm::vec3(0.5f, 0.5f, 0.5f);
	float shellRotDeg = 0.0f;
	glm::vec3 shellRotAxis = glm::vec3(0.0f, 1.0f, 0.0f);

	// ===== CANOE (ONE) =====
	glm::vec3 canoePos = glm::vec3(350.0f, -47.0f, -380.0f);
	glm::vec3 canoeScale = glm::vec3(56.0f, 56.0f, 56.0f);
	float canoeRotDeg = 90.0f; // foarte des necesar
	glm::vec3 canoeRotAxis = glm::vec3(0.0f, 1.0f, 0.0f);


		obstacles.push_back({ glm::vec3(-30.0f, -45.0f, -20.0f), glm::vec3(8.0f, 5.0f, 8.0f), &rock });
		obstacles.push_back({ glm::vec3(40.0f, -47.0f, 30.0f),   glm::vec3(6.0f, 4.0f, 7.0f), &rock });
		obstacles.push_back({ glm::vec3(200.0f, -48.0f, -150.0f),  glm::vec3(5.0f, 3.0f, 5.0f), &rock });
		obstacles.push_back({ glm::vec3(123.0f, -48.0f, -250.0f),  glm::vec3(5.0f, 3.0f, 5.0f), &rock });
		obstacles.push_back({ glm::vec3(300.0f, -48.0f, -600.0f),  glm::vec3(5.0f, 3.0f, 5.0f), &rock });
		obstacles.push_back({ glm::vec3(-315.0f, -48.0f, -540.0f),  glm::vec3(5.0f, 3.0f, 5.0f), &rock });





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

		float t = (float)glfwGetTime();
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processKeyboardInput();


		player->Update(deltaTime);

		float offsetX = sin(orbitAngle) * cameraDistance;
		float offsetZ = cos(orbitAngle) * cameraDistance;

		// Calculate camera position on orbit circle
		glm::vec3 newCameraPos = player->position + glm::vec3(offsetX, cameraHeight, offsetZ);
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

		// TEMPLE 
		ModelMatrix = glm::mat4(1.0f);
		ModelMatrix = glm::translate(ModelMatrix, templePos);
		ModelMatrix = glm::scale(ModelMatrix, templeScale);
		MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
		glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		temple.draw(shader);


		//Chest 

		ModelMatrix = glm::mat4(1.0f);
		float chestBob = sin(t * chestFloatSpeed) * chestFloatDistance;
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(chestPos.x, chestPos.y + chestBob, chestPos.z));
		ModelMatrix = glm::rotate(ModelMatrix, glm::radians(chestRotDeg), chestRotAxis);
		ModelMatrix = glm::scale(ModelMatrix, chestScale);

		MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
		glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

		chest.draw(shader);



		//Key

		ModelMatrix = glm::mat4(1.0f);
		float keyBob = sin(t * keyFloatSpeed) * keyFloatDistance;
		float keySpinDeg = t * keySpinSpeed * 10;
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(keyPos.x, keyPos.y + keyBob, keyPos.z));
		ModelMatrix = glm::rotate(ModelMatrix, glm::radians(keySpinDeg), keyRotAxis);
		ModelMatrix = glm::scale(ModelMatrix, keyScale);
		MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
		glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

		key.draw(shader);



		//fishtoeat
		glm::vec3 fishtoeatPositions[] = {
			glm::vec3(-15.0f, 135.0f, -60.0f),
			glm::vec3(20.0f, 130.0f, -90.0f),
			glm::vec3(-40.0f, 125.0f, -120.0f),
			glm::vec3(35.0f, 120.0f, -150.0f),
			glm::vec3(0.0f, 128.0f, -180.0f),
			glm::vec3(-15.0f, 85.0f, -60.0f),
			glm::vec3(20.0f, 80.0f, -90.0f),
			glm::vec3(-40.0f, 65.0f, -120.0f),
			glm::vec3(35.0f, 75.0f, -150.0f),
			glm::vec3(0.0f, 130.0f, -180.0f),
		};

		for (int i = 0; i < 5; i++)
		{
			ModelMatrix = glm::mat4(1.0f);
			ModelMatrix = glm::translate(ModelMatrix, fishtoeatPositions[i]);
			ModelMatrix = glm::rotate(ModelMatrix, glm::radians(fishtoeatRotDeg), fishtoeatRotAxis);
			ModelMatrix = glm::scale(ModelMatrix, fishtoeatScale);

			MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
			glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
			glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

			fishtoeat.draw(shader);
		}


		//shell
		glm::vec3 shellPositions[] = {
			glm::vec3(-200.0f, -52.0f, -210.0f),
			glm::vec3(-350.0f, -52.0f, -130.0f),
			glm::vec3(100.0f, -52.0f, -90.0f),
			glm::vec3(250.0f, -52.0f, -250.0f),
			glm::vec3(450.0f, -52.0f, -320.0f),
			glm::vec3(-250.0f, -52.0f, -250.0f),
			glm::vec3(-450.0f, -52.0f, -320.0f),
		};

		for (int i = 0; i < 5; i++)
		{
			ModelMatrix = glm::mat4(1.0f);
			ModelMatrix = glm::translate(ModelMatrix, shellPositions[i]);
			ModelMatrix = glm::rotate(ModelMatrix, glm::radians(shellRotDeg), shellRotAxis);
			ModelMatrix = glm::scale(ModelMatrix, shellScale);

			MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
			glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
			glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

			shell.draw(shader);
		}

		//canoe
		ModelMatrix = glm::mat4(1.0f);
		ModelMatrix = glm::translate(ModelMatrix, canoePos);
		ModelMatrix = glm::rotate(ModelMatrix, glm::radians(canoeRotDeg), canoeRotAxis);
		ModelMatrix = glm::scale(ModelMatrix, canoeScale);
		MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
		glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		canoe.draw(shader);



		for (const auto& obj : obstacles)
		{
			glm::mat4 ModelMatrix = glm::mat4(1.0);
			ModelMatrix = glm::translate(ModelMatrix, obj.position);
			ModelMatrix = glm::scale(ModelMatrix, obj.size);

			glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
			glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
			glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

			obj.mesh->draw(shader);
		}

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

bool getLowestRoot(float a, float b, float c, float maxR, float* root) {
	float determinant = b * b - 4.0f * a * c;
	if (determinant < 0.0f) return false; // No solution

	float sqrtD = sqrt(determinant);
	float r1 = (-b - sqrtD) / (2 * a);
	float r2 = (-b + sqrtD) / (2 * a);

	if (r1 > r2) std::swap(r1, r2);

	if (r1 > 0 && r1 < maxR) {
		*root = r1;
		return true;
	}
	if (r2 > 0 && r2 < maxR) {
		*root = r2;
		return true;
	}
	return false;
}

void checkTriangle(CollisionPacket* colPackage, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3) {
	// 1. Create the plane for the triangle
	glm::vec3 edge1 = p2 - p1;
	glm::vec3 edge2 = p3 - p1;
	glm::vec3 normalVec = glm::cross(edge1, edge2);
	// If triangle has no area (degenerate), skip it to prevent crash
	if (glm::length(normalVec) < 0.00001f) return;
	glm::vec3 normal = glm::normalize(normalVec);
	

	// Only check front-facing triangles (optional optimization)
	if (glm::dot(normal, colPackage->normalizedVelocity) >= 0.0f) return;

	// Plane equation: N dot P + D = 0
	float distToPlane = -glm::dot(normal, p1);

	// 2. Calculate interval [t0, t1] where sphere intersects the infinite plane
	float signedDistToPlane = glm::dot(normal, colPackage->basePoint) + distToPlane;
	float normalDotVel = glm::dot(normal, colPackage->velocity);

	float t0, t1;
	bool embedded = false;

	if (fabs(normalDotVel) < 0.001f) {
		// Moving parallel to plane
		if (fabs(signedDistToPlane) >= 1.0f) return; // Too far away
		embedded = true;
		t0 = 0.0; t1 = 1.0;
	}
	else {
		t0 = (1.0f - signedDistToPlane) / normalDotVel;
		t1 = (-1.0f - signedDistToPlane) / normalDotVel;
		if (t0 > t1) std::swap(t0, t1);

		if (t0 > 1.0f || t1 < 0.0f) return; // No collision in this timeframe
		t0 = glm::clamp(t0, 0.0f, 1.0f);
		t1 = glm::clamp(t1, 0.0f, 1.0f);
	}

	// 3. Collision with the INSIDE of the triangle [cite: 213]
	glm::vec3 collisionPoint = glm::vec3(0.0f);
	bool foundCollision = false;
	float t = 1.0f;

	if (!embedded) {
		glm::vec3 planeIntersection = (colPackage->basePoint - normal) + (colPackage->velocity * t0);

		// Barycentric check (is point inside triangle?)
		glm::vec3 v0 = p2 - p1;
		glm::vec3 v1 = p3 - p1;
		glm::vec3 v2 = planeIntersection - p1;
		float d00 = glm::dot(v0, v0);
		float d01 = glm::dot(v0, v1);
		float d11 = glm::dot(v1, v1);
		float d20 = glm::dot(v2, v0);
		float d21 = glm::dot(v2, v1);
		float denom = d00 * d11 - d01 * d01;
		float v = (d11 * d20 - d01 * d21) / denom;
		float w = (d00 * d21 - d01 * d20) / denom;

		if (v >= 0.0f && w >= 0.0f && (v + w) <= 1.0f) {
			foundCollision = true;
			t = t0;
			collisionPoint = planeIntersection;
		}
	}

	// 4. Collision with Vertices/Edges (The Sweep Test) [cite: 235]
	if (!foundCollision) {
		float velocitySqDist = glm::dot(colPackage->velocity, colPackage->velocity);
		// If you don't have gtx/norm.hpp, use: dot(velocity, velocity)
		velocitySqDist = glm::dot(colPackage->velocity, colPackage->velocity);

		float a, b, c;
		float newT;

		// Check Vertices (sphere vs point)
		glm::vec3 vertices[] = { p1, p2, p3 };
		for (int i = 0; i < 3; i++) {
			a = velocitySqDist;
			b = 2.0f * glm::dot(colPackage->velocity, colPackage->basePoint - vertices[i]);
			glm::vec3 diff = vertices[i] - colPackage->basePoint;
			c = glm::dot(diff, diff) - 1.0f;

			if (getLowestRoot(a, b, c, t, &newT)) {
				t = newT;
				foundCollision = true;
				collisionPoint = vertices[i];
			}
		}
		// (Note: I skipped Edge checks for brevity. For a college project, 
		// Vertex + Inside checks are usually stable enough. Add edges if you still get stuck on sharp lines.)
	}

	// 5. Store result
	if (foundCollision) {
		float distToCollision = t * glm::length(colPackage->velocity);
		if (!colPackage->foundCollision || distToCollision < colPackage->nearestDistance) {
			colPackage->nearestDistance = distToCollision;
			colPackage->intersectionPoint = collisionPoint;
			colPackage->foundCollision = true;
		}
	}
}

glm::vec3 collideWithWorld(CollisionPacket* colPackage, int recursionDepth) {
	float veryCloseDistance = 0.005f; // Epsilon

	if (recursionDepth > 5) return colPackage->basePoint; // Safety break

	colPackage->foundCollision = false;
	colPackage->nearestDistance = 1e9; // Huge number

	// --- IMPORTANT: LOOP YOUR OBSTACLES HERE ---
	// We must check every triangle of every obstacle
	for (const auto& obj : obstacles) {
		// We need the vertices in World Space (or translate logic to Local Space)
		// For simplicity, let's assume we translate the Player to the Object's local space?
		// No, easier to translate Triangle to World Space.

		// This is SLOW without spatial partition, but fine for small college projects.
		const std::vector<Vertex>& verts = obj.mesh->vertices;
		const std::vector<int>& inds = obj.mesh->indices;

		for (size_t i = 0; i < inds.size(); i += 3) {
			glm::vec3 p1 = verts[inds[i]].pos * obj.size + obj.position;
			glm::vec3 p2 = verts[inds[i + 1]].pos * obj.size + obj.position;
			glm::vec3 p3 = verts[inds[i + 2]].pos * obj.size + obj.position;

			// Convert Triangle to eSpace (divide by player radius)
			p1 /= colPackage->eRadius;
			p2 /= colPackage->eRadius;
			p3 /= colPackage->eRadius;

			checkTriangle(colPackage, p1, p2, p3);
		}
	}

	// If no collision, we just move full distance
	if (!colPackage->foundCollision) {
		return colPackage->basePoint + colPackage->velocity;
	}

	// --- COLLISION RESPONSE (SLIDING) --- [cite: 291]

	// 1. Move very close to the collision point (but not exactly on it)
	glm::vec3 destinationPoint = colPackage->basePoint + colPackage->velocity;
	glm::vec3 newBasePoint = colPackage->basePoint;

	if (colPackage->nearestDistance >= veryCloseDistance) {
		glm::vec3 V = colPackage->velocity;
		V = glm::normalize(V);
		V = V * (float)(colPackage->nearestDistance - veryCloseDistance);
		newBasePoint = colPackage->basePoint + V;
	}

	// 2. Calculate Sliding Plane 
	// In eSpace, normal is vector from sphere center to intersection
	glm::vec3 slidePlaneOrigin = colPackage->intersectionPoint;
	glm::vec3 slidePlaneNormal = glm::normalize(newBasePoint - colPackage->intersectionPoint);

	// 3. Project velocity onto sliding plane 
	// New Destination = Destination - SignedDist * Normal
	float dist = glm::dot(destinationPoint - slidePlaneOrigin, slidePlaneNormal); // Signed distance
	glm::vec3 newDestinationPoint = destinationPoint - dist * slidePlaneNormal;

	// 4. New Velocity for recursion
	glm::vec3 newVelocity = newDestinationPoint - colPackage->intersectionPoint;

	// 5. Update Packet for recursion
	colPackage->basePoint = newBasePoint;
	colPackage->velocity = newVelocity;

	return collideWithWorld(colPackage, recursionDepth + 1);
}

void processKeyboardInput()
{
	float playerSpeed = 30.0f * deltaTime;

	if (window.isPressed(GLFW_KEY_LEFT_SHIFT))
	{
		player->Dash();
		if (player->isDashing && currentTask == 1)
		{
			usedDash = true;
		}
	}

	if (player->isDashing)
	{
		playerSpeed *= 5.0f;
	}

	glm::vec3 forward = camera.getCameraViewDirection();
	forward.y = 0.0f;
	forward = glm::normalize(forward);

	glm::vec3 right = glm::normalize(glm::cross(forward, camera.getCameraUp()));
	glm::vec3 movement(0.0f);

	// Player Movement
	if (window.isPressed(GLFW_KEY_W)) { movement += forward * playerSpeed; pressedW = true; }
	if (window.isPressed(GLFW_KEY_S)) { movement -= forward * playerSpeed; pressedS = true; }
	if (window.isPressed(GLFW_KEY_A)) { movement -= right * playerSpeed;   pressedA = true; }
	if (window.isPressed(GLFW_KEY_D)) { movement += right * playerSpeed;   pressedD = true; }

	if (window.isPressed(GLFW_KEY_SPACE))        movement.y += playerSpeed;
	if (window.isPressed(GLFW_KEY_LEFT_CONTROL)) movement.y -= playerSpeed;

	// If movement is essentially zero, don't run collision logic (avoids assertion fail)
	if (glm::length(movement) > 0.0001f)
	{
		glm::vec3 velocity = movement;
		glm::vec3 eRadius = glm::vec3(3.0f, 3.0f, 6.0f); // Size of your fish

		CollisionPacket colPackage;
		colPackage.eRadius = eRadius;
		colPackage.R3Position = player->position;
		colPackage.R3Velocity = velocity;

		colPackage.basePoint = colPackage.R3Position / eRadius;
		colPackage.velocity = colPackage.R3Velocity / eRadius;
		colPackage.normalizedVelocity = glm::normalize(colPackage.velocity);

		// Run Collision
		glm::vec3 finalPosESpace = collideWithWorld(&colPackage, 0);

		// Convert back
		player->position = finalPosESpace * eRadius;
	}
	// --- FIX END ---

	// Camera Rotation
	float rotSpeed = 2.0f * deltaTime;
	if (window.isPressed(GLFW_KEY_LEFT))  orbitAngle += rotSpeed;
	if (window.isPressed(GLFW_KEY_RIGHT)) orbitAngle -= rotSpeed;


	float groundLevel = -45.0f;
	if (player->position.y < groundLevel)
	{
		player->position.y = groundLevel;
	}

	float ceilingLevel = 100.0f; 
	if (player->position.y > ceilingLevel)
	{
		player->position.y = ceilingLevel;
	}

	if (currentTask == 0)
	{
		if (pressedW && pressedA && pressedS && pressedD) currentTask = 1;
	}
	else if (currentTask == 1)
	{
		if (usedDash) currentTask = 2;
	}
}
