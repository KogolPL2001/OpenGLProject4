#pragma warning( disable : 26495)
#pragma warning( disable : 26451)
#pragma warning(disable:4996)
#define STB_IMAGE_IMPLEMENTATION
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <deps/linmath.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <headers/shader_m.h>
#include <headers/camera.h>
#include <stb-master/stb_image.h>

#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// lighting
int moveCameraOrLight = 0;
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

//window
GLFWwindow* window;

struct Vertex {
    glm::vec3 pos;
    glm::vec2 texture;
    glm::vec3 normals;
};

struct Face {
    unsigned int v;
    unsigned int vt;
    unsigned int vn;
};

struct Material {
    char name[128];
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;
};

void frame();

void processObj();


std::vector<int> ind;
std::vector<Vertex> vertices;


GLuint VBO, VAO, EBO, TEX;

Shader lightObjectShader("shader.vs", "shader.fs");
Shader lambertShader("lambert.vs", "lambert.fs");

char objPath[12] = "monkey.obj";


void frame() {
    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    processInput(window);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, TEX);



    // view/projection transformations
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();


    // world transformation
    glm::mat4 model = glm::mat4(1.0f);

    lambertShader.use();
    lambertShader.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
    lambertShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
    lambertShader.setVec3("lightPos", lightPos);
    lambertShader.setMat4("projection", projection);
    lambertShader.setMat4("view", view);
    //model = glm::translate(model, glm::vec3(3.0f, 0.0f, 0.0f));
    lambertShader.setMat4("model", model);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, ind.size(), GL_UNSIGNED_INT,0);
    glBindVertexArray(0);
}
int main() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Tutorial 07 - Model Loading", NULL, NULL);
    if (window == NULL) {
        fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
        getchar();
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
    }

    processObj();

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices.front(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, ind.size() * sizeof(uint32_t), &ind.front(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glGenTextures(1, &TEX);
    glBindTexture(GL_TEXTURE_2D, TEX);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);

    unsigned char* data = stbi_load("dirt.jpg", &width, &height, &nrChannels, 0);

    if (data) {
        if (nrChannels == 3)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        else
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "Failed to load texture" << std::endl;
    }
    lambertShader.use();
    lambertShader.setInt("texture1", 1);

    glEnable(GL_DEPTH_TEST);


    while (!glfwWindowShouldClose(window)) {
        frame();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glfwDestroyWindow(window);
    glfwTerminate();
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
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
        /*if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            camera.ProcessKeyboard(UP, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
            camera.ProcessKeyboard(DOWN, deltaTime);*/
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

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
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void processObj() {
    std::vector<glm::vec3> position;
    std::vector<glm::vec2> texture;
    std::vector<glm::vec3> normals;

    glm::vec3 posVec3;
    glm::vec2 textVec2;
    glm::vec3 normVec3;
    Face face;
    Vertex vertex;
    int counter = 0;

    FILE* file = fopen(objPath, "r");
    if (file == NULL) {
        printf("Impossible to open the file !\n");
        return;
    }

    while (1) {
        char lineHeader[128];
        // read the first word of the line
        int res = fscanf(file, "%s", lineHeader);
        if (res == EOF)
            break; // EOF = End Of File. Quit the loop.

        // else : parse lineHeader
        if (strcmp(lineHeader, "v") == 0) {
            fscanf(file, "%f %f %f\n", &posVec3.x, &posVec3.y, &posVec3.z);
            position.push_back(posVec3);
        }
        else if (strcmp(lineHeader, "vt") == 0) {
            fscanf(file, "%f %f\n", &textVec2.x, &textVec2.y);
            texture.push_back(textVec2);
        }
        else if (strcmp(lineHeader, "vn") == 0) {
            fscanf(file, "%f %f %f\n", &normVec3.x, &normVec3.y, &normVec3.z);
            normals.push_back(normVec3);
        }
        else if (strcmp(lineHeader, "f") == 0) {
            unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
            int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0],
                &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2],
                &normalIndex[2]);
            if (matches != 9) {
                printf("File can't be read by our simple parser : ( Try exporting with other options\n");
                return;
            }
            Face temp[3];
            temp[0].v = vertexIndex[0];
            temp[1].v = vertexIndex[1];
            temp[2].v = vertexIndex[2];

            temp[0].vt = uvIndex[0];
            temp[1].vt = uvIndex[1];
            temp[2].vt = uvIndex[2];

            temp[0].vn = normalIndex[0];
            temp[1].vn = normalIndex[1];
            temp[2].vn = normalIndex[2];

            // sprawdzenie czy juz byl
            for (int j = 0; j < 3; j++) {
                bool flag = false;

                for (int i = 0; i < counter; i++) {
                    if (position[temp[j].v - 1] == vertices[i].pos
                        && texture[temp[j].vt - 1] == vertices[i].texture
                        && normals[temp[j].vn - 1] == vertices[i].normals) {
                        flag = true;
                        ind.push_back(i);
                        break;
                    }
                }
                if (!flag) {
                    vertex.pos = position[temp[j].v - 1];
                    vertex.texture = texture[temp[j].vt - 1];
                    vertex.normals = normals[temp[j].vn - 1];

                    vertices.push_back(vertex);
                    ind.push_back(counter);
                    counter++;
                }
            }
        }
    }

}

