#include "Act2Level5.h"
#include "helpers.h"
#include <cmath>
using namespace sf;
using namespace std;


static void balancedOrder(const int sorted[], int n, int out[])
{
    int ql[80], qr[80];
    int head = 0, tail = 0, k = 0;

    ql[tail] = 0; qr[tail] = n - 1; tail++;

    while (head < tail)
    {
        int l = ql[head];
        int r = qr[head];
        head++;
        if (l > r) continue;

        int m = (l + r) / 2;
        out[k++] = sorted[m];

        ql[tail] = l;     qr[tail] = m - 1; tail++;
        ql[tail] = m + 1; qr[tail] = r;     tail++;
    }
}

Act2Level5Screen::Act2Level5Screen(RenderWindow& win) : Screen(win)
{
    if (!font.loadFromFile("assets/fonts/ARIAL.ttf"))
        loadFont(font);

    storyFont.loadFromFile("assets/fonts/MedievalSharp.ttf");

    if (backgroundTexture.loadFromFile("assets/pictures/Act2_level5.jpeg"))
    {
        background.setTexture(backgroundTexture);
        background.setScale(1280.0f / backgroundTexture.getSize().x,
            720.0f / backgroundTexture.getSize().y);
    }

    nextScreen = 11;
    nodeCount = 0;
    simCount = 0;
    simHeight = 0;
    phase = PH_DELIVER;
    shown = 0;
    deliverT = 0;
    round = 0;
    curId = -1;
    targetValue = 0;
    par = 0;
    totalPar = 0;
    corridorHeight = 0;
    capacity = 0;
    alarm = 0;
    totalAlarm = 0;
    penalties = 0;
    hintUsed = false;
    hintChildId = -1;
    state = LOST;

    seed = (unsigned)time(NULL) + 113;
    puzzleSeed = seed;
    flashTime = 0;
    messageTime = 0;
    animTime = 0;
    messageColor = Color::White;

    for (int i = 0; i < ROUNDS; i++) { targets[i] = 0; pars[i] = 0; }
    for (int i = 0; i < MAXID; i++)
    {
        idxOf[i] = -1;
        simIdxOf[i] = -1;
        visited[i] = false;
    }
}

void Act2Level5Screen::rebuild()
{
    bst.computeLayout(100.0f, 1180.0f, 175.0f, 605.0f);
    nodeCount = bst.collectNodes(nodes, MAXN);

    for (int i = 0; i < MAXID; i++) idxOf[i] = -1;
    for (int i = 0; i < nodeCount; i++)
        if (nodes[i].id >= 0 && nodes[i].id < MAXID)
            idxOf[nodes[i].id] = i;
}

int Act2Level5Screen::idx(int id) const
{
    if (id < 0 || id >= MAXID) return -1;
    return idxOf[id];
}

void Act2Level5Screen::newPuzzle()
{
    puzzleSeed = seed++;
    loadPuzzle(puzzleSeed);
}

void Act2Level5Screen::loadPuzzle(unsigned s)
{
    srand(s);
    int n = 10 + rand() % 3;                   

    bst.reset();
    bst.generateLazyArchitect(n);                
    rebuild();
    corridorHeight = bst.height(bst.getRoot());
    totalPar = 0;
    for (int r = 0; r < ROUNDS; r++)
    {
        int want = nodeCount - 3 * r;
        int pick = nodeCount - 1;
        for (int i = 0; i < nodeCount; i++)
            if (nodes[i].depth == want) { pick = i; break; }

        targets[r] = nodes[pick].value;
        pars[r] = bst.searchPathLength(targets[r]);
        totalPar += pars[r];
    }

    round = 0;
    par = pars[0];
    capacity = BST::capacityFor(pars[0]);
    alarm = 0;
    totalAlarm = 0;
    penalties = 0;
    hintUsed = false;
    hintChildId = -1;
    targetValue = targets[0];
    curId = -1;
    for (int i = 0; i < MAXID; i++) visited[i] = false;

    phase = PH_DELIVER;
    shown = 0;
    deliverT = 0;
    flashTime = 0;
    messageTime = 0;
    message.clear();
    state = (bst.getRoot() != NULL) ? PLAYING : LOST;
}

void Act2Level5Screen::finishDelivery()
{
    phase = PH_SEARCH;
    shown = nodeCount;
    startRound();
}

void Act2Level5Screen::startRound()
{
    for (int i = 0; i < MAXID; i++) visited[i] = false;

    BSTNode* r = bst.getRoot();
    curId = (r != NULL) ? r->id : -1;
    if (curId >= 0 && curId < MAXID) visited[curId] = true;

    targetValue = targets[round];
    par = pars[round];

    int cap = BST::capacityFor(par) - SHRINK * round;
    if (cap < par + 1) cap = par + 1;
    capacity = cap;

    alarm = 1;                 
    hintChildId = -1;

    message = "ROUND " + to_string(round + 1) + "/" + to_string(ROUNDS) +
        ": steal " + to_string(targetValue);
    messageColor = Color(255, 215, 70);
    messageTime = 2.5f;
}

void Act2Level5Screen::completeRound()
{
    totalAlarm += alarm;
    round++;

    if (round >= ROUNDS)
    {
        state = WON;
        phase = PH_CUTSCENE;
        buildCutscene();
    }
    else
    {
        startRound();
        message = "FOUND! Round " + to_string(round + 1) + ": less alarm to spare";
        messageColor = Color(120, 255, 120);
        messageTime = 2.5f;
    }
}

void Act2Level5Screen::tryMove(int dir)
{
    if (state != PLAYING || phase != PH_SEARCH) return;

    BSTNode* cur = bst.getNodeById(curId);
    if (cur == NULL || idx(curId) < 0) return;

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
            completeRound();
            return;
        }
    }

    if (state == PLAYING && alarm >= capacity)
        state = LOST;
}

void Act2Level5Screen::buildCutscene()
{
    int vals[MAXN];
    for (int i = 0; i < nodeCount; i++) vals[i] = nodes[i].value;
    for (int i = 1; i < nodeCount; i++)
    {
        int v = vals[i];
        int j = i - 1;
        while (j >= 0 && vals[j] > v) { vals[j + 1] = vals[j]; j--; }
        vals[j + 1] = v;
    }

    int order[MAXN];
    balancedOrder(vals, nodeCount, order);

    sim.reset();
    sim.buildFromKeys(order, nodeCount);
    sim.computeLayout(740.0f, 1190.0f, 270.0f, 500.0f);
    simCount = sim.collectNodes(simNodes, MAXN);

    for (int i = 0; i < MAXID; i++) simIdxOf[i] = -1;
    for (int i = 0; i < simCount; i++)
        if (simNodes[i].id >= 0 && simNodes[i].id < MAXID)
            simIdxOf[simNodes[i].id] = i;

    simHeight = sim.height(sim.getRoot());
}

int Act2Level5Screen::findNodeAt(float x, float y) const
{
    for (int i = 0; i < nodeCount; i++)
    {
        if (nodes[i].depth > shown) continue;
        float dx = x - nodes[i].x;
        float dy = y - nodes[i].y;
        if (dx * dx + dy * dy <= 30.0f * 30.0f)
            return i;
    }
    return -1;
}

int Act2Level5Screen::stars() const
{
    if (state != WON) return 0;
    int s = 1;
    if (totalAlarm <= totalPar && penalties == 0) s = 3;
    else if (totalAlarm * 4 <= 5 * totalPar && penalties <= 1) s = 2;
    if (hintUsed && s > 2) s = 2;
    return s;
}

int Act2Level5Screen::score() const
{
    if (state != WON) return 0;
    int s = 1000 - 10 * totalAlarm;
    return s < 0 ? 0 : s;
}

void Act2Level5Screen::onEnter()
{
    isActive = true;
    nextScreen = 11;
    newPuzzle();
}

void Act2Level5Screen::onResume()
{
    isActive = true;
    nextScreen = 11;
    flashTime = 0;
}

void Act2Level5Screen::handleInput()
{
    Event e;

    while (window.pollEvent(e))
    {
        if (e.type == Event::Closed)
            nextScreen = 99;

        if (e.type == Event::MouseButtonPressed && e.mouseButton.button == Mouse::Left)
        {
            if (state == PLAYING && phase == PH_SEARCH && idx(curId) >= 0)
            {
                int i = findNodeAt((float)e.mouseButton.x, (float)e.mouseButton.y);
                if (i >= 0)
                {
                    const NodeInfo cur = nodes[idx(curId)];
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
                if (state == PLAYING && phase == PH_SEARCH && idx(curId) >= 0)
                {
                    BSTNode* cur = bst.getNodeById(curId);
                    BSTNode* nx = (cur != NULL) ? bst.childToward(cur, targetValue) : NULL;
                    if (nx != NULL)
                    {
                        hintUsed = true;
                        hintChildId = nx->id;
                        bool left = (nx->id == nodes[idx(curId)].leftId);
                        message = left ? "HINT: go LEFT" : "HINT: go RIGHT";
                        messageColor = Color(255, 215, 70);
                        messageTime = 3.0f;
                    }
                }
            }

            else if (e.key.code == Keyboard::Space)
                loadPuzzle(puzzleSeed);         

            else if (e.key.code == Keyboard::R)
                newPuzzle();
            else if (e.key.code == Keyboard::N)
                nextScreen = 17;
            else if (e.key.code == Keyboard::N)
                nextScreen = 12;

            else if (e.key.code == Keyboard::Return)
            {
                if (phase == PH_DELIVER && state == PLAYING)
                {
                    shown = nodeCount;         
                    deliverT = 0.8f;
                }
                else if (phase == PH_CUTSCENE)
                    nextScreen = 17;             
            }
        }
    }
}

void Act2Level5Screen::update(float dt)
{
    animTime += dt;
    if (flashTime > 0) flashTime -= dt;
    if (messageTime > 0) messageTime -= dt;

    if (phase == PH_DELIVER && state == PLAYING)
    {
        deliverT += dt;
        if (shown < nodeCount)
        {
            if (deliverT >= 0.4f)
            {
                deliverT = 0;
                shown++;
            }
        }
        else if (deliverT >= 0.8f)
        {
            finishDelivery();
        }
    }
}

void Act2Level5Screen::drawCutscene()
{
    drawText(window, font, "ACT 2 - LEVEL 5", 40, 500, 15, Color::White, false);
    drawText(window, font, "BOSS: THE LAZY ARCHITECT", 26, 640, 95, Color(255, 215, 70), true);

    RectangleShape panel(Vector2f(1200, 470));
    panel.setPosition(40, 150);
    panel.setFillColor(Color(0, 0, 0, 170));
    panel.setOutlineColor(Color(86, 170, 113));
    panel.setOutlineThickness(3);
    window.draw(panel);

    string story =
        "The Lazy Architect built each room\n"
        "in the order the deliveries arrived.\n"
        "Sorted keys became a corridor:\n" +
        to_string(nodeCount) + " rooms, height " + to_string(corridorHeight) + ".\n\n"
        "Your searches walked it room by room:\n" +
        to_string(totalAlarm) + " alarm steps in all.\n\n"
        "The same rooms, balanced, stand only\n" +
        to_string(simHeight) + " high. A search costs at most " + to_string(simHeight) + ".\n\n"
        "Balance is not a luxury. It is the lock.";

    drawText(window, storyFont, story, 22, 70, 175, Color::White, false);

    // balanced version of the same rooms
    drawText(window, font, "SAME ROOMS, BALANCED  (height " + to_string(simHeight) + ")",
        20, 965, 195, Color(208, 221, 183), true);
    drawText(window, font, "CORRIDOR: height " + to_string(corridorHeight) + "   BALANCED: height " + to_string(simHeight),
        16, 965, 225, Color(255, 215, 70), true);

    for (int i = 0; i < simCount; i++)
    {
        int p = simNodes[i].parentId;
        if (p < 0 || p >= MAXID || simIdxOf[p] < 0) continue;

        const NodeInfo& par = simNodes[simIdxOf[p]];
        Vector2f start(par.x, par.y);
        Vector2f end(simNodes[i].x, simNodes[i].y);
        Vector2f diff = end - start;

        RectangleShape line(Vector2f(sqrt(diff.x * diff.x + diff.y * diff.y), 3));
        line.setPosition(start);
        line.setRotation(atan2(diff.y, diff.x) * 180.0f / 3.14159f);
        line.setFillColor(Color(208, 221, 183));
        window.draw(line);
    }

    for (int i = 0; i < simCount; i++)
    {
        CircleShape circle(20);
        circle.setOrigin(20, 20);
        circle.setPosition(simNodes[i].x, simNodes[i].y);
        circle.setFillColor(Color(73, 103, 164));
        circle.setOutlineThickness(4);
        circle.setOutlineColor(Color(208, 221, 183));
        window.draw(circle);

        drawText(window, font, to_string(simNodes[i].value), 16,
            simNodes[i].x, simNodes[i].y, Color::White, true);
    }

    drawText(window, font, "VAULT CLEARED!", 30, 640, 650, Color::Green, true);
    drawText(window, font,
        "STARS: " + to_string(stars()) + "   SCORE: " + to_string(score()) + "   (ENTER: continue)",
        20, 640, 685, Color(208, 221, 183), true);
}

void Act2Level5Screen::draw()
{
    window.clear();
    window.draw(background);

    if (phase == PH_CUTSCENE)
    {
        drawCutscene();
        return;
    }

    drawText(window, font, "ACT 2 - LEVEL 5", 40, 500, 15, Color::White, false);

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

    float fraction = (capacity > 0) ? (float)alarm / (float)capacity : 0.0f;
    if (fraction > 1.0f) fraction = 1.0f;
    RectangleShape bar(Vector2f(214 * fraction, 44));
    bar.setPosition(53, 73);
    if (fraction < 0.5f)       bar.setFillColor(Color(60, 160, 80, 190));
    else if (fraction < 0.8f)  bar.setFillColor(Color(200, 170, 40, 190));
    else                       bar.setFillColor(Color(200, 60, 60, 190));
    window.draw(bar);

    drawText(window, font, "ALARM: " + to_string(alarm) + " / " + to_string(capacity),
        18, 70, 82, Color::White, false);

    if (phase == PH_DELIVER)
    {
        string key = (shown > 0) ? to_string(nodes[shown - 1].value) : "-";
        for (int i = 0; i < nodeCount; i++)
            if (nodes[i].depth == shown) key = to_string(nodes[i].value);
        drawText(window, font, "DELIVERY: key " + key,
            22, 1020, 95, Color(255, 215, 70), true);
    }
    else
    {
        drawText(window, font,
            "TREASURE: " + to_string(targetValue) + "  (" + to_string(round + 1) + "/" + to_string(ROUNDS) + ")",
            22, 1020, 95, Color(255, 215, 70), true);
    }

    if (phase == PH_DELIVER)
        drawText(window, font, "Deliveries arrive in sorted order... the vault is a corridor!",
            19, 540, 95, Color::White, true);
    else if (messageTime > 0)
        drawText(window, font, message, 22, 540, 95, messageColor, true);
    else
        drawText(window, font, "Find the treasure before the shrinking alarm fills",
            19, 540, 95, Color::White, true);

    if (phase == PH_DELIVER)
        drawText(window, font, "Enter: Skip delivery",
            16, 640, 125, Color(208, 221, 183), true);
    else
        drawText(window, font, "Left/Right or Click Child: Move | H: Hint | Space: Retry | R: New Vault",
            16, 640, 125, Color(208, 221, 183), true);

    for (int i = 0; i < nodeCount; i++)
    {
        if (nodes[i].depth > shown) continue;
        int p = nodes[i].parentId;
        if (idx(p) < 0) continue;

        const NodeInfo& par = nodes[idx(p)];
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

    float radius = 30.0f;
    for (int i = 0; i < nodeCount; i++)
    {
        const NodeInfo& n = nodes[i];
        if (n.depth > shown) continue;

        bool isCur = (n.id == curId) && phase == PH_SEARCH;
        bool isHint = (n.id == hintChildId);
        bool isNewDelivery = (phase == PH_DELIVER && n.depth == shown);
        bool isVisited = (n.id >= 0 && n.id < MAXID && visited[n.id]);

        CircleShape circle(radius);
        circle.setOrigin(radius, radius);
        circle.setPosition(n.x, n.y);
        circle.setFillColor(Color(73, 103, 164));
        circle.setOutlineThickness(7);

        if (isNewDelivery || isHint)  circle.setOutlineColor(Color(255, 215, 70));
        else if (isVisited && phase == PH_SEARCH) circle.setOutlineColor(Color(212, 110, 194));
        else                          circle.setOutlineColor(Color(208, 221, 183));

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

        if (isCur)
            drawText(window, font, "HERE", 16, n.x, n.y + 48, Color(255, 200, 90), true);
        else if (isHint)
            drawText(window, font, "HINT", 16, n.x, n.y + 48, Color(255, 215, 70), true);
        else if (isNewDelivery)
            drawText(window, font, "NEW", 16, n.x, n.y + 48, Color(255, 215, 70), true);
    }

    drawText(window, font, "PAR: " + to_string(totalPar) + "   PENALTIES: " + to_string(penalties),
        20, 60, 650, Color::White, false);
    drawText(window, font, "HEIGHT: " + to_string(corridorHeight) + "   ROOMS: " + to_string(shown) + "/" + to_string(nodeCount),
        20, 60, 680, Color(255, 215, 70), false);

    if (state == LOST)
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