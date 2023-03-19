/* 
Michael Salton
251175125
Assignment 4 - Link's House
CS3388
Alexander Brandt 
*/

// Standard Stuff
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

// Exotic stuff
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Screen width and height
const float WIDTH = 800.0f;
const float HEIGHT = 600.0f;

glm::vec3 position = {0.5f, 0.4f, 0.5f};
glm::vec3 front = {0.0f, 1.0f, 0.0f};
glm::vec3 center = {0.0f, 0.0f, -1.0f};


// Vertex structure
struct Vertex{
	float x, y, z;
	float nx, ny, nz;
	float u, v;
};


// Face structure
struct Face{
	glm::ivec3 indices;
};

// Keyboard input
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    glm::vec3 moveDir;
	float cameraSpeed = 0.05f;
	float cameraRotationSpeed = 0.005f;

    if ((key == GLFW_KEY_UP || key == GLFW_KEY_W) && (action == GLFW_PRESS || action == GLFW_REPEAT)) 
		moveDir += cameraSpeed * center;
    if ((key == GLFW_KEY_DOWN || key == GLFW_KEY_S) && (action == GLFW_PRESS || action == GLFW_REPEAT)) 
		moveDir -= cameraSpeed * center;
    if ((key == GLFW_KEY_LEFT || key == GLFW_KEY_A) && (action == GLFW_PRESS || action == GLFW_REPEAT))
        center = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(3.0f), front) * glm::vec4(center, 0.0f));
    if ((key == GLFW_KEY_RIGHT || key == GLFW_KEY_D) && (action == GLFW_PRESS || action == GLFW_REPEAT))
        center = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(-3.0f), front) * glm::vec4(center, 0.0f));
	position += moveDir;
}


// Load the textures
void loadARGB_BMP(const char* imagepath, unsigned char** data, unsigned int* width, unsigned int* height) {

    printf("Reading image %s\n", imagepath);

    // Data read from the header of the BMP file
    unsigned char header[54];
    unsigned int dataPos;
    unsigned int imageSize;
    // Actual RGBA data

    // Open the file
    FILE * file = fopen(imagepath,"rb");
    if (!file){
        printf("%s could not be opened. Are you in the right directory?\n", imagepath);
        getchar();
        return;
    }

    // Read the header, i.e. the 54 first bytes

    // If less than 54 bytes are read, problem
    if ( fread(header, 1, 54, file)!=54 ){
        printf("Not a correct BMP file1\n");
        fclose(file);
        return;
    }

    // Read the information about the image
    dataPos    = *(int*)&(header[0x0A]);
    imageSize  = *(int*)&(header[0x22]);
    *width      = *(int*)&(header[0x12]);
    *height     = *(int*)&(header[0x16]);
    // A BMP files always begins with "BM"
    if ( header[0]!='B' || header[1]!='M' ){
        printf("Not a correct BMP file2\n");
        fclose(file);
        return;
    }
    // Make sure this is a 32bpp file
    if ( *(int*)&(header[0x1E])!=3  ) {
        printf("Not a correct BMP file3\n");
        fclose(file);
        return;
    }
    // fprintf(stderr, "header[0x1c]: %d\n", *(int*)&(header[0x1c]));
    // if ( *(int*)&(header[0x1C])!=32 ) {
    //     printf("Not a correct BMP file4\n");
    //     fclose(file);
    //     return;
    // }

    // Some BMP files are misformatted, guess missing information
    if (imageSize==0)    imageSize=(*width)* (*height)*4; // 4 : one byte for each Red, Green, Blue, Alpha component
    if (dataPos==0)      dataPos=54; // The BMP header is done that way

    // Create a buffer
    *data = new unsigned char [imageSize];

    if (dataPos != 54) {
        fread(header, 1, dataPos - 54, file);
    }

    // Read the actual data from the file into the buffer
    fread(*data,1,imageSize,file);

    // Everything is in memory now, the file can be closed.
    fclose (file);
}

// Read the PLY fikes
void readPLYFile(std::string fname, std::vector<Vertex>& vertices, std::vector<Face>& faces) {
	std::ifstream plyFile(fname);
	std::string line;
	std::stringstream stream;
	bool normals = false, coords = false, verts = false, onFaces = false;
	int numVertices = 0, numFaces = 0, cordIndex = 0, normIndex = 0, uIndex = 0, currIndex = 0;

    if (!plyFile.is_open()) {
        std::cerr << "Error: " << fname << std::endl;
        return;
    }
	
	// Parse throguh lines
    while (getline(plyFile, line)) {
		if(line.find("element vertex") != std::string::npos) {
			stream.clear();
			stream << line;
			stream.ignore(14);
			stream >> numVertices;
		}
		else if(line.find("element face") != std::string::npos) {
			stream.clear();
			stream << line;
			stream.ignore(12);
			stream >> numFaces;
		}
		if(line.find("property float x") != std::string::npos) {
			cordIndex = currIndex;
			currIndex++;
			verts = true;
		}
		else if(line.find("property float nx") != std::string::npos) {
			normIndex = currIndex;
			currIndex++;
			normals = true;
		}
		else if(line.find("property float u") != std::string::npos) {
			uIndex = currIndex;
			currIndex++;
			coords = true;
		}
		else if(line.find("end_header") != std::string::npos) {
			break;
		}
	}

	for (int i = 0; i < numVertices; i++) {
		Vertex vertex;
		getline(plyFile, line);
		stream.clear();
		stream << line;
		int index = 0;

		for (int j = 0; j < 5; j++) {
			if (verts == true && index == cordIndex) stream >> vertex.x >> vertex.y >> vertex.z;
			if (normals == true && index == normIndex) stream >> vertex.nx >> vertex.ny >> vertex.nz;
			if (coords == true && index == uIndex) stream >> vertex.u >> vertex.v;
			index++;
		}
		vertices.push_back(vertex);
	}

	for (int i = 0; i < numFaces; i++) {
		Face face;
		getline(plyFile, line);
		stream.clear();
		stream << line;
		int numVertsInFace;
		stream >> numVertsInFace;
		if (numVertsInFace == 3) {
			Face face;
			int v1, v2, v3;
			stream >> v1 >> v2 >> v3;
			glm::ivec3 list = {v1, v2, v3};
			face.indices = list;
			faces.push_back(face);
		}
	}
}

// Render the textures
class TexturedMesh{
public:
  TexturedMesh(const std::string &plyFilePath, const std::string &bmpFilePath)
  {
    readPLYFile(plyFilePath, vertices, faces);
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    std::string VertexShaderCode = "\
            #version 330 core\n\
            // Input vertex data, different for all executions of this shader.\n\
            layout(location = 0) in vec3 vertexPosition;\n\
            layout(location = 1) in vec2 uv;\n\
            // Output data ; will be interpolated for each fragment.\n\
            out vec2 uv_out;\n\
            // Values that stay constant for the whole mesh.\n\
            uniform mat4 MVP;\n\
            void main(){ \n\
                // Output position of the vertex, in clip space : MVP * position\n\
                gl_Position =  MVP * vec4(vertexPosition,1);\n\
                // The color will be interpolated to produce the color of each fragment\n\
                uv_out = uv;\n\
            }\n";
    std::string FragmentShaderCode = "\
            #version 330 core\n\
            in vec2 uv_out; \n\
            uniform sampler2D tex;\n\
            void main() {\n\
                gl_FragColor = texture(tex, uv_out);\n\
            }\n";
    char const *VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
    glCompileShader(VertexShaderID);
    char const *FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
    glCompileShader(FragmentShaderID);
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, VertexShaderID);
    glAttachShader(shaderProgram, FragmentShaderID);
    glLinkProgram(shaderProgram);
    glDetachShader(shaderProgram, VertexShaderID);
    glDetachShader(shaderProgram, FragmentShaderID);
    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

	unsigned char* imageData;
	GLuint width, height;
    loadARGB_BMP(bmpFilePath.c_str(), &imageData, &width, &height);

    glGenTextures(1, &bitmapImage);
    glBindTexture(GL_TEXTURE_2D, bitmapImage);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, imageData);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenVertexArrays(1, &vaoMesh);
    glGenBuffers(1, &vboVertexPosition);
    glGenBuffers(1, &vboTextureCoords);
    glGenBuffers(1, &vboFaceIndices);
    glBindVertexArray(vaoMesh);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboFaceIndices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * sizeof(Face), faces.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, vboVertexPosition);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *)0);
    glBindBuffer(GL_ARRAY_BUFFER, vboTextureCoords);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *)(offsetof(Vertex, u)));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }
  void render(const glm::mat4 &MVP) const{
    GLuint MatrixID = glGetUniformLocation(shaderProgram, "MVP");
    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, bitmapImage);
    glUseProgram(shaderProgram);
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, value_ptr(MVP));
    glBindVertexArray(vaoMesh);
    glDrawElements(GL_TRIANGLES, faces.size() * 3, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glUseProgram(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
  }
private:
	std::vector<Vertex> vertices;
	std::vector<Face> faces;
	GLuint vboVertexPosition;
	GLuint vboTextureCoords;
	GLuint vboFaceIndices;
	GLuint bitmapImage;
	GLuint vaoMesh;
	GLuint shaderProgram;
};

int main(){
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Wild Breathe Simulator", NULL, NULL);
	glfwMakeContextCurrent(window);
	glewInit();
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetKeyCallback(window, keyCallback);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glClearColor(0.2f, 0.2f, 0.3f, 0.0f);

	std::vector<TexturedMesh> meshList;
	meshList.push_back(TexturedMesh("LinksHouse/Floor.ply", "LinksHouse/floor.bmp"));
	meshList.push_back(TexturedMesh("LinksHouse/Walls.ply", "LinksHouse/walls.bmp"));
	meshList.push_back(TexturedMesh("LinksHouse/Table.ply", "LinksHouse/table.bmp"));
	meshList.push_back(TexturedMesh("LinksHouse/Patio.ply", "LinksHouse/patio.bmp"));
	meshList.push_back(TexturedMesh("LinksHouse/Bottles.ply", "LinksHouse/bottles.bmp"));
	meshList.push_back(TexturedMesh("LinksHouse/WoodObjects.ply", "LinksHouse/woodobjects.bmp"));
	meshList.push_back(TexturedMesh("LinksHouse/MetalObjects.ply", "LinksHouse/metalobjects.bmp"));
	meshList.push_back(TexturedMesh("LinksHouse/WindowBG.ply", "LinksHouse/windowbg.bmp"));
	meshList.push_back(TexturedMesh("LinksHouse/DoorBG.ply", "LinksHouse/doorbg.bmp"));
	meshList.push_back(TexturedMesh("LinksHouse/Curtains.ply", "LinksHouse/curtains.bmp"));

	glm::mat4 projection = glm::perspective(glm::radians(45.0f), WIDTH/HEIGHT, 0.001f, 1000.0f);

	do{
		glLoadMatrixf(glm::value_ptr(projection));
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glm::mat4 view = glm::lookAt(position, position + center, front); 
		glm::mat4 matrix = glm::mat4(1.0f);
		glm::mat4 MVP = projection * view * matrix;
		glLoadMatrixf(glm::value_ptr(view));
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		for (TexturedMesh& mesh : meshList) {
			mesh.render(MVP);
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	while(glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0);

	glfwTerminate();
	return 0;
}
