#include "screen.h"
#include "helpers.h"
#include <cmath>
using namespace sf;
using namespace std;
static void loadAct4Font(Font& font)
{
    if (!font.loadFromFile("assets/fonts/ARIAL.ttf"))
        loadFont(font);
}

static void loadAct4Background(Texture& tex, Sprite& sprite)
{
    if (tex.loadFromFile("assets/pictures/act4l1.jfif"))
    {
        sprite.setTexture(tex);
        sprite.setScale(1280.0f / tex.getSize().x, 720.0f / tex.getSize().y);
    }
}

static void drawLine(RenderWindow& window, Vector2f a, Vector2f b, float thick, Color c)
{
    Vector2f d = b - a;
    float length = sqrt(d.x * d.x + d.y * d.y);

    RectangleShape line(Vector2f(length, thick));
    line.setOrigin(0, thick / 2.0f);
    line.setPosition(a);
    line.setRotation(atan2(d.y, d.x) * 180.0f / 3.14159f);
    line.setFillColor(c);
    window.draw(line);
}

static void drawAct4Hud(RenderWindow& window, const Font& font, int levelNo,
    int alarm, int capacity, const string& rightText)
{
    drawText(window, font, "ACT 4 - LEVEL " + to_string(levelNo), 40, 500, 15, Color::White, false);

    RectangleShape alarmBox(Vector2f(240, 50));
    RectangleShape infoBox(Vector2f(420, 50));

    alarmBox.setPosition(50, 70);
    infoBox.setPosition(810, 70);
    alarmBox.setFillColor(Color::Black);
    infoBox.setFillColor(Color::Black);
    alarmBox.setOutlineColor(Color(22, 157, 81));
    alarmBox.setOutlineThickness(3);
    infoBox.setOutlineColor(Color(22, 157, 81));
    infoBox.setOutlineThickness(3);

    window.draw(alarmBox);
    window.draw(infoBox);

    drawText(window, font, "ALARM: " + to_string(alarm) + " / " + to_string(capacity),
        18, 70, 82, Color::White, false);

    drawText(window, font, rightText, 20, 1020, 95, Color(255, 215, 70), true);
}

static const Color ACT4_FALLBACK_BG(18, 12, 30);


Act4Level1Screen::Act4Level1Screen(RenderWindow& win) : Screen(win)
{
    loadAct4Font(font);
    loadAct4Background(backgroundTexture, background);

    hintButton = makeRoundedRect(200, 50, 12);
    hintButton.setPosition(50, 650);
    hintButton.setFillColor(Color(22, 157, 81, 120));

    hint.setFont(font);
    hint.setCharacterSize(22);
    hint.setFillColor(Color::Black);
    hint.setString("HINT");
    centerText(hint, hintButton);

    hintTime = 0;
    hintP = hintSide = hintC = -1;
    hintMsg = "";

    nextScreen = 17;
    seed = (unsigned)time(NULL) + 53;
    selectedNode = -1;
    flashTime = 0;
    animTime = 0;
    messageColor = Color::White;

    for (int i = 0; i < 15; i++)
    {
        nodeX[i] = 0;
        nodeY[i] = 0;
    }
}

void Act4Level1Screen::showHint()
{
    int p, side, c;
    if (!level.getHint(p, side, c))
    {
        if (level.getState() == act4Level1::PLAYING)
        {
            hintMsg = "Every link is correct - press Enter!";
            hintP = hintC = -1;
            hintTime = 4.0f;
        }
        return;
    }

    hintP = p;
    hintSide = side;
    hintC = c;

    if (side == act4Level1::HINT_UNLINK)
        hintMsg = "Click " + to_string(level.getVal(p)) + " twice to unlink it";
    else
        hintMsg = "Click " + to_string(level.getVal(p)) + ", then " +
        (side == act4Level1::LEFT ? "LEFT" : "RIGHT") + "-click " +
        to_string(level.getVal(c));
    hintTime = 5.0f;
}

void Act4Level1Screen::newPuzzle()
{
    srand(seed++);
    level.start();

    selectedNode = -1;
    flashTime = 0;
    hintTime = 0;
    message = "Connect the rooms into a complete tree where every parent is BIGGER than its children";
    messageColor = Color::White;

    int cols = 5;
    for (int i = 0; i < level.getN(); i++)
    {
        int row = i / cols;
        int col = i % cols;
        nodeX[i] = 240.0f + col * 200.0f;
        nodeY[i] = 290.0f + row * 170.0f;
    }
}

void Act4Level1Screen::onEnter()
{
    isActive = true;
    nextScreen = 17;
    newPuzzle();
}

void Act4Level1Screen::onResume()
{
    isActive = true;
    nextScreen = 17;
    flashTime = 0;
}

void Act4Level1Screen::doCheck()
{
    int r = level.check();

    if (r == act4Level1::CHECK_NOT_CONNECTED)
    {
        flashTime = 0.35f;
        message = "Connect all rooms first! (" + to_string(level.getN() - 1) + " links needed)";
        messageColor = Color(255, 200, 90);
    }
    else if (r == act4Level1::CHECK_NOT_COMPLETE)
    {
        flashTime = 0.35f;
        message = "Not a complete tree! Fill every level from the left. Alarm +1";
        messageColor = Color(255, 110, 110);
    }
    else if (r == act4Level1::CHECK_NOT_HEAP)
    {
        flashTime = 0.35f;
        message = "A parent is smaller than its child! Alarm +1";
        messageColor = Color(255, 110, 110);
    }
    else if (r == act4Level1::CHECK_OK)
    {
        message = "MAX HEAP BUILT!";
        messageColor = Color(120, 255, 120);
    }
}

void Act4Level1Screen::handleInput()
{
    Event e;

    while (window.pollEvent(e))
    {
        if (e.type == Event::Closed)
            nextScreen = 99;

        if (e.type == Event::MouseButtonPressed)
        {
            Vector2f m = window.mapPixelToCoords(Vector2i(e.mouseButton.x, e.mouseButton.y));
            float mx = m.x;
            float my = m.y;

            if (e.mouseButton.button == Mouse::Left && hintButton.getGlobalBounds().contains(m))
            {
                showHint();
                continue;
            }

            int clicked = -1;
            for (int i = 0; i < level.getN(); i++)
            {
                float dx = mx - nodeX[i];
                float dy = my - nodeY[i];
                if (dx * dx + dy * dy <= 30.0f * 30.0f)
                {
                    clicked = i;
                    break;
                }
            }

            if (clicked != -1)
            {
                if (selectedNode == -1)
                {
                    selectedNode = clicked;
                }
                else if (selectedNode == clicked)
                {
                    level.detach(clicked);
                    selectedNode = -1;
                }
                else
                {
                    int side = (e.mouseButton.button == Mouse::Left)
                        ? act4Level1::LEFT
                        : act4Level1::RIGHT;

                    if (!level.connect(selectedNode, side, clicked))
                    {
                        flashTime = 0.35f;
                        message = "Cannot create a loop!";
                        messageColor = Color(255, 110, 110);
                    }
                    selectedNode = -1;
                    hintTime = 0;          // player made a move, hide the old hint
                }
            }
            else
            {
                selectedNode = -1;
            }
        }

        if (e.type == Event::KeyPressed)
        {
            if (e.key.code == Keyboard::Escape)
                nextScreen = 25;

            else if (e.key.code == Keyboard::R)
                newPuzzle();

            else if (e.key.code == Keyboard::N)
                nextScreen = 18;                 

            else if (e.key.code == Keyboard::Space)
            {
                level.clearLinks();
                selectedNode = -1;
            }

            else if (e.key.code == Keyboard::Return)
            {
                if (level.getState() == act4Level1::WON)
                    nextScreen = 18;             
                else if (level.getState() == act4Level1::PLAYING)
                    doCheck();
            }
        }
    }
}

void Act4Level1Screen::update(float dt)
{
    animTime += dt;
    if (flashTime > 0)
        flashTime -= dt;
    if (hintTime > 0)
        hintTime -= dt;

    Vector2f mouse = window.mapPixelToCoords(Mouse::getPosition(window));
    applyHover(hintButton, mouse);
}

void Act4Level1Screen::draw()
{
    window.clear(ACT4_FALLBACK_BG);
    window.draw(background);

    int n = level.getN();

    int links = level.linkCount();
    drawAct4Hud(window, font, 1, level.getAlarm(), level.getCapacity(),
        "LINKS: " + to_string(links) + " / " + to_string(n - 1));

    RectangleShape arrayBox(Vector2f(480, 50));
    arrayBox.setPosition(310, 70);
    arrayBox.setFillColor(Color::Black);
    arrayBox.setOutlineColor(Color(22, 157, 81));
    arrayBox.setOutlineThickness(3);
    window.draw(arrayBox);
    window.draw(hintButton);
    window.draw(hint);

    string arrStr = "ARRAY: ";
    for (int i = 0; i < n; i++)
        arrStr += to_string(level.getVal(i)) + " ";
    drawText(window, font, arrStr, 17, 325, 84, Color::White, false);

    drawText(window, font, message, 19, 640, 140, messageColor, true);
    drawText(window, font,
        "Click parent, then child: Left-Click = LEFT child, Right-Click = RIGHT child | Same room twice: Unlink | Space: Reset | Enter: Check | R: New",
        14, 640, 166, Color(208, 221, 183), true);

    // links
    for (int i = 0; i < n; i++)
    {
        for (int side = 0; side < 2; side++)
        {
            int childIdx = level.getChild(i, side);
            if (childIdx == -1) continue;

            Vector2f start(nodeX[i], nodeY[i]);
            Vector2f end(nodeX[childIdx], nodeY[childIdx]);

            drawLine(window, start, end, 5, side == act4Level1::LEFT ? Color::Yellow : Color::Cyan);

            float midX = (start.x + end.x) / 2.0f;
            float midY = (start.y + end.y) / 2.0f;
            drawText(window, font, side == act4Level1::LEFT ? "L" : "R",
                18, midX, midY - 12, Color::White, true);
        }
    }

    // rooms
    for (int i = 0; i < n; i++)
    {
        float radius = 30.0f;
        CircleShape circle(radius);
        circle.setOrigin(radius, radius);
        circle.setPosition(nodeX[i], nodeY[i]);
        circle.setFillColor(Color(73, 103, 164));
        circle.setOutlineThickness(7);

        if (level.getState() == act4Level1::WON)
            circle.setOutlineColor(Color::Green);
        else if (i == selectedNode)
            circle.setOutlineColor(Color(151, 212, 110));
        else
            circle.setOutlineColor(Color(208, 221, 183));

        window.draw(circle);

        drawText(window, font, to_string(level.getVal(i)),
            26, nodeX[i], nodeY[i], Color::White, true);
    }

    if (hintTime > 0)
    {
        int ring[2] = { hintP, hintC };
        for (int r = 0; r < 2; r++)
        {
            if (ring[r] < 0) continue;

            CircleShape c(38);
            c.setOrigin(38, 38);
            c.setPosition(nodeX[ring[r]], nodeY[ring[r]]);
            c.setFillColor(Color::Transparent);
            c.setOutlineThickness(5);
            c.setOutlineColor(Color(22, 157, 81));
            window.draw(c);
        }

        Text t(hintMsg, font, 20);
        float w = t.getLocalBounds().width + 30;

        RectangleShape box(Vector2f(w, 40));
        box.setPosition(265, 655);
        box.setFillColor(Color(0, 0, 0, 200));
        box.setOutlineThickness(3);
        box.setOutlineColor(Color(22, 157, 81));
        window.draw(box);

        drawText(window, font, hintMsg, 20, 280, 662, Color(255, 220, 60), false);
    }

    if (level.getCheckedCount() > 0)
    {
        string s = "TREE AS ARRAY (level order): ";
        for (int i = 0; i < level.getCheckedCount(); i++)
            s += to_string(level.getChecked(i)) + " ";
        drawText(window, font, s, 20, 640, 630, Color(208, 221, 183), true);
    }

    if (level.getState() == act4Level1::WON)
    {
        drawText(window, font,
            "MAX HEAP BUILT!  STARS: " + to_string(level.stars()) + "  SCORE: " + to_string(level.score()) + "  - PRESS ENTER",
            26, 640, 680, Color::Green, true);
    }
    else if (level.getState() == act4Level1::LOST)
    {
        drawText(window, font, "ALARM TRIGGERED! YOU LOST!   (R: retry)",
            27, 640, 680, Color::Red, true);
    }

    if (flashTime > 0)
    {
        RectangleShape redScreen(Vector2f(1280, 720));
        redScreen.setFillColor(Color(255, 0, 0, 90));
        window.draw(redScreen);
    }
}

//-----------------------------level 2----------------------------------


Act4Level2Screen::Act4Level2Screen(RenderWindow& win) : Screen(win)
{
    loadAct4Font(font);
    if (backgroundTexture.loadFromFile("assets/pictures/act4l2.jfif"))
    {
        background.setTexture(backgroundTexture);
        background.setScale(1280.0f / backgroundTexture.getSize().x, 720.0f / backgroundTexture.getSize().y);
    }
    hintButton = makeRoundedRect(200, 50, 12);
    hintButton.setPosition(50, 650);
    hintButton.setFillColor(Color(22, 157, 81, 120));

    hint.setFont(font);
    hint.setCharacterSize(22);
    hint.setFillColor(Color::Black);
    hint.setString("HINT");
    centerText(hint, hintButton);

    hintTime = 0;
    hintA = hintB = -1;
    hintMsg = "";

    nextScreen = 18;
    seed = (unsigned)time(NULL) + 71;
    selectedNode = -1;
    flashTime = 0;
    animTime = 0;
    messageColor = Color::White;

    for (int i = 0; i < 15; i++)
    {
        nodeX[i] = 0;
        nodeY[i] = 0;
    }
}

void Act4Level2Screen::showHint()
{
    int a, b;
    if (!level.getHint(a, b))
    {
        if (level.getState() == act4Level2::PLAYING)
        {
            hintMsg = "Already a valid Max Heap!";
            hintA = hintB = -1;
            hintTime = 4.0f;
        }
        return;
    }

    hintA = a;
    hintB = b;
    hintMsg = "Swap " + to_string(level.getVal(a)) + " and " + to_string(level.getVal(b));
    hintTime = 5.0f;
}

void Act4Level2Screen::newPuzzle()
{
    srand(seed++);
    level.start();

    selectedNode = -1;
    flashTime = 0;
    hintTime = 0;
    message = "Click two rooms to swap their values until every parent is BIGGER than its children";
    messageColor = Color::White;

    for (int i = 0; i < level.getN(); i++)
    {
        int depth = (int)log2(i + 1);
        int posInRow = i - ((1 << depth) - 1);
        int slotsInRow = 1 << depth;

        nodeX[i] = 1280.0f * (posInRow + 1) / (slotsInRow + 1);
        nodeY[i] = 235.0f + depth * 120.0f;
    }
}

void Act4Level2Screen::onEnter()
{
    isActive = true;
    nextScreen = 18;
    newPuzzle();
}

void Act4Level2Screen::onResume()
{
    isActive = true;
    nextScreen = 18;
    flashTime = 0;
}

void Act4Level2Screen::handleInput()
{
    Event e;

    while (window.pollEvent(e))
    {
        if (e.type == Event::Closed)
            nextScreen = 99;

        if (e.type == Event::MouseButtonPressed && e.mouseButton.button == Mouse::Left)
        {
            Vector2f m = window.mapPixelToCoords(Vector2i(e.mouseButton.x, e.mouseButton.y));

            if (hintButton.getGlobalBounds().contains(m))
            {
                showHint();
                continue;
            }

            if (level.getState() != act4Level2::PLAYING)
                continue;

            int clicked = -1;
            for (int i = 0; i < level.getN(); i++)
            {
                float dx = m.x - nodeX[i];
                float dy = m.y - nodeY[i];
                if (dx * dx + dy * dy <= 30.0f * 30.0f)
                {
                    clicked = i;
                    break;
                }
            }

            if (clicked != -1)
            {
                if (selectedNode == -1)
                {
                    selectedNode = clicked;
                }
                else if (selectedNode == clicked)
                {
                    selectedNode = -1;
                }
                else
                {
                    int prevState = level.getState();
                    level.swapNodes(selectedNode, clicked);
                    selectedNode = -1;
                    hintTime = 0;

                    if (level.getState() == act4Level2::WON)
                    {
                        message = "MAX HEAP ORGANIZED!";
                        messageColor = Color(120, 255, 120);
                    }
                    else if (level.getState() == act4Level2::LOST && prevState == act4Level2::PLAYING)
                    {
                        flashTime = 0.35f;
                        message = "OUT OF MOVES! Alarm filled.";
                        messageColor = Color(255, 110, 110);
                    }
                }
            }
            else
            {
                selectedNode = -1;
            }
        }

        if (e.type == Event::KeyPressed)
        {
            if (e.key.code == Keyboard::Escape)
                nextScreen = 25;

            else if (e.key.code == Keyboard::R)
                newPuzzle();

            else if (e.key.code == Keyboard::N)
                nextScreen = 19; 

            else if (e.key.code == Keyboard::Space)
            {
                level.resetPuzzle();
                selectedNode = -1;
                hintTime = 0;
            }

            else if (e.key.code == Keyboard::Return)
            {
                if (level.getState() == act4Level2::WON)
                    nextScreen = 19; 
            }
        }
    }
}

void Act4Level2Screen::update(float dt)
{
    animTime += dt;
    if (flashTime > 0)
        flashTime -= dt;
    if (hintTime > 0)
        hintTime -= dt;

    Vector2f mouse = window.mapPixelToCoords(Mouse::getPosition(window));
    applyHover(hintButton, mouse);
}

void Act4Level2Screen::draw()
{
    window.clear(ACT4_FALLBACK_BG);
    window.draw(background);

    int n = level.getN();

    drawAct4Hud(window, font, 2, level.getAlarm(), level.getCapacity(),
        "ROOMS: " + to_string(n));

    window.draw(hintButton);
    window.draw(hint);

    drawText(window, font, message, 19, 640, 140, messageColor, true);
    drawText(window, font,
        "Click first room, then second room to swap | Click same room: Deselect | Space: Reset | R: New",
        14, 640, 166, Color(208, 221, 183), true);

    for (int i = 0; i < n; i++)
    {
        int left = 2 * i + 1;
        int right = 2 * i + 2;

        if (left < n)
            drawLine(window, Vector2f(nodeX[i], nodeY[i]), Vector2f(nodeX[left], nodeY[left]), 5, Color(208, 221, 183));
        if (right < n)
            drawLine(window, Vector2f(nodeX[i], nodeY[i]), Vector2f(nodeX[right], nodeY[right]), 5, Color(208, 221, 183));
    }

    for (int i = 0; i < n; i++)
    {
        float radius = 30.0f;
        CircleShape circle(radius);
        circle.setOrigin(radius, radius);
        circle.setPosition(nodeX[i], nodeY[i]);
        circle.setFillColor(Color(73, 103, 164));
        circle.setOutlineThickness(7);

        if (level.getState() == act4Level2::WON)
            circle.setOutlineColor(Color::Green);
        else if (i == selectedNode)
            circle.setOutlineColor(Color(22, 157, 81));
        else
            circle.setOutlineColor(Color(208, 221, 183));

        window.draw(circle);

        drawText(window, font, to_string(level.getVal(i)),
            26, nodeX[i], nodeY[i], Color::White, true);
    }

    if (hintTime > 0)
    {
        int ring[2] = { hintA, hintB };
        for (int r = 0; r < 2; r++)
        {
            if (ring[r] < 0) continue;

            CircleShape c(38);
            c.setOrigin(38, 38);
            c.setPosition(nodeX[ring[r]], nodeY[ring[r]]);
            c.setFillColor(Color::Transparent);
            c.setOutlineThickness(5);
            c.setOutlineColor(Color(22, 157, 81));
            window.draw(c);
        }

        Text t(hintMsg, font, 20);
        float w = t.getLocalBounds().width + 30;

        RectangleShape box(Vector2f(w, 40));
        box.setPosition(265, 655);
        box.setFillColor(Color(0, 0, 0, 200));
        box.setOutlineThickness(3);
        box.setOutlineColor(Color(22, 157, 81));
        window.draw(box);

        drawText(window, font, hintMsg, 20, 280, 662, Color(255, 220, 60), false);
    }

    if (level.getState() == act4Level2::WON)
    {
        drawText(window, font,
            "MAX HEAP ORGANIZED!  STARS: " + to_string(level.stars()) + "  SCORE: " + to_string(level.score()) + "  - PRESS ENTER",
            26, 640, 680, Color::Green, true);
    }
    else if (level.getState() == act4Level2::LOST)
    {
        drawText(window, font, "ALARM TRIGGERED! YOU LOST!   (R: retry)",
            27, 640, 680, Color::Red, true);
    }

    if (flashTime > 0)
    {
        RectangleShape redScreen(Vector2f(1280, 720));
        redScreen.setFillColor(Color(255, 0, 0, 90));
        window.draw(redScreen);
    }
}

// ---------------------------act 4 level 3

//screen 18
Act4Level3Screen::Act4Level3Screen(RenderWindow& win) : Screen(win)
{
    loadAct4Font(font);
    if (backgroundTexture.loadFromFile("assets/pictures/act4l3.jfif"))
    {
        background.setTexture(backgroundTexture);
        background.setScale(1280.0f / backgroundTexture.getSize().x, 720.0f / backgroundTexture.getSize().y);
    }

    hintButton = makeRoundedRect(200, 50, 12);
    hintButton.setPosition(50, 650);
    hintButton.setFillColor(Color(212, 110, 194, 120));

    hint.setFont(font);
    hint.setCharacterSize(22);
    hint.setFillColor(Color::Black);
    hint.setString("HINT");
    centerText(hint, hintButton);

    removeRootButton = makeRoundedRect(230, 50, 12);
    removeRootButton.setPosition(1000, 650);
    removeRootButton.setFillColor(Color(212, 110, 194, 120));

    removeRootText.setFont(font);
    removeRootText.setCharacterSize(20);
    removeRootText.setFillColor(Color::Black);
    removeRootText.setString("REMOVE ROOT");
    centerText(removeRootText, removeRootButton);

    hintTime = 0;
    hintA = hintB = -1;
    hintMsg = "";

    nextScreen = 19;
    seed = (unsigned)time(NULL) + 137;
    selectedNode = -1;
    flashTime = 0;
    animTime = 0;
    messageColor = Color::White;

    for (int i = 0; i < 15; i++)
    {
        nodeX[i] = 0;
        nodeY[i] = 0;
    }
}

void Act4Level3Screen::showHint()
{
    int a, b;
    if (!level.getHint(a, b))
        return;

    hintA = a;
    hintB = b;

    if (!level.isRootRemoved())
        hintMsg = "Click REMOVE ROOT (or click root " + to_string(level.getVal(0)) + ")";
    else
        hintMsg = "Swap " + to_string(level.getVal(a)) + " with larger child " + to_string(level.getVal(b));

    hintTime = 5.0f;
}

void Act4Level3Screen::doRemoveRoot()
{
    if (level.getState() != act4Level3::PLAYING || level.isRootRemoved())
        return;

    int oldRoot = level.getVal(0);
    int movedLeaf = level.getVal(level.getN() - 1);

    if (level.removeRoot())
    {
        selectedNode = -1;
        hintTime = 0;

        if (level.getState() == act4Level3::WON)
        {
            message = "Removed " + to_string(oldRoot) + "! Heap is already valid!";
            messageColor = Color(120, 255, 120);
        }
        else if (level.getState() == act4Level3::LOST)
        {
            flashTime = 0.35f;
            message = "OUT OF MOVES! Alarm filled.";
            messageColor = Color(255, 110, 110);
        }
        else
        {
            message = "Removed " + to_string(oldRoot) + "! Now sift " + to_string(movedLeaf) + " down to fix the Max Heap";
            messageColor = Color(255, 220, 120);
        }
    }
}

void Act4Level3Screen::newPuzzle()
{
    srand(seed++);
    level.start();

    selectedNode = -1;
    flashTime = 0;
    hintTime = 0;
    message = "Step 1: Remove the root node (" + to_string(level.getVal(0)) + ") using the button or by clicking it";
    messageColor = Color::White;

    for (int i = 0; i < level.getN(); i++)
    {
        int depth = (int)log2(i + 1);
        int posInRow = i - ((1 << depth) - 1);
        int slotsInRow = 1 << depth;

        nodeX[i] = 1280.0f * (posInRow + 1) / (slotsInRow + 1);
        nodeY[i] = 235.0f + depth * 120.0f;
    }
}

void Act4Level3Screen::onEnter()
{
    isActive = true;
    nextScreen = 19;
    newPuzzle();
}

void Act4Level3Screen::onResume()
{
    isActive = true;
    nextScreen = 19;
    flashTime = 0;
}

void Act4Level3Screen::handleInput()
{
    Event e;

    while (window.pollEvent(e))
    {
        if (e.type == Event::Closed)
            nextScreen = 99;

        if (e.type == Event::MouseButtonPressed && e.mouseButton.button == Mouse::Left)
        {
            Vector2f m = window.mapPixelToCoords(Vector2i(e.mouseButton.x, e.mouseButton.y));

            if (hintButton.getGlobalBounds().contains(m))
            {
                showHint();
                continue;
            }

            if (removeRootButton.getGlobalBounds().contains(m))
            {
                doRemoveRoot();
                continue;
            }

            if (level.getState() != act4Level3::PLAYING)
                continue;

            int clicked = -1;
            for (int i = 0; i < level.getN(); i++)
            {
                float dx = m.x - nodeX[i];
                float dy = m.y - nodeY[i];
                if (dx * dx + dy * dy <= 30.0f * 30.0f)
                {
                    clicked = i;
                    break;
                }
            }

            if (clicked != -1)
            {
                if (!level.isRootRemoved() && clicked == 0)
                {
                    doRemoveRoot();
                    continue;
                }

                if (selectedNode == -1)
                {
                    selectedNode = clicked;
                }
                else if (selectedNode == clicked)
                {
                    selectedNode = -1;
                }
                else
                {
                    int prevState = level.getState();
                    bool valid = level.swapNodes(selectedNode, clicked);
                    selectedNode = -1;
                    hintTime = 0;

                    if (valid)
                    {
                        if (level.getState() == act4Level3::WON)
                        {
                            message = "MAX HEAP RESTORED!";
                            messageColor = Color(120, 255, 120);
                        }
                        else if (level.getState() == act4Level3::LOST && prevState == act4Level3::PLAYING)
                        {
                            flashTime = 0.35f;
                            message = "OUT OF MOVES! Alarm filled.";
                            messageColor = Color(255, 110, 110);
                        }
                        else
                        {
                            message = "Good swap! Keep sifting " + to_string(level.getVal(level.getCurIdx())) + " down";
                            messageColor = Color(160, 255, 160);
                        }
                    }
                    else
                    {
                        flashTime = 0.35f;
                        if (level.getState() == act4Level3::LOST && prevState == act4Level3::PLAYING)
                            message = "WRONG MOVE & OUT OF MOVES! Alarm filled.";
                        else if (!level.isRootRemoved())
                            message = "Remove the root FIRST! Move reverted (+1 alarm).";
                        else
                            message = "WRONG SWAP! Reverted to previous step (+1 alarm).";
                        messageColor = Color(255, 110, 110);
                    }
                }
            }
            else
            {
                selectedNode = -1;
            }
        }

        if (e.type == Event::KeyPressed)
        {
            if (e.key.code == Keyboard::Escape)
                nextScreen = 25;

            else if (e.key.code == Keyboard::R)
                newPuzzle();

            else if (e.key.code == Keyboard::N)
                nextScreen = 20; 

            else if (e.key.code == Keyboard::Space)
            {
                level.resetPuzzle();
                selectedNode = -1;
                hintTime = 0;
                message = "Puzzle reset! Step 1: Remove the root node (" + to_string(level.getVal(0)) + ")";
                messageColor = Color::White;
            }

            else if (e.key.code == Keyboard::Return)
            {
                if (level.getState() == act4Level3::WON)
                    nextScreen = 20; 
            }
        }
    }
}

void Act4Level3Screen::update(float dt)
{
    animTime += dt;
    if (flashTime > 0)
        flashTime -= dt;
    if (hintTime > 0)
        hintTime -= dt;

    Vector2f mouse = window.mapPixelToCoords(Mouse::getPosition(window));
    applyHover(hintButton, mouse);
    if (!level.isRootRemoved() && level.getState() == act4Level3::PLAYING)
        applyHover(removeRootButton, mouse);
}

void Act4Level3Screen::draw()
{
    window.clear(ACT4_FALLBACK_BG);
    window.draw(background);

    int n = level.getN();

    drawAct4Hud(window, font, 3, level.getAlarm(), level.getCapacity(),
        "ROOMS: " + to_string(n));

    window.draw(hintButton);
    window.draw(hint);

    if (!level.isRootRemoved() && level.getState() == act4Level3::PLAYING)
    {
        window.draw(removeRootButton);
        window.draw(removeRootText);
    }

    drawText(window, font, message, 19, 640, 140, messageColor, true);
    drawText(window, font,
        "1) Remove Root  2) Swap sinking node with its LARGER child | Space: Reset | R: New",
        14, 640, 166, Color(208, 221, 183), true);
    for (int i = 0; i < n; i++)
    {
        int left = 2 * i + 1;
        int right = 2 * i + 2;

        if (left < n)
            drawLine(window, Vector2f(nodeX[i], nodeY[i]), Vector2f(nodeX[left], nodeY[left]), 5, Color(208, 221, 183));
        if (right < n)
            drawLine(window, Vector2f(nodeX[i], nodeY[i]), Vector2f(nodeX[right], nodeY[right]), 5, Color(208, 221, 183));
    }
    for (int i = 0; i < n; i++)
    {
        float radius = 30.0f;
        CircleShape circle(radius);
        circle.setOrigin(radius, radius);
        circle.setPosition(nodeX[i], nodeY[i]);

        if (level.isRootRemoved() && i == level.getCurIdx() && level.getState() == act4Level3::PLAYING)
            circle.setFillColor(Color(95, 130, 195));
        else
            circle.setFillColor(Color(73, 103, 164));

        circle.setOutlineThickness(7);

        if (level.getState() == act4Level3::WON)
            circle.setOutlineColor(Color::Green);
        else if (i == selectedNode)
            circle.setOutlineColor(Color(212, 110, 194));
        else
            circle.setOutlineColor(Color(208, 221, 183));

        window.draw(circle);

        drawText(window, font, to_string(level.getVal(i)),
            26, nodeX[i], nodeY[i], Color::White, true);
    }

    // hint highlight
    if (hintTime > 0)
    {
        int ring[2] = { hintA, hintB };
        for (int r = 0; r < 2; r++)
        {
            if (ring[r] < 0 || ring[r] >= n) continue;

            CircleShape c(38);
            c.setOrigin(38, 38);
            c.setPosition(nodeX[ring[r]], nodeY[ring[r]]);
            c.setFillColor(Color::Transparent);
            c.setOutlineThickness(5);
            c.setOutlineColor(Color(255, 220, 60));
            window.draw(c);
        }

        Text t(hintMsg, font, 20);
        float w = t.getLocalBounds().width + 30;

        RectangleShape box(Vector2f(w, 40));
        box.setPosition(265, 655);
        box.setFillColor(Color(0, 0, 0, 200));
        box.setOutlineThickness(3);
        box.setOutlineColor(Color(212, 110, 194));
        window.draw(box);

        drawText(window, font, hintMsg, 20, 280, 662, Color(255, 220, 60), false);
    }

    if (level.getState() == act4Level3::WON)
    {
        drawText(window, font,
            "MAX HEAP RESTORED!  STARS: " + to_string(level.stars()) + "  SCORE: " + to_string(level.score()) + "  - PRESS ENTER",
            26, 640, 680, Color::Green, true);
    }
    else if (level.getState() == act4Level3::LOST)
    {
        drawText(window, font, "ALARM TRIGGERED! YOU LOST!   (R: retry)",
            27, 640, 680, Color::Red, true);
    }

    if (flashTime > 0)
    {
        RectangleShape redScreen(Vector2f(1280, 720));
        redScreen.setFillColor(Color(255, 0, 0, 90));
        window.draw(redScreen);
    }
}

//------------------------------act 4 level 4

Act4Level4Screen::Act4Level4Screen(RenderWindow& win) : Screen(win)
{
    loadAct4Font(font);
    if (backgroundTexture.loadFromFile("assets/pictures/act4l4.jfif"))
    {
        background.setTexture(backgroundTexture);
        background.setScale(1280.0f / backgroundTexture.getSize().x, 720.0f / backgroundTexture.getSize().y);
    }

    hintButton = makeRoundedRect(200, 50, 12);
    hintButton.setPosition(50, 650);
    hintButton.setFillColor(Color(212, 110, 194, 120));

    hint.setFont(font);
    hint.setCharacterSize(22);
    hint.setFillColor(Color::Black);
    hint.setString("HINT");
    centerText(hint, hintButton);

    hintTime = 0;
    hintA = hintB = -1;
    hintMsg = "";

    nextScreen = 20;
    seed = (unsigned)time(NULL) + 211;
    selectedNode = -1;
    flashTime = 0;
    animTime = 0;
    messageColor = Color::White;

    for (int i = 0; i < 15; i++)
    {
        nodeX[i] = 0;
        nodeY[i] = 0;
    }
}

void Act4Level4Screen::showHint()
{
    int a, b;
    if (!level.getHint(a, b))
        return;

    hintA = a;
    hintB = b;

    if (level.isNeedExtract())
        hintMsg = "Extract max: swap root " + to_string(level.getVal(a)) + " with last heap node " + to_string(level.getVal(b));
    else
        hintMsg = "Sift down: swap " + to_string(level.getVal(a)) + " with larger child " + to_string(level.getVal(b));

    hintTime = 5.0f;
}

void Act4Level4Screen::newPuzzle()
{
    srand(seed++);
    level.start();

    selectedNode = -1;
    flashTime = 0;
    hintTime = 0;
    message = "Step 1: Swap root (" + to_string(level.getVal(0)) + ") with last heap room (" + to_string(level.getVal(level.getHeapEnd() - 1)) + ")";
    messageColor = Color::White;

    for (int i = 0; i < level.getN(); i++)
    {
        int depth = (int)log2(i + 1);
        int posInRow = i - ((1 << depth) - 1);
        int slotsInRow = 1 << depth;

        nodeX[i] = 1280.0f * (posInRow + 1) / (slotsInRow + 1);
        nodeY[i] = 235.0f + depth * 120.0f;
    }
}

void Act4Level4Screen::onEnter()
{
    isActive = true;
    nextScreen = 20;
    newPuzzle();
}

void Act4Level4Screen::onResume()
{
    isActive = true;
    nextScreen = 20;
    flashTime = 0;
}

void Act4Level4Screen::handleInput()
{
    Event e;

    while (window.pollEvent(e))
    {
        if (e.type == Event::Closed)
            nextScreen = 99;

        if (e.type == Event::MouseButtonPressed && e.mouseButton.button == Mouse::Left)
        {
            Vector2f m = window.mapPixelToCoords(Vector2i(e.mouseButton.x, e.mouseButton.y));

            if (hintButton.getGlobalBounds().contains(m))
            {
                showHint();
                continue;
            }

            if (level.getState() != act4Level4::PLAYING)
                continue;

            int clicked = -1;
            for (int i = 0; i < level.getN(); i++)
            {
                float dx = m.x - nodeX[i];
                float dy = m.y - nodeY[i];
                if (dx * dx + dy * dy <= 30.0f * 30.0f)
                {
                    clicked = i;
                    break;
                }
            }

            if (clicked != -1)
            {
                if (selectedNode == -1)
                {
                    selectedNode = clicked;
                }
                else if (selectedNode == clicked)
                {
                    selectedNode = -1;
                }
                else
                {
                    int prevState = level.getState();
                    bool wasExtract = level.isNeedExtract();
                    bool valid = level.swapNodes(selectedNode, clicked);
                    selectedNode = -1;
                    hintTime = 0;

                    if (valid)
                    {
                        if (level.getState() == act4Level4::WON)
                        {
                            message = "TREE FULLY SORTED BY HEAP SORT!";
                            messageColor = Color(120, 255, 120);
                        }
                        else if (level.getState() == act4Level4::LOST && prevState == act4Level4::PLAYING)
                        {
                            flashTime = 0.35f;
                            message = "OUT OF MOVES! Alarm filled.";
                            messageColor = Color(255, 110, 110);
                        }
                        else if (level.isNeedExtract())
                        {
                            message = "Heap fixed! Now swap root (" + to_string(level.getVal(0))
                                + ") with last active room (" + to_string(level.getVal(level.getHeapEnd() - 1)) + ")";
                            messageColor = Color(255, 220, 120);
                        }
                        else if (wasExtract)
                        {
                            message = "Max locked! Now sift " + to_string(level.getVal(level.getCurIdx())) + " down inside the active heap";
                            messageColor = Color(160, 255, 160);
                        }
                        else
                        {
                            message = "Good swap! Keep sifting " + to_string(level.getVal(level.getCurIdx())) + " down";
                            messageColor = Color(160, 255, 160);
                        }
                    }
                    else
                    {
                        flashTime = 0.35f;
                        if (level.getState() == act4Level4::LOST && prevState == act4Level4::PLAYING)
                            message = "WRONG MOVE & OUT OF MOVES! Alarm filled.";
                        else if (level.isNeedExtract())
                            message = "WRONG SWAP! Swap root (" + to_string(level.getVal(0)) + ") with last active room (" + to_string(level.getVal(level.getHeapEnd() - 1)) + ") (+1 alarm)";
                        else
                            message = "WRONG SWAP! Sift " + to_string(level.getVal(level.getCurIdx())) + " with its LARGER active child (+1 alarm)";
                        messageColor = Color(255, 110, 110);
                    }
                }
            }
            else
            {
                selectedNode = -1;
            }
        }

        if (e.type == Event::KeyPressed)
        {
            if (e.key.code == Keyboard::Escape)
                nextScreen = 25;

            else if (e.key.code == Keyboard::R)
                newPuzzle();

            else if (e.key.code == Keyboard::N)
                nextScreen = 21;

            else if (e.key.code == Keyboard::Space)
            {
                level.resetPuzzle();
                selectedNode = -1;
                hintTime = 0;
                message = "Puzzle reset! Swap root (" + to_string(level.getVal(0)) + ") with last heap room (" + to_string(level.getVal(level.getHeapEnd() - 1)) + ")";
                messageColor = Color::White;
            }

            else if (e.key.code == Keyboard::Return)
            {
                if (level.getState() == act4Level4::WON)
                    nextScreen = 21;
            }
        }
    }
}

void Act4Level4Screen::update(float dt)
{
    animTime += dt;
    if (flashTime > 0)
        flashTime -= dt;
    if (hintTime > 0)
        hintTime -= dt;

    Vector2f mouse = window.mapPixelToCoords(Mouse::getPosition(window));
    applyHover(hintButton, mouse);
}

void Act4Level4Screen::draw()
{
    window.clear(ACT4_FALLBACK_BG);
    window.draw(background);

    int n = level.getN();
    int heapEnd = level.getHeapEnd();
    int sortedCount = n - heapEnd;

    drawAct4Hud(window, font, 4, level.getAlarm(), level.getCapacity(),
        "SORTED: " + to_string(sortedCount) + "/" + to_string(n));

    window.draw(hintButton);
    window.draw(hint);

    drawText(window, font, message, 19, 640, 140, messageColor, true);
    drawText(window, font,
        "1) Swap Root <-> Last Active  2) Sift Root Down (Green = Locked Sorted) | Space: Reset | R: New",
        14, 640, 166, Color(208, 221, 183), true);

    for (int i = 0; i < n; i++)
    {
        int left = 2 * i + 1;
        int right = 2 * i + 2;

        if (left < n)
        {
            Color edgeCol = (left >= heapEnd || level.getState() == act4Level4::WON)
                ? Color(110, 140, 110, 140)
                : Color(208, 221, 183);
            drawLine(window, Vector2f(nodeX[i], nodeY[i]), Vector2f(nodeX[left], nodeY[left]), 5, edgeCol);
        }
        if (right < n)
        {
            Color edgeCol = (right >= heapEnd || level.getState() == act4Level4::WON)
                ? Color(110, 140, 110, 140)
                : Color(208, 221, 183);
            drawLine(window, Vector2f(nodeX[i], nodeY[i]), Vector2f(nodeX[right], nodeY[right]), 5, edgeCol);
        }
    }

    for (int i = 0; i < n; i++)
    {
        bool isLocked = (i >= heapEnd) || (level.getState() == act4Level4::WON);

        float radius = 30.0f;
        CircleShape circle(radius);
        circle.setOrigin(radius, radius);
        circle.setPosition(nodeX[i], nodeY[i]);

        if (isLocked)
            circle.setFillColor(Color(45, 115, 75)); 
        else if (!level.isNeedExtract() && i == level.getCurIdx() && level.getState() == act4Level4::PLAYING)
            circle.setFillColor(Color(95, 130, 195)); 
        else
            circle.setFillColor(Color(73, 103, 164)); 

        circle.setOutlineThickness(7);

        if (i == selectedNode)
            circle.setOutlineColor(Color(212, 110, 194));
        else if (isLocked)
            circle.setOutlineColor(Color::Green);
        else
            circle.setOutlineColor(Color(208, 221, 183));

        window.draw(circle);

        drawText(window, font, to_string(level.getVal(i)),
            26, nodeX[i], nodeY[i], Color::White, true);
    }

    if (hintTime > 0)
    {
        int ring[2] = { hintA, hintB };
        for (int r = 0; r < 2; r++)
        {
            if (ring[r] < 0 || ring[r] >= n) continue;

            CircleShape c(38);
            c.setOrigin(38, 38);
            c.setPosition(nodeX[ring[r]], nodeY[ring[r]]);
            c.setFillColor(Color::Transparent);
            c.setOutlineThickness(5);
            c.setOutlineColor(Color(255, 220, 60));
            window.draw(c);
        }

        Text t(hintMsg, font, 20);
        float w = t.getLocalBounds().width + 30;

        RectangleShape box(Vector2f(w, 40));
        box.setPosition(265, 655);
        box.setFillColor(Color(0, 0, 0, 200));
        box.setOutlineThickness(3);
        box.setOutlineColor(Color(212, 110, 194));
        window.draw(box);

        drawText(window, font, hintMsg, 20, 280, 662, Color(255, 220, 60), false);
    }

    if (level.getState() == act4Level4::WON)
    {
        drawText(window, font,
            "HEAP SORT COMPLETE!  STARS: " + to_string(level.stars()) + "  SCORE: " + to_string(level.score()) + "  - PRESS ENTER",
            26, 640, 680, Color::Green, true);
    }
    else if (level.getState() == act4Level4::LOST)
    {
        drawText(window, font, "ALARM TRIGGERED! YOU LOST!   (R: retry)",
            27, 640, 680, Color::Red, true);
    }

    if (flashTime > 0)
    {
        RectangleShape redScreen(Vector2f(1280, 720));
        redScreen.setFillColor(Color(255, 0, 0, 90));
        window.draw(redScreen);
    }
}

//-----------level 5


Act4Level5Screen::Act4Level5Screen(RenderWindow& win) : Screen(win)
{
    loadAct4Font(font);
    if (backgroundTexture.loadFromFile("assets/pictures/act4l5.jfif"))
    {
        background.setTexture(backgroundTexture);
        background.setScale(1280.0f / backgroundTexture.getSize().x, 720.0f / backgroundTexture.getSize().y);
    }

    unlockButton = makeRoundedRect(240, 50, 12);
    unlockButton.setPosition(520, 650);
    unlockButton.setFillColor(Color(212, 110, 194, 120));

    unlockText.setFont(font);
    unlockText.setCharacterSize(20);
    unlockText.setFillColor(Color::Black);
    unlockText.setString("UNLOCK VAULT");
    centerText(unlockText, unlockButton);

    nextScreen = 21;
    seed = (unsigned)time(NULL) + 307;
    selectedNode = -1;
    flashTime = 0;
    animTime = 0;
    messageColor = Color::White;

    for (int i = 0; i < 15; i++)
    {
        nodeX[i] = 0;
        nodeY[i] = 0;
    }
}

void Act4Level5Screen::doUnlock()
{
    if (level.getState() != act4Level5::PLAYING)
        return;

    int prevState = level.getState();
    if (level.checkVault())
    {
        message = "FINAL VAULT UNLOCKED! ACT 4 COMPLETE!";
        messageColor = Color(120, 255, 120);
    }
    else
    {
        flashTime = 0.35f;
        if (level.getState() == act4Level5::LOST && prevState == act4Level5::PLAYING)
            message = "VAULT LOCKED & OUT OF MOVES! Alarm filled.";
        else
            message = "VAULT SEALED! Tree is not sorted in ascending order yet (+1 alarm).";
        messageColor = Color(255, 110, 110);
    }
}

void Act4Level5Screen::newPuzzle()
{
    srand(seed++);
    level.start();

    selectedNode = -1;
    flashTime = 0;
    message = "FINAL VAULT BOSS: Build a Max Heap, then sort the tree in ascending order independently!";
    messageColor = Color::White;

    for (int i = 0; i < level.getN(); i++)
    {
        int depth = (int)log2(i + 1);
        int posInRow = i - ((1 << depth) - 1);
        int slotsInRow = 1 << depth;

        nodeX[i] = 1280.0f * (posInRow + 1) / (slotsInRow + 1);
        nodeY[i] = 235.0f + depth * 120.0f;
    }
}

void Act4Level5Screen::onEnter()
{
    isActive = true;
    nextScreen = 21;
    newPuzzle();
}

void Act4Level5Screen::onResume()
{
    isActive = true;
    nextScreen = 21;
    flashTime = 0;
}

void Act4Level5Screen::handleInput()
{
    Event e;

    while (window.pollEvent(e))
    {
        if (e.type == Event::Closed)
            nextScreen = 99;

        if (e.type == Event::MouseButtonPressed && e.mouseButton.button == Mouse::Left)
        {
            Vector2f m = window.mapPixelToCoords(Vector2i(e.mouseButton.x, e.mouseButton.y));

            if (unlockButton.getGlobalBounds().contains(m))
            {
                doUnlock();
                continue;
            }

            if (level.getState() != act4Level5::PLAYING)
                continue;

            int clicked = -1;
            for (int i = 0; i < level.getN(); i++)
            {
                float dx = m.x - nodeX[i];
                float dy = m.y - nodeY[i];
                if (dx * dx + dy * dy <= 30.0f * 30.0f)
                {
                    clicked = i;
                    break;
                }
            }

            if (clicked != -1)
            {
                if (selectedNode == -1)
                {
                    selectedNode = clicked;
                }
                else if (selectedNode == clicked)
                {
                    selectedNode = -1;
                }
                else
                {
                    int prevState = level.getState();
                    bool wasHeapBuilt = level.isHeapBuilt();
                    level.swapNodes(selectedNode, clicked);
                    selectedNode = -1;

                    if (level.getState() == act4Level5::WON)
                    {
                        message = "FINAL VAULT UNLOCKED! ACT 4 COMPLETE!";
                        messageColor = Color(120, 255, 120);
                    }
                    else if (level.getState() == act4Level5::LOST && prevState == act4Level5::PLAYING)
                    {
                        flashTime = 0.35f;
                        message = "OUT OF MOVES! Vault alarm triggered.";
                        messageColor = Color(255, 110, 110);
                    }
                    else if (!wasHeapBuilt && level.isHeapBuilt())
                    {
                        message = "Stage 1 Complete: Max Heap built! Now sort the tree in ascending order.";
                        messageColor = Color(255, 220, 120);
                    }
                }
            }
            else
            {
                selectedNode = -1;
            }
        }

        if (e.type == Event::KeyPressed)
        {
            if (e.key.code == Keyboard::Escape)
                nextScreen = 25;

            else if (e.key.code == Keyboard::R)
                newPuzzle();

            else if (e.key.code == Keyboard::N)
                nextScreen = 1;

            else if (e.key.code == Keyboard::Space)
            {
                level.resetPuzzle();
                selectedNode = -1;
                message = "Vault reset! Build a Max Heap, then sort the tree in ascending order.";
                messageColor = Color::White;
            }

            else if (e.key.code == Keyboard::Return)
            {
                if (level.getState() == act4Level5::WON)
                    nextScreen = 1;
                else if (level.getState() == act4Level5::PLAYING)
                    doUnlock();
            }
        }
    }
}

void Act4Level5Screen::update(float dt)
{
    animTime += dt;
    if (flashTime > 0)
        flashTime -= dt;

    Vector2f mouse = window.mapPixelToCoords(Mouse::getPosition(window));
    if (level.getState() == act4Level5::PLAYING)
        applyHover(unlockButton, mouse);
}

void Act4Level5Screen::draw()
{
    window.clear(ACT4_FALLBACK_BG);
    window.draw(background);

    int n = level.getN();
    string phaseTag = level.isHeapBuilt() ? "HEAP READY" : "UNSORTED";

    drawAct4Hud(window, font, 5, level.getAlarm(), level.getCapacity(),
        "VAULT: " + phaseTag);

    if (level.getState() == act4Level5::PLAYING)
    {
        window.draw(unlockButton);
        window.draw(unlockText);
    }

    drawText(window, font, message, 19, 640, 140, messageColor, true);
    drawText(window, font,
        "No Hints! Swap two rooms to perform full Heap Sort (Root = Smallest -> Leaves = Largest) | Space: Reset | R: New",
        14, 640, 166, Color(208, 221, 183), true);

    // tree edges
    for (int i = 0; i < n; i++)
    {
        int left = 2 * i + 1;
        int right = 2 * i + 2;

        if (left < n)
            drawLine(window, Vector2f(nodeX[i], nodeY[i]), Vector2f(nodeX[left], nodeY[left]), 5, Color(208, 221, 183));
        if (right < n)
            drawLine(window, Vector2f(nodeX[i], nodeY[i]), Vector2f(nodeX[right], nodeY[right]), 5, Color(208, 221, 183));
    }

    // tree nodes
    for (int i = 0; i < n; i++)
    {
        float radius = 30.0f;
        CircleShape circle(radius);
        circle.setOrigin(radius, radius);
        circle.setPosition(nodeX[i], nodeY[i]);

        if (level.getState() == act4Level5::WON)
            circle.setFillColor(Color(45, 115, 75));
        else
            circle.setFillColor(Color(73, 103, 164));

        circle.setOutlineThickness(7);

        if (level.getState() == act4Level5::WON)
            circle.setOutlineColor(Color::Green);
        else if (i == selectedNode)
            circle.setOutlineColor(Color(212, 110, 194));
        else
            circle.setOutlineColor(Color(208, 221, 183));

        window.draw(circle);

        drawText(window, font, to_string(level.getVal(i)),
            26, nodeX[i], nodeY[i], Color::White, true);
    }

    if (level.getState() == act4Level5::WON)
    {
        drawText(window, font,
            "FINAL VAULT UNLOCKED!  STARS: " + to_string(level.stars()) + "  SCORE: " + to_string(level.score()) + "  - PRESS ENTER",
            26, 640, 680, Color::Green, true);
    }
    else if (level.getState() == act4Level5::LOST)
    {
        drawText(window, font, "VAULT ALARM TRIGGERED! YOU LOST!   (R: retry)",
            27, 640, 680, Color::Red, true);
    }

    if (flashTime > 0)
    {
        RectangleShape redScreen(Vector2f(1280, 720));
        redScreen.setFillColor(Color(255, 0, 0, 90));
        window.draw(redScreen);
    }
}