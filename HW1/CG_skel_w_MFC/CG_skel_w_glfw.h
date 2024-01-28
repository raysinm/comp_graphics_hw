#pragma once
#include "resource.h"
#include <vector>
#include "GLFW/glfw3.h"
#include "imgui.h"

//void display(void);
//void reshape(int width, int height);
//void keyboard(unsigned char key, int x, int y);
//void mouse(int button, int state, int x, int y);
//void fileMenu(int id);
//void mainMenu(int id);
//void selectMenu(int id);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

void initMenu();
#ifdef _DEBUG
//void debug_PlayWithVectors();
//void debug_PlayWithMatrices();
void debug_func();
void imguiDemoRun(ImGuiIO& io);

#endif

