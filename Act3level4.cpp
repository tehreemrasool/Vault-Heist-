#include "Act3Level4.h"
#include "helpers.h"
#include <cmath>
using namespace sf;
using namespace std;

static void drawEdge(RenderWindow& window, float x1, float y1, float x2, float y2)
{
    Vector2f diff(x2 - x1, y2 - y1);
    RectangleShape line(Vector2f(sqrt(diff.x * diff.x + diff.y * diff.y), 5));
    line.setPosition(x1, y1);
    line.setRotation(atan2(diff.y, diff.x) * 180.0f / 3.14159f);
    line.setFillColor(Color(208, 221, 183));
    window.draw(line);
}

Act3Level4Screen::Act3Level4Screen(RenderWindow& win) : Screen(win)
{
    loadFont(font);

    if (backgroundTexture.loadFromFile("assets/pictures/Act3_level4.jpeg"))
    {
        background.setTexture(backgroundTexture);
        background.setScale(1280.0f / backgroundTexture.getSize().x,
            720.0f / backgroundTexture.getSize().y);
    }

    nextScreen = SCREEN_ID;
    nodeCount = 0;
    maxDepth = 1;
    oldCount = 0;
    animT = 1.0f;
    newId = -1;
    roundNo = 0;
    incomingKey = 0;
    expectedCase = AVLTree::ROT_NONE;
    optimal = 0;
    capacity = 0;
    alarm = 0;
    penalties = 0;
    steps = 0;
    hintUsed = false;
    hintCount = 0;
    state = LOST;
    advancePending = false;
    advanceTimer = 0;
    seed = (unsigned)time(NULL) + 419;
    puzzleSeed = seed;
    flashTime = 0;
    messageTime = 0;
    animTime = 0;
    messageColor = Color::White;

    caseOfButton[0] = AVLTree::ROT_NONE;
    caseOfButton[1] = AVLTree::ROT_LL;
    caseOfButton[2] = AVLTree::ROT_RR;
    caseOfButton[3] = AVLTree::ROT_LR;
    caseOfButton[4] = AVLTree::ROT_RL;

    for (int i = 0; i < NUM_CHOICES; i++) triedWrong[i] = false;
    for (int i = 0; i < MAX_PATH; i++) hintPath[i] = -1;

    for (int i = 0; i < MAXN; i++)
    {
        startX[i] = 0;
        startY[i] = 0;
        oldId[i] = -1;
        oldX[i] = 0;
        oldY[i] = 0;
    }

    tree.setRecordEvents(false);
}

int Act3Level4Screen::indexOfId(int id) const
{
    for (int i = 0; i < nodeCount; i++)
        if (nodes[i].id == id) return i;
    return -1;
}

void Act3Level4Screen::layoutRec(int i, int& pos)
{
    if (i < 0) return;

    layoutRec(indexOfId(nodes[i].leftId), pos);

    float xStep = (nodeCount > 1) ? 1080.0f / (nodeCount - 1) : 0.0f;
    float yStep = (maxDepth > 1) ? 430.0f / (maxDepth - 1) : 0.0f;
    if (yStep > 120.0f) yStep = 120.0f;

    nodes[i].x = 100 + pos * xStep;
    nodes[i].y = 185 + (nodes[i].depth - 1) * yStep;
    pos++;

    layoutRec(indexOfId(nodes[i].rightId), pos);
}

void Act3Level4Screen::loadModel()
{
    nodeCount = tree.collectAVLNodes(nodes, MAXN);

    maxDepth = 1;
    for (int i = 0; i < nodeCount; i++)
        if (nodes[i].depth > maxDepth) maxDepth = nodes[i].depth;

    int root = -1;
    for (int i = 0; i < nodeCount; i++)
        if (nodes[i].parentId == -1) { root = i; break; }

    int pos = 0;
    layoutRec(root, pos);

    for (int i = 0; i < nodeCount; i++)
    {
        startX[i] = nodes[i].x;
        startY[i] = nodes[i].y;
    }
    animT = 1.0f;
}

void Act3Level4Screen::snapshotPositions()
{
    oldCount = nodeCount;
    for (int i = 0; i < nodeCount; i++)
    {
        oldId[i] = nodes[i].id;
        oldX[i] = nodes[i].x;
        oldY[i] = nodes[i].y;
    }
}

// existing rooms slide from their old spot, the new room drops in from above
void Act3Level4Screen::startTween()
{
    newId = -1;
    for (int i = 0; i < nodeCount; i++)
    {
        bool found = false;
        for (int j = 0; j < oldCount; j++)
        {
            if (oldId[j] == nodes[i].id)
            {
                startX[i] = oldX[j];
                startY[i] = oldY[j];
                found = true;
                break;
            }
        }
        if (!found)
        {
            newId = nodes[i].id;
            startX[i] = nodes[i].x;
            startY[i] = nodes[i].y - 60.0f;
        }
    }
    animT = 0.0f;
}

// asks the model for a key that triggers the wanted case; falls back to other cases
int Act3Level4Screen::pickKey(int r)
{
    unsigned rng = puzzleSeed * 7919u + (unsigned)(r + 1) * 104729u;
    int start = (int)((puzzleSeed + (unsigned)r * 2) % NUM_CHOICES);
    int key = -1;

    for (int k = 0; k < NUM_CHOICES && key == -1; k++)
    {
        int want = caseOfButton[(start + k) % NUM_CHOICES];
        key = tree.findInsertKeyFor(want, rng);
    }

    if (key == -1)
    {
        for (int tries = 0; tries < 500; tries++)
        {
            int cand = (int)(AVLTree::nextRandom(rng) % 100);
            if (!tree.containsKey(cand)) { key = cand; break; }
        }
    }

    if (key == -1) key = 100 + r;
    return key;
}

void Act3Level4Screen::startRound(int r)
{
    incomingKey = pickKey(r);
    expectedCase = tree.predictInsert(incomingKey);   // model decides, tree untouched

    newId = -1;
    hintCount = 0;
    for (int i = 0; i < NUM_CHOICES; i++) triedWrong[i] = false;
}

void Act3Level4Screen::restartRun()
{
    roundNo = 0;
    optimal = TOTAL_ROUNDS;                 // one correct prediction per insert
    capacity = (3 * optimal + 1) / 2 + 2;

    alarm = 0;
    penalties = 0;
    steps = 0;
    hintUsed = false;
    advancePending = false;
    advanceTimer = 0;
    flashTime = 0;
    messageTime = 0;
    message.clear();
    state = PLAYING;

    unsigned st = puzzleSeed;
    int n = 6 + (int)(AVLTree::nextRandom(st) % 3);     // 6..8 starting rooms

    tree.resetTree();
    tree.buildRandomAVL(puzzleSeed, n, 99);
    loadModel();

    startRound(0);
}

void Act3Level4Screen::newPuzzle()
{
    puzzleSeed = seed++;
    restartRun();
}

void Act3Level4Screen::onEnter()
{
    isActive = true;
    nextScreen = SCREEN_ID;
    newPuzzle();
}

void Act3Level4Screen::onResume()
{
    isActive = true;
    nextScreen = SCREEN_ID;
    flashTime = 0;
}

int Act3Level4Screen::findNodeAt(float x, float y) const
{
    for (int i = 0; i < nodeCount; i++)
    {
        float dx = x - nodes[i].x;
        float dy = y - nodes[i].y;
        if (dx * dx + dy * dy <= 30.0f * 30.0f)
            return i;
    }
    return -1;
}

// five buttons at the bottom: x = 360 + b*132, y = 636, size 124 x 40
int Act3Level4Screen::buttonAt(float x, float y) const
{
    for (int b = 0; b < NUM_CHOICES; b++)
    {
        float bx = 360.0f + b * 132.0f;
        if (x >= bx && x <= bx + 124.0f && y >= 636.0f && y <= 676.0f)
            return b;
    }
    return -1;
}

bool Act3Level4Screen::isHintId(int id) const
{
    for (int i = 0; i < hintCount; i++)
        if (hintPath[i] == id) return true;
    return false;
}

void Act3Level4Screen::chooseCase(int b)
{
    if (state != PLAYING || advancePending || animT < 1.0f) return;
    if (b < 0 || b >= NUM_CHOICES) return;
    if (triedWrong[b]) return;

    steps++;

    if (caseOfButton[b] != expectedCase)
    {
        triedWrong[b] = true;
        alarm += 2;
        penalties++;
        flashTime = 0.35f;
        message = "Wrong prediction! Alarm +2";
        messageColor = Color(255, 110, 110);
        messageTime = 1.6f;

        if (alarm >= capacity) state = LOST;
        return;
    }

    // correct: the vault repairs itself, animated
    snapshotPositions();

    if (!tree.insert(incomingKey))
    {
        // cannot happen (keys are never duplicates), but stay safe: new key
        steps--;
        startRound(roundNo);
        return;
    }

    int rot = tree.getLastRotationCount();

    alarm++;
    loadModel();
    startTween();
    hintCount = 0;

    if (expectedCase == AVLTree::ROT_NONE)
        message = "Correct! Key " + to_string(incomingKey) + " fits, no rotation needed.";
    else
        message = "Correct! " + string(AVLTree::rotationName(expectedCase)) +
        " case: the vault repaired itself (" + to_string(rot) +
        ((rot == 1) ? " rotation)" : " rotations)");
    messageColor = Color(120, 255, 120);
    messageTime = 2.2f;

    roundNo++;
    if (roundNo >= TOTAL_ROUNDS)
        state = WON;
    else
    {
        advancePending = true;
        advanceTimer = 1.6f;
    }

    if (state == PLAYING && alarm >= capacity)
        state = LOST;
}

// hint: gold rooms are the path the new key will walk down
void Act3Level4Screen::useHint()
{
    if (state != PLAYING || advancePending || animT < 1.0f) return;

    int total = tree.rankPath(incomingKey, hintPath, MAX_PATH);
    hintCount = (total > MAX_PATH) ? MAX_PATH : total;
    hintUsed = true;

    message = "Gold rooms: the insertion path. Check their balance badges!";
    messageColor = Color(255, 215, 70);
    messageTime = 2.4f;
}

int Act3Level4Screen::stars() const
{
    if (state != WON) return 0;

    int s = 1;
    if (steps <= optimal && penalties == 0) s = 3;
    else if (steps * 4 <= 5 * optimal && penalties <= 1) s = 2;

    if (hintUsed && s > 2) s = 2;
    return s;
}

int Act3Level4Screen::score() const
{
    if (state != WON) return 0;
    int s = 1000 - 10 * alarm - 50 * penalties;
    return s < 0 ? 0 : s;
}

void Act3Level4Screen::handleInput()
{
    Event e;

    while (window.pollEvent(e))
    {
        if (e.type == Event::Closed)
            nextScreen = 99;

        if (e.type == Event::MouseButtonPressed && e.mouseButton.button == Mouse::Left)
        {
            int btn = buttonAt((float)e.mouseButton.x, (float)e.mouseButton.y);
            if (btn != -1)
                chooseCase(btn);
        }

        if (e.type == Event::KeyPressed)
        {
            if (e.key.code == Keyboard::Escape)
                nextScreen = 25;

            else if (e.key.code == Keyboard::R)
                newPuzzle();

            else if (e.key.code == Keyboard::Space)
                restartRun();

            else if (e.key.code == Keyboard::H)
                useHint();

            else if (e.key.code == Keyboard::Num1)
                chooseCase(0);
            else if (e.key.code == Keyboard::Num2)
                chooseCase(1);
            else if (e.key.code == Keyboard::Num3)
                chooseCase(2);
            else if (e.key.code == Keyboard::Num4)
                chooseCase(3);
            else if (e.key.code == Keyboard::Num5)
                chooseCase(4);

            else if (e.key.code == Keyboard::N)
                nextScreen = NEXT_SCREEN;

            else if (e.key.code == Keyboard::Return)
            {
                if (state == WON)
                    nextScreen = NEXT_SCREEN;
            }
        }
    }
}

void Act3Level4Screen::update(float dt)
{
    animTime += dt;
    if (flashTime > 0) flashTime -= dt;
    if (messageTime > 0) messageTime -= dt;

    if (animT < 1.0f)
    {
        animT += dt / 0.7f;
        if (animT > 1.0f) animT = 1.0f;
    }

    if (advancePending && animT >= 1.0f)
    {
        advanceTimer -= dt;
        if (advanceTimer <= 0)
        {
            advancePending = false;
            if (state == PLAYING)
                startRound(roundNo);
        }
    }
}

void Act3Level4Screen::draw()
{
    window.clear();
    window.draw(background);

    drawText(window, font, "ACT 3 - LEVEL 4", 40, 500, 15, Color::White, false);

    // ---- HUD boxes ----
    RectangleShape alarmBox(Vector2f(220, 50));
    RectangleShape roundBox(Vector2f(320, 50));

    alarmBox.setPosition(50, 70);
    roundBox.setPosition(910, 70);
    alarmBox.setFillColor(Color::Black);
    roundBox.setFillColor(Color::Black);
    alarmBox.setOutlineColor(Color(213, 41, 69));
    alarmBox.setOutlineThickness(3);
    roundBox.setOutlineColor(Color(213, 41, 69));
    roundBox.setOutlineThickness(3);

    window.draw(alarmBox);
    window.draw(roundBox);

    float fraction = (capacity > 0) ? (float)alarm / (float)capacity : 0.0f;
    if (fraction > 1.0f) fraction = 1.0f;

    RectangleShape bar(Vector2f(212 * fraction, 42));
    bar.setPosition(54, 74);
    if (fraction < 0.5f)       bar.setFillColor(Color(60, 160, 80, 190));
    else if (fraction < 0.8f)  bar.setFillColor(Color(200, 170, 40, 190));
    else                       bar.setFillColor(Color(200, 60, 60, 190));
    window.draw(bar);

    drawText(window, font, "ALARM: " + to_string(alarm) + " / " + to_string(capacity),
        18, 70, 82, Color::White, false);

    int shownRound = (roundNo < TOTAL_ROUNDS) ? roundNo + 1 : TOTAL_ROUNDS;
    drawText(window, font,
        "ROUND " + to_string(shownRound) + "/" + to_string(TOTAL_ROUNDS) +
        "   KEY: " + to_string(incomingKey),
        18, 1070, 95, Color::White, true);

    if (messageTime > 0)
        drawText(window, font, message, 20, 590, 90, messageColor, true);
    else
        drawText(window, font,
            "Key " + to_string(incomingKey) + " enters the vault. Which rotation will it trigger?",
            18, 590, 90, Color::White, true);

    drawText(window, font, "Click Case: Predict | 1-5: Case | H: Hint | Space: Reset | R: New | Esc: Briefing",
        16, 640, 125, Color(208, 221, 183), true);

    // ---- current (tweened) positions ----
    float px[MAXN];
    float py[MAXN];
    float ease = animT * animT * (3.0f - 2.0f * animT);
    for (int i = 0; i < nodeCount; i++)
    {
        px[i] = startX[i] + (nodes[i].x - startX[i]) * ease;
        py[i] = startY[i] + (nodes[i].y - startY[i]) * ease;
    }

    // ---- edges ----
    for (int i = 0; i < nodeCount; i++)
    {
        int l = indexOfId(nodes[i].leftId);
        int r = indexOfId(nodes[i].rightId);
        if (l != -1) drawEdge(window, px[i], py[i], px[l], py[l]);
        if (r != -1) drawEdge(window, px[i], py[i], px[r], py[r]);
    }

    // ---- rooms ----
    for (int i = 0; i < nodeCount; i++)
    {
        float radius = 30.0f;
        bool isHint = isHintId(nodes[i].id);

        if (isHint)
        {
            int alpha = 170 + (int)(50 * sin(animTime * 6));
            CircleShape halo(radius + 13);
            halo.setOrigin(radius + 13, radius + 13);
            halo.setPosition(px[i], py[i]);
            halo.setFillColor(Color(240, 190, 50, (Uint8)(alpha / 3)));
            window.draw(halo);
        }

        CircleShape circle(radius);
        circle.setOrigin(radius, radius);
        circle.setPosition(px[i], py[i]);
        circle.setFillColor(Color(73, 103, 164));
        circle.setOutlineThickness(7);

        if (nodes[i].id == newId)
            circle.setOutlineColor(Color(120, 255, 120));
        else if (isHint)
            circle.setOutlineColor(Color(255, 215, 70));
        else if (nodes[i].unbalanced)
            circle.setOutlineColor(Color(255, 110, 110));
        else
            circle.setOutlineColor(Color(208, 221, 183));

        window.draw(circle);

        drawText(window, font, to_string(nodes[i].value),
            26, px[i], py[i], Color::White, true);
    }

    // ---- balance badges ----
    for (int i = 0; i < nodeCount; i++)
    {
        int b = nodes[i].balance;
        string txt = (b > 0 ? "+" : "") + to_string(b);
        if (nodes[i].unbalanced) txt += "!";

        RectangleShape badge(Vector2f(48, 24));
        badge.setOrigin(24, 12);
        badge.setPosition(px[i] + 28, py[i] - 28);
        badge.setOutlineThickness(2);
        badge.setOutlineColor(Color::White);
        badge.setFillColor(nodes[i].unbalanced ? Color(200, 60, 60) : Color(60, 160, 80));
        window.draw(badge);

        drawText(window, font, txt, 16, px[i] + 28, py[i] - 28, Color::White, true);
    }

    Vector2i mp = Mouse::getPosition(window);

    // ---- hover tooltip ----
    int hov = findNodeAt((float)mp.x, (float)mp.y);
    if (hov != -1)
    {
        float tx = (float)mp.x + 16;
        float ty = (float)mp.y + 16;
        if (tx > 1030) tx = 1030;
        if (ty > 560) ty = 560;

        RectangleShape tip(Vector2f(230, 66));
        tip.setPosition(tx, ty);
        tip.setFillColor(Color(0, 0, 0, 220));
        tip.setOutlineColor(Color(180, 100, 220));
        tip.setOutlineThickness(2);
        window.draw(tip);

        drawText(window, font,
            "Key: " + to_string(nodes[hov].value) + "   Depth: " + to_string(nodes[hov].depth),
            16, tx + 10, ty + 8, Color::White, false);
        drawText(window, font,
            "Height: " + to_string(nodes[hov].height) + "   Size: " + to_string(nodes[hov].size),
            16, tx + 10, ty + 34, Color::White, false);
    }

    // ---- prediction buttons + incoming room (only while playing) ----
    if (state == PLAYING)
    {
        int hoverBtn = buttonAt((float)mp.x, (float)mp.y);
        bool locked = advancePending || animT < 1.0f;

        for (int b = 0; b < NUM_CHOICES; b++)
        {
            RectangleShape btn(Vector2f(124, 40));
            btn.setPosition(360.0f + b * 132.0f, 636.0f);

            string label = to_string(b + 1) + ": " + AVLTree::rotationName(caseOfButton[b]);
            Color textColor = Color::Black;

            if (advancePending && caseOfButton[b] == expectedCase)
            {
                btn.setFillColor(Color(60, 160, 80, 230));
                btn.setOutlineThickness(3);
                btn.setOutlineColor(Color::White);
                label += " OK";
                textColor = Color::White;
            }
            else if (triedWrong[b])
            {
                btn.setFillColor(Color(200, 60, 60, 170));
                btn.setOutlineThickness(0);
                label += " X";
                textColor = Color::White;
            }
            else if (!locked && hoverBtn == b)
            {
                btn.setFillColor(Color(212, 110, 194, 230));
                btn.setOutlineThickness(3);
                btn.setOutlineColor(Color::White);
            }
            else
            {
                btn.setFillColor(Color(212, 110, 194, 120));
                btn.setOutlineThickness(0);
            }

            window.draw(btn);
            drawText(window, font, label, 18, 360.0f + b * 132.0f + 62.0f, 656.0f, textColor, true);
        }

        // incoming key
        CircleShape incoming(24.0f);
        incoming.setOrigin(24.0f, 24.0f);
        incoming.setPosition(1055.0f, 656.0f);
        incoming.setFillColor(Color(73, 103, 164));
        incoming.setOutlineThickness(5);
        incoming.setOutlineColor(Color(255, 215, 70));
        window.draw(incoming);
        drawText(window, font, to_string(incomingKey), 22, 1055.0f, 656.0f, Color::White, true);
    }

    // ---- bottom line ----
    drawText(window, font, "BF = height(left) - height(right)",
        16, 60, 646, Color(208, 221, 183), false);
    drawText(window, font, "SEED: " + to_string(puzzleSeed),
        16, 1100, 672, Color(208, 221, 183), false);

    if (state == WON)
    {
        drawText(window, font,
            "VAULT STABLE!  STARS: " + to_string(stars()) + "  SCORE: " + to_string(score()) + "  (ENTER)",
            26, 640, 672, Color::Green, true);
    }
    else if (state == LOST)
    {
        drawText(window, font, "ALARM TRIGGERED! YOU LOST!   (R: retry)",
            27, 640, 672, Color::Red, true);
    }

    if (flashTime > 0)
    {
        RectangleShape redScreen(Vector2f(1280, 720));
        redScreen.setFillColor(Color(255, 0, 0, 90));
        window.draw(redScreen);
    }
}