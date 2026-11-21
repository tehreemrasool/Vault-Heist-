
#pragma once
#include "screen.h"
#include "Act3.h"

// screen id 15
class Act3Level4Screen : public Screen {

    Font font;
    Texture backgroundTexture;
    Sprite background;
    int nextScreen;

    static const int SCREEN_ID = 15;
    static const int NEXT_SCREEN = 16;    
    static const int MAXN = 32;
    static const int MAX_PATH = 32;
    static const int TOTAL_ROUNDS = 6;
    static const int NUM_CHOICES = 5;       // NONE, LL, RR, LR, RL
    static const int PLAYING = 0;
    static const int WON = 1;
    static const int LOST = 2;

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
    int   newId;                    // room created by the last insert, -1 if none

    // game state
    int roundNo;
    int incomingKey;                // key about to be inserted
    int expectedCase;               // what the model says will happen (ROT_*)
    int optimal;
    int capacity;
    int alarm;
    int penalties;
    int steps;                      // prediction attempts
    bool hintUsed;
    int hintPath[MAX_PATH];
    int hintCount;
    int state;

    // prediction buttons
    int caseOfButton[NUM_CHOICES];
    bool triedWrong[NUM_CHOICES];

    bool advancePending;
    float advanceTimer;

    unsigned seed;
    unsigned puzzleSeed;
    float flashTime;
    float messageTime;
    float animTime;
    string message;
    Color messageColor;

    void newPuzzle();
    void restartRun();
    void startRound(int r);
    int  pickKey(int r);
    void loadModel();
    void snapshotPositions();
    void startTween();
    int  indexOfId(int id) const;
    void layoutRec(int i, int& pos);
    int  findNodeAt(float x, float y) const;
    int  buttonAt(float x, float y) const;
    bool isHintId(int id) const;
    void chooseCase(int b);
    void useHint();
    int  stars() const;
    int  score() const;

public:
    Act3Level4Screen(RenderWindow& win);
    void draw() override;
    void onResume() override;
    void handleInput() override;
    void update(float time) override;
    int getNextScreen() const override { return nextScreen; }
    void onEnter() override;
};