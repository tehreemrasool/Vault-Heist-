#pragma once
#include "screen.h"
#include "Act2.h"

class Act2Level5Screen : public Screen {

    Font font;
    Font storyFont;
    Texture backgroundTexture;
    Sprite background;
    int nextScreen;


    static const int MAXN = 32;
    static const int MAXID = 256;
    static const int ROUNDS = 3;
    static const int SHRINK = 3;       

    static const int PLAYING = 0;
    static const int WON = 1;
    static const int LOST = 2;

    static const int PH_DELIVER = 0;    
    static const int PH_SEARCH = 1;     
    static const int PH_CUTSCENE = 2;   

    BST bst;                
    BST sim;  

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
    void tryMove(int dir);         
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
    bool getResult(int& st, int& sc, int& al, int& pe) const override
    {
        if (state != WON) return false;
        st = stars(); sc = score(); al = alarm; pe = penalties;
        return true;
    }
};