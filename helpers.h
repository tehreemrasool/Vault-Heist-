#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <cmath>
#include "act1.h" 
using namespace sf;
using namespace std;
inline bool loadFont(Font& f)
{
    if (f.loadFromFile("assets/Fonts/SuperShiny.ttf")) return true;
    return false;
}
static void applyHover(ConvexShape& b, Vector2f mouse)
{
    if (b.getGlobalBounds().contains(mouse))
    {
        b.setFillColor(Color(212, 110, 194, 230));
        b.setOutlineThickness(3);
        b.setOutlineColor(Color::White);
    }
    else
    {
        b.setFillColor(Color(212, 110, 194, 120));
        b.setOutlineThickness(0);
    }
}
static View makeLetterboxView(float winW, float winH)
{
    View view(FloatRect(0, 0, 1280, 720));

    float winRatio = winW / winH;
    float viewRatio = 1280.f / 720.f;
    float sizeX = 1, sizeY = 1, posX = 0, posY = 0;

    if (winRatio > viewRatio) { sizeX = viewRatio / winRatio; posX = (1 - sizeX) / 2; }
    else { sizeY = winRatio / viewRatio; posY = (1 - sizeY) / 2; }

    view.setViewport(FloatRect(posX, posY, sizeX, sizeY));
    return view;
}

inline void drawSmoke(RenderWindow& window, float progress)
{
    int x[] = { 100, 250, 400, 550, 700, 850, 1000, 1150,
                180, 350, 520, 690, 860, 1030, 1200 };

    int y[] = { 150, 500, 250, 600, 180, 450, 200, 550,
                350, 100, 400, 100, 550, 350, 100 };

    float opacity;

    if (progress < 0.5f)
        opacity = progress * 2;
    else
        opacity = (1 - progress) * 2;

    int alpha = (int)(opacity * 170);

    for (int i = 0; i < 15; i++)
    {
        float radius = 90 + progress * 100 + (i % 3) * 20;

        CircleShape smoke(radius);

        smoke.setPosition(
            x[i] - radius + sin(progress * 5 + i) * 25,
            y[i] - radius - progress * 40
        );

        smoke.setFillColor(Color(180, 180, 190, alpha));

        window.draw(smoke);
    }
}

inline void drawText(RenderWindow& w, const Font& f, const string& s, unsigned size, float x, float y, Color c, bool center)
{
    Text t(s, f, size);
    t.setFillColor(c);
    FloatRect b = t.getLocalBounds();
    if (center) t.setOrigin(b.left + b.width / 2, b.top + b.height / 2);
    t.setPosition(x, y);
    w.draw(t);
}

inline bool isMouseOver(RenderWindow& w, float x, float y, float width, float height)
{
    Vector2i mp = Mouse::getPosition(w);
    return mp.x >= x && mp.x <= x + width && mp.y >= y && mp.y <= y + height;
}

static const char* typeName(int t)
{
    if (t == act1Level1::PREORDER)  return "PREORDER (root, left, right)";
    if (t == act1Level1::INORDER)   return "INORDER (left, root, right)";
    if (t == act1Level1::POSTORDER) return "POSTORDER (left, right, root)";
    return "LEVEL ORDER (top to bottom, left to right)";
}

inline int getMaxDepth(TreeNode* root)
{
    if (root == NULL)
        return 0;

    int left = getMaxDepth(root->left);
    int right = getMaxDepth(root->right);

    return 1 + (left > right ? left : right);
}


inline void setTreePositions(TreeNode* root, int depth, int& position,
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


inline void drawTreeLines(RenderWindow& window, TreeNode* root)
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


inline void drawTreeNodes(RenderWindow& window, Font& font,
    TreeNode* root, act1Level1& level)
{
    if (root == NULL)
        return;

    float radius = 30;

    CircleShape circle(radius);

    circle.setOrigin(radius, radius);
    circle.setPosition(root->x, root->y);
    circle.setFillColor(Color(73, 103, 164));
    circle.setOutlineThickness(7);

    if (level.orderOf(root->id) > 0)
        circle.setOutlineColor(Color(212, 110, 194));
    else
        circle.setOutlineColor(Color(208, 221, 183));

    window.draw(circle);

    drawText(window, font, to_string(root->value),
        26, root->x, root->y, Color(255, 255, 255), true);

    drawTreeNodes(window, font, root->left, level);
    drawTreeNodes(window, font, root->right, level);
}


inline TreeNode* findClickedNode(TreeNode* root, float x, float y)
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

inline TreeNode* findNodeById(TreeNode* root, int id)
{
    if (root == NULL) return NULL;
    if (root->id == id) return root;
    TreeNode* f = findNodeById(root->left, id);
    if (f != NULL) return f;
    return findNodeById(root->right, id);
}


inline static ConvexShape makeRoundedRect(float w, float h, float r)
{
    const int pointsPerCorner = 8;
    const float PI = 3.14159265f;

    ConvexShape shape(pointsPerCorner * 4);

    float cx[4] = { r, w - r, w - r, r };
    float cy[4] = { r, r, h - r, h - r };
    float startAngle[4] = { 180, 270, 0, 90 };

    int index = 0;
    for (int c = 0; c < 4; c++)
    {
        for (int i = 0; i < pointsPerCorner; i++)
        {
            float angle = (startAngle[c] + 90.0f * i / (pointsPerCorner - 1)) * PI / 180.0f;
            shape.setPoint(index, Vector2f(cx[c] + r * cos(angle), cy[c] + r * sin(angle)));
            index++;
        }
    }
    return shape;
}
inline static void centerText(Text& t, const ConvexShape& b)
{
    FloatRect tb = t.getLocalBounds();
    FloatRect bb = b.getGlobalBounds();
    t.setOrigin(tb.left + tb.width / 2.0f, tb.top + tb.height / 2.0f);
    t.setPosition(bb.left + bb.width / 2.0f, bb.top + bb.height / 2.0f);
}

class RoundedRectangleShape : public Shape {
    Vector2f size;
    float radius;
    unsigned cornerPoints;
public:
    RoundedRectangleShape(Vector2f s = { 0, 0 }, float r = 0, unsigned pts = 8)
        : size(s), radius(r), cornerPoints(pts) {
        update();
    }

    void setSize(Vector2f s) { size = s; update(); }
    void setRadius(float r) { radius = r; update(); }

    size_t getPointCount() const override { return cornerPoints * 4; }

    Vector2f getPoint(size_t index) const override {
        size_t corner = index / cornerPoints;
        float delta = 90.f / (cornerPoints - 1);
        Vector2f c;
        switch (corner) {
        case 0: c = { size.x - radius, radius }; break;
        case 1: c = { radius, radius }; break;
        case 2: c = { radius, size.y - radius }; break;
        default: c = { size.x - radius, size.y - radius }; break;
        }
        float a = delta * (index - corner) * 3.14159265f / 180.f;
        return { radius * std::cos(a) + c.x, -radius * std::sin(a) + c.y };
    }
};