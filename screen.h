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

//bool loadFont(Font& f);
void drawText(RenderWindow& w, const Font& f, const string& s, unsigned size,float x, float y, Color c, bool center);
bool isMouseOver(RenderWindow& w, float x, float y, float width, float height);
//void drawRoundButton(RenderWindow& w, float x, float y, float width, float height,Color c, bool hover);

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

// screen id 4
class Level3Screen : public Screen {

    Font font;
    Texture backgroundTexture;
    Sprite background;
    int nextScreen;

    // game state
    TreeNode* root;
    int count;
    act1Level3 level;
    unsigned seed;

    float timeLeft;        
    float flashTime;       
    float messageTime;     
    float animTime;        
    string input;          
    string message;        
    Color messageColor;

    void newTree();
    void submitAnswer();

public:
    Level3Screen(RenderWindow& win);
    ~Level3Screen();
    void draw() override;
    void onResume() override;
    void handleInput() override;
    void update(float time) override;
    int getNextScreen() const override { return nextScreen; }
    void onEnter() override;
    //bool isFinished() const override { return level.getState() == act1Level3::WON; }
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

//



// screen id 5
class Level4Screen : public Screen {

    Font font;
    Texture backgroundTexture;
    Sprite background;
    int nextScreen;

    // game state
    TreeNode* root;
    int count;
    act1Level4 level;
    unsigned seed;

    int selectedLcaId;
    int selectedLcaVal;
    string pathInput;
    int activeBox;          
    float flashTime;
    float messageTime;
    float animTime;
    string message;
    Color messageColor;

    void newTree();
    void submitAnswer();

public:
    Level4Screen(RenderWindow& win);
    ~Level4Screen();
    void draw() override;
    void onResume() override;
    void handleInput() override;
    void update(float time) override;
    int getNextScreen() const override { return nextScreen; }
    void onEnter() override;
};

// screen id 6
class Level5Screen : public Screen {

    Font font;
    Texture backgroundTexture;
    Sprite background;
    int nextScreen;

    // game state
    act1Level5 level;
    int count;
    unsigned seed;

    int lastSwappedId;
    float flashTime;
    float messageTime;
    string message;
    Color messageColor;

    void newTree();
    void refreshLayout();

public:
    Level5Screen(RenderWindow& win);
    void draw() override;
    void onResume() override;
    void handleInput() override;
    void update(float time) override;
    int getNextScreen() const override { return nextScreen; }
    void onEnter() override;
};

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

// screen id 21

class instructionWindow  :public Screen
{
    Font font;
    Text instructions;
    Texture backgroundTexture;
    Sprite background;
    RectangleShape square;
    int nextScreen;
    int targetScreen;

public:
    instructionWindow(RenderWindow& window);
    void draw() override;
    void update(float time) override;
    int getNextScreen() const override { return nextScreen; }
    void onEnter() override;
    void handleInput() override;

    
    

};