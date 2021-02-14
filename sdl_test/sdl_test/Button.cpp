#include "Button.h"

Button::Button(){}

Button::Button(int x, int y, int w, int h)
{
	box.x = x;
	box.y = y;
	box.w = w;
	box.h = h;
	pressed = false;
}

Button::~Button()
{
	pressed = false;
}

bool Button::handle_events(SDL_Event event)
{
	int x = 0, y = 0;
	bool success = false;
	if (event.type == SDL_MOUSEMOTION) {
		x = event.motion.x;
		y = event.motion.y;
		if ((x > box.x) && (x < box.x + box.w) && (y > box.y) && (y < box.y + box.h)) {
			success = true;
		}
	}
	return success;
}

bool Button::mouse_event(SDL_Event event) {
	return 0;
}

void Button::setbox(int x, int y, int w, int h)
{
	box.x = x;
	box.y = y;
	box.w = w;
	box.h = h;
}

int Button::getCoord(char _t)
{
	if (_t == 'x') return box.x;
	if (_t == 'y') return box.y;
	if (_t == 'w') return box.w;
	if (_t == 'h') return box.h;

}

SDL_Rect Button::getbox()
{
	return box;
}

void Button::hidebox()
{
	hidenbox.x = box.x;
	hidenbox.y = box.y;
	hidenbox.h = box.h;
	hidenbox.w = box.w;

	box.x = -1; box.y = -1; box.h = -1; box.w = -1;
}

void Button::showbox()
{
	box.x = hidenbox.x;
	box.y = hidenbox.y;
	box.h = hidenbox.h;
	box.w = hidenbox.w;
}

void Button::setPressed(bool p)
{
	pressed = p;
}

void Button::setActive(bool _active)
{
	active = _active;
}

bool Button::getActive()
{
	return active;
}

ButtonPressed::~ButtonPressed() {
	pressed = false;
}

bool ButtonPressed::mouse_event(SDL_Event event)
{
	int x = 0, y = 0;
	if (event.type == SDL_MOUSEBUTTONDOWN) {
		x = event.motion.x;
		y = event.motion.y;
		if ((x > box.x) && (x < box.x + box.w) && (y > box.y) && (y < box.y + box.h)) {
			if (event.button.button == SDL_BUTTON_LEFT) {
				pressed = true;
			}
		}
	}
	return pressed;
}
