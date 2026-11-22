#pragma once
#include "screen.h"
#include "Act2.h"

// screen id 9
class Act2Level3Screen : public Screen {

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

    static const int ANIM_NONE = 0;
    static const int ANIM_FLASH = 1;    // target / successor highlighted
    static const int ANIM_SLIDE = 2;    // successor value slides into target
    static const int ANIM_DONE = 3;     // short pause after the delete
    static constexpr float BTN_X = 330.0f;
    static constexpr float BTN_Y = 655.0f;
    static constexpr float BTN_W = 190.0f;
    static constexpr float BTN_H = 46.0f;
    static constexpr float BTN_STEP = 210.0f;
    BST bst;
    NodeInfo nodes[MAXN];
    int nodeCount;
    int idxOf[MAXID];

    int caseOrder[ROUNDS];  // which delete case each round asks for
    int round;
    int targetId;
    int targetValue;
    int targetCase;         // 1 leaf, 2 one child, 3 two children
    int succId;             // -1 unless the two-child case is playing
    int succValue;
    int resultId;           // room that received the successor value

    int capacity;
    int alarm;
    int penalties;
    bool hintUsed;
    int state;

    int animPhase;
    float animT;

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
    void pickTarget(int r);
    void choose(int c);
    void startAnim();
    void applyDelete();
    void finishRound();
    int  buttonAt(float x, float y) const;
    int  stars() const;
    int  score() const;

public:
    Act2Level3Screen(RenderWindow& win);
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