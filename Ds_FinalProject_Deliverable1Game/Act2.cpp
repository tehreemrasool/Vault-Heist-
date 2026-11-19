
#include "Act2.h"

// Constructor
BST::BST()
{
    root = NULL;
}

// Destructor
BST::~BST()
{
    clearTree(root);
}

// Return root
BSTNode* BST::getRoot()
{
    return root;
}

// Return event list
EventNode* BST::getEvents()
{
    return events.getHead();
}

// Clear event list
void BST::clearEvents()
{
    events.clear();
}

// Search for a value
BSTNode* BST::searchNode(int value, int& alarm_count)
{
    events.clear();
    alarm_count = 0;

    BSTNode* temp = root;

    while (temp != NULL)
    {
        alarm_count++;

        events.addEvent("VISIT", temp->id);

        int result;

        if (temp->value < value)
            result = -1;
        else if (temp->value > value)
            result = 1;
        else
            result = 0;

        events.addEvent("COMPARE", temp->id, -1, -1, -1,
            value, result);

        if (result == 0)
            return temp;

        else if (result == 1)
            temp = static_cast<BSTNode*>(temp->left);

        else
            temp = static_cast<BSTNode*>(temp->right);
    }

    return NULL;
}

// Insert a new node
bool BST::insertNode(int id, int value)
{
    events.clear();

    BSTNode* temp = root;
    BSTNode* parent = NULL;
    char side = 'N';

    // Find insertion position and reject duplicates
    while (temp != NULL)
    {
        events.addEvent("VISIT", temp->id);

        int result;

        if (temp->value < value)
            result = -1;
        else if (temp->value > value)
            result = 1;
        else
            result = 0;

        events.addEvent("COMPARE", temp->id, -1, -1, -1,
            value, result);

        if (result == 0)
            return false;

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

    // Create new node
    BSTNode* newNode = new BSTNode(id, value);
    newNode->parent = parent;

    if (parent == NULL)
        root = newNode;
    else if (side == 'L')
        parent->left = newNode;
    else
        parent->right = newNode;

    events.addEvent("INSERT", newNode->id,
        parent == NULL ? -1 : parent->id,
        -1, -1, value, 0, side);

    return true;
}

// Delete a node
bool BST::deleteNode(int value)
{
    events.clear();

    BSTNode* node = root;

    // Find the node to delete
    while (node != NULL)
    {
        events.addEvent("VISIT", node->id);

        int result;

        if (node->value < value)
            result = -1;
        else if (node->value > value)
            result = 1;
        else
            result = 0;

        events.addEvent("COMPARE", node->id, -1, -1, -1,
            value, result);

        if (result == 0)
            break;

        else if (result == 1)
            node = static_cast<BSTNode*>(node->left);

        else
            node = static_cast<BSTNode*>(node->right);
    }

    if (node == NULL)
        return false;

    // Case 1: No children
    if (node->left == NULL && node->right == NULL)
    {
        int parentId = -1;
        char side = 'N';

        if (node->parent != NULL)
        {
            parentId = node->parent->id;

            if (node->parent->left == node)
                side = 'L';
            else
                side = 'R';
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
    }

    // Case 2: Only one child
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

            if (node->parent->left == node)
                side = 'L';
            else
                side = 'R';
        }

        events.addEvent("UNLINK", node->id, parentId,
            -1, -1, node->value, 0, side);

        child->parent = node->parent;

        if (node->parent == NULL)
            root = child;
        else if (side == 'L')
            node->parent->left = child;
        else
            node->parent->right = child;

        delete node;
    }

    // Case 3: Two children
    else
    {
        // Find inorder successor
        BSTNode* successorNode =
            static_cast<BSTNode*>(node->right);

        while (successorNode->left != NULL)
        {
            successorNode =
                static_cast<BSTNode*>(successorNode->left);
        }

        // Record value swap
        events.addEvent("SWAP_VALUE", -1, -1,
            node->id, successorNode->id);

        int tempValue = node->value;
        node->value = successorNode->value;
        successorNode->value = tempValue;

        // Successor has no left child
        BSTNode* child =
            static_cast<BSTNode*>(successorNode->right);

        int parentId = successorNode->parent->id;
        char side;

        if (successorNode->parent->left == successorNode)
            side = 'L';
        else
            side = 'R';

        events.addEvent("UNLINK", successorNode->id,
            parentId, -1, -1,
            successorNode->value, 0, side);

        if (child != NULL)
            child->parent = successorNode->parent;

        if (side == 'L')
            successorNode->parent->left = child;
        else
            successorNode->parent->right = child;

        delete successorNode;
    }

    if (root != NULL)
        root->parent = NULL;

    return true;
}

// Find minimum value
BSTNode* BST::findMinimum(BSTNode* node)
{
    if (node == NULL)
        return NULL;

    while (node->left != NULL)
        node = static_cast<BSTNode*>(node->left);

    return node;
}

// Find maximum value
BSTNode* BST::findMaximum(BSTNode* node)
{
    if (node == NULL)
        return NULL;

    while (node->right != NULL)
        node = static_cast<BSTNode*>(node->right);

    return node;
}

// Find inorder successor
BSTNode* BST::successor(BSTNode* node)
{
    if (node == NULL)
        return NULL;

    if (node->right != NULL)
        return findMinimum(static_cast<BSTNode*>(node->right));

    BSTNode* parentNode =
        static_cast<BSTNode*>(node->parent);

    while (parentNode != NULL && node == parentNode->right)
    {
        node = parentNode;
        parentNode = static_cast<BSTNode*>(parentNode->parent);
    }

    return parentNode;
}

// Find inorder predecessor
BSTNode* BST::predecessor(BSTNode* node)
{
    if (node == NULL)
        return NULL;

    if (node->left != NULL)
        return findMaximum(static_cast<BSTNode*>(node->left));

    BSTNode* parentNode =
        static_cast<BSTNode*>(node->parent);

    while (parentNode != NULL && node == parentNode->left)
    {
        node = parentNode;
        parentNode = static_cast<BSTNode*>(parentNode->parent);
    }

    return parentNode;
}

// Range Raid
void BST::RangeRaid(BSTNode* node, int lo, int hi,
    RangeResult*& head, RangeResult*& tail,
    bool firstCall)
{
    // Initialize only on the first call
    if (firstCall)
    {
        events.clear();
        head = NULL;
        tail = NULL;
    }

    if (node == NULL || lo > hi)
        return;

    events.addEvent("HIGHLIGHT_SUBTREE", node->id,
        -1, -1, -1, node->value, 0, 'N', 'V');

    // Search left subtree
    if (node->value > lo)
    {
        RangeRaid(static_cast<BSTNode*>(node->left),
            lo, hi, head, tail, false);
    }
    else if (node->left != NULL)
    {
        events.addEvent("HIGHLIGHT_SUBTREE",
            node->left->id, -1, -1, -1,
            0, 0, 'N', 'S');
    }

    // Add current node if it is in range
    if (node->value >= lo && node->value <= hi)
    {
        RangeResult* newNode = new RangeResult(node);

        if (head == NULL)
            head = tail = newNode;
        else
        {
            tail->next = newNode;
            tail = newNode;
        }

        events.addEvent("HIGHLIGHT_SUBTREE", node->id,
            -1, -1, -1, node->value, 0, 'N', 'R');
    }

    // Search right subtree
    if (node->value < hi)
    {
        RangeRaid(static_cast<BSTNode*>(node->right),
            lo, hi, head, tail, false);
    }
    else if (node->right != NULL)
    {
        events.addEvent("HIGHLIGHT_SUBTREE",
            node->right->id, -1, -1, -1,
            0, 0, 'N', 'S');
    }
}

// Lowest Common Ancestor
BSTNode* BST::lcaBST(int value1, int value2)
{
    BSTNode* temp = root;

    // Check that both values exist
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

    // Find lowest common ancestor
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

// Find kth smallest value
BSTNode* BST::kthSmallest(int k)
{
    if (k <= 0)
        return NULL;

    BSTNode* temp = findMinimum(root);

    for (int i = 1; i < k && temp != NULL; i++)
        temp = successor(temp);

    return temp;
}

// Calculate tree height
int BST::height(BSTNode* node)
{
    if (node == NULL)
        return 0;

    int leftHeight =
        height(static_cast<BSTNode*>(node->left));

    int rightHeight =
        height(static_cast<BSTNode*>(node->right));

    if (leftHeight > rightHeight)
        return leftHeight + 1;

    return rightHeight + 1;
}

// Delete all tree nodes
void BST::clearTree(BSTNode* node)
{
    if (node == NULL)
        return;

    clearTree(static_cast<BSTNode*>(node->left));
    clearTree(static_cast<BSTNode*>(node->right));

    delete node;
}

// Generate the Lazy Architect's right-skewed BST
void BST::generateLazyArchitect(int count)
{
    events.clear();

    // Remove the previous tree
    clearTree(root);
    root = NULL;

    BSTNode* parent = NULL;

    // Insert sorted deliveries: 1, 2, 3, ..., count
    for (int i = 1; i <= count; i++)
    {
        BSTNode* newNode = new BSTNode(i, i);

        if (root == NULL)
        {
            root = newNode;

            events.addEvent("INSERT", newNode->id,
                -1, -1, -1, newNode->value, 0, 'N');
        }
        else
        {
            parent->right = newNode;
            newNode->parent = parent;

            events.addEvent("INSERT", newNode->id,
                parent->id, -1, -1, newNode->value, 0, 'R');
        }

        parent = newNode;
    }
}

// Search for the boss target with a limited alarm capacity
// Search for the boss target with a limited alarm capacity
BSTNode* BST::bossSearch(int target, int& alarmCount,
    int& remainingCapacity)
{
    events.clear();
    alarmCount = 0;

    // Count nodes in the tree
    int count = 0;
    BSTNode* temp = root;

    while (temp != NULL)
    {
        count++;
        temp = static_cast<BSTNode*>(temp->right);
    }

    // Calculate ideal balanced BST search steps
    int optimalSteps = 0;
    int power = 1;

    while (power < count + 1)
    {
        power *= 2;
        optimalSteps++;
    }

    // Calculate initial alarm capacity
    remainingCapacity = (3 * optimalSteps + 1) / 2 + 2;

    temp = root;

    while (temp != NULL)
    {
        // Check alarm before visiting the next node
        if (remainingCapacity == 0)
        {
            events.addEvent("BOSS_LOSE",
                -1, -1, -1, -1, target, 0, 'N', 'A');

            return NULL;
        }

        alarmCount++;
        remainingCapacity--;

        events.addEvent("VISIT", temp->id);

        int result;

        if (temp->value < target)
            result = -1;
        else if (temp->value > target)
            result = 1;
        else
            result = 0;

        events.addEvent("COMPARE", temp->id, -1, -1, -1,
            target, result);

        if (result == 0)
        {
            events.addEvent("BOSS_WIN", temp->id,
                -1, -1, -1, target, 1);

            return temp;
        }

        if (result == 1)
            temp = static_cast<BSTNode*>(temp->left);
        else
            temp = static_cast<BSTNode*>(temp->right);
    }

    // Search reached NULL, so the target does not exist
    events.addEvent("BOSS_MISSING",
        -1, -1, -1, -1, target, 0);

    return NULL;
}