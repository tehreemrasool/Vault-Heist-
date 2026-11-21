#include "Act3level1.h"
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

Act3Level1Screen::Act3Level1Screen(RenderWindow& win) : Screen(win)
{
    loadFont(font);

    if (backgroundTexture.loadFromFile("assets/pictures/Act3_level1.jpg"))
    {
        background.setTexture(backgroundTexture);
        background.setScale(1280.0f / backgroundTexture.getSize().x,
            720.0f / backgroundTexture.getSize().y);
    }

    nextScreen = SCREEN_ID;
    nodeCount = 0;
    maxDepth = 1;
    optimal = 0;
    capacity = 0;
    alarm = 0;
    penalties = 0;
    found = 0;
    hintUsed = false;
    hintIdx = -1;
    state = LOST;
    seed = (unsigned)time(NULL) + 101;
    puzzleSeed = seed;
    flashTime = 0;
    messageTime = 0;
    animTime = 0;
    messageColor = Color::White;

    for (int i = 0; i < MAXN; i++)
    {
        checked[i] = false;
        isBad[i] = false;
    }

    tree.setRecordEvents(false);
}

int Act3Level1Screen::indexOfId(int id) const
{
    for (int i = 0; i < nodeCount; i++)
        if (nodes[i].id == id) return i;
    return -1;
}

// inorder layout, same idea as the partner's setTreePositions
void Act3Level1Screen::layoutRec(int i, int& pos)
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

void Act3Level1Screen::loadModel()
{
    nodeCount = tree.collectAVLNodes(nodes, MAXN);

    maxDepth = 1;
    optimal = 0;
    for (int i = 0; i < nodeCount; i++)
    {
        if (nodes[i].depth > maxDepth) maxDepth = nodes[i].depth;
        isBad[i] = nodes[i].unbalanced;
        if (isBad[i]) optimal++;
    }

    int root = -1;
    for (int i = 0; i < nodeCount; i++)
        if (nodes[i].parentId == -1) { root = i; break; }

    int pos = 0;
    layoutRec(root, pos);

    capacity = (3 * optimal + 1) / 2 + 2;
}

void Act3Level1Screen::restartRun()
{
    for (int i = 0; i < MAXN; i++) checked[i] = false;
    alarm = 0;
    penalties = 0;
    found = 0;
    hintUsed = false;
    hintIdx = -1;
    flashTime = 0;
    messageTime = 0;
    message.clear();
    state = (optimal > 0) ? PLAYING : LOST;
}

void Act3Level1Screen::newPuzzle()
{
    int attempts = 0;

    while (true)
    {
        puzzleSeed = seed++;
        unsigned st = puzzleSeed;
        int n = 7 + (int)(AVLTree::nextRandom(st) % 6);

        tree.resetTree();
        tree.loadRandomUnbalanced(puzzleSeed, n);

        int ids[MAXN];
        int bad = tree.getUnbalancedIds(ids, MAXN);
        attempts++;

        if ((bad >= 2 && tree.nodeCount() > 0) || attempts >= 40)
            break;
    }

    loadModel();
    restartRun();
}

void Act3Level1Screen::onEnter()
{
    isActive = true;
    nextScreen = SCREEN_ID;
    newPuzzle();
}

void Act3Level1Screen::onResume()
{
    isActive = true;
    nextScreen = SCREEN_ID;
    flashTime = 0;
}

int Act3Level1Screen::findNodeAt(float x, float y) const
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

void Act3Level1Screen::clickNode(int i)
{
    if (state != PLAYING) return;

    if (checked[i])
    {
        message = "You already checked that room.";
        messageColor = Color(255, 200, 90);
        messageTime = 1.0f;
        return;
    }

    checked[i] = true;
    alarm++;
    if (hintIdx == i) hintIdx = -1;

    if (isBad[i])
    {
        found++;
        message = "UNBALANCED! Balance factor " + to_string(nodes[i].balance);
        messageColor = Color(120, 255, 120);
    }
    else
    {
        penalties++;
        flashTime = 0.35f;
        message = "That room is balanced! Alarm +1";
        messageColor = Color(255, 110, 110);
    }
    messageTime = 1.3f;

    if (found == optimal) state = WON;
    else if (alarm >= capacity) state = LOST;
}

void Act3Level1Screen::useHint()
{
    if (state != PLAYING) return;

    for (int i = 0; i < nodeCount; i++)
    {
        if (isBad[i] && !checked[i])
        {
            hintIdx = i;
            hintUsed = true;
            message = "Hint: look at the gold room.";
            messageColor = Color(255, 215, 70);
            messageTime = 1.3f;
            return;
        }
    }
}

int Act3Level1Screen::stars() const
{
    if (state != WON) return 0;

    int s = 1;
    if (alarm <= optimal && penalties == 0) s = 3;
    else if (alarm * 4 <= 5 * optimal && penalties <= 1) s = 2;

    if (hintUsed && s > 2) s = 2;
    return s;
}

int Act3Level1Screen::score() const
{
    if (state != WON) return 0;
    int s = 1000 - 10 * alarm - (hintUsed ? 50 : 0);
    return s < 0 ? 0 : s;
}

void Act3Level1Screen::handleInput()
{
    Event e;

    while (window.pollEvent(e))
    {
        if (e.type == Event::Closed)
            nextScreen = 99;

        if (e.type == Event::MouseButtonPressed && e.mouseButton.button == Mouse::Left)
        {
            int hit = findNodeAt((float)e.mouseButton.x, (float)e.mouseButton.y);
            if (hit != -1)
                clickNode(hit);
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

void Act3Level1Screen::update(float dt)
{
    animTime += dt;
    if (flashTime > 0) flashTime -= dt;
    if (messageTime > 0) messageTime -= dt;
}

void Act3Level1Screen::draw()
{
    window.clear();
    window.draw(background);

    drawText(window, font, "ACT 3 - LEVEL 1", 40, 500, 15, Color::White, false);

    // ---- HUD boxes ----
    RectangleShape alarmBox(Vector2f(220, 50));
    RectangleShape foundBox(Vector2f(320, 50));

    alarmBox.setPosition(50, 70);
    foundBox.setPosition(910, 70);
    alarmBox.setFillColor(Color::Black);
    foundBox.setFillColor(Color::Black);
    alarmBox.setOutlineColor(Color(180, 100, 220));
    alarmBox.setOutlineThickness(3);
    foundBox.setOutlineColor(Color(180, 100, 220));
    foundBox.setOutlineThickness(3);

    window.draw(alarmBox);
    window.draw(foundBox);

    // alarm meter fill
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

    drawText(window, font, "FOUND: " + to_string(found) + " / " + to_string(optimal),
        20, 1070, 95, Color::White, true);

    if (messageTime > 0)
        drawText(window, font, message, 22, 590, 90, messageColor, true);
    else
        drawText(window, font, "Click every room whose balance is beyond +1 or -1",
            18, 590, 90, Color::White, true);

    drawText(window, font, "Click Room: Check | H: Hint | Space: Reset | R: New Tree | Esc: Briefing",
        16, 640, 125, Color(208, 221, 183), true);

    // ---- edges ----
    for (int i = 0; i < nodeCount; i++)
    {
        int l = indexOfId(nodes[i].leftId);
        int r = indexOfId(nodes[i].rightId);
        if (l != -1) drawEdge(window, nodes[i].x, nodes[i].y, nodes[l].x, nodes[l].y);
        if (r != -1) drawEdge(window, nodes[i].x, nodes[i].y, nodes[r].x, nodes[r].y);
    }

    // ---- rooms ----
    for (int i = 0; i < nodeCount; i++)
    {
        bool revealed = SHOW_COLOURS_FROM_START || checked[i] || state != PLAYING;
        float radius = 30.0f;

        if (i == hintIdx)
        {
            int alpha = 170 + (int)(50 * sin(animTime * 6));
            CircleShape halo(radius + 13);
            halo.setOrigin(radius + 13, radius + 13);
            halo.setPosition(nodes[i].x, nodes[i].y);
            halo.setFillColor(Color(240, 190, 50, (Uint8)(alpha / 3)));
            window.draw(halo);
        }

        CircleShape circle(radius);
        circle.setOrigin(radius, radius);
        circle.setPosition(nodes[i].x, nodes[i].y);
        circle.setFillColor(Color(73, 103, 164));
        circle.setOutlineThickness(7);

        if (i == hintIdx)
            circle.setOutlineColor(Color(255, 215, 70));
        else if (revealed && isBad[i])
            circle.setOutlineColor(Color(255, 110, 110));
        else if (revealed)
            circle.setOutlineColor(Color::Green);
        else
            circle.setOutlineColor(Color(208, 221, 183));

        window.draw(circle);

        drawText(window, font, to_string(nodes[i].value),
            26, nodes[i].x, nodes[i].y, Color::White, true);
    }

    // ---- balance badges (drawn after all rooms so nothing covers them) ----
    for (int i = 0; i < nodeCount; i++)
    {
        bool revealed = SHOW_COLOURS_FROM_START || checked[i] || state != PLAYING;
        int b = nodes[i].balance;
        string txt = (b > 0 ? "+" : "") + to_string(b);
        if (revealed && isBad[i]) txt += "!";

        RectangleShape badge(Vector2f(48, 24));
        badge.setOrigin(24, 12);
        badge.setPosition(nodes[i].x + 28, nodes[i].y - 28);
        badge.setOutlineThickness(2);

        if (revealed && isBad[i])
        {
            badge.setFillColor(Color(200, 60, 60));
            badge.setOutlineColor(Color::White);
        }
        else if (revealed)
        {
            badge.setFillColor(Color(60, 160, 80));
            badge.setOutlineColor(Color::White);
        }
        else
        {
            badge.setFillColor(Color::Black);
            badge.setOutlineColor(Color(180, 100, 220));
        }
        window.draw(badge);

        drawText(window, font, txt, 16, nodes[i].x + 28, nodes[i].y - 28, Color::White, true);
    }

    // ---- hover tooltip ----
    Vector2i mp = Mouse::getPosition(window);
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

    // ---- bottom line ----
    drawText(window, font, "BF = height(left) - height(right)",
        16, 60, 640, Color(208, 221, 183), false);
    drawText(window, font, "SEED: " + to_string(puzzleSeed),
        16, 1100, 672, Color(208, 221, 183), false);

    if (state == WON)
    {
        drawText(window, font,
            "VAULT CLEARED!  STARS: " + to_string(stars()) + "  SCORE: " + to_string(score()) + "  (ENTER)",
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