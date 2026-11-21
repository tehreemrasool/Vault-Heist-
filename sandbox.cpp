#include "screen.h"
#include "helpers.h"
#include <cmath>

static int countNodes(TreeNode* n)
{
    if (n == NULL) return 0;
    return 1 + countNodes(n->left) + countNodes(n->right);
}

static void heapPos(int idx, float& x, float& y)
{
    int L = (int)floor(log2((double)idx + 1));
    int k = idx + 1 - (1 << L);
    x = 1280.0f * (k + 0.5f) / (1 << L);
    y = 160.0f + L * 85;
}

SandboxScreen::SandboxScreen(RenderWindow& win) : Screen(win), heap(31)
{
    if (!font.loadFromFile("assets/fonts/ARIAL.ttf")) loadFont(font);
    if (backgroundTexture.loadFromFile("assets/pictures/Act2_level1.jpeg"))
    {
        background.setTexture(backgroundTexture);
        background.setScale(1280.0f / backgroundTexture.getSize().x,
            720.0f / backgroundTexture.getSize().y);
    }
    for (int i = 0; i < 6; i++)
    {
        btn[i].setSize(Vector2f(190, 44));
        btn[i].setPosition(30.0f + i * 205, 640);
        btn[i].setFillColor(Color(212, 110, 194, 200));
    }
    nextScreen = 23;
    mode = 1;                 // 0 = Binary, 1 = BST, 2 = AVL, 3 = Max Heap
    root = NULL;
    nextId = 0;
    sortedCount = 0;
    highlightId = -1;
}

SandboxScreen::~SandboxScreen() { deleteTree(root); }

void SandboxScreen::onEnter() { isActive = true; nextScreen = 23; clearAll(); }

void SandboxScreen::clearAll()
{
    deleteTree(root);
    root = NULL;
    heap = maxHeap(31);
    sortedCount = 0;
    nextId = 0;
    highlightId = -1;
    input.clear();
    for (int i = 0; i < 8; i++) logLines[i].clear();
}

void SandboxScreen::addLog(const string& s)
{
    for (int i = 7; i > 0; i--) logLines[i] = logLines[i - 1];
    logLines[0] = s;
}

TreeNode* SandboxScreen::insertBST(TreeNode* n, TreeNode* par, int v, bool& ok)
{
    if (!n)
    {
        TreeNode* t = new TreeNode(v);
        t->id = nextId++;
        t->parent = par;
        ok = true;
        return t;
    }
    if (v == n->value) { ok = false; return n; }      // duplicates rejected
    if (v < n->value) n->left = insertBST(n->left, n, v, ok);
    else              n->right = insertBST(n->right, n, v, ok);
    return n;
}

TreeNode* SandboxScreen::deleteBST(TreeNode* n, int v, bool& found)
{
    if (!n) return NULL;
    if (v < n->value) { n->left = deleteBST(n->left, v, found); if (n->left)  n->left->parent = n; }
    else if (v > n->value) { n->right = deleteBST(n->right, v, found); if (n->right) n->right->parent = n; }
    else
    {
        found = true;
        if (!n->left || !n->right)                     // leaf or one child
        {
            TreeNode* c = n->left ? n->left : n->right;
            if (c) c->parent = n->parent;
            delete n;
            return c;
        }
        TreeNode* s = n->right;                        // two children: inorder successor
        while (s->left) s = s->left;
        addLog("Two children: successor " + to_string(s->value));
        n->value = s->value;
        bool f2 = false;
        n->right = deleteBST(n->right, s->value, f2);
        if (n->right) n->right->parent = n;
    }
    return n;
}

// ---------- operations ----------
void SandboxScreen::randomFill()
{
    clearAll();
    if (mode == 0)
    {
        root = constructBinaryTree(8);
        nextId = 8;
    }
    else if (mode == 3)
    {
        for (int i = 0; i < 10; i++) heap.insert(rand() % 100);
    }
    else
    {
        for (int i = 0; i < 10; i++)
        {
            bool ok = false;
            root = insertBST(root, NULL, rand() % 100, ok);
        }
    }
    addLog("Random structure generated");
}

void SandboxScreen::runOp(int op)
{
    bool hasVal = !input.empty();
    int v = hasVal ? atoi(input.c_str()) : 0;
    highlightId = -1;

    if (op == 3) { randomFill(); return; }
    if (op == 4) { clearAll(); addLog("Cleared"); return; }
    if (op == 5)
    {
        mode = (mode + 1) % 4;
        clearAll();
        addLog("Mode switched");
        return;
    }

    if (mode == 1 || mode == 2)    // BST / AVL (AVL rebalancing to be added)
    {
        if (op == 0 && hasVal)
        {
            bool ok = false;
            root = insertBST(root, NULL, v, ok);
            addLog(ok ? "Inserted " + to_string(v) : "Duplicate " + to_string(v) + " rejected");
        }
        else if (op == 1 && hasVal)
        {
            bool found = false;
            root = deleteBST(root, v, found);
            addLog(found ? "Deleted " + to_string(v) : to_string(v) + " not found");
        }
        else if (op == 2 && hasVal)
        {
            TreeNode* c = root;
            int comps = 0;
            while (c && c->value != v)
            {
                comps++;
                addLog("Compare " + to_string(c->value) + (v < c->value ? " -> left" : " -> right"));
                c = (v < c->value) ? c->left : c->right;
            }
            if (c) { highlightId = c->id; addLog("Found after " + to_string(comps + 1) + " comparisons"); }
            else addLog(to_string(v) + " not found (" + to_string(comps) + " comparisons)");
        }
    }
    else if (mode == 0)            // general binary tree
    {
        if (op == 0 && hasVal)
        {
            if (!root) { root = new TreeNode(v); root->id = nextId++; }
            else insertRandom(root, v, nextId++);
            addLog("Inserted " + to_string(v) + " at random position");
        }
        else if (op == 1) { root = mirror(root); addLog("Tree mirrored"); }
        else if (op == 2)
            addLog("Height " + to_string(findHeight(root)) + ", diameter " + to_string(findDiameter(root)));
    }
    else                           // mode 3: max heap
    {
        if (op == 0 && hasVal)
        {
            if (heap.getSize() >= 31) addLog("Heap full");
            else
            {
                sortedCount = 0;
                heap.insert(v);
                addLog("Inserted " + to_string(v) + " (sift-up)");
            }
        }
        else if (op == 1)
        {
            if (heap.getSize() == 0) { addLog("Heap empty"); }
            else
            {
                int m = heap.extractMax();
                sortedVals[sortedCount++] = m;
                addLog("Extracted max " + to_string(m) + " -> sorted region");
            }
        }
        else if (op == 2)
        {
            while (heap.getSize() > 0) sortedVals[sortedCount++] = heap.extractMax();
            addLog("Heap sort complete");
        }
    }
    input.clear();
}

// ---------- input ----------
void SandboxScreen::handleInput()
{
    Event e;
    while (window.pollEvent(e))
    {
        if (e.type == Event::Closed) nextScreen = 99;

        if (e.type == Event::TextEntered)
        {
            if (e.text.unicode >= '0' && e.text.unicode <= '9' && input.size() < 4)
                input += (char)e.text.unicode;
            else if (e.text.unicode == 8 && !input.empty())
                input.pop_back();
        }
        if (e.type == Event::KeyPressed)
        {
            if (e.key.code == Keyboard::Escape) nextScreen = 1;
            if (e.key.code == Keyboard::Return) runOp(0);
        }
        if (e.type == Event::MouseButtonPressed && e.mouseButton.button == Mouse::Left)
        {
            Vector2f p((float)e.mouseButton.x, (float)e.mouseButton.y);
            for (int i = 0; i < 6; i++)
                if (btn[i].getGlobalBounds().contains(p)) runOp(i);
        }
    }
}

// ---------- drawing ----------
void SandboxScreen::drawTree(TreeNode* n)
{
    if (!n) return;
    TreeNode* kids[2] = { n->left, n->right };
    for (int k = 0; k < 2; k++)
    {
        if (!kids[k]) continue;
        Vector2f s(n->x, n->y), d = Vector2f(kids[k]->x, kids[k]->y) - s;
        RectangleShape line(Vector2f(sqrt(d.x * d.x + d.y * d.y), 4));
        line.setPosition(s);
        line.setRotation(atan2(d.y, d.x) * 180.0f / 3.14159f);
        line.setFillColor(Color(208, 221, 183));
        window.draw(line);
    }
    CircleShape c(26);
    c.setOrigin(26, 26);
    c.setPosition(n->x, n->y);
    c.setFillColor(Color(73, 103, 164));
    c.setOutlineThickness(6);
    c.setOutlineColor(n->id == highlightId ? Color(255, 215, 70) : Color(208, 221, 183));
    window.draw(c);
    drawText(window, font, to_string(n->value), 22, n->x, n->y, Color::White, true);
    drawTree(n->left);
    drawTree(n->right);
}

void SandboxScreen::drawHeap()
{
    int n = heap.getSize();
    for (int i = 1; i < n; i++)                       // edges first
    {
        int p = (i - 1) / 2;
        float x1, y1, x2, y2;
        heapPos(p, x1, y1);
        heapPos(i, x2, y2);
        Vector2f s(x1, y1), d(x2 - x1, y2 - y1);
        RectangleShape line(Vector2f(sqrt(d.x * d.x + d.y * d.y), 4));
        line.setPosition(s);
        line.setRotation(atan2(d.y, d.x) * 180.0f / 3.14159f);
        line.setFillColor(Color(208, 221, 183));
        window.draw(line);
    }
    for (int i = 0; i < n; i++)                       // nodes
    {
        float x, y;
        heapPos(i, x, y);
        CircleShape c(24);
        c.setOrigin(24, 24);
        c.setPosition(x, y);
        c.setFillColor(Color(73, 103, 164));
        c.setOutlineThickness(6);
        c.setOutlineColor(i == 0 ? Color(255, 215, 70) : Color(208, 221, 183));
        window.draw(c);
        drawText(window, font, to_string(heap.getValue(i)), 20, x, y, Color::White, true);
        if (i == 0) drawText(window, font, "MAX", 14, x, y - 40, Color(255, 215, 70), true);
    }

    // array view: active heap | sorted region
    int N = n + sortedCount;
    float startX = 640 - N * 20.0f;
    for (int j = 0; j < N; j++)
    {
        bool sorted = j >= n;
        RectangleShape cell(Vector2f(38, 38));
        cell.setPosition(startX + j * 40, 540);
        cell.setFillColor(sorted ? Color(60, 160, 80) : Color(73, 103, 164));
        cell.setOutlineThickness(2);
        cell.setOutlineColor(Color::White);
        window.draw(cell);
        int val = sorted ? sortedVals[N - 1 - j] : heap.getValue(j);
        drawText(window, font, to_string(val), 16, startX + j * 40 + 19, 559, Color::White, true);
    }
    drawText(window, font, "ACTIVE HEAP [0.." + to_string(n - 1) + "]   |   SORTED REGION",
        16, 640, 595, Color(208, 221, 183), true);
}

void SandboxScreen::draw()
{
    window.clear(Color(20, 20, 30));
    window.draw(background);

    const char* names[4] = { "BINARY TREE", "BST", "AVL", "MAX HEAP" };
    drawText(window, font, string("SANDBOX - ") + names[mode], 32, 640, 30, Color::White, true);

    if (mode == 3) drawHeap();
    else if (root)
    {
        int pos = 0;
        setTreePositions(root, 0, pos, countNodes(root), getMaxDepth(root));
        drawTree(root);
    }

    const char* labels[4][3] = {
        { "Insert", "Mirror", "Height/Diam" }, { "Insert", "Delete", "Search" },
        { "Insert", "Delete", "Search" },      { "Insert", "Extract Max", "Heap Sort" } };
    for (int i = 0; i < 6; i++)
    {
        window.draw(btn[i]);
        string t = i < 3 ? labels[mode][i] : (i == 3 ? "Random" : (i == 4 ? "Clear" : "Switch Type"));
        drawText(window, font, t, 18, btn[i].getPosition().x + 95, 662, Color::Black, true);
    }
    drawText(window, font, "Value: " + input + "_   (type number, Enter = Insert, Esc = menu)",
        18, 30, 612, Color::White, false);

    for (int i = 0; i < 8; i++)
        drawText(window, font, logLines[i], 15, 30, 80.0f + i * 20, Color(255, 215, 70 + i * 20), false);
}