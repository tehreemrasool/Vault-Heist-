#include "Act2.h"

// small random generator (no globals, does not touch rand())
static unsigned nextRandom(unsigned& state)
{
    state = state * 1103515245u + 12345u;
    return (state >> 16) & 0x7fff;
}

// ---------------------------------------------------------------
// Constructor / destructor / reset
// ---------------------------------------------------------------
BST::BST()
{
    root = NULL;
    nextId = 1;
    lastAlarmSteps = 0;
    lastMessage = "";
    rangeHead = NULL;
    rangeTail = NULL;
    rangeVisited = 0;
    rangeSkippedSubtrees = 0;
    rangeSkippedNodes = 0;
    rangeInRange = 0;
}

BST::~BST()
{
    clearTree(root);
    root = NULL;
    clearRangeResults();
}

void BST::reset()
{
    clearTree(root);
    root = NULL;
    clearRangeResults();
    events.clear();
    nextId = 1;
    lastAlarmSteps = 0;
    lastMessage = "";
    rangeVisited = 0;
    rangeSkippedSubtrees = 0;
    rangeSkippedNodes = 0;
    rangeInRange = 0;
}

// ---------------------------------------------------------------
// Basic access
// ---------------------------------------------------------------
BSTNode* BST::getRoot()
{
    return root;
}

bool BST::isEmpty()
{
    return root == NULL;
}

EventNode* BST::getEvents()
{
    return events.getHead();
}

void BST::clearEvents()
{
    events.clear();
}

int BST::getEventCount()
{
    return events.count();
}

EventNode* BST::getEvent(int index)
{
    return events.getAt(index);
}

std::string BST::getLastMessage()
{
    return lastMessage;
}

int BST::getLastAlarmSteps()
{
    return lastAlarmSteps;
}

int BST::getNextId()
{
    return nextId;
}

// ---------------------------------------------------------------
// Event text for the Event Log panel
// ---------------------------------------------------------------
std::string BST::describeEvent(EventNode* e)
{
    if (e == NULL)
        return "";

    if (e->is("VISIT"))
        return "Entered room " + std::to_string(e->key) + " (alarm +1)";

    if (e->is("COMPARE"))
    {
        std::string s = "Compared " + std::to_string(e->key) +
            " with " + std::to_string(e->nodeA);

        if (e->result == 0)
            s += ": match";
        else if (e->result == 1)
            s += ": smaller, go left";
        else
            s += ": larger, go right";
        return s;
    }

    if (e->is("INSERT"))
    {
        if (e->parentId == -1)
            return "Inserted " + std::to_string(e->key) + " as the root";

        return "Inserted " + std::to_string(e->key) +
            (e->side == 'L' ? " as left child of " : " as right child of ") +
            std::to_string(e->nodeA);
    }

    if (e->is("UNLINK"))
        return "Removed room " + std::to_string(e->key);

    if (e->is("SWAP_VALUE"))
        return "Replaced " + std::to_string(e->key) +
        " with its inorder successor " + std::to_string(e->result);

    if (e->is("SUCCESSOR_STEP"))
        return "Looking for inorder successor, at " + std::to_string(e->key);

    if (e->is("SUCCESSOR_FOUND"))
        return "Inorder successor is " + std::to_string(e->key);

    if (e->is("HIGHLIGHT_SUBTREE"))
    {
        if (e->mode == 'V')
            return "Visiting room " + std::to_string(e->key);
        if (e->mode == 'S')
            return "Pruned subtree rooted at " + std::to_string(e->key) +
            " (" + std::to_string(e->result) + " rooms skipped)";
        if (e->mode == 'R')
            return std::to_string(e->key) + " is inside the range";
        return "Highlighted subtree";
    }

    if (e->is("NOT_FOUND"))
        return "Key " + std::to_string(e->key) + " not found";

    if (e->is("BOSS_WIN"))
        return "Boss target " + std::to_string(e->key) + " found";

    if (e->is("BOSS_LOSE"))
        return "Alarm is full! The guards arrived";

    if (e->is("BOSS_MISSING"))
        return "Boss target " + std::to_string(e->key) + " does not exist";

    return e->type;
}

// ---------------------------------------------------------------
// Comparison helpers
// ---------------------------------------------------------------
int BST::compareAt(BSTNode* node, int key)
{
    if (node->value < key)
        return -1;
    if (node->value > key)
        return 1;
    return 0;
}

BSTNode* BST::childToward(BSTNode* node, int key)
{
    if (node == NULL)
        return NULL;

    int result = compareAt(node, key);

    if (result == 1)
        return static_cast<BSTNode*>(node->left);
    if (result == -1)
        return static_cast<BSTNode*>(node->right);
    return NULL;
}

// records VISIT + COMPARE for one room and returns the compare result
int BST::stepCompare(BSTNode* node, int key)
{
    events.addEvent("VISIT", node->id, -1, -1, -1, node->value);

    int result = compareAt(node, key);

    // nodeA stores the room's value so the log can print it later
    events.addEvent("COMPARE", node->id, -1, node->value, -1,
        key, result);

    return result;
}

// ---------------------------------------------------------------
// Search
// ---------------------------------------------------------------
BSTNode* BST::searchNode(int value, int& alarm_count)
{
    events.clear();
    alarm_count = 0;
    lastAlarmSteps = 0;

    BSTNode* temp = root;

    while (temp != NULL)
    {
        alarm_count++;
        lastAlarmSteps = alarm_count;

        int result = stepCompare(temp, value);

        if (result == 0)
        {
            lastMessage = "Found " + std::to_string(value);
            return temp;
        }
        else if (result == 1)
            temp = static_cast<BSTNode*>(temp->left);
        else
            temp = static_cast<BSTNode*>(temp->right);
    }

    events.addEvent("NOT_FOUND", -1, -1, -1, -1, value);
    lastMessage = "Key " + std::to_string(value) + " not found";
    return NULL;
}

// ---------------------------------------------------------------
// Insert
// ---------------------------------------------------------------
bool BST::insertNode(int id, int value)
{
    events.clear();
    lastAlarmSteps = 0;

    BSTNode* temp = root;
    BSTNode* parent = NULL;
    char side = 'N';

    while (temp != NULL)
    {
        lastAlarmSteps++;

        int result = stepCompare(temp, value);

        if (result == 0)
        {
            lastMessage = "Duplicate key " + std::to_string(value) +
                " rejected - that treasure is already in the vault";
            return false;
        }

        parent = temp;

        if (result == 1)
        {
            temp = static_cast<BSTNode*>(temp->left);
            side = 'L';
        }
        else
        {
            temp = static_cast<BSTNode*>(temp->right);
            side = 'R';
        }
    }

    BSTNode* newNode = new BSTNode(id, value);
    newNode->parent = parent;

    if (parent == NULL)
        root = newNode;
    else if (side == 'L')
        parent->left = newNode;
    else
        parent->right = newNode;

    if (id >= nextId)
        nextId = id + 1;

    // nodeA stores the parent's value for the log text
    events.addEvent("INSERT", newNode->id,
        parent == NULL ? -1 : parent->id,
        parent == NULL ? 0 : parent->value,
        -1, value, 0, side);

    lastMessage = "Inserted " + std::to_string(value);
    return true;
}

bool BST::insertValue(int value)
{
    return insertNode(nextId, value);
}

// ---------------------------------------------------------------
// Delete
// ---------------------------------------------------------------
bool BST::deleteNode(int value)
{
    events.clear();
    lastAlarmSteps = 0;

    BSTNode* node = root;

    // find the node
    while (node != NULL)
    {
        lastAlarmSteps++;

        int result = stepCompare(node, value);

        if (result == 0)
            break;
        else if (result == 1)
            node = static_cast<BSTNode*>(node->left);
        else
            node = static_cast<BSTNode*>(node->right);
    }

    if (node == NULL)
    {
        events.addEvent("NOT_FOUND", -1, -1, -1, -1, value);
        lastMessage = "Key " + std::to_string(value) + " not found";
        return false;
    }

    // Case 1: leaf
    if (node->left == NULL && node->right == NULL)
    {
        int parentId = -1;
        char side = 'N';

        if (node->parent != NULL)
        {
            parentId = node->parent->id;
            side = (node->parent->left == node) ? 'L' : 'R';
        }

        events.addEvent("UNLINK", node->id, parentId,
            -1, -1, node->value, 0, side);

        if (node->parent == NULL)
            root = NULL;
        else if (side == 'L')
            node->parent->left = NULL;
        else
            node->parent->right = NULL;

        delete node;
        lastMessage = "Deleted " + std::to_string(value) + " (leaf)";
    }

    // Case 2: one child
    else if (node->left == NULL || node->right == NULL)
    {
        BSTNode* child;

        if (node->left != NULL)
            child = static_cast<BSTNode*>(node->left);
        else
            child = static_cast<BSTNode*>(node->right);

        int parentId = -1;
        char side = 'N';

        if (node->parent != NULL)
        {
            parentId = node->parent->id;
            side = (node->parent->left == node) ? 'L' : 'R';
        }

        // nodeA = id of the child that moves up into the gap
        events.addEvent("UNLINK", node->id, parentId,
            child->id, -1, node->value, 0, side);

        child->parent = node->parent;

        if (node->parent == NULL)
            root = child;
        else if (side == 'L')
            node->parent->left = child;
        else
            node->parent->right = child;

        delete node;
        lastMessage = "Deleted " + std::to_string(value) + " (one child)";
    }

    // Case 3: two children
    else
    {
        // walk down to the inorder successor, showing each step
        BSTNode* successorNode = static_cast<BSTNode*>(node->right);
        events.addEvent("SUCCESSOR_STEP", successorNode->id, -1, -1, -1,
            successorNode->value);

        while (successorNode->left != NULL)
        {
            successorNode = static_cast<BSTNode*>(successorNode->left);
            events.addEvent("SUCCESSOR_STEP", successorNode->id, -1, -1, -1,
                successorNode->value);
        }

        events.addEvent("SUCCESSOR_FOUND", successorNode->id, -1, -1, -1,
            successorNode->value);

        // key = deleted value, result = successor value
        events.addEvent("SWAP_VALUE", -1, -1,
            node->id, successorNode->id,
            node->value, successorNode->value);

        int tempValue = node->value;
        node->value = successorNode->value;
        successorNode->value = tempValue;

        // the successor has no left child
        BSTNode* child = static_cast<BSTNode*>(successorNode->right);

        int parentId = successorNode->parent->id;
        char side = (successorNode->parent->left == successorNode) ? 'L' : 'R';

        events.addEvent("UNLINK", successorNode->id, parentId,
            child == NULL ? -1 : child->id, -1,
            successorNode->value, 0, side);

        if (child != NULL)
            child->parent = successorNode->parent;

        if (side == 'L')
            successorNode->parent->left = child;
        else
            successorNode->parent->right = child;

        delete successorNode;
        lastMessage = "Deleted " + std::to_string(value) + " (two children)";
    }

    if (root != NULL)
        root->parent = NULL;

    return true;
}

// 0 = not found, 1 = leaf, 2 = one child, 3 = two children
int BST::classifyDelete(int key)
{
    BSTNode* node = root;

    while (node != NULL && node->value != key)
    {
        if (key < node->value)
            node = static_cast<BSTNode*>(node->left);
        else
            node = static_cast<BSTNode*>(node->right);
    }

    if (node == NULL)
        return 0;
    if (node->left == NULL && node->right == NULL)
        return 1;
    if (node->left == NULL || node->right == NULL)
        return 2;
    return 3;
}

// ---------------------------------------------------------------
// Min / max / successor / predecessor
// ---------------------------------------------------------------
BSTNode* BST::findMinimum(BSTNode* node)
{
    if (node == NULL)
        return NULL;

    while (node->left != NULL)
        node = static_cast<BSTNode*>(node->left);

    return node;
}

BSTNode* BST::findMaximum(BSTNode* node)
{
    if (node == NULL)
        return NULL;

    while (node->right != NULL)
        node = static_cast<BSTNode*>(node->right);

    return node;
}

BSTNode* BST::successor(BSTNode* node)
{
    if (node == NULL)
        return NULL;

    if (node->right != NULL)
        return findMinimum(static_cast<BSTNode*>(node->right));

    BSTNode* parentNode = static_cast<BSTNode*>(node->parent);

    while (parentNode != NULL && node == parentNode->right)
    {
        node = parentNode;
        parentNode = static_cast<BSTNode*>(parentNode->parent);
    }

    return parentNode;
}

BSTNode* BST::predecessor(BSTNode* node)
{
    if (node == NULL)
        return NULL;

    if (node->left != NULL)
        return findMaximum(static_cast<BSTNode*>(node->left));

    BSTNode* parentNode = static_cast<BSTNode*>(node->parent);

    while (parentNode != NULL && node == parentNode->left)
    {
        node = parentNode;
        parentNode = static_cast<BSTNode*>(parentNode->parent);
    }

    return parentNode;
}

// ---------------------------------------------------------------
// Range Raid (Level 2.4)
// ---------------------------------------------------------------
void BST::freeRangeResults(RangeResult*& head)
{
    while (head != NULL)
    {
        RangeResult* temp = head;
        head = head->next;
        delete temp;
    }
    head = NULL;
}

void BST::clearRangeResults()
{
    freeRangeResults(rangeHead);
    rangeTail = NULL;
}

RangeResult* BST::rangeQuery(int lo, int hi)
{
    events.clear();
    clearRangeResults();
    rangeVisited = 0;
    rangeSkippedSubtrees = 0;
    rangeSkippedNodes = 0;
    rangeInRange = 0;
    lastAlarmSteps = 0;

    if (lo > hi)
    {
        lastMessage = "Invalid range: lo is bigger than hi";
        return NULL;
    }

    RangeRaid(root, lo, hi);

    lastAlarmSteps = rangeVisited;
    lastMessage = "Range [" + std::to_string(lo) + ", " +
        std::to_string(hi) + "]: " + std::to_string(rangeInRange) +
        " treasures found";

    return rangeHead;
}

void BST::RangeRaid(BSTNode* node, int lo, int hi)
{
    if (node == NULL)
        return;

    rangeVisited++;

    events.addEvent("HIGHLIGHT_SUBTREE", node->id,
        -1, -1, -1, node->value, 0, 'N', 'V');

    // left side
    if (node->value > lo)
    {
        RangeRaid(static_cast<BSTNode*>(node->left), lo, hi);
    }
    else if (node->left != NULL)
    {
        BSTNode* skipped = static_cast<BSTNode*>(node->left);
        int skippedCount = countNodes(skipped);

        rangeSkippedSubtrees++;
        rangeSkippedNodes += skippedCount;

        // key = root value of skipped subtree, result = rooms skipped
        events.addEvent("HIGHLIGHT_SUBTREE", skipped->id,
            -1, -1, -1, skipped->value, skippedCount, 'N', 'S');
    }

    // this node
    if (node->value >= lo && node->value <= hi)
    {
        RangeResult* newNode = new RangeResult(node);

        if (rangeHead == NULL)
            rangeHead = rangeTail = newNode;
        else
        {
            rangeTail->next = newNode;
            rangeTail = newNode;
        }

        rangeInRange++;

        events.addEvent("HIGHLIGHT_SUBTREE", node->id,
            -1, -1, -1, node->value, 0, 'N', 'R');
    }

    // right side
    if (node->value < hi)
    {
        RangeRaid(static_cast<BSTNode*>(node->right), lo, hi);
    }
    else if (node->right != NULL)
    {
        BSTNode* skipped = static_cast<BSTNode*>(node->right);
        int skippedCount = countNodes(skipped);

        rangeSkippedSubtrees++;
        rangeSkippedNodes += skippedCount;

        events.addEvent("HIGHLIGHT_SUBTREE", skipped->id,
            -1, -1, -1, skipped->value, skippedCount, 'N', 'S');
    }
}

int BST::countRangeVisits(BSTNode* node, int lo, int hi)
{
    if (node == NULL || lo > hi)
        return 0;

    int c = 1;

    if (node->value > lo)
        c += countRangeVisits(static_cast<BSTNode*>(node->left), lo, hi);

    if (node->value < hi)
        c += countRangeVisits(static_cast<BSTNode*>(node->right), lo, hi);

    return c;
}

int BST::rangeOptimalSteps(int lo, int hi)
{
    return countRangeVisits(root, lo, hi);
}

RangeResult* BST::getRangeResults()
{
    return rangeHead;
}

int BST::getRangeVisited()
{
    return rangeVisited;
}

int BST::getRangeSkippedSubtrees()
{
    return rangeSkippedSubtrees;
}

int BST::getRangeSkippedNodes()
{
    return rangeSkippedNodes;
}

int BST::getRangeInRange()
{
    return rangeInRange;
}

// ---------------------------------------------------------------
// LCA / kth smallest
// ---------------------------------------------------------------
BSTNode* BST::lcaBST(int value1, int value2)
{
    BSTNode* temp = root;

    // check that both values exist
    BSTNode* first = root;
    while (first != NULL && first->value != value1)
    {
        if (value1 < first->value)
            first = static_cast<BSTNode*>(first->left);
        else
            first = static_cast<BSTNode*>(first->right);
    }

    if (first == NULL)
        return NULL;

    BSTNode* second = root;
    while (second != NULL && second->value != value2)
    {
        if (value2 < second->value)
            second = static_cast<BSTNode*>(second->left);
        else
            second = static_cast<BSTNode*>(second->right);
    }

    if (second == NULL)
        return NULL;

    while (temp != NULL)
    {
        if (value1 < temp->value && value2 < temp->value)
            temp = static_cast<BSTNode*>(temp->left);
        else if (value1 > temp->value && value2 > temp->value)
            temp = static_cast<BSTNode*>(temp->right);
        else
            return temp;
    }

    return NULL;
}

BSTNode* BST::kthSmallest(int k)
{
    if (k <= 0)
        return NULL;

    BSTNode* temp = findMinimum(root);

    for (int i = 1; i < k && temp != NULL; i++)
        temp = successor(temp);

    return temp;
}

// ---------------------------------------------------------------
// Height / counting / depth / lookup   (leaf height = 1)
// ---------------------------------------------------------------
int BST::height(BSTNode* node)
{
    if (node == NULL)
        return 0;

    int leftHeight = height(static_cast<BSTNode*>(node->left));
    int rightHeight = height(static_cast<BSTNode*>(node->right));

    if (leftHeight > rightHeight)
        return leftHeight + 1;

    return rightHeight + 1;
}

int BST::countNodes(BSTNode* node)
{
    if (node == NULL)
        return 0;

    return 1 + countNodes(static_cast<BSTNode*>(node->left))
        + countNodes(static_cast<BSTNode*>(node->right));
}

int BST::countNodes()
{
    return countNodes(root);
}

int BST::depthOf(BSTNode* node)
{
    int d = 0;

    while (node != NULL)
    {
        d++;
        node = static_cast<BSTNode*>(node->parent);
    }

    return d;
}

BSTNode* BST::findById(BSTNode* node, int id)
{
    if (node == NULL)
        return NULL;

    if (node->id == id)
        return node;

    BSTNode* found = findById(static_cast<BSTNode*>(node->left), id);
    if (found != NULL)
        return found;

    return findById(static_cast<BSTNode*>(node->right), id);
}

BSTNode* BST::getNodeById(int id)
{
    return findById(root, id);
}

// deletes a subtree without recursion (safe for very tall trees)
void BST::clearTree(BSTNode* node)
{
    if (node == NULL)
        return;

    TreeNode* stop = node->parent;
    BSTNode* cur = node;

    while (cur != NULL && cur != stop)
    {
        if (cur->left != NULL)
            cur = static_cast<BSTNode*>(cur->left);
        else if (cur->right != NULL)
            cur = static_cast<BSTNode*>(cur->right);
        else
        {
            BSTNode* up = static_cast<BSTNode*>(cur->parent);

            if (up != NULL)
            {
                if (up->left == cur)
                    up->left = NULL;
                else if (up->right == cur)
                    up->right = NULL;
            }

            delete cur;
            cur = up;
        }
    }
}

// ---------------------------------------------------------------
// Optimal-step helpers
// ---------------------------------------------------------------
int BST::capacityFor(int optimal)
{
    return (3 * optimal + 1) / 2 + 2;
}

int BST::searchPathLength(int key)
{
    int steps = 0;
    BSTNode* node = root;

    while (node != NULL)
    {
        steps++;

        if (node->value == key)
            break;

        if (key < node->value)
            node = static_cast<BSTNode*>(node->left);
        else
            node = static_cast<BSTNode*>(node->right);
    }

    return steps;
}

int BST::insertPathLength(int key)
{
    int steps = 0;
    BSTNode* node = root;

    while (node != NULL)
    {
        steps++;

        if (node->value == key)
            return -1;

        if (key < node->value)
            node = static_cast<BSTNode*>(node->left);
        else
            node = static_cast<BSTNode*>(node->right);
    }

    return steps;
}

// ---------------------------------------------------------------
// Level 2.5 boss
// ---------------------------------------------------------------
void BST::generateLazyArchitect(int count)
{
    reset();

    if (count < 1)
        return;

    BSTNode* parent = NULL;

    for (int i = 1; i <= count; i++)
    {
        BSTNode* newNode = new BSTNode(i, i);

        if (root == NULL)
        {
            root = newNode;
            events.addEvent("INSERT", newNode->id,
                -1, 0, -1, newNode->value, 0, 'N');
        }
        else
        {
            parent->right = newNode;
            newNode->parent = parent;

            events.addEvent("INSERT", newNode->id,
                parent->id, parent->value, -1, newNode->value, 0, 'R');
        }

        parent = newNode;
    }

    nextId = count + 1;
}

// round / shrinkPerRound make the alarm capacity shrink each attempt
BSTNode* BST::bossSearch(int target, int& alarmCount,
    int& remainingCapacity, int round, int shrinkPerRound)
{
    events.clear();
    alarmCount = 0;
    lastAlarmSteps = 0;

    int count = countNodes(root);

    // steps needed in an ideal balanced BST
    int optimalSteps = 0;
    int power = 1;

    while (power < count + 1)
    {
        power *= 2;
        optimalSteps++;
    }

    remainingCapacity = capacityFor(optimalSteps) - round * shrinkPerRound;
    if (remainingCapacity < 1)
        remainingCapacity = 1;

    BSTNode* temp = root;

    while (temp != NULL)
    {
        if (remainingCapacity == 0)
        {
            events.addEvent("BOSS_LOSE",
                -1, -1, -1, -1, target, 0, 'N', 'A');
            lastMessage = "Alarm full - the guards arrived";
            return NULL;
        }

        alarmCount++;
        remainingCapacity--;
        lastAlarmSteps = alarmCount;

        int result = stepCompare(temp, target);

        if (result == 0)
        {
            events.addEvent("BOSS_WIN", temp->id,
                -1, -1, -1, target, 1);
            lastMessage = "Boss target found";
            return temp;
        }

        if (result == 1)
            temp = static_cast<BSTNode*>(temp->left);
        else
            temp = static_cast<BSTNode*>(temp->right);
    }

    events.addEvent("BOSS_MISSING", -1, -1, -1, -1, target, 0);
    lastMessage = "Boss target does not exist";
    return NULL;
}

// ---------------------------------------------------------------
// Level generation (seeded)
// ---------------------------------------------------------------
int BST::buildFromKeys(const int keys[], int n)
{
    reset();

    int inserted = 0;

    for (int i = 0; i < n; i++)
    {
        if (insertNode(nextId, keys[i]))
            inserted++;
    }

    events.clear();
    lastAlarmSteps = 0;
    lastMessage = "Built tree with " + std::to_string(inserted) + " rooms";
    return inserted;
}

void BST::generateRandom(unsigned seed, int n, int skewPercent)
{
    reset();

    if (n < 1)
        return;
    if (n > 99)
        n = 99;
    if (skewPercent < 0)
        skewPercent = 0;
    if (skewPercent > 100)
        skewPercent = 100;

    unsigned state = seed;

    // pick n distinct keys from 1..99 (shuffle then take the first n)
    int pool[99];
    for (int i = 0; i < 99; i++)
        pool[i] = i + 1;

    for (int i = 98; i > 0; i--)
    {
        int j = nextRandom(state) % (i + 1);
        int t = pool[i];
        pool[i] = pool[j];
        pool[j] = t;
    }

    int keys[99];
    for (int i = 0; i < n; i++)
        keys[i] = pool[i];

    // sort ascending (insertion sort)
    for (int i = 1; i < n; i++)
    {
        int k = keys[i];
        int j = i - 1;

        while (j >= 0 && keys[j] > k)
        {
            keys[j + 1] = keys[j];
            j--;
        }
        keys[j + 1] = k;
    }

    // shuffle each position with probability (100 - skew)%
    for (int i = 0; i < n; i++)
    {
        if ((int)(nextRandom(state) % 100) >= skewPercent)
        {
            int j = nextRandom(state) % n;
            int t = keys[i];
            keys[i] = keys[j];
            keys[j] = t;
        }
    }

    for (int i = 0; i < n; i++)
        insertNode(nextId, keys[i]);

    events.clear();
    lastAlarmSteps = 0;
    lastMessage = "Generated tree with " + std::to_string(n) + " rooms";
}

// ---------------------------------------------------------------
// Validator (no recursion: walks the tree in sorted order)
// ---------------------------------------------------------------
bool BST::isValidBST()
{
    if (root == NULL)
        return true;

    if (root->parent != NULL)
        return false;

    const int LIMIT = 1000000;   // protects against cycles
    int count = 0;

    // go to the leftmost node
    BSTNode* cur = root;
    while (cur->left != NULL)
    {
        if (++count > LIMIT)
            return false;
        cur = static_cast<BSTNode*>(cur->left);
    }

    BSTNode* prev = NULL;
    count = 0;

    while (cur != NULL)
    {
        if (++count > LIMIT)
            return false;

        // parent links must agree with child links
        if (cur->left != NULL && cur->left->parent != cur)
            return false;
        if (cur->right != NULL && cur->right->parent != cur)
            return false;

        // inorder keys must strictly increase
        if (prev != NULL && prev->value >= cur->value)
            return false;

        prev = cur;
        cur = successor(cur);
    }

    return true;
}

// ---------------------------------------------------------------
// Save / load (preorder with '#' for null)
// ---------------------------------------------------------------
void BST::writeNode(BSTNode* node, std::string& out)
{
    if (node == NULL)
    {
        out += "# ";
        return;
    }

    out += std::to_string(node->value) + " ";
    writeNode(static_cast<BSTNode*>(node->left), out);
    writeNode(static_cast<BSTNode*>(node->right), out);
}

std::string BST::treeToString()
{
    std::string out = "";
    writeNode(root, out);

    if (!out.empty() && out[out.size() - 1] == ' ')
        out.erase(out.size() - 1);

    return out;
}

BSTNode* BST::parseNode(const std::string& s, int& pos,
    BSTNode* parent, bool& ok)
{
    int len = (int)s.size();

    while (pos < len && s[pos] == ' ')
        pos++;

    if (pos >= len)
    {
        ok = false;
        return NULL;
    }

    int start = pos;
    while (pos < len && s[pos] != ' ')
        pos++;

    std::string token = s.substr(start, pos - start);

    if (token == "#")
        return NULL;

    int i = 0;
    bool negative = false;

    if (token[0] == '-')
    {
        negative = true;
        i = 1;
    }

    int size = (int)token.size();

    if (i >= size || size - i > 9)
    {
        ok = false;
        return NULL;
    }

    int v = 0;
    for (; i < size; i++)
    {
        if (token[i] < '0' || token[i] > '9')
        {
            ok = false;
            return NULL;
        }
        v = v * 10 + (token[i] - '0');
    }
    if (negative)
        v = -v;

    BSTNode* node = new BSTNode(nextId++, v);
    node->parent = parent;

    node->left = parseNode(s, pos, node, ok);
    if (!ok)
        return node;

    node->right = parseNode(s, pos, node, ok);
    return node;
}

bool BST::loadFromString(const std::string& text)
{
    reset();

    int pos = 0;
    bool ok = true;

    BSTNode* newRoot = parseNode(text, pos, NULL, ok);

    int len = (int)text.size();
    while (pos < len && text[pos] == ' ')
        pos++;

    if (pos < len)
        ok = false;             // extra tokens after the tree

    root = newRoot;

    if (!ok || !isValidBST())
    {
        reset();
        lastMessage = "Could not load: the saved tree is damaged";
        return false;
    }

    lastMessage = "Loaded tree with " + std::to_string(countNodes()) + " rooms";
    return true;
}

// ---------------------------------------------------------------
// SFML-friendly data
// ---------------------------------------------------------------
int BST::collectNodes(NodeInfo out[], int maxN)
{
    int n = countNodes();

    if (n == 0 || maxN <= 0)
        return 0;

    BSTNode** queue = new BSTNode * [n];
    int* depths = new int[n];

    int head = 0;
    int tail = 0;
    int written = 0;

    queue[tail] = root;
    depths[tail] = 1;
    tail++;

    while (head < tail && written < maxN)
    {
        BSTNode* cur = queue[head];
        int d = depths[head];
        head++;

        NodeInfo& info = out[written];
        info.id = cur->id;
        info.value = cur->value;
        info.depth = d;
        info.height = height(cur);
        info.parentId = (cur->parent != NULL) ? cur->parent->id : -1;
        info.leftId = (cur->left != NULL) ? cur->left->id : -1;
        info.rightId = (cur->right != NULL) ? cur->right->id : -1;
        info.x = cur->x;
        info.y = cur->y;
        written++;

        if (cur->left != NULL)
        {
            queue[tail] = static_cast<BSTNode*>(cur->left);
            depths[tail] = d + 1;
            tail++;
        }
        if (cur->right != NULL)
        {
            queue[tail] = static_cast<BSTNode*>(cur->right);
            depths[tail] = d + 1;
            tail++;
        }
    }

    delete[] queue;
    delete[] depths;

    return written;
}

// x = position in sorted order, y = depth (same idea as setTreePositions)
void BST::computeLayout(float left, float right, float top, float bottom)
{
    int n = countNodes();

    if (n == 0)
        return;

    int h = height(root);

    BSTNode* cur = findMinimum(root);
    int i = 0;

    while (cur != NULL)
    {
        if (n > 1)
            cur->x = left + i * (right - left) / (float)(n - 1);
        else
            cur->x = (left + right) / 2.0f;

        int d = depthOf(cur);

        if (h > 1)
            cur->y = top + (d - 1) * (bottom - top) / (float)(h - 1);
        else
            cur->y = top;

        i++;
        cur = successor(cur);
    }
}

// returns number of rooms on the path (target included), -1 if not found
// outIds should hold at least height(root) entries
int BST::pathTo(int key, int outIds[], int maxN)
{
    int count = 0;
    BSTNode* node = root;

    while (node != NULL)
    {
        if (count < maxN)
            outIds[count] = node->id;
        count++;

        if (node->value == key)
            return count;

        if (key < node->value)
            node = static_cast<BSTNode*>(node->left);
        else
            node = static_cast<BSTNode*>(node->right);
    }

    return -1;
}

void BST::collectIds(BSTNode* node, int out[], int maxN, int& count)
{
    if (node == NULL || count >= maxN)
        return;

    out[count++] = node->id;
    collectIds(static_cast<BSTNode*>(node->left), out, maxN, count);
    collectIds(static_cast<BSTNode*>(node->right), out, maxN, count);
}

// ids of every node under subRoot (including subRoot), preorder
int BST::collectSubtreeIds(BSTNode* subRoot, int out[], int maxN)
{
    int count = 0;
    collectIds(subRoot, out, maxN, count);
    return count;
}