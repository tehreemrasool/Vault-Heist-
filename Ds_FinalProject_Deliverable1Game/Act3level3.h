#pragma once
#include<iostream>
using namespace std;
#include "screen.h"
#include "Act3.h"

// screen id 14
class Act3Level3Screen : public Screen {

    Font font;
    Texture backgroundTexture;
    Sprite background;
    int nextScreen;

    static const int SCREEN_ID = 14;
    static const int NEXT_SCREEN = 15;      
    static const int MAXN = 32;
    static const int TOTAL_ROUNDS = 4;
    static const int PLAYING = 0;
    static const int WON = 1;
    static const int LOST = 2;
    static const int DRAG_MIN = 40;

    // model
    AVLTree tree;
    AVLNodeInfo nodes[MAXN];
    int nodeCount;
    int maxDepth;

    // tween between old and new room positions
    float startX[MAXN];
    float startY[MAXN];
    int   oldId[MAXN];
    float oldX[MAXN];
    float oldY[MAXN];
    int   oldCount;
    float animT;

    // game state
    int roundNo;
    int roundCase;                  // the case the model reports for this round
    bool identified;                // player named the case correctly
    int doneRot;                    // rotations done in this round
    int optimal;
    int capacity;
    int alarm;
    int penalties;                  // wrong choices (identify or rotate)
    int steps;                      // drag attempts
    bool hintUsed;
    int hintId;
    int state;

    // identification buttons
    int caseOfButton[4];
    bool triedWrong[4];

    // dragging
    int dragIdx;
    float dragStartX;
    float dragStartY;

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
    int  buttonAt(float x, float y) const;
    int  unbalancedCount() const;
    bool expectedMove(int& nodeId, char& dir);
    void chooseCase(int b);
    void tryRotation(int idx, int dx);
    void useHint();
    int  stars() const;
    int  score() const;

public:
    Act3Level3Screen(RenderWindow& win);
    void draw() override;
    void onResume() override;
    void handleInput() override;
    void update(float time) override;
    int getNextScreen() const override { return nextScreen; }
    void onEnter() override;
};