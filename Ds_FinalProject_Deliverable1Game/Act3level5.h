#pragma once
#include "screen.h"
#include "Act3.h"

// screen id 16
class Act3Level5Screen : public Screen {

    Font font;
    Texture backgroundTexture;
    Sprite background;
    int nextScreen;

    static const int SCREEN_ID = 16;
    static const int NEXT_SCREEN = 1;       // change to 17 when Act 4 exists
    static const int MAXN = 32;
    static const int MAX_PATH = 32;
    static const int TOTAL_ROUNDS = 5;
    static const int PLAYING = 0;
    static const int WON = 1;
    static const int LOST = 2;
    static const int T_SMALL = 0;           // k-th smallest
    static const int T_LARGE = 1;           // k-th most valuable
    static const int T_AUDIT = 2;           // find the corrupted size

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

    // round state
    int roundNo;
    int roundType;
    int roundK;                     // k shown to the player
    int roundRank;                  // same target as a smallest-first rank
    int curK;                       // rank still being searched for in the current subtree
    int path[MAX_PATH];             // correct walk from the root (ids)
    int pathCount;
    int step;                       // rooms of the path already visited
    int corruptTrueSize;            // real size of the corrupted room
    int checkedIds[MAXN];           // audit: rooms already verified as fine
    int checkedCount;

    // scoring
    int optimal;
    int capacity;
    int alarm;
    int penalties;
    int steps;
    bool hintUsed;
    int hintId;
    int state;

    bool advancePending;
    float advanceTimer;

    unsigned seed;
    unsigned puzzleSeed;
    float flashTime;
    float messageTime;
    float animTime;
    string message;
    string guide;
    Color messageColor;

    void newPuzzle();
    void restartRun();
    void startRound(int r);
    void planRound(AVLTree& t, int r, int& type, int& k, int& rank,
        int& clicks, int& mutKey, int& mutKind);
    void loadModel();
    void snapshotPositions();
    void startTween();
    int  indexOfId(int id) const;
    void layoutRec(int i, int& pos);
    int  findNodeAt(float x, float y) const;
    bool isVisited(int id) const;
    bool isChecked(int id) const;
    void updateGuide();
    void advanceK();
    void roundDone();
    void handleNodeClick(int i);
    void useHint();
    int  stars() const;
    int  score() const;

public:
    Act3Level5Screen(RenderWindow& win);
    void draw() override;
    void onResume() override;
    void handleInput() override;
    void update(float time) override;
    int getNextScreen() const override { return nextScreen; }
    void onEnter() override;
};