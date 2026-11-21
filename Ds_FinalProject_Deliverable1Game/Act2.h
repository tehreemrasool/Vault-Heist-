#pragma once
#include <string>
#include <cstring>
#include "header.h"   

// ---------------------------------------------------------------
// Event node (one step of what an operation did)
// ---------------------------------------------------------------
struct EventNode
{
    const char* type;
    int nodeId;
    int parentId;
    int nodeA;
    int nodeB;
    int key;
    int result;
    char side;
    char mode;
    EventNode* next;

    EventNode(const char* t, int n = -1, int p = -1,
        int a = -1, int b = -1, int k = 0,
        int r = 0, char s = 'N', char m = 'N')
    {
        type = t;
        nodeId = n;
        parentId = p;
        nodeA = a;
        nodeB = b;
        key = k;
        result = r;
        side = s;
        mode = m;
        next = NULL;
    }

    // true if this event is of the given type, e.g. e->is("VISIT")
    bool is(const char* t) const
    {
        return strcmp(type, t) == 0;
    }
};

// ---------------------------------------------------------------
// Event log (linked list of events)
// ---------------------------------------------------------------
class EventLog
{
private:
    EventNode* head;
    EventNode* tail;

public:
    EventLog()
    {
        head = NULL;
        tail = NULL;
    }

    ~EventLog()
    {
        clear();
    }

    // copying would cause a double delete, so it is forbidden
    EventLog(const EventLog&) = delete;
    EventLog& operator=(const EventLog&) = delete;

    void addEvent(const char* type, int nodeId = -1,
        int parentId = -1, int nodeA = -1,
        int nodeB = -1, int key = 0,
        int result = 0, char side = 'N',
        char mode = 'N')
    {
        EventNode* newEvent = new EventNode(
            type, nodeId, parentId, nodeA, nodeB,
            key, result, side, mode);

        if (head == NULL)
            head = tail = newEvent;
        else
        {
            tail->next = newEvent;
            tail = newEvent;
        }
    }

    EventNode* getHead()
    {
        return head;
    }

    int count() const
    {
        int c = 0;
        EventNode* t = head;
        while (t != NULL)
        {
            c++;
            t = t->next;
        }
        return c;
    }

    // returns the index-th event (0 = first) or NULL
    EventNode* getAt(int index)
    {
        if (index < 0)
            return NULL;

        EventNode* t = head;
        while (t != NULL && index > 0)
        {
            t = t->next;
            index--;
        }
        return t;
    }

    void clear()
    {
        while (head != NULL)
        {
            EventNode* temp = head;
            head = head->next;
            delete temp;
        }
        tail = NULL;
    }
};

// ---------------------------------------------------------------
// BST node
// ---------------------------------------------------------------
struct BSTNode : public TreeNode
{
    BSTNode(int i, int v) : TreeNode(v)
    {
        id = i;
        x = 0;
        y = 0;
    }
    virtual ~BSTNode() {}
};

// Linked list node for Range Raid results
struct RangeResult
{
    BSTNode* data;
    RangeResult* next;

    RangeResult(BSTNode* n)
    {
        data = n;
        next = NULL;
    }
};

// Everything the renderer / tooltip needs about one node
// depth: root = 1 (number of rooms entered to reach it)
// height: leaf = 1
struct NodeInfo
{
    int id;
    int value;
    int depth;
    int height;
    int parentId;   // -1 if none
    int leftId;     // -1 if none
    int rightId;    // -1 if none
    float x;
    float y;

    NodeInfo()
    {
        id = -1; value = 0; depth = 0; height = 0;
        parentId = -1; leftId = -1; rightId = -1;
        x = 0; y = 0;
    }
};

// ---------------------------------------------------------------
// Binary Search Tree
//
// compare convention used everywhere:
//   0  -> keys match
//   1  -> room value is BIGGER than key  (go LEFT)
//  -1  -> room value is SMALLER than key (go RIGHT)
// ---------------------------------------------------------------
class BST
{
protected:
    BSTNode* root;
    EventLog events;

    int nextId;             // next free node id
    int lastAlarmSteps;     // alarm steps of the last operation
    std::string lastMessage;

    // Range Raid state (list is owned by the BST)
    RangeResult* rangeHead;
    RangeResult* rangeTail;
    int rangeVisited;
    int rangeSkippedSubtrees;
    int rangeSkippedNodes;
    int rangeInRange;

    // helpers
    void clearTree(BSTNode* node);          // node must be a subtree root
    void clearRangeResults();
    int  countNodes(BSTNode* node);
    BSTNode* findById(BSTNode* node, int id);
    int  stepCompare(BSTNode* node, int key);   // adds VISIT + COMPARE
    void RangeRaid(BSTNode* node, int lo, int hi);
    int  countRangeVisits(BSTNode* node, int lo, int hi);
    void collectIds(BSTNode* node, int out[], int maxN, int& count);
    void writeNode(BSTNode* node, std::string& out);
    BSTNode* parseNode(const std::string& s, int& pos,
        BSTNode* parent, bool& ok);

public:
    BST();
    virtual ~BST();

    // trees own raw pointers, so copying is forbidden
    BST(const BST&) = delete;
    BST& operator=(const BST&) = delete;

    // ----- basic access -----
    BSTNode* getRoot();
    bool isEmpty();
    void reset();                       // delete everything, back to empty

    // ----- events -----
    EventNode* getEvents();
    void clearEvents();
    int getEventCount();
    EventNode* getEvent(int index);
    std::string describeEvent(EventNode* e);

    // ----- last operation info -----
    std::string getLastMessage();
    int getLastAlarmSteps();
    int getNextId();

    // ----- core operations -----
    BSTNode* searchNode(int value, int& alarm_count);
    bool insertNode(int id, int value);
    bool insertValue(int value);        // uses the BST's own id counter
    bool deleteNode(int value);

    BSTNode* findMinimum(BSTNode* node);
    BSTNode* findMaximum(BSTNode* node);
    BSTNode* successor(BSTNode* node);
    BSTNode* predecessor(BSTNode* node);
    BSTNode* lcaBST(int value1, int value2);
    BSTNode* kthSmallest(int k);

    int height(BSTNode* node);          // leaf = 1, empty = 0
    int countNodes();
    int depthOf(BSTNode* node);         // root = 1
    BSTNode* getNodeById(int id);

    // ----- level helpers: single-step navigation (Levels 2.1 / 2.2) -----
    int compareAt(BSTNode* node, int key);          // no events
    BSTNode* childToward(BSTNode* node, int key);   // NULL on match / no child

    // ----- level helpers: optimal steps (never typed by hand) -----
    static int capacityFor(int optimal);            // ceil(1.5*optimal) + 2
    int searchPathLength(int key);      // rooms visited until found / NULL
    int insertPathLength(int key);      // rooms visited; -1 if key exists
    int rangeOptimalSteps(int lo, int hi);

    // ----- Level 2.3: delete case -----
    // 0 = key not found, 1 = leaf, 2 = one child, 3 = two children
    int classifyDelete(int key);

    // ----- Level 2.4: Range Raid -----
    // result list is owned by the BST; valid until next query / reset
    RangeResult* rangeQuery(int lo, int hi);
    RangeResult* getRangeResults();
    int getRangeVisited();
    int getRangeSkippedSubtrees();
    int getRangeSkippedNodes();
    int getRangeInRange();
    static void freeRangeResults(RangeResult*& head);

    // ----- Level 2.5: boss -----
    void generateLazyArchitect(int count);
    BSTNode* bossSearch(int target, int& alarmCount, int& remainingCapacity,
        int round = 0, int shrinkPerRound = 0);

    // ----- level generation (seeded, reproducible) -----
    // skewPercent: 0 = random order, 100 = sorted order (corridor)
    int buildFromKeys(const int keys[], int n);
    void generateRandom(unsigned seed, int n, int skewPercent = 0);

    // ----- validator -----
    bool isValidBST();

    // ----- save / load / undo snapshots -----
    std::string treeToString();                     // e.g. "50 30 # # 70 60 # # #"
    bool loadFromString(const std::string& text);

    // ----- SFML-friendly data -----
    int collectNodes(NodeInfo out[], int maxN);     // level order, returns count
    void computeLayout(float left, float right, float top, float bottom);
    int pathTo(int key, int outIds[], int maxN);    // path length, -1 if missing
    int collectSubtreeIds(BSTNode* subRoot, int out[], int maxN);
};