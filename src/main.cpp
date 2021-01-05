#define STB_IMAGE_IMPLEMENTATION

#include "utils.h"
#include "camera.h"
#include "perlin.h"
#include "createimage.h"
#include "erosion.h"
#include "terrain.h"

#include "shader.h"
#include "SOIL.h"
#include "modelLoader.h"
#include "glm/gtx/string_cast.hpp"



// Tasks -
// 1. Check if erosion is working - Not sure, hangs for numIterations > 2000. Might need multi threading
// 2. Stitching along one axis not proper
// 3. Look into multi threading
// 4. Check varying numchunks visible and terrainXChunks and terrainYChunks
// 5. check code at line 180

int windowWidth = 1920, windowHeight=1080;

// camera
Camera camera(glm::vec3(0.0f, 15.0f, 0.0f));
float lastX = windowWidth / 2.0f;
float lastY = windowHeight / 2.0f;
bool firstMouse = true;
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

//Perlin noise generation
int octaves = 7; // Number of overlapping perlin maps
float persistence = 0.5f; // Persistence --> Decrease in amplitude of octaves
float lacunarity = 2.0f; // Lacunarity  --> Increase in frequency of octaves
float noiseScale = 64.0f; //Scale of the obtained map

//For infinite terrain
int numChunksVisible = 1;
int terrainXChunks = 3;
int terrainYChunks = 3;

// For trees
unsigned int amount = 500;
glm::mat4* modelMatrices;
std::map<std::vector<int>, bool> terrainChunkDict; //Keep track of whether terrain chunk at position or not
std::vector<std::vector<int>> lastViewedChunks; //Keep track of last viewed vectors
std::map<std::vector<int>, int> terrainPosVsChunkIndexDict; //Chunk position vs index in map_chunks

//Keep track of number of map chunks
int countChunks = 0;

//For erosion
int numIterations = 70000; //NUmber of drops to be simulated

void createPlane(std::vector<int> &position,  int xOffset, int yOffset, int height, int width, float heightMultiplier, float mapScale, unsigned int &program, GLuint &plane_VAO);
void setupModelTransformation(unsigned int &);
void setupViewTransformation(unsigned int &);
void setupProjectionTransformation(unsigned int &, int, int);
void createWorldTerrain(int height, int width, float heightMultiplier, float mapScale, unsigned int &program, std::vector<GLuint> &map_chunks, int numChunksVisible);
void processInput(GLFWwindow *window);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
std::vector<float> generateNoiseMap(int offsetX, int offsetY, int chunkHeight, int chunkWidth);
unsigned int skyBox();
unsigned int loadCubemap(vector<std::string> faces);

int main(int, char**)
{
	//For map generation
	int mapHeight = 128; //Height of each chunk
	int mapWidth = 128; //Width of each chunk
	float heightMultiplier = 75.0f; //Scale for height of peak
	float mapScale = 1.0f; //Scale for height and breadth of each chunk

	// Setup window
	GLFWwindow *window = setupWindow(windowWidth, windowHeight);
	ImGuiIO& io = ImGui::GetIO(); // Create IO object
	// glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	ImVec4 clearColor = ImVec4(1.0f, 1.0f, 1.0f, 1.00f);

	unsigned int shaderProgram = createProgram("./shaders/vshader.vs", "./shaders/fshader.fs");
	glUseProgram(shaderProgram);

	//unsigned int VAO;
	std::vector<GLuint> map_chunks(terrainXChunks * terrainYChunks);
	//glGenVertexArrays(1, &VAO);

	setupModelTransformation(shaderProgram);
	setupProjectionTransformation(shaderProgram, mapWidth , mapHeight); //maybe window width and height, check

	//createWorldTerrain(mapHeight, mapWidth, heightMultiplier, mapScale, shaderProgram, map_chunks, numChunksVisible);
    Shader ourShader("./shaders/model.vs", "./shaders/model.fs");
    Model ourModel("./objects/Tree/Tree.obj");
    ourShader.use();


	unsigned int skyboxVAO = skyBox();
    vector<std::string> faces
            {
                    "tex/right.jpg",
                    "tex/left.jpg",
                    "tex/top.jpg",
                    "tex/bottom.jpg",
                    "tex/front.jpg",
                    "tex/back.jpg"
            };

    unsigned int cubemapTexture = loadCubemap(faces);
    Shader skyboxShader("./shaders/skybox.vs", "./shaders/skybox.fs");
    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);


	//Create initial map chunk for testing

	//For testing
	// std::vector<int> position(2);
	// std::vector<float> noiseMap = generateNoiseMap(position[0], position[1], mapHeight, mapWidth);
 //    unsigned int VAO;
 //    glGenVertexArrays(1, &VAO);
	// createPlane(position, 0, 0, mapHeight, mapWidth, 150.0f, mapScale, shaderProgram, VAO);





    while (!glfwWindowShouldClose(window))
	{

		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		// -----
		processInput(window);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		//std::cout << camera.Position.x << " " << camera.Position.y << " " << camera.Position.z << " " <<std::endl;

		int camPosition_uniform = glGetUniformLocation(shaderProgram, "camPosition");
		if(camPosition_uniform == -1){
			fprintf(stderr, "Could not bind location: camPosition\n");
			exit(0);
		}
		glUniform3fv(camPosition_uniform, 1, glm::value_ptr(camera.Position));

		//Creating the terrain
		createWorldTerrain(mapHeight, mapWidth, heightMultiplier, mapScale, shaderProgram, map_chunks, numChunksVisible);

		setupViewTransformation(shaderProgram);

		glUseProgram(shaderProgram);



        {
			static float f = 0.0f;
			static int counter = 0;

			// ImGui::Begin("Information");
			// ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			// ImGui::End();
		}

		// Rendering
		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);

		for(int i = 0; i<terrainXChunks * terrainYChunks; i++){
			//std::cout << i << " " << map_chunks[i] << std::endl;
			glBindVertexArray(map_chunks[i]);
			glDrawArrays(GL_TRIANGLES, 0, (mapWidth-1) * (mapHeight-1) * 6);
		}

        ourShader.use();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)windowWidth / (float)windowHeight, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
        ourShader.setMat4("model", model);
        ourModel.Draw(ourShader);
        for (unsigned int i = 0; i < amount; i++)
        {
            ourShader.setMat4("model", modelMatrices[i]);
            ourModel.Draw(ourShader);
        }

		glDepthFunc(GL_LEQUAL);
        skyboxShader.use();
        view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);

		//For plane test
		// glBindVertexArray(VAO);
		// glDrawArrays(GL_TRIANGLES, 0, (mapWidth-1) * (mapHeight-1) * 6 * 9);






		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwPollEvents();
		glfwSwapBuffers(window);

	}

	for (int i = 0; i < map_chunks.size(); i++) {
		glDeleteVertexArrays(1, &map_chunks[i]);
	}
	// Cleanup
	cleanup(window);

	return 0;
}

//Creates the noise map using perlin noise
std::vector<float> generateNoiseMap(int offsetX, int offsetY, int chunkHeight, int chunkWidth) {
	std::vector<float> noiseValues;
	std::vector<float> normalizedNoiseValues;

	float amp  = 1;
	float freq = 1;
	float maxPossibleHeight = 0;

	for (int i = 0; i < octaves; i++) {
		maxPossibleHeight += amp;
		amp *= persistence;
	}

	for (int y = 0; y < chunkHeight; y++) {
		for (int x = 0; x < chunkWidth; x++) {
			amp  = 1;
			freq = 1;
			float noiseHeight = 0;
			//std::cout << x << " " <<  y << std::endl;
			for (int i = 0; i < octaves; i++) {
				float xSample = (x + (offsetX*(chunkWidth-1)) - (float)((chunkWidth-1)/2))  / noiseScale * freq;
				float ySample = (-y + (offsetY*(chunkHeight-1)) + (float)((chunkHeight-1)/2)) / noiseScale * freq;

				float perlinValue = getPerlinNoise(xSample, ySample);
				noiseHeight += perlinValue * amp;

				// Lacunarity  --> Increase in frequency of octaves
				// Persistence --> Decrease in amplitude of octaves
				amp  *= persistence;
				freq *= lacunarity;
			}

			noiseValues.push_back(noiseHeight);
		}
	}

	for (int y = 0; y < chunkHeight; y++) {
		for (int x = 0; x < chunkWidth; x++) {
			// Inverse lerp and scale values to range from 0 to 1
			float normalVal = (noiseValues[x + y*chunkWidth] + 1) / maxPossibleHeight;
			if(normalVal<0){
				normalVal = 0;
			}
			normalizedNoiseValues.push_back(normalVal);
		}
	}

	return normalizedNoiseValues;
}


//Create mesh for one chunk
void createPlane(std::vector<int> &position, int xOffset, int yOffset, int height, int width, float heightMultiplier, float mapScale, unsigned int &program, GLuint &plane_VAO) //Check if offsets required
{

	// Generate height map using perlin noise
	std::vector<float> noiseMap = generateNoiseMap(position[0], position[1], height, width);

	//Call erosion function
	//erode(noiseMap, height, width, numIterations);

	// // Create image of map
	// int isImageCreated = createImage(height, width, noiseMap, "noise_map");
	// if(isImageCreated==0){
	//     std::cout << "Noise Map Image successfully created" <<std::endl;
	// }


	glUseProgram(program);

	//Bind shader variables
	// int vVertex_attrib = glGetAttribLocation(program, "vVertex");
	// if(vVertex_attrib == -1) {
	// 	fprintf(stderr, "Could not bind location: vVertex\n");
	// 	exit(0);
	// }
	// int vColor_attrib = glGetAttribLocation(program, "vColor");
	// if(vColor_attrib == -1) {
	// 	fprintf(stderr, "Could not bind location: vColor\n");
	// 	exit(0);
	// }
	// int vNormal_attrib = glGetAttribLocation(program, "vNormal");
	// if(vNormal_attrib == -1) {
	// 	fprintf(stderr, "Could not bind location: vNormal\n");
	// 	exit(0);
	// }

	terrain* currTerrain = new terrain(position, height, width, heightMultiplier, mapScale, noiseMap);

    //For tree generation



	//Generate VAO object
	glGenVertexArrays(1, &plane_VAO);
	glBindVertexArray(plane_VAO);

	//Create VBOs for the VAO
	//Position information (data + format)
	int nVertices = currTerrain->getTriangleVerticesCount(height, width);
	GLfloat *expanded_vertices = currTerrain->getTrianglePoints();

	GLuint vertex_VBO;
	glGenBuffers(1, &vertex_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_VBO);
	glBufferData(GL_ARRAY_BUFFER, nVertices*3*sizeof(GLfloat), expanded_vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);


	GLfloat *expanded_normals = currTerrain->getNormals(height, width);

	GLuint normal_VBO;
	glGenBuffers(1, &normal_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, normal_VBO);
	glBufferData(GL_ARRAY_BUFFER, nVertices*3*sizeof(GLfloat), expanded_normals, GL_STATIC_DRAW);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	// delete []expanded_normals;
	delete []expanded_vertices;

	//Color - one for each face
	//GLfloat *expanded_colors = currTerrain->getTrianglePoints();

	GLfloat *expanded_vertices2 = currTerrain->getTrianglePoints();
	GLfloat *expanded_colors = new GLfloat[nVertices*3];

    modelMatrices = new glm::mat4[amount];
    srand(glfwGetTime()); // initialize random seed
    float radius = 100.0;
    float offset = 20.0f;
    for (unsigned int i = 0; i < amount; i++)
    {
        glm::mat4 model = glm::mat4(1.0f);
        // 1. translation: displace along circle with 'radius' in range [-offset, offset]
        float angle = (float)i / (float)amount * 360.0f;
        float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float x = sin(angle) * radius + displacement;
        displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float y = displacement * 0.5f; // keep height of asteroid field smaller compared to width of x and z
        displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float z = cos(angle) * radius + displacement;
        model = glm::translate(model, glm::vec3(x, y, z));

        // 2. scale: Scale between 0.05 and 0.25f
        float scale = (rand() % 20) / 100.0f + 1;
        model = glm::scale(model, glm::vec3(scale));

        // 3. rotation: add random rotation around a (semi)randomly picked rotation axis vector
//        float rotAngle = (rand() % 360);
//        model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));

        // 4. now add to list of matrices
        modelMatrices[i] = model;
    }

	//Creating different terrain types
	//Height is the upper threshold
	terrainType snowTerrain = terrainType("snow", heightMultiplier * 1.0f, 0.9, 0.9, 0.9); //white
	terrainType mountainTerrain = terrainType("mountain", heightMultiplier * 0.7f, 0.37, 0.18, 0.05); //Brown
	terrainType grassTerrain = terrainType("grass", heightMultiplier * 0.12f, 0.0, 0.49, 0.0); //green
	terrainType sandTerrain = terrainType("sand", heightMultiplier * 0.08f, 0.95, 0.83, 0.67); //Light Brown
	terrainType waterTerrain = terrainType("water", heightMultiplier * 0.04f, 0.0, 0.47, 0.75); //Blue

	int terrainCount = 5;
	std::vector<terrainType> terrainArr;
	terrainArr.push_back(waterTerrain);
	terrainArr.push_back(sandTerrain);
	terrainArr.push_back(grassTerrain);
	terrainArr.push_back(mountainTerrain);
	terrainArr.push_back(snowTerrain);

	for(int i=0; i<nVertices; i++) {

		//Assign color according to height
		float currHeight = expanded_vertices2[i*3+1];
		for(int j = 0; j<terrainCount; j++){
			if(currHeight <= terrainArr[j].height){
				// expanded_colors[i*3] = lerp(currHeight/heightMultiplier, 0.3, 1);
				// expanded_colors[i*3+1] = lerp(currHeight/heightMultiplier, 0.3, 1);
				// expanded_colors[i*3+2] = lerp(currHeight/heightMultiplier, 0.3, 1);
				expanded_colors[i*3] = terrainArr[j].red;
				expanded_colors[i*3+1] = terrainArr[j].green;
				expanded_colors[i*3+2] = terrainArr[j].blue;
				break;
			}
		}

	}

	GLuint color_VBO;
	glGenBuffers(1, &color_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, color_VBO);
	glBufferData(GL_ARRAY_BUFFER, nVertices*3*sizeof(GLfloat), expanded_colors, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
	delete []expanded_colors;
	delete []expanded_vertices2;

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//glBindVertexArray(0); //Unbind the VAO to disable changes outside this function.
}

//Create and combine multiple chunks - Endless terrain generation
void createWorldTerrain(int mapHeight, int mapWidth, float heightMultiplier, float mapScale, unsigned int &program, std::vector<GLuint> &map_chunks, int numChunksVisible){

	int chunkHeight = mapHeight;
	int chunkWidth = mapWidth;

	//Confirm if we are taking correct x and y coordinates
	int currChunkCoordX = (int)round(camera.Position.x / chunkWidth); //Get current chunk x coordinate
	int currChunkCoordY = (int)round(camera.Position.z / chunkHeight); //Get current chunk y(or z according to our coord system) coordinate

	//std::cout << " Currently at " << currChunkCoordX << " " << currChunkCoordY << std::endl;

	//Check which chunks have become out of view
	std::vector<std::vector<int>> chunksOutOfView;
	if(lastViewedChunks.empty()==false){
		for(std::vector<int> v: lastViewedChunks){
			if(abs(currChunkCoordX - v[0]) <= numChunksVisible && abs(currChunkCoordY-v[1])<=numChunksVisible){
				//Chunks inside render view
			}
			else{
				//chunks outside render view
				chunksOutOfView.push_back(v);
				std::cout << "chunks out of view size " << chunksOutOfView.size() << std::endl;
			}
		}
	}

	//Clear the last viewedChunks
	lastViewedChunks.clear();

	for(int yOffset = -numChunksVisible; yOffset<=numChunksVisible; yOffset++){
		for(int xOffset = -numChunksVisible; xOffset<=numChunksVisible; xOffset++){

			std::vector<int> viewedChunkCoord(2);
			viewedChunkCoord[0] = currChunkCoordX + xOffset;
			viewedChunkCoord[1] = currChunkCoordY + yOffset;

			if(terrainChunkDict.count(viewedChunkCoord)>0){ //terrain chunk already present


			}

			else{ //Not present

				//Check if replacement needed in map_chunks
				if(countChunks >= terrainXChunks * terrainYChunks){
					terrainChunkDict[viewedChunkCoord] = true;
					std::vector<int> replaceVector = chunksOutOfView[0];
					std::cout << "Replace vector is " << replaceVector[0]<< " " << replaceVector[1] << std::endl;
					int map_index = terrainPosVsChunkIndexDict[replaceVector];
					std::cout << "map index is " << map_index <<std::endl;
					createPlane(viewedChunkCoord,  xOffset, yOffset, mapHeight, mapWidth, heightMultiplier, mapScale, program, map_chunks[map_index]);
					terrainPosVsChunkIndexDict[viewedChunkCoord] = map_index;
					chunksOutOfView.erase(chunksOutOfView.begin());
					terrainChunkDict.erase(replaceVector);
					std::cout<<"Plane created at " << viewedChunkCoord[0] << " " << viewedChunkCoord[1] <<std::endl;
				}

				else{
					//No replacement needed
					terrainChunkDict[viewedChunkCoord] = true;
					terrainPosVsChunkIndexDict[viewedChunkCoord] = (xOffset+numChunksVisible) + (yOffset+numChunksVisible) * terrainXChunks;
					std::cout << "Initial map index is " << (xOffset+numChunksVisible) + (yOffset+numChunksVisible) * terrainXChunks <<std::endl;
					createPlane(viewedChunkCoord,  xOffset, yOffset, mapHeight, mapWidth, heightMultiplier, mapScale, program, map_chunks[(xOffset+numChunksVisible) + (yOffset+numChunksVisible) * terrainXChunks]);
					std::cout<<"Initial Plane created at " << viewedChunkCoord[0] << " " << viewedChunkCoord[1] <<std::endl;

				}

				for(int x: map_chunks){
					std::cout << "map_chunk " << x << std::endl;
				}

				for(std::vector<int> v: lastViewedChunks){
					std::cout << v[0] << " " << v[1] << " - " << terrainPosVsChunkIndexDict[v] << std::endl;
				}

				countChunks ++;

			}

			lastViewedChunks.push_back(viewedChunkCoord);


		}
	}

}

void setupModelTransformation(unsigned int &program)
{
	//Modelling transformations (Model -> World coordinates)
	glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.0, 0.0));//Model coordinates are the world coordinates
	//TODO: Q1 - Apply modelling transformations here.

	glm::vec3 point_a = glm::vec3(1.0,2.0,2.0);
	glm::vec3 vec_w = glm::normalize(point_a);

	glm::vec3 point_t = glm::vec3(vec_w.y, vec_w.x, vec_w.y);
	glm::vec3 vec_u = glm::normalize(glm::cross(point_t,vec_w));

	glm::vec3 vec_v = glm::cross(vec_w, vec_u);

	glm::mat4 matr_uvw = glm::mat4(
			vec_u.x, vec_u.y, vec_u.z, 0.0f,
			vec_v.x, vec_v.y, vec_v.z, 0.0f,
			vec_w.x, vec_w.y, vec_w.z, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
	);

	double degrees = 30;

	glm::mat4 matr_rot = glm::mat4(
			cos(glm::radians(degrees)), sin(glm::radians(degrees)), 0.0f, 0.0f,
			-sin(glm::radians(degrees)), cos(glm::radians(degrees)), 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
	);

	glm::mat4 matr_trans = glm::transpose(matr_uvw);

	model = matr_uvw * matr_rot * matr_trans;
	//TODO: Reset modelling transformations to Identity. Uncomment line below before attempting Q4!
	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.0, 0.0));//Model coordinates are the world coordinates

	//Pass on the modelling matrix to the vertex shader
	glUseProgram(program);
	int vModel_uniform = glGetUniformLocation(program, "vModel");
	if(vModel_uniform == -1){
		fprintf(stderr, "Could not bind location: vModel\n");
		exit(0);
	}
	glUniformMatrix4fv(vModel_uniform, 1, GL_FALSE, glm::value_ptr(model));
}

void setupViewTransformation(unsigned int &program)
{

	glm::mat4 view = camera.GetViewMatrix();
	glUseProgram(program);
	int vView_uniform = glGetUniformLocation(program, "vView");
	if(vView_uniform == -1){
		fprintf(stderr, "Could not bind location: vView\n");
		exit(0);
	}
	glUniformMatrix4fv(vView_uniform, 1, GL_FALSE, glm::value_ptr(view));
}

void setupProjectionTransformation(unsigned int &program, int screen_width, int screen_height)
{
	//Projection transformation
	float aspect = (float)screen_width/(float)screen_height;

	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)screen_width/(GLfloat)screen_height, 0.1f, 1000.0f);

	//Pass on the projection matrix to the vertex shader
	glUseProgram(program);
	int vProjection_uniform = glGetUniformLocation(program, "vProjection");
	if(vProjection_uniform == -1){
		fprintf(stderr, "Could not bind location: vProjection\n");
		exit(0);
	}
	glUniformMatrix4fv(vProjection_uniform, 1, GL_FALSE, glm::value_ptr(projection));
}

void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed, this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}


unsigned int skyBox(){
    float skyboxVertices[] = {
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f,  1.0f
    };

    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);


    return skyboxVAO;
}

unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}