#include "Act2Level2.h"
#include "helpers.h"
#include <cmath>
using namespace sf;
using namespace std;

Act2Level2Screen::Act2Level2Screen(RenderWindow& win) : Screen(win)
{
    if (!font.loadFromFile("assets/fonts/ARIAL.ttf"))
        loadFont(font);

    if (backgroundTexture.loadFromFile("assets/pictures/Act2_level2 (2).jpg"))
    {
        background.setTexture(backgroundTexture);
        background.setScale(1280.0f / backgroundTexture.getSize().x,
            720.0f / backgroundTexture.getSize().y);
    }

    nextScreen = 8;
    nodeCount = 0;
    round = 0;
    curId = -1;
    newId = -1;
    totalOptimal = 0;
    capacity = 0;
    alarm = 0;
    penalties = 0;
    hintUsed = false;
    hintDir = -1;
    hintChildId = -1;
    state = LOST;

    seed = (unsigned)time(NULL) + 67;
    puzzleSeed = seed;
    flashTime = 0;
    messageTime = 0;
    animTime = 0;
    messageColor = Color::White;

    for (int i = 0; i < ROUNDS; i++) keys[i] = 0;
    for (int i = 0; i < MAXID; i++)
    {
        idxOf[i] = -1;
        visited[i] = false;
    }
}

void Act2Level2Screen::rebuild()
{
    bst.computeLayout(100.0f, 1180.0f, 175.0f, 605.0f);
    nodeCount = bst.collectNodes(nodes, MAXN);

    for (int i = 0; i < MAXID; i++) idxOf[i] = -1;
    for (int i = 0; i < nodeCount; i++)
        if (nodes[i].id >= 0 && nodes[i].id < MAXID)
            idxOf[nodes[i].id] = i;
}

void Act2Level2Screen::newPuzzle()
{
    puzzleSeed = seed++;
    loadPuzzle(puzzleSeed);
}

void Act2Level2Screen::loadPuzzle(unsigned s)
{
    srand(s);
    int n = 7 + rand() % 3;                       // 7 to 9 starting rooms

    bst.generateRandom(s, n, 0);
    sim.generateRandom(s, n, 0);                  // identical tree for the dry run

    // choose the loot keys and add up the optimal steps with the sim tree
    srand(s + 1);
    totalOptimal = 0;
    for (int r = 0; r < ROUNDS; r++)
    {
        int k, len;
        do
        {
            k = 1 + rand() % 99;
            len = sim.insertPathLength(k);        // -1 if the key already exists
        } while (len == -1);

        keys[r] = k;
        totalOptimal += len;
        sim.insertValue(k);
    }

    capacity = BST::capacityFor(totalOptimal);

    rebuild();

    round = 0;
    alarm = 0;
    penalties = 0;
    hintUsed = false;
    newId = -1;
    flashTime = 0;
    messageTime = 0;
    message.clear();
    state = (bst.getRoot() != NULL) ? PLAYING : LOST;

    startRound();
}

void Act2Level2Screen::startRound()
{
    for (int i = 0; i < MAXID; i++) visited[i] = false;

    BSTNode* r = bst.getRoot();
    curId = (r != NULL) ? r->id : -1;
    if (curId >= 0 && curId < MAXID) visited[curId] = true;

    alarm++;                    // entering the root costs 1 step
    hintDir = -1;
    hintChildId = -1;
}

void Act2Level2Screen::tryMove(int dir)
{
    if (state != PLAYING) return;
    if (round >= ROUNDS) return;

    BSTNode* cur = bst.getNodeById(curId);
    if (cur == NULL || curId < 0 || curId >= MAXID || idxOf[curId] < 0) return;

    int key = keys[round];

    // 1 = room is bigger than the key (go LEFT), -1 = smaller (go RIGHT)
    int c = bst.compareAt(cur, key);
    int correct = (c == 1) ? 0 : 1;

    NodeInfo info = nodes[idxOf[curId]];          // copy: nodes[] is rebuilt on insert

    if (dir != correct)
    {
        penalties++;
        alarm++;
        flashTime = 0.35f;
        messageTime = 1.4f;
        message = (correct == 0)
            ? "WRONG WAY! Loot is smaller: go LEFT. Alarm +1"
            : "WRONG WAY! Loot is bigger: go RIGHT. Alarm +1";
        messageColor = Color(255, 110, 110);
    }
    else
    {
        int childId = (correct == 0) ? info.leftId : info.rightId;

        if (childId == -1)
        {
            // empty slot: stash the loot here
            if (!bst.insertValue(key))
            {
                message = "Duplicate rejected! That room already exists.";
                messageColor = Color(255, 200, 90);
                messageTime = 2.0f;
                return;
            }

            rebuild();

            newId = -1;
            for (int i = 0; i < nodeCount; i++)
                if (nodes[i].value == key) newId = nodes[i].id;

            round++;
            hintDir = -1;
            hintChildId = -1;

            if (round >= ROUNDS)
            {
                state = WON;
                message = "ALL LOOT STASHED!";
                messageColor = Color(120, 255, 120);
                messageTime = 3.0f;
            }
            else
            {
                startRound();
                message = "STASHED! Next loot: " + to_string(keys[round]);
                messageColor = Color(120, 255, 120);
                messageTime = 2.0f;
            }
        }
        else
        {
            BSTNode* nxt = bst.childToward(cur, key);
            if (nxt == NULL) return;

            curId = nxt->id;
            if (curId >= 0 && curId < MAXID) visited[curId] = true;
            alarm++;
            hintDir = -1;
            hintChildId = -1;
            newId = -1;
            messageTime = 0;
        }
    }

    if (state == PLAYING && alarm >= capacity)
        state = LOST;
}

int Act2Level2Screen::findNodeAt(float x, float y) const
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

// where the hollow "empty slot" circle of the current room is drawn
void Act2Level2Screen::slotPos(int side, float& x, float& y) const
{
    const NodeInfo& cur = nodes[idxOf[curId]];
    x = cur.x + (side == 0 ? -55.0f : 55.0f);
    y = cur.y + 70.0f;
}

int Act2Level2Screen::stars() const
{
    if (state != WON) return 0;
    int s = 1;
    if (alarm <= totalOptimal && penalties == 0) s = 3;
    else if (alarm * 4 <= 5 * totalOptimal && penalties <= 1) s = 2;
    if (hintUsed && s > 2) s = 2;
    return s;
}

int Act2Level2Screen::score() const
{
    if (state != WON) return 0;
    int s = 1000 - 10 * alarm;
    return s < 0 ? 0 : s;
}

void Act2Level2Screen::onEnter()
{
    isActive = true;
    nextScreen = 8;
    newPuzzle();
}

void Act2Level2Screen::onResume()
{
    isActive = true;
    nextScreen = 8;
    flashTime = 0;
}

void Act2Level2Screen::handleInput()
{
    Event e;

    while (window.pollEvent(e))
    {
        if (e.type == Event::Closed)
            nextScreen = 99;

        if (e.type == Event::MouseButtonPressed && e.mouseButton.button == Mouse::Left)
        {
            if (state == PLAYING && curId >= 0 && curId < MAXID && idxOf[curId] >= 0)
            {
                float mx = (float)e.mouseButton.x;
                float my = (float)e.mouseButton.y;
                const NodeInfo cur = nodes[idxOf[curId]];
                bool handled = false;

                // clicking an empty slot
                for (int side = 0; side < 2 && !handled; side++)
                {
                    int childId = (side == 0) ? cur.leftId : cur.rightId;
                    if (childId != -1) continue;

                    float sx, sy;
                    slotPos(side, sx, sy);
                    float dx = mx - sx;
                    float dy = my - sy;
                    if (dx * dx + dy * dy <= 22.0f * 22.0f)
                    {
                        tryMove(side);
                        handled = true;
                    }
                }

                // clicking a child room
                if (!handled)
                {
                    int i = findNodeAt(mx, my);
                    if (i >= 0)
                    {
                        if (nodes[i].id == cur.leftId)       tryMove(0);
                        else if (nodes[i].id == cur.rightId) tryMove(1);
                        else
                        {
                            message = "Click a child room or an empty slot!";
                            messageColor = Color(255, 200, 90);
                            messageTime = 1.0f;
                        }
                    }
                }
            }
        }

        if (e.type == Event::KeyPressed)
        {
            if (e.key.code == Keyboard::Escape)
                nextScreen = 25;

            else if (e.key.code == Keyboard::Left || e.key.code == Keyboard::A)
                tryMove(0);

            else if (e.key.code == Keyboard::Right || e.key.code == Keyboard::D)
                tryMove(1);

            else if (e.key.code == Keyboard::H)
            {
                if (state == PLAYING && round < ROUNDS &&
                    curId >= 0 && curId < MAXID && idxOf[curId] >= 0)
                {
                    BSTNode* cur = bst.getNodeById(curId);
                    if (cur != NULL)
                    {
                        const NodeInfo& info = nodes[idxOf[curId]];
                        int c = bst.compareAt(cur, keys[round]);
                        hintDir = (c == 1) ? 0 : 1;
                        hintChildId = (hintDir == 0) ? info.leftId : info.rightId;
                        hintUsed = true;

                        string side = (hintDir == 0) ? "LEFT" : "RIGHT";
                        message = (hintChildId == -1)
                            ? "HINT: go " + side + " - the slot is empty, stash here"
                            : "HINT: go " + side;
                        messageColor = Color(255, 215, 70);
                        messageTime = 3.0f;
                    }
                }
            }

            else if (e.key.code == Keyboard::Space)
                loadPuzzle(puzzleSeed);         // retry the same vault

            else if (e.key.code == Keyboard::R)
                newPuzzle();

            else if (e.key.code == Keyboard::Return)
            {
                if (state == WON)
                    nextScreen = 9;     // TODO: change to 9 once Act 2 Level 3 exists
            }
        }
    }
}

void Act2Level2Screen::update(float dt)
{
    animTime += dt;
    if (flashTime > 0) flashTime -= dt;
    if (messageTime > 0) messageTime -= dt;
}

void Act2Level2Screen::draw()
{
    window.clear();
    window.draw(background);

    drawText(window, font, "ACT 2 - LEVEL 2", 40, 500, 15, Color::White, false);

    // ---- HUD boxes ----
    RectangleShape alarmBox(Vector2f(220, 50));
    RectangleShape lootBox(Vector2f(420, 50));

    alarmBox.setPosition(50, 70);
    lootBox.setPosition(810, 70);
    alarmBox.setFillColor(Color::Black);
    lootBox.setFillColor(Color::Black);
    alarmBox.setOutlineColor(Color(180, 100, 220));
    alarmBox.setOutlineThickness(3);
    lootBox.setOutlineColor(Color(180, 100, 220));
    lootBox.setOutlineThickness(3);

    window.draw(alarmBox);
    window.draw(lootBox);

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

    string lootStr;
    if (round < ROUNDS)
        lootStr = "STASH: " + to_string(keys[round]) + "  (" + to_string(round + 1) + "/" + to_string(ROUNDS) + ")";
    else
        lootStr = "STASHED: " + to_string(ROUNDS) + " / " + to_string(ROUNDS);
    drawText(window, font, lootStr, 22, 1020, 95, Color(255, 215, 70), true);

    // ---- prompt / feedback ----
    if (messageTime > 0)
        drawText(window, font, message, 22, 540, 95, messageColor, true);
    else
        drawText(window, font, "Step toward an empty side to stash the loot",
            19, 540, 95, Color::White, true);

    drawText(window, font, "Left/Right or Click: Move | H: Hint | Space: Retry | R: New Vault",
        16, 640, 125, Color(208, 221, 183), true);

    // ---- edges ----
    for (int i = 0; i < nodeCount; i++)
    {
        int p = nodes[i].parentId;
        if (p < 0 || p >= MAXID || idxOf[p] < 0) continue;

        const NodeInfo& par = nodes[idxOf[p]];
        Vector2f start(par.x, par.y);
        Vector2f end(nodes[i].x, nodes[i].y);
        Vector2f diff = end - start;

        RectangleShape line(Vector2f(sqrt(diff.x * diff.x + diff.y * diff.y), 5));
        line.setPosition(start);
        line.setRotation(atan2(diff.y, diff.x) * 180.0f / 3.14159f);

        bool onPath = visited[p] && nodes[i].id >= 0 && nodes[i].id < MAXID && visited[nodes[i].id];
        line.setFillColor(onPath ? Color(212, 110, 194) : Color(208, 221, 183));
        window.draw(line);
    }

    // ---- empty slots of the current room ----
    if (state == PLAYING && curId >= 0 && curId < MAXID && idxOf[curId] >= 0)
    {
        const NodeInfo& cur = nodes[idxOf[curId]];
        for (int side = 0; side < 2; side++)
        {
            int childId = (side == 0) ? cur.leftId : cur.rightId;
            if (childId != -1) continue;

            float sx, sy;
            slotPos(side, sx, sy);

            CircleShape slot(16);
            slot.setOrigin(16, 16);
            slot.setPosition(sx, sy);
            slot.setFillColor(Color(0, 0, 0, 120));
            slot.setOutlineThickness(3);
            slot.setOutlineColor(hintDir == side ? Color(255, 215, 70) : Color(208, 221, 183));
            window.draw(slot);

            drawText(window, font, side == 0 ? "L" : "R", 16, sx, sy, Color::White, true);
        }
    }

    // ---- nodes ----
    float radius = 30.0f;
    for (int i = 0; i < nodeCount; i++)
    {
        const NodeInfo& n = nodes[i];
        bool isCur = (n.id == curId);
        bool isHint = (n.id == hintChildId);
        bool isNew = (n.id == newId);
        bool isVisited = (n.id >= 0 && n.id < MAXID && visited[n.id]);

        CircleShape circle(radius);
        circle.setOrigin(radius, radius);
        circle.setPosition(n.x, n.y);
        circle.setFillColor(Color(73, 103, 164));
        circle.setOutlineThickness(7);

        if (isNew)                       circle.setOutlineColor(Color(255, 215, 70));
        else if (isHint)                 circle.setOutlineColor(Color(255, 215, 70));
        else if (isVisited && state == PLAYING) circle.setOutlineColor(Color(212, 110, 194));
        else                             circle.setOutlineColor(Color(208, 221, 183));

        if (state == WON && isNew)       circle.setOutlineColor(Color::Green);

        window.draw(circle);

        if (isCur && state == PLAYING)
        {
            int alpha = 170 + (int)(50 * sin(animTime * 6));

            CircleShape halo(radius + 14);
            halo.setOrigin(radius + 14, radius + 14);
            halo.setPosition(n.x, n.y);
            halo.setFillColor(Color(150, 60, 220, (Uint8)(alpha / 3)));
            window.draw(halo);

            CircleShape shade(radius);
            shade.setOrigin(radius, radius);
            shade.setPosition(n.x, n.y);
            shade.setFillColor(Color(160, 70, 230, (Uint8)alpha));
            shade.setOutlineThickness(7);
            shade.setOutlineColor(Color(190, 110, 255, (Uint8)alpha));
            window.draw(shade);
        }

        drawText(window, font, to_string(n.value), 26, n.x, n.y, Color::White, true);

        // text labels so colour is never the only signal
        if (isCur && state == PLAYING)
            drawText(window, font, "HERE", 16, n.x, n.y - 46, Color(255, 200, 90), true);
        else if (isNew)
            drawText(window, font, "NEW", 16, n.x, n.y + 48, Color(255, 215, 70), true);
        else if (isHint)
            drawText(window, font, "HINT", 16, n.x, n.y + 48, Color(255, 215, 70), true);
    }

    // ---- bottom status ----
    drawText(window, font, "PAR: " + to_string(totalOptimal) + "   PENALTIES: " + to_string(penalties),
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

    if (flashTime > 0)
    {
        RectangleShape redScreen(Vector2f(1280, 720));
        redScreen.setFillColor(Color(255, 0, 0, 90));
        window.draw(redScreen);
    }
}