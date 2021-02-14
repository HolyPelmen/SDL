#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <iostream>
#include <map>
#include <vector>
#include <random>
#include <algorithm>
#include <chrono>
#include <memory>
#include <type_traits>
#include <string>
#include "Button.h"
#include "Card.h"

using namespace std;

struct sdl_deleter
{
	void operator()(SDL_Window* p) const { SDL_DestroyWindow(p); }
	void operator()(SDL_Surface* p) const { p = NULL; SDL_FreeSurface(p); }
	void operator()(SDL_Renderer* p) const { SDL_DestroyRenderer(p); }
	void operator()(SDL_Texture* p) const { SDL_DestroyTexture(p); }
};

//-------------------------------------------------------------------------------------------------------------------------------------

bool init(const char* name, unique_ptr<SDL_Window, sdl_deleter>* win, unique_ptr<SDL_Surface, sdl_deleter>* screen, int width, int height, SDL_WindowFlags flag);
bool initRender(unique_ptr<SDL_Renderer, sdl_deleter>* ren, unique_ptr<SDL_Window, sdl_deleter>* win);
void createTexture(unique_ptr<SDL_Renderer, sdl_deleter>* ren, unique_ptr<SDL_Texture, sdl_deleter>* texture, SDL_Rect rect, const char* name);
void updateTexture(unique_ptr<SDL_Renderer, sdl_deleter>* ren, unique_ptr<SDL_Texture, sdl_deleter>* texture, SDL_Rect rect);
void print_ttf(unique_ptr<SDL_Renderer, sdl_deleter>* renderer, unique_ptr<SDL_Texture, sdl_deleter>* texture, const char* message, const char* font, int size, SDL_Color color, SDL_Rect dest);
void print_ttf(unique_ptr<SDL_Renderer, sdl_deleter>* renderer, unique_ptr<SDL_Texture, sdl_deleter>* texture, int num, const char* font, int size, SDL_Color color, SDL_Rect dest);
void closeWindow(unique_ptr<SDL_Window, sdl_deleter>* win);

//-------------------------------------------------------------------------------------------------------------------------------------

unique_ptr<SDL_Window, sdl_deleter> startWindow = NULL;
unique_ptr<SDL_Surface, sdl_deleter> background = NULL;

unique_ptr<SDL_Window, sdl_deleter> optionWindow = NULL;
unique_ptr<SDL_Surface, sdl_deleter> optionBackground = NULL;

unique_ptr<SDL_Surface, sdl_deleter> textTemp = NULL;

unique_ptr<SDL_Texture, sdl_deleter> tableBackground = NULL;
unique_ptr<SDL_Renderer, sdl_deleter> render = NULL;

unique_ptr<SDL_Texture, sdl_deleter> startNewGameTexrure = NULL;
unique_ptr<SDL_Texture, sdl_deleter> optionTexture = NULL;
unique_ptr<SDL_Texture, sdl_deleter> exitTexture = NULL;

//-------------------------------------------------------------------------------------------------------------------------------------

const SDL_Color kBlack = { 0x00, 0x00, 0x00, 0xFF };
const int kCard = 52;

//-------------------------------------------------------------------------------------------------------------------------------------

const map<string, int> cardDeck = {
	{"2diamonds", 2}, {"3diamonds", 3}, {"4diamonds", 4}, {"5diamonds", 5}, {"6diamonds", 6}, {"7diamonds", 7}, {"8diamonds", 8}, {"9diamonds", 9}, {"10diamonds", 10}, {"jDiamonds", 10}, {"qDiamonds", 10}, {"kDiamonds", 10}, {"aceDiamonds", 0},  //Бубны
	{"2hearts",2}, {"3hearts",3}, {"4hearts",4}, {"5hearts",5}, {"6hearts",6}, {"7hearts",7}, {"8hearts",8}, {"9hearts",9}, {"10hearts",10}, {"jHearts",10}, {"qHearts",10}, {"kHearts",10}, {"aceHearts",0},                                         //Черви
	{"2spades",2}, {"3spades" ,3}, {"4spades",4}, {"5spades",5}, {"6spades",6}, {"7spades",7}, {"8spades",8}, {"9spades",9}, {"10spades",10}, {"jSpades",10}, {"qSpades",10}, {"kSpades",10}, {"aceSpades",0},                                        //Пики
	{"2clubs",2}, {"3clubs",3}, {"4clubs",4}, {"5clubs",5}, {"6clubs",6}, {"7clubs",7}, {"8clubs",8}, {"9clubs",9}, {"10clubs",10}, {"jClubs",10}, {"qClubs",10}, {"kClubs",10}, {"aceClubs",0},                                                      //Крест
};

int main(int argc, char* args[]) {

	int SCREEN_HEIGHT = 720, SCREEN_WIDTH = 1280;

	int seed = chrono::steady_clock::now().time_since_epoch().count();

	double scale = 3;
	vector<int> deckID(kCard);
	for (int q = 0; q < kCard; ++q) { deckID[q] = (q + 1); }

	try { if (init("Start window", &startWindow, &background, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN) == 0) throw - 1; }
	catch (int a) { cout << "Initialize start window result: " << a << ". Error: " << SDL_GetError() << endl; }

	try { TTF_Init(); atexit(TTF_Quit);}
	catch (int a) { cout << "Initialize TTF result: " << a << ". Error: " << TTF_GetError() << endl; }

	try { if (initRender(&render, &startWindow) == 0) throw -1; }
	catch (int a) { cout << "Initialize render result: " << a << ". Error: " << SDL_GetError() << endl; }

	SDL_Rect tableRect; tableRect.x = 0; tableRect.y = 0; tableRect.w = SCREEN_WIDTH; tableRect.h = SCREEN_HEIGHT;
	createTexture(&render, &tableBackground, tableRect, "Texture/pokerTable.png");
	
	unique_ptr<Button> startNewGameBut (new ButtonPressed(((SCREEN_WIDTH/2)-((SCREEN_WIDTH/ scale)/2)), ((SCREEN_HEIGHT / 2) - ((SCREEN_HEIGHT / scale)))-75, (SCREEN_WIDTH / scale), (SCREEN_HEIGHT / (scale*1,5))));
	createTexture(&render, &startNewGameTexrure, startNewGameBut->getbox(), "Texture/mainMenu/startNewGame.png");

	unique_ptr<Button> optionsBut(new ButtonPressed(((SCREEN_WIDTH / 2) - ((SCREEN_WIDTH / scale) / 2)), (SCREEN_HEIGHT / 2)-75, (SCREEN_WIDTH / scale), (SCREEN_HEIGHT / (scale * 1, 5))));
	createTexture(&render, &optionTexture, optionsBut->getbox(), "Texture/mainMenu/options.png");

	unique_ptr<Button> exitBut(new ButtonPressed(((SCREEN_WIDTH / 2) - ((SCREEN_WIDTH / scale) / 2)), ((SCREEN_HEIGHT / 2) + ((SCREEN_HEIGHT / scale)))-75, (SCREEN_WIDTH / scale), (SCREEN_HEIGHT / (scale * 1, 5))));
	createTexture(&render, &exitTexture, exitBut->getbox(), "Texture/mainMenu/exit.png");

#pragma region mainCycle
	bool runProgramm = true;
	SDL_Event e;

	while (runProgramm == true) {
		while (SDL_PollEvent(&e) != 0) {
			if (e.type == SDL_QUIT || exitBut->mouse_event(e) == true) runProgramm = false;

			if (optionsBut->mouse_event(e) == true) {
				init("Options", &optionWindow, &optionBackground, 400, 350, SDL_WINDOW_INPUT_FOCUS);

				unique_ptr<SDL_Renderer, sdl_deleter> optionsRender = NULL;
				unique_ptr<SDL_Texture, sdl_deleter> scaleTexture = NULL; SDL_Rect scaleRect = { 5, 5, 150, 30 };
				unique_ptr<SDL_Texture, sdl_deleter> but1Texture = NULL;
				unique_ptr<SDL_Texture, sdl_deleter> but087Texture = NULL;
				unique_ptr<SDL_Texture, sdl_deleter> but075Texture = NULL;
				unique_ptr<SDL_Texture, sdl_deleter> but062Texture = NULL;
				unique_ptr<SDL_Texture, sdl_deleter> but05Texture = NULL;

				initRender(&optionsRender, &optionWindow);
				SDL_RenderClear(optionsRender.get());
				createTexture(&optionsRender, &scaleTexture, scaleRect, "Texture/mainMenu/scale.png");

				unique_ptr<Button> but1 (new ButtonPressed(5, 45, 70, 70));
				createTexture(&optionsRender, &but1Texture, but1->getbox(), "Texture/mainMenu/1.png");

				unique_ptr<Button> but087 (new ButtonPressed(85, 45, 70, 70));
				createTexture(&optionsRender, &but087Texture, but087->getbox(), "Texture/mainMenu/087.png");

				unique_ptr<Button> but075 (new ButtonPressed(165, 45, 70, 70));
				createTexture(&optionsRender, &but075Texture, but075->getbox(), "Texture/mainMenu/075.png");

				unique_ptr<Button> but062 (new ButtonPressed(245, 45, 70, 70));
				createTexture(&optionsRender, &but062Texture, but062->getbox(), "Texture/mainMenu/062.png");

				unique_ptr<Button> but05 (new ButtonPressed(325, 45, 70, 70));
				createTexture(&optionsRender, &but05Texture, but05->getbox(), "Texture/mainMenu/05.png");

				bool runOptionsWin = true;
				while(runOptionsWin == true)
				{
					while (SDL_PollEvent(&e) != 0) {
						if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_CLOSE) {
							if (SDL_GetWindowID(optionWindow.get()) == e.window.windowID) {
								runOptionsWin = false;
								optionsBut->setPressed(false);
							}
						}
						if (but1->mouse_event(e) == true) { scale = 3; but1->setPressed(false); }
						if (but087->mouse_event(e) == true) { scale = 3.5; but087->setPressed(false); }
						if (but075->mouse_event(e) == true) { scale = 4; but075->setPressed(false); }
						if (but062->mouse_event(e) == true) { scale = 4.5; but062->setPressed(false); }
						if (but05->mouse_event(e) == true) { scale = 5; but05->setPressed(false); }

					}
				}
				closeWindow(&optionWindow);
				SDL_RenderClear(render.get());

				tableRect.w = SCREEN_WIDTH; tableRect.h = SCREEN_HEIGHT;
				createTexture(&render, &tableBackground, tableRect, "Texture/pokerTable.png");
				startNewGameBut->setbox(((SCREEN_WIDTH / 2) - ((SCREEN_WIDTH / scale) / 2)), ((SCREEN_HEIGHT / 2) - ((SCREEN_HEIGHT / scale))) - 75, (SCREEN_WIDTH / scale), (SCREEN_HEIGHT / (scale * 1, 5)));
				createTexture(&render, &startNewGameTexrure, startNewGameBut->getbox(), "Texture/mainMenu/startNewGame.png");
				optionsBut->setbox(((SCREEN_WIDTH / 2) - ((SCREEN_WIDTH / scale) / 2)), (SCREEN_HEIGHT / 2) - 75, (SCREEN_WIDTH / scale), (SCREEN_HEIGHT / (scale * 1, 5)));
				createTexture(&render, &optionTexture, optionsBut->getbox(), "Texture/mainMenu/options.png");
				exitBut->setbox(((SCREEN_WIDTH / 2) - ((SCREEN_WIDTH / scale) / 2)), ((SCREEN_HEIGHT / 2) + ((SCREEN_HEIGHT / scale))) - 75, (SCREEN_WIDTH / scale), (SCREEN_HEIGHT / (scale * 1, 5)));
				createTexture(&render, &exitTexture, exitBut->getbox(), "Texture/mainMenu/exit.png");
			}

			if (startNewGameBut->mouse_event(e) == true) {
				SDL_SetRenderDrawColor(render.get(), 0x1A, 0x052, 0x14, 0xFF);
				SDL_RenderClear(render.get());
				startNewGameBut->setPressed(false);

				int cash = 10000, bet = 0;

				string betTemp = "Bet:" + to_string(bet) + "$";
				char betStr[20];
				strcpy_s(betStr, betTemp.c_str());

				startNewGameBut.get()->hidebox(); optionsBut.get()->hidebox(); exitBut.get()->hidebox();
				updateTexture(&render, &tableBackground, tableRect);

				SDL_Rect controlPanel = { SCREEN_WIDTH / 1.5, SCREEN_HEIGHT/1.2, (SCREEN_WIDTH - (SCREEN_WIDTH / 1.5)), (SCREEN_HEIGHT - (SCREEN_HEIGHT / 1.2)) };
				SDL_RenderFillRect(render.get(), &controlPanel);
				SDL_RenderPresent(render.get());

				unique_ptr <SDL_Texture, sdl_deleter> betLabel = NULL;
				SDL_Rect betLabelRect = { 0, controlPanel.y, SCREEN_WIDTH/(scale*2), controlPanel.h };
				print_ttf(&render, &betLabel, betStr, "times.ttf", 200, kBlack, betLabelRect);

				unique_ptr <SDL_Texture, sdl_deleter> cashLabelTexture = NULL;
				SDL_Rect cashLabelRect = { 0, 0, SCREEN_WIDTH / (scale * 5), SCREEN_HEIGHT / (scale * 3) };
				print_ttf(&render, &cashLabelTexture, "Cash:", "times.ttf", 200, kBlack, cashLabelRect);

				unique_ptr <SDL_Texture, sdl_deleter> cashLabelTexture_num = NULL;
				SDL_Rect cashLabelRect_num = { 0, cashLabelRect.h, SCREEN_WIDTH / (scale * 5),  SCREEN_HEIGHT / (scale * 3) };
				print_ttf(&render, &cashLabelTexture_num, cash, "times.ttf", 200, kBlack, cashLabelRect_num);

				unique_ptr<Button> upBetBut(new ButtonPressed(controlPanel.x, controlPanel.y, controlPanel.h / 2, controlPanel.h / 2));
				unique_ptr<SDL_Texture, sdl_deleter> upBetTexture = NULL;
				createTexture(&render, &upBetTexture, upBetBut->getbox(), "Texture/controlPanel/upBet.png");

				unique_ptr<Button> lowBetBut(new ButtonPressed(controlPanel.x, controlPanel.y + upBetBut->getCoord('h'), controlPanel.h / 2, controlPanel.h / 2));
				unique_ptr<SDL_Texture, sdl_deleter> lowBetTexture = NULL;
				createTexture(&render, &lowBetTexture, lowBetBut->getbox(), "Texture/controlPanel/downBet.png");

				unique_ptr<Button> betBut(new ButtonPressed(controlPanel.x + upBetBut->getCoord('w') + (20 / scale), controlPanel.y + (controlPanel.h / 5), controlPanel.h / 1.5, controlPanel.h / 1.5));
				unique_ptr<SDL_Texture, sdl_deleter> betTexture = NULL;
				createTexture(&render, &betTexture, betBut->getbox(), "Texture/controlPanel/bet.png");

				unique_ptr<Button> standBut(new ButtonPressed(betBut->getCoord('x') + betBut->getCoord('w') + (20 / scale), controlPanel.y + (controlPanel.h / 5), controlPanel.h / 1.5, controlPanel.h / 1.5));
				unique_ptr<SDL_Texture, sdl_deleter> standTexture = NULL;
				createTexture(&render, &standTexture, standBut->getbox(), "Texture/controlPanel/stand.png");

				unique_ptr<Button> hitBut(new ButtonPressed(standBut->getCoord('x') + standBut->getCoord('w') + (20 / scale), controlPanel.y + (controlPanel.h / 5), controlPanel.h / 1.5, controlPanel.h / 1.5));
				unique_ptr<SDL_Texture, sdl_deleter> hitTexture = NULL;
				createTexture(&render, &hitTexture, hitBut->getbox(), "Texture/controlPanel/hit.png");

				unique_ptr<Button> mainMenuBut(new ButtonPressed(hitBut->getCoord('x') + hitBut->getCoord('w') + (20 / scale), controlPanel.y + (controlPanel.h / 5), controlPanel.h / 1.5, controlPanel.h / 1.5));
				unique_ptr < SDL_Texture, sdl_deleter> mainMenuTexture = NULL;
				createTexture(&render, &mainMenuTexture, mainMenuBut->getbox(), "Texture/controlPanel/mainMenu.png");

				SDL_Rect Deck = { SCREEN_WIDTH - (SCREEN_WIDTH / 5), 0 + (SCREEN_HEIGHT / 30), SCREEN_WIDTH / (scale*2), 1.3 * SCREEN_HEIGHT / scale };
				unique_ptr<SDL_Texture, sdl_deleter> deckTexture = NULL;
				createTexture(&render, &deckTexture, Deck, "Texture/deck/top.png");

				unique_ptr <SDL_Texture, sdl_deleter> dealerPointTexture = NULL;
				SDL_Rect dealerPointRect = { (SCREEN_WIDTH / 3) - ((SCREEN_WIDTH / 2.9) / scale), 0, 250 / scale, 350 / scale };

				unique_ptr <SDL_Texture, sdl_deleter> playerPointTexture = NULL;
				SDL_Rect playerPointRect = { (SCREEN_WIDTH / 3) - ((SCREEN_WIDTH / 2.9) / scale), SCREEN_HEIGHT-(SCREEN_HEIGHT/2), 250 / scale, 350 / scale };

				vector<unique_ptr<Card>> dealerHand;
				vector<unique_ptr<SDL_Texture, sdl_deleter>> dealerHandTexture;
				vector<char*> dealerWorkArr;

				vector<unique_ptr<Card>> playerHand;
				vector<unique_ptr<SDL_Texture, sdl_deleter>> playerHandTexture;

				int dealerPoint = 0, playerPoint = 0;
				int ID = 0;

				bool runGame = true;
				while (runGame == true) {
					while (SDL_PollEvent(&e) != 0) {
						if (e.type == SDL_QUIT) {
							startNewGameBut->setPressed(false);
							runGame = false;
							runProgramm = false;
						}
						if (upBetBut->mouse_event(e) == true && upBetBut->getActive() == true) {
							upBetBut->setPressed(false);
							if (bet < 1000) {
								bet += 5;
								betTemp = "Bet:" + to_string(bet) + "$";
								strcpy_s(betStr, betTemp.c_str());
								updateTexture(&render, &tableBackground, tableRect);
								SDL_RenderFillRect(render.get(), &controlPanel);
								updateTexture(&render, &upBetTexture, upBetBut->getbox());
								updateTexture(&render, &lowBetTexture, lowBetBut->getbox());
								updateTexture(&render, &betTexture, betBut->getbox());
								updateTexture(&render, &standTexture, standBut->getbox());
								updateTexture(&render, &hitTexture, hitBut->getbox());
								updateTexture(&render, &mainMenuTexture, mainMenuBut->getbox());
								updateTexture(&render, &deckTexture, Deck);
								updateTexture(&render, &cashLabelTexture, cashLabelRect);
								updateTexture(&render, &cashLabelTexture_num, cashLabelRect_num);
								print_ttf(&render, &betLabel, betStr, "times.ttf", 200, kBlack, betLabelRect);
							}
						}
						if (lowBetBut->mouse_event(e) == true && lowBetBut->getActive() == true) {
							lowBetBut->setPressed(false);
							if (bet != 0) {
								bet -= 5;
								betTemp = "Bet:" + to_string(bet) + "$";
								strcpy_s(betStr, betTemp.c_str());
								updateTexture(&render, &tableBackground, tableRect);
								SDL_RenderFillRect(render.get(), &controlPanel);
								updateTexture(&render, &upBetTexture, upBetBut->getbox());
								updateTexture(&render, &lowBetTexture, lowBetBut->getbox());
								updateTexture(&render, &betTexture, betBut->getbox());
								updateTexture(&render, &standTexture, standBut->getbox());
								updateTexture(&render, &hitTexture, hitBut->getbox());
								updateTexture(&render, &mainMenuTexture, mainMenuBut->getbox());
								updateTexture(&render, &deckTexture, Deck);
								updateTexture(&render, &cashLabelTexture, cashLabelRect);
								updateTexture(&render, &cashLabelTexture_num, cashLabelRect_num);
								print_ttf(&render, &betLabel, betStr, "times.ttf", 200, kBlack, betLabelRect);
							}
						}
						if (betBut->mouse_event(e) == true && betBut->getActive() == true) {
							SDL_RenderClear(render.get());

							dealerPoint = 0, playerPoint = 0, ID = 0;
							cash -= bet;
							print_ttf(&render, &cashLabelTexture_num, cash, "times.ttf", 200, kBlack, cashLabelRect_num);

							vector<unique_ptr<Card>>().swap(dealerHand);
							vector<unique_ptr<SDL_Texture, sdl_deleter>>().swap(dealerHandTexture);
							vector<char*>().swap(dealerWorkArr);

							vector<unique_ptr<Card>>().swap(playerHand);
							vector<unique_ptr<SDL_Texture, sdl_deleter>>().swap(playerHandTexture);

							betBut->setPressed(false);
							upBetBut->setActive(false); lowBetBut->setActive(false); betBut->setActive(false);
							createTexture(&render, &upBetTexture, upBetBut->getbox(), "Texture/controlPanel/upBetLock.png");
							createTexture(&render, &lowBetTexture, lowBetBut->getbox(), "Texture/controlPanel/downBetLock.png");
							createTexture(&render, &betTexture, betBut->getbox(), "Texture/controlPanel/betLock.png");

							standBut->setActive(true); hitBut->setActive(true);
							createTexture(&render, &standTexture, standBut->getbox(), "Texture/controlPanel/stand.png");
							createTexture(&render, &hitTexture, hitBut->getbox(), "Texture/controlPanel/hit.png");

							auto rnd = default_random_engine(seed);
							shuffle(begin(deckID), end(deckID), rnd);

							//Раздача карт диллеру
							for (int q = 0; q < 2; ++q, ++ID) {
								auto iterator = cardDeck.begin();
								advance(iterator, deckID[ID]);
								if (iterator->first == "aceDiamonds" || iterator->first == "aceHearts" || iterator->first == "aceSpades" || iterator->first == "aceClubs") {
									dealerHand.push_back(unique_ptr<Card>(new Ace(SCREEN_WIDTH - (SCREEN_WIDTH / 5), 0 + (SCREEN_HEIGHT / 30), SCREEN_WIDTH / (scale * 2), 1.3 * SCREEN_HEIGHT / scale, iterator->first, iterator->second)));
									dealerHandTexture.push_back(unique_ptr<SDL_Texture, sdl_deleter>());
								}
								else {
									dealerHand.push_back(unique_ptr<Card>(new Card(SCREEN_WIDTH - (SCREEN_WIDTH / 5), 0 + (SCREEN_HEIGHT / 30), SCREEN_WIDTH / (scale * 2), 1.3 * SCREEN_HEIGHT / scale, iterator->first, iterator->second)));
									dealerHandTexture.push_back(unique_ptr<SDL_Texture, sdl_deleter>());
								}

								auto f = [&dealerHand, q]()->int {
									if (dealerHand.size() > 1) {
										return (dealerHand[(q - 1)].get()->getBox().w / 2);
									}
									else return 0;
								};

								while (dealerHand[q].get()->getBox().x > (SCREEN_WIDTH / 3) + f())
								{
									updateTexture(&render, &tableBackground, tableRect);
									SDL_RenderFillRect(render.get(), &controlPanel);
									if (dealerHand.size() > 1) {
										updateTexture(&render, &dealerHandTexture[(q-1)], dealerHand[(q-1)].get()->getBox());
									}
									auto animationX = dealerHand[q].get()->getBox().x;
									animationX -= 70;
									dealerHand[q].get()->setBox(animationX, dealerHand[q].get()->getBox().y, dealerHand[q].get()->getBox().w, dealerHand[0].get()->getBox().h);
									createTexture(&render, &dealerHandTexture[q], dealerHand[q].get()->getBox(), "Texture/deck/top.png");

									updateTexture(&render, &deckTexture, Deck);
									updateTexture(&render, &upBetTexture, upBetBut->getbox());
									updateTexture(&render, &lowBetTexture, lowBetBut->getbox());
									updateTexture(&render, &betTexture, betBut->getbox());
									updateTexture(&render, &standTexture, standBut->getbox());
									updateTexture(&render, &hitTexture, hitBut->getbox());
									updateTexture(&render, &mainMenuTexture, mainMenuBut->getbox());
									updateTexture(&render, &betLabel, betLabelRect);
									updateTexture(&render, &cashLabelTexture, cashLabelRect);
									updateTexture(&render, &cashLabelTexture_num, cashLabelRect_num);
								}
								string temp = "Texture/deck/" + dealerHand[q].get()->getName() + ".png";
								char tempStr[30];
								strcpy_s(tempStr, temp.c_str());
								dealerWorkArr.push_back(tempStr);

								if (dealerHand[q].get()->getvalue() == 0) dealerHand[q].get()->setvalue(dealerHand[q].get()->betterChoice(dealerPoint));
								if (q == 0) {
								    createTexture(&render, &dealerHandTexture[q], dealerHand[q].get()->getBox(), dealerWorkArr[q]);
									dealerPoint += dealerHand[q].get()->getvalue();
							    }
							}
							print_ttf(&render, &dealerPointTexture, dealerPoint, "times.ttf", 200, kBlack, dealerPointRect);

							//Раздача карт игроку
							for (int q = 0; q < 2; ++q, ++ID) {
								auto iterator = cardDeck.begin();
								advance(iterator, deckID[ID]);
								if (iterator->first == "aceDiamonds" || iterator->first == "aceHearts" || iterator->first == "aceSpades" || iterator->first == "aceClubs") {
									playerHand.push_back(unique_ptr<Card>(new Ace(SCREEN_WIDTH - (SCREEN_WIDTH / 5), 0 + (SCREEN_HEIGHT / 30), SCREEN_WIDTH / (scale * 2), 1.3 * SCREEN_HEIGHT / scale, iterator->first, iterator->second)));
									playerHandTexture.push_back(unique_ptr<SDL_Texture, sdl_deleter>());
								}
								else {
									playerHand.push_back(unique_ptr<Card>(new Card(SCREEN_WIDTH - (SCREEN_WIDTH / 5), 0 + (SCREEN_HEIGHT / 30), SCREEN_WIDTH / (scale * 2), 1.3 * SCREEN_HEIGHT / scale, iterator->first, iterator->second)));
									playerHandTexture.push_back(unique_ptr<SDL_Texture, sdl_deleter>());
								}

								auto f = [&playerHand, q]()->int {
									if (playerHand.size() > 1) {
										return (playerHand[(q - 1)].get()->getBox().w / 2);
									}
									else return 0;
								};

								while (playerHand[q].get()->getBox().x > (SCREEN_WIDTH / 3) + f() || playerHand[q].get()->getBox().y < (SCREEN_HEIGHT- (SCREEN_HEIGHT / 2)))
								{
									updateTexture(&render, &tableBackground, tableRect);
									SDL_RenderFillRect(render.get(), &controlPanel);
									if (playerHand.size() > 1) {
										updateTexture(&render, &playerHandTexture[(q - 1)], playerHand[(q - 1)].get()->getBox());
									}
									for (int t = 0; t < dealerHand.size(); ++t) {
										updateTexture(&render, &dealerHandTexture[t], dealerHand[t].get()->getBox());
									}
									auto animationX = playerHand[q].get()->getBox().x;
									auto animationY = playerHand[q].get()->getBox().y;
									if (playerHand[q].get()->getBox().x > (SCREEN_WIDTH / 3) + f()) {
										animationX -= 70;
									}
									if (playerHand[q].get()->getBox().y < (SCREEN_HEIGHT - (SCREEN_HEIGHT / 2))) {
										animationY += 50;
									}
									playerHand[q].get()->setBox(animationX, animationY, playerHand[q].get()->getBox().w, playerHand[0].get()->getBox().h);
									createTexture(&render, &playerHandTexture[q], playerHand[q].get()->getBox(), "Texture/deck/top.png");

									updateTexture(&render, &deckTexture, Deck);
									updateTexture(&render, &upBetTexture, upBetBut->getbox());
									updateTexture(&render, &lowBetTexture, lowBetBut->getbox());
									updateTexture(&render, &betTexture, betBut->getbox());
									updateTexture(&render, &standTexture, standBut->getbox());
									updateTexture(&render, &hitTexture, hitBut->getbox());
									updateTexture(&render, &mainMenuTexture, mainMenuBut->getbox());
									updateTexture(&render, &betLabel, betLabelRect);
									updateTexture(&render, &dealerPointTexture, dealerPointRect);
									updateTexture(&render, &cashLabelTexture, cashLabelRect);
									updateTexture(&render, &cashLabelTexture_num, cashLabelRect_num);
								}
								string temp = "Texture/deck/" + playerHand[q].get()->getName() + ".png";
								char tempStr[30];
								strcpy_s(tempStr, temp.c_str());

								createTexture(&render, &playerHandTexture[q], playerHand[q].get()->getBox(), tempStr);
								if (playerHand[q].get()->getvalue() == 0) playerHand[q].get()->setvalue(playerHand[q].get()->betterChoice(playerPoint));
								playerPoint += playerHand[q].get()->getvalue();
							}
							print_ttf(&render, &playerPointTexture, playerPoint, "times.ttf", 200, kBlack, playerPointRect);
						}

						if (hitBut->mouse_event(e) == true && hitBut->getActive() == true) {
							hitBut->setPressed(false);
							auto iterator = cardDeck.begin();
							advance(iterator, deckID[ID]);
							if (iterator->first == "aceDiamonds" || iterator->first == "aceHearts" || iterator->first == "aceSpades" || iterator->first == "aceClubs") {
								playerHand.push_back(unique_ptr<Card>(new Ace(SCREEN_WIDTH - (SCREEN_WIDTH / 5), 0 + (SCREEN_HEIGHT / 30), SCREEN_WIDTH / (scale * 2), 1.3 * SCREEN_HEIGHT / scale, iterator->first, iterator->second)));
								playerHandTexture.push_back(unique_ptr<SDL_Texture, sdl_deleter>());
							}
							else {
								playerHand.push_back(unique_ptr<Card>(new Card(SCREEN_WIDTH - (SCREEN_WIDTH / 5), 0 + (SCREEN_HEIGHT / 30), SCREEN_WIDTH / (scale * 2), 1.3 * SCREEN_HEIGHT / scale, iterator->first, iterator->second)));
								playerHandTexture.push_back(unique_ptr<SDL_Texture, sdl_deleter>());
							}
							while (playerHand.back().get()->getBox().x > (SCREEN_WIDTH / 3) + ((Deck.w/2)*(playerHand.size()-1)) || playerHand.back().get()->getBox().y < (SCREEN_HEIGHT - (SCREEN_HEIGHT / 2)))
							{
								updateTexture(&render, &tableBackground, tableRect);
								SDL_RenderFillRect(render.get(), &controlPanel);
								for (int i = 0; i < playerHand.size(); ++i) {
									updateTexture(&render, &playerHandTexture[i], playerHand[i].get()->getBox());
								}
								for (int t = 0; t < dealerHand.size(); ++t) {
									updateTexture(&render, &dealerHandTexture[t], dealerHand[t].get()->getBox());
								}
								auto animationX = playerHand.back().get()->getBox().x;
								auto animationY = playerHand.back().get()->getBox().y;
								if (playerHand.back().get()->getBox().x > (SCREEN_WIDTH / 3) + ((Deck.w / 2) * (playerHand.size()-1))) {
									animationX -= 70;
								}
								if (playerHand.back().get()->getBox().y < (SCREEN_HEIGHT - (SCREEN_HEIGHT / 2))) {
									animationY += 50;
								}
								playerHand.back().get()->setBox(animationX, animationY, playerHand.back().get()->getBox().w, playerHand[0].get()->getBox().h);
								createTexture(&render, &playerHandTexture.back(), playerHand.back().get()->getBox(), "Texture/deck/top.png");

								updateTexture(&render, &deckTexture, Deck);
								updateTexture(&render, &upBetTexture, upBetBut->getbox());
								updateTexture(&render, &lowBetTexture, lowBetBut->getbox());
								updateTexture(&render, &betTexture, betBut->getbox());
								updateTexture(&render, &standTexture, standBut->getbox());
								updateTexture(&render, &hitTexture, hitBut->getbox());
								updateTexture(&render, &mainMenuTexture, mainMenuBut->getbox());
								updateTexture(&render, &betLabel, betLabelRect);
								updateTexture(&render, &dealerPointTexture, dealerPointRect);
								updateTexture(&render, &cashLabelTexture, cashLabelRect);
								updateTexture(&render, &cashLabelTexture_num, cashLabelRect_num);
							}
							string temp = "Texture/deck/" + playerHand.back().get()->getName() + ".png";
							char tempStr[30];
							strcpy_s(tempStr, temp.c_str());

							createTexture(&render, &playerHandTexture.back(), playerHand.back().get()->getBox(), tempStr);
							if (playerHand.back().get()->getvalue() == 0) playerHand.back().get()->setvalue(playerHand.back().get()->betterChoice(playerPoint));
							playerPoint += playerHand.back().get()->getvalue();
							print_ttf(&render, &playerPointTexture, playerPoint, "times.ttf", 200, kBlack, playerPointRect);
						}

						if ((standBut->mouse_event(e) == true || playerPoint > 21 || dealerPoint > 21) && standBut->getActive() == true) {
							standBut->setPressed(false);
							standBut->setActive(false); hitBut->setActive(false);
							createTexture(&render, &standTexture, standBut->getbox(), "Texture/controlPanel/standLock.png");
							createTexture(&render, &hitTexture, hitBut->getbox(), "Texture/controlPanel/hitLock.png");

							betBut->setActive(true);  upBetBut->setActive(true); lowBetBut->setActive(true);
							createTexture(&render, &betTexture, betBut->getbox(), "Texture/controlPanel/bet.png");
							createTexture(&render, &upBetTexture, upBetBut->getbox(), "Texture/controlPanel/upBet.png");
							createTexture(&render, &lowBetTexture, lowBetBut->getbox(), "Texture/controlPanel/downBet.png");

							createTexture(&render, &dealerHandTexture[1], dealerHand[1].get()->getBox(), dealerWorkArr[1]);
							dealerPoint += dealerHand[1].get()->getvalue();

							SDL_RenderClear(render.get());

							updateTexture(&render, &tableBackground, tableRect);
							SDL_RenderFillRect(render.get(), &controlPanel);
							for (int i = 0; i < dealerHand.size(); ++i) {
								updateTexture(&render, &dealerHandTexture[i], dealerHand[i].get()->getBox());
							}
							for (int i = 0; i < playerHand.size(); ++i) {
								updateTexture(&render, &playerHandTexture[i], playerHand[i].get()->getBox());
							}
							updateTexture(&render, &upBetTexture, upBetBut->getbox());
							updateTexture(&render, &lowBetTexture, lowBetBut->getbox());
							updateTexture(&render, &betTexture, betBut->getbox());
							updateTexture(&render, &standTexture, standBut->getbox());
							updateTexture(&render, &hitTexture, hitBut->getbox());
							updateTexture(&render, &mainMenuTexture, mainMenuBut->getbox());
							updateTexture(&render, &deckTexture, Deck);
							updateTexture(&render, &betLabel, betLabelRect);
							print_ttf(&render, &dealerPointTexture, dealerPoint, "times.ttf", 200, kBlack, dealerPointRect);
							updateTexture(&render, &playerPointTexture, playerPointRect);
							updateTexture(&render, &cashLabelTexture, cashLabelRect);
							updateTexture(&render, &cashLabelTexture_num, cashLabelRect_num);

							int chance = (21 - dealerPoint) * 5;
							default_random_engine rnd(seed);
							uniform_int_distribution<int> dealer(0, 100);

							while (dealerPoint <= 20) {
								if (dealer(rnd) <= chance) {
									auto iterator = cardDeck.begin();
									advance(iterator, deckID[ID]);
									if (iterator->first == "aceDiamonds" || iterator->first == "aceHearts" || iterator->first == "aceSpades" || iterator->first == "aceClubs") {
										dealerHand.push_back(unique_ptr<Card>(new Ace(SCREEN_WIDTH - (SCREEN_WIDTH / 5), 0 + (SCREEN_HEIGHT / 30), SCREEN_WIDTH / (scale * 2), 1.3 * SCREEN_HEIGHT / scale, iterator->first, iterator->second)));
										dealerHandTexture.push_back(unique_ptr<SDL_Texture, sdl_deleter>());
									}
									else {
										dealerHand.push_back(unique_ptr<Card>(new Card(SCREEN_WIDTH - (SCREEN_WIDTH / 5), 0 + (SCREEN_HEIGHT / 30), SCREEN_WIDTH / (scale * 2), 1.3 * SCREEN_HEIGHT / scale, iterator->first, iterator->second)));
										dealerHandTexture.push_back(unique_ptr<SDL_Texture, sdl_deleter>());
									}
									while (dealerHand.back()->getBox().x > (SCREEN_WIDTH / 3) + dealerHand.at((dealerHand.size() - 1)).get()->getBox().w / 2)
									{
										updateTexture(&render, &tableBackground, tableRect);
										SDL_RenderFillRect(render.get(), &controlPanel);
										
										auto animationX = dealerHand.back().get()->getBox().x;
										animationX -= 70;
										dealerHand.back().get()->setBox(animationX, dealerHand.back().get()->getBox().y, dealerHand.back().get()->getBox().w, dealerHand[0].get()->getBox().h);
										createTexture(&render, &dealerHandTexture.back(), dealerHand.back().get()->getBox(), "Texture/deck/top.png");

										for(int i = 0; i < dealerHand.size(); ++i) {
											updateTexture(&render, &dealerHandTexture[i], dealerHand[i].get()->getBox());
										}
										for (int i = 0; i < playerHand.size(); ++i) {
											updateTexture(&render, &playerHandTexture[i], playerHand[i].get()->getBox());
										}

										updateTexture(&render, &deckTexture, Deck);
										updateTexture(&render, &upBetTexture, upBetBut->getbox());
										updateTexture(&render, &lowBetTexture, lowBetBut->getbox());
										updateTexture(&render, &betTexture, betBut->getbox());
										updateTexture(&render, &standTexture, standBut->getbox());
										updateTexture(&render, &hitTexture, hitBut->getbox());
										updateTexture(&render, &mainMenuTexture, mainMenuBut->getbox());
										print_ttf(&render, &betLabel, betStr, "times.ttf", 200, kBlack, betLabelRect);
										print_ttf(&render, &playerPointTexture, playerPoint, "times.ttf", 200, kBlack, playerPointRect);
									}
									string temp = "Texture/deck/" + dealerHand.back().get()->getName() + ".png";
									char tempStr[30];
									strcpy_s(tempStr, temp.c_str());
									dealerWorkArr.push_back(tempStr);
									createTexture(&render, &dealerHandTexture.back(), dealerHand.back().get()->getBox(), dealerWorkArr.back());
									if (dealerHand.back().get()->getvalue() == 0) dealerHand.back().get()->setvalue(dealerHand.back().get()->betterChoice(dealerPoint));
									dealerPoint += dealerHand.back().get()->getvalue();
									print_ttf(&render, &dealerPointTexture, dealerPoint, "times.ttf", 200, kBlack, dealerPointRect);
								}
								else break;
							}

							if (dealerPoint > playerPoint) {
								bet = 0;
								updateTexture(&render, &tableBackground, tableRect);
								SDL_RenderFillRect(render.get(), &controlPanel);
								for (int i = 0; i < dealerHand.size(); ++i) {
									updateTexture(&render, &dealerHandTexture[i], dealerHand[i].get()->getBox());
								}
								for (int i = 0; i < dealerHand.size(); ++i) {
									updateTexture(&render, &playerHandTexture[i], playerHand[i].get()->getBox());
								}
								updateTexture(&render, &upBetTexture, upBetBut->getbox());
								updateTexture(&render, &lowBetTexture, lowBetBut->getbox());
								updateTexture(&render, &betTexture, betBut->getbox());
								updateTexture(&render, &standTexture, standBut->getbox());
								updateTexture(&render, &hitTexture, hitBut->getbox());
								updateTexture(&render, &mainMenuTexture, mainMenuBut->getbox());
								updateTexture(&render, &deckTexture, Deck);
								print_ttf(&render, &betLabel, betStr, "times.ttf", 200, kBlack, betLabelRect);
								updateTexture(&render, &dealerPointTexture, dealerPointRect);
								updateTexture(&render, &playerPointTexture, playerPointRect);
								updateTexture(&render, &cashLabelTexture, cashLabelRect);
								updateTexture(&render, &cashLabelTexture_num, cashLabelRect_num);
							}
							else if (dealerPoint < playerPoint) {

								cash += (2 * bet);
								bet = 0;

								bet = 0;
								updateTexture(&render, &tableBackground, tableRect);
								SDL_RenderFillRect(render.get(), &controlPanel);
								for (int i = 0; i < dealerHand.size(); ++i) {
									updateTexture(&render, &dealerHandTexture[i], dealerHand[i].get()->getBox());
								}
								for (int i = 0; i < playerHand.size(); ++i) {
									updateTexture(&render, &playerHandTexture[i], playerHand[i].get()->getBox());
								}
								updateTexture(&render, &upBetTexture, upBetBut->getbox());
								updateTexture(&render, &lowBetTexture, lowBetBut->getbox());
								updateTexture(&render, &betTexture, betBut->getbox());
								updateTexture(&render, &standTexture, standBut->getbox());
								updateTexture(&render, &hitTexture, hitBut->getbox());
								updateTexture(&render, &mainMenuTexture, mainMenuBut->getbox());
								updateTexture(&render, &deckTexture, Deck);
								print_ttf(&render, &betLabel, betStr, "times.ttf", 200, kBlack, betLabelRect);
								updateTexture(&render, &dealerPointTexture, dealerPointRect);
								updateTexture(&render, &playerPointTexture, playerPointRect);
								updateTexture(&render, &cashLabelTexture, cashLabelRect);
								print_ttf(&render, &cashLabelTexture_num, cash, "times.ttf", 200, kBlack, cashLabelRect_num);
							}
						}
						if (mainMenuBut->mouse_event(e) == true) {
							mainMenuBut->setPressed(false);
							runGame = false;
							SDL_RenderClear(render.get());
							vector<unique_ptr<Card>>().swap(dealerHand);
							vector<unique_ptr<SDL_Texture, sdl_deleter>>().swap(dealerHandTexture);
							vector<unique_ptr<Card>>().swap(playerHand);
							vector<unique_ptr<SDL_Texture, sdl_deleter>>().swap(playerHandTexture);

							startNewGameBut->showbox(); optionsBut->showbox(); exitBut->showbox();
							createTexture(&render, &tableBackground, tableRect, "Texture/pokerTable.png");
							createTexture(&render, &startNewGameTexrure, startNewGameBut->getbox(), "Texture/mainMenu/startNewGame.png");
							createTexture(&render, &optionTexture, optionsBut->getbox(), "Texture/mainMenu/options.png");
							createTexture(&render, &exitTexture, exitBut->getbox(), "Texture/mainMenu/exit.png");
						}
					}
				}
			}
		}
	}
#pragma endregion

	TTF_Quit();
	SDL_Quit();

	return 0;
}

//-------------------------------------------------------------------------------------------------------------------------------------

bool init(const char* name, unique_ptr<SDL_Window, sdl_deleter>*win, unique_ptr<SDL_Surface, sdl_deleter>*screen, int width, int height, SDL_WindowFlags flag ) {
	bool success = true;
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		success = false;
	}

	else {
		int flags = IMG_INIT_PNG;
		if (!(IMG_Init(flags) & flags)) {
			cout << "Can't initialize image" << IMG_GetError() << endl;
			success = false;
		}
			
		*win = static_cast<unique_ptr<SDL_Window, sdl_deleter>>(SDL_CreateWindow(name, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, flag));

		if (&win == NULL)
		{
			cout << "Window could not be created! SDL_Error: " << SDL_GetError();
			success = false;
		}
		else {
			*screen = static_cast<unique_ptr<SDL_Surface, sdl_deleter>>(SDL_GetWindowSurface(win->get()));
			SDL_FillRect(screen->get(), nullptr, SDL_MapRGB((*screen)->format, 0xFF, 0xFF, 0xFF));
			SDL_UpdateWindowSurface(win->get());
		}
	}
	return success;
}

bool initRender(unique_ptr<SDL_Renderer, sdl_deleter>* ren, unique_ptr<SDL_Window, sdl_deleter>*win) {
	bool success = true;
	*ren = static_cast<unique_ptr<SDL_Renderer, sdl_deleter>>(SDL_CreateRenderer(win->get(), -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC));    //Использование аппаратного ускорения + вертикальная синхронизация
	if (*ren == NULL) {
		cout << "Can't create render: " << SDL_GetError() << endl;
		success = false;
	}
	SDL_SetRenderDrawColor(ren->get(), 0xFF, 0xFF, 0xFF, 0xFF);
	return success;
}

void createTexture(unique_ptr<SDL_Renderer, sdl_deleter>* ren, unique_ptr<SDL_Texture, sdl_deleter>* texture, SDL_Rect rect, const char* name) {
	bool success = true;
	SDL_Surface* tempSur = NULL;

	tempSur = IMG_Load(name);
	if (tempSur == NULL) {
		cout << "Can't load image: " << IMG_GetError() << endl;
		success = false;
	}

	*texture = static_cast<unique_ptr<SDL_Texture, sdl_deleter>>(SDL_CreateTextureFromSurface(ren->get(), tempSur));
	if (*texture == NULL) {
		cout << "Can't create texture: " << SDL_GetError() << endl;
		success = false;
	}
	SDL_FreeSurface(tempSur);
	
	try {
		if (success == false) throw -1;

		SDL_RenderSetViewport(ren->get(), &rect);
		SDL_RenderCopy(ren->get(), texture->get(), NULL, NULL);
		SDL_RenderPresent(ren->get());
    }
	catch (int a) { cout << "Initialize texture " << "\"" << name << "\"" << " result: " << a << endl; }
}

void updateTexture(unique_ptr<SDL_Renderer, sdl_deleter>* ren, unique_ptr<SDL_Texture, sdl_deleter>* texture, SDL_Rect rect) {
	SDL_RenderSetViewport(ren->get(), &rect);
	SDL_RenderCopy(ren->get(), texture->get(), NULL, NULL);
	SDL_RenderPresent(ren->get());
}

void print_ttf(unique_ptr<SDL_Renderer, sdl_deleter>* renderer, unique_ptr<SDL_Texture, sdl_deleter>* texture, const char* message, const char* font, int size, SDL_Color color, SDL_Rect dest) {
	TTF_Font* fnt = TTF_OpenFont(font, size);
	try { if (fnt == nullptr) throw -1; }
	catch (...) { cout << "Font loading error: " << TTF_GetError() << endl; }

	unique_ptr<SDL_Surface, sdl_deleter> sText = unique_ptr<SDL_Surface, sdl_deleter>(TTF_RenderText_Blended(fnt, message, color));
	*texture = static_cast<unique_ptr<SDL_Texture, sdl_deleter>>(SDL_CreateTextureFromSurface(renderer->get(), sText.get()));

	SDL_RenderSetViewport(renderer->get(), &dest);
	SDL_RenderCopy(renderer->get(), texture->get(), NULL, NULL);
	SDL_RenderPresent(renderer->get());
    
	TTF_CloseFont(fnt);
}

void print_ttf(unique_ptr<SDL_Renderer, sdl_deleter>* renderer, unique_ptr<SDL_Texture, sdl_deleter>* texture, int num, const char* font, int size, SDL_Color color, SDL_Rect dest) {
	TTF_Font* fnt = TTF_OpenFont(font, size);
	try { if (fnt == nullptr) throw - 1; }
	catch (...) { cout << "Font loading error: " << TTF_GetError() << endl; }

	string temp = to_string(num);
	char message[10];
	strcpy_s(message, temp.c_str());

	unique_ptr<SDL_Surface, sdl_deleter> sText = unique_ptr<SDL_Surface, sdl_deleter>(TTF_RenderText_Blended(fnt, message, color));
	*texture = static_cast<unique_ptr<SDL_Texture, sdl_deleter>>(SDL_CreateTextureFromSurface(renderer->get(), sText.get()));

	SDL_RenderSetViewport(renderer->get(), &dest);
	SDL_RenderCopy(renderer->get(), texture->get(), NULL, NULL);
	SDL_RenderPresent(renderer->get());

	TTF_CloseFont(fnt);
}

void closeWindow(unique_ptr<SDL_Window, sdl_deleter>* win) {
	SDL_DestroyWindow(win->get());
	win = NULL;
}

//-------------------------------------------------------------------------------------------------------------------------------------