#include "Act3Level2.h"
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

Act3Level2Screen::Act3Level2Screen(RenderWindow& win) : Screen(win)
{
    loadFont(font);

    if (backgroundTexture.loadFromFile("assets/pictures/Act3_level2.jpg"))
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
    roundNo = 0;
    optimal = 0;
    capacity = 0;
    alarm = 0;
    penalties = 0;
    steps = 0;
    hintUsed = false;
    hintId = -1;
    state = LOST;
    dragIdx = -1;
    dragStartX = 0;
    dragStartY = 0;
    selectedIdx = -1;
    advancePending = false;
    advanceTimer = 0;
    seed = (unsigned)time(NULL) + 211;
    puzzleSeed = seed;
    flashTime = 0;
    messageTime = 0;
    animTime = 0;
    messageColor = Color::White;

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

// LL and RR alternate; which one comes first depends on the seed
int Act3Level2Screen::caseForRound(int r) const
{
    int a = (puzzleSeed % 2 == 0) ? AVLTree::ROT_LL : AVLTree::ROT_RR;
    int b = (a == AVLTree::ROT_LL) ? AVLTree::ROT_RR : AVLTree::ROT_LL;
    return (r % 2 == 0) ? a : b;
}

int Act3Level2Screen::indexOfId(int id) const
{
    for (int i = 0; i < nodeCount; i++)
        if (nodes[i].id == id) return i;
    return -1;
}

void Act3Level2Screen::layoutRec(int i, int& pos)
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

void Act3Level2Screen::loadModel()
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

void Act3Level2Screen::snapshotPositions()
{
    oldCount = nodeCount;
    for (int i = 0; i < nodeCount; i++)
    {
        oldId[i] = nodes[i].id;
        oldX[i] = nodes[i].x;
        oldY[i] = nodes[i].y;
    }
}

void Act3Level2Screen::startTween()
{
    for (int i = 0; i < nodeCount; i++)
    {
        for (int j = 0; j < oldCount; j++)
        {
            if (oldId[j] == nodes[i].id)
            {
                startX[i] = oldX[j];
                startY[i] = oldY[j];
                break;
            }
        }
    }
    animT = 0.0f;
}

int Act3Level2Screen::unbalancedCount() const
{
    int c = 0;
    for (int i = 0; i < nodeCount; i++)
        if (nodes[i].unbalanced) c++;
    return c;
}

void Act3Level2Screen::startRound(int r)
{
    unsigned roundSeed = puzzleSeed + (unsigned)r * 17;

    tree.resetTree();
    tree.loadCase(caseForRound(r), roundSeed);
    loadModel();

    hintId = -1;
    selectedIdx = -1;
    dragIdx = -1;
}

void Act3Level2Screen::restartRun()
{
    roundNo = 0;

    optimal = 0;
    for (int r = 0; r < TOTAL_ROUNDS; r++)
        optimal += AVLTree::expectedRotationSteps(caseForRound(r));
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

    startRound(0);
}

void Act3Level2Screen::newPuzzle()
{
    puzzleSeed = seed++;
    restartRun();
}

void Act3Level2Screen::onEnter()
{
    isActive = true;
    nextScreen = SCREEN_ID;
    newPuzzle();
}

void Act3Level2Screen::onResume()
{
    isActive = true;
    nextScreen = SCREEN_ID;
    flashTime = 0;
    dragIdx = -1;
}

int Act3Level2Screen::findNodeAt(float x, float y) const
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

void Act3Level2Screen::tryRotation(int idx, int dx)
{
    if (state != PLAYING) return;

    int pivotId = -1;
    int rc = AVLTree::ROT_NONE;
    if (!tree.getHint(pivotId, rc)) return;    

    char dir = (dx > 0) ? 'R' : 'L';
    char needDir = (rc == AVLTree::ROT_LL) ? 'R' : 'L';  

    steps++;

    if (nodes[idx].id != pivotId)
    {
        alarm += 2;
        penalties++;
        flashTime = 0.35f;
        message = "Wrong room! Fix the LOWEST unbalanced room. Alarm +2";
        messageColor = Color(255, 110, 110);
        messageTime = 1.8f;
    }
    else if (dir != needDir)
    {
        alarm += 2;
        penalties++;
        flashTime = 0.35f;
        message = "Right room, wrong direction! Alarm +2";
        messageColor = Color(255, 110, 110);
        messageTime = 1.8f;
    }
    else
    {
        snapshotPositions();

        if (!tree.applyManualRotation(pivotId, dir))
        {
            steps--;
            message = "That rotation is not possible here.";
            messageColor = Color(255, 200, 90);
            messageTime = 1.3f;
            return;
        }

        alarm++;
        loadModel();
        startTween();
        hintId = -1;
        selectedIdx = -1;

        message = string(AVLTree::rotationName(rc)) + " fixed with a " +
            ((dir == 'R') ? "RIGHT" : "LEFT") + " rotation!";
        messageColor = Color(120, 255, 120);
        messageTime = 1.8f;

        if (unbalancedCount() == 0)
        {
            roundNo++;
            if (roundNo >= TOTAL_ROUNDS)
                state = WON;
            else
            {
                advancePending = true;
                advanceTimer = 1.3f;
            }
        }
    }

    if (state == PLAYING && alarm >= capacity)
        state = LOST;
}

void Act3Level2Screen::useHint()
{
    if (state != PLAYING) return;

    int pivotId = -1;
    int rc = AVLTree::ROT_NONE;
    if (!tree.getHint(pivotId, rc)) return;

    hintId = pivotId;
    hintUsed = true;
    message = string("Gold room is the pivot. Case: ") + AVLTree::rotationName(rc);
    messageColor = Color(255, 215, 70);
    messageTime = 2.0f;
}

int Act3Level2Screen::stars() const
{
    if (state != WON) return 0;

    int s = 1;
    if (steps <= optimal && penalties == 0) s = 3;
    else if (steps * 4 <= 5 * optimal && penalties <= 1) s = 2;

    if (hintUsed && s > 2) s = 2;
    return s;
}

int Act3Level2Screen::score() const
{
    if (state != WON) return 0;
    int s = 1000 - 10 * alarm - 50 * penalties;
    return s < 0 ? 0 : s;
}

void Act3Level2Screen::handleInput()
{
    Event e;

    while (window.pollEvent(e))
    {
        if (e.type == Event::Closed)
            nextScreen = 99;

        if (e.type == Event::MouseButtonPressed && e.mouseButton.button == Mouse::Left)
        {
            if (state == PLAYING && animT >= 1.0f && !advancePending)
            {
                int hit = findNodeAt((float)e.mouseButton.x, (float)e.mouseButton.y);
                if (hit != -1)
                {
                    dragIdx = hit;
                    dragStartX = (float)e.mouseButton.x;
                    dragStartY = (float)e.mouseButton.y;
                }
            }
        }

        if (e.type == Event::MouseButtonReleased && e.mouseButton.button == Mouse::Left)
        {
            if (dragIdx != -1)
            {
                int dx = e.mouseButton.x - (int)dragStartX;
                int adx = (dx < 0) ? -dx : dx;

                if (adx >= DRAG_MIN)
                {
                    tryRotation(dragIdx, dx);
                }
                else
                {
                    selectedIdx = dragIdx;
                    message = "Drag the room sideways: right = RIGHT rotation, left = LEFT rotation";
                    messageColor = Color(255, 200, 90);
                    messageTime = 1.8f;
                }
                dragIdx = -1;
            }
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

void Act3Level2Screen::update(float dt)
{
    animTime += dt;
    if (flashTime > 0) flashTime -= dt;
    if (messageTime > 0) messageTime -= dt;

    if (animT < 1.0f)
    {
        animT += dt / 0.6f;
        if (animT > 1.0f) animT = 1.0f;
    }

    if (advancePending && animT >= 1.0f)
    {
        advanceTimer -= dt;
        if (advanceTimer <= 0)
        {
            advancePending = false;
            startRound(roundNo);
        }
    }
}

void Act3Level2Screen::draw()
{
    window.clear();
    window.draw(background);

    drawText(window, font, "ACT 3 - LEVEL 2", 40, 500, 15, Color::White, false);

    RectangleShape alarmBox(Vector2f(220, 50));
    RectangleShape roundBox(Vector2f(320, 50));

    alarmBox.setPosition(50, 70);
    roundBox.setPosition(910, 70);
    alarmBox.setFillColor(Color::Black);
    roundBox.setFillColor(Color::Black);
    alarmBox.setOutlineColor(Color(180, 100, 220));
    alarmBox.setOutlineThickness(3);
    roundBox.setOutlineColor(Color(180, 100, 220));
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
    drawText(window, font, "ROUND: " + to_string(shownRound) + " / " + to_string(TOTAL_ROUNDS),
        20, 1070, 95, Color::White, true);

    if (messageTime > 0)
        drawText(window, font, message, 20, 590, 90, messageColor, true);
    else
        drawText(window, font, "Drag the lowest unbalanced room sideways to rotate it",
            18, 590, 90, Color::White, true);

    drawText(window, font, "Drag Room: Rotate | H: Hint | Space: Reset | R: New | Esc: Briefing",
        16, 640, 125, Color(208, 221, 183), true);

    float px[MAXN];
    float py[MAXN];
    float ease = animT * animT * (3.0f - 2.0f * animT);
    for (int i = 0; i < nodeCount; i++)
    {
        px[i] = startX[i] + (nodes[i].x - startX[i]) * ease;
        py[i] = startY[i] + (nodes[i].y - startY[i]) * ease;
    }

    for (int i = 0; i < nodeCount; i++)
    {
        int l = indexOfId(nodes[i].leftId);
        int r = indexOfId(nodes[i].rightId);
        if (l != -1) drawEdge(window, px[i], py[i], px[l], py[l]);
        if (r != -1) drawEdge(window, px[i], py[i], px[r], py[r]);
    }

    for (int i = 0; i < nodeCount; i++)
    {
        float radius = 30.0f;
        bool isHint = (nodes[i].id == hintId);

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

        if (i == dragIdx || i == selectedIdx)
            circle.setOutlineColor(Color(212, 110, 194));
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

    if (dragIdx != -1)
    {
        CircleShape ghost(30.0f);
        ghost.setOrigin(30.0f, 30.0f);
        ghost.setPosition((float)mp.x, (float)mp.y);
        ghost.setFillColor(Color(73, 103, 164, 200));
        ghost.setOutlineThickness(7);
        ghost.setOutlineColor(Color(212, 110, 194));
        window.draw(ghost);

        drawText(window, font, to_string(nodes[dragIdx].value),
            26, (float)mp.x, (float)mp.y, Color::White, true);

        int dx = mp.x - (int)dragStartX;
        int adx = (dx < 0) ? -dx : dx;
        string label;
        if (adx < DRAG_MIN)  label = "drag sideways...";
        else if (dx > 0)     label = "ROTATE RIGHT >>";
        else                 label = "<< ROTATE LEFT";

        drawText(window, font, label, 18, (float)mp.x, (float)mp.y - 52,
            Color(255, 215, 70), true);
    }
    else
    {
        int hov = findNodeAt((float)mp.x, (float)mp.y);
        if (hov != -1)
        {
            float tx = (float)mp.x + 16;
            float ty = (float)mp.y + 16;
            if (tx > 1030) tx = 1030;
            if (ty > 640) ty = 640;

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
    }

    drawText(window, font, "BF = height(left) - height(right)",
        16, 60, 640, Color(208, 221, 183), false);
    drawText(window, font, "SEED: " + to_string(puzzleSeed),
        16, 1100, 672, Color(208, 221, 183), false);

    if (state == WON)
    {
        drawText(window, font,
            "ALL REPAIRED!  STARS: " + to_string(stars()) + "  SCORE: " + to_string(score()) + "  (ENTER)",
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