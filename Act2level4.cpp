#include "Act2Level4.h"
#include "helpers.h"
#include <cmath>
using namespace sf;
using namespace std;

Act2Level4Screen::Act2Level4Screen(RenderWindow& win) : Screen(win)
{
    if (!font.loadFromFile("assets/fonts/ARIAL.ttf"))
        loadFont(font);

    if (backgroundTexture.loadFromFile("assets/pictures/Act2_level4 (4).jpg"))
    {
        background.setTexture(backgroundTexture);
        background.setScale(1280.0f / backgroundTexture.getSize().x,
            720.0f / backgroundTexture.getSize().y);
    }

    nextScreen = 10;
    nodeCount = 0;
    lo = 0;
    hi = 0;
    totalLoot = 0;
    looted = 0;
    visitedCount = 0;
    par = 0;
    capacity = 0;
    alarm = 0;
    penalties = 0;
    hintUsed = false;
    hintId = -1;
    state = LOST;

    seed = (unsigned)time(NULL) + 97;
    puzzleSeed = seed;
    flashTime = 0;
    messageTime = 0;
    animTime = 0;
    messageColor = Color::White;

    for (int i = 0; i < MAXID; i++)
    {
        idxOf[i] = -1;
        inRange[i] = false;
        visited[i] = false;
        greyed[i] = false;
    }
}

void Act2Level4Screen::rebuild()
{
    bst.computeLayout(100.0f, 1180.0f, 175.0f, 605.0f);
    nodeCount = bst.collectNodes(nodes, MAXN);

    for (int i = 0; i < MAXID; i++) idxOf[i] = -1;
    for (int i = 0; i < nodeCount; i++)
        if (nodes[i].id >= 0 && nodes[i].id < MAXID)
            idxOf[nodes[i].id] = i;
}

int Act2Level4Screen::idx(int id) const
{
    if (id < 0 || id >= MAXID) return -1;
    return idxOf[id];
}

void Act2Level4Screen::newPuzzle()
{
    puzzleSeed = seed++;
    loadPuzzle(puzzleSeed);
}

void Act2Level4Screen::loadPuzzle(unsigned s)
{
    srand(s);
    int n = 12 + rand() % 3;                     

    bst.generateRandom(s, n, 0);
    rebuild();
    srand(s + 5);
    for (int attempt = 0; attempt < 100; attempt++)
    {
        int a = nodes[rand() % nodeCount].value;
        int b = nodes[rand() % nodeCount].value;
        if (a == b) continue;
        lo = (a < b) ? a : b;
        hi = (a < b) ? b : a;

        bst.rangeQuery(lo, hi);
        int inCount = bst.getRangeInRange();
        int skipped = bst.getRangeSkippedNodes();
        if (inCount >= 3 && inCount <= 6 && skipped >= 3)
            break;
    }

    // run the final query and copy its result list
    for (int i = 0; i < MAXID; i++)
    {
        inRange[i] = false;
        visited[i] = false;
        greyed[i] = false;
    }

    RangeResult* r = bst.rangeQuery(lo, hi);
    totalLoot = 0;
    while (r != NULL)
    {
        if (r->data != NULL && r->data->id >= 0 && r->data->id < MAXID)
        {
            inRange[r->data->id] = true;
            totalLoot++;
        }
        r = r->next;
    }
    bst.clearEvents();

    par = bst.rangeOptimalSteps(lo, hi);
    capacity = BST::capacityFor(par);

    // enter the root
    BSTNode* root = bst.getRoot();
    looted = 0;
    visitedCount = 0;
    if (root != NULL && root->id >= 0 && root->id < MAXID)
    {
        visited[root->id] = true;
        visitedCount = 1;
        if (inRange[root->id]) looted++;
    }

    alarm = 1;
    penalties = 0;
    hintUsed = false;
    hintId = -1;
    flashTime = 0;
    messageTime = 0;
    message.clear();
    state = (root != NULL) ? PLAYING : LOST;
    if (state == PLAYING && looted == totalLoot) state = WON;
}

bool Act2Level4Screen::isNeeded(int parentId, int side)
{
    BSTNode* p = bst.getNodeById(parentId);
    if (p == NULL) return false;

    if (side == 0) return bst.compareAt(p, lo) == 1;    
    return bst.compareAt(p, hi) == -1;                  
}

void Act2Level4Screen::clickRoom(int i)
{
    if (state != PLAYING || i < 0) return;

    const NodeInfo n = nodes[i];
    if (n.id < 0 || n.id >= MAXID) return;

    if (visited[n.id])
    {
        message = "You are already inside that room.";
        messageColor = Color(255, 200, 90);
        messageTime = 1.0f;
        return;
    }
    if (greyed[n.id])
    {
        message = "That subtree is already skipped.";
        messageColor = Color(255, 200, 90);
        messageTime = 1.0f;
        return;
    }

    int p = n.parentId;
    if (idx(p) < 0 || !visited[p])
    {
        message = "Reach it through a room you have entered!";
        messageColor = Color(255, 200, 90);
        messageTime = 1.2f;
        return;
    }

    int side = (nodes[idx(p)].leftId == n.id) ? 0 : 1;

    if (isNeeded(p, side))
    {
        visited[n.id] = true;
        visitedCount++;
        alarm++;
        hintId = -1;

        if (inRange[n.id])
        {
            looted++;
            message = "LOOT! " + to_string(n.value) + " is in range";
            messageColor = Color(120, 255, 120);
            messageTime = 1.2f;
        }
        else
        {
            message = to_string(n.value) + " is outside the range, but the path continues";
            messageColor = Color::White;
            messageTime = 1.2f;
        }

        if (looted == totalLoot)
        {
            state = WON;
            message = "ALL LOOT COLLECTED!";
            messageColor = Color(120, 255, 120);
            messageTime = 3.0f;
        }
    }
    else
    {
        // wrong click: that subtree cannot hold any key inside the range
        penalties++;
        alarm++;
        flashTime = 0.35f;
        message = "OUT OF RANGE! Subtree skipped. Alarm +1";
        messageColor = Color(255, 110, 110);
        messageTime = 1.6f;

        int ids[MAXN];
        int cnt = bst.collectSubtreeIds(bst.getNodeById(n.id), ids, MAXN);
        for (int k = 0; k < cnt; k++)
            if (ids[k] >= 0 && ids[k] < MAXID)
                greyed[ids[k]] = true;
    }

    if (state == PLAYING && alarm >= capacity)
        state = LOST;
}

int Act2Level4Screen::findNodeAt(float x, float y) const
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

int Act2Level4Screen::prunedCount() const
{
    return nodeCount - visitedCount;
}

int Act2Level4Screen::stars() const
{
    if (state != WON) return 0;
    int s = 1;
    if (alarm <= par && penalties == 0) s = 3;
    else if (alarm * 4 <= 5 * par && penalties <= 1) s = 2;
    if (hintUsed && s > 2) s = 2;
    return s;
}

int Act2Level4Screen::score() const
{
    if (state != WON) return 0;
    int s = 1000 - 10 * alarm + 5 * prunedCount();      // pruning bonus
    return s < 0 ? 0 : s;
}

void Act2Level4Screen::onEnter()
{
    isActive = true;
    nextScreen = 10;
    newPuzzle();
}

void Act2Level4Screen::onResume()
{
    isActive = true;
    nextScreen = 10;
    flashTime = 0;
}

void Act2Level4Screen::handleInput()
{
    Event e;

    while (window.pollEvent(e))
    {
        if (e.type == Event::Closed)
            nextScreen = 99;

        if (e.type == Event::MouseButtonPressed && e.mouseButton.button == Mouse::Left)
        {
            int i = findNodeAt((float)e.mouseButton.x, (float)e.mouseButton.y);
            if (i >= 0)
                clickRoom(i);
        }

        if (e.type == Event::KeyPressed)
        {
            if (e.key.code == Keyboard::Escape)
                nextScreen = 25;

            else if (e.key.code == Keyboard::H)
            {
                if (state == PLAYING)
                {
                    // first room next to a visited room that still leads toward the range
                    for (int i = 0; i < nodeCount; i++)
                    {
                        int id = nodes[i].id;
                        if (id < 0 || id >= MAXID || visited[id] || greyed[id]) continue;

                        int p = nodes[i].parentId;
                        if (idx(p) < 0 || !visited[p]) continue;

                        int side = (nodes[idx(p)].leftId == id) ? 0 : 1;
                        if (isNeeded(p, side))
                        {
                            hintId = id;
                            hintUsed = true;
                            message = "HINT: enter the gold room";
                            messageColor = Color(255, 215, 70);
                            messageTime = 3.0f;
                            break;
                        }
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
                    nextScreen = 11;  
            }
        }
    }
}

void Act2Level4Screen::update(float dt)
{
    animTime += dt;
    if (flashTime > 0) flashTime -= dt;
    if (messageTime > 0) messageTime -= dt;
}

void Act2Level4Screen::draw()
{
    window.clear();
    window.draw(background);

    drawText(window, font, "ACT 2 - LEVEL 4", 40, 500, 15, Color::White, false);

    // ---- HUD boxes ----
    RectangleShape alarmBox(Vector2f(220, 50));
    RectangleShape rangeBox(Vector2f(420, 50));

    alarmBox.setPosition(50, 70);
    rangeBox.setPosition(810, 70);
    alarmBox.setFillColor(Color::Black);
    rangeBox.setFillColor(Color::Black);
    alarmBox.setOutlineColor(Color(180, 100, 220));
    alarmBox.setOutlineThickness(3);
    rangeBox.setOutlineColor(Color(180, 100, 220));
    rangeBox.setOutlineThickness(3);

    window.draw(alarmBox);
    window.draw(rangeBox);

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

    drawText(window, font, "RANGE: [" + to_string(lo) + ", " + to_string(hi) + "]",
        22, 1020, 95, Color(255, 215, 70), true);

    // ---- prompt / feedback ----
    if (messageTime > 0)
        drawText(window, font, message, 22, 540, 95, messageColor, true);
    else
        drawText(window, font, "Loot every room in the range. Skip subtrees that cannot hold any.",
            19, 540, 95, Color::White, true);

    drawText(window, font, "Click a room next to one you entered | H: Hint | Space: Retry | R: New Vault",
        16, 640, 125, Color(208, 221, 183), true);

    bool won = (state == WON);

    // ---- edges ----
    for (int i = 0; i < nodeCount; i++)
    {
        int p = nodes[i].parentId;
        int id = nodes[i].id;
        if (idx(p) < 0 || id < 0 || id >= MAXID) continue;

        const NodeInfo& par = nodes[idx(p)];
        Vector2f start(par.x, par.y);
        Vector2f end(nodes[i].x, nodes[i].y);
        Vector2f diff = end - start;

        RectangleShape line(Vector2f(sqrt(diff.x * diff.x + diff.y * diff.y), 5));
        line.setPosition(start);
        line.setRotation(atan2(diff.y, diff.x) * 180.0f / 3.14159f);

        bool skipped = greyed[id] || (won && !visited[id]);
        if (skipped)                         line.setFillColor(Color(100, 100, 110));
        else if (visited[p] && visited[id])  line.setFillColor(Color(212, 110, 194));
        else                                 line.setFillColor(Color(208, 221, 183));
        window.draw(line);
    }

    // ---- nodes ----
    float radius = 30.0f;
    for (int i = 0; i < nodeCount; i++)
    {
        const NodeInfo& n = nodes[i];
        int id = n.id;
        if (id < 0 || id >= MAXID) continue;

        bool isVisited = visited[id];
        bool isSkipped = greyed[id] || (won && !isVisited);
        bool isHint = (id == hintId);
        bool isLoot = isVisited && inRange[id];

        CircleShape circle(radius);
        circle.setOrigin(radius, radius);
        circle.setPosition(n.x, n.y);
        circle.setOutlineThickness(7);

        if (isSkipped)
        {
            circle.setFillColor(Color(80, 80, 90));
            circle.setOutlineColor(Color(130, 130, 140));
        }
        else
        {
            circle.setFillColor(Color(73, 103, 164));
            if (isLoot)           circle.setOutlineColor(won ? Color::Green : Color(255, 215, 70));
            else if (isHint)      circle.setOutlineColor(Color(255, 215, 70));
            else if (isVisited)   circle.setOutlineColor(Color(212, 110, 194));
            else                  circle.setOutlineColor(Color(208, 221, 183));
        }

        window.draw(circle);

        drawText(window, font, to_string(n.value), 26, n.x, n.y,
            isSkipped ? Color(150, 150, 150) : Color::White, true);

        // text labels so colour is never the only signal
        if (isLoot)
            drawText(window, font, "LOOT", 16, n.x, n.y + 48, Color(255, 215, 70), true);
        else if (isVisited)
            drawText(window, font, "OUT", 16, n.x, n.y + 48, Color(212, 110, 194), true);
        else if (isHint)
            drawText(window, font, "HINT", 16, n.x, n.y + 48, Color(255, 215, 70), true);
        else if (isSkipped)
        {
            // label only the top of each skipped subtree
            int p = n.parentId;
            if (idx(p) >= 0 && visited[p] && !visited[id])
                drawText(window, font, "SKIPPED", 16, n.x, n.y + 48, Color(160, 160, 170), true);
            else if (greyed[id] && idx(p) >= 0 && !greyed[p])
                drawText(window, font, "SKIPPED", 16, n.x, n.y + 48, Color(160, 160, 170), true);
        }
    }

    // ---- bottom status ----
    drawText(window, font, "PAR: " + to_string(par) + "   PENALTIES: " + to_string(penalties),
        20, 60, 650, Color::White, false);
    drawText(window, font, "LOOT: " + to_string(looted) + " / " + to_string(totalLoot),
        20, 60, 680, Color(255, 215, 70), false);

    if (state == WON)
    {
        drawText(window, font, "VAULT CLEARED!", 30, 640, 650, Color::Green, true);
        drawText(window, font,
            "STARS: " + to_string(stars()) + "  SCORE: " + to_string(score()) +
            "  PRUNED: " + to_string(prunedCount()) + "  (ENTER: continue)",
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