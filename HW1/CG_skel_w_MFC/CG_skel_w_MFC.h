#pragma once

#include "resource.h"

void display( void );
void reshape( int width, int height );
void keyboard( unsigned char key, int x, int y );
void mouse(int button, int state, int x, int y);
void fileMenu(int id);
void mainMenu(int id);
void initMenu();
#ifdef _DEBUG
void debug_PlayWithVectors();
void debug_PlayWithMatrices();

#endif