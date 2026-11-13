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
void drawRoundButton(sf::RenderWindow& w, float x, float y, float width, float height,
    sf::Color c, bool hover);

class Screen {
protected:
    sf::RenderWindow& window;
    bool isActive;

public:
    Screen(sf::RenderWindow& win) : window(win), isActive(false) {}
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
    static const int W = 1280;
    static const int H = 720;

    sf::Font font;
    int nextScreen;

    // tree layout (plain arrays, indexed by node id)
    TreeNode* byId[101];
    float px[101], py[101];
    int slotOf[101], depthOf[101];
    int slotCounter, maxDepth;
    float radius;

    // game state
    TreeNode* root;
    int count;
    act1Level1 level;
    int curType;
    unsigned seed;
    int flashId;
    float flashTime;

    void assign(TreeNode* r, int d);
    void layoutTree();
    int hitTest(float mx, float my) const;
    void newTree();

public:
    Level1Screen(sf::RenderWindow& win);
    ~Level1Screen();
    void draw() override;
    void handleInput() override;
    void update(float time) override;
    int getNextScreen() const override { return nextScreen; }
    void onEnter() override;
};