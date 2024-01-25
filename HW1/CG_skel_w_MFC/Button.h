#pragma once
#include "vec.h"
typedef void (*callback_function)(void);
class Button
{
public:
	Button(int x, int y, int width, int height);
	Button(vec2 pos, vec2 size);
	void Draw(float* buffer, int width, int height);

	void BtnClick();
	void SetCallbackFunc(callback_function f);
	void setPos(vec2 pos);
	void setPos(int x, int y);
	bool isInside(vec2 mPos);
private:
	vec2 pos;
	vec2 size;
	callback_function click_func;
	float* pixels; //size: 3*width*height
};