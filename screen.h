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
#include "act4.h"
using namespace sf;
using namespace std;



bool isMouseOver(RenderWindow& w, float x, float y, float width, float height);

class Screen {
protected:
    RenderWindow& window;
    int levelNumber;
    bool isActive;


public:
    Screen(RenderWindow& win) : window(win), isActive(false) {}
    virtual ~Screen() = default;

    virtual void draw() = 0;
    virtual bool getResult(int& stars, int& score, int& alarm, int& penalties) const { return false; }
    virtual void handleInput() = 0;
    virtual void update(float time) = 0;
    virtual int getNextScreen() const = 0;
    virtual void onResume() { isActive = true; }
    virtual void onEnter() { isActive = true; }
    virtual void onExit() { isActive = false; }
};

class MainMenuScreen : public Screen {
    Font font;
    Font font2;
    Text title;
    Text storyLine;
    Text buttonText;
    Text buttontext2;
    Text leaderboardText;
    Text extraText;

    ConvexShape button;
    ConvexShape instructionButton;
    ConvexShape leaderboardButton;
    ConvexShape extraButton;

    RectangleShape storyBox;
    Texture borderTexture;
    Sprite borderSprite;

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
    Text hint;
    Font font;

    ConvexShape hintButton;

    float hintTime;
    int   hintValue;
    float hintX, hintY;
    Texture backgroundTexture;
    Sprite background;
    int nextScreen;
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
    void  showHint();
    void draw() override;
    void onResume() override;
    void handleInput() override;
    void update(float time) override;
    int getNextScreen() const override { return nextScreen; }
    void onEnter() override;
    bool getResult(int& st, int& sc, int& al, int& pe) const override
    {
        if (level.getState() != act1Level1::WON) return false;
        st = level.stars(); sc = level.score();
        al = level.getAlarm(); pe = level.getPenalties();
        return true;
    }
};

// screen id 3
class Level2Screen : public Screen {

    Font font;
    Texture backgroundTexture;
    Sprite background;
    int nextScreen;

    Text hint;
    ConvexShape hintButton;
    float hintTime;
    int hintP, hintSide, hintC;
    string hintMsg;

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
    void  showHint();
    void onResume() override;
    void handleInput() override;
    void update(float time) override;
    int getNextScreen() const override { return nextScreen; }
    void onEnter() override;
    bool getResult(int& st, int& sc, int& al, int& pe) const override
    {
        if (level.getState() != act1Level2::WON) return false;
        st = level.stars(); sc = level.score();
        al = level.getWrongChecks(); pe = level.getWrongChecks();
        return true;
    }
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

    Text hint;
    ConvexShape hintButton;
    float hintTime;
    string hintMsg;
    void showHint();
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
    // Level1Screen  (use act1Level3 / act1Level4 / act1Level5 for the others)
    bool getResult(int& st, int& sc, int& al, int& pe) const override
    {
        if (level.getState() != act1Level3::WON) return false;
        st = level.stars(); sc = level.score();
        al = level.getAlarm(); pe = level.getPenalties();
        return true;
    }
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


    Text hint;
    ConvexShape hintButton;
    float hintTime;
    string hintMsg;
    int hintStage;
    int hintLcaId;
    void showHint();

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
    // Level1Screen  (use act1Level3 / act1Level4 / act1Level5 for the others)
    bool getResult(int& st, int& sc, int& al, int& pe) const override
    {
        if (level.getState() != act1Level4::WON) return false;
        st = level.stars(); sc = level.score();
        al = level.getAlarm(); pe = level.getPenalties();
        return true;
    }
};

// screen id 6
class Level5Screen : public Screen {

    Font font;
    Texture backgroundTexture;
    Sprite background;
    int nextScreen;

    Text hint;
    ConvexShape hintButton;
    float hintTime;
    string hintMsg;
    int hintNodeId;
    void showHint();

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
    // Level1Screen  (use act1Level3 / act1Level4 / act1Level5 for the others)
    bool getResult(int& st, int& sc, int& al, int& pe) const override
    {
        if (level.getState() != act1Level5::WON) return false;
        st = level.stars(); sc = level.score();
        al = level.getAlarm(); pe = level.getPenalties();
        return true;
    }
};

//screen id 25
class blackScreen :public::Screen {
    Font font;
    Text msg;
    Texture backgroundTexture;
    Sprite background;
    int nextScreen;
    int targetScreen;
    ConvexShape menuButton;
    Text menuText;
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

class instructionWindow :public Screen
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



class Act4Level1Screen : public Screen {

    Font font;
    Texture backgroundTexture;
    Sprite background;
    int nextScreen;

    Text hint;
    ConvexShape hintButton;
    float hintTime;
    int hintP, hintSide, hintC;
    string hintMsg;
    void showHint();

    act4Level1 level;
    float nodeX[15];
    float nodeY[15];
    int selectedNode;
    unsigned seed;
    float flashTime;
    float animTime;
    string message;
    Color messageColor;

    void newPuzzle();
    void doCheck();

public:
    Act4Level1Screen(RenderWindow& win);
    void draw() override;
    void onResume() override;
    void handleInput() override;
    void update(float time) override;
    int getNextScreen() const override { return nextScreen; }
    void onEnter() override;
    bool getResult(int& st, int& sc, int& al, int& pe) const override
    {
        if (level.getState() != act4Level1::WON) return false;   
        st = level.stars(); sc = level.score();
        al = level.getAlarm(); pe = level.getPenalties();
        return true;
    }

};

// screen id 18
class Act4Level2Screen : public Screen {

    Font font;
    Texture backgroundTexture;
    Sprite background;
    int nextScreen;

    Text hint;
    ConvexShape hintButton;
    float hintTime;
    int hintA, hintB;
    string hintMsg;
    void showHint();

    act4Level2 level;
    float nodeX[15];
    float nodeY[15];
    int selectedNode;
    unsigned seed;
    float flashTime;
    float animTime;
    string message;
    Color messageColor;

    void newPuzzle();

public:
    Act4Level2Screen(RenderWindow& win);
    void draw() override;
    void onResume() override;
    void handleInput() override;
    void update(float time) override;
    int getNextScreen() const override { return nextScreen; }
    void onEnter() override;
    bool getResult(int& st, int& sc, int& al, int& pe) const override
    {
        if (level.getState() != act4Level2::WON) return false; 
        st = level.stars(); sc = level.score();
        al = level.getAlarm(); pe = level.getPenalties();
        return true;
    }
};

//------------------------level 3

// screen id 19
class Act4Level3Screen : public Screen {

    Font font;
    Texture backgroundTexture;
    Sprite background;
    int nextScreen;

    Text hint;
    ConvexShape hintButton;
    float hintTime;
    int hintA, hintB;
    string hintMsg;
    void showHint();

    Text removeRootText;
    ConvexShape removeRootButton;

    act4Level3 level;
    float nodeX[15];
    float nodeY[15];
    int selectedNode;
    unsigned seed;
    float flashTime;
    float animTime;
    string message;
    Color messageColor;

    void newPuzzle();
    void doRemoveRoot();

public:
    Act4Level3Screen(RenderWindow& win);
    void draw() override;
    void onResume() override;
    void handleInput() override;
    void update(float time) override;
    int getNextScreen() const override { return nextScreen; }
    void onEnter() override;
    bool getResult(int& st, int& sc, int& al, int& pe) const override
    {
        if (level.getState() != act4Level3::WON) return false;   
        st = level.stars(); sc = level.score();
        al = level.getAlarm(); pe = level.getPenalties();
        return true;
    }
};

//----------------------level 4
// screen id 20
class Act4Level4Screen : public Screen {

    Font font;
    Texture backgroundTexture;
    Sprite background;
    int nextScreen;

    Text hint;
    ConvexShape hintButton;
    float hintTime;
    int hintA, hintB;
    string hintMsg;
    void showHint();

    act4Level4 level;
    float nodeX[15];
    float nodeY[15];
    int selectedNode;
    unsigned seed;
    float flashTime;
    float animTime;
    string message;
    Color messageColor;

    void newPuzzle();

public:
    Act4Level4Screen(RenderWindow& win);
    void draw() override;
    void onResume() override;
    void handleInput() override;
    void update(float time) override;
    int getNextScreen() const override { return nextScreen; }
    void onEnter() override;
    bool getResult(int& st, int& sc, int& al, int& pe) const override
    {
        if (level.getState() != act4Level4::WON) return false;   
        st = level.stars(); sc = level.score();
        al = level.getAlarm(); pe = level.getPenalties();
        return true;
    }
};
//--------------act 4 level 5
// screen id 21
class Act4Level5Screen : public Screen {

    Font font;
    Texture backgroundTexture;
    Sprite background;
    int nextScreen;

    Text unlockText;
    ConvexShape unlockButton;


    act4Level5 level;
    float nodeX[15];
    float nodeY[15];
    int selectedNode;
    unsigned seed;
    float flashTime;
    float animTime;
    string message;
    Color messageColor;

    void newPuzzle();
    void doUnlock();

public:
    Act4Level5Screen(RenderWindow& win);
    void draw() override;
    void onResume() override;
    void handleInput() override;
    void update(float time) override;
    int getNextScreen() const override { return nextScreen; }
    void onEnter() override;
    bool getResult(int& st, int& sc, int& al, int& pe) const override
    {
        if (level.getState() != act4Level5::WON) return false;  
        st = level.stars(); sc = level.score();
        al = level.getAlarm(); pe = level.getPenalties();
        return true;
    }
};

//-------------------------------------------------------
//sand box

// screen id 23
class SandboxScreen : public Screen {
    int mode;         
   
        
    Font font;
    Texture backgroundTexture;
    Sprite background;
    int nextScreen;

    TreeNode* root;
    int nextId;
    maxHeap heap;
    int sortedVals[31];
    int sortedCount;

    string input;
    string logLines[8];
    int highlightId;
    RectangleShape btn[6];

    void clearAll();
    void addLog(const string& s);
    void runOp(int op);
    void randomFill();
    void drawTree(TreeNode* n);
    void drawHeap();

    TreeNode* insertBST(TreeNode* n, TreeNode* par, int v, bool& ok);
    TreeNode* deleteBST(TreeNode* n, int v, bool& found);

public:
    SandboxScreen(RenderWindow& win);
    ~SandboxScreen();
    void draw() override;
    void handleInput() override;
    void update(float time) override {}
    int getNextScreen() const override { return nextScreen; }
    void onEnter() override;
};

//----------------leaderboard---------------------------
void saveResult(int levelId, int stars, int score);
int  loadResults(int keys[], int maxN);
int  previousBest(int levelId);

class ResultPopup {
    RenderWindow& window;
    Font font;
    bool open;
    int action;            // 0 none, 1 continue, 2 leaderboard, 99 quit
    int levelId, stars, score, alarm, penalties;
    bool newBest;

public:
    ResultPopup(RenderWindow& win);
    void show(int id, int st, int sc, int al, int pe);
    void close() { open = false; }
    bool isOpen() const { return open; }
    int takeAction() { int a = action; action = 0; return a; }
    void handleInput();
    void draw();
};

// screen id 26
class LeaderboardScreen : public Screen {
    Font font;
    Texture backgroundTexture;
    Sprite background;
    int nextScreen;
    int topKeys[10];
    int topCount;
    int total;

public:
    LeaderboardScreen(RenderWindow& win);
    void draw() override;
    void handleInput() override;
    void update(float time) override {}
    int getNextScreen() const override { return nextScreen; }
    void onEnter() override;
};