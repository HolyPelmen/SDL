#pragma once
#include <string>
#include <SDL.h>
class Card
{
public:
	Card() {};
	Card(int, int, int, int, std::string, int);
	virtual ~Card();

	virtual void setvalue(int);
	virtual int getvalue();

	virtual int betterChoice(int);

	SDL_Rect getBox();
	void setBox(int, int, int, int);

	std::string getName();
protected:
	SDL_Rect box;
	std::string name;
	int value;
};

class Ace : public Card {
public:
	Ace(int x, int y, int w, int h, std::string _name, int _value) : Card(x, y, w, h, _name, _value) {
		box.x = x;
		box.y = y;
		box.w = w;
		box.h = h;
		name = _name;
		value = _value;
	}

	int betterChoice(int) override;
};

