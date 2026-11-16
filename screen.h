#pragma once
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include "header.h"
#include "act1.h"
using namespace sf;
using namespace std;

// shared helpers (defined in screen.cpp)
bool loadFont(Font& f);
void drawText(RenderWindow& w, const Font& f, const string& s, unsigned size,float x, float y, Color c, bool center);
bool isMouseOver(RenderWindow& w, float x, float y, float width, float height);
void drawRoundButton(RenderWindow& w, float x, float y, float width, float height,Color c, bool hover);

class Screen {
protected:
    RenderWindow& window;
    int levelNumber;
    bool isActive;
    

public:
    Screen(RenderWindow& win) : window(win), isActive(false) {}
    virtual ~Screen() = default;

    virtual void draw() = 0;
    virtual void handleInput() = 0;
    virtual void update(float time) = 0;
    virtual int getNextScreen() const = 0;
    virtual void onResume() { isActive = true; }
    virtual void onEnter() { isActive = true; }
    virtual void onExit() { isActive = false; }
};

// screen id 1
class MainMenuScreen : public Screen {
    Font font;
    Font font2;
    Text title;
    Text buttontext2;;
    //int level;
    Text storyLine;
    Text buttonText;

    CircleShape button;
    CircleShape instructionButton;
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
    void onResume() override;
    void handleInput() override;
    void update(float time) override;
    int getNextScreen() const override { return nextScreen; }
    void onEnter() override;
};

// screen id 3
class Level2Screen : public Screen {

    Font font;
    Texture backgroundTexture;
    Sprite background;
    int nextScreen;

    // game state
    act1Level2 level;
    float nodeX[15];
    float nodeY[15];
    int selectedNode;
    int curMode;
    unsigned seed;
    float flashTime;
    string statusMsg;

    void newPuzzle();

public:
    Level2Screen(RenderWindow& win);
    void draw() override;
    void onResume() override;
    void handleInput() override;
    void update(float time) override;
    int getNextScreen() const override { return nextScreen; }
    void onEnter() override;
};
//screen id 4
//class Level3Screen : public Screen {
//
//    Font font;
//    Texture backgroundTexture;
//    Sprite background;
//    int nextScreen;
//
//    // game state
//    act1Level2 level;
//    float nodeX[15];
//    float nodeY[15];
//    int selectedNode;
//    int curMode;
//    unsigned seed;
//    float flashTime;
//    string statusMsg;
//
//    void newPuzzle();
//
//public:
//    Level3Screen(RenderWindow& win);
//    void draw() override;
//    void onResume() override;
//    void handleInput() override;
//    void update(float time) override;
//    int getNextScreen() const override { return nextScreen; }
//    void onEnter() override;
//};
//
////screen id 24
//
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

//
//screen id 25
class blackScreen :public::Screen {
    Font font;
    Text msg;    
    Texture backgroundTexture;
    Sprite background;
    int nextScreen;
    int targetScreen;

public:
    blackScreen(RenderWindow& window);
    void setTarget(int screenId);
    void setMessage(int level);
    void draw() override;
    void update(float time) override;
    int getNextScreen() const override { return nextScreen; }
    void onEnter() override;
    void handleInput() override;

};