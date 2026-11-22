#include "Act2Level3.h"
#include "helpers.h"
#include <cmath>
using namespace sf;
using namespace std;


static const char* caseName(int c)
{
    if (c == 1) return "LEAF";
    if (c == 2) return "ONE CHILD";
    return "TWO CHILDREN";
}

Act2Level3Screen::Act2Level3Screen(RenderWindow& win) : Screen(win)
{
    if (!font.loadFromFile("assets/fonts/ARIAL.ttf"))
        loadFont(font);

    if (backgroundTexture.loadFromFile("assets/pictures/Act2_level3.jpeg"))
    {
        background.setTexture(backgroundTexture);
        background.setScale(1280.0f / backgroundTexture.getSize().x,
            720.0f / backgroundTexture.getSize().y);
    }

    nextScreen = 9;
    nodeCount = 0;
    round = 0;
    targetId = -1;
    targetValue = 0;
    targetCase = 0;
    succId = -1;
    succValue = 0;
    resultId = -1;
    capacity = 0;
    alarm = 0;
    penalties = 0;
    hintUsed = false;
    state = LOST;
    animPhase = ANIM_NONE;
    animT = 0;

    seed = (unsigned)time(NULL) + 79;
    puzzleSeed = seed;
    flashTime = 0;
    messageTime = 0;
    animTime = 0;
    messageColor = Color::White;

    for (int i = 0; i < ROUNDS; i++) caseOrder[i] = i + 1;
    for (int i = 0; i < MAXID; i++) idxOf[i] = -1;
}

void Act2Level3Screen::rebuild()
{
    bst.computeLayout(100.0f, 1180.0f, 175.0f, 605.0f);
    nodeCount = bst.collectNodes(nodes, MAXN);

    for (int i = 0; i < MAXID; i++) idxOf[i] = -1;
    for (int i = 0; i < nodeCount; i++)
        if (nodes[i].id >= 0 && nodes[i].id < MAXID)
            idxOf[nodes[i].id] = i;
}

int Act2Level3Screen::idx(int id) const
{
    if (id < 0 || id >= MAXID) return -1;
    return idxOf[id];
}

void Act2Level3Screen::newPuzzle()
{
    puzzleSeed = seed++;
    loadPuzzle(puzzleSeed);
}

void Act2Level3Screen::loadPuzzle(unsigned s)
{
    srand(s);
    int n = 10 + rand() % 3;                  


    bool ok = false;
    for (int attempt = 0; attempt < 50 && !ok; attempt++)
    {
        bst.generateRandom(s + (unsigned)attempt * 7919u, n, 0);
        rebuild();

        int has[4] = { 0, 0, 0, 0 };
        for (int i = 0; i < nodeCount; i++)
        {
            int c = bst.classifyDelete(nodes[i].value);
            if (c >= 1 && c <= 3) has[c]++;
        }
        ok = (has[1] > 0 && has[2] > 0 && has[3] > 0);
    }

    srand(s + 3);
    for (int i = 0; i < ROUNDS; i++) caseOrder[i] = i + 1;
    for (int i = ROUNDS - 1; i > 0; i--)
    {
        int j = rand() % (i + 1);
        int t = caseOrder[i]; caseOrder[i] = caseOrder[j]; caseOrder[j] = t;
    }

    capacity = BST::capacityFor(ROUNDS);          

    round = 0;
    alarm = 0;
    penalties = 0;
    hintUsed = false;
    animPhase = ANIM_NONE;
    animT = 0;
    flashTime = 0;
    messageTime = 0;
    message.clear();
    state = (bst.getRoot() != NULL) ? PLAYING : LOST;

    pickTarget(0);
}

void Act2Level3Screen::pickTarget(int r)
{
    succId = -1;
    resultId = -1;

    srand(puzzleSeed + 10 + r);

    int cand[MAXN];
    int c = 0;
    for (int i = 0; i < nodeCount; i++)
        if (bst.classifyDelete(nodes[i].value) == caseOrder[r])
            cand[c++] = i;

    // fallback: the tree changed and that case no longer exists
    if (c == 0)
        for (int i = 0; i < nodeCount; i++)
            if (bst.classifyDelete(nodes[i].value) != 0)
                cand[c++] = i;

    if (c == 0) { state = LOST; return; }

    int pick = cand[rand() % c];
    targetId = nodes[pick].id;
    targetValue = nodes[pick].value;
    targetCase = bst.classifyDelete(targetValue);
}

void Act2Level3Screen::choose(int c)
{
    if (state != PLAYING || animPhase != ANIM_NONE) return;

    alarm++;

    if (c == targetCase)
    {
        messageTime = 0;
        startAnim();
    }
    else
    {
        penalties++;
        flashTime = 0.35f;
        message = "WRONG CASE! Alarm +1";
        messageColor = Color(255, 110, 110);
        messageTime = 1.4f;
        if (alarm >= capacity) state = LOST;
    }
}

void Act2Level3Screen::startAnim()
{
    animPhase = ANIM_FLASH;
    animT = 0;
    succId = -1;
    resultId = -1;

    if (targetCase == 3)
    {
        BSTNode* t = bst.getNodeById(targetId);
        BSTNode* s = (t != NULL) ? bst.successor(t) : NULL;
        if (s != NULL)
        {
            succId = s->id;
            succValue = s->value;
        }
    }
}

void Act2Level3Screen::applyDelete()
{
    bst.clearEvents();
    bst.deleteNode(targetValue);
    rebuild();

    // in the two-child case the target room keeps its place and takes the successor's value
    resultId = (targetCase == 3) ? targetId : -1;

    message = "ROOM REMOVED!";
    messageColor = Color(120, 255, 120);
    messageTime = 0.8f;

    animPhase = ANIM_DONE;
    animT = 0;
}

void Act2Level3Screen::finishRound()
{
    animPhase = ANIM_NONE;
    animT = 0;
    succId = -1;
    resultId = -1;
    round++;

    if (round >= ROUNDS)
    {
        state = WON;
        message = "ALL ROOMS COLLAPSED!";
        messageColor = Color(120, 255, 120);
        messageTime = 3.0f;
    }
    else if (alarm >= capacity)
    {
        state = LOST;
    }
    else
    {
        pickTarget(round);
    }
}

int Act2Level3Screen::buttonAt(float x, float y) const
{
    for (int i = 0; i < 3; i++)
    {
        float bx = BTN_X + i * BTN_STEP;
        if (x >= bx && x <= bx + BTN_W && y >= BTN_Y && y <= BTN_Y + BTN_H)
            return i + 1;
    }
    return -1;
}

int Act2Level3Screen::stars() const
{
    if (state != WON) return 0;
    int s = 1;
    if (alarm <= ROUNDS && penalties == 0) s = 3;
    else if (alarm * 4 <= 5 * ROUNDS && penalties <= 1) s = 2;
    if (hintUsed && s > 2) s = 2;
    return s;
}

int Act2Level3Screen::score() const
{
    if (state != WON) return 0;
    int s = 1000 - 10 * alarm;
    return s < 0 ? 0 : s;
}

void Act2Level3Screen::onEnter()
{
    isActive = true;
    nextScreen = 9;
    newPuzzle();
}

void Act2Level3Screen::onResume()
{
    isActive = true;
    nextScreen = 9;
    flashTime = 0;
}

void Act2Level3Screen::handleInput()
{
    Event e;

    while (window.pollEvent(e))
    {
        if (e.type == Event::Closed)
            nextScreen = 99;

        if (e.type == Event::MouseButtonPressed && e.mouseButton.button == Mouse::Left)
        {
            int b = buttonAt((float)e.mouseButton.x, (float)e.mouseButton.y);
            if (b != -1)
                choose(b);
        }

        if (e.type == Event::KeyPressed)
        {
            if (e.key.code == Keyboard::Escape)
                nextScreen = 25;

            else if (e.key.code == Keyboard::Num1)
                choose(1);

            else if (e.key.code == Keyboard::Num2)
                choose(2);

            else if (e.key.code == Keyboard::Num3)
                choose(3);

            else if (e.key.code == Keyboard::H)
            {
                if (state == PLAYING && animPhase == ANIM_NONE)
                {
                    hintUsed = true;
                    message = string("HINT: this room has ") + caseName(targetCase);
                    messageColor = Color(255, 215, 70);
                    messageTime = 3.0f;
                }
            }
            else if (e.key.code == Keyboard::N)
                nextScreen = 10;

            else if (e.key.code == Keyboard::Space)
                loadPuzzle(puzzleSeed);       

            else if (e.key.code == Keyboard::R)
                newPuzzle();

            else if (e.key.code == Keyboard::Return)
            {
                if (state == WON)
                    nextScreen = 10;     // TODO: change to 10 once Act 2 Level 4 exists
            }
        }
    }
}

void Act2Level3Screen::update(float dt)
{
    animTime += dt;
    if (flashTime > 0) flashTime -= dt;
    if (messageTime > 0) messageTime -= dt;

    if (animPhase == ANIM_NONE) return;

    animT += dt;

    if (animPhase == ANIM_FLASH && animT >= 1.0f)
    {
        animT = 0;
        if (succId != -1) animPhase = ANIM_SLIDE;
        else              applyDelete();
    }
    else if (animPhase == ANIM_SLIDE && animT >= 1.2f)
    {
        applyDelete();
    }
    else if (animPhase == ANIM_DONE && animT >= 0.8f)
    {
        finishRound();
    }
}

void Act2Level3Screen::draw()
{
    window.clear();
    window.draw(background);

    drawText(window, font, "ACT 2 - LEVEL 3", 40, 500, 15, Color::White, false);

    // ---- HUD boxes ----
    RectangleShape alarmBox(Vector2f(220, 50));
    RectangleShape targetBox(Vector2f(420, 50));

    alarmBox.setPosition(50, 70);
    targetBox.setPosition(810, 70);
    alarmBox.setFillColor(Color::Black);
    targetBox.setFillColor(Color::Black);
    alarmBox.setOutlineColor(Color(75, 144, 184));
    alarmBox.setOutlineThickness(3);
    targetBox.setOutlineColor(Color(75, 144, 184));
    targetBox.setOutlineThickness(3);

    window.draw(alarmBox);
    window.draw(targetBox);

    float fraction = (capacity > 0) ? (float)alarm / (float)capacity : 1.0f;
    if (fraction > 1.0f) fraction = 1.0f;
    RectangleShape bar(Vector2f(214 * fraction, 44));
    bar.setPosition(53, 73);
    if (fraction < 0.5f)       bar.setFillColor(Color(60, 160, 80, 190));
    else if (fraction < 0.8f)  bar.setFillColor(Color(200, 170, 40, 190));
    else                       bar.setFillColor(Color(200, 60, 60, 190));
    window.draw(bar);

    drawText(window, font, "ALARM: " + to_string(alarm) + " / " + to_string(capacity),
        18, 70, 82, Color::White, false);

    int shownRound = (round < ROUNDS) ? round + 1 : ROUNDS;
    drawText(window, font,
        "DELETE: " + to_string(targetValue) + "  (" + to_string(shownRound) + "/" + to_string(ROUNDS) + ")",
        22, 1020, 95, Color(255, 215, 70), true);

    // ---- prompt / feedback ----
    if (animPhase == ANIM_FLASH || animPhase == ANIM_SLIDE)
    {
        const char* t = (targetCase == 1) ? "Leaf: just unlink it"
            : (targetCase == 2) ? "One child: bypass it, link parent to child"
            : "Two children: successor's value replaces it";
        drawText(window, font, t, 19, 540, 95, Color(255, 215, 70), true);
    }
    else if (messageTime > 0)
        drawText(window, font, message, 22, 540, 95, messageColor, true);
    else
        drawText(window, font, "Which delete case applies to the gold room?",
            19, 540, 95, Color::White, true);

    drawText(window, font, "1/2/3 or Click: Choose case | H: Hint | Space: Retry | R: New Vault",
        16, 640, 125, Color(208, 221, 183), true);

    // ---- edges ----
    for (int i = 0; i < nodeCount; i++)
    {
        int p = nodes[i].parentId;
        if (idx(p) < 0) continue;

        const NodeInfo& par = nodes[idx(p)];
        Vector2f start(par.x, par.y);
        Vector2f end(nodes[i].x, nodes[i].y);
        Vector2f diff = end - start;

        RectangleShape line(Vector2f(sqrt(diff.x * diff.x + diff.y * diff.y), 5));
        line.setPosition(start);
        line.setRotation(atan2(diff.y, diff.x) * 180.0f / 3.14159f);
        line.setFillColor(Color(208, 221, 183));
        window.draw(line);
    }

    if (succId != -1 && idx(targetId) >= 0 && idx(succId) >= 0 &&
        (animPhase == ANIM_FLASH || animPhase == ANIM_SLIDE))
    {
        Vector2f a(nodes[idx(targetId)].x, nodes[idx(targetId)].y);
        Vector2f b(nodes[idx(succId)].x, nodes[idx(succId)].y);
        Vector2f diff = b - a;

        RectangleShape link(Vector2f(sqrt(diff.x * diff.x + diff.y * diff.y), 4));
        link.setPosition(a);
        link.setRotation(atan2(diff.y, diff.x) * 180.0f / 3.14159f);
        link.setFillColor(Color(255, 215, 70, 200));
        window.draw(link);
    }

    // ---- nodes ----
    float radius = 30.0f;
    bool animating = (animPhase == ANIM_FLASH || animPhase == ANIM_SLIDE);

    for (int i = 0; i < nodeCount; i++)
    {
        const NodeInfo& n = nodes[i];
        bool isTarget = (n.id == targetId) && state == PLAYING && animPhase != ANIM_DONE;
        bool isSucc = (n.id == succId) && animating;
        bool isResult = (n.id == resultId);

        CircleShape circle(radius);
        circle.setOrigin(radius, radius);
        circle.setPosition(n.x, n.y);
        circle.setFillColor(Color(73, 103, 164));
        circle.setOutlineThickness(7);

        if (isResult)                    circle.setOutlineColor(Color::Green);
        else if (isSucc)                 circle.setOutlineColor(Color(255, 215, 70));
        else if (isTarget && animating)  circle.setOutlineColor(Color(255, 110, 110));
        else if (isTarget)               circle.setOutlineColor(Color(255, 215, 70));
        else                             circle.setOutlineColor(Color(208, 221, 183));

        window.draw(circle);

        if (isTarget && !animating)
        {
            int alpha = 170 + (int)(50 * sin(animTime * 6));
            CircleShape halo(radius + 13);
            halo.setOrigin(radius + 13, radius + 13);
            halo.setPosition(n.x, n.y);
            halo.setFillColor(Color(150, 60, 220, (Uint8)(alpha / 3)));
            window.draw(halo);
        }

        drawText(window, font, to_string(n.value), 26, n.x, n.y, Color::White, true);

        if (isResult)
            drawText(window, font, "NEW VALUE", 16, n.x, n.y - 46, Color(120, 255, 120), true);
        else if (isSucc)
            drawText(window, font, "SUCCESSOR", 16, n.x, n.y - 46, Color(255, 215, 70), true);
        else if (isTarget && animating)
            drawText(window, font, "DELETE", 16, n.x, n.y - 46, Color(255, 110, 110), true);
        else if (isTarget)
            drawText(window, font, "TARGET", 16, n.x, n.y - 46, Color(255, 215, 70), true);
    }

    // ---- sliding value token (two-child case) ----
    if (animPhase == ANIM_SLIDE && idx(succId) >= 0 && idx(targetId) >= 0)
    {
        float p = animT / 1.2f;
        if (p > 1.0f) p = 1.0f;

        const NodeInfo& s = nodes[idx(succId)];
        const NodeInfo& t = nodes[idx(targetId)];
        float tx = s.x + (t.x - s.x) * p;
        float ty = s.y + (t.y - s.y) * p;

        CircleShape token(26);
        token.setOrigin(26, 26);
        token.setPosition(tx, ty);
        token.setFillColor(Color(110, 127, 235));
        token.setOutlineThickness(5);
        token.setOutlineColor(Color(110, 127, 235));
        window.draw(token);

        drawText(window, font, to_string(succValue), 24, tx, ty, Color::White, true);
    }

    // ---- bottom status ----
    drawText(window, font, "PAR: " + to_string(ROUNDS) + "   PENALTIES: " + to_string(penalties),
        20, 60, 668, Color::White, false);

    if (state == WON)
    {
        drawText(window, font, "VAULT CLEARED!", 30, 640, 650, Color::Green, true);
        drawText(window, font,
            "STARS: " + to_string(stars()) + "   SCORE: " + to_string(score()) + "   (ENTER: continue)",
            20, 640, 685, Color(208, 221, 183), true);
    }
    else if (state == LOST)
    {
        drawText(window, font, "ALARM TRIGGERED! YOU LOST!   (Space: retry)",
            26, 640, 672, Color::Red, true);
    }
    else
    {
        Vector2i mp = Mouse::getPosition(window);
        int hover = buttonAt((float)mp.x, (float)mp.y);
        bool locked = (animPhase != ANIM_NONE);

        const char* labels[3] = { "1  LEAF", "2  ONE CHILD", "3  TWO CHILDREN" };
        for (int i = 0; i < 3; i++)
        {
            RectangleShape btn(Vector2f(BTN_W, BTN_H));
            btn.setPosition(BTN_X + i * BTN_STEP, BTN_Y);
            btn.setFillColor(Color::Black);
            btn.setOutlineThickness(3);

            if (locked)                  btn.setOutlineColor(Color(90, 60, 110));
            else if (hover == i + 1)     btn.setOutlineColor(Color(255, 215, 70));
            else                         btn.setOutlineColor(Color(100, 108, 220));
            window.draw(btn);

            drawText(window, font, labels[i], 18, BTN_X + i * BTN_STEP + BTN_W / 2, BTN_Y + BTN_H / 2,
                locked ? Color(130, 130, 130) : Color::White, true);
        }
    }

    if (flashTime > 0)
    {
        RectangleShape redScreen(Vector2f(1280, 720));
        redScreen.setFillColor(Color(255, 0, 0, 90));
        window.draw(redScreen);
    }
}