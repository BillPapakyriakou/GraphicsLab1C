//********************************
//Αυτό το αρχείο θα το χρησιμοποιήσετε
// για να υλοποιήσετε την άσκηση 1B της OpenGL
//
//ΑΜ:   5186             Όνομα:  Βασιλείου Νικόλαος Μιχαήλ
//ΑΜ:   5324             Όνομα: Παπακυριακού Βασίλειος

//*********************************

#define _USE_MATH_DEFINES

#define STB_IMAGE_IMPLEMENTATION

#define STB_VORBIS_IMPLEMENTATION

// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <random>
#include <chrono>  
#include <cmath>

#include "stb_image.h"
#include "stb_vorbis.h"



// Include OpenAL
#include <AL/al.h>
#include <AL/alc.h>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;
using namespace std;


glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;


glm::mat4 getViewMatrix() {
	return ViewMatrix;
}
glm::mat4 getProjectionMatrix() {
	return ProjectionMatrix;
}



GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if (VertexShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << VertexShaderStream.rdbuf();
		VertexShaderCode = sstr.str();
		VertexShaderStream.close();
	}
	else {
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path);
		getchar();
		return 0;
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if (FragmentShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << FragmentShaderStream.rdbuf();
		FragmentShaderCode = sstr.str();
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;


	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const* VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}



	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const* FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}



	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}


	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}
///////////////////////////////////////////////////

// 2D Array for storing the coords of the cubes for maze generation

int labyrinth[10][10] = {
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
	{0, 0, 1, 1, 1, 1, 0, 1, 0, 1},
	{1, 0, 1, 0, 0, 0, 0, 1, 0, 1},
	{1, 0, 1, 0, 1, 1, 0, 1, 0, 1},
	{1, 0, 0, 0, 0, 1, 0, 0, 0, 1},
	{1, 0, 1, 1, 0, 1, 1, 1, 0, 1},
	{1, 0, 0, 0, 0, 0, 0, 1, 0, 0},
	{1, 0, 1, 0, 1, 1, 0, 0, 0, 1},
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};

const int rows = 10;  // const type because we use them later as function paremeters 
const int cols = 10;  // const type because we use them later as function paremeters

std::vector<float> vertices;    // for storing maze walls coords 
std::vector<float> verticesA;   // for storing playerA coords 
std::vector<float> verticesT;   // for storing treasure coords

float C_x = -4.5f; // character x pos
float C_y = 2.5f;  // character y pos
float C_z = 0.5f;  // character z pos

float T_x = -1.5f; // treasure x pos
float T_y = 1.5f;  // treasure y pos
float T_z = 0.5f;  // treasure z pos

glm::vec3 treasurePosition = glm::vec3(T_x, T_y, T_z);  // treasure pos vector(x, y, z)


// buffers that are updated outside of main
GLuint vertexBufferPlayer;   
GLuint vertexBufferMaze;      
GLuint vertexBufferTreasure; 
GLuint particlesVBO;


// Starting camera position, zoom factor and rotation angles
glm::vec3 cameraPosition = glm::vec3(0.0f, 0.0f, 20.0f);
float zoomFactor = cameraPosition.z;
float rotationAngleX = 0.0f;
float rotationAngleY = 0.0f;


// Create OpenGL texture
GLuint textureID;
int TextureWidth, TextureHeight, nrChannels;  // texture data
unsigned char* imageData;					  // image data for texture


std::vector<std::string> treasureTextures = { // for storing different textures
	"coins.jpg",
	"dirt.jpg",
	"spiral.jpg",
};


bool gameWon = false;           // game status (whether player has touched the treasure or not)
bool treasureVisible = true;

float scaleFactor = 1.0f;       // start with normal scaling for treasure object
float shrinkDuration = 1.0f;    // time to shrink 
float elapsedShrinkTime = 0.0f; 
float lastFrameTime = 0.0;     // helps in calculating delta time 



// early function declearations
void triggerTreasureSound();  
void createParticles(int count, glm::vec3 origin);


// for sound

bool soundPlayed = false;  // Flag to track if the sound has been played

std::vector<short> treasureAudioData;

int sampleRate, channels;

ALCdevice* device = nullptr;
ALCcontext* context = nullptr;
ALuint buffer = 0;
ALuint source = 0;


// for particles system

struct Particle {
	glm::vec3 position;
	glm::vec3 velocity; 
	float lifetime;     
	float size;         
};

std::vector<Particle> particles;


// Starting light cube position, rotationPoint (center of world space), zoom factor and rotation angles

glm::vec3 lightPos(10.0f, 8.0f, 4.0f);
glm::vec3 rotationPoint = glm::vec3(0.0f, 0.0f, 0.25f);
float zoomFactorLight = lightPos.z;
float rotationAngleXLight = 0.0f;
float rotationAngleYLight = 0.0f;


void generateMazeVertices() {
	

	// Each square is of size 1x1, and we want to center the maze at (0, 0).

	for (int i = 0; i < rows; ++i) {
		for (int j = 0; j < cols; ++j) {

			if (labyrinth[i][j] == 1) {

				// Calculate the four corners of the base of the cube

				float topLeftX = j - cols / 2.0f;
				float topLeftY = (rows / 2.0f) - i;

				float topRightX = topLeftX + 1.0f;
				float topRightY = topLeftY;

				float bottomLeftX = topLeftX;
				float bottomLeftY = topLeftY - 1.0f;

				float bottomRightX = topRightX;
				float bottomRightY = bottomLeftY;

				// Calculate Z positions for top and bottom faces

				float zBottom = 0.0f;
				float zTop = 1.0f;

				// Calculate triangles (2 for each face)

				// Bottom face (z = 0) triangles
				vertices.push_back(topLeftX); vertices.push_back(topLeftY); vertices.push_back(zBottom);
				vertices.push_back(bottomLeftX); vertices.push_back(bottomLeftY); vertices.push_back(zBottom);
				vertices.push_back(topRightX); vertices.push_back(topRightY); vertices.push_back(zBottom);

				vertices.push_back(bottomLeftX); vertices.push_back(bottomLeftY); vertices.push_back(zBottom);
				vertices.push_back(bottomRightX); vertices.push_back(bottomRightY); vertices.push_back(zBottom);
				vertices.push_back(topRightX); vertices.push_back(topRightY); vertices.push_back(zBottom);


				// Top face (z = 0) triangles
				vertices.push_back(topLeftX); vertices.push_back(topLeftY); vertices.push_back(zTop);
				vertices.push_back(bottomLeftX); vertices.push_back(bottomLeftY); vertices.push_back(zTop);
				vertices.push_back(topRightX); vertices.push_back(topRightY); vertices.push_back(zTop);

				vertices.push_back(bottomLeftX); vertices.push_back(bottomLeftY); vertices.push_back(zTop);
				vertices.push_back(bottomRightX); vertices.push_back(bottomRightY); vertices.push_back(zTop);
				vertices.push_back(topRightX); vertices.push_back(topRightY); vertices.push_back(zTop);


				// Front face triangles
				vertices.push_back(topLeftX); vertices.push_back(topLeftY); vertices.push_back(zBottom);
				vertices.push_back(topLeftX); vertices.push_back(topLeftY); vertices.push_back(zTop);
				vertices.push_back(topRightX); vertices.push_back(topRightY); vertices.push_back(zBottom);

				vertices.push_back(topRightX); vertices.push_back(topRightY); vertices.push_back(zBottom);
				vertices.push_back(topLeftX); vertices.push_back(topLeftY); vertices.push_back(zTop);
				vertices.push_back(topRightX); vertices.push_back(topRightY); vertices.push_back(zTop);


				// Back face triangles
				vertices.push_back(bottomLeftX); vertices.push_back(bottomLeftY); vertices.push_back(zBottom);
				vertices.push_back(bottomRightX); vertices.push_back(bottomRightY); vertices.push_back(zBottom);
				vertices.push_back(bottomLeftX); vertices.push_back(bottomLeftY); vertices.push_back(zTop);

				vertices.push_back(bottomLeftX); vertices.push_back(bottomLeftY); vertices.push_back(zTop);
				vertices.push_back(bottomRightX); vertices.push_back(bottomRightY); vertices.push_back(zBottom);
				vertices.push_back(bottomRightX); vertices.push_back(bottomRightY); vertices.push_back(zTop);


				// Left face triangles
				vertices.push_back(topLeftX); vertices.push_back(topLeftY); vertices.push_back(zBottom);
				vertices.push_back(bottomLeftX); vertices.push_back(bottomLeftY); vertices.push_back(zBottom);
				vertices.push_back(topLeftX); vertices.push_back(topLeftY); vertices.push_back(zTop);

				vertices.push_back(topLeftX); vertices.push_back(topLeftY); vertices.push_back(zTop);
				vertices.push_back(bottomLeftX); vertices.push_back(bottomLeftY); vertices.push_back(zBottom);
				vertices.push_back(bottomLeftX); vertices.push_back(bottomLeftY); vertices.push_back(zTop);


				// Right face triangles
				vertices.push_back(topRightX); vertices.push_back(topRightY); vertices.push_back(zBottom);
				vertices.push_back(topRightX); vertices.push_back(topRightY); vertices.push_back(zTop);
				vertices.push_back(bottomRightX); vertices.push_back(bottomRightY); vertices.push_back(zBottom);

				vertices.push_back(bottomRightX); vertices.push_back(bottomRightY); vertices.push_back(zBottom);
				vertices.push_back(topRightX); vertices.push_back(topRightY); vertices.push_back(zTop);
				vertices.push_back(bottomRightX); vertices.push_back(bottomRightY); vertices.push_back(zTop);

			}
		}
	}
}

void DrawPlayer() {

	verticesA.clear();

	float halfSize = 0.25f;  // Half of the cube's side length
	

	// Front face (two triangles)
	verticesA.push_back(C_x - halfSize); verticesA.push_back(C_y + halfSize); verticesA.push_back(C_z + halfSize);  // Top-left front
	verticesA.push_back(C_x - halfSize); verticesA.push_back(C_y - halfSize); verticesA.push_back(C_z + halfSize);  // Bottom-left front
	verticesA.push_back(C_x + halfSize); verticesA.push_back(C_y + halfSize); verticesA.push_back(C_z + halfSize);  // Top-right front

	verticesA.push_back(C_x - halfSize); verticesA.push_back(C_y - halfSize); verticesA.push_back(C_z + halfSize);  // Bottom-left front
	verticesA.push_back(C_x + halfSize); verticesA.push_back(C_y - halfSize); verticesA.push_back(C_z + halfSize);  // Bottom-right front
	verticesA.push_back(C_x + halfSize); verticesA.push_back(C_y + halfSize); verticesA.push_back(C_z + halfSize);  // Top-right front

	// Back face (two triangles)
	verticesA.push_back(C_x - halfSize); verticesA.push_back(C_y + halfSize); verticesA.push_back(C_z - halfSize);  // Top-left back
	verticesA.push_back(C_x - halfSize); verticesA.push_back(C_y - halfSize); verticesA.push_back(C_z - halfSize);  // Bottom-left back
	verticesA.push_back(C_x + halfSize); verticesA.push_back(C_y + halfSize); verticesA.push_back(C_z - halfSize);  // Top-right back

	verticesA.push_back(C_x - halfSize); verticesA.push_back(C_y - halfSize); verticesA.push_back(C_z - halfSize);  // Bottom-left back
	verticesA.push_back(C_x + halfSize); verticesA.push_back(C_y - halfSize); verticesA.push_back(C_z - halfSize);  // Bottom-right back
	verticesA.push_back(C_x + halfSize); verticesA.push_back(C_y + halfSize); verticesA.push_back(C_z - halfSize);  // Top-right back

	// Left face
	verticesA.push_back(C_x - halfSize); verticesA.push_back(C_y + halfSize); verticesA.push_back(C_z + halfSize);  // Top-left front
	verticesA.push_back(C_x - halfSize); verticesA.push_back(C_y - halfSize); verticesA.push_back(C_z + halfSize);  // Bottom-left front
	verticesA.push_back(C_x - halfSize); verticesA.push_back(C_y + halfSize); verticesA.push_back(C_z - halfSize);  // Top-left back

	verticesA.push_back(C_x - halfSize); verticesA.push_back(C_y - halfSize); verticesA.push_back(C_z + halfSize);  // Bottom-left front
	verticesA.push_back(C_x - halfSize); verticesA.push_back(C_y - halfSize); verticesA.push_back(C_z - halfSize);  // Bottom-left back
	verticesA.push_back(C_x - halfSize); verticesA.push_back(C_y + halfSize); verticesA.push_back(C_z - halfSize);  // Top-left back

	// Right face
	verticesA.push_back(C_x + halfSize); verticesA.push_back(C_y + halfSize); verticesA.push_back(C_z + halfSize);  // Top-right front
	verticesA.push_back(C_x + halfSize); verticesA.push_back(C_y - halfSize); verticesA.push_back(C_z + halfSize);  // Bottom-right front
	verticesA.push_back(C_x + halfSize); verticesA.push_back(C_y + halfSize); verticesA.push_back(C_z - halfSize);  // Top-right back

	verticesA.push_back(C_x + halfSize); verticesA.push_back(C_y - halfSize); verticesA.push_back(C_z + halfSize);  // Bottom-right front
	verticesA.push_back(C_x + halfSize); verticesA.push_back(C_y - halfSize); verticesA.push_back(C_z - halfSize);  // Bottom-right back
	verticesA.push_back(C_x + halfSize); verticesA.push_back(C_y + halfSize); verticesA.push_back(C_z - halfSize);  // Top-right back

	// Top face
	verticesA.push_back(C_x - halfSize); verticesA.push_back(C_y + halfSize); verticesA.push_back(C_z + halfSize);  // Top-left front
	verticesA.push_back(C_x + halfSize); verticesA.push_back(C_y + halfSize); verticesA.push_back(C_z + halfSize);  // Top-right front
	verticesA.push_back(C_x - halfSize); verticesA.push_back(C_y + halfSize); verticesA.push_back(C_z - halfSize);  // Top-left back

	verticesA.push_back(C_x + halfSize); verticesA.push_back(C_y + halfSize); verticesA.push_back(C_z + halfSize);  // Top-right front
	verticesA.push_back(C_x + halfSize); verticesA.push_back(C_y + halfSize); verticesA.push_back(C_z - halfSize);  // Top-right back
	verticesA.push_back(C_x - halfSize); verticesA.push_back(C_y + halfSize); verticesA.push_back(C_z - halfSize);  // Top-left back

	// Bottom face
	verticesA.push_back(C_x - halfSize); verticesA.push_back(C_y - halfSize); verticesA.push_back(C_z + halfSize);  // Bottom-left front
	verticesA.push_back(C_x + halfSize); verticesA.push_back(C_y - halfSize); verticesA.push_back(C_z + halfSize);  // Bottom-right front
	verticesA.push_back(C_x - halfSize); verticesA.push_back(C_y - halfSize); verticesA.push_back(C_z - halfSize);  // Bottom-left back

	verticesA.push_back(C_x + halfSize); verticesA.push_back(C_y - halfSize); verticesA.push_back(C_z + halfSize);  // Bottom-right front
	verticesA.push_back(C_x + halfSize); verticesA.push_back(C_y - halfSize); verticesA.push_back(C_z - halfSize);  // Bottom-right back
	verticesA.push_back(C_x - halfSize); verticesA.push_back(C_y - halfSize); verticesA.push_back(C_z - halfSize);  // Bottom-left back

}

void DrawTreasure() {

	verticesT.clear();

	float halfSizeT = 0.4f * scaleFactor;  // Adjust treasure size with scaleFactor
										   // 0.4 = Half of the treasure cube's side length

	// Front face (two triangles)
	verticesT.push_back(T_x - halfSizeT); verticesT.push_back(T_y + halfSizeT); verticesT.push_back(T_z + halfSizeT);  // Top-left front
	verticesT.push_back(T_x - halfSizeT); verticesT.push_back(T_y - halfSizeT); verticesT.push_back(T_z + halfSizeT);  // Bottom-left front
	verticesT.push_back(T_x + halfSizeT); verticesT.push_back(T_y + halfSizeT); verticesT.push_back(T_z + halfSizeT);  // Top-right front

	verticesT.push_back(T_x - halfSizeT); verticesT.push_back(T_y - halfSizeT); verticesT.push_back(T_z + halfSizeT);  // Bottom-left front
	verticesT.push_back(T_x + halfSizeT); verticesT.push_back(T_y - halfSizeT); verticesT.push_back(T_z + halfSizeT);  // Bottom-right front
	verticesT.push_back(T_x + halfSizeT); verticesT.push_back(T_y + halfSizeT); verticesT.push_back(T_z + halfSizeT);  // Top-right front

	// Back face (two triangles)
	verticesT.push_back(T_x - halfSizeT); verticesT.push_back(T_y + halfSizeT); verticesT.push_back(T_z - halfSizeT);  // Top-left back
	verticesT.push_back(T_x - halfSizeT); verticesT.push_back(T_y - halfSizeT); verticesT.push_back(T_z - halfSizeT);  // Bottom-left back
	verticesT.push_back(T_x + halfSizeT); verticesT.push_back(T_y + halfSizeT); verticesT.push_back(T_z - halfSizeT);  // Top-right back

	verticesT.push_back(T_x - halfSizeT); verticesT.push_back(T_y - halfSizeT); verticesT.push_back(T_z - halfSizeT);  // Bottom-left back
	verticesT.push_back(T_x + halfSizeT); verticesT.push_back(T_y - halfSizeT); verticesT.push_back(T_z - halfSizeT);  // Bottom-right back
	verticesT.push_back(T_x + halfSizeT); verticesT.push_back(T_y + halfSizeT); verticesT.push_back(T_z - halfSizeT);  // Top-right back

	// Left face
	verticesT.push_back(T_x - halfSizeT); verticesT.push_back(T_y + halfSizeT); verticesT.push_back(T_z + halfSizeT);  // Top-left front
	verticesT.push_back(T_x - halfSizeT); verticesT.push_back(T_y - halfSizeT); verticesT.push_back(T_z + halfSizeT);  // Bottom-left front
	verticesT.push_back(T_x - halfSizeT); verticesT.push_back(T_y + halfSizeT); verticesT.push_back(T_z - halfSizeT);  // Top-left back

	verticesT.push_back(T_x - halfSizeT); verticesT.push_back(T_y - halfSizeT); verticesT.push_back(T_z + halfSizeT);  // Bottom-left front
	verticesT.push_back(T_x - halfSizeT); verticesT.push_back(T_y - halfSizeT); verticesT.push_back(T_z - halfSizeT);  // Bottom-left back
	verticesT.push_back(T_x - halfSizeT); verticesT.push_back(T_y + halfSizeT); verticesT.push_back(T_z - halfSizeT);  // Top-left back

	// Right face
	verticesT.push_back(T_x + halfSizeT); verticesT.push_back(T_y + halfSizeT); verticesT.push_back(T_z + halfSizeT);  // Top-right front
	verticesT.push_back(T_x + halfSizeT); verticesT.push_back(T_y - halfSizeT); verticesT.push_back(T_z + halfSizeT);  // Bottom-right front
	verticesT.push_back(T_x + halfSizeT); verticesT.push_back(T_y + halfSizeT); verticesT.push_back(T_z - halfSizeT);  // Top-right back

	verticesT.push_back(T_x + halfSizeT); verticesT.push_back(T_y - halfSizeT); verticesT.push_back(T_z + halfSizeT);  // Bottom-right front
	verticesT.push_back(T_x + halfSizeT); verticesT.push_back(T_y - halfSizeT); verticesT.push_back(T_z - halfSizeT);  // Bottom-right back
	verticesT.push_back(T_x + halfSizeT); verticesT.push_back(T_y + halfSizeT); verticesT.push_back(T_z - halfSizeT);  // Top-right back

	// Top face
	verticesT.push_back(T_x - halfSizeT); verticesT.push_back(T_y + halfSizeT); verticesT.push_back(T_z + halfSizeT);  // Top-left front
	verticesT.push_back(T_x + halfSizeT); verticesT.push_back(T_y + halfSizeT); verticesT.push_back(T_z + halfSizeT);  // Top-right front
	verticesT.push_back(T_x - halfSizeT); verticesT.push_back(T_y + halfSizeT); verticesT.push_back(T_z - halfSizeT);  // Top-left back

	verticesT.push_back(T_x + halfSizeT); verticesT.push_back(T_y + halfSizeT); verticesT.push_back(T_z + halfSizeT);  // Top-right front
	verticesT.push_back(T_x + halfSizeT); verticesT.push_back(T_y + halfSizeT); verticesT.push_back(T_z - halfSizeT);  // Top-right back
	verticesT.push_back(T_x - halfSizeT); verticesT.push_back(T_y + halfSizeT); verticesT.push_back(T_z - halfSizeT);  // Top-left back

	// Bottom face
	verticesT.push_back(T_x - halfSizeT); verticesT.push_back(T_y - halfSizeT); verticesT.push_back(T_z + halfSizeT);  // Bottom-left front
	verticesT.push_back(T_x + halfSizeT); verticesT.push_back(T_y - halfSizeT); verticesT.push_back(T_z + halfSizeT);  // Bottom-right front
	verticesT.push_back(T_x - halfSizeT); verticesT.push_back(T_y - halfSizeT); verticesT.push_back(T_z - halfSizeT);  // Bottom-left back

	verticesT.push_back(T_x + halfSizeT); verticesT.push_back(T_y - halfSizeT); verticesT.push_back(T_z + halfSizeT);  // Bottom-right front
	verticesT.push_back(T_x + halfSizeT); verticesT.push_back(T_y - halfSizeT); verticesT.push_back(T_z - halfSizeT);  // Bottom-right back
	verticesT.push_back(T_x - halfSizeT); verticesT.push_back(T_y - halfSizeT); verticesT.push_back(T_z - halfSizeT);  // Bottom-left back


}



GLuint LoadTextureFromImage(const std::string& textureName) {

	// Load the image using stbi_load
	imageData = stbi_load(textureName.c_str(), & TextureWidth, & TextureHeight, & nrChannels, 0);

	if (!imageData) {
		std::cerr << "Failed to load texture!" << std::endl;
		return 0;
	}
	else {
		//std::cout << "Loaded texture!" << std::endl;
	}

	
	glGenTextures(1, &textureID);
	glActiveTexture(GL_TEXTURE0);


	// Bind texture
	glBindTexture(GL_TEXTURE_2D, textureID);

	// Give image to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TextureWidth, TextureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData);


	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// clear load image dara
	stbi_image_free(imageData);

	return textureID;

}


// Finds valid positions to place treasure object
std::vector<std::pair<int, int>> findFreePositions(const int labyrinth[rows][cols], float excludeX, float excludeY) {
	
	std::vector<std::pair<int, int>> freePositions;  // free positions array (x,y)

	// initial position in labyrinth is (0,2)
	int labyrinthPosX = static_cast<int>(excludeX + 4.5f); 
	int labyrinthPosY = static_cast<int>(4.5f - excludeY); 
	

	for (int i = 0; i < rows; ++i) {
		for (int j = 0; j < cols; ++j) {

			if (labyrinth[i][j] == 0 && !(j == labyrinthPosX && i == labyrinthPosY)) { 
				//std::cout << "checked for: (" << labyrinthPosX << ", " << labyrinthPosY << ")" << std::endl;
				freePositions.emplace_back(i, j);  // push_back for pairs or tuples
			}
		}
	}

	return freePositions;
}

// This was a test to check if the free positions for the treasure were correct
// Iterates through all free positions on the grid
/*

// Timer variables
std::chrono::steady_clock::time_point lastMoveTime;
bool canMoveTreasure = true;
size_t currentPosIndex = 0;  // Tracks the current position in freePositions

void moveTreasureToRandomPosition() {
	
	std::vector<std::pair<int, int>> freePositions = findFreePositions(labyrinth, C_x, C_y);
	// Only allow moving the treasure once every second and ensure there are valid positions
	if (canMoveTreasure && !freePositions.empty()) {
		
		// Move the treasure to the current position
		
		T_x = freePositions[currentPosIndex].second - 4.5f;
		T_y = 4.5f - freePositions[currentPosIndex].first;
		T_z = 0.5f;  

		// Draws the treasure after updating its position
		DrawTreasure();

		// Bind OpenGL buffers to reflect the new treasure position
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferTreasure);
		glBufferData(GL_ARRAY_BUFFER, verticesT.size() * sizeof(float), verticesT.data(), GL_DYNAMIC_DRAW);


		//std::cout << "Treasure moved to: (" << freePositions[currentPosIndex].first << ", " << freePositions[currentPosIndex].second << ")" << std::endl;

		// Set timer for next treasure movement
		lastMoveTime = std::chrono::steady_clock::now();
		canMoveTreasure = false;  

		// Move to next position in list (loop back to start after last position)
		currentPosIndex = (currentPosIndex + 1) % freePositions.size();
	}

	// Check if 1 second has passed since last move
	if (!canMoveTreasure) {
		auto currentTime = std::chrono::steady_clock::now();
		std::chrono::duration<float> elapsed = currentTime - lastMoveTime;

		// Re-enable treasure movement
		if (elapsed.count() >= 1.0f) {
			canMoveTreasure = true;
		}
	}
}
*/

void moveTreasureToRandomPosition() {

	std::vector<std::pair<int, int>> freePositions = findFreePositions(labyrinth, C_x, C_y);

	
	if (!freePositions.empty()) {

		// pick random position
		int randomIndex = rand() % freePositions.size();
		T_x = freePositions[randomIndex].second - 4.5f; // map position to grid position
		T_y = 4.5f - freePositions[randomIndex].first;  // map position to grid position


		// Load a random texture
		static GLuint treasureTextureID = 0;
		int randomTextureIndex = rand() % treasureTextures.size();

		//std::cout << "Selected texture index: " << randomTextureIndex
		//	<< " Texture Name: " << treasureTextures[randomTextureIndex] << std::endl;

		if (treasureTextureID != 0) {
			glDeleteTextures(1, &treasureTextureID); // Clean up old texture
			//std::cout << "cleaned texture " << std::endl;
		}
		treasureTextureID = LoadTextureFromImage(treasureTextures[randomTextureIndex]);

		// bind texture
		glBindTexture(GL_TEXTURE_2D, treasureTextureID);

		DrawTreasure();  // redraw treasure after change

		// bind buffers to reflect the treasure positions change
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferTreasure);
		glBufferData(GL_ARRAY_BUFFER, verticesT.size() * sizeof(float), verticesT.data(), GL_DYNAMIC_DRAW);

		//std::cout << "Treasure moved to: (" << T_x << ", " << T_y << ")" << std::endl;
	}
	
}


// Delta time : time that has passed between 2 consecutive frames
// Achieves smooth movement regardless of the programs frame rate

float calculateDeltaTime() {

	float currentFrameTime = glfwGetTime(); // Get the current time
	float deltaTime = currentFrameTime - lastFrameTime;
	lastFrameTime = currentFrameTime; // Update the last frame time
	
	return deltaTime;

}


void updateTreasurePosition() {

	float deltaTime = calculateDeltaTime();
	float currentTime = glfwGetTime();  // get the current time

	static float lastUpdate = 0.0f;

	if (!gameWon) {

		static float elapsedTime = 0.0f;
		elapsedTime = currentTime - lastUpdate;

		if (elapsedTime >= 2.0f) {

			moveTreasureToRandomPosition();
			lastUpdate = currentTime;  // reset timer

		}
	}

	else {

		// Timer to track time passed since the shrinking started
		static float shrinkTimer = 0.0f;

		if (treasureVisible) {

			if (scaleFactor > 0.5f) {

				elapsedShrinkTime += deltaTime;

				// calculates scale factor (interpolate between values 1.0 and 0.5)
				scaleFactor = glm::max(0.5f, 1.0f - elapsedShrinkTime / shrinkDuration);

				//std::cout << " scaling.. " << std::endl;

				DrawTreasure();

				// bind buffers to reflect the treasure positions change
				glBindBuffer(GL_ARRAY_BUFFER, vertexBufferTreasure);
				glBufferData(GL_ARRAY_BUFFER, verticesT.size() * sizeof(float), verticesT.data(), GL_DYNAMIC_DRAW);

			}

			// Increment the shrink timer
			shrinkTimer += deltaTime;

			// If given seconds have passed after shrinking started, clear vertices
			if (shrinkTimer >= 0.8f) {
				verticesT.clear();  // Clear vertices
				//std::cout << "clear vertices." << std::endl;
				treasureVisible = false;  // Mark treasure as no longer visible
			}
		}
	}
}



// returns true if player move is valid
bool isValidMove(int newX, int newY) {


	int checkX = newX;
	int checkY = newY + 2; // because player starts at [0][2] 


	if (checkX < 0 || checkX >= 10 || checkY < 0 || checkY >= 10) {
		//std::cout << "Out of bounds: (" << checkX << ", " << checkY << ")" << std::endl;
		return false;
	}

	//std::cout << "(" << checkX << ", " << checkY << ")" << std::endl;

	return labyrinth[checkY][checkX] == 0;

}


void keyCallbackPlayer(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS || action == GLFW_REPEAT) {

		// assumes initial player pos as (0, 0)
		int gridX = static_cast<int>(C_x + 4.5f); // Convert to grid position
		int gridY = static_cast<int>(2.5f - C_y); // Convert to grid position


		int newX = gridX;
		int newY = gridY;

		const int startX = 0;
		const int startY = 0;
		const int exitX = 9;
		const int exitY = 5;

		// check if player is at start or at exit positions

		// check if L is pressed at exit and teleport to start
		if (gridX == exitX && gridY == exitY && key == GLFW_KEY_L) {
			C_x = startX - 4.5f;
			C_y = 2.5f - startY;

			// Update gridX and gridY to the new position
			gridX = startX;
			gridY = startY;

			DrawPlayer();  // redraw player after change

			// bind buffers to reflect the players positions change
			glBindBuffer(GL_ARRAY_BUFFER, vertexBufferPlayer);
			glBufferData(GL_ARRAY_BUFFER, verticesA.size() * sizeof(float), verticesA.data(), GL_DYNAMIC_DRAW);


		}
		// check if J is pressed at start and teleport to exit
		else if (gridX == startX && gridY == startY && key == GLFW_KEY_J) {
			C_x = exitX - 4.5f;
			C_y = 2.5f - exitY;

			// Update gridX and gridY to the new position
			gridX = exitX;
			gridY = exitY;

			DrawPlayer();  // redraw player after change

			// bind buffers to reflect the players positions change
			glBindBuffer(GL_ARRAY_BUFFER, vertexBufferPlayer);
			glBufferData(GL_ARRAY_BUFFER, verticesA.size() * sizeof(float), verticesA.data(), GL_DYNAMIC_DRAW);


		}

		else {


			// Handle movement keys
			switch (key) {
			case GLFW_KEY_L: newX += 1; break; // right
			case GLFW_KEY_J: newX -= 1; break; // left
			case GLFW_KEY_K: newY += 1; break; // down
			case GLFW_KEY_I: newY -= 1; break; // up
			}


			// Check if the new move is valid before updating
			if (isValidMove(newX, newY)) {

				C_x = newX - 4.5f;
				C_y = 2.5f - newY;

				// Update gridX and gridY after C_x and C_y change to reflect new position 
				gridX = newX;
				gridY = newY;

				DrawPlayer();  // redraw player after change

				//std::cout << "valid move" << std::endl;

				glBindBuffer(GL_ARRAY_BUFFER, vertexBufferPlayer);
				glBufferData(GL_ARRAY_BUFFER, verticesA.size() * sizeof(float), verticesA.data(), GL_DYNAMIC_DRAW);


			}

			
			if (C_x == T_x && C_y == T_y && treasureVisible) {
				//std::cout << "collision" << std::endl;
				//gameWon = !gameWon;
				gameWon = true;
				triggerTreasureSound();
				soundPlayed = true;

				createParticles(100, treasurePosition); // creates particles
			}
			
		}

		//std::cout << "Current grid position: (" << gridX << ", " << gridY << ")" << std::endl;	

	}

}



void keyCallbackCamera(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS || action == GLFW_REPEAT) {

		// Rotate around the X-axis
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			//cameraPosition.x -= 0.1f;  // left movement
			rotationAngleX -= 0.1f;
		}
		if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
			//cameraPosition.x += 0.1f;  // right movement
			rotationAngleX += 0.1f;
		}

		// Rotate around the Y-axis
		if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
			//cameraPosition.y -= 0.1f;  // down movement
			rotationAngleY -= 0.1f;
		}
		if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
			//cameraPosition.y += 0.1f;  // up movement
			rotationAngleY += 0.1f;
		}

		// Zoom in and out (move along the Z axis)
		if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS) {
			zoomFactor -= 0.1f;  // zoom in
		}
		if (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS) {
			zoomFactor += 0.1f;  // zoom out
		}

		// Move along the X-axis (panning)
		if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
			cameraPosition.x -= 0.1f;  // left movement
			
		}
		if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) {
			cameraPosition.x += 0.1f;  // right movement
		}

		// Move along the Y-axis (panning)
		if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
			cameraPosition.y += 0.1f;  // down movement			
		}
		if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
			cameraPosition.y -= 0.1f;  // up movement			
		}

		// Minimum zoom level limiter
		zoomFactor = std::max(zoomFactor, 1.0f);
		cameraPosition.z = zoomFactor;


	}
}


void keyCallbackLightSource(GLFWwindow* window, int key, int scancode, int action, int mods)
{


	if (action == GLFW_PRESS || action == GLFW_REPEAT) {

		// Rotate around the X-axis
		if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
			//cameraPosition.x -= 0.1f;  // left movement
			rotationAngleXLight -= 1.1f;
		}
		if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) {
			//cameraPosition.x += 0.1f;  // right movement
			rotationAngleXLight += 1.1f;
		}

		// Rotate around the Y-axis
		if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
			//cameraPosition.y -= 0.1f;  // down movement
			rotationAngleYLight -= 1.1f;
		}
		if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
			//cameraPosition.y += 0.1f;  // up movement
			rotationAngleYLight += 1.1f;
		}

		// Zoom in and out (move along the Z axis)
		if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
			zoomFactorLight -= 0.1f;  // zoom in
		}
		if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
			zoomFactorLight += 0.1f;  // zoom out
		}

		// Move along the X-axis (panning)
		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
			lightPos.x -= 0.1f;  // left movement

		}
		if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
			lightPos.x += 0.1f;  // right movement
		}

		// Move along the Y-axis (panning)
		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
			lightPos.y -= 0.1f;  // down movement			
		}
		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
			lightPos.y += 0.1f;  // up movement			
		}

		lightPos.z = zoomFactorLight;


	}
	
}


// This function is called by GLFW whenever a key is pressed.
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {

	// Handle player keys
	keyCallbackPlayer(window, key, scancode, action, mods);

	// Handle camera keys
	keyCallbackCamera(window, key, scancode, action, mods);

	// Handle light source keys
	keyCallbackLightSource(window, key, scancode, action, mods);

}



// loads and decodes the OGG file - uses stb_vorbis library
void loadTreasureSound(const char* filename) {
	
	FILE* file = nullptr;
	errno_t err = fopen_s(&file, filename, "rb");

	if (err != 0 || file == nullptr) {

		std::cerr << "Failed to open OGG file!" << std::endl;
		exit(1);

	}

	stb_vorbis* vorbis = stb_vorbis_open_file(file, NULL, NULL, NULL);

	if (!vorbis) {
		std::cerr << "Failed to open OGG file with stb_vorbis!" << std::endl;
		fclose(file);
		exit(1);
	}

	// Get the sample rate and number of channels
	stb_vorbis_info info = stb_vorbis_get_info(vorbis);
	sampleRate = info.sample_rate;
	channels = info.channels;

	// Decode the audio to a buffer
	int numFrames = stb_vorbis_stream_length_in_samples(vorbis);
	treasureAudioData.resize(numFrames * channels);

	int decoded = stb_vorbis_get_samples_short_interleaved(vorbis, channels, treasureAudioData.data(), treasureAudioData.size());
	
	if (decoded < 0) {
		std::cerr << "Failed to decode OGG file!" << std::endl;
		stb_vorbis_close(vorbis);
		fclose(file);
		exit(1);
	}

	stb_vorbis_close(vorbis);
	fclose(file);

}


// Initialize OpenAL
void initializeOpenAL() {

	// Open an audio device
	device = alcOpenDevice(nullptr);

	if (!device) {

		std::cerr << "Failed to open OpenAL device!" << std::endl;
		return;

	}

	// Create an OpenAL context
	context = alcCreateContext(device, nullptr);
	alcMakeContextCurrent(context);

	// Generate a buffer and load the preloaded audio data (only once)
	alGenBuffers(1, &buffer);
	alBufferData(buffer, (channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16,
		treasureAudioData.data(), treasureAudioData.size() * sizeof(short), sampleRate);

	// Generate a source (only once)
	alGenSources(1, &source);
	alSourcei(source, AL_BUFFER, buffer);
}

// cleanup OpenAL 
void cleanupOpenAL() {
	if (source != 0) {
		alDeleteSources(1, &source);
		source = 0;
	}
	if (buffer != 0) {
		alDeleteBuffers(1, &buffer);
		buffer = 0;
	}
	if (context != nullptr) {
		alcDestroyContext(context);
		context = nullptr;
	}
	if (device != nullptr) {
		alcCloseDevice(device);
		device = nullptr;
	}
}

// Function to trigger and play the sound
void triggerTreasureSound() {

	if (!soundPlayed) {
		// plays sound from audio buffer
		alSourcePlay(source);
	}
	
}


void createParticles(int numParticles, glm::vec3 origin) {
	particles.clear(); // Clear existing particles

	for (int i = 0; i < numParticles; ++i) {
		Particle p;
		p.position = origin; 

		// velocity range
		p.velocity = glm::vec3(
			static_cast<float>(rand() % 200 - 100) / 100.0f, // Random X velocity (-1 to +1)
			static_cast<float>(rand() % 200) / 100.0f,      // Random Y velocity (0 to 2)
			static_cast<float>(rand() % 200 - 100) / 100.0f  // Random Z velocity (-1 to +1)
		);

		p.lifetime = 10.0f; // Lasts for 10 seconds (not actually, 10, depends on delta time)
		particles.push_back(p);
	}
	//std::cout << "Particles created: " << particles.size() << std::endl;
}



void updateParticles(float deltaTime) {
	for (auto it = particles.begin(); it != particles.end(); ) {
		// Update position based on velocity and deltaTime
		it->position += it->velocity * 0.3f * deltaTime; // Position update

		// bind buffers to reflect the particles positions change
		glBindBuffer(GL_ARRAY_BUFFER, particlesVBO);
		glBufferData(GL_ARRAY_BUFFER, particles.size() * sizeof(Particle), particles.data(), GL_DYNAMIC_DRAW);

		// Apply gravity effect (particles fall)
		it->velocity.y -= 0.05f * deltaTime;

		// Reduce lifetime based on deltaTime
		it->lifetime -= 0.1 * deltaTime;

		// Remove dead particles
		if (it->lifetime <= 0.0f) {
			it = particles.erase(it);
			//std::cout << "Particle deleted" << std::endl;
		}
		else {
			++it;
		}

		
	}
}


void drawParticles() {

	glPointSize(10.0f); // Set point size for the particles
	glBegin(GL_POINTS); // Start drawing points
	

	for (const auto& p : particles) {
	
		glVertex3f(p.position.x, p.position.y, p.position.z); // Draw particle at its position

		//std::cout << "Particle position: " << p.position.x << ", " << p.position.y << ", " << p.position.z << std::endl;
	}

	glEnd();

}







int main(void)
{
	
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	// create 950 x 950 window
	window = glfwCreateWindow(950, 950, u8"Εργασία 1Γ – 2024 – Κυνήγι Θησαυρού", NULL, NULL);


	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	glfwSwapInterval(1); // enable vsync

	// Initialize GLEW
	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Set key callback
	glfwSetKeyCallback(window, keyCallback);

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// Black background color
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glEnable(GL_DEPTH_TEST);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders

	GLuint programID = LoadShaders("P1BVertexShader.vertexshader", "P1BFragmentShader.fragmentshader");

	GLuint MatrixID = glGetUniformLocation(programID, "MVP");

	GLuint LightMatrixID = glGetUniformLocation(programID, "lightMVP");

	GLuint lightPositionUniformLoc = glGetUniformLocation(programID, "lightPos");




	// Uv mapping for treasure object
	std::vector<GLfloat> uvDataForTreasure = {
		// Front face
		0.0f, 1.0f,  // Top-left
		0.0f, 0.0f,  // Bottom-left
		1.0f, 1.0f,  // Top-right

		0.0f, 0.0f,  // Bottom-left
		1.0f, 0.0f,  // Bottom-right
		1.0f, 1.0f,  // Top-right

		// Back face
		1.0f, 1.0f,  // Top-left
		1.0f, 0.0f,  // Bottom-left
		0.0f, 1.0f,  // Top-right

		1.0f, 0.0f,  // Bottom-left
		0.0f, 0.0f,  // Bottom-right
		0.0f, 1.0f,  // Top-right

		// Left face
		0.0f, 1.0f,  // Top-left
		0.0f, 0.0f,  // Bottom-left
		1.0f, 1.0f,  // Top-right

		0.0f, 0.0f,  // Bottom-left
		1.0f, 0.0f,  // Bottom-right
		1.0f, 1.0f,  // Top-right

		// Right face
		0.0f, 1.0f,  // Top-left
		0.0f, 0.0f,  // Bottom-left
		1.0f, 1.0f,  // Top-right

		0.0f, 0.0f,  // Bottom-left
		1.0f, 0.0f,  // Bottom-right
		1.0f, 1.0f,  // Top-right

		// Top face
		0.0f, 1.0f,  // Top-left
		1.0f, 1.0f,  // Top-right
		0.0f, 0.0f,  // Bottom-left

		1.0f, 1.0f,  // Top-right
		1.0f, 0.0f,  // Bottom-right
		0.0f, 0.0f,  // Bottom-left

		// Bottom face
		0.0f, 0.0f,  // Top-left
		1.0f, 0.0f,  // Top-right
		0.0f, 1.0f,  // Bottom-left

		1.0f, 0.0f,  // Top-right
		1.0f, 1.0f,  // Bottom-right
		0.0f, 1.0f,  // Bottom-left
	};


	// set normals for the maze
	std::vector<GLfloat> cubeNormalsForMaze;

	for (int i = 0; i < rows; ++i) {
		for (int j = 0; j < cols; ++j) {
			if (labyrinth[i][j] == 1) {
				// Add normals for bottom face (z = 0)
				for (int k = 0; k < 6; ++k) cubeNormalsForMaze.insert(cubeNormalsForMaze.end(), { 0.0f, 0.0f, -1.0f });

				// Add normals for top face (z = 1)
				for (int k = 0; k < 6; ++k) cubeNormalsForMaze.insert(cubeNormalsForMaze.end(), { 0.0f, 0.0f, 1.0f });

				// Add normals for front face
				for (int k = 0; k < 6; ++k) cubeNormalsForMaze.insert(cubeNormalsForMaze.end(), { 0.0f, 1.0f, 0.0f });

				// Add normals for back face
				for (int k = 0; k < 6; ++k) cubeNormalsForMaze.insert(cubeNormalsForMaze.end(), { 0.0f, -1.0f, 0.0f });

				// Add normals for left face
				for (int k = 0; k < 6; ++k) cubeNormalsForMaze.insert(cubeNormalsForMaze.end(), { -1.0f, 0.0f, 0.0f });

				// Add normals for right face
				for (int k = 0; k < 6; ++k) cubeNormalsForMaze.insert(cubeNormalsForMaze.end(), { 1.0f, 0.0f, 0.0f });
			}
		}
	}

	// set normals for the player and the treasure
	std::vector<GLfloat> cubeNormals = {
		// Front face
	0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,
	0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,

	// Back face
	0.0f, 0.0f, -1.0f,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f, -1.0f,
	0.0f, 0.0f, -1.0f,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f, -1.0f,

	// Left face
	-1.0f, 0.0f, 0.0f,  -1.0f, 0.0f, 0.0f,  -1.0f, 0.0f, 0.0f,
	-1.0f, 0.0f, 0.0f,  -1.0f, 0.0f, 0.0f,  -1.0f, 0.0f, 0.0f,

	// Right face
	1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,
	1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,

	// Top face
	0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,
	0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,

	// Bottom face
	0.0f, -1.0f, 0.0f,  0.0f, -1.0f, 0.0f,  0.0f, -1.0f, 0.0f,
	0.0f, -1.0f, 0.0f,  0.0f, -1.0f, 0.0f,  0.0f, -1.0f, 0.0f,
	};
	

	
	std::vector<GLfloat> lightCubeVertices = {
		// Positions         // Colors           // Normals
		// Back face (red)
		-2.0f, -2.0f, -2.0f,  1.0f, 0.0f, 0.0f,  0.0f,  0.0f, -1.0f,
		 2.0f, -2.0f, -2.0f,  1.0f, 0.0f, 0.0f,  0.0f,  0.0f, -1.0f,
		 2.0f,  2.0f, -2.0f,  1.0f, 0.0f, 0.0f,  0.0f,  0.0f, -1.0f,
		-2.0f,  2.0f, -2.0f,  1.0f, 0.0f, 0.0f,  0.0f,  0.0f, -1.0f,

		// Front face (green)
		-2.0f, -2.0f,  2.0f,  0.0f, 1.0f, 0.0f,  0.0f,  0.0f,  1.0f,
		 2.0f, -2.0f,  2.0f,  0.0f, 1.0f, 0.0f,  0.0f,  0.0f,  1.0f,
		 2.0f,  2.0f,  2.0f,  0.0f, 1.0f, 0.0f,  0.0f,  0.0f,  1.0f,
		-2.0f,  2.0f,  2.0f,  0.0f, 1.0f, 0.0f,  0.0f,  0.0f,  1.0f,

		// Bottom face (blue)
		-2.0f, -2.0f, -2.0f,  0.0f, 0.0f, 1.0f,  0.0f, -1.0f,  0.0f,
		 2.0f, -2.0f, -2.0f,  0.0f, 0.0f, 1.0f,  0.0f, -1.0f,  0.0f,
		 2.0f, -2.0f,  2.0f,  0.0f, 0.0f, 1.0f,  0.0f, -1.0f,  0.0f,
		-2.0f, -2.0f,  2.0f,  0.0f, 0.0f, 1.0f,  0.0f, -1.0f,  0.0f,

		// Top face (yellow)
		-2.0f,  2.0f, -2.0f,  1.0f, 1.0f, 0.0f,  0.0f,  1.0f,  0.0f,
		 2.0f,  2.0f, -2.0f,  1.0f, 1.0f, 0.0f,  0.0f,  1.0f,  0.0f,
		 2.0f,  2.0f,  2.0f,  1.0f, 1.0f, 0.0f,  0.0f,  1.0f,  0.0f,
		-2.0f,  2.0f,  2.0f,  1.0f, 1.0f, 0.0f,  0.0f,  1.0f,  0.0f,

		// Left face (purple)
		-2.0f, -2.0f, -2.0f,  1.0f, 0.0f, 1.0f, -1.0f,  0.0f,  0.0f,
		-2.0f,  2.0f, -2.0f,  1.0f, 0.0f, 1.0f, -1.0f,  0.0f,  0.0f,
		-2.0f,  2.0f,  2.0f,  1.0f, 0.0f, 1.0f, -1.0f,  0.0f,  0.0f,
		-2.0f, -2.0f,  2.0f,  1.0f, 0.0f, 1.0f, -1.0f,  0.0f,  0.0f,

		// Right face (cyan)
		 2.0f, -2.0f, -2.0f,  0.0f, 1.0f, 1.0f,  1.0f,  0.0f,  0.0f,
		 2.0f,  2.0f, -2.0f,  0.0f, 1.0f, 1.0f,  1.0f,  0.0f,  0.0f,
		 2.0f,  2.0f,  2.0f,  0.0f, 1.0f, 1.0f,  1.0f,  0.0f,  0.0f,
		 2.0f, -2.0f,  2.0f,  0.0f, 1.0f, 1.0f,  1.0f,  0.0f,  0.0f,
	};
	


	std::vector<GLuint> lightCubeIndices = {
		// Back face
		0, 1, 2,  2, 3, 0,

		// Front face
		4, 5, 6,  6, 7, 4,

		// Bottom face
		8, 9, 10,  10, 11, 8,

		// Top face
		12, 13, 14,  14, 15, 12,

		// Left face
		16, 17, 18,  18, 19, 16,

		// Right face
		20, 21, 22,  22, 23, 20,
	};



	// initialize random seed for random treasure pos
	// by using current time it makes sure that
	// the seed is different every time, since 
	// time changes every second
	std::srand(static_cast<unsigned int>(time(nullptr)));
	

	// draw maze 
	generateMazeVertices();


	// draw player
	DrawPlayer();


	// draw treasure
	DrawTreasure();


	// randomize treasure position at the start
	moveTreasureToRandomPosition();


	// load treasure sound data 
	loadTreasureSound("boop.ogg");


	// initialize OpenAL 
	initializeOpenAL();



	std::vector<GLfloat> mazeColors(vertices.size() / 3 * 4, 0.0f);
	// GLfloat array size = total num of vertices (vertices_size / 3) times 4 (color chanels)


	// Define blue color for each maze vertex

	for (size_t i = 0; i < mazeColors.size(); i += 4) {
		mazeColors[i] = 0.0f;       // Red 
		mazeColors[i + 1] = 0.0f;   // Green 
		mazeColors[i + 2] = 1.0f;   // Blue
		mazeColors[i + 3] = 1.0f;   // Alpha 
	}

	std::vector<GLfloat> mazeColorsA(verticesA.size() / 3 * 4, 0.0f);
	// GLfloat array size = total num of verticesA (verticesA_size / 3) times 4 (color chanels)


	// Define yellow color for each player vertex

	for (size_t i = 0; i < mazeColorsA.size(); i += 4) {
		mazeColorsA[i] = 1.0f;       // Red
		mazeColorsA[i + 1] = 1.0f;   // Green
		mazeColorsA[i + 2] = 0.0f;   // Blue
		mazeColorsA[i + 3] = 1.0f;   // Alpha
	}

	// Define cyan color for each treasure vertex

	std::vector<GLfloat> mazeColorsT(verticesT.size() / 3 * 4, 0.0f);
	// GLfloat array size = total num of verticesT (verticesT_size / 3) times 4 (color chanels)

	for (size_t i = 0; i < mazeColorsT.size(); i += 4) {
		mazeColorsT[i] = 0.0f;       // Red
		mazeColorsT[i + 1] = 1.0f;   // Green
		mazeColorsT[i + 2] = 1.0f;   // Blue
		mazeColorsT[i + 3] = 1.0f;   // Alpha
	}

	// Generate and bind buffer for the maze
	glGenBuffers(1, &vertexBufferMaze);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferMaze);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);


	// Generate and bind buffer for the player
	glGenBuffers(1, &vertexBufferPlayer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferPlayer);
	glBufferData(GL_ARRAY_BUFFER, verticesA.size() * sizeof(float), verticesA.data(), GL_STATIC_DRAW);


	// Generate and bind buffer for the treasure
	glGenBuffers(1, &vertexBufferTreasure);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferTreasure);
	glBufferData(GL_ARRAY_BUFFER, verticesT.size() * sizeof(float), verticesT.data(), GL_STATIC_DRAW);


	// Generate and bind buffer for the particles
	glGenBuffers(1, &particlesVBO);
	glBindBuffer(GL_ARRAY_BUFFER, particlesVBO);
	glBufferData(GL_ARRAY_BUFFER, particles.size() * sizeof(Particle), particles.data(), GL_STATIC_DRAW);
	 


	// Generate and bind the color buffer for the maze
	GLuint colorBufferMaze;
	glGenBuffers(1, &colorBufferMaze);
	glBindBuffer(GL_ARRAY_BUFFER, colorBufferMaze);
	glBufferData(GL_ARRAY_BUFFER, mazeColors.size() * sizeof(GLfloat), mazeColors.data(), GL_STATIC_DRAW);


	// Generate and bind the color buffer for the player
	GLuint colorBufferPlayer;
	glGenBuffers(1, &colorBufferPlayer);
	glBindBuffer(GL_ARRAY_BUFFER, colorBufferPlayer);
	glBufferData(GL_ARRAY_BUFFER, mazeColorsA.size() * sizeof(GLfloat), mazeColorsA.data(), GL_STATIC_DRAW);


	// Generate and bind the color buffer for the treasure
	GLuint colorBufferTreasure;
	glGenBuffers(1, &colorBufferTreasure);
	glBindBuffer(GL_ARRAY_BUFFER, colorBufferTreasure);
	glBufferData(GL_ARRAY_BUFFER, mazeColorsT.size() * sizeof(GLfloat), mazeColorsT.data(), GL_STATIC_DRAW);

	
	// Generate and bind the color buffer for the particles
	GLuint particlesVAO;
	glGenVertexArrays(1, &particlesVAO);
	glBindBuffer(GL_ARRAY_BUFFER, particlesVAO);
	glBindVertexArray(particlesVAO);
	


	// Generate and bind the uv buffer for the treasure
	GLuint uvBufferTreasure;
	glGenBuffers(1, &uvBufferTreasure);
	glBindBuffer(GL_ARRAY_BUFFER, uvBufferTreasure);
	glBufferData(GL_ARRAY_BUFFER, uvDataForTreasure.size() * sizeof(GLfloat), uvDataForTreasure.data(), GL_STATIC_DRAW);

	
	// Generate and bind buffers for the light cube

	GLuint lightVBO, lightVAO, lightEBO;
	glGenVertexArrays(1, &lightVAO);
	glGenBuffers(1, &lightEBO);
	glGenBuffers(1, &lightVBO);

	glBindVertexArray(lightVAO);

	glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
	glBufferData(GL_ARRAY_BUFFER, lightCubeVertices.size() * sizeof(GLfloat), lightCubeVertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lightEBO); // Bind index buffer;
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, lightCubeIndices.size() * sizeof(GLuint), lightCubeIndices.data(), GL_STATIC_DRAW);

	


	// Generate and bind the buffer for the maze normals
	GLuint normalBufferMaze; // Normal buffer
	glGenBuffers(1, &normalBufferMaze);
	glBindBuffer(GL_ARRAY_BUFFER, normalBufferMaze);
	glBufferData(GL_ARRAY_BUFFER, cubeNormalsForMaze.size() * sizeof(GLuint), cubeNormalsForMaze.data(), GL_STATIC_DRAW);


	// Generate and bind the buffer for the player normals
	GLuint normalBufferPlayer; // Normal buffer
	glGenBuffers(1, &normalBufferPlayer);
	glBindBuffer(GL_ARRAY_BUFFER, normalBufferPlayer);
	glBufferData(GL_ARRAY_BUFFER, cubeNormals.size() * sizeof(GLuint), cubeNormals.data(), GL_STATIC_DRAW);


	// Generate and bind the buffer for the treasure normals
	GLuint normalBufferTreasure; // Normal buffer
	glGenBuffers(1, &normalBufferTreasure);
	glBindBuffer(GL_ARRAY_BUFFER, normalBufferTreasure);
	glBufferData(GL_ARRAY_BUFFER, cubeNormals.size() * sizeof(GLuint), cubeNormals.data(), GL_STATIC_DRAW);


	
	double deltaTime = calculateDeltaTime();
	
	

	do {


		//std::cout << "DeltaTime: " << deltaTime << std::endl;
		
		updateTreasurePosition();  // update treasure position every 2 seconds


		updateParticles(deltaTime);
		drawParticles();


		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Enable depth tests
		glEnable(GL_DEPTH_TEST);



		// use our shader
		glUseProgram(programID);




		// For camera movement 


		// Projection matrix
		glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 4.0f / 4.0f, 0.1f, 100.0f);


		// Update the view matrix based on the current position of the camera

		// View matrix
		glm::mat4 View = glm::lookAt(
			cameraPosition,               // camera position
			glm::vec3(0.0f, 0.0f, 0.25f), // looks towards the center of the labyrinth
			glm::vec3(0.0f, 1.0f, 0.0f)   // up vector
		);

		// Model matrix
		glm::mat4 Model = glm::mat4(1.0f);

		View = glm::rotate(View, rotationAngleX, glm::vec3(0.5f, 0.0f, 0.0f)); // rotate around X 
		View = glm::rotate(View, rotationAngleY, glm::vec3(0.0f, 0.5f, 0.0f)); // rotate around Y


		// Calculate the MVP matrix
		glm::mat4 MVP = Projection * View * Model;

		// Pass MVP matrix to shader
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);




		// calculate the normal matrix
		glm::mat4 modelMatrix = glm::mat4(1.0f);
		glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));
		glUniformMatrix3fv(glGetUniformLocation(programID, "normalMatrix"), 1, GL_FALSE, &normalMatrix[0][0]);


		/*
		// Print the current camera position
		std::cout << "Camera Position: ("
			<< cameraPosition.x << ", "
			<< cameraPosition.y << ", "
			<< cameraPosition.z << ")" << std::endl;
		*/


		// Set and pass light cube attributes to the shader


		// Set the light cube position
		glUniform3fv(glGetUniformLocation(programID, "lightPos"), 1, &lightPos[0]);


		// Set the light color 
		glm::vec3 lightColor(1.0f, 1.0f, 1.0f); // White light
		glUniform3fv(glGetUniformLocation(programID, "lightColor"), 1, &lightColor[0]);


		// Set the camera (view) position
		glm::vec3 viewPos(cameraPosition.x, cameraPosition.y, cameraPosition.z);
		glUniform3fv(glGetUniformLocation(programID, "viewPos"), 1, &viewPos[0]);


		// Set shininess
		glUniform1f(glGetUniformLocation(programID, "shininess"), 32.0f); // Example value


		// Set ambient strength
		glUniform1f(glGetUniformLocation(programID, "ambienceStrength"), 0.6f); // Example value


		// Enable lighting
		glUniform1i(glGetUniformLocation(programID, "useLighting"), 1);


		glUniform1i(glGetUniformLocation(programID, "visualizeNormals"), 0);


		glUniform1i(glGetUniformLocation(programID, "useTexture"), 0); // Disable texture


		//glUniform1i(isLightObjectLoc, 1); 
		//glUniform1i(glGetUniformLocation(programID, "useLighting"), 1); // No lighting for now



		// Draw maze walls
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferMaze);
		glVertexAttribPointer(
			0,
			3,
			GL_FLOAT,
			GL_FALSE,
			0,
			(void*)0
		);
		
		// Enable the color attribute array (index 1 in shader)
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, colorBufferMaze);
		glVertexAttribPointer(
			1,            // Attribute location in shader
			4,            // Size of each color component (RGBA)
			GL_FLOAT,     // Type of each component
			GL_FALSE,     // Don't normalize
			0,            // Stride
			(void*)0      // Offset
		);
		
		
		// Enable the normals attribute array (index 3 in shader)
		glEnableVertexAttribArray(3);
		glBindBuffer(GL_ARRAY_BUFFER, normalBufferMaze);
		glVertexAttribPointer(
			3,          // Normal attribute location
			3,          // Normal components (x, y, z)
			GL_FLOAT,   // Type of each normal component
			GL_FALSE,   // Don't normalize
			0,          // Stride
			(void*)0    // Offset
		);

		
		/*
		// Send normal matrix to shader
		GLuint normalMatrixLoc = glGetUniformLocation(programID, "normalMatrix");
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, &normalMatrix[0][0]);


		// Send MVP matrix to shader
		GLuint mvpLoc = glGetUniformLocation(programID, "MVP");
		glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, &MVP[0][0]);
		*/


		// Draw the maze vertices
		glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 3);


		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(3);

		//glUniform1i(glGetUniformLocation(programID, "useLighting"), 1); // No lighting for now



		// Draw player
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferPlayer);
		glVertexAttribPointer(
			0,
			3,
			GL_FLOAT,
			GL_FALSE,
			0,
			(void*)0
		);

		// Enable the color attribute array (index 1 in shader)
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, colorBufferPlayer);
		glVertexAttribPointer(
			1,            // Attribute location in shader
			4,            // Size of each color component (RGBA)
			GL_FLOAT,     // Type of each component
			GL_FALSE,     // Don't normalize
			0,            // Stride
			(void*)0      // Offset
		);
		
		// Enable the normal attribute array (index 3 in shader)
		glEnableVertexAttribArray(3);
		glBindBuffer(GL_ARRAY_BUFFER, normalBufferPlayer);
		glVertexAttribPointer(
			3,            // Attribute location in shader
			3,            // Size of each color component (RGBA)
			GL_FLOAT,     // Type of each component
			GL_FALSE,     // Don't normalize
			0,            // Stride
			(void*)0      // Offset
		);

		

		glDrawArrays(GL_TRIANGLES, 0, verticesA.size() / 3);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(3);


		// Since texture is enabled for treasure object then color buffer will be ignored
		glUniform1i(glGetUniformLocation(programID, "useTexture"), 1);		 // Enable texture (set bool to true)

		// Draw treasure
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferTreasure);
		glVertexAttribPointer(
			0,
			3,
			GL_FLOAT,
			GL_FALSE,
			0,
			(void*)0
		);
		
		// Enable the color attribute array (index 1 in shader)
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, colorBufferTreasure);
		glVertexAttribPointer(
			1,            // Attribute location in shader
			4,            // Size of each color component (RGBA)
			GL_FLOAT,     // Type of each component
			GL_FALSE,     // Don't normalize
			0,            // Stride
			(void*)0      // Offset
		);
		
		// Enable the texture attribute array (index 2 in shader)
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, uvBufferTreasure);
		glVertexAttribPointer(
			2,            // Attribute location in shader
			2,            // Size of each color component (RGBA)
			GL_FLOAT,     // Type of each component
			GL_FALSE,     // Don't normalize
			0,            // Stride
			(void*)0      // Offset
		);

		// Enable the normal attribute array (index 3 in shader)
		glEnableVertexAttribArray(3);
		glBindBuffer(GL_ARRAY_BUFFER, normalBufferTreasure);
		glVertexAttribPointer(
			3,            // Attribute location in shader
			3,            // Size of each color component (RGBA)
			GL_FLOAT,     // Type of each component
			GL_FALSE,     // Don't normalize
			0,            // Stride
			(void*)0      // Offset
		);


		glDrawArrays(GL_TRIANGLES, 0, verticesT.size() / 3);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
		glDisableVertexAttribArray(3);


		
		
		glUniform1i(glGetUniformLocation(programID, "useTexture"), 0); // Disable texture
		

		// Draw particles
		glBindBuffer(GL_ARRAY_BUFFER, particlesVBO);
		glEnableVertexAttribArray(0);

		// Position attribute
		glVertexAttribPointer(
			0,
			3,
			GL_FLOAT,
			GL_FALSE,
			0,
			(void*)0
		);
		
		
		// Color attribute
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, particlesVAO);
		glVertexAttribPointer(
			1,
			3,
			GL_FLOAT,
			GL_FALSE,
			0,
			(void*)0
		);
		

		glDrawArrays(GL_POINTS, 0, particles.size()); // Draw as points

		glDisableVertexAttribArray(0);
	    glDisableVertexAttribArray(1);


		
		// light Cube Model matrix (for translation, rotation, and scaling)
		glm::mat4 lightCubeModel = glm::mat4(1.0f);

		// calculate relative position to the rotation point (center)
		glm::vec3 offsetPosition = lightPos - rotationPoint;

		// apply rotations around the rotation point
		glm::mat4 rotationMatrix = glm::mat4(1.0f);
		rotationMatrix = glm::rotate(rotationMatrix, glm::radians(rotationAngleXLight), glm::vec3(1.0f, 0.0f, 0.0f)); // X-axis
		rotationMatrix = glm::rotate(rotationMatrix, glm::radians(rotationAngleYLight), glm::vec3(0.0f, 1.0f, 0.0f)); // Y-axis
		glm::vec4 rotatedOffsetPosition = rotationMatrix * glm::vec4(offsetPosition, 1.0f);

		// compute the lights final world position
		glm::vec3 finalPosition = rotationPoint + glm::vec3(rotatedOffsetPosition);


		// apply translation (for movement) to place the light cube
		lightCubeModel = glm::translate(lightCubeModel, finalPosition);

		// apply scaling
		lightCubeModel = glm::scale(lightCubeModel, glm::vec3(0.2f)); // Scale light cube

		// calculate MVP matrix for light cube
		glm::mat4 lightCubeMVP = Projection * View * lightCubeModel;

		// pass the light cubes MVP matrix to the shader
		GLuint lightCubeMatrixID = glGetUniformLocation(programID, "MVP");
		glUniformMatrix4fv(lightCubeMatrixID, 1, GL_FALSE, &lightCubeMVP[0][0]);

		// pass the final world position to the lighting shader
		glUniform3fv(lightPositionUniformLoc, 1, &finalPosition[0]); // Use finalPosition for lighting

		

		// Draw light cube (light source)
		
		glBindVertexArray(lightVAO); // Bind VAO
		glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lightEBO);

		// layout for vertex positions (location = 0)
		glVertexAttribPointer(
			0,
			3,
			GL_FLOAT,
			GL_FALSE,
			9 * sizeof(GLfloat),
			(void*)0
		);
		glEnableVertexAttribArray(0);

		// layout for vertex colors (location = 1)
		glVertexAttribPointer(
			1,
			3,
			GL_FLOAT,
			GL_FALSE,
			9 * sizeof(GLfloat),
			(GLvoid*)(3 * sizeof(GLfloat))
		);
		glEnableVertexAttribArray(1);

		// layout for vertex normals (location = 3)
		glVertexAttribPointer(3,
			3,
			GL_FLOAT,
			GL_FALSE,
			9 * sizeof(GLfloat),
			(GLvoid*)(3 * sizeof(GLfloat))
		);
		glEnableVertexAttribArray(3);


		// Draw the cube
		glDrawElements(GL_TRIANGLES, lightCubeIndices.size(), GL_UNSIGNED_INT, 0);

		//glBindVertexArray(0); // Unbind VAO

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(3);




		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();


	} // Check if the SPACE key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_SPACE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	// Cleanup VBO
	glDeleteBuffers(1, &vertexBufferMaze);
	glDeleteBuffers(1, &vertexBufferPlayer);
	glDeleteBuffers(1, &vertexBufferTreasure);
	glDeleteBuffers(1, &colorBufferMaze);
	glDeleteBuffers(1, &colorBufferPlayer);
	glDeleteBuffers(1, &colorBufferTreasure);
	glDeleteBuffers(1, &uvBufferTreasure);
	glDeleteBuffers(1, &particlesVBO);
	glDeleteBuffers(1, &particlesVAO);
	glDeleteBuffers(1, &normalBufferMaze);
	glDeleteBuffers(1, &normalBufferPlayer);
	glDeleteBuffers(1, &normalBufferTreasure);

	glDeleteProgram(programID);

	glDeleteVertexArrays(1, &VertexArrayID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;

}


