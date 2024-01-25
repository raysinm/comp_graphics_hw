#include "stdafx.h"
#include "Button.h"

#define INDEX(width,x,y,c) (x+y*width)*3 + c
#define RED   0
#define GREEN 1
#define BLUE  2

Button::Button(int x, int y, int width, int height)
{
	this->pos = vec2(x, y);
	this->size = vec2(width, height);

	pixels = new float[3 * size.x * size.y];
	for (int i = 0; i < 3 * size.x * size.y; i++)
	{
		pixels[i] = 0;
	}
}

Button::Button(vec2 pos, vec2 size)
{
	this->pos = pos;
	this->size = size;

	pixels = new float[3 * size.x * size.y];
	for (int i = 0; i < 3 * size.x * size.y; i++)
	{
		pixels[i] = 0;
	}
}

void Button::Draw(float* buffer, int width, int height)
{
	if (pos.x < 0 || pos.x + size.x > width || pos.y < 0 || pos.y + size.y > height)
		return; //Button is out of bound. don't draw it all.
	if (buffer == nullptr || pixels == nullptr)
		return;

	for (int x = 0; x < size.x; x++)
	{
		for (int y = 0; y < size.y; y++)
		{
			const int screen_x = pos.x + x;
			const int screen_y = height - (pos.y + y); //Inverse only y !

			buffer[INDEX(width, screen_x, screen_y, RED)]   = pixels[INDEX((int)size.x, x, y, RED)];
			buffer[INDEX(width, screen_x, screen_y, GREEN)] = pixels[INDEX((int)size.x, x, y, GREEN)];
			buffer[INDEX(width, screen_x, screen_y, BLUE)]  = pixels[INDEX((int)size.x, x, y, BLUE)];
		}
	}


}

void Button::BtnClick()
{
	if (click_func != nullptr)
	{
		click_func();
	}
}

void Button::SetCallbackFunc(callback_function f)
{
	this->click_func = f;
}

void Button::setPos(vec2 pos)
{
	this->pos = pos;
}

void Button::setPos(int x, int y)
{
	this->pos = vec2(x, y);
}

bool Button::isInside(vec2 mPos)
{
	if (mPos.x < pos.x || mPos.x > pos.x + size.x ||
		mPos.y < pos.y || mPos.y > pos.y + size.y)
		return false;
	
	return true;
}
