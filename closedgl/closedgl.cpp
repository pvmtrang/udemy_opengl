// closedgl.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <string.h>
#include <iostream>
#include <cmath>

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"



// Glint, etc is consistent on any compiler != normal int
const GLint WIDTH = 800, HEIGHT = 600;
const float TO_RADIAN = 3.14f / 180;

GLuint VAO, VBO, shaderId, uniformXMove, uniformModelTranslateId, IBO, uniformProjectionId;

bool direction = true;
float triOffset = 0.0f;
float triMaxOffset = 0.7f;
float triIncrement = 0.0002f;

float triRotateOffset = 0.0f;
float triRotateIncrement = 0.2f;

float triScaleOffset = 0.0f;
float triScaleMax = 1.0000f;
float triScaleIncrement = 0.001f;

//normally shader codes are in separate files

//vertex shader 
static const char* vShader = "                                                \n\
#version 330                                                                  \n\
                                                                              \n\
layout (location = 0) in vec3 pos;											  \n\
										                                        \n\
out vec4 vertexColor;										                    \n\
										                                        \n\
//uniform float xMove;									  \n\
uniform mat4 modelTranslate;									  \n\
uniform mat4 projection;                                                       \n\
                                                                              \n\
void main()                                                                   \n\
{                                                                             \n\
    //gl_Position = vec4(0.4 * pos.x + xMove, 0.4 * pos.y, pos.z, 1.0);				  \n\
    gl_Position =  projection * modelTranslate * vec4(pos.x, pos.y, pos.z, 1.0);				  \n\
    vertexColor = vec4(clamp(pos, 0.0f, 1.0f), 1.0f);                                                          \n\
}";
/* clamp(value, low, high)
* if value < low -> return low
* if value > high -> return high
* The funny thing: it is ```std::clamp()```. But if i do ```std::clamp(pos,...)``` -> wrong result
* Why??? when i didn't do ```using namespace std```
*/

// Fragment Shader
static const char* fShader = "                                                \n\
#version 330                                                                \n\
                                                                  \n\
in vec4 vertexColor;										                    \n\
                                                                              \n\
out vec4 colour;                                                               \n\
                                                                              \n\
void main()                                                                   \n\
{                                                                             \n\
    //colour = vec4(1.0, 1.0, 0.0, 1.0);                                         \n\
    colour = vertexColor;                                        \n\
}";

//the render's window coordinate is in the middle 
void CreateTriangle() {
    unsigned int indices[] = {
        0, 2, 3,
        0, 1, 3,
        1, 2, 3,
        0, 1, 2
    };

    GLfloat vertices[] = {
       -1.0f, -1.0f, 0.0f,  //0
       0.0f, -1.0f, 1.0f,   //1
        1.0f, -1.0f, 0.0f,  //2
        0.0f, 1.0f, 0.0f    //3
    };

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    //GL_STATIC DRAW: the data aren't gonna change at all
    //GL_DYNAMIC_DRAW: drawing moving shapes, aka changing data

    //0: index: aka location of the layout of the shader
    //3: size of each values passed in (i.e, size of each rows
    //GL_FLOAT: data type of values
    //0: stride <- seems cool
    //0: where to first read the data, i.e if = 1 -> read from the second line
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
   

    //unbiding
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    //unbind IBO/EBO MUST be AFTER unbinding VAO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

}

void AddShader(GLuint theProgram, const char* shaderCode, GLenum shaderType)
{
    GLuint theShader = glCreateShader(shaderType);

    //because shaderCode is char* while the function needs glChar
    const GLchar* theCode[1];
    theCode[0] = shaderCode;
    //NOT const GLchar* = shaderCode; -> weird code, can't

    GLint codeLength[1];
    codeLength[0] = strlen(shaderCode);

    glShaderSource(theShader, 1, theCode, codeLength);
    glCompileShader(theShader);

    GLint result = 0;
    GLchar eLog[1024] = { 0 };

    glGetShaderiv(theShader, GL_COMPILE_STATUS, &result);
    if (!result)
    {
        glGetShaderInfoLog(theShader, 1024, NULL, eLog);
        fprintf(stderr, "Error compiling the %d shader: '%s'\n", shaderType, eLog);
        return;
    }

    glAttachShader(theProgram, theShader);
}

void CompileShaders()
{
    shaderId = glCreateProgram();

    if (!shaderId)
    {
        printf("Failed to create shader\n");
        return;
    }

    AddShader(shaderId, vShader, GL_VERTEX_SHADER);
    AddShader(shaderId, fShader, GL_FRAGMENT_SHADER);

    //result of 2 above functions
    GLint result = 0;
    //store error logs
    GLchar eLog[1024] = { 0 };

    glLinkProgram(shaderId);
    glGetProgramiv(shaderId, GL_LINK_STATUS, &result);
    if (!result)
    {
        glGetProgramInfoLog(shaderId, sizeof(eLog), NULL, eLog);
        printf("Error linking program: '%s'\n", eLog);
        return;
    }

    glValidateProgram(shaderId);
    glGetProgramiv(shaderId, GL_VALIDATE_STATUS, &result);
    if (!result)
    {
        glGetProgramInfoLog(shaderId, sizeof(eLog), NULL, eLog);
        printf("Error validating program: '%s'\n", eLog);
        return;
    }

	//uniformXMove = glGetUniformLocation(shader, "xMove");
    uniformModelTranslateId = glGetUniformLocation(shaderId, "modelTranslate");
    uniformProjectionId = glGetUniformLocation(shaderId, "projection");
}


int main()
{   
    //Initialize GLFW
    if (!glfwInit()) {
        std::cout << "initialize glfw failed\n";
        glfwTerminate();
        return 1;
    }

    //Setup GLFW window properties

    //set opengl version, ver 3.3, older than that is deprecated
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); 
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); 

    //core profile = no backwards compatibility aka. not using deprecate, about-to-be-removed features
    //2 types of profiles: core profile and compatibility profile
    //CORE_PROFILE: new stuff vs. COMPAT_PROFILE: old rooif
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //allow forward compatibility
    //this one is required in macOS
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* mainWindow = glfwCreateWindow(WIDTH, HEIGHT, "Hello mf", NULL, NULL);
    if (!mainWindow) {
        std::cout << "create glfw window failed\n";
        return 1;
    }

    //get framebuffer size
    int bufferWidth, bufferHeight;
    glfwGetFramebufferSize(mainWindow, &bufferWidth, &bufferHeight);

    //set context for glew to use
    glfwMakeContextCurrent(mainWindow);

    //allow modern extension features to retrieve function pointers and stuff
    glewExperimental = GL_TRUE;

    //initialize GLEW
    if (glewInit() != GLEW_OK) {
        std::cout << "initialize GLEW failed\n";
        glfwDestroyWindow(mainWindow);
        glfwTerminate();
        return 1;
    }

    glEnable(GL_DEPTH_TEST);


    //set viewport size
    glViewport(0, 0, bufferWidth, bufferHeight);

    CreateTriangle();
    CompileShaders();

    glm::mat4 projection = glm::perspective(45.0f, (GLfloat)bufferWidth / (GLfloat)bufferHeight, 2.8f, 100.0f);

    while (!glfwWindowShouldClose(mainWindow)) {
        glfwPollEvents();

        if (direction)
        {
            triOffset += triIncrement;
        }
        else {
            triOffset -= triIncrement;
        }

        if (abs(triOffset) >= triMaxOffset)
        {
            direction = !direction;
        }

        triRotateOffset += triRotateIncrement;
        triScaleOffset += triScaleIncrement;

       if (triScaleOffset > triScaleMax || triScaleOffset < 0) {
            triScaleIncrement *= -1;
       }

        glClearColor(0.3f, 0.2f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderId);

        glm::mat4 model(1.0f);
        model = glm::translate(model, glm::vec3(triOffset, triOffset, -3.5f));
        //glm::vec3 is for indicating which axis the model is rotating around, value doesn't matter.
        model = glm::rotate(model, triRotateOffset* TO_RADIAN, glm::vec3(1.0f, 1.0f, 1.0f));
        model = glm::scale(model, glm::vec3(triScaleOffset, triScaleOffset, triScaleOffset));

        //glUniform1f(uniformXMove, triOffset);
        glUniformMatrix4fv(uniformModelTranslateId, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(uniformProjectionId, 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(VAO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
        //sizeof(indices) / sizeof(unsigned int)
        glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        //glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);

        glUseProgram(0);

        glfwSwapBuffers(mainWindow);
    }

    

    return 0;
}

