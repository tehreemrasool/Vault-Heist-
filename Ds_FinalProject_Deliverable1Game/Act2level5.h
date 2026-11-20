#pragma once
#include "screen.h"
#include "Act2.h"

// screen id 11
class Act2Level5Screen : public Screen {

    Font font;
    Font storyFont;
    Texture backgroundTexture;
    Sprite background;
    int nextScreen;

    // game state
    static const int MAXN = 32;
    static const int MAXID = 256;
    static const int ROUNDS = 3;
    static const int SHRINK = 3;        // capacity lost per round

    static const int PLAYING = 0;
    static const int WON = 1;
    static const int LOST = 2;

    static const int PH_DELIVER = 0;    // corridor builds itself
    static const int PH_SEARCH = 1;     // boss searches
    static const int PH_CUTSCENE = 2;   // why balancing is needed

    BST bst;                // the corridor the player searches
    BST sim;                // balanced version of the same keys (cutscene)

    NodeInfo nodes[MAXN];
    int nodeCount;
    int idxOf[MAXID];
    bool visited[MAXID];

    NodeInfo simNodes[MAXN];
    int simCount;
    int simIdxOf[MAXID];
    int simHeight;

    int phase;
    int shown;              // rooms delivered so far
    float deliverT;

    int targets[ROUNDS];
    int pars[ROUNDS];
    int round;
    int curId;
    int targetValue;
    int par;                // par of the current round
    int totalPar;
    int corridorHeight;
    int capacity;
    int alarm;              // alarm of the current round
    int totalAlarm;         // alarm of finished rounds
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
    void loadPuzzle(unsigned s);
    void rebuild();
    int  idx(int id) const;
    void finishDelivery();
    void startRound();
    void completeRound();
    void tryMove(int dir);          // 0 = left, 1 = right
    void buildCutscene();
    int  findNodeAt(float x, float y) const;
    int  stars() const;
    int  score() const;
    void drawCutscene();

public:
    Act2Level5Screen(RenderWindow& win);
    void draw() override;
    void onResume() override;
    void handleInput() override;
    void update(float time) override;
    int getNextScreen() const override { return nextScreen; }
    void onEnter() override;
};