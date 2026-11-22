#pragma once
#include "screen.h"
#include "Act2.h"

// screen id 7
class Act2Level1Screen : public Screen {

    Font font;
    Texture backgroundTexture;
    Sprite background;
    int nextScreen;

    // game state
    static const int MAXN = 32;
    static const int MAXID = 256;
    static const int PLAYING = 0;
    static const int WON = 1;
    static const int LOST = 2;

    BST bst;
    NodeInfo nodes[MAXN];
    int nodeCount;
    int idxOf[MAXID];       // node id -> index in nodes[]
    bool visited[MAXID];

    int curId;              // room the thief is standing in
    int targetValue;        // treasure to steal
    int optimal;            // from the model, never typed by hand
    int capacity;
    int alarm;
    int penalties;
    bool hintUsed;
    int hintChildId;
    int state;

    unsigned seed;
    unsigned puzzleSeed;
    float flashTime;
    float messageTime;
    float animTime;
    string message;
    Color messageColor;

    void newPuzzle();
    void restartRun();
    void tryMove(int dir);          // 0 = left, 1 = right
    int  findNodeAt(float x, float y) const;
    int  stars() const;
    int  score() const;

public:
    Act2Level1Screen(RenderWindow& win);
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