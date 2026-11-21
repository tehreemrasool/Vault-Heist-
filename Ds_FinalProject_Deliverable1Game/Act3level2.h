#pragma once
#include "screen.h"
#include "Act3.h"

class Act3Level2Screen : public Screen {

    Font font;
    Texture backgroundTexture;
    Sprite background;
    int nextScreen;

    static const int SCREEN_ID = 13;
    static const int NEXT_SCREEN = 14; 
    static const int MAXN = 32;
    static const int TOTAL_ROUNDS = 4;
    static const int PLAYING = 0;
    static const int WON = 1;
    static const int LOST = 2;
    static const int DRAG_MIN = 40;      
    AVLTree tree;
    AVLNodeInfo nodes[MAXN];
    int nodeCount;
    int maxDepth;

    // tween between old and new room positions after a rotation
    float startX[MAXN];
    float startY[MAXN];
    int   oldId[MAXN];
    float oldX[MAXN];
    float oldY[MAXN];
    int   oldCount;
    float animT;   
    int roundNo;
    int optimal;
    int capacity;
    int alarm;
    int penalties;                      
    int steps;                            
    bool hintUsed;
    int hintId;                             // node id, -1 if none
    int state;

    int dragIdx;
    float dragStartX;
    float dragStartY;
    int selectedIdx;

    bool advancePending;
    float advanceTimer;

    unsigned seed;
    unsigned puzzleSeed;
    float flashTime;
    float messageTime;
    float animTime;
    string message;
    Color messageColor;

    int  caseForRound(int r) const;
    void newPuzzle();
    void restartRun();
    void startRound(int r);
    void loadModel();
    void snapshotPositions();
    void startTween();
    int  indexOfId(int id) const;
    void layoutRec(int i, int& pos);
    int  findNodeAt(float x, float y) const;
    int  unbalancedCount() const;
    void tryRotation(int idx, int dx);
    void useHint();
    int  stars() const;
    int  score() const;

public:
    Act3Level2Screen(RenderWindow& win);
    void draw() override;
    void onResume() override;
    void handleInput() override;
    void update(float time) override;
    int getNextScreen() const override { return nextScreen; }
    void onEnter() override;
}; 
