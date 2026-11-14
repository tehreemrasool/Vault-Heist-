#include "screen.h"

using namespace std;

// ---------------- shared helpers ----------------
bool loadFont(Font& f)
{
    if (f.loadFromFile("assets/Fonts/SuperShiny.ttf")) return true;
    return false;
}

void drawText(sf::RenderWindow& w, const sf::Font& f, const std::string& s, unsigned size,
    float x, float y, sf::Color c, bool center)
{
    Text t(s, f, size);
    t.setFillColor(c);
    FloatRect b = t.getLocalBounds();
    if (center) t.setOrigin(b.left + b.width / 2, b.top + b.height / 2);
    t.setPosition(x, y);
    w.draw(t);
}

bool isMouseOver(sf::RenderWindow& w, float x, float y, float width, float height)
{
    Vector2i mp = sf::Mouse::getPosition(w);
    return mp.x >= x && mp.x <= x + width && mp.y >= y && mp.y <= y + height;
}

void drawRoundButton(sf::RenderWindow& w, float x, float y, float width, float height,Color c, bool hover)
{
    if (hover)
    {
        int r = c.r + 30, g = c.g + 30, b = c.b + 30;
        c = Color((sf::Uint8)(r > 255 ? 255 : r), (Uint8)(g > 255 ? 255 : g),
            (Uint8)(b > 255 ? 255 : b));
    }
    RectangleShape box(sf::Vector2f(width, height));
    box.setPosition(x, y);
    box.setFillColor(c);
    box.setOutlineThickness(3.f);
    box.setOutlineColor(Color(200, 165, 70));
    w.draw(box);
}

static float slotY(int slot)
{
    return 260.f + slot * 70.f;
}

static void drawVaultBackground(RenderWindow& w)
{
    w.clear(Color(22, 24, 34));

    CircleShape dial(260.f);
    dial.setOrigin(260.f, 260.f);
    dial.setPosition(640.f, 380.f);
    dial.setFillColor(Color(30, 33, 46));
    dial.setOutlineThickness(8.f);
    dial.setOutlineColor(Color(48, 52, 70));
    w.draw(dial);

    for (int i = 0; i < 8; i++)
    {
        RectangleShape spoke(Vector2f(240.f, 6.f));
        spoke.setOrigin(0.f, 3.f);
        spoke.setPosition(640.f, 380.f);
        spoke.setRotation(i * 45.f);
        spoke.setFillColor(Color(40, 44, 60));
        w.draw(spoke);
    }

    sf::RectangleShape frame(Vector2f(1240.f, 680.f));
    frame.setPosition(20.f, 20.f);
    frame.setFillColor(sf::Color::Transparent);
    frame.setOutlineThickness(4.f);
    frame.setOutlineColor(sf::Color(200, 165, 70));
    w.draw(frame);
}

static const char* typeName(int t)
{
    if (t == act1Level1::PREORDER)  return "PREORDER (root, left, right)";
    if (t == act1Level1::INORDER)   return "INORDER (left, root, right)";
    if (t == act1Level1::POSTORDER) return "POSTORDER (left, right, root)";
    return "LEVEL ORDER (top to bottom, left to right)";
}

// Find the maximum depth of the tree
int getMaxDepth(TreeNode* root)
{
    if (root == NULL)
        return 0;

    int left = getMaxDepth(root->left);
    int right = getMaxDepth(root->right);

    return 1 + (left > right ? left : right);
}


// Give every node an x and y position
void setTreePositions(TreeNode* root, int depth, int& position,
    int total, int maxDepth)
{
    if (root == NULL)
        return;

    setTreePositions(root->left, depth + 1, position, total, maxDepth);

    root->x = 100 + position * (1080.0f / (total > 1 ? total - 1 : 1));
    root->y = 175 + depth * (430.0f / (maxDepth > 1 ? maxDepth - 1 : 1));

    position++;

    setTreePositions(root->right, depth + 1, position, total, maxDepth);
}


// Draw lines connecting parent and child nodes
void drawTreeLines(RenderWindow& window, TreeNode* root)
{
    if (root == NULL)
        return;

    if (root->left != NULL)
    {
        Vector2f start(root->x, root->y);
        Vector2f end(root->left->x, root->left->y);

        Vector2f difference = end - start;

        RectangleShape line(Vector2f(sqrt(difference.x * difference.x +
            difference.y * difference.y), 5));

        line.setPosition(start);
        line.setRotation(atan2(difference.y, difference.x) * 180 / 3.14159f);
        line.setFillColor(Color(208, 221, 183));

        window.draw(line);
    }

    if (root->right != NULL)
    {
        Vector2f start(root->x, root->y);
        Vector2f end(root->right->x, root->right->y);

        Vector2f difference = end - start;

        RectangleShape line(Vector2f(sqrt(difference.x * difference.x +
            difference.y * difference.y), 5));

        line.setPosition(start);
        line.setRotation(atan2(difference.y, difference.x) * 180 / 3.14159f);
        line.setFillColor(Color(208, 221, 183));

        window.draw(line);
    }

    drawTreeLines(window, root->left);
    drawTreeLines(window, root->right);
}


void drawTreeNodes(RenderWindow& window, Font& font,
    TreeNode* root, act1Level1& level)
{
    if (root == NULL)
        return;

    float radius = 30;

    CircleShape circle(radius);

    circle.setOrigin(radius, radius);
    circle.setPosition(root->x, root->y);
    circle.setFillColor(Color(73,103,164));
    circle.setOutlineThickness(7);

    if (level.orderOf(root->id) > 0)
        circle.setOutlineColor(Color::Green);
    else
        circle.setOutlineColor(Color(208, 221, 183));

    window.draw(circle);

    drawText(window, font, to_string(root->value),
        26, root->x, root->y, Color(255, 255,255), true);

    drawTreeNodes(window, font, root->left, level);
    drawTreeNodes(window, font, root->right, level);
}


// Find which node the player clicked
TreeNode* findClickedNode(TreeNode* root, float x, float y)
{
    if (root == NULL)
        return NULL;

    float dx = x - root->x;
    float dy = y - root->y;

    if (dx * dx + dy * dy <= 22 * 22)
        return root;

    TreeNode* found = findClickedNode(root->left, x, y);

    if (found != NULL)
        return found;

    return findClickedNode(root->right, x, y);
}


// ---------------- MainMenuScreen ----------------
MainMenuScreen::MainMenuScreen(RenderWindow& win) : Screen(win), button(35, 100), nextScreen(1)
{
    font.loadFromFile("assets/fonts/SuperShiny.ttf");

    title.setFont(font);
    title.setString("Vault heist");
    title.setCharacterSize(100);
    title.setFillColor(Color::White);
    title.setOutlineColor(Color::Black);
    title.setOutlineThickness(3);
    title.setPosition(400, 100);

    button.setPosition(500, 350);
    button.setScale(3.5f, 1.0f);
    button.setFillColor(Color(248, 117, 170));

    buttonText.setFont(font);
    buttonText.setString("START GAME");
    buttonText.setCharacterSize(25);
    buttonText.setFillColor(Color::Black);
    buttonText.setPosition(570, 370);

    backgroundTexture.loadFromFile("assets/pictures/mainbg.jfif");
    background.setTexture(backgroundTexture);
    background.setScale(1280.0f / backgroundTexture.getSize().x,
        720.0f / backgroundTexture.getSize().y);
}

void MainMenuScreen::onEnter()
{
    isActive = true;
    nextScreen = 1;     // stay on the menu until the button is clicked
}

void MainMenuScreen::handleInput()
{
    Event e;
    while (window.pollEvent(e))
    {
        if (e.type == Event::Closed) nextScreen = 99;

        if (e.type == Event::MouseButtonPressed && e.mouseButton.button == Mouse::Left)
        {
            if (button.getGlobalBounds().contains((float)e.mouseButton.x, (float)e.mouseButton.y))
                nextScreen = 24;     // start Act 1 Level 1
        }
    }
}

void MainMenuScreen::update(float time) {}

void MainMenuScreen::draw()
{
    window.clear();
    window.draw(background);
    window.draw(title);
    window.draw(button);
    window.draw(buttonText);
}



// ---------------- Level1 Screen ----------------

Level1Screen::Level1Screen(RenderWindow& win) : Screen(win)
{
    loadFont(font);

    if (backgroundTexture.loadFromFile("assets/pictures/act1level1.jfif"))
    {
        background.setTexture(backgroundTexture);

        background.setScale(1280.0f / backgroundTexture.getSize().x,720.0f / backgroundTexture.getSize().y
        );
    }

    nextScreen = 2;
    root = NULL;
    count = 0;

    curType = act1Level1::PREORDER;

    seed = (unsigned)time(NULL);

    flashTime = 0;
}


Level1Screen::~Level1Screen()
{
    deleteTree(root);
}


// Generate a new random tree
void Level1Screen::newTree()
{
    deleteTree(root);

    srand(seed++);

    count = 6 + rand() % 10;

    root = constructBinaryTree(count);

    int position = 0;
    int maxDepth = getMaxDepth(root);

    setTreePositions(root, 0, position, count, maxDepth);

    level.start(root, curType);
}


// Called when Level 1 opens
void Level1Screen::onEnter()
{
    isActive = true;
    nextScreen = 2;

    flashTime = 0;

    newTree();
}


// Handle mouse and keyboard
void Level1Screen::handleInput()
{
    Event e;

    while (window.pollEvent(e))
    {
        if (e.type == Event::Closed)
            nextScreen = 99;


        // MOUSE CLICK
        if (e.type == Event::MouseButtonPressed &&
            e.mouseButton.button == Mouse::Left)
        {
            float x = (float)e.mouseButton.x;
            float y = (float)e.mouseButton.y;

            TreeNode* clicked = findClickedNode(root, x, y);

            if (clicked != NULL)
            {
                int result = level.checkClick(clicked->id);

                if (result == act1Level1::WRONG)
                    flashTime = 0.35f;
            }
        }


        // KEYBOARD
        if (e.type == Event::KeyPressed)
        {
            if (e.key.code == Keyboard::Escape)
                nextScreen = 1;

            else if (e.key.code == Keyboard::Return &&
                level.getState() == act1Level1::WON)
                nextScreen = 3;

            else if (e.key.code == Keyboard::R)
                newTree();

            else if (e.key.code == Keyboard::Space)
                level.start(root, curType);

            else if (e.key.code == Keyboard::Num1)
            {
                curType = act1Level1::PREORDER;
                level.start(root, curType);
            }

            else if (e.key.code == Keyboard::Num2)
            {
                curType = act1Level1::INORDER;
                level.start(root, curType);
            }

            else if (e.key.code == Keyboard::Num3)
            {
                curType = act1Level1::POSTORDER;
                level.start(root, curType);
            }

            else if (e.key.code == Keyboard::Num4)
            {
                curType = act1Level1::LEVELORDER;
                level.start(root, curType);
            }
        }
    }
}


// Update flash timer
void Level1Screen::update(float dt)
{
    if (flashTime > 0)
        flashTime -= dt;
}


// Draw Level 1
void Level1Screen::draw()
{
    window.clear();

    // BACKGROUND
    window.draw(background);


    // TITLE
    drawText(window, font, "ACT 1 - LEVEL 1",40, 500, 15, Color::White, false);


    // ALARM
    RectangleShape alarmBox(Vector2f(200, 50));
    RectangleShape orderBox(Vector2f(420, 50));

    alarmBox.setPosition(50, 70);
    orderBox.setPosition(850, 70);
    alarmBox.setFillColor(Color::Black);
    orderBox.setFillColor(Color::Black);
    alarmBox.setOutlineColor(Color(180, 100, 220));
    alarmBox.setOutlineThickness(3);
    orderBox.setOutlineColor(Color(180, 100, 220));
    orderBox.setOutlineThickness(3);

    window.draw(alarmBox);
    window.draw(orderBox);

    drawText(window, font,"ALARM: " + to_string(level.getAlarm()) + " / " + to_string(level.getCapacity()),
        18, 70, 80, Color(255, 255, 255), false);


    // TRAVERSAL ORDER
    drawText(window, font, typeName(level.getType()),
        22, 885, 75, Color::White, false);


    // INSTRUCTIONS
    drawText(window, font, "Which room will the guard visit next?",
        20,600 , 100, Color::White, true);


    // TREE LINES
    drawTreeLines(window, root);


    // TREE NODES
    drawTreeNodes(window, font, root, level);


    // GAME STATUS
    if (level.getState() == act1Level1::WON)
    {
        drawText(window, font, "VAULT CLEARED!",30, 640, 670, Color::Green, true);
    }

    else if (level.getState() == act1Level1::LOST)
    {
        drawText(window, font, "ALARM TRIGGERED! YOU LOST!",
            27, 640, 670, Color::Red, true);
    }

    


    // RED SCREEN FLASH AFTER WRONG CLICK
    if (flashTime > 0)
    {
        RectangleShape redScreen(Vector2f(1280, 720));

        redScreen.setFillColor(Color(255, 0, 0, 90));

        window.draw(redScreen);
    }
}


// ---------------- Level2Screen ----------------
static void drawLine(sf::RenderWindow& w, float x1, float y1, float x2, float y2,
    float thick, Color c)
{
    float dx = x2 - x1, dy = y2 - y1;
    float len = std::sqrt(dx * dx + dy * dy);
    if (len < 0.5f) return;
    sf::RectangleShape r(sf::Vector2f(len, thick));
    r.setOrigin(0.f, thick / 2.f);
    r.setPosition(x1, y1);
    r.setRotation(std::atan2(dy, dx) * 180.f / 3.14159265f);
    r.setFillColor(c);
    w.draw(r);
}

Level2Screen::Level2Screen(sf::RenderWindow& win) : Screen(win)
{
    loadFont(font);
    nextScreen = 3;
    radius = 24;
    portR = 9;
    mode = act1Level2::INPRE;
    drag = -1;
    portFrom = -1;
    portSide = 0;
    offX = offY = 0;
    msgTime = 0;
    seed = (unsigned)time(NULL) + 7;
    for (int i = 0; i < 15; i++)
        px[i] = py[i] = homeX[i] = homeY[i] = 0;
}

void Level2Screen::setMsg(const std::string& s)
{
    message = s;
    msgTime = 2.5f;
}

void Level2Screen::newPuzzle(int m)
{
    mode = m;
    srand(seed++);
    level.start(mode, 5 + rand() % 5);      // 5..9 rooms
    drag = -1;
    portFrom = -1;
    msgTime = 0;
    placePieces();
}

void Level2Screen::placePieces()
{
    int n = level.getN();
    float spacing = 1000.0f / (n > 1 ? n - 1 : 1);
    if (spacing > 110) spacing = 110;
    float x0 = W / 2.0f - spacing * (n - 1) / 2.0f;
    for (int i = 0; i < n; i++)
    {
        homeX[i] = x0 + i * spacing;
        homeY[i] = H - 60.0f;
        px[i] = homeX[i];
        py[i] = homeY[i];
    }
}

void Level2Screen::onEnter()
{
    isActive = true;
    nextScreen = 3;
    newPuzzle(rand() % 2);
}

void Level2Screen::moveSubtree(int i, float dx, float dy)
{
    if (i < 0) return;
    px[i] += dx;
    py[i] += dy;
    moveSubtree(level.getChild(i, 0), dx, dy);
    moveSubtree(level.getChild(i, 1), dx, dy);
}

int Level2Screen::hitNode(float mx, float my) const
{
    for (int i = level.getN() - 1; i >= 0; i--)      // topmost first
    {
        float dx = mx - px[i], dy = my - py[i];
        if (dx * dx + dy * dy <= radius * radius) return i;
    }
    return -1;
}

void Level2Screen::portPos(int i, int side, float& x, float& y) const
{
    x = px[i] + (side == 0 ? -1.0f : 1.0f) * radius * 0.75f;
    y = py[i] + radius * 0.75f;
}

bool Level2Screen::hitPort(float mx, float my, int& piece, int& side) const
{
    for (int i = level.getN() - 1; i >= 0; i--)
    {
        for (int s = 0; s < 2; s++)
        {
            float x, y;
            portPos(i, s, x, y);
            float dx = mx - x, dy = my - y;
            if (dx * dx + dy * dy <= portR * portR)
            {
                piece = i; side = s;
                return true;
            }
        }
    }
    return false;
}

void Level2Screen::doCheck()
{
    if (level.getState() == act1Level2::WON) return;
    if (level.linkCount() < level.getN() - 1)
        setMsg("Link all the rooms into one tree first.");
    else if (!level.check())
        setMsg("Not quite - that tree doesn't match both sequences.");
}

void Level2Screen::handleInput()
{
    sf::Event e;
    while (window.pollEvent(e))
    {
        if (e.type == sf::Event::Closed) nextScreen = 99;

        if (e.type == sf::Event::MouseButtonPressed)
        {
            float mx = (float)e.mouseButton.x, my = (float)e.mouseButton.y;

            if (e.mouseButton.button == sf::Mouse::Left)
            {
                if (isMouseOver(window, 1040, 14, 200, 40)) doCheck();
                else if (isMouseOver(window, 1040, 62, 200, 40)) newPuzzle(mode);
                else
                {
                    int p, s;
                    if (hitPort(mx, my, p, s)) { portFrom = p; portSide = s; }
                    else
                    {
                        int h = hitNode(mx, my);
                        if (h != -1) { drag = h; offX = mx - px[h]; offY = my - py[h]; }
                    }
                }
            }
            else if (e.mouseButton.button == sf::Mouse::Right)
            {
                int h = hitNode(mx, my);
                if (h != -1) level.detach(h);          // cut from its parent
            }
        }

        if (e.type == sf::Event::MouseButtonReleased && e.mouseButton.button == sf::Mouse::Left)
        {
            if (portFrom != -1)
            {
                int t = hitNode((float)e.mouseButton.x, (float)e.mouseButton.y);
                if (t != -1 && t != portFrom)
                {
                    if (!level.connect(portFrom, portSide, t))
                        setMsg("That link isn't allowed (it would make a loop).");
                }
                portFrom = -1;
            }
            drag = -1;
        }

        if (e.type == sf::Event::KeyPressed)
        {
            if (e.key.code == sf::Keyboard::Escape) nextScreen = 1;
            else if (e.key.code == sf::Keyboard::Return) doCheck();
            else if (e.key.code == sf::Keyboard::N) newPuzzle(mode);
            else if (e.key.code == sf::Keyboard::Num1) newPuzzle(act1Level2::INPRE);
            else if (e.key.code == sf::Keyboard::Num2) newPuzzle(act1Level2::INPOST);
            else if (e.key.code == sf::Keyboard::Space)
            {
                level.clearLinks();
                if (level.getState() == act1Level2::PLAYING) placePieces();
            }
        }
    }
}

void Level2Screen::update(float dt)
{
    if (msgTime > 0) msgTime -= dt;

    if (drag != -1)
    {
        sf::Vector2i mp = sf::Mouse::getPosition(window);
        float tx = mp.x - offX, ty = mp.y - offY;
        if (tx < radius) tx = radius;
        if (tx > W - radius) tx = W - radius;
        if (ty < 135 + radius) ty = 135 + radius;
        if (ty > H - radius) ty = H - radius;
        moveSubtree(drag, tx - px[drag], ty - py[drag]);     // children follow
    }
}

void Level2Screen::draw()
{
    int n = level.getN();
    bool won = (level.getState() == act1Level2::WON);
    sf::Vector2i mp = sf::Mouse::getPosition(window);

    window.clear(sf::Color(24, 26, 38));

    // top panel
    sf::RectangleShape top(sf::Vector2f((float)W, 135));
    top.setFillColor(sf::Color(32, 35, 52));
    window.draw(top);
    drawLine(window, 0, 135, (float)W, 135, 3, sf::Color(200, 165, 70));

    // bottom tray
    sf::RectangleShape tray(sf::Vector2f((float)W, (float)(H - 585)));
    tray.setPosition(0, 585);
    tray.setFillColor(sf::Color(34, 37, 54));
    window.draw(tray);
    drawLine(window, 0, 585, (float)W, 585, 3, sf::Color(200, 165, 70));
    drawText(window, font, "Rooms - drag them up into the map", 14, 20, 592,
        sf::Color(150, 155, 180), false);

    // sequences
    drawText(window, font, "Act 1 - Level 2: Rebuild the Vault Map", 24, 40, 12,
        sf::Color(255, 215, 90), false);
    std::string s1 = "Inorder:      ";
    std::string s2 = (level.getMode() == act1Level2::INPRE) ? "Preorder:     " : "Postorder:   ";
    for (int i = 0; i < n; i++)
    {
        s1 += std::to_string(level.getInSeq(i)) + "   ";
        s2 += std::to_string(level.getOtherSeq(i)) + "   ";
    }
    drawText(window, font, s1, 22, 40, 50, sf::Color::White, false);
    drawText(window, font, s2, 22, 40, 80, sf::Color::White, false);
    drawText(window, font, "Drag a room to move it (its subtree follows). Drag its L / R dot onto another room to make that room its left / right child.",
        13, 40, 112, sf::Color(170, 175, 200), false);

    // buttons
    sf::Color navy(20, 30, 70);
    bool hov = isMouseOver(window, 1040, 14, 200, 40);
    drawRoundButton(window, 1040, 14, 200, 40, sf::Color(120, 220, 140), hov);
    drawText(window, font, "Check", 1140, 34, 20, navy, true);
    hov = isMouseOver(window, 1040, 62, 200, 40);
    drawRoundButton(window, 1040, 62, 200, 40, sf::Color(130, 170, 240), hov);
    drawText(window, font, "New Puzzle", 1140, 82, 20, navy, true);

    // links
    sf::Color lineCol = won ? sf::Color(120, 230, 150) : sf::Color(170, 175, 205);
    for (int i = 0; i < n; i++)
    {
        for (int s = 0; s < 2; s++)
        {
            int c = level.getChild(i, s);
            if (c == -1) continue;
            float x, y;
            portPos(i, s, x, y);
            drawLine(window, x, y, px[c], py[c] - radius, 3, lineCol);
        }
    }

    // link being dragged
    if (portFrom != -1)
    {
        float x, y;
        portPos(portFrom, portSide, x, y);
        drawLine(window, x, y, (float)mp.x, (float)mp.y, 2, sf::Color(255, 215, 90));
    }

    // rooms
    for (int i = 0; i < n; i++)
    {
        sf::CircleShape c(radius);
        c.setOrigin(radius, radius);
        c.setPosition(px[i], py[i]);
        c.setOutlineThickness(3);

        sf::Color fill(70, 90, 140), outline(150, 170, 230);
        if (won) { fill = sf::Color(40, 150, 80); outline = sf::Color(120, 230, 150); }
        else if (i == drag) { fill = sf::Color(110, 130, 200); outline = sf::Color::White; }
        c.setFillColor(fill);
        c.setOutlineColor(outline);
        window.draw(c);

        drawText(window, font, std::to_string(level.getVal(i)), 20, px[i], py[i] - 6,
            sf::Color::White, true);

        // L / R ports
        for (int s = 0; s < 2; s++)
        {
            float x, y;
            portPos(i, s, x, y);
            sf::CircleShape p(portR);
            p.setOrigin(portR, portR);
            p.setPosition(x, y);
            bool used = (level.getChild(i, s) != -1);
            if (s == 0) p.setFillColor(used ? sf::Color(255, 190, 70) : sf::Color(150, 110, 40));
            else        p.setFillColor(used ? sf::Color(100, 215, 245) : sf::Color(50, 110, 130));
            p.setOutlineThickness(1);
            p.setOutlineColor(sf::Color::White);
            window.draw(p);
            drawText(window, font, s == 0 ? "L" : "R", 11, x, y - 2, sf::Color::Black, true);
        }
    }

    // banner / message
    if (won)
    {
        std::string msg = "VAULT MAP RESTORED!  Stars: " + std::to_string(level.stars()) +
            "/3   Score: " + std::to_string(level.score());
        drawText(window, font, msg, 22, W / 2.0f, 555, sf::Color(255, 215, 90), true);
        drawText(window, font, "N: new puzzle  |  Esc: menu", 14, W / 2.0f, 575,
            sf::Color(170, 175, 200), true);
    }
    else if (msgTime > 0)
        drawText(window, font, message, 20, W / 2.0f, 555, sf::Color(255, 120, 120), true);

    drawText(window, font, "Drag room: move | Drag L/R dot onto a room: link | Right click: cut link | Enter: check | Space: reset | 1/2: pre or post | N: new | Esc: menu",
        13, W / 2.0f, H - 14.0f, sf::Color(150, 155, 180), true);
}



Map::Map(RenderWindow& window) : Screen(window)
{
    font.loadFromFile("assets/fonts/SuperShiny.ttf");
    if (backgroundTexture.loadFromFile("assets/pictures/map.png"))
    {
        background.setTexture(backgroundTexture);
        background.setScale(1280.0f / backgroundTexture.getSize().x, 720.0f / backgroundTexture.getSize().y
        );
    }

    nextScreen = 24;

}

void Map::onEnter()
{
    isActive = true;
    nextScreen = 24;     
}

void Map::handleInput()
{
    Event e;
    while (window.pollEvent(e))
    {
        if (e.type == Event::Closed) nextScreen = 99;

        if (e.type == Event::MouseButtonPressed && e.mouseButton.button == Mouse::Left)
        {
                nextScreen = 2;     // start Act 1 Level 1
        }
    }
}
void Map::update(float time) {}

void Map::draw()
{
    window.clear();
    window.draw(background);
}