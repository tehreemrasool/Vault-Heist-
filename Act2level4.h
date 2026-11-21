#pragma once
#include "screen.h"
#include "Act2.h"

// screen id 10
class Act2Level4Screen : public Screen {

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
    int idxOf[MAXID];

    bool inRange[MAXID];    // rooms whose value is inside [lo, hi]
    bool visited[MAXID];    // rooms the thief has entered
    bool greyed[MAXID];     // rooms in a subtree the player wrongly tried to enter

    int lo;
    int hi;
    int totalLoot;
    int looted;
    int visitedCount;
    int par;
    int capacity;
    int alarm;
    int penalties;
    bool hintUsed;
    int hintId;
    int state;

    unsigned seed;
    unsigned puzzleSeed;
    float flashTime;
    float messageTime;
    float animTime;
    string message;
    Color messageColor;

    void newPuzzle();
    void loadPuzzle(unsigned s);
    void rebuild();
    int  idx(int id) const;
    bool isNeeded(int parentId, int side);   // can this side still hold in-range keys?
    void clickRoom(int i);
    int  findNodeAt(float x, float y) const;
    int  prunedCount() const;
    int  stars() const;
    int  score() const;

public:
    Act2Level4Screen(RenderWindow& win);
    void draw() override;
    void onResume() override;
    void handleInput() override;
    void update(float time) override;
    int getNextScreen() const override { return nextScreen; }
    void onEnter() override;
};