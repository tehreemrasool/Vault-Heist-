#include "Act2Level1.h"
#include "helpers.h"
#include <cmath>
using namespace sf;
using namespace std;

Act2Level1Screen::Act2Level1Screen(RenderWindow& win) : Screen(win)
{
    if (!font.loadFromFile("assets/fonts/ARIAL.ttf"))
        loadFont(font);

    if (backgroundTexture.loadFromFile("assets/pictures/Act2_level1 (1).jpg"))
    {
        background.setTexture(backgroundTexture);
        background.setScale(1280.0f / backgroundTexture.getSize().x,
            720.0f / backgroundTexture.getSize().y);
    }

    nextScreen = 7;
    nodeCount = 0;
    curId = -1;
    targetValue = 0;
    optimal = 0;
    capacity = 0;
    alarm = 0;
    penalties = 0;
    hintUsed = false;
    hintChildId = -1;
    state = LOST;

    seed = (unsigned)time(NULL) + 53;
    puzzleSeed = seed;
    flashTime = 0;
    messageTime = 0;
    animTime = 0;
    messageColor = Color::White;

    for (int i = 0; i < MAXID; i++)
    {
        idxOf[i] = -1;
        visited[i] = false;
    }
}

void Act2Level1Screen::newPuzzle()
{
    puzzleSeed = seed++;
    srand(puzzleSeed);
    int n = 9 + rand() % 5;                       // 9 to 13 rooms

    bst.generateRandom(puzzleSeed, n, 0);
    bst.computeLayout(100.0f, 1180.0f, 175.0f, 605.0f);
    nodeCount = bst.collectNodes(nodes, MAXN);

    for (int i = 0; i < MAXID; i++) idxOf[i] = -1;
    for (int i = 0; i < nodeCount; i++)
        if (nodes[i].id >= 0 && nodes[i].id < MAXID)
            idxOf[nodes[i].id] = i;

    // pick a target that is not near the root, so the search takes some steps
    int cand[MAXN];
    int c = 0;
    for (int i = 0; i < nodeCount; i++)
        if (nodes[i].depth >= 3) cand[c++] = i;
    if (c == 0)
        for (int i = 0; i < nodeCount; i++)
            if (nodes[i].depth >= 2) cand[c++] = i;
    if (c == 0) cand[c++] = 0;

    srand(puzzleSeed + 1);
    int pick = cand[rand() % c];
    targetValue = nodes[pick].value;

    optimal = bst.searchPathLength(targetValue);
    capacity = BST::capacityFor(optimal);

    restartRun();
}

void Act2Level1Screen::restartRun()
{
    for (int i = 0; i < MAXID; i++) visited[i] = false;

    BSTNode* r = bst.getRoot();
    curId = (r != NULL) ? r->id : -1;
    if (curId >= 0 && curId < MAXID) visited[curId] = true;

    alarm = 1;                  // entering the root costs 1 step
    penalties = 0;
    hintUsed = false;
    hintChildId = -1;
    state = (r != NULL) ? PLAYING : LOST;

    flashTime = 0;
    messageTime = 0;
    message.clear();
}

void Act2Level1Screen::tryMove(int dir)
{
    if (state != PLAYING) return;

    BSTNode* cur = bst.getNodeById(curId);
    if (cur == NULL) return;

    // 1 = room is bigger than the key (go LEFT), -1 = smaller (go RIGHT)
    int c = bst.compareAt(cur, targetValue);
    int correct = (c == 1) ? 0 : 1;

    if (dir != correct)
    {
        penalties++;
        alarm++;
        flashTime = 0.35f;
        messageTime = 1.4f;
        message = (correct == 0)
            ? "WRONG WAY! The room is bigger: go LEFT. Alarm +1"
            : "WRONG WAY! The room is smaller: go RIGHT. Alarm +1";
        messageColor = Color(255, 110, 110);
    }
    else
    {
        BSTNode* nxt = bst.childToward(cur, targetValue);
        if (nxt == NULL) return;

        curId = nxt->id;
        if (curId >= 0 && curId < MAXID) visited[curId] = true;
        alarm++;
        hintChildId = -1;
        messageTime = 0;

        if (bst.compareAt(nxt, targetValue) == 0)
        {
            state = WON;
            message = "TREASURE FOUND!";
            messageColor = Color(120, 255, 120);
            messageTime = 3.0f;
        }
    }

    if (state == PLAYING && alarm >= capacity)
        state = LOST;
}

int Act2Level1Screen::findNodeAt(float x, float y) const
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

int Act2Level1Screen::stars() const
{
    if (state != WON) return 0;
    int s = 1;
    if (alarm <= optimal && penalties == 0) s = 3;
    else if (alarm * 4 <= 5 * optimal && penalties <= 1) s = 2;
    if (hintUsed && s > 2) s = 2;           // hint caps stars at 2
    return s;
}

int Act2Level1Screen::score() const
{
    if (state != WON) return 0;
    int s = 1000 - 10 * alarm;
    return s < 0 ? 0 : s;
}

void Act2Level1Screen::onEnter()
{
    isActive = true;
    nextScreen = 7;
    newPuzzle();
}

void Act2Level1Screen::onResume()
{
    isActive = true;
    nextScreen = 7;
    flashTime = 0;
}

void Act2Level1Screen::handleInput()
{
    Event e;

    while (window.pollEvent(e))
    {
        if (e.type == Event::Closed)
            nextScreen = 99;

        if (e.type == Event::MouseButtonPressed && e.mouseButton.button == Mouse::Left)
        {
            if (state == PLAYING)
            {
                int i = findNodeAt((float)e.mouseButton.x, (float)e.mouseButton.y);
                if (i >= 0 && curId >= 0 && curId < MAXID && idxOf[curId] >= 0)
                {
                    const NodeInfo& cur = nodes[idxOf[curId]];
                    if (nodes[i].id == cur.leftId)       tryMove(0);
                    else if (nodes[i].id == cur.rightId) tryMove(1);
                    else
                    {
                        message = "Click a child of the glowing room!";
                        messageColor = Color(255, 200, 90);
                        messageTime = 1.0f;
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
                if (state == PLAYING && curId >= 0 && curId < MAXID && idxOf[curId] >= 0)
                {
                    BSTNode* cur = bst.getNodeById(curId);
                    BSTNode* nx = (cur != NULL) ? bst.childToward(cur, targetValue) : NULL;
                    if (nx != NULL)
                    {
                        hintUsed = true;
                        hintChildId = nx->id;
                        bool left = (nx->id == nodes[idxOf[curId]].leftId);
                        message = left ? "HINT: go LEFT" : "HINT: go RIGHT";
                        messageColor = Color(255, 215, 70);
                        messageTime = 3.0f;
                    }
                }
            }

            else if (e.key.code == Keyboard::Space)
                restartRun();

            else if (e.key.code == Keyboard::R)
                newPuzzle();

            else if (e.key.code == Keyboard::Return)
            {
                if (state == WON)
                    nextScreen = 1;     // TODO: change to 8 once Act 2 Level 2 exists
            }
        }
    }
}

void Act2Level1Screen::update(float dt)
{
    animTime += dt;
    if (flashTime > 0) flashTime -= dt;
    if (messageTime > 0) messageTime -= dt;
}

void Act2Level1Screen::draw()
{
    window.clear();
    window.draw(background);

    drawText(window, font, "ACT 2 - LEVEL 1", 40, 500, 15, Color::White, false);

    // ---- HUD boxes ----
    RectangleShape alarmBox(Vector2f(220, 50));
    RectangleShape targetBox(Vector2f(420, 50));

    alarmBox.setPosition(50, 70);
    targetBox.setPosition(810, 70);
    alarmBox.setFillColor(Color::Black);
    targetBox.setFillColor(Color::Black);
    alarmBox.setOutlineColor(Color(180, 100, 220));
    alarmBox.setOutlineThickness(3);
    targetBox.setOutlineColor(Color(180, 100, 220));
    targetBox.setOutlineThickness(3);

    window.draw(alarmBox);
    window.draw(targetBox);

    // alarm meter fill (inside the alarm box)
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

    drawText(window, font, "TREASURE: " + to_string(targetValue),
        22, 1020, 95, Color(255, 215, 70), true);

    // ---- prompt / feedback ----
    if (messageTime > 0)
        drawText(window, font, message, 22, 540, 95, messageColor, true);
    else
        drawText(window, font, "Smaller than the room: go LEFT. Bigger: go RIGHT.",
            19, 540, 95, Color::White, true);

    drawText(window, font, "Left/Right or Click Child: Move | H: Hint | Space: Retry | R: New Vault",
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

    // ---- nodes ----
    float radius = 30.0f;
    for (int i = 0; i < nodeCount; i++)
    {
        const NodeInfo& n = nodes[i];
        bool isCur = (n.id == curId);
        bool isHint = (n.id == hintChildId);
        bool isVisited = (n.id >= 0 && n.id < MAXID && visited[n.id]);

        CircleShape circle(radius);
        circle.setOrigin(radius, radius);
        circle.setPosition(n.x, n.y);
        circle.setFillColor(Color(73, 103, 164));
        circle.setOutlineThickness(7);

        if (state == WON && isCur)       circle.setOutlineColor(Color::Green);
        else if (isHint)                 circle.setOutlineColor(Color(255, 215, 70));
        else if (isVisited)              circle.setOutlineColor(Color(212, 110, 194));
        else                             circle.setOutlineColor(Color(208, 221, 183));

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
        if (isCur)
            drawText(window, font, (state == WON) ? "FOUND" : "HERE",
                16, n.x, n.y + 48, Color(255, 200, 90), true);
        else if (isHint)
            drawText(window, font, "HINT", 16, n.x, n.y + 48, Color(255, 215, 70), true);
    }

    // ---- bottom status ----
    drawText(window, font, "PAR: " + to_string(optimal) + "   PENALTIES: " + to_string(penalties),
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