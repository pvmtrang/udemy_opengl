// closedgl.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <iostream>
#include "GL/glew.h"
#include "GLFW/glfw3.h"

const GLint WIDTH = 800, HEIGHT = 600;

int main()
{   
    //Initialize GLFW
    if (!glfwInit()) {
        std::cout << "initialize glfw failed\n";
        glfwTerminate();
        return 1;
    }
    //Setup GLFW window properties
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); //set opengl version???
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); //ver 3.3
    //core profile = no backwards compatibility
    //aka. not using deprecate, about-to-be-removed features
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //allow forward compatibility
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

    //allow modern extension features
    glewExperimental = GL_TRUE;

    //initialize GLEW
    if (glewInit() != GLEW_OK) {
        std::cout << "initialize GLEW failed\n";
        glfwDestroyWindow(mainWindow);
        glfwTerminate();
        return 1;
    }

    //set viewport size
    glViewport(0, 0, bufferWidth, bufferHeight);

    while (!glfwWindowShouldClose(mainWindow)) {
        glfwPollEvents();
        glClearColor(0.3f, 0.2f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(mainWindow);
    }

    return 0;
}

