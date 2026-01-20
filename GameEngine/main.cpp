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
float currentPlayerScale = 4.0f;

glm::vec3 rockPositions[] = {
	glm::vec3(-30.0f, -45.0f, -20.0f),
	glm::vec3(40.0f, -47.0f, 30.0f),
	glm::vec3(200.0f, -48.0f, -150.0f),
	glm::vec3(123.0f, -48.0f, -250.0f),
	glm::vec3(300.0f, -48.0f, -600.0f),
	glm::vec3(-315.0f, -48.0f, -540.0f),
	glm::vec3(50.0f, -45.0f, -100.0f),
	};

// Underwater lighting
glm::vec3 lightColor = glm::vec3(0.8f, 0.9f, 1.0f);
glm::vec3 lightPos = glm::vec3(0.0f, 200.0f, 0.0f);

// Spotlight
glm::vec3 spotlightPos = glm::vec3(0.0f, 0.0f, 0.0f);           // Position of spotlight
glm::vec3 spotlightDir = glm::vec3(0.0f, -1.0f, 0.0f);          // Direction (pointing down)
glm::vec3 spotlightColor = glm::vec3(1.0f, 1.0f, 0.5f);         // Bright golden color
float spotlightCutOff = glm::cos(glm::radians(30.0f));          // Inner cone angle (wider)
float spotlightOuterCutOff = glm::cos(glm::radians(45.0f));     // Outer cone angle (wider)

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
bool nearKey = false;
bool hasKey = false;
bool isGameOver = false;
bool nearChest = false;
bool chestSearched = false;
bool isGameWon = false;
const int totalTasks = 6;
int fishEatenCount = 0;



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
	GLuint texRed = loadBMP("Resources/Textures/red.bmp");
	GLuint texShip = loadBMP("Resources/Textures/shipp.bmp");

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

	std::vector<Texture> texturesRed;
	texturesRed.push_back(Texture());
	texturesRed[0].id = texRed;
	texturesRed[0].type = "texture_diffuse";

	std::vector<Texture> texturesShip;
	texturesShip.push_back(Texture());
	texturesShip[0].id = texShip;
	texturesShip[0].type = "texture_diffuse";

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
	Mesh shark = loader.loadObj("Resources/Models/shark.obj", texturesRed);
	Mesh fishkey = loader.loadObj("Resources/Models/fishkey.obj", textures3);
	Mesh reef = loader.loadObj("Resources/Models/reef.obj"); 
	Mesh ship = loader.loadObj("Resources/Models/ship.obj", texturesShip);



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

	// ===== SHARK =====
	glm::vec3 sharkPos = glm::vec3(200.0f, -20.0f, -500.0f);
	glm::vec3 sharkScale = glm::vec3(1.0f, 1.0f, 1.0f);
	float sharkRotDeg = 0.0f;
	glm::vec3 sharkRotAxis = glm::vec3(0.0f, 1.0f, 0.0f);

	// ===== REEFS  =====
		glm::vec3 reefPositions[] = {
		glm::vec3(-260.0f, -52.0f, -120.0f),
		glm::vec3(-180.0f, -52.0f, -220.0f),
		glm::vec3(-90.0f,  -52.0f, -320.0f),
		glm::vec3(20.0f, -52.0f, -180.0f),
		glm::vec3(110.0f, -52.0f, -260.0f),
		glm::vec3(210.0f, -52.0f, -340.0f),
		glm::vec3(280.0f, -52.0f, -480.0f),
		glm::vec3(120.0f, -52.0f, -520.0f),
		glm::vec3(-120.0f, -52.0f, -520.0f),
		glm::vec3(-260.0f, -52.0f, -420.0f),
	};

		float reefRotY[] = { 10.f, 45.f, 90.f, 25.f, 70.f, 140.f, 5.f, 110.f, 160.f, 35.f };
		glm::vec3 reefScale = glm::vec3(1.0f, 1.0f, 1.0f);


		// ===== SHip =====

		glm::vec3 shipPosition = glm::vec3(0.0f, -48.0f, -650.0f);
		glm::vec3 shipScale = glm::vec3(3.0f, 3.0f, 3.0f);
		float shipRotY = 180.0f;


	//fishtoeat
	glm::vec3 fishtoeatPositions[] = {
	glm::vec3(-15.0f, -10.0f, -60.0f),
	glm::vec3(20.0f, -5.0f, -90.0f),
	glm::vec3(-40.0f, -20.0f, -120.0f),
	glm::vec3(35.0f, 0.0f, -150.0f),
	glm::vec3(0.0f, -15.0f, -180.0f),
	glm::vec3(-25.0f, 10.0f, -70.0f),
	glm::vec3(40.0f, 20.0f, -100.0f),
	glm::vec3(-50.0f, 15.0f, -130.0f),
	glm::vec3(55.0f, 25.0f, -160.0f),
	glm::vec3(10.0f, 30.0f, -190.0f),
	};

	bool fishtoeatEaten[10] = { false };
		

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
		else if (currentTask == 2)
		{
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Task 1: COMPLETE!");
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Task 2: COMPLETE!");
			ImGui::Text("Task 3: Eat 3 fish!");
			ImGui::Text("Swim into smaller fish to eat them.");
			ImGui::Text("Progress: %d / 3", fishEatenCount);
		}
		else if (currentTask == 3)
		{
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Task 1: COMPLETE!");
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Task 2: COMPLETE!");
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Task 3: COMPLETE!");
			ImGui::Text("Task 4: Find the Key!");
			ImGui::Text("Explore the ocean to find it.");
		}
		else if (currentTask == 4)
		{
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Task 1: COMPLETE!");
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Task 2: COMPLETE!");
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Task 3: COMPLETE!");
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Task 4: COMPLETE!");
			ImGui::Text("Task 5: Take the Key!");
			ImGui::Text("Press E to take the key.");
		}
		else if (currentTask == 5)
		{
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Task 1: COMPLETE!");
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Task 2: COMPLETE!");
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Task 3: COMPLETE!");
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Task 4: COMPLETE!");
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Task 5: COMPLETE!");
			ImGui::Text("Task 6: Search the chest!");
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "WARNING: Avoid the shark!");

			if (nearChest)
			{
				ImGui::Separator();
				ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Press E to open the chest");
			}
			else
			{
				ImGui::Text("Follow the spotlight to the chest.");
			}
		}
		else if (currentTask >= totalTasks)
		{
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Task 1: COMPLETE!");
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Task 2: COMPLETE!");
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Task 3: COMPLETE!");
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Task 4: COMPLETE!");
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Task 5: COMPLETE!");
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Task 6: COMPLETE!");
			ImGui::Separator();
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "YOU WON! All tasks complete!");
		}

		ImGui::End();

		float t = (float)glfwGetTime();
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processKeyboardInput();

		if(!isGameOver)
		{
			player->Update(deltaTime);
		}

		if (!isGameOver)
		{
			if (glm::distance(player->position, sharkPos) < 40.0f)
			{
				isGameOver = true;
				lowHealthMode = true;
				waterColor = glm::vec3(0.5f, 0.0f, 0.0f);
			}
		}
		else
		{
			ImGui::SetNextWindowPos(ImVec2(window.getWidth() / 2, window.getHeight() / 2), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
			ImGui::Begin("GameOver", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground);
			ImGui::SetWindowFontScale(4.0f);
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "YOU DIED");
			ImGui::End();
		}

		if (isGameWon)
		{
			ImGui::SetNextWindowPos(ImVec2(window.getWidth() / 2, window.getHeight() / 2), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
			ImGui::Begin("GameWon", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground);
			ImGui::SetWindowFontScale(4.0f);
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "YOU WON!");
			ImGui::End();
		}

		for (int i = 0; i < 10; i++)
		{
			// If the fish is NOT eaten yet...
			if (!fishtoeatEaten[i])
			{
				// Check distance
				if (glm::distance(player->position, fishtoeatPositions[i]) < 8.0f)
				{
					fishtoeatEaten[i] = true;   // Mark as eaten
					fishEatenCount++;
					currentPlayerScale += 0.2f; // Grow bigger
					if (currentPlayerScale > 15.0f) currentPlayerScale = 15.0f; // Limit size
				}
			}
		}

		if (currentTask == 3 && !nearKey)
		{
			float distanceToKey = glm::distance(player->position, keyPos);

			if (distanceToKey < 200.0f)
			{
				nearKey = true;
				currentTask = 4;
			}
		}

		if (currentTask == 4 && !hasKey)
		{
			float dist = glm::distance(player->position, keyPos);
			if (dist < 20.0f)
			{
				if (window.isPressed(GLFW_KEY_E))
				{
					hasKey = true;
					currentTask = 5;
					glm::vec3 currentPos = player->position;
					delete player;
					player = new Player(&fishkey, currentPos);
				}
			}
		}

		if (currentTask == 5 && !chestSearched)
		{
			float distToChest = glm::distance(player->position, chestPos);

			if (distToChest < 100.0f)
			{
				nearChest = true;

				if (window.isPressed(GLFW_KEY_E))
				{
					chestSearched = true;
					currentTask = 6;
					isGameWon = true;
				}
			}
			else
			{
				nearChest = false;
			}
		}

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

		if (currentTask == 3 || currentTask == 4)
		{
			spotlightPos = glm::vec3(keyPos.x, keyPos.y + 50.0f, keyPos.z);
			spotlightDir = glm::vec3(0.0f, -1.0f, 0.0f);
			spotlightColor = glm::vec3(1.0f, 1.0f, 0.5f);
		}
		else if (currentTask == 5 || currentTask == 6)
		{
			// Spotlight on the chest
			spotlightPos = glm::vec3(chestPos.x, chestPos.y + 60.0f, chestPos.z);
			spotlightDir = glm::vec3(0.0f, -1.0f, 0.0f);
			spotlightColor = glm::vec3(0.5f, 1.0f, 0.5f);  // Green color to indicate goal
		}
		else
		{
			// Default - no spotlight
			spotlightPos = glm::vec3(0.0f, 0.0f, 0.0f);
			spotlightDir = glm::vec3(0.0f, -1.0f, 0.0f);
			spotlightColor = glm::vec3(1.0f, 1.0f, 0.8f);
		}

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

		if (!hasKey) 
		{
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
		}

		for (int i = 0; i < 10; i++)
		{
			if(!fishtoeatEaten[i])
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

		//ship
		{
			glm::mat4 ModelMatrix = glm::mat4(1.0f);
			ModelMatrix = glm::translate(ModelMatrix, shipPosition);
			ModelMatrix = glm::rotate(ModelMatrix, glm::radians(shipRotY), glm::vec3(0.0f, 1.0f, 0.0f));
			ModelMatrix = glm::scale(ModelMatrix, shipScale);

			glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
			glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
			glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

			ship.draw(shader);
		}

		//reefs
		for (int i = 0; i < 10; i++)
		{
			glm::mat4 ModelMatrix = glm::mat4(1.0f);
			ModelMatrix = glm::translate(ModelMatrix, reefPositions[i]);
			ModelMatrix = glm::rotate(ModelMatrix, glm::radians(reefRotY[i]), glm::vec3(0.0f, 1.0f, 0.0f));
			ModelMatrix = glm::scale(ModelMatrix, reefScale);

			glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
			glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
			glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

			reef.draw(shader);
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

		// ===== SHARK =====
		ModelMatrix = glm::mat4(1.0f);
		ModelMatrix = glm::translate(ModelMatrix, sharkPos);
		ModelMatrix = glm::rotate(ModelMatrix, glm::radians(sharkRotDeg), sharkRotAxis);
		ModelMatrix = glm::scale(ModelMatrix, sharkScale);

		MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
		glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

		shark.draw(shader);


		for (int i = 0; i < 7; i++)
		{
			glm::mat4 ModelMatrix = glm::mat4(1.0);
			ModelMatrix = glm::translate(ModelMatrix, rockPositions[i]);
			ModelMatrix = glm::scale(ModelMatrix, glm::vec3(8.0f, 5.0f, 8.0f)); // Fixed size for rocks

			glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
			glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
			glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

			rock.draw(shader);
		}

		ModelMatrix = glm::mat4(1.0);
		ModelMatrix = glm::translate(ModelMatrix, player->position);
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(currentPlayerScale));

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

bool CheckCollision(glm::vec3 targetPosition, float playerRadius)
{
	for (int i = 0; i < 7; i++)
	{
		glm::vec3 rockCenter = rockPositions[i];

		float rockRadius = 7.0f;

		float distance = glm::distance(targetPosition, rockCenter);

		if (distance < (playerRadius + rockRadius))
		{
			return true; // Collision!
		}
	}
	return false;
}

void processKeyboardInput()
{
	if (isGameOver)
		return;

	static bool f11Pressed = false;
	if (window.isPressed(GLFW_KEY_F11))
	{
		if (!f11Pressed)
		{
			window.toggleFullscreen();
			f11Pressed = true;
		}
	}
	else
	{
		f11Pressed = false;
	}

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
	glm::vec3 movement = player->position;

	// Player Movement
	if (window.isPressed(GLFW_KEY_W)) { movement += forward * playerSpeed; pressedW = true; }
	if (window.isPressed(GLFW_KEY_S)) { movement -= forward * playerSpeed; pressedS = true; }
	if (window.isPressed(GLFW_KEY_A)) { movement -= right * playerSpeed;   pressedA = true; }
	if (window.isPressed(GLFW_KEY_D)) { movement += right * playerSpeed;   pressedD = true; }

	if (window.isPressed(GLFW_KEY_SPACE))        movement.y += playerSpeed;
	if (window.isPressed(GLFW_KEY_LEFT_CONTROL)) movement.y -= playerSpeed;

	float playerRadius = 3.0f * (currentPlayerScale / 4.0f);

	if (CheckCollision(movement, playerRadius) == false)
	{
		player->position = movement;
	}

	// Task logic updates
	if (window.isPressed(GLFW_KEY_W)) pressedW = true;
	if (window.isPressed(GLFW_KEY_S)) pressedS = true;
	if (window.isPressed(GLFW_KEY_A)) pressedA = true;
	if (window.isPressed(GLFW_KEY_D)) pressedD = true;

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
	else if (currentTask == 2)
	{
		if (fishEatenCount >= 3) currentTask = 3;
	}


}
