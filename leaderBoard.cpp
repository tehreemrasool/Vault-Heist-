#include "screen.h"
#include "helpers.h"
#include <fstream>
#include <cmath>

// ---------- file storage: one line per win -> "levelId stars score" ----------
void saveResult(int levelId, int stars, int score)
{
    ofstream f("leaderboard.txt", ios::app);
    if (f) f << levelId << " " << stars << " " << score << "\n";
}

// packs each record into one int: score*10000 + levelId*10 + stars
int loadResults(int keys[], int maxN)
{
    ifstream f("leaderboard.txt");
    int n = 0, id, st, sc;
    while (n < maxN && f >> id >> st >> sc)
        keys[n++] = sc * 10000 + id * 10 + st;
    return n;
}

int previousBest(int levelId)
{
    int keys[500];
    int n = loadResults(keys, 500);
    int best = -1;
    for (int i = 0; i < n; i++)
    {
        int id = (keys[i] / 10) % 1000;
        int sc = keys[i] / 10000;
        if (id == levelId && sc > best) best = sc;
    }
    return best;
}

static string levelName(int id)      // ids 2-6 = Act 1, 7-11 = Act 2, 12-16 = Act 3, 17-21 = Act 4
{
    return "ACT " + to_string((id - 2) / 5 + 1) + " - LEVEL " + to_string((id - 2) % 5 + 1);
}

static void drawStar(RenderWindow& w, float cx, float cy, float r, bool filled)
{
    ConvexShape s(10);
    for (int i = 0; i < 10; i++)
    {
        float a = (-90.0f + i * 36.0f) * 3.14159f / 180.0f;
        float rad = (i % 2 == 0) ? r : r * 0.45f;
        s.setPoint(i, Vector2f(cx + rad * cos(a), cy + rad * sin(a)));
    }
    s.setFillColor(filled ? Color(255, 215, 70) : Color(40, 40, 50));
    s.setOutlineThickness(3);
    s.setOutlineColor(filled ? Color(255, 245, 160) : Color(120, 120, 130));
    w.draw(s);
}

// ---------- ResultPopup ----------
ResultPopup::ResultPopup(RenderWindow& win) : window(win)
{
    if (!font.loadFromFile("assets/fonts/ARIAL.ttf")) loadFont(font);
    open = false;
    action = 0;
    levelId = 0; stars = 0; score = 0; alarm = 0; penalties = 0;
    newBest = false;
}

void ResultPopup::show(int id, int st, int sc, int al, int pe)
{
    levelId = id; stars = st; score = sc; alarm = al; penalties = pe;
    newBest = (sc > previousBest(id));       // must be checked BEFORE saving
    saveResult(id, st, sc);
    open = true;
    action = 0;
}

void ResultPopup::handleInput()
{
    Event e;
    while (window.pollEvent(e))
    {
        if (e.type == Event::Closed) action = 99;

        if (e.type == Event::KeyPressed)
        {
            if (e.key.code == Keyboard::Return)      action = 1;
            else if (e.key.code == Keyboard::L)      action = 2;
            else if (e.key.code == Keyboard::Escape) open = false;   // stay on the level
        }
    }
}

void ResultPopup::draw()
{
    if (!open) return;

    RectangleShape dim(Vector2f(1280, 720));
    dim.setFillColor(Color(0, 0, 0, 150));
    window.draw(dim);

    RectangleShape box(Vector2f(560, 380));
    box.setOrigin(280, 190);
    box.setPosition(640, 360);
    box.setFillColor(Color(0, 0, 0, 230));
    box.setOutlineThickness(4);
    box.setOutlineColor(Color(212, 110, 194));
    window.draw(box);

    drawText(window, font, "VAULT CLEARED!", 34, 640, 200, Color::Green, true);
    drawText(window, font, levelName(levelId), 20, 640, 240, Color(208, 221, 183), true);

    for (int i = 0; i < 3; i++)
        drawStar(window, 560.0f + i * 80, 305, 30, i < stars);

    drawText(window, font, "SCORE: " + to_string(score), 30, 640, 375, Color(255, 215, 70), true);

    if (newBest)
        drawText(window, font, "NEW BEST!", 20, 640, 410, Color(120, 255, 120), true);

    drawText(window, font, "ALARM STEPS: " + to_string(alarm) + "     PENALTIES: " + to_string(penalties),
        20, 640, 450, Color::White, true);

    string next = (levelId == 21) ? "ENTER: Leaderboard" : "ENTER: Next level";
    drawText(window, font, next + "    L: Leaderboard    ESC: Stay",
        17, 640, 515, Color(208, 221, 183), true);
}

// ---------- LeaderboardScreen ----------
LeaderboardScreen::LeaderboardScreen(RenderWindow& win) : Screen(win)
{
    if (!font.loadFromFile("assets/fonts/ARIAL.ttf")) loadFont(font);
    if (backgroundTexture.loadFromFile("assets/pictures/Hmain.jfif"))
    {
        background.setTexture(backgroundTexture);
        background.setScale(1280.0f / backgroundTexture.getSize().x,
            720.0f / backgroundTexture.getSize().y);
    }
    nextScreen = 26;
    topCount = 0;
    total = 0;
}

void LeaderboardScreen::onEnter()
{
    isActive = true;
    nextScreen = 26;

    int keys[500];
    int n = loadResults(keys, 500);
    heapSort(keys, n);                       // your own heap sort, ascending

    topCount = 0;
    for (int i = n - 1; i >= 0 && topCount < 10; i--)
        topKeys[topCount++] = keys[i];

    int best[22];
    for (int i = 0; i < 22; i++) best[i] = 0;
    for (int i = 0; i < n; i++)
    {
        int id = (keys[i] / 10) % 1000;
        int sc = keys[i] / 10000;
        if (id >= 2 && id <= 21 && sc > best[id]) best[id] = sc;
    }
    total = 0;
    for (int i = 2; i <= 21; i++) total += best[i];
}

void LeaderboardScreen::handleInput()
{
    Event e;
    while (window.pollEvent(e))
    {
        if (e.type == Event::Closed) nextScreen = 99;
        if (e.type == Event::KeyPressed &&
            (e.key.code == Keyboard::BackSpace || e.key.code == Keyboard::Escape))
            nextScreen = 1;
    }
}

void LeaderboardScreen::draw()
{
    window.clear();
    window.draw(background);

    RectangleShape panel(Vector2f(720, 560));
    panel.setOrigin(360, 280);
    panel.setPosition(640, 360);
    panel.setFillColor(Color(0, 0, 0, 220));
    panel.setOutlineThickness(4);
    panel.setOutlineColor(Color(212, 110, 194));
    window.draw(panel);

    drawText(window, font, "LEADERBOARD", 38, 640, 112, Color::White, true);
    drawText(window, font, "#", 18, 310, 170, Color(208, 221, 183), false);
    drawText(window, font, "LEVEL", 18, 370, 170, Color(208, 221, 183), false);
    drawText(window, font, "STARS", 18, 600, 170, Color(208, 221, 183), false);
    drawText(window, font, "SCORE", 18, 820, 170, Color(208, 221, 183), false);

    if (topCount == 0)
        drawText(window, font, "No scores yet - clear a level!", 22, 640, 330, Color(255, 215, 70), true);

    for (int i = 0; i < topCount; i++)
    {
        int stars = topKeys[i] % 10;
        int id = (topKeys[i] / 10) % 1000;
        int sc = topKeys[i] / 10000;
        float y = 205.0f + i * 34;
        Color c = (i == 0) ? Color(255, 215, 70) : Color::White;

        drawText(window, font, to_string(i + 1) + ".", 20, 310, y, c, false);
        drawText(window, font, levelName(id), 20, 370, y, c, false);
        drawText(window, font, to_string(stars) + " / 3", 20, 600, y, c, false);
        drawText(window, font, to_string(sc), 20, 820, y, c, false);
    }

    drawText(window, font, "TOTAL (best of each level): " + to_string(total),
        24, 640, 572, Color(120, 255, 120), true);
    drawText(window, font, "BACKSPACE: menu", 17, 640, 622, Color(208, 221, 183), true);
}