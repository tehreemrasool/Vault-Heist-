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

//void drawRoundButton(RenderWindow& w, float x, float y, float width, float height,Color c, bool hover)
//{
//    if (hover)
//    {
//        int r = c.r + 30, g = c.g + 30, b = c.b + 30;
//        c = Color((sf::Uint8)(r > 255 ? 255 : r), (Uint8)(g > 255 ? 255 : g),
//            (Uint8)(b > 255 ? 255 : b));
//    }
//    RectangleShape box(Vector2f(width, height));
//    box.setPosition(x, y);
//    box.setFillColor(c);
//    box.setOutlineThickness(3.f);
//    box.setOutlineColor(Color(200, 165, 70));
//    w.draw(box);
//}
//
//static float slotY(int slot)
//{
//    return 260.f + slot * 70.f;
//}
//
//static void drawVaultBackground(RenderWindow& w)
//{
//    w.clear(Color(22, 24, 34));
//    CircleShape dial(260.f);
//    dial.setOrigin(260.f, 260.f);
//    dial.setPosition(640.f, 380.f);
//    dial.setFillColor(Color(30, 33, 46));
//    dial.setOutlineThickness(8.f);
//    dial.setOutlineColor(Color(48, 52, 70));
//    w.draw(dial);
//
//    for (int i = 0; i < 8; i++)
//    {
//        RectangleShape spoke(Vector2f(240.f, 6.f));
//        spoke.setOrigin(0.f, 3.f);
//        spoke.setPosition(640.f, 380.f);
//        spoke.setRotation(i * 45.f);
//        spoke.setFillColor(Color(40, 44, 60));
//        w.draw(spoke);
//    }
//
//    sf::RectangleShape frame(Vector2f(1240.f, 680.f));
//    frame.setPosition(20.f, 20.f);
//    frame.setFillColor(sf::Color::Transparent);
//    frame.setOutlineThickness(4.f);
//    frame.setOutlineColor(sf::Color(200, 165, 70));
//    w.draw(frame);
//}

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
        circle.setOutlineColor(Color(212, 110, 194));
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


// ------------------------------------- MainMenuScreen ----------------
MainMenuScreen::MainMenuScreen(RenderWindow& win) : Screen(win), button(30, 80), nextScreen(1), instructionButton(30, 80)
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
    title.setPosition(400, 60);

    button.setPosition(500, 410);
    button.setScale(3.0f, 1.0f);
    button.setFillColor(Color(212, 110, 194, 120));
    instructionButton.setPosition(500, 520);
    instructionButton.setScale(3.0f, 1.0f);
    instructionButton.setFillColor(Color(212, 110, 194, 140));
    buttontext2.setFont(font);
    buttontext2.setFillColor(Color::Black);
    buttontext2.setCharacterSize(22);
    buttontext2.setPosition(542, 524);
    buttontext2.setString("Instructions");

    buttonText.setFont(font);
    buttonText.setString("START GAME");
    buttonText.setCharacterSize(22);
    buttonText.setFillColor(Color::Black);
    buttonText.setPosition(550, 420);

    backgroundTexture.loadFromFile("assets/pictures/Hmain.jfif");
    background.setTexture(backgroundTexture);
    background.setScale(1280.0f / backgroundTexture.getSize().x,720.0f / backgroundTexture.getSize().y);
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
                nextScreen = 24;  
            if (instructionButton.getGlobalBounds().contains((float)e.mouseButton.x, (float)e.mouseButton.y))
                nextScreen = 21;

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



// -------------------------------------------- Level1 Screen ----------------

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
            else if (e.key.code == Keyboard::N)
                nextScreen = 3;

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

            else if (e.key.code == Keyboard::N)
                nextScreen = 4;

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
static const float L3_TIME_LIMIT = 5.0f;

static void drawLevel3Nodes(RenderWindow & window, const Font & font, TreeNode * node,
    const act1Level3 & level, float animTime)
{
    if (node == NULL)
        return;

    float radius = 30;

    CircleShape circle(radius);
    circle.setOrigin(radius, radius);
    circle.setPosition(node->x, node->y);
    circle.setFillColor(Color(73, 103, 164));
    circle.setOutlineThickness(7);

    if (level.isSolved(node->id))
        circle.setOutlineColor(Color::Green);
    else
        circle.setOutlineColor(Color(208, 221, 183));

    window.draw(circle);
    if (node->id == level.getCurrentId())
    {
        int alpha = 170 + (int)(50 * sin(animTime * 6));     

        CircleShape halo(radius + 14);
        halo.setOrigin(radius + 14, radius + 14);
        halo.setPosition(node->x, node->y);
        halo.setFillColor(Color(150, 60, 220, (Uint8)(alpha / 3)));
        window.draw(halo);

        CircleShape shade(radius);
        shade.setOrigin(radius, radius);
        shade.setPosition(node->x, node->y);
        shade.setFillColor(Color(160, 70, 230, (Uint8)alpha));
        shade.setOutlineThickness(7);
        shade.setOutlineColor(Color(190, 110, 255, (Uint8)alpha));
        window.draw(shade);
    }

    drawText(window, font, to_string(node->value),
        26, node->x, node->y, Color(255, 255, 255), true);

    drawLevel3Nodes(window, font, node->left, level, animTime);
    drawLevel3Nodes(window, font, node->right, level, animTime);
}


Level3Screen::Level3Screen(RenderWindow & win) : Screen(win)
{
    font.loadFromFile("assets/fonts/ARIAL.ttf");

    if (backgroundTexture.loadFromFile("assets/pictures/pic2.jfif"))
    {
        background.setTexture(backgroundTexture);
        background.setScale(1280.0f / backgroundTexture.getSize().x,
            720.0f / backgroundTexture.getSize().y);
    }

    nextScreen = 4;
    root = NULL;
    count = 0;
    seed = (unsigned)time(NULL) + 13;

    timeLeft = L3_TIME_LIMIT;
    flashTime = 0;
    messageTime = 0;
    animTime = 0;
    messageColor = Color::White;
}


Level3Screen::~Level3Screen()
{
    deleteTree(root);
}
void Level3Screen::newTree()
{
    deleteTree(root);
    root = NULL;

    srand(seed++);

    count = 6 + rand() % 10;

    root = constructBinaryTree(count);

    int position = 0;
    int maxDepth = getMaxDepth(root);

    setTreePositions(root, 0, position, count, maxDepth);

    level.start(root);

    input.clear();
    message.clear();
    messageTime = 0;
    flashTime = 0;
    timeLeft = L3_TIME_LIMIT;
}


void Level3Screen::onEnter()
{
    isActive = true;
    nextScreen = 4;

    newTree();
}


void Level3Screen::onResume()
{
    isActive = true;
    nextScreen = 4;

    flashTime = 0;
    timeLeft = L3_TIME_LIMIT;
}
void Level3Screen::submitAnswer()
{
    if (input.empty())
        return;

    int guess = atoi(input.c_str());
    input.clear();

    int result = level.submit(guess);

    timeLeft = L3_TIME_LIMIT;       
    messageTime = 1.2f;

    if (result == act1Level3::CORRECT)
    {
        message = "CORRECT!  Alarm +1";
        messageColor = Color(120, 255, 120);
    }
    else
    {
        message = "WRONG!  Alarm +1";
        messageColor = Color(255, 110, 110);
        flashTime = 0.35f;
    }
}


void Level3Screen::handleInput()
{
    Event e;

    while (window.pollEvent(e))
    {
        if (e.type == Event::Closed)
            nextScreen = 99;
        if (e.type == Event::TextEntered)
        {
            if (level.getState() == act1Level3::PLAYING &&
                e.text.unicode >= '0' && e.text.unicode <= '9' &&
                input.size() < 2)
            {
                input += (char)e.text.unicode;
            }
        }
        if (e.type == Event::KeyPressed)
        {

            if (e.key.code == Keyboard::Escape)
                nextScreen = 25;

            else if (e.key.code == Keyboard::BackSpace)
            {
                if (!input.empty())
                    input.pop_back();
            }

            else if (e.key.code == Keyboard::Return)
            {
                if (level.getState() == act1Level3::PLAYING)
                    submitAnswer();
            }

            else if (e.key.code == Keyboard::R)
                newTree();
            else if (e.key.code == Keyboard::N)
                nextScreen = 5;

            else if (e.key.code == Keyboard::Return)
            {
                if (level.getState() == act1Level3::WON)
                {
                    nextScreen = 5;
                    levelNumber = 4;
                }
                else if (level.getState() == act1Level3::PLAYING)
                    submitAnswer();
            }
        }
    }
}


void Level3Screen::update(float dt)
{
    animTime += dt;

    if (flashTime > 0)
        flashTime -= dt;

    if (messageTime > 0)
        messageTime -= dt;

    if (level.getState() != act1Level3::PLAYING)
        return;
    timeLeft -= dt;

    if (timeLeft <= 0)
    {
        level.timeUp();                

        timeLeft = L3_TIME_LIMIT;
        input.clear();

        message = "TIME'S UP!  Alarm +1";
        messageColor = Color(255, 110, 110);
        messageTime = 1.2f;
        flashTime = 0.35f;
    }
}


void Level3Screen::draw()
{
    window.clear();
    window.draw(background);
    drawText(window, font, "ACT 1 - LEVEL 3", 40, 500, 15, Color::White, false);

    RectangleShape alarmBox(Vector2f(200, 50));
    RectangleShape timerBox(Vector2f(420, 50));

    alarmBox.setPosition(50, 70);
    timerBox.setPosition(850, 70);
    alarmBox.setFillColor(Color::Black);
    timerBox.setFillColor(Color::Black);
    alarmBox.setOutlineColor(Color(180, 100, 220));
    alarmBox.setOutlineThickness(3);
    timerBox.setOutlineColor(Color(180, 100, 220));
    timerBox.setOutlineThickness(3);

    window.draw(alarmBox);
    window.draw(timerBox);

    drawText(window, font, "ALARM: " + to_string(level.getAlarm()) + " / " + to_string(level.getCapacity()),
        18, 70, 80, Color(255, 255, 255), false);


    float shownTime = timeLeft;
    if (shownTime < 0) shownTime = 0;

    float fraction = shownTime / L3_TIME_LIMIT;

    RectangleShape bar(Vector2f(412 * fraction, 42));
    bar.setPosition(854, 74);

    if (fraction > 0.6f)       bar.setFillColor(Color(60, 160, 80, 190));
    else if (fraction > 0.3f)  bar.setFillColor(Color(200, 170, 40, 190));
    else                       bar.setFillColor(Color(200, 60, 60, 190));

    window.draw(bar);

    int whole = (int)shownTime;
    int tenth = (int)(shownTime * 10) % 10;

    drawText(window, font, "TIME: " + to_string(whole) + "." + to_string(tenth),
        22, 1060, 95, Color::White, true);

    if (messageTime > 0)
        drawText(window, font, message, 24, 640, 100, messageColor, true);
    else
        drawText(window, font, "Height of the purple room? (leaf = 1)",
            20, 640, 100, Color::White, true);
    drawTreeLines(window, root);

    drawLevel3Nodes(window, font, root, level, animTime);
    drawText(window, font, "ROOMS: " + to_string(level.getSolved()) + " / " + to_string(level.getN()),
        20, 60, 668, Color::White, false);
    if (level.getState() == act1Level3::WON)
    {
        drawText(window, font, "VAULT CLEARED!", 30, 640, 670, Color::Green, true);
    }

    else if (level.getState() == act1Level3::LOST)
    {
        drawText(window, font, "ALARM TRIGGERED! YOU LOST!   (R: retry)",
            27, 640, 670, Color::Red, true);
    }

    else
    {
        drawText(window, font, "HEIGHT:", 24, 520, 681, Color::White, true);

        RectangleShape answerBox(Vector2f(100, 46));
        answerBox.setPosition(590, 657);
        answerBox.setFillColor(Color::Black);
        answerBox.setOutlineColor(Color(180, 100, 220));
        answerBox.setOutlineThickness(3);
        window.draw(answerBox);

        string shown = input;
        if (((int)(animTime * 2)) % 2 == 0)
            shown += "|";                     

        drawText(window, font, shown, 28, 607, 662, Color::White, false);

        drawText(window, font, "ENTER to submit", 18, 710, 672, Color(208, 221, 183), false);
    }
    if (flashTime > 0)
    {
        RectangleShape redScreen(Vector2f(1280, 720));

        redScreen.setFillColor(Color(255, 0, 0, 90));

        window.draw(redScreen);
    }
}

//--------------------------------level 4----------------------------------------------

static void drawLevel4Nodes(RenderWindow& window, const Font& font, TreeNode* node,
    const act1Level4& level, int selectedLcaId, float animTime)
{
    if (node == NULL)
        return;

    float radius = 30.0f;
    CircleShape circle(radius);
    circle.setOrigin(radius, radius);
    circle.setPosition(node->x, node->y);
    circle.setFillColor(Color(73, 103, 164));
    circle.setOutlineThickness(7);

    TreeNode* a = level.getNodeA();
    TreeNode* b = level.getNodeB();
    bool isTarget = (a != NULL && node->id == a->id) || (b != NULL && node->id == b->id);

    if (isTarget)
    {
        int alpha = 170 + (int)(50 * sin(animTime * 6));
        CircleShape halo(radius + 13);
        halo.setOrigin(radius + 13, radius + 13);
        halo.setPosition(node->x, node->y);
        halo.setFillColor(Color(240, 190, 50, (Uint8)(alpha / 3)));
        window.draw(halo);

        circle.setFillColor(Color(160, 120, 30));
        circle.setOutlineColor(Color(255, 215, 70));
    }
    else if (node->id == selectedLcaId)
    {
        circle.setFillColor(Color(120, 60, 170));
        circle.setOutlineColor(Color(212, 110, 194));
    }
    else
    {
        circle.setOutlineColor(Color(208, 221, 183));
    }

    window.draw(circle);

    drawText(window, font, to_string(node->value),
        26, node->x, node->y, Color::White, true);

    drawLevel4Nodes(window, font, node->left, level, selectedLcaId, animTime);
    drawLevel4Nodes(window, font, node->right, level, selectedLcaId, animTime);
}


Level4Screen::Level4Screen(RenderWindow& win) : Screen(win)
{
    if (!font.loadFromFile("assets/fonts/ARIAL.ttf"))
        loadFont(font);

    if (backgroundTexture.loadFromFile("assets/pictures/pic2.jfif"))
    {
        background.setTexture(backgroundTexture);
        background.setScale(1280.0f / backgroundTexture.getSize().x,
            720.0f / backgroundTexture.getSize().y);
    }

    nextScreen = 5;
    root = NULL;
    count = 0;
    seed = (unsigned)time(NULL) + 29;

    selectedLcaId = -1;
    selectedLcaVal = -1;
    activeBox = 0;
    flashTime = 0;
    messageTime = 0;
    animTime = 0;
    messageColor = Color::White;
}

Level4Screen::~Level4Screen()
{
    deleteTree(root);
}

void Level4Screen::newTree()
{
    deleteTree(root);
    root = NULL;

    srand(seed++);
    count = 7 + rand() % 8;
    root = constructBinaryTree(count);

    int position = 0;
    int maxDepth = getMaxDepth(root);
    setTreePositions(root, 0, position, count, maxDepth);

    level.start(root, 3);

    selectedLcaId = -1;
    selectedLcaVal = -1;
    pathInput.clear();
    activeBox = 0;
    message.clear();
    messageTime = 0;
    flashTime = 0;
}

void Level4Screen::onEnter()
{
    isActive = true;
    nextScreen = 5;
    newTree();
}

void Level4Screen::onResume()
{
    isActive = true;
    nextScreen = 5;
    flashTime = 0;
}

void Level4Screen::submitAnswer()
{
    if (selectedLcaId == -1 || pathInput.empty())
    {
        message = "Click the LCA room AND enter path length!";
        messageColor = Color(255, 200, 90);
        messageTime = 1.4f;
        flashTime = 0.25f;
        return;
    }

    int lenGuess = atoi(pathInput.c_str());
    int result = level.submit(selectedLcaId, lenGuess);

    selectedLcaId = -1;
    selectedLcaVal = -1;
    pathInput.clear();
    activeBox = 0;
    messageTime = 1.4f;

    if (result == act1Level4::CORRECT)
    {
        message = "CORRECT! Route verified.";
        messageColor = Color(120, 255, 120);
    }
    else
    {
        message = "WRONG LCA or PATH LENGTH! Alarm +1";
        messageColor = Color(255, 110, 110);
        flashTime = 0.35f;
    }
}

void Level4Screen::handleInput()
{
    Event e;

    while (window.pollEvent(e))
    {
        if (e.type == Event::Closed)
            nextScreen = 99;

        if (e.type == Event::MouseButtonPressed && e.mouseButton.button == Mouse::Left)
        {
            float mx = (float)e.mouseButton.x;
            float my = (float)e.mouseButton.y;

            // Check if player clicked Box 1 (LCA) or Box 2 (Path Length)
            if (mx >= 430 && mx <= 550 && my >= 654 && my <= 702)
            {
                activeBox = 0;
            }
            else if (mx >= 760 && mx <= 880 && my >= 654 && my <= 702)
            {
                activeBox = 1;
            }
            else if (level.getState() == act1Level4::PLAYING)
            {
                // Click a node on the tree (using radius 30 matching drawn circle)
                TreeNode* clicked = NULL;
                TreeNode* q[101];
                int f = 0, b = 0;
                if (root != NULL) q[b++] = root;
                while (f < b)
                {
                    TreeNode* cur = q[f++];
                    float dx = mx - cur->x;
                    float dy = my - cur->y;
                    if (dx * dx + dy * dy <= 30.0f * 30.0f)
                    {
                        clicked = cur;
                        break;
                    }
                    if (cur->left) q[b++] = cur->left;
                    if (cur->right) q[b++] = cur->right;
                }

                if (clicked != NULL)
                {
                    selectedLcaId = clicked->id;
                    selectedLcaVal = clicked->value;
                    activeBox = 1; // automatically focus path length box next
                }
            }
        }

        if (e.type == Event::TextEntered)
        {
            if (level.getState() == act1Level4::PLAYING &&
                e.text.unicode >= '0' && e.text.unicode <= '9' &&
                pathInput.size() < 2)
            {
                activeBox = 1;
                pathInput += (char)e.text.unicode;
            }
        }

        if (e.type == Event::KeyPressed)
        {
            if (e.key.code == Keyboard::Escape)
                nextScreen = 25;

            else if (e.key.code == Keyboard::Tab)
                activeBox = 1 - activeBox;

            else if (e.key.code == Keyboard::BackSpace)
            {
                if (activeBox == 1 && !pathInput.empty())
                    pathInput.pop_back();
                else if (activeBox == 0)
                {
                    selectedLcaId = -1;
                    selectedLcaVal = -1;
                }
            }

            else if (e.key.code == Keyboard::Return)
            {
                if (level.getState() == act1Level4::WON)
                {
                    nextScreen = 6;
                    levelNumber = 5;
                }
                else if (level.getState() == act1Level4::PLAYING)
                {
                    submitAnswer();
                }
            }

            else if (e.key.code == Keyboard::N)
                nextScreen = 6;

            else if (e.key.code == Keyboard::R)
                newTree();
        }
    }
}

void Level4Screen::update(float dt)
{
    animTime += dt;
    if (flashTime > 0) flashTime -= dt;
    if (messageTime > 0) messageTime -= dt;
}

void Level4Screen::draw()
{
    window.clear();
    window.draw(background);

    drawText(window, font, "ACT 1 - LEVEL 4", 40, 500, 15, Color::White, false);

    // Top HUD boxes
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

    drawText(window, font, "ALARM: " + to_string(level.getAlarm()) + " / " + to_string(level.getCapacity()),
        18, 70, 82, Color::White, false);

    TreeNode* a = level.getNodeA();
    TreeNode* b = level.getNodeB();
    string pairStr = "ROOMS: ";
    if (a != NULL && b != NULL)
        pairStr += to_string(a->value) + "  &  " + to_string(b->value);

    drawText(window, font, pairStr, 22, 1020, 95, Color(255, 215, 70), true);

    if (messageTime > 0)
        drawText(window, font, message, 22, 540, 95, messageColor, true);
    else
        drawText(window, font, "Click the Lowest Common Ancestor & enter edge distance",
            19, 540, 95, Color::White, true);

    // Tree
    drawTreeLines(window, root);
    drawLevel4Nodes(window, font, root, level, selectedLcaId, animTime);

    // Round progress
    drawText(window, font, "ROUND: " + to_string(level.getCurrentRound()) + " / " + to_string(level.getTotalRounds()),
        20, 60, 668, Color::White, false);

    // Bottom status / 2 submission boxes
    if (level.getState() == act1Level4::WON)
    {
        drawText(window, font, "VAULT CLEARED! PRESS ENTER FOR LEVEL 5!",
            28, 640, 675, Color::Green, true);
    }
    else if (level.getState() == act1Level4::LOST)
    {
        drawText(window, font, "ALARM TRIGGERED! YOU LOST!   (R: retry)",
            26, 640, 675, Color::Red, true);
    }
    else
    {
        // Box 1: LCA Room (selected by clicking node)
        drawText(window, font, "LCA ROOM:", 20, 355, 678, Color::White, true);

        RectangleShape lcaBox(Vector2f(120, 46));
        lcaBox.setPosition(430, 655);
        lcaBox.setFillColor(Color::Black);
        lcaBox.setOutlineColor(activeBox == 0 ? Color(255, 215, 70) : Color(180, 100, 220));
        lcaBox.setOutlineThickness(3);
        window.draw(lcaBox);

        string lcaShown = (selectedLcaVal != -1) ? to_string(selectedLcaVal) : "Click";
        Color lcaColor = (selectedLcaVal != -1) ? Color(212, 110, 194) : Color(160, 160, 160);
        drawText(window, font, lcaShown, 22, 490, 678, lcaColor, true);

        // Box 2: Path Length (edges)
        drawText(window, font, "EDGES:", 20, 695, 678, Color::White, true);

        RectangleShape lenBox(Vector2f(120, 46));
        lenBox.setPosition(760, 655);
        lenBox.setFillColor(Color::Black);
        lenBox.setOutlineColor(activeBox == 1 ? Color(255, 215, 70) : Color(180, 100, 220));
        lenBox.setOutlineThickness(3);
        window.draw(lenBox);

        string lenShown = pathInput;
        if (activeBox == 1 && ((int)(animTime * 2)) % 2 == 0)
            lenShown += "|";

        drawText(window, font, lenShown, 24, 780, 663, Color::White, false);

        drawText(window, font, "ENTER: Submit", 18, 910, 668, Color(208, 221, 183), false);
    }

    if (flashTime > 0)
    {
        RectangleShape redScreen(Vector2f(1280, 720));
        redScreen.setFillColor(Color(255, 0, 0, 90));
        window.draw(redScreen);
    }
}


//--------------------------------level 5----------------------------------------------

static void drawLevel5Nodes(RenderWindow& window, const Font& font, TreeNode* node,
    int lastSwappedId, int state)
{
    if (node == NULL)
        return;

    float radius = 30.0f;
    CircleShape circle(radius);
    circle.setOrigin(radius, radius);
    circle.setPosition(node->x, node->y);
    circle.setFillColor(Color(73, 103, 164));
    circle.setOutlineThickness(7);

    if (state == act1Level5::WON)
        circle.setOutlineColor(Color::Green);
    else if (node->id == lastSwappedId)
        circle.setOutlineColor(Color(212, 110, 194));
    else
        circle.setOutlineColor(Color(208, 221, 183));

    window.draw(circle);

    drawText(window, font, to_string(node->value),
        26, node->x, node->y, Color::White, true);

    drawLevel5Nodes(window, font, node->left, lastSwappedId, state);
    drawLevel5Nodes(window, font, node->right, lastSwappedId, state);
}


Level5Screen::Level5Screen(RenderWindow& win) : Screen(win)
{
    if (!font.loadFromFile("assets/fonts/ARIAL.ttf"))
        loadFont(font);

    if (backgroundTexture.loadFromFile("assets/pictures/bglevel.jfif"))
    {
        background.setTexture(backgroundTexture);
        background.setScale(1280.0f / backgroundTexture.getSize().x,
            720.0f / backgroundTexture.getSize().y);
    }

    nextScreen = 6;
    count = 0;
    seed = (unsigned)time(NULL) + 41;
    lastSwappedId = -1;
    flashTime = 0;
    messageTime = 0;
    messageColor = Color::White;
}

void Level5Screen::refreshLayout()
{
    TreeNode* wRoot = level.getWorkingRoot();
    if (wRoot == NULL) return;
    int position = 0;
    int maxDepth = getMaxDepth(wRoot);
    setTreePositions(wRoot, 0, position, count, maxDepth);
}

void Level5Screen::newTree()
{
    srand(seed++);
    count = 6 + rand() % 7;
    TreeNode* temp = constructBinaryTree(count);
    level.start(temp);
    deleteTree(temp);

    lastSwappedId = -1;
    flashTime = 0;
    messageTime = 0;
    message.clear();
    refreshLayout();
}

void Level5Screen::onEnter()
{
    isActive = true;
    nextScreen = 6;
    newTree();
}

void Level5Screen::onResume()
{
    isActive = true;
    nextScreen = 6;
    flashTime = 0;
}

void Level5Screen::handleInput()
{
    Event e;

    while (window.pollEvent(e))
    {
        if (e.type == Event::Closed)
            nextScreen = 99;

        if (e.type == Event::MouseButtonPressed && e.mouseButton.button == Mouse::Left)
        {
            if (level.getState() == act1Level5::PLAYING)
            {
                float mx = (float)e.mouseButton.x;
                float my = (float)e.mouseButton.y;

                // Find clicked node in working tree
                TreeNode* clicked = NULL;
                TreeNode* q[101];
                int f = 0, b = 0;
                TreeNode* wRoot = level.getWorkingRoot();
                if (wRoot != NULL) q[b++] = wRoot;
                while (f < b)
                {
                    TreeNode* cur = q[f++];
                    float dx = mx - cur->x;
                    float dy = my - cur->y;
                    if (dx * dx + dy * dy <= 30.0f * 30.0f)
                    {
                        clicked = cur;
                        break;
                    }
                    if (cur->left) q[b++] = cur->left;
                    if (cur->right) q[b++] = cur->right;
                }

                if (clicked != NULL)
                {
                    if (level.swapChildren(clicked->id))
                    {
                        lastSwappedId = clicked->id;
                        refreshLayout();
                    }
                    else
                    {
                        message = "Leaf room has no children to swap!";
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

            else if (e.key.code == Keyboard::Space)
            {
                level.resetWorking();
                lastSwappedId = -1;
                refreshLayout();
                message = "Tree reset to original.";
                messageColor = Color(208, 221, 183);
                messageTime = 1.0f;
            }

            else if (e.key.code == Keyboard::R)
                newTree();

            else if (e.key.code == Keyboard::N)
                nextScreen = 1;

            else if (e.key.code == Keyboard::Return)
            {
                if (level.getState() == act1Level5::WON)
                {
                    nextScreen = 1; // Act 1 complete -> return to Main Menu (or Act 2 later)
                }
                else if (level.getState() == act1Level5::PLAYING)
                {
                    if (level.submit())
                    {
                        message = "MIRROR COMPLETE!";
                        messageColor = Color::Green;
                        messageTime = 2.0f;
                    }
                    else
                    {
                        flashTime = 0.35f;
                        message = "Not mirrored yet! Alarm +1";
                        messageColor = Color(255, 110, 110);
                        messageTime = 1.3f;
                    }
                }
            }
        }
    }
}

void Level5Screen::update(float dt)
{
    if (flashTime > 0) flashTime -= dt;
    if (messageTime > 0) messageTime -= dt;
}

void Level5Screen::draw()
{
    window.clear();
    window.draw(background);

    drawText(window, font, "ACT 1 - LEVEL 5", 40, 500, 15, Color::White, false);

    // Top HUD boxes
    RectangleShape alarmBox(Vector2f(220, 50));
    RectangleShape swapBox(Vector2f(320, 50));

    alarmBox.setPosition(50, 70);
    swapBox.setPosition(910, 70);
    alarmBox.setFillColor(Color::Black);
    swapBox.setFillColor(Color::Black);
    alarmBox.setOutlineColor(Color(180, 100, 220));
    alarmBox.setOutlineThickness(3);
    swapBox.setOutlineColor(Color(180, 100, 220));
    swapBox.setOutlineThickness(3);

    window.draw(alarmBox);
    window.draw(swapBox);

    drawText(window, font, "ALARM: " + to_string(level.getAlarm()) + " / " + to_string(level.getCapacity()),
        18, 70, 82, Color::White, false);

    drawText(window, font, "SWAPS MADE: " + to_string(level.getSwaps()),
        20, 1070, 95, Color::White, true);

    if (messageTime > 0)
        drawText(window, font, message, 22, 590, 90, messageColor, true);
    else
        drawText(window, font, "Click a room to swap its Left & Right children to mirror the vault!",
            19, 590, 90, Color::White, true);

    drawText(window, font, "Click Room: Swap Children | Space: Reset | Enter: Submit | R: New Tree",
        16, 640, 125, Color(208, 221, 183), true);

    // Working Tree
    TreeNode* wRoot = level.getWorkingRoot();
    drawTreeLines(window, wRoot);
    drawLevel5Nodes(window, font, wRoot, lastSwappedId, level.getState());

    // Bottom status
    if (level.getState() == act1Level5::WON)
    {
        drawText(window, font, "ACT 1 CLEARED! VAULT MIRRORED! PRESS ENTER!",
            28, 640, 672, Color::Green, true);
    }
    else if (level.getState() == act1Level5::LOST)
    {
        drawText(window, font, "ALARM TRIGGERED! YOU LOST!   (R: retry)",
            26, 640, 672, Color::Red, true);
    }

    if (flashTime > 0)
    {
        RectangleShape redScreen(Vector2f(1280, 720));
        redScreen.setFillColor(Color(255, 0, 0, 90));
        window.draw(redScreen);
    }
}