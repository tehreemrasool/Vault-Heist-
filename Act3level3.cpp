#include "Act3Level3.h"
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

Act3Level3Screen::Act3Level3Screen(RenderWindow& win) : Screen(win)
{
    loadFont(font);

    if (backgroundTexture.loadFromFile("assets/pictures/Act3_level3.jpeg"))
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
    roundCase = AVLTree::ROT_NONE;
    identified = false;
    doneRot = 0;
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
    advancePending = false;
    advanceTimer = 0;
    seed = (unsigned)time(NULL) + 317;
    puzzleSeed = seed;
    flashTime = 0;
    messageTime = 0;
    animTime = 0;
    messageColor = Color::White;

    caseOfButton[0] = AVLTree::ROT_LL;
    caseOfButton[1] = AVLTree::ROT_RR;
    caseOfButton[2] = AVLTree::ROT_LR;
    caseOfButton[3] = AVLTree::ROT_RL;

    for (int i = 0; i < 4; i++) triedWrong[i] = false;

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

// LR and RL alternate; which one comes first depends on the seed
int Act3Level3Screen::caseForRound(int r) const
{
    int a = (puzzleSeed % 2 == 0) ? AVLTree::ROT_LR : AVLTree::ROT_RL;
    int b = (a == AVLTree::ROT_LR) ? AVLTree::ROT_RL : AVLTree::ROT_LR;
    return (r % 2 == 0) ? a : b;
}

int Act3Level3Screen::indexOfId(int id) const
{
    for (int i = 0; i < nodeCount; i++)
        if (nodes[i].id == id) return i;
    return -1;
}

void Act3Level3Screen::layoutRec(int i, int& pos)
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

void Act3Level3Screen::loadModel()
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

void Act3Level3Screen::snapshotPositions()
{
    oldCount = nodeCount;
    for (int i = 0; i < nodeCount; i++)
    {
        oldId[i] = nodes[i].id;
        oldX[i] = nodes[i].x;
        oldY[i] = nodes[i].y;
    }
}

void Act3Level3Screen::startTween()
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

int Act3Level3Screen::unbalancedCount() const
{
    int c = 0;
    for (int i = 0; i < nodeCount; i++)
        if (nodes[i].unbalanced) c++;
    return c;
}

void Act3Level3Screen::startRound(int r)
{
    unsigned roundSeed = puzzleSeed + (unsigned)r * 17;

    tree.resetTree();
    tree.loadCase(caseForRound(r), roundSeed);
    loadModel();

    // the model decides which case this tree really is
    int pivotId = -1;
    int rc = AVLTree::ROT_NONE;
    if (tree.getHint(pivotId, rc))
        roundCase = rc;
    else
        roundCase = caseForRound(r);

    identified = false;
    doneRot = 0;
    hintId = -1;
    dragIdx = -1;
    for (int i = 0; i < 4; i++) triedWrong[i] = false;
}

void Act3Level3Screen::restartRun()
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

void Act3Level3Screen::newPuzzle()
{
    puzzleSeed = seed++;
    restartRun();
}

void Act3Level3Screen::onEnter()
{
    isActive = true;
    nextScreen = SCREEN_ID;
    newPuzzle();
}

void Act3Level3Screen::onResume()
{
    isActive = true;
    nextScreen = SCREEN_ID;
    flashTime = 0;
    dragIdx = -1;
}

int Act3Level3Screen::findNodeAt(float x, float y) const
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

// four buttons at the bottom: x = 400 + b*142, y = 636, size 130 x 40
int Act3Level3Screen::buttonAt(float x, float y) const
{
    for (int b = 0; b < 4; b++)
    {
        float bx = 400.0f + b * 142.0f;
        if (x >= bx && x <= bx + 130.0f && y >= 636.0f && y <= 676.0f)
            return b;
    }
    return -1;
}

// asks the model what the next correct drag is (room id + direction)
bool Act3Level3Screen::expectedMove(int& nodeId, char& dir)
{
    int pivotId = -1;
    int rc = AVLTree::ROT_NONE;
    if (!tree.getHint(pivotId, rc)) return false;

    if (rc == AVLTree::ROT_LL) { nodeId = pivotId; dir = 'R'; return true; }
    if (rc == AVLTree::ROT_RR) { nodeId = pivotId; dir = 'L'; return true; }

    int pi = indexOfId(pivotId);
    if (pi == -1) return false;

    if (rc == AVLTree::ROT_LR) { nodeId = nodes[pi].leftId;  dir = 'L'; return true; }
    if (rc == AVLTree::ROT_RL) { nodeId = nodes[pi].rightId; dir = 'R'; return true; }

    return false;
}

void Act3Level3Screen::chooseCase(int b)
{
    if (state != PLAYING || identified || advancePending || animT < 1.0f) return;
    if (b < 0 || b > 3) return;
    if (triedWrong[b]) return;

    if (caseOfButton[b] == roundCase)
    {
        identified = true;
        hintId = -1;
        message = string("Correct: ") + AVLTree::rotationName(roundCase) +
            " case. Now rotate in two steps.";
        messageColor = Color(120, 255, 120);
        messageTime = 2.0f;
    }
    else
    {
        triedWrong[b] = true;
        alarm += 2;
        penalties++;
        flashTime = 0.35f;
        message = "Wrong case! Alarm +2";
        messageColor = Color(255, 110, 110);
        messageTime = 1.6f;

        if (alarm >= capacity) state = LOST;
    }
}

void Act3Level3Screen::tryRotation(int idx, int dx)
{
    if (state != PLAYING) return;

    if (!identified)
    {
        message = "Identify the case first!";
        messageColor = Color(255, 200, 90);
        messageTime = 1.4f;
        return;
    }

    int needId = -1;
    char needDir = 'L';
    if (!expectedMove(needId, needDir)) return;

    char dir = (dx > 0) ? 'R' : 'L';

    steps++;

    if (nodes[idx].id != needId)
    {
        alarm += 2;
        penalties++;
        flashTime = 0.35f;
        message = "Wrong room for this step! Alarm +2";
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

        if (!tree.applyManualRotation(needId, dir))
        {
            steps--;
            message = "That rotation is not possible here.";
            messageColor = Color(255, 200, 90);
            messageTime = 1.3f;
            return;
        }

        alarm++;
        doneRot++;
        loadModel();
        startTween();
        hintId = -1;

        if (unbalancedCount() == 0)
        {
            message = string(AVLTree::rotationName(roundCase)) + " repaired in " +
                to_string(doneRot) + " rotations!";
            messageColor = Color(120, 255, 120);
            messageTime = 1.8f;

            roundNo++;
            if (roundNo >= TOTAL_ROUNDS)
                state = WON;
            else
            {
                advancePending = true;
                advanceTimer = 1.3f;
            }
        }
        else
        {
            message = "Rotation " + to_string(doneRot) + " done. Keep repairing!";
            messageColor = Color(120, 255, 120);
            messageTime = 1.6f;
        }
    }

    if (state == PLAYING && alarm >= capacity)
        state = LOST;
}

void Act3Level3Screen::useHint()
{
    if (state != PLAYING || advancePending) return;

    if (!identified)
    {
        int pivotId = -1;
        int rc = AVLTree::ROT_NONE;
        if (!tree.getHint(pivotId, rc)) return;

        hintId = pivotId;
        hintUsed = true;
        message = "Gold room is the unbalanced pivot. Which case is it?";
        messageColor = Color(255, 215, 70);
        messageTime = 2.0f;
    }
    else
    {
        int needId = -1;
        char needDir = 'L';
        if (!expectedMove(needId, needDir)) return;

        hintId = needId;
        hintUsed = true;
        message = "Gold room is the one to drag next.";
        messageColor = Color(255, 215, 70);
        messageTime = 2.0f;
    }
}

int Act3Level3Screen::stars() const
{
    if (state != WON) return 0;

    int s = 1;
    if (steps <= optimal && penalties == 0) s = 3;
    else if (steps * 4 <= 5 * optimal && penalties <= 1) s = 2;

    if (hintUsed && s > 2) s = 2;
    return s;
}

int Act3Level3Screen::score() const
{
    if (state != WON) return 0;
    int s = 1000 - 10 * alarm - 50 * penalties;
    return s < 0 ? 0 : s;
}

void Act3Level3Screen::handleInput()
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
                int btn = buttonAt((float)e.mouseButton.x, (float)e.mouseButton.y);
                if (btn != -1)
                {
                    chooseCase(btn);
                }
                else
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
                else if (!identified)
                {
                    message = "Identify the case first: LL, RR, LR or RL?";
                    messageColor = Color(255, 200, 90);
                    messageTime = 1.6f;
                }
                else
                {
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

            else if (e.key.code == Keyboard::Num1)
                chooseCase(0);
            else if (e.key.code == Keyboard::Num2)
                chooseCase(1);
            else if (e.key.code == Keyboard::Num3)
                chooseCase(2);
            else if (e.key.code == Keyboard::Num4)
                chooseCase(3);

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

void Act3Level3Screen::update(float dt)
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

void Act3Level3Screen::draw()
{
    window.clear();
    window.draw(background);

    drawText(window, font, "ACT 3 - LEVEL 3", 40, 500, 15, Color::White, false);

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
    int rotTotal = AVLTree::expectedRotationSteps(roundCase);
    drawText(window, font,
        "ROUND " + to_string(shownRound) + "/" + to_string(TOTAL_ROUNDS) +
        "   ROT " + to_string(doneRot) + "/" + to_string(rotTotal),
        18, 1070, 95, Color::White, true);

    if (messageTime > 0)
        drawText(window, font, message, 20, 590, 90, messageColor, true);
    else if (!identified)
        drawText(window, font, "Which case is it? Click LL, RR, LR or RL (keys 1-4)",
            18, 590, 90, Color::White, true);
    else
        drawText(window, font, "Rotate in two steps: drag the right rooms sideways",
            18, 590, 90, Color::White, true);

    drawText(window, font, "Drag Room: Rotate | 1-4: Case | H: Hint | Space: Reset | R: New | Esc: Briefing",
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

        if (i == dragIdx)
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

    // ---- dragged room follows the mouse ----
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
    }

    // ---- case buttons (only while playing) ----
    if (state == PLAYING)
    {
        int hoverBtn = buttonAt((float)mp.x, (float)mp.y);

        for (int b = 0; b < 4; b++)
        {
            RectangleShape btn(Vector2f(130, 40));
            btn.setPosition(400.0f + b * 142.0f, 636.0f);

            string label = to_string(b + 1) + ": " + AVLTree::rotationName(caseOfButton[b]);
            Color textColor = Color::Black;

            if (identified && caseOfButton[b] == roundCase)
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
            else if (!identified && hoverBtn == b)
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
            drawText(window, font, label, 20, 400.0f + b * 142.0f + 65.0f, 656.0f, textColor, true);
        }
    }

    // ---- bottom line ----
    drawText(window, font, "BF = height(left) - height(right)",
        16, 60, 646, Color(208, 221, 183), false);
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