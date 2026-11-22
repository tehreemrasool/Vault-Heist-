#pragma once
#include "screen.h"
#include "Act2.h"

// screen id 8
class Act2Level2Screen : public Screen {

    Font font;
    Texture backgroundTexture;
    Sprite background;
    int nextScreen;

    // game state
    static const int MAXN = 32;
    static const int MAXID = 256;
    static const int ROUNDS = 3;
    static const int PLAYING = 0;
    static const int WON = 1;
    static const int LOST = 2;

    BST bst;                // the vault the player sees
    BST sim;                // dry run, only used to compute optimal steps
    NodeInfo nodes[MAXN];
    int nodeCount;
    int idxOf[MAXID];
    bool visited[MAXID];

    int keys[ROUNDS];       // loot values to stash
    int round;              // 0..ROUNDS
    int curId;
    int newId;              // last stashed room
    int totalOptimal;
    int capacity;
    int alarm;
    int penalties;
    bool hintUsed;
    int hintDir;            // -1 none, 0 left, 1 right
    int hintChildId;        // -1 if the hinted side is an empty slot
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
    void rebuild();                 // layout + node list after any change
    void startRound();
    void tryMove(int dir);          // 0 = left, 1 = right
    int  findNodeAt(float x, float y) const;
    void slotPos(int side, float& x, float& y) const;
    int  stars() const;
    int  score() const;

public:
    Act2Level2Screen(RenderWindow& win);
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