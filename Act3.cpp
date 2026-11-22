#include "Act3.h"



AVLTree::AVLTree()
{
    idBase = nextId;
    recordEvents = true;
    validationEpoch = 0;
    lastRotations = 0;
    totalRotations = 0;
}
bool AVLTree::isSpace(char c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}
AVLNode* AVLTree::getAVLRoot()
{
    return static_cast<AVLNode*>(root);
}

AVLNode* AVLTree::getAVLNodeById(int id)
{
    return static_cast<AVLNode*>(getNodeById(id));
}

int AVLTree::nodeCount()
{
    return (root == NULL) ? 0 : getAVLRoot()->size;
}

int AVLTree::treeHeight()
{
    return (root == NULL) ? 0 : getAVLRoot()->height;
}

bool AVLTree::containsKey(int key)
{
    AVLNode* node = getAVLRoot();
    while (node != NULL)
    {
        if (key == node->value)
            return true;
        if (key < node->value)
            node = static_cast<AVLNode*>(node->left);
        else
            node = static_cast<AVLNode*>(node->right);
    }
    return false;
}

void AVLTree::resetTree()
{
    reset();
    nextId = idBase;
    lastRotations = 0;
    totalRotations = 0;
    clearEvents();
}

void AVLTree::setRecordEvents(bool on) { recordEvents = on; }
bool AVLTree::getRecordEvents() { return recordEvents; }
int  AVLTree::getLastRotationCount() { return lastRotations; }
int  AVLTree::getTotalRotations() { return totalRotations; }

void AVLTree::resetRotationCounters()
{
    lastRotations = 0;
    totalRotations = 0;
}

void AVLTree::emit(const char* type, int nodeId, int parentId,
    int nodeA, int nodeB, int key, int result, char side, char mode)
{
    if (recordEvents)
        events.addEvent(type, nodeId, parentId, nodeA, nodeB,
            key, result, side, mode);
}

void AVLTree::update(AVLNode* node)
{
    if (node == NULL)
        return;

    int oldHeight = node->height;
    int oldSize = node->size;

    AVLNode* left = static_cast<AVLNode*>(node->left);
    AVLNode* right = static_cast<AVLNode*>(node->right);

    int leftHeight = (left == NULL) ? 0 : left->height;
    int rightHeight = (right == NULL) ? 0 : right->height;
    int leftSize = (left == NULL) ? 0 : left->size;
    int rightSize = (right == NULL) ? 0 : right->size;

    node->height = 1 + ((leftHeight > rightHeight) ? leftHeight : rightHeight);
    node->size = 1 + leftSize + rightSize;

    if (oldHeight != node->height)
        emit("UPDATE_HEIGHT", node->id, -1, -1, -1, oldHeight, node->height);

    if (oldSize != node->size)
        emit("UPDATE_SIZE", node->id, -1, -1, -1, oldSize, node->size);
}

void AVLTree::updateUpward(AVLNode* from)
{
    AVLNode* p = from;
    while (p != NULL)
    {
        update(p);
        p = static_cast<AVLNode*>(p->parent);
    }
}

void AVLTree::recompute(AVLNode* node)
{
    if (node == NULL)
        return;

    AVLNode* left = static_cast<AVLNode*>(node->left);
    AVLNode* right = static_cast<AVLNode*>(node->right);

    recompute(left);
    recompute(right);

    int leftHeight = (left == NULL) ? 0 : left->height;
    int rightHeight = (right == NULL) ? 0 : right->height;
    int leftSize = (left == NULL) ? 0 : left->size;
    int rightSize = (right == NULL) ? 0 : right->size;

    node->height = 1 + ((leftHeight > rightHeight) ? leftHeight : rightHeight);
    node->size = 1 + leftSize + rightSize;
}

int AVLTree::balanceFactor(AVLNode* node)
{
    if (node == NULL)
        return 0;

    AVLNode* left = static_cast<AVLNode*>(node->left);
    AVLNode* right = static_cast<AVLNode*>(node->right);

    int leftHeight = (left == NULL) ? 0 : left->height;
    int rightHeight = (right == NULL) ? 0 : right->height;
    return leftHeight - rightHeight;
}

AVLNode* AVLTree::rotL(AVLNode* node)
{
    if (node == NULL || node->right == NULL)
        return node;

    AVLNode* pivot = static_cast<AVLNode*>(node->right);
    AVLNode* middle = static_cast<AVLNode*>(pivot->left);
    AVLNode* parent = static_cast<AVLNode*>(node->parent);

    emit("ROTATE_STEP", node->id,
        parent == NULL ? -1 : parent->id,
        pivot->id, middle == NULL ? -1 : middle->id,
        node->value, pivot->value, 'N', 'L');

    pivot->parent = parent;
    if (parent == NULL)
        root = pivot;
    else if (parent->left == node)
        parent->left = pivot;
    else
        parent->right = pivot;

    pivot->left = node;
    node->parent = pivot;
    node->right = middle;
    if (middle != NULL)
        middle->parent = node;

    update(node);
    update(pivot);

    lastRotations++;
    totalRotations++;
    return pivot;
}

AVLNode* AVLTree::rotR(AVLNode* node)
{
    if (node == NULL || node->left == NULL)
        return node;

    AVLNode* pivot = static_cast<AVLNode*>(node->left);
    AVLNode* middle = static_cast<AVLNode*>(pivot->right);
    AVLNode* parent = static_cast<AVLNode*>(node->parent);

    emit("ROTATE_STEP", node->id,
        parent == NULL ? -1 : parent->id,
        pivot->id, middle == NULL ? -1 : middle->id,
        node->value, pivot->value, 'N', 'R');

    pivot->parent = parent;
    if (parent == NULL)
        root = pivot;
    else if (parent->left == node)
        parent->left = pivot;
    else
        parent->right = pivot;

    pivot->right = node;
    node->parent = pivot;
    node->left = middle;
    if (middle != NULL)
        middle->parent = node;

    update(node);
    update(pivot);

    lastRotations++;
    totalRotations++;
    return pivot;
}

// public versions: rotate AND fix the heights/sizes of every ancestor
AVLNode* AVLTree::rotateLeft(AVLNode* node)
{
    AVLNode* result = rotL(node);
    if (result != NULL && result != node)
        updateUpward(static_cast<AVLNode*>(result->parent));
    return result;
}

AVLNode* AVLTree::rotateRight(AVLNode* node)
{
    AVLNode* result = rotR(node);
    if (result != NULL && result != node)
        updateUpward(static_cast<AVLNode*>(result->parent));
    return result;
}

int AVLTree::detectRotationCase(AVLNode* node)
{
    if (node == NULL)
        return ROT_NONE;

    int factor = balanceFactor(node);

    if (factor > 1)
    {
        AVLNode* left = static_cast<AVLNode*>(node->left);
        return (balanceFactor(left) >= 0) ? ROT_LL : ROT_LR;
    }
    if (factor < -1)
    {
        AVLNode* right = static_cast<AVLNode*>(node->right);
        return (balanceFactor(right) <= 0) ? ROT_RR : ROT_RL;
    }
    return ROT_NONE;
}

int AVLTree::detectRotationCaseById(int id)
{
    return detectRotationCase(getAVLNodeById(id));
}

AVLNode* AVLTree::rebalance(AVLNode* node)
{
    if (node == NULL)
        return NULL;

    update(node);

    int rotCase = detectRotationCase(node);
    if (rotCase == ROT_NONE)
        return node;

    int factor = balanceFactor(node);
    AVLNode* left = static_cast<AVLNode*>(node->left);
    AVLNode* right = static_cast<AVLNode*>(node->right);

    int pivotId = -1;
    if (rotCase == ROT_LL)
        pivotId = left->id;
    else if (rotCase == ROT_RR)
        pivotId = right->id;
    else if (rotCase == ROT_LR)
        pivotId = left->right->id;
    else
        pivotId = right->left->id;

    emit("BALANCE_ALERT", node->id, -1, -1, -1, factor, 0);
    emit("ROTATE", node->id,
        node->parent == NULL ? -1 : node->parent->id,
        pivotId, -1, factor, rotCase);

    if (rotCase == ROT_LL)
        return rotR(node);

    if (rotCase == ROT_RR)
        return rotL(node);

    if (rotCase == ROT_LR)
    {
        rotL(left);
        return rotR(node);
    }

    rotR(right);
    return rotL(node);
}

AVLNode* AVLTree::insertRec(AVLNode* node, int key,
    AVLNode* parent, bool& inserted)
{
    if (node == NULL)
    {
        AVLNode* newNode = new AVLNode(nextId++, key);
        newNode->parent = parent;
        emit("INSERT", newNode->id,
            parent == NULL ? -1 : parent->id,
            parent == NULL ? 0 : parent->value,
            -1, key, 0,
            parent == NULL ? 'N' : (key < parent->value ? 'L' : 'R'));
        inserted = true;
        return newNode;
    }

    if (recordEvents)
        stepCompare(node, key);

    if (key == node->value)
        return node;

    if (key < node->value)
    {
        node->left = insertRec(static_cast<AVLNode*>(node->left),
            key, node, inserted);
        if (!inserted)
            return node;
        node->left->parent = node;
    }
    else
    {
        node->right = insertRec(static_cast<AVLNode*>(node->right),
            key, node, inserted);
        if (!inserted)
            return node;
        node->right->parent = node;
    }

    return rebalance(node);
}

bool AVLTree::insert(int key)
{
    clearEvents();
    lastRotations = 0;

    bool inserted = false;
    root = insertRec(getAVLRoot(), key, NULL, inserted);

    if (root != NULL)
        root->parent = NULL;

    if (!inserted)
    {
        emit("DUPLICATE_REJECTED", -1, -1, -1, -1, key);
        lastMessage = "Duplicate key rejected.";
    }
    else
        lastMessage = "Key inserted.";

    return inserted;
}

bool AVLTree::insertValue(int value)
{
    return insert(value);
}

AVLNode* AVLTree::deleteRec(AVLNode* node, int key, bool& deleted,
    bool emitCompare)
{
    if (node == NULL)
        return NULL;

    if (emitCompare && recordEvents)
        stepCompare(node, key);

    if (key < node->value)
    {
        node->left = deleteRec(static_cast<AVLNode*>(node->left),
            key, deleted, emitCompare);
        if (!deleted)
            return node;
        if (node->left != NULL)
            node->left->parent = node;
    }
    else if (key > node->value)
    {
        node->right = deleteRec(static_cast<AVLNode*>(node->right),
            key, deleted, emitCompare);
        if (!deleted)
            return node;
        if (node->right != NULL)
            node->right->parent = node;
    }
    else
    {
        deleted = true;

        if (node->left != NULL && node->right != NULL)
        {
           
            AVLNode* successor = static_cast<AVLNode*>(node->right);
            while (successor->left != NULL)
                successor = static_cast<AVLNode*>(successor->left);

            int successorValue = successor->value;

            emit("SWAP_VALUE", -1, -1, node->id, successor->id,
                node->value, successorValue);

            node->value = successorValue;

            bool successorDeleted = false;
            node->right = deleteRec(static_cast<AVLNode*>(node->right),
                successorValue, successorDeleted, false);
            if (node->right != NULL)
                node->right->parent = node;
        }
        else
        {
           
            AVLNode* child = (node->left != NULL)
                ? static_cast<AVLNode*>(node->left)
                : static_cast<AVLNode*>(node->right);
            AVLNode* parent = static_cast<AVLNode*>(node->parent);

            emit("UNLINK", node->id,
                parent == NULL ? -1 : parent->id,
                child == NULL ? -1 : child->id, -1,
                node->value, 0,
                parent == NULL ? 'N' : (parent->left == node ? 'L' : 'R'));

            if (child != NULL)
                child->parent = parent;
            delete node;
            return child;
        }
    }

    return rebalance(node);
}

bool AVLTree::deleteNode(int key)
{
    clearEvents();
    lastRotations = 0;

    bool deleted = false;
    root = deleteRec(getAVLRoot(), key, deleted, true);

    if (root != NULL)
        root->parent = NULL;

    if (!deleted)
    {
        emit("NOT_FOUND", -1, -1, -1, -1, key);
        lastMessage = "Key not found.";
    }
    else
        lastMessage = "Key removed.";

    return deleted;
}

AVLNode* AVLTree::kth(int k)
{
    if (k <= 0 || root == NULL || getAVLRoot()->size < k)
        return NULL;

    AVLNode* node = getAVLRoot();
    while (node != NULL)
    {
        AVLNode* left = static_cast<AVLNode*>(node->left);
        int leftSize = (left == NULL) ? 0 : left->size;

        if (k == leftSize + 1)
            return node;
        if (k <= leftSize)
            node = left;
        else
        {
            k -= leftSize + 1;
            node = static_cast<AVLNode*>(node->right);
        }
    }

    return NULL;
}

int AVLTree::rankLL(long long key)
{
    int result = 0;
    AVLNode* node = getAVLRoot();

    while (node != NULL)
    {
        if (key < node->value)
            node = static_cast<AVLNode*>(node->left);
        else
        {
            AVLNode* left = static_cast<AVLNode*>(node->left);
            result += ((left == NULL) ? 0 : left->size) + 1;
            node = static_cast<AVLNode*>(node->right);
        }
    }

    return result;
}

int AVLTree::rank(int key)
{
    return rankLL((long long)key);
}

int AVLTree::countInRange(int lo, int hi)
{
    if (lo > hi)
        return 0;
    return rankLL((long long)hi) - rankLL((long long)lo - 1);
}

int AVLTree::kthPath(int k, int out[], int maxLen)
{
    if (k <= 0 || root == NULL || getAVLRoot()->size < k)
        return 0;

    int count = 0;
    AVLNode* node = getAVLRoot();

    while (node != NULL)
    {
        if (out != NULL && count < maxLen)
            out[count] = node->id;
        count++;

        AVLNode* left = static_cast<AVLNode*>(node->left);
        int leftSize = (left == NULL) ? 0 : left->size;

        if (k == leftSize + 1)
            return count;
        if (k <= leftSize)
            node = left;
        else
        {
            k -= leftSize + 1;
            node = static_cast<AVLNode*>(node->right);
        }
    }

    return count;
}

int AVLTree::rankPath(int key, int out[], int maxLen)
{
    int count = 0;
    AVLNode* node = getAVLRoot();

    while (node != NULL)
    {
        if (out != NULL && count < maxLen)
            out[count] = node->id;
        count++;

        if (key < node->value)
            node = static_cast<AVLNode*>(node->left);
        else
            node = static_cast<AVLNode*>(node->right);
    }

    return count;
}

bool AVLTree::validateRec(AVLNode* node, AVLNode* parent,
    long long minKey, long long maxKey, bool checkBalance,
    int& actualHeight, int& actualSize)
{
    if (node == NULL)
    {
        actualHeight = 0;
        actualSize = 0;
        return true;
    }

    if (node->mark == validationEpoch)
        return false;
    node->mark = validationEpoch;

    if (node->parent != parent)
        return false;
    if (node->value <= minKey || node->value >= maxKey)
        return false;

    int leftHeight = 0, rightHeight = 0, leftSize = 0, rightSize = 0;

    if (!validateRec(static_cast<AVLNode*>(node->left), node,
        minKey, node->value, checkBalance, leftHeight, leftSize))
        return false;
    if (!validateRec(static_cast<AVLNode*>(node->right), node,
        node->value, maxKey, checkBalance, rightHeight, rightSize))
        return false;

    actualHeight = 1 + ((leftHeight > rightHeight) ? leftHeight : rightHeight);
    actualSize = 1 + leftSize + rightSize;

    if (checkBalance)
    {
        int factor = leftHeight - rightHeight;
        if (factor < -1 || factor > 1)
            return false;
    }

    return node->height == actualHeight && node->size == actualSize;
}

bool AVLTree::validate(bool checkBalance)
{
    if (root == NULL)
        return true;

    if (root->parent != NULL)
        return false;

    validationEpoch++;
    int h = 0, s = 0;
    long long low = -(1LL << 62);
    long long high = (1LL << 62);
    return validateRec(getAVLRoot(), NULL, low, high, checkBalance, h, s);
}

bool AVLTree::isBalanced()
{
    return validate(true);
}

bool AVLTree::validateStructure()
{
    return validate(false);
}

void AVLTree::checkStoredRec(AVLNode* node, int& h, int& s,
    int& badSizeId, int& badHeightId)
{
    if (node == NULL)
    {
        h = 0;
        s = 0;
        return;
    }

    int lh = 0, ls = 0, rh = 0, rs = 0;
    checkStoredRec(static_cast<AVLNode*>(node->left), lh, ls,
        badSizeId, badHeightId);
    checkStoredRec(static_cast<AVLNode*>(node->right), rh, rs,
        badSizeId, badHeightId);

    h = 1 + ((lh > rh) ? lh : rh);
    s = 1 + ls + rs;

    if (badSizeId == -1 && node->size != s)
        badSizeId = node->id;
    if (badHeightId == -1 && node->height != h)
        badHeightId = node->id;
}

int AVLTree::findWrongSizeId()
{
    int h = 0, s = 0, badSize = -1, badHeight = -1;
    checkStoredRec(getAVLRoot(), h, s, badSize, badHeight);
    return badSize;
}

int AVLTree::findWrongHeightId()
{
    int h = 0, s = 0, badSize = -1, badHeight = -1;
    checkStoredRec(getAVLRoot(), h, s, badSize, badHeight);
    return badHeight;
}

bool AVLTree::forceStoredSize(int id, int size)
{
    AVLNode* node = getAVLNodeById(id);
    if (node == NULL)
        return false;
    node->size = size;
    return true;
}

int AVLTree::subtreeSizeOf(int id)
{
    AVLNode* node = getAVLNodeById(id);
    return (node == NULL) ? 0 : node->size;
}

int AVLTree::getBalanceFactorById(int id)
{
    return balanceFactor(getAVLNodeById(id));
}

bool AVLTree::isNodeUnbalanced(int id)
{
    AVLNode* node = getAVLNodeById(id);
    if (node == NULL)
        return false;
    int f = balanceFactor(node);
    return f > 1 || f < -1;
}

void AVLTree::collectUnbalancedRec(AVLNode* node, int out[],
    int maxCount, int& count)
{
    if (node == NULL)
        return;

    collectUnbalancedRec(static_cast<AVLNode*>(node->left), out, maxCount, count);

    int f = balanceFactor(node);
    if (f > 1 || f < -1)
    {
        if (out != NULL && count < maxCount)
            out[count] = node->id;
        count++;
    }

    collectUnbalancedRec(static_cast<AVLNode*>(node->right), out, maxCount, count);
}

int AVLTree::getUnbalancedIds(int out[], int maxCount)
{
    int count = 0;
    collectUnbalancedRec(getAVLRoot(), out, maxCount, count);
    return count;
}

std::string AVLTree::badgeText(int id)
{
    AVLNode* node = getAVLNodeById(id);
    if (node == NULL)
        return "?";

    int f = balanceFactor(node);
    if (f > 0)
        return "+" + std::to_string(f);
    return std::to_string(f);
}

AVLNode* AVLTree::lowestUnbalancedRec(AVLNode* node)
{
    if (node == NULL)
        return NULL;

    AVLNode* found = lowestUnbalancedRec(static_cast<AVLNode*>(node->left));
    if (found != NULL)
        return found;

    found = lowestUnbalancedRec(static_cast<AVLNode*>(node->right));
    if (found != NULL)
        return found;

    int f = balanceFactor(node);
    if (f > 1 || f < -1)
        return node;

    return NULL;
}

AVLNode* AVLTree::findLowestUnbalanced()
{
    return lowestUnbalancedRec(getAVLRoot());
}

bool AVLTree::getHint(int& pivotId, int& rotCase)
{
    AVLNode* node = findLowestUnbalanced();
    if (node == NULL)
        return false;

    pivotId = node->id;
    rotCase = detectRotationCase(node);
    return true;
}

bool AVLTree::applyManualRotation(int nodeId, char dir)
{
    AVLNode* node = getAVLNodeById(nodeId);
    if (node == NULL)
        return false;

    if (dir == 'L' || dir == 'l')
    {
        if (node->right == NULL)
            return false;
        clearEvents();
        lastRotations = 0;
        rotateLeft(node);
        return true;
    }

    if (dir == 'R' || dir == 'r')
    {
        if (node->left == NULL)
            return false;
        clearEvents();
        lastRotations = 0;
        rotateRight(node);
        return true;
    }

    return false;
}

int AVLTree::repairNode(int nodeId)
{
    AVLNode* node = getAVLNodeById(nodeId);
    if (node == NULL)
        return ROT_NONE;

    int rotCase = detectRotationCase(node);
    if (rotCase == ROT_NONE)
        return ROT_NONE;

    clearEvents();
    lastRotations = 0;

    AVLNode* parent = static_cast<AVLNode*>(node->parent);
    rebalance(node);
    updateUpward(parent);
    return rotCase;
}

int AVLTree::repairAll()
{
    clearEvents();
    lastRotations = 0;

    int guard = nodeCount() * 10 + 10;     // safety net
    while (guard-- > 0)
    {
        AVLNode* node = findLowestUnbalanced();
        if (node == NULL)
            break;

        AVLNode* parent = static_cast<AVLNode*>(node->parent);
        rebalance(node);
        updateUpward(parent);
    }

    return lastRotations;
}

int AVLTree::expectedRotationSteps(int rotCase)
{
    if (rotCase == ROT_LL || rotCase == ROT_RR)
        return 1;
    if (rotCase == ROT_LR || rotCase == ROT_RL)
        return 2;
    return 0;
}

const char* AVLTree::rotationName(int rotCase)
{
    if (rotCase == ROT_LL) return "LL";
    if (rotCase == ROT_RR) return "RR";
    if (rotCase == ROT_LR) return "LR";
    if (rotCase == ROT_RL) return "RL";
    return "NONE";
}

// ---------------------------------------------------------------
// Level 3.4: prediction (works on a copy, the real tree is untouched)
// ---------------------------------------------------------------
int AVLTree::firstRotationCase()
{
    for (EventNode* e = events.getHead(); e != NULL; e = e->next)
        if (e->is("ROTATE"))
            return e->result;
    return ROT_NONE;
}

bool AVLTree::wouldBeDuplicate(int key)
{
    return containsKey(key);
}

int AVLTree::predictInsert(int key)
{
    if (containsKey(key))
        return ROT_NONE;

    AVLTree copy;
    copy.cloneFrom(*this);
    copy.insert(key);
    return copy.firstRotationCase();
}

int AVLTree::predictDelete(int key)
{
    if (!containsKey(key))
        return ROT_NONE;

    AVLTree copy;
    copy.cloneFrom(*this);
    copy.deleteNode(key);
    return copy.firstRotationCase();
}

int AVLTree::findInsertKeyFor(int wantCase, unsigned& rngState)
{
    for (int tries = 0; tries < 400; tries++)
    {
        int key = (int)(nextRandom(rngState) % 100);
        if (containsKey(key))
            continue;
        if (predictInsert(key) == wantCase)
            return key;
    }
    return -1;
}

unsigned AVLTree::nextRandom(unsigned& state)
{
    state = state * 1664525u + 1013904223u;
    return state >> 8;
}

bool AVLTree::rawInsert(int key)
{
    AVLNode* parent = NULL;
    AVLNode* current = getAVLRoot();

    while (current != NULL && current->value != key)
    {
        parent = current;
        if (key < current->value)
            current = static_cast<AVLNode*>(current->left);
        else
            current = static_cast<AVLNode*>(current->right);
    }

    if (current != NULL)
        return false;

    AVLNode* newNode = new AVLNode(nextId++, key);
    newNode->parent = parent;

    if (parent == NULL)
        root = newNode;
    else if (key < parent->value)
        parent->left = newNode;
    else
        parent->right = newNode;

    emit("INSERT", newNode->id,
        parent == NULL ? -1 : parent->id,
        parent == NULL ? 0 : parent->value,
        -1, key, 0,
        parent == NULL ? 'N' : (key < parent->value ? 'L' : 'R'));

    updateUpward(parent);
    return true;
}

bool AVLTree::insertRaw(int key)
{
    clearEvents();
    lastRotations = 0;

    bool ok = rawInsert(key);
    if (!ok)
        emit("DUPLICATE_REJECTED", -1, -1, -1, -1, key);
    return ok;
}

int AVLTree::buildUnbalanced(const int keys[], int count)
{
    resetTree();
    if (keys == NULL || count <= 0)
        return 0;

    int insertedCount = 0;
    for (int i = 0; i < count; i++)
        if (rawInsert(keys[i]))
            insertedCount++;

    clearEvents();    
    return insertedCount;
}

int AVLTree::generateCase(int caseType, unsigned seed, int keysOut[], int maxKeys)
{
     const int T_LL[3][6] = {
        { 3, 2, 1, 0, 0, 0 },      
        { 5, 3, 6, 2, 4, 1 },      
        { 3, 2, 1, 6, 5, 4 }      
    };
     const int N_LL[3] = { 3, 6, 6 };

     const int T_LR[3][6] = {
        { 3, 1, 2, 0, 0, 0 },
        { 5, 2, 6, 1, 3, 4 },
        { 3, 2, 1, 6, 4, 5 }
    };
     const int N_LR[3] = { 3, 6, 6 };

    if (keysOut == NULL)
        return 0;

    int variant = (int)(seed % 3);
    const int* tpl = NULL;
    int n = 0;
    bool mirrorIt = false;

    if (caseType == ROT_LL) { tpl = T_LL[variant]; n = N_LL[variant]; }
    else if (caseType == ROT_RR) { tpl = T_LL[variant]; n = N_LL[variant]; mirrorIt = true; }
    else if (caseType == ROT_LR) { tpl = T_LR[variant]; n = N_LR[variant]; }
    else if (caseType == ROT_RL) { tpl = T_LR[variant]; n = N_LR[variant]; mirrorIt = true; }
    else return 0;

    if (n > maxKeys)
        return 0;

    unsigned state = seed * 2654435761u + 7919u;
    int values[8];
    values[1] = 5 + (int)(nextRandom(state) % 10);
    for (int r = 2; r <= n; r++)
        values[r] = values[r - 1] + 2 + (int)(nextRandom(state) % 11);

    for (int i = 0; i < n; i++)
    {
        int rankValue = tpl[i];
        if (mirrorIt)
            rankValue = n + 1 - rankValue;
        keysOut[i] = values[rankValue];
    }

    return n;
}

bool AVLTree::loadCase(int caseType, unsigned seed)
{
    int keys[8];
    int n = generateCase(caseType, seed, keys, 8);
    if (n == 0)
        return false;

    buildUnbalanced(keys, n);

    AVLNode* lowest = findLowestUnbalanced();
    return lowest != NULL && detectRotationCase(lowest) == caseType;
}

int AVLTree::loadRandomUnbalanced(unsigned seed, int n)
{
    if (n < 3) n = 3;
    if (n > 15) n = 15;

    int keys[15];

    for (int attempt = 0; attempt < 20; attempt++)
    {
        unsigned state = (seed + (unsigned)attempt) * 2654435761u + 31u;
        int count = 0;
        int guard = 0;

        while (count < n && guard < 500)
        {
            guard++;
            int key = (int)(nextRandom(state) % 100);
            bool duplicate = false;
            for (int i = 0; i < count; i++)
                if (keys[i] == key)
                    duplicate = true;
            if (!duplicate)
                keys[count++] = key;
        }

        buildUnbalanced(keys, count);
        if (findLowestUnbalanced() != NULL)
            return nodeCount();
    }

    for (int i = 0; i < n; i++)
        keys[i] = 10 + i * 5;
    buildUnbalanced(keys, n);
    return nodeCount();
}

int AVLTree::buildRandomAVL(unsigned seed, int n, int maxKey)
{
    resetTree();
    if (n <= 0 || maxKey < 0)
        return 0;
    if (maxKey > 1000000)
        maxKey = 1000000;
    if (n > maxKey + 1)
        n = maxKey + 1;

    unsigned state = seed * 2654435761u + 977u;
    int inserted = 0;
    int tries = 0;

    while (inserted < n && tries < n * 50 + 100)
    {
        tries++;
        int key = (int)(nextRandom(state) % (unsigned)(maxKey + 1));
        if (insert(key))
            inserted++;
    }

    clearEvents();
    lastRotations = 0;
    totalRotations = 0;
    return inserted;
}

bool AVLTree::randomOperation(unsigned& rngState, int& keyOut, bool& wasInsert)
{
    int n = nodeCount();
    bool doInsert;

    if (n < 6)
        doInsert = true;
    else if (n > 40)
        doInsert = false;
    else
        doInsert = (nextRandom(rngState) % 100) < 60;

    if (doInsert)
    {
        for (int tries = 0; tries < 200; tries++)
        {
            int key = (int)(nextRandom(rngState) % 200);
            if (!containsKey(key))
            {
                keyOut = key;
                wasInsert = true;
                return insert(key);
            }
        }
        return false;
    }

    int k = 1 + (int)(nextRandom(rngState) % (unsigned)n);
    AVLNode* target = kth(k);
    if (target == NULL)
        return false;

    keyOut = target->value;
    wasInsert = false;
    return deleteNode(keyOut);
}

void AVLTree::renumberIds()
{
    int n = nodeCount();
    if (n <= 0)
    {
        nextId = idBase;
        return;
    }

    AVLNode** queue = new AVLNode * [n];
    int head = 0, tail = 0;
    queue[tail++] = getAVLRoot();

    int id = idBase;
    while (head < tail)
    {
        AVLNode* node = queue[head++];
        node->id = id++;
        if (node->left != NULL && tail < n)
            queue[tail++] = static_cast<AVLNode*>(node->left);
        if (node->right != NULL && tail < n)
            queue[tail++] = static_cast<AVLNode*>(node->right);
    }

    delete[] queue;
    nextId = id;
}

AVLNode* AVLTree::cloneRec(const AVLNode* src, AVLNode* parent)
{
    if (src == NULL)
        return NULL;

    AVLNode* copy = new AVLNode(src->id, src->value);
    copy->height = src->height;
    copy->size = src->size;
    copy->x = src->x;
    copy->y = src->y;
    copy->parent = parent;
    copy->left = cloneRec(static_cast<const AVLNode*>(src->left), copy);
    copy->right = cloneRec(static_cast<const AVLNode*>(src->right), copy);
    return copy;
}

void AVLTree::cloneFrom(const AVLTree& other)
{
    if (&other == this)
        return;

    resetTree();
    root = cloneRec(static_cast<AVLNode*>(other.root), NULL);
    nextId = other.nextId;
    clearEvents();
}

AVLNode* AVLTree::parseRec(const std::string& s, int& pos, AVLNode* parent,
    bool& ok, int& count, int& idCounter, int depth)
{
    if (!ok)
        return NULL;

    int len = (int)s.size();
    while (pos < len && isSpace(s[pos]))
        pos++;

    if (pos >= len)
    {
        ok = false;
        return NULL;
    }

    if (s[pos] == '#')
    {
        pos++;
        return NULL;
    }

    bool negative = false;
    if (s[pos] == '-')
    {
        negative = true;
        pos++;
    }

    long long number = 0;
    int digits = 0;
    while (pos < len && s[pos] >= '0' && s[pos] <= '9')
    {
        number = number * 10 + (s[pos] - '0');
        digits++;
        pos++;
        if (number > 2000000000LL)
        {
            ok = false;
            return NULL;
        }
    }

    if (digits == 0 || (pos < len && !isSpace(s[pos])))
    {
        ok = false;
        return NULL;
    }

    count++;
    if (count > MAX_LOAD_NODES || depth > MAX_LOAD_NODES)
    {
        ok = false;
        return NULL;
    }

    if (negative)
        number = -number;

    AVLNode* node = new AVLNode(idCounter++, (int)number);
    node->parent = parent;
    node->left = parseRec(s, pos, node, ok, count, idCounter, depth + 1);
    node->right = parseRec(s, pos, node, ok, count, idCounter, depth + 1);
    return node;  
}

bool AVLTree::loadFromString(const std::string& text)
{
    bool blank = true;
    for (size_t i = 0; i < text.size(); i++)
        if (!isSpace(text[i]))
            blank = false;

    if (blank)
    {
        resetTree();
        return true;
    }

    int pos = 0;
    int count = 0;
    int idCounter = idBase;
    bool ok = true;

    AVLNode* newRoot = parseRec(text, pos, NULL, ok, count, idCounter, 0);

    if (ok)
    {
        while (pos < (int)text.size() && isSpace(text[pos]))
            pos++;
        if (pos < (int)text.size())
            ok = false;   
    }

    if (ok && newRoot != NULL)
    {
        recompute(newRoot);        

        validationEpoch++;
        int h = 0, s = 0;
        long long low = -(1LL << 62);
        long long high = (1LL << 62);
        if (!validateRec(newRoot, NULL, low, high, false, h, s))
            ok = false;              
    }

    if (!ok)
    {
        if (newRoot != NULL)
            clearTree(newRoot);
        return false;              
    }

    resetTree();
    root = newRoot;
    nextId = idCounter;
    return true;
}

// ---------------------------------------------------------------
// SFML-friendly data
// ---------------------------------------------------------------
int AVLTree::collectAVLNodes(AVLNodeInfo out[], int maxN)
{
    if (out == NULL || maxN <= 0 || root == NULL)
        return 0;

    AVLNode** queue = new AVLNode * [maxN];
    int head = 0, tail = 0;
    queue[tail++] = getAVLRoot();

    int count = 0;
    while (head < tail && count < maxN)
    {
        AVLNode* node = queue[head++];
        AVLNodeInfo& info = out[count++];

        int f = balanceFactor(node);
        int depthValue = 0;
        for (const TreeNode* p = node; p != NULL; p = p->parent)
            depthValue++;

        info.id = node->id;
        info.value = node->value;
        info.depth = depthValue;
        info.height = node->height;
        info.size = node->size;
        info.balance = f;
        info.parentId = (node->parent == NULL) ? -1 : node->parent->id;
        info.leftId = (node->left == NULL) ? -1 : node->left->id;
        info.rightId = (node->right == NULL) ? -1 : node->right->id;
        info.unbalanced = (f > 1 || f < -1);
        info.badgeLevel = (f == 0) ? 0 : ((f == 1 || f == -1) ? 1 : 2);
        info.x = node->x;
        info.y = node->y;

        if (node->left != NULL && tail < maxN)
            queue[tail++] = static_cast<AVLNode*>(node->left);
        if (node->right != NULL && tail < maxN)
            queue[tail++] = static_cast<AVLNode*>(node->right);
    }

    delete[] queue;
    return count;
}

std::string AVLTree::roomName(int id)
{
    AVLNode* node = getAVLNodeById(id);
    if (node != NULL)
        return "room " + std::to_string(node->value);
    return "room #" + std::to_string(id);
}

std::string AVLTree::describeAVLEvent(EventNode* e)
{
    if (e == NULL)
        return "";

    if (e->is("INSERT"))
    {
        std::string s = "Inserted " + std::to_string(e->key);
        if (e->parentId == -1)
            s += " as the root";
        else
            s += (e->side == 'L' ? " as left child of " : " as right child of ")
            + std::to_string(e->nodeA);
        return s;
    }
    if (e->is("DUPLICATE_REJECTED"))
        return "Duplicate key " + std::to_string(e->key) + " rejected";
    if (e->is("NOT_FOUND"))
        return "Key " + std::to_string(e->key) + " not found";
    if (e->is("UNLINK"))
        return "Removed room " + std::to_string(e->key);
    if (e->is("SWAP_VALUE"))
        return "Swapped " + std::to_string(e->key) + " with inorder successor "
        + std::to_string(e->result);
    if (e->is("UPDATE_HEIGHT"))
        return "Height of " + roomName(e->nodeId) + ": "
        + std::to_string(e->key) + " -> " + std::to_string(e->result);
    if (e->is("UPDATE_SIZE"))
        return "Size of " + roomName(e->nodeId) + ": "
        + std::to_string(e->key) + " -> " + std::to_string(e->result);
    if (e->is("BALANCE_ALERT"))
        return roomName(e->nodeId) + " is unbalanced (balance factor "
        + std::to_string(e->key) + ")";
    if (e->is("ROTATE"))
    {
        std::string s = std::string("Case ") + rotationName(e->result)
            + " at " + roomName(e->nodeId) + ": ";
        s += (expectedRotationSteps(e->result) == 2) ? "double rotation" : "single rotation";
        return s;
    }
    if (e->is("ROTATE_STEP"))
        return std::string("Rotate ") + (e->mode == 'L' ? "left" : "right")
        + " at " + std::to_string(e->key) + ": "
        + std::to_string(e->result) + " moves up";

    return describeEvent(e);        
}