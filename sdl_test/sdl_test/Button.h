#pragma once
#include <SDL.h>
class Button
{
public:
	Button();
	Button(int, int, int, int);
	virtual ~Button();

	bool handle_events(SDL_Event);
	virtual bool mouse_event(SDL_Event);

	void setbox(int, int, int, int);
	int getCoord(char);
	SDL_Rect getbox();

	void hidebox();
	void showbox();

	void setPressed(bool);

	void setActive(bool);
	bool getActive();
protected:
	bool pressed = false;
	bool active = true;
	SDL_Rect box;
	SDL_Rect hidenbox;
};

class ButtonPressed : public Button
{
public:
	ButtonPressed(int x, int y, int w, int h) : Button(x, y, w, h)
	{
		box.x = x;
		box.y = y;
		box.w = w;
		box.h = h;
		pressed = false;
	}
	~ButtonPressed();
	bool mouse_event(SDL_Event event) override;
};

