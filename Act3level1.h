#pragma once
#include "screen.h"
#include "Act3.h"

class Act3Level1Screen : public Screen {

    Font font;
    Texture backgroundTexture;
    Sprite background;
    int nextScreen;

    static const int SCREEN_ID = 12;
    static const int NEXT_SCREEN = 13;  
    static const int MAXN = 32;
    static const int PLAYING = 0;
    static const int WON = 1;
    static const int LOST = 2;
    static const bool SHOW_COLOURS_FROM_START = false;

    AVLTree tree;
    AVLNodeInfo nodes[MAXN];
    int nodeCount;
    int maxDepth;
    bool checked[MAXN];     // player already clicked this room
    bool isBad[MAXN];       // from the model: |balance| > 1

    int optimal;            // number of unbalanced rooms, from the model
    int capacity;
    int alarm;
    int penalties;
    int found;
    bool hintUsed;
    int hintIdx;
    int state;

    unsigned seed;
    unsigned puzzleSeed;
    float flashTime;
    float messageTime;
    float animTime;
    string message;
    Color messageColor;

    void newPuzzle();
    void loadModel();
    void restartRun();
    int  indexOfId(int id) const;
    void layoutRec(int i, int& pos);
    int  findNodeAt(float x, float y) const;
    void clickNode(int i);
    void useHint();
    int  stars() const;
    int  score() const;

public:
    Act3Level1Screen(RenderWindow& win);
    void draw() override;
    void onResume() override;
    void handleInput() override;
    void update(float time) override;
    int getNextScreen() const override { return nextScreen; }
    void onEnter() override;
    bool getResult(int& st, int& sc, int& al, int& pe) const override
    {
        if (state != WON) return false;
        st = stars(); sc = score(); al = alarm; pe = penalties;
        return true;
    }
};