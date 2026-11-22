#include "Act3Level5.h"
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

static string ordinal(int n)
{
    string suffix = "th";
    int m100 = n % 100;
    if (m100 < 11 || m100 > 13)
    {
        int m10 = n % 10;
        if (m10 == 1) suffix = "st";
        else if (m10 == 2) suffix = "nd";
        else if (m10 == 3) suffix = "rd";
    }
    return to_string(n) + suffix;
}

Act3Level5Screen::Act3Level5Screen(RenderWindow& win) : Screen(win)
{
    loadFont(font);

    if (backgroundTexture.loadFromFile("assets/pictures/Act3_level5.jpeg"))
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
    roundType = T_SMALL;
    roundK = 1;
    roundRank = 1;
    curK = 1;
    pathCount = 0;
    step = 0;
    corruptTrueSize = 0;
    checkedCount = 0;
    optimal = 0;
    capacity = 0;
    alarm = 0;
    penalties = 0;
    steps = 0;
    hintUsed = false;
    hintId = -1;
    state = LOST;
    advancePending = false;
    advanceTimer = 0;
    seed = (unsigned)time(NULL) + 523;
    puzzleSeed = seed;
    flashTime = 0;
    messageTime = 0;
    animTime = 0;
    messageColor = Color::White;

    for (int i = 0; i < MAX_PATH; i++) path[i] = -1;

    for (int i = 0; i < MAXN; i++)
    {
        startX[i] = 0;
        startY[i] = 0;
        oldId[i] = -1;
        oldX[i] = 0;
        oldY[i] = 0;
        checkedIds[i] = -1;
    }

    tree.setRecordEvents(false);
}

int Act3Level5Screen::indexOfId(int id) const
{
    for (int i = 0; i < nodeCount; i++)
        if (nodes[i].id == id) return i;
    return -1;
}

void Act3Level5Screen::layoutRec(int i, int& pos)
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

void Act3Level5Screen::loadModel()
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

void Act3Level5Screen::snapshotPositions()
{
    oldCount = nodeCount;
    for (int i = 0; i < nodeCount; i++)
    {
        oldId[i] = nodes[i].id;
        oldX[i] = nodes[i].x;
        oldY[i] = nodes[i].y;
    }
}

void Act3Level5Screen::startTween()
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

void Act3Level5Screen::planRound(AVLTree& t, int r, int& type, int& k, int& rank,
    int& clicks, int& mutKey, int& mutKind)
{
    unsigned rng = puzzleSeed * 7919u + (unsigned)(r + 1) * 104729u;
    mutKey = -1;
    mutKind = 0;

    if (r == 1 || r == 2 || r == 4)
    {
        int key = -1;
        for (int tries = 0; tries < 300; tries++)
        {
            int cand = (int)(AVLTree::nextRandom(rng) % 100);
            if (!t.containsKey(cand)) { key = cand; break; }
        }
        if (key == -1) key = 100 + r;

        if (t.insert(key)) { mutKey = key; mutKind = 1; }
    }
    else if (r == 3 && t.nodeCount() > 6)
    {
        int pick = 1 + (int)(AVLTree::nextRandom(rng) % (unsigned)t.nodeCount());
        AVLNode* target = t.kth(pick);
        if (target != NULL)
        {
            int key = target->value;
            if (t.deleteNode(key)) { mutKey = key; mutKind = 2; }
        }
    }

    if (r == 0) type = T_SMALL;
    else if (r % 2 == 1) type = T_AUDIT;
    else type = T_LARGE;

    k = 0;
    rank = 0;
    clicks = 1;

    if (type != T_AUDIT)
    {
        int n = t.nodeCount();
        int tmp[MAX_PATH];

        for (int tries = 0; tries < 30 && k == 0; tries++)
        {
            int pick = 1 + (int)(AVLTree::nextRandom(rng) % (unsigned)n);
            int rk = (type == T_LARGE) ? (n - pick + 1) : pick;
            int cnt = t.kthPath(rk, tmp, MAX_PATH);
            if (cnt >= 2)
            {
                k = pick;
                rank = rk;
                clicks = cnt - 1;          
            }
        }

        if (k == 0)                       
        {
            k = 1;
            rank = (type == T_LARGE) ? n : 1;
            int cnt = t.kthPath(rank, tmp, MAX_PATH);
            clicks = (cnt > 1) ? cnt - 1 : 1;
        }
    }
}

void Act3Level5Screen::updateGuide()
{
    guide.clear();
    if (roundType == T_AUDIT || step < 1 || step > pathCount) return;

    int ci = indexOfId(path[step - 1]);
    if (ci == -1) return;

    int li = indexOfId(nodes[ci].leftId);
    int leftSize = (li == -1) ? 0 : nodes[li].size;
    string who = "Room " + to_string(nodes[ci].value);

    if (step >= pathCount)
        guide = who + ": left holds " + to_string(leftSize) +
        " rooms, so #" + to_string(curK) + " is HERE!";
    else
        guide = who + ": left holds " + to_string(leftSize) +
        " rooms. Looking for #" + to_string(curK);
}

void Act3Level5Screen::advanceK()
{
    int ci = indexOfId(path[step - 1]);
    if (ci == -1) return;

    int li = indexOfId(nodes[ci].leftId);
    int leftSize = (li == -1) ? 0 : nodes[li].size;

    if (curK > leftSize + 1)
        curK -= leftSize + 1;
}

void Act3Level5Screen::startRound(int r)
{
    snapshotPositions();

    int type, k, rank, clicks, mutKey, mutKind;
    planRound(tree, r, type, k, rank, clicks, mutKey, mutKind);

    roundType = type;
    roundK = k;
    roundRank = rank;

    loadModel();

    hintId = -1;
    checkedCount = 0;
    newId = -1;
    pathCount = 0;
    step = 0;
    guide.clear();

    if (mutKind != 0)
    {
        int rots = tree.getLastRotationCount();
        bool valid = tree.isBalanced();     
        startTween();

        message = string((mutKind == 1) ? "Key " : "Key ") + to_string(mutKey) +
            ((mutKind == 1) ? " inserted, " : " removed, ") + to_string(rots) +
            " rot. Sizes " + (valid ? "valid" : "INVALID");
        messageColor = Color(255, 215, 70);
        messageTime = 2.4f;
    }

    if (roundType == T_AUDIT)
    {
        unsigned rng = puzzleSeed * 31u + (unsigned)(r + 3) * 977u;
        int idx = (int)(AVLTree::nextRandom(rng) % (unsigned)nodeCount);
        int fake = nodes[idx].size + 1 + (int)(AVLTree::nextRandom(rng) % 2);

        corruptTrueSize = nodes[idx].size;
        tree.forceStoredSize(nodes[idx].id, fake);
        nodes[idx].size = fake;             
    }
    else
    {
        pathCount = tree.kthPath(roundRank, path, MAX_PATH);
        if (pathCount > MAX_PATH) pathCount = MAX_PATH;
        step = 1;                        
        curK = roundRank;
        updateGuide();
    }
}

void Act3Level5Screen::restartRun()
{
    roundNo = 0;

    unsigned st = puzzleSeed;
    int n = 8 + (int)(AVLTree::nextRandom(st) % 3);       // 8..10 starting rooms

    tree.resetTree();
    tree.buildRandomAVL(puzzleSeed, n, 99);

    AVLTree sim;
    sim.setRecordEvents(false);
    sim.cloneFrom(tree);

    optimal = 0;
    for (int r = 0; r < TOTAL_ROUNDS; r++)
    {
        int type, k, rank, clicks, mutKey, mutKind;
        planRound(sim, r, type, k, rank, clicks, mutKey, mutKind);
        optimal += clicks;
    }
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
    guide.clear();
    state = PLAYING;

    loadModel();
    startRound(0);
}

void Act3Level5Screen::newPuzzle()
{
    puzzleSeed = seed++;
    restartRun();
}

void Act3Level5Screen::onEnter()
{
    isActive = true;
    nextScreen = SCREEN_ID;
    newPuzzle();
}

void Act3Level5Screen::onResume()
{
    isActive = true;
    nextScreen = SCREEN_ID;
    flashTime = 0;
}

int Act3Level5Screen::findNodeAt(float x, float y) const
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

bool Act3Level5Screen::isVisited(int id) const
{
    if (roundType == T_AUDIT) return false;
    for (int i = 0; i < step && i < pathCount; i++)
        if (path[i] == id) return true;
    return false;
}

bool Act3Level5Screen::isChecked(int id) const
{
    for (int i = 0; i < checkedCount; i++)
        if (checkedIds[i] == id) return true;
    return false;
}

void Act3Level5Screen::roundDone()
{
    roundNo++;
    if (roundNo >= TOTAL_ROUNDS)
        state = WON;
    else
    {
        advancePending = true;
        advanceTimer = 1.6f;
    }
}

void Act3Level5Screen::handleNodeClick(int i)
{
    if (state != PLAYING) return;

    newId = -1;
    hintId = -1;

    if (roundType == T_AUDIT)
    {
        if (isChecked(nodes[i].id))
        {
            message = "You already checked that room.";
            messageColor = Color(255, 200, 90);
            messageTime = 1.0f;
            return;
        }

        steps++;
        int bad = tree.findWrongSizeId();       // the model knows which size is corrupted

        if (nodes[i].id == bad)
        {
            tree.forceStoredSize(bad, corruptTrueSize);     // repair it
            nodes[i].size = corruptTrueSize;
            if (checkedCount < MAXN) checkedIds[checkedCount++] = bad;

            alarm++;
            message = "Size flagged and repaired!";
            messageColor = Color(120, 255, 120);
            messageTime = 1.8f;
            roundDone();
        }
        else
        {
            bool aboveBad = (bad != -1) && (nodes[i].leftId == bad || nodes[i].rightId == bad);

            alarm += 2;
            penalties++;
            flashTime = 0.35f;
            if (aboveBad)
                message = "Only off because of a child. Look lower! Alarm +2";
            else
            {
                message = "That size is correct! Alarm +2";
                if (checkedCount < MAXN) checkedIds[checkedCount++] = nodes[i].id;
            }
            messageColor = Color(255, 110, 110);
            messageTime = 1.8f;
        }
    }
    else
    {
        if (isVisited(nodes[i].id))
        {
            message = "You already stand in that room.";
            messageColor = Color(255, 200, 90);
            messageTime = 1.0f;
            return;
        }

        steps++;

        if (step < pathCount && nodes[i].id == path[step])
        {
            int oldK = curK;
            advanceK();
            step++;
            alarm++;
            updateGuide();

            if (step >= pathCount)
            {
                message = "FOUND IT! Room " + to_string(nodes[i].value) + " is the " +
                    ordinal(roundK) + ((roundType == T_LARGE) ? " most valuable!" : " smallest!");
                messageColor = Color(120, 255, 120);
                messageTime = 2.2f;
                roundDone();
            }
            else
            {
                message = (curK != oldK)
                    ? "Correct! Turn RIGHT, new target #" + to_string(curK)
                    : "Correct! Turn LEFT";
                messageColor = Color(120, 255, 120);
                messageTime = 1.6f;
            }
        }
        else
        {
            alarm += 2;
            penalties++;
            flashTime = 0.35f;

            int ci = (step >= 1) ? indexOfId(path[step - 1]) : -1;
            bool isChild = (ci != -1) &&
                (nodes[i].id == nodes[ci].leftId || nodes[i].id == nodes[ci].rightId);

            message = isChild ? "Wrong side! Compare # with left size. Alarm +2"
                : "Only step into a child room! Alarm +2";
            messageColor = Color(255, 110, 110);
            messageTime = 1.8f;
        }
    }

    if (state == PLAYING && alarm >= capacity)
        state = LOST;
}

void Act3Level5Screen::useHint()
{
    if (state != PLAYING || advancePending || animT < 1.0f) return;

    if (roundType == T_AUDIT)
    {
        int bad = tree.findWrongSizeId();
        if (bad == -1) return;
        hintId = bad;
        message = "Gold room: its stored size is wrong.";
    }
    else
    {
        if (step >= pathCount) return;
        hintId = path[step];
        message = "Gold room: step into this one next.";
    }

    hintUsed = true;
    messageColor = Color(255, 215, 70);
    messageTime = 2.0f;
}

int Act3Level5Screen::stars() const
{
    if (state != WON) return 0;

    int s = 1;
    if (steps <= optimal && penalties == 0) s = 3;
    else if (steps * 4 <= 5 * optimal && penalties <= 1) s = 2;

    if (hintUsed && s > 2) s = 2;
    return s;
}

int Act3Level5Screen::score() const
{
    if (state != WON) return 0;
    int s = 1000 - 10 * alarm - 50 * penalties;
    return s < 0 ? 0 : s;
}

void Act3Level5Screen::handleInput()
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
                    handleNodeClick(hit);
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
                nextScreen = 17;

            else if (e.key.code == Keyboard::Return)
            {
                if (state == WON)
                    nextScreen = 17;
            }
        }
    }
}

void Act3Level5Screen::update(float dt)
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

void Act3Level5Screen::draw()
{
    window.clear();
    window.draw(background);

    drawText(window, font, "ACT 3 - LEVEL 5", 40, 500, 15, Color::White, false);

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
    string label;
    if (roundType == T_AUDIT) label = "AUDIT";
    else if (roundType == T_SMALL) label = "k=" + to_string(roundK) + " LOW";
    else label = "k=" + to_string(roundK) + " TOP";

    drawText(window, font,
        "ROUND " + to_string(shownRound) + "/" + to_string(TOTAL_ROUNDS) + "   " + label,
        18, 1070, 95, Color::White, true);

    if (messageTime > 0)
        drawText(window, font, message, 20, 590, 90, messageColor, true);
    else if (roundType == T_AUDIT)
        drawText(window, font, "A size badge is wrong! Click the lowest bad room",
            18, 590, 90, Color::White, true);
    else if (roundType == T_SMALL)
        drawText(window, font, "Find the " + ordinal(roundK) + " SMALLEST room: walk down from the root",
            18, 590, 90, Color::White, true);
    else
        drawText(window, font, "Steal the " + ordinal(roundK) + " MOST VALUABLE room: walk down from root",
            18, 590, 90, Color::White, true);

    drawText(window, font, "Click Room: Check/Step | H: Hint | Space: Reset | R: New | Esc: Briefing",
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
    bool found = (roundType != T_AUDIT && pathCount > 0 && step >= pathCount);

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

        if (isHint)
            circle.setOutlineColor(Color(255, 215, 70));
        else if (found && nodes[i].id == path[pathCount - 1])
            circle.setOutlineColor(Color(120, 255, 120));
        else if (isVisited(nodes[i].id))
            circle.setOutlineColor(Color(212, 110, 194));
        else if (isChecked(nodes[i].id))
            circle.setOutlineColor(Color::Green);
        else if (nodes[i].id == newId)
            circle.setOutlineColor(Color(120, 200, 255));
        else
            circle.setOutlineColor(Color(208, 221, 183));

        window.draw(circle);

        drawText(window, font, to_string(nodes[i].value),
            26, px[i], py[i], Color::White, true);
    }

    // ---- size badges (drawn after all rooms so nothing covers them) ----
    for (int i = 0; i < nodeCount; i++)
    {
        RectangleShape badge(Vector2f(52, 24));
        badge.setOrigin(26, 12);
        badge.setPosition(px[i] + 28, py[i] - 28);
        badge.setOutlineThickness(2);
        badge.setOutlineColor(Color::White);
        badge.setFillColor(Color(60, 110, 170));
        window.draw(badge);

        drawText(window, font, "s=" + to_string(nodes[i].size),
            16, px[i] + 28, py[i] - 28, Color::White, true);
    }

    // ---- hover tooltip ----
    Vector2i mp = Mouse::getPosition(window);
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

    // ---- bottom line ----
    drawText(window, font, "s = rooms in subtree",
        16, 60, 646, Color(208, 221, 183), false);
    drawText(window, font, "SEED: " + to_string(puzzleSeed),
        16, 1100, 672, Color(208, 221, 183), false);

    if (state == PLAYING && !guide.empty())
        drawText(window, font, guide, 18, 640, 640, Color(255, 215, 70), true);

    if (state == WON)
    {
        drawText(window, font,
            "BOSS DEFEATED!  STARS: " + to_string(stars()) + "  SCORE: " + to_string(score()) + "  (ENTER)",
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