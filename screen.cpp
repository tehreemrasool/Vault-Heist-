#include "screen.h"

using namespace std;

// ---------------- shared helpers ----------------
bool loadFont(sf::Font& f)
{
    const char* paths[] = {
        "assets/Fonts/SuperShiny-0v0rG.ttf", "arial.ttf", "font.ttf",
        "C:/Windows/Fonts/arial.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/System/Library/Fonts/Supplemental/Arial.ttf" };
    for (int i = 0; i < 6; i++)
        if (f.loadFromFile(paths[i])) return true;
    return false;
}

void drawText(sf::RenderWindow& w, const sf::Font& f, const std::string& s, unsigned size,
    float x, float y, sf::Color c, bool center)
{
    sf::Text t(s, f, size);
    t.setFillColor(c);
    sf::FloatRect b = t.getLocalBounds();
    if (center) t.setOrigin(b.left + b.width / 2, b.top + b.height / 2);
    t.setPosition(x, y);
    w.draw(t);
}

bool isMouseOver(sf::RenderWindow& w, float x, float y, float width, float height)
{
    sf::Vector2i mp = sf::Mouse::getPosition(w);
    return mp.x >= x && mp.x <= x + width && mp.y >= y && mp.y <= y + height;
}

void drawRoundButton(sf::RenderWindow& w, float x, float y, float width, float height,
    sf::Color c, bool hover)
{
    if (hover)
    {
        int r = c.r + 30, g = c.g + 30, b = c.b + 30;
        c = sf::Color((sf::Uint8)(r > 255 ? 255 : r), (sf::Uint8)(g > 255 ? 255 : g),
            (sf::Uint8)(b > 255 ? 255 : b));
    }
    sf::RectangleShape box(sf::Vector2f(width, height));
    box.setPosition(x, y);
    box.setFillColor(c);
    box.setOutlineThickness(3.f);
    box.setOutlineColor(sf::Color(200, 165, 70));
    w.draw(box);
}

static float slotY(int slot)
{
    return 260.f + slot * 70.f;
}

static void drawVaultBackground(sf::RenderWindow& w)
{
    w.clear(sf::Color(22, 24, 34));

    sf::CircleShape dial(260.f);
    dial.setOrigin(260.f, 260.f);
    dial.setPosition(640.f, 380.f);
    dial.setFillColor(sf::Color(30, 33, 46));
    dial.setOutlineThickness(8.f);
    dial.setOutlineColor(sf::Color(48, 52, 70));
    w.draw(dial);

    for (int i = 0; i < 8; i++)
    {
        sf::RectangleShape spoke(sf::Vector2f(240.f, 6.f));
        spoke.setOrigin(0.f, 3.f);
        spoke.setPosition(640.f, 380.f);
        spoke.setRotation(i * 45.f);
        spoke.setFillColor(sf::Color(40, 44, 60));
        w.draw(spoke);
    }

    sf::RectangleShape frame(sf::Vector2f(1240.f, 680.f));
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
                nextScreen = 2;     // start Act 1 Level 1
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


// ---------------- Level1Screen ----------------
Level1Screen::Level1Screen(sf::RenderWindow& win) : Screen(win)
{
    loadFont(font);
    nextScreen = 2;
    root = NULL;
    count = 0;
    radius = 22;
    slotCounter = 0;
    maxDepth = 0;
    curType = act1Level1::PREORDER;
    seed = (unsigned)time(NULL);
    flashId = -1;
    flashTime = 0;
    for (int i = 0; i < 101; i++)
    {
        byId[i] = NULL;
        px[i] = py[i] = 0;
        slotOf[i] = depthOf[i] = 0;
    }
}

Level1Screen::~Level1Screen() { deleteTree(root); }

void Level1Screen::assign(TreeNode* r, int d)
{
    if (!r) return;
    assign(r->left, d + 1);
    slotOf[r->id] = slotCounter++;
    depthOf[r->id] = d;
    byId[r->id] = r;
    if (d > maxDepth) maxDepth = d;
    assign(r->right, d + 1);
}

void Level1Screen::layoutTree()
{
    for (int i = 0; i < 101; i++) byId[i] = NULL;
    slotCounter = 0; maxDepth = 0;
    assign(root, 0);
    int n = count;

    float xs = 900.0f / (n > 1 ? n - 1 : 1);
    if (xs > 90) xs = 90;
    float x0 = W / 2.0f - xs * (n - 1) / 2.0f;
    float ys = 440.0f / (maxDepth > 0 ? maxDepth : 1);
    if (ys > 80) ys = 80;
    radius = ys * 0.45f; if (radius > 22) radius = 22;
    if (xs * 0.45f < radius) radius = xs * 0.45f;

    for (int i = 0; i < n; i++)
    {
        px[i] = x0 + slotOf[i] * xs;
        py[i] = 150 + depthOf[i] * ys;
    }
}

int Level1Screen::hitTest(float mx, float my) const
{
    for (int i = 0; i < count; i++)
    {
        float dx = mx - px[i], dy = my - py[i];
        if (dx * dx + dy * dy <= radius * radius) return i;
    }
    return -1;
}

void Level1Screen::newTree()
{
    deleteTree(root);
    srand(seed++);
    count = 6 + rand() % 10;              // 6..15 rooms
    root = constructBinaryTree(count);
    layoutTree();
    level.start(root, curType);
}

void Level1Screen::onEnter()
{
    isActive = true;
    nextScreen = 2;
    flashId = -1;
    flashTime = 0;
    newTree();
}

void Level1Screen::handleInput()
{
    sf::Event e;
    while (window.pollEvent(e))
    {
        if (e.type == sf::Event::Closed) nextScreen = 99;

        if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left)
        {
            int id = hitTest((float)e.mouseButton.x, (float)e.mouseButton.y);
            if (id != -1 && level.checkClick(id) == act1Level1::WRONG)
            {
                flashId = id; flashTime = 0.35f;
            }
        }
        if (e.type == sf::Event::KeyPressed)
        {
            if (e.key.code == sf::Keyboard::Escape) nextScreen = 1;           // back to menu
            else if (e.key.code == sf::Keyboard::R) newTree();                // new random tree
            else if (e.key.code == sf::Keyboard::Space) level.start(root, curType);  // retry
            else
            {
                bool change = true;
                if (e.key.code == sf::Keyboard::Num1) curType = act1Level1::PREORDER;
                else if (e.key.code == sf::Keyboard::Num2) curType = act1Level1::INORDER;
                else if (e.key.code == sf::Keyboard::Num3) curType = act1Level1::POSTORDER;
                else if (e.key.code == sf::Keyboard::Num4) curType = act1Level1::LEVELORDER;
                else change = false;
                if (change) level.start(root, curType);
            }
        }
    }
}

void Level1Screen::update(float dt)
{
    if (flashTime > 0) flashTime -= dt;
}

void Level1Screen::draw()
{
    sf::Vector2i mp = sf::Mouse::getPosition(window);
    int hover = hitTest((float)mp.x, (float)mp.y);

    window.clear(sf::Color(24, 26, 38));

    // ---- HUD ----
    drawText(window, font, "Act 1 - Level 1: The Patrol Route", 24, 40, 12, sf::Color(255, 215, 90), false);
    drawText(window, font, std::string("Click the rooms in ") + typeName(level.getType()),
        18, 40, 44, sf::Color::White, false);

    float ratio = (float)level.getAlarm() / level.getCapacity();
    if (ratio > 1) ratio = 1;
    sf::RectangleShape barBack(sf::Vector2f(920, 20));
    barBack.setPosition(40, 80); barBack.setFillColor(sf::Color(55, 58, 75));
    window.draw(barBack);
    sf::RectangleShape bar(sf::Vector2f(920 * ratio, 20));
    bar.setPosition(40, 80);
    bar.setFillColor(sf::Color((sf::Uint8)(60 + 195 * ratio), (sf::Uint8)(200 - 150 * ratio), 70));
    window.draw(bar);
    drawText(window, font, "ALARM " + std::to_string(level.getAlarm()) + " / " +
        std::to_string(level.getCapacity()) + "   (wrong clicks: " +
        std::to_string(level.getPenalties()) + ")", 14, 500, 90, sf::Color::White, true);
    drawText(window, font, "Progress: " + std::to_string(level.getNext()) + " / " +
        std::to_string(level.getN()) + "   Seed: " + std::to_string(seed - 1),
        14, 40, 108, sf::Color(170, 175, 200), false);

    // ---- edges ----
    int n = count;
    for (int i = 0; i < n; i++)
    {
        TreeNode* nd = byId[i];
        TreeNode* kids[2] = { nd->left, nd->right };
        for (int k = 0; k < 2; k++)
        {
            if (!kids[k]) continue;
            sf::Vertex line[2] = {
                sf::Vertex(sf::Vector2f(px[i], py[i]), sf::Color(120, 125, 150)),
                sf::Vertex(sf::Vector2f(px[kids[k]->id], py[kids[k]->id]), sf::Color(120, 125, 150)) };
            window.draw(line, 2, sf::Lines);
        }
    }

    // ---- nodes ----
    for (int i = 0; i < n; i++)
    {
        sf::CircleShape c(radius);
        c.setOrigin(radius, radius);
        c.setPosition(px[i], py[i]);
        c.setOutlineThickness(3);

        int ord = level.orderOf(i);
        sf::Color fill(70, 90, 140), outline(150, 170, 230);
        if (ord > 0) { fill = sf::Color(40, 150, 80); outline = sf::Color(120, 230, 150); }
        else if (i == hover && level.getState() == act1Level1::PLAYING)
        {
            fill = sf::Color(110, 130, 200); outline = sf::Color::White;
        }
        if (i == flashId && flashTime > 0) { fill = sf::Color(190, 50, 50); outline = sf::Color(255, 120, 120); }
        c.setFillColor(fill); c.setOutlineColor(outline);
        window.draw(c);

        drawText(window, font, std::to_string(byId[i]->value), (unsigned)(radius * 0.8f + 4),
            px[i], py[i] - 2, sf::Color::White, true);
        if (ord > 0)
            drawText(window, font, "#" + std::to_string(ord), 13, px[i], py[i] - radius - 10,
                sf::Color(150, 255, 180), true);
    }

    // ---- bottom banner ----
    std::string msg; sf::Color mc = sf::Color::White;
    if (level.getState() == act1Level1::PLAYING)
        msg = "Which room is visited next?";
    else if (level.getState() == act1Level1::WON)
    {
        msg = "VAULT CLEARED!  Stars: " + std::to_string(level.stars()) + "/3   Score: " +
            std::to_string(level.score());
        mc = sf::Color(255, 215, 90);
    }
    else { msg = "ALARM TRIGGERED - the guards caught you!  Stars: 0"; mc = sf::Color(255, 100, 100); }
    drawText(window, font, msg, 22, W / 2.0f, 625, mc, true);
    drawText(window, font, "Left click: choose room | 1-4: traversal type | Space: retry | R: new tree | Esc: menu",
        14, W / 2.0f, 670, sf::Color(150, 155, 180), true);
}