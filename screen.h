#pragma once
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include "header.h"
#include "act1Level1.h"
using namespace sf;
using namespace std;

// shared helpers (defined in screen.cpp)
bool loadFont(sf::Font& f);
void drawText(sf::RenderWindow& w, const sf::Font& f, const std::string& s, unsigned size,
    float x, float y, sf::Color c, bool center);
bool isMouseOver(sf::RenderWindow& w, float x, float y, float width, float height);
void drawRoundButton(sf::RenderWindow& w, float x, float y, float width, float height,Color c, bool hover);

class Screen {
protected:
    RenderWindow& window;
    bool isActive;

public:
    Screen(RenderWindow& win) : window(win), isActive(false) {}
    virtual ~Screen() = default;

    virtual void draw() = 0;
    virtual void handleInput() = 0;
    virtual void update(float time) = 0;
    virtual int getNextScreen() const = 0;

    virtual void onEnter() { isActive = true; }
    virtual void onExit() { isActive = false; }
};

// screen id 1
class MainMenuScreen : public Screen {
    Font font;
    Text title;
    Text buttonText;
    CircleShape button;
    Texture backgroundTexture;
    Sprite background;
    int nextScreen;
public:
    MainMenuScreen(RenderWindow& win);
    void draw() override;
    void handleInput() override;
    void update(float time) override;
    int getNextScreen() const override { return nextScreen; }
    void onEnter() override;
};

// screen id 2
class Level1Screen : public Screen {

    Font font;
    Texture backgroundTexture;
    Sprite background;
    int nextScreen;

    // game state
    TreeNode* root;
    int count;
    act1Level1 level;
    int curType;
    unsigned seed;
    float flashTime;

    void newTree();


public:
    Level1Screen(RenderWindow& win);
    ~Level1Screen();
    void draw() override;
    void handleInput() override;
    void update(float time) override;
    int getNextScreen() const override { return nextScreen; }
    void onEnter() override;
};

// screen id 3
class Level2Screen : public Screen {
   
    static const int W = 1280;
    static const int H = 720;

    sf::Font font;
    int nextScreen;
    act1Level2 level;

    float px[15], py[15];       // current position of each room
    float homeX[15], homeY[15]; // tray position of each room
    float radius, portR;
    int mode;
    int drag;                   // room being dragged, -1 = none
    float offX, offY;
    int portFrom, portSide;     // room/port a link is being dragged from, -1 = none
    std::string message;
    float msgTime;
    unsigned seed;

    void newPuzzle(int m);
    void placePieces();
    void moveSubtree(int i, float dx, float dy);
    int hitNode(float mx, float my) const;
    bool hitPort(float mx, float my, int& piece, int& side) const;
    void portPos(int i, int side, float& x, float& y) const;
    void setMsg(const std::string& s);
    void doCheck();

public:
    Level2Screen(sf::RenderWindow& win);
    void draw() override;
    void handleInput() override;
    void update(float time) override;
    int getNextScreen() const override { return nextScreen; }
    void onEnter() override;
};

//screen id 24

class Map : public ::Screen
{
    Font font;
    Text title;
    Texture backgroundTexture;
    Sprite background;
    int nextScreen;

public:
    Map(RenderWindow& window);
    void draw() override;
    void update(float time) override;
    int getNextScreen() const override { return nextScreen; }
    void onEnter() override;
    void handleInput() override;
};