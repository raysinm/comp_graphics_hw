#pragma once
#include "resource.h"
#include <vector>
#include "GLFW/glfw3.h"
#include "imgui.h"

#define START_WIDTH 512
#define START_HEIGHT 512


//void display(void);
//void reshape(int width, int height);
//void keyboard(unsigned char key, int x, int y);
//void mouse(int button, int state, int x, int y);
//void fileMenu(int id);
//void mainMenu(int id);
//void selectMenu(int id);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_move_callback(GLFWwindow* window, double x, double y);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

#ifdef _DEBUG
void debug_func();
#endif

