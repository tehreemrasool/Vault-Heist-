#include "screen.h"
using namespace sf;
using namespace std;

// ---------------- shared helpers ----------------
bool loadFont(Font& f)
{
    if (f.loadFromFile("assets/Fonts/SuperShiny.ttf")) return true;
    return false;
}

void drawText(RenderWindow& w, const Font& f, const string& s, unsigned size,float x, float y, Color c, bool center)
{
    Text t(s, f, size);
    t.setFillColor(c);
    FloatRect b = t.getLocalBounds();
    if (center) t.setOrigin(b.left + b.width / 2, b.top + b.height / 2);
    t.setPosition(x, y);
    w.draw(t);
}

bool isMouseOver(RenderWindow& w, float x, float y, float width, float height)
{
    Vector2i mp = sf::Mouse::getPosition(w);
    return mp.x >= x && mp.x <= x + width && mp.y >= y && mp.y <= y + height;
}

void drawRoundButton(RenderWindow& w, float x, float y, float width, float height,Color c, bool hover)
{
    if (hover)
    {
        int r = c.r + 30, g = c.g + 30, b = c.b + 30;
        c = Color((sf::Uint8)(r > 255 ? 255 : r), (Uint8)(g > 255 ? 255 : g),
            (Uint8)(b > 255 ? 255 : b));
    }
    RectangleShape box(Vector2f(width, height));
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

int getMaxDepth(TreeNode* root)
{
    if (root == NULL)
        return 0;

    int left = getMaxDepth(root->left);
    int right = getMaxDepth(root->right);

    return 1 + (left > right ? left : right);
}


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
    font2.loadFromFile("assets/fonts/MedievalSharp.ttf");
    
    title.setFont(font);

    storyLine.setFont(font2);
    storyLine.setString("Legends speak of an ancient vault hidden beneath a forgotten kingdom.\nIt holds treasures beyond imagination,\nguarded by puzzles no one has ever solved.\nCan you find it ? ");
    storyLine.setFillColor(Color(212, 110, 194));
    storyLine.setCharacterSize(25);
    storyLine.setPosition(300, 250);
    storyLine.setOutlineColor(Color::Black);
    storyLine.setOutlineThickness(3);


    title.setString("Vault heist");
    title.setCharacterSize(100);
    title.setFillColor(Color::White);
    title.setOutlineColor(Color::Black);
    title.setOutlineThickness(1);
    title.setPosition(400, 100);

    button.setPosition(500, 350);
    button.setScale(3.0f, 1.0f);
    button.setFillColor(Color(212, 110, 194, 120));
    instructionButton.setFillColor(Color(212, 110, 194, 120));
    instructionButton.setScale(3.0f, 1.0f);
    instructionButton.setPosition(500, 400);


    buttontext2.setFont(font);
    buttontext2.setFillColor(Color::Black);
    buttontext2.setCharacterSize(25);
    buttontext2.setPosition(550, 450);
    buttontext2.setString("Instructions");

    buttonText.setFont(font);
    buttonText.setString("START GAME");
    buttonText.setCharacterSize(25);
    buttonText.setFillColor(Color::Black);
    buttonText.setPosition(550, 363);

    backgroundTexture.loadFromFile("assets/pictures/Hmain.jfif");
    background.setTexture(backgroundTexture);
    background.setScale(1280.0f / backgroundTexture.getSize().x,
        720.0f / backgroundTexture.getSize().y);
}

void MainMenuScreen::onEnter()
{
    isActive = true;
    nextScreen = 1;     
    levelNumber = 0;
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
                nextScreen = 24;     //show map
            levelNumber = 1;
        }
    }
}

void MainMenuScreen::update(float time) {}

void MainMenuScreen::draw()
{
    window.clear();
    window.draw(background);
    window.draw(storyLine);
    window.draw(title);
    window.draw(button);
    window.draw(buttonText);
    window.draw(instructionButton);
    window.draw(buttontext2);
}



// ---------------- Level1 Screen ----------------

Level1Screen::Level1Screen(RenderWindow& win) : Screen(win)
{
    loadFont(font);

    if (backgroundTexture.loadFromFile("assets/pictures/act1lvl1.jfif"))
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


        if (e.type == Event::KeyPressed)
        {
            if (e.key.code == Keyboard::Escape)
                nextScreen = 25;

            else if (e.key.code == Keyboard::Return &&
                level.getState() == act1Level1::WON)
            {
                nextScreen = 3;
                levelNumber = 2;

            }
                

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


void Level1Screen::update(float dt)
{
    if (flashTime > 0)
        flashTime -= dt;
}


void Level1Screen::draw()
{
    window.clear();
    window.draw(background);
    drawText(window, font, "ACT 1 - LEVEL 1",40, 500, 15, Color::White, false);

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


    drawText(window, font, typeName(level.getType()),
        22, 885, 75, Color::White, false);

    drawText(window, font, "Which room will the guard visit next?",
        20,600 , 100, Color::White, true);

    drawTreeLines(window, root);


    drawTreeNodes(window, font, root, level);


    if (level.getState() == act1Level1::WON)
    {
        drawText(window, font, "VAULT CLEARED!",30, 640, 670, Color::Green, true);
    }

    else if (level.getState() == act1Level1::LOST)
    {
        drawText(window, font, "ALARM TRIGGERED! YOU LOST!",
            27, 640, 670, Color::Red, true);
    }

        if (flashTime > 0)
    {
        RectangleShape redScreen(Vector2f(1280, 720));

        redScreen.setFillColor(Color(255, 0, 0, 90));

        window.draw(redScreen);
    }
}

void Level1Screen::onResume()
{
    isActive = true;
    nextScreen = 2;     
    flashTime = 0;
}



// ---------------- Level2Screen ----------------

Level2Screen::Level2Screen(RenderWindow& win) : Screen(win)
{
    loadFont(font);

    if (backgroundTexture.loadFromFile("assets/pictures/bglevel.jfif"))
    {
        background.setTexture(backgroundTexture);
        background.setScale(1280.0f / backgroundTexture.getSize().x,
            720.0f / backgroundTexture.getSize().y);
    }

    nextScreen = 3;
    selectedNode = -1;
    curMode = act1Level2::INPRE;
    seed = (unsigned)time(NULL) + 7;
    flashTime = 0;
    statusMsg = "";

    for (int i = 0; i < 15; i++)
    {
        nodeX[i] = 0;
        nodeY[i] = 0;
    }
}

void Level2Screen::newPuzzle()
{
    srand(seed++);
    int count = 5 + rand() % 4; 
    level.start(curMode, count);

    selectedNode = -1;
    flashTime = 0;
    statusMsg = "Click a parent room, then Left-Click (Left Child) or Right-Click (Right Child)";

    int cols = 4;
    for (int i = 0; i < count; i++)
    {
        int row = i / cols;
        int col = i % cols;
        nodeX[i] = 320.0f + col * 210.0f;
        nodeY[i] = 260.0f + row * 170.0f;
    }
}

void Level2Screen::onEnter()
{
    isActive = true;
    nextScreen = 3;
    newPuzzle();
}

void Level2Screen::handleInput()
{
    Event e;

    while (window.pollEvent(e))
    {
        if (e.type == Event::Closed)
            nextScreen = 99;

        if (e.type == Event::MouseButtonPressed)
        {
            float mx = (float)e.mouseButton.x;
            float my = (float)e.mouseButton.y;

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
                        ? act1Level2::LEFT
                        : act1Level2::RIGHT;

                    if (!level.connect(selectedNode, side, clicked))
                    {
                        flashTime = 0.35f;
                        statusMsg = "Cannot create a loop!";
                    }
                    selectedNode = -1;
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

            else if (e.key.code == Keyboard::Space)
            {
                level.clearLinks();
                selectedNode = -1;
            }

            else if (e.key.code == Keyboard::Num1)
            {
                curMode = act1Level2::INPRE;
                newPuzzle();
            }

            else if (e.key.code == Keyboard::Num2)
            {
                curMode = act1Level2::INPOST;
                newPuzzle();
            }

            else if (e.key.code == Keyboard::Return)
            {
                if (level.getState() == act1Level2::WON)
                {
                    nextScreen = 1; 
                }
                else if (level.linkCount() < level.getN() - 1)
                {
                    flashTime = 0.35f;
                    statusMsg = "Connect all rooms first!";
                }
                else if (!level.check())
                {
                    flashTime = 0.35f;
                    statusMsg = "Wrong tree! Try again.";
                }
            }
        }
    }
}

void Level2Screen::update(float dt)
{
    if (flashTime > 0)
        flashTime -= dt;
}

void Level2Screen::draw()
{
    window.clear();
    window.draw(background);

    int n = level.getN();

    drawText(window, font, "ACT 1 - LEVEL 2", 40, 500, 15, Color::White, false);

    RectangleShape seqBox1(Vector2f(420, 45));
    RectangleShape seqBox2(Vector2f(420, 45));

    seqBox1.setPosition(50, 70);
    seqBox2.setPosition(810, 70);
    seqBox1.setFillColor(Color::Black);
    seqBox2.setFillColor(Color::Black);
    seqBox1.setOutlineColor(Color(180, 100, 220));
    seqBox1.setOutlineThickness(3);
    seqBox2.setOutlineColor(Color(180, 100, 220));
    seqBox2.setOutlineThickness(3);

    window.draw(seqBox1);
    window.draw(seqBox2);

    string inStr = "INORDER: ";
    string otherStr = (level.getMode() == act1Level2::INPRE) ? "PREORDER: " : "POSTORDER: ";

    for (int i = 0; i < n; i++)
    {
        inStr += to_string(level.getInSeq(i)) + " ";
        otherStr += to_string(level.getOtherSeq(i)) + " ";
    }

    drawText(window, font, inStr, 20, 65, 80, Color::White, false);
    drawText(window, font, otherStr, 20, 825, 80, Color::White, false);

    drawText(window, font, statusMsg, 18, 640, 140, Color::White, true);
    drawText(window, font, "Click same room twice: Unlink | Space: Reset | Enter: Check | 1/2: Mode | R: New",
        16, 640, 170, Color(208, 221, 183), true);

    for (int i = 0; i < n; i++)
    {
        for (int side = 0; side < 2; side++)
        {
            int childIdx = level.getChild(i, side);
            if (childIdx == -1) continue;

            Vector2f start(nodeX[i], nodeY[i]);
            Vector2f end(nodeX[childIdx], nodeY[childIdx]);
            Vector2f diff = end - start;

            float length = sqrt(diff.x * diff.x + diff.y * diff.y);
            RectangleShape line(Vector2f(length, 5));
            line.setPosition(start);
            line.setRotation(atan2(diff.y, diff.x) * 180.0f / 3.14159f);

            line.setFillColor(side == act1Level2::LEFT ? Color::Yellow : Color::Cyan);
            window.draw(line);

            float midX = (start.x + end.x) / 2.0f;
            float midY = (start.y + end.y) / 2.0f;
            drawText(window, font, side == act1Level2::LEFT ? "L" : "R",
                18, midX, midY - 12, Color::White, true);
        }
    }

    for (int i = 0; i < n; i++)
    {
        float radius = 30.0f;
        CircleShape circle(radius);
        circle.setOrigin(radius, radius);
        circle.setPosition(nodeX[i], nodeY[i]);
        circle.setFillColor(Color(73, 103, 164));
        circle.setOutlineThickness(7);

        if (level.getState() == act1Level2::WON)
            circle.setOutlineColor(Color(212, 110, 194));
        else if (i == selectedNode)
            circle.setOutlineColor(Color(212, 110, 194));
        else
            circle.setOutlineColor(Color(208, 221, 183));

        window.draw(circle);

        drawText(window, font, to_string(level.getVal(i)),
            26, nodeX[i], nodeY[i], Color::White, true);
    }

    if (level.getState() == act1Level2::WON)
    {
        drawText(window, font, "VAULT MAP RESTORED! PRESS ENTER!",
            28, 640, 660, Color::Green, true);
    }

    if (flashTime > 0)
    {
        RectangleShape redScreen(Vector2f(1280, 720));
        redScreen.setFillColor(Color(255, 0, 0, 90));
        window.draw(redScreen);
    }
}

void Level2Screen::onResume()
{
    isActive = true;
    nextScreen = 3;      
    flashTime = 0;
}


//--------------------------------level 3----------------------------------------------

//Level3Screen::Level3Screen(RenderWindow& win) : Screen (window)
//{
//    font.loadFromFile("assets/fonts/ARIAL.ttf");
//    backgroundTexture.loadFromFile("assets/pictures/pic2.jfif");
//    nextScreen = 4;
//
//
//}


