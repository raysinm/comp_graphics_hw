#pragma once
#include "resource.h"
#include <vector>
#include "GLFW/glfw3.h"
#include "imgui.h"

#define START_WIDTH 1024
#define START_HEIGHT 712
<<<<<<< HEAD
//const float modelAspectRatio = 16.0f / 9.0f;
=======
>>>>>>> 77a337371d15ac05dc911cef8f28ce9e5750a630
const float modelAspectRatio = 1.0f / 1.0f;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_move_callback(GLFWwindow* window, double x, double y);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

