#include "Card.h"

Card::Card(int x, int y, int w, int h, std::string _name, int _value)
{
	box.x = x;
	box.y = y;
	box.w = w;
	box.h = h;
	name = _name;
	value = _value;
}

Card::~Card() {}

void Card::setvalue(int _value)
{
	value = _value;
}

int Card::getvalue()
{
	return value;
}

int Card::betterChoice(int)
{
	return value;
}

SDL_Rect Card::getBox()
{
	return box;
}

void Card::setBox(int x, int y, int w, int h)
{
	box.x = x;
	box.y = y;
	box.w = w;
	box.h = h;
}

std::string Card::getName()
{
	return name;
}

int Ace::betterChoice(int points)
{
	if ((21 - points) > 11) value = 11;
	else if ((21 - points) <= 11) value = 1;
	return value;
}
