
#pragma once
#include <iostream>


using namespace std;
struct TreeNode
{
    int id;
    TreeNode* left;
    TreeNode* right;
    TreeNode* parent;
    int value;
    float x;
    float y;
    TreeNode()
    {
        x = 0;
        y = 0;
        id = 0;
        value = 0;
        left = NULL;
        right = NULL;
        parent = NULL;
    }
    TreeNode(int val)
    {
        id = 0;
        value = val;
        left = NULL;
        right = NULL;
        parent = NULL;

    }
};
// Event node
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
};

// Event Log
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

// BST Node
struct BSTNode : public TreeNode
{
    BSTNode(int i, int v) : TreeNode(v)
    {
        id = i;
        x = 0;
        y = 0;
    }
};

// Linked list node for Range Raid
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

// Binary Search Tree
class BST
{
private:
    BSTNode* root;
    EventLog events;

public:
    BST();
    ~BST();

    BSTNode* getRoot();

    EventNode* getEvents();
    void clearEvents();

    BSTNode* searchNode(int value, int& alarm_count);

    bool insertNode(int id, int value);

    bool deleteNode(int value);

    BSTNode* findMinimum(BSTNode* node);
    BSTNode* findMaximum(BSTNode* node);

    BSTNode* successor(BSTNode* node);
    BSTNode* predecessor(BSTNode* node);

    void RangeRaid(BSTNode* node, int lo, int hi,
        RangeResult*& head, RangeResult*& tail,
        bool firstCall = true);

    BSTNode* lcaBST(int value1, int value2);

    BSTNode* kthSmallest(int k);

    int height(BSTNode* node);

    void clearTree(BSTNode* node);
    void generateLazyArchitect(int count);

    BSTNode* bossSearch(int target, int& alarmCount, int& remainingCapacity);
};
