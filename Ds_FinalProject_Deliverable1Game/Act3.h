#pragma once
#include <string>
#include "Act2.h"

struct AVLNode : public BSTNode
{
    int height;
    int size;
    int mark;   
    AVLNode(int id, int value) : BSTNode(id, value)
    {
        height = 1;
        size = 1;
        mark = 0;
    }
};
struct AVLNodeInfo
{
    int id;
    int value;
    int depth;          // root = 1
    int height;         // leaf = 1
    int size;           // leaf = 1
    int balance;        // height(left) - height(right)
    int parentId;       // -1 if none
    int leftId;         // -1 if none
    int rightId;        // -1 if none
    bool unbalanced;    // |balance| > 1
    int badgeLevel;  
    float x;
    float y;

    AVLNodeInfo()
    {
        id = -1; value = 0; depth = 0; height = 0; size = 0; balance = 0;
        parentId = -1; leftId = -1; rightId = -1;
        unbalanced = false; badgeLevel = 0;
        x = 0; y = 0;
    }
};

class AVLTree : public BST
{
public:
 
    static constexpr int ROT_NONE = 0;
    static constexpr int ROT_LL = 1;
    static constexpr int ROT_RR = 2;
    static constexpr int ROT_LR = 3;
    static constexpr int ROT_RL = 4;
    static constexpr int MAX_LOAD_NODES = 5000;   

private:
    int idBase;             
    bool recordEvents;      
    int validationEpoch;    
    int lastRotations;      
    int totalRotations;    
    // ---- events -------------
    void emit(const char* type, int nodeId = -1, int parentId = -1,
        int nodeA = -1, int nodeB = -1, int key = 0,
        int result = 0, char side = 'N', char mode = 'N');
    void update(AVLNode* node);            
    void updateUpward(AVLNode* from);       
    void recompute(AVLNode* node);         
    int  firstRotationCase();               
    std::string roomName(int id);
    static bool isSpace(char c);
    AVLNode* rotL(AVLNode* node);          
    AVLNode* rotR(AVLNode* node);

    AVLNode* insertRec(AVLNode* node, int key, AVLNode* parent, bool& inserted);
    AVLNode* deleteRec(AVLNode* node, int key, bool& deleted, bool emitCompare);
    bool rawInsert(int key);               

    bool validateRec(AVLNode* node, AVLNode* parent,
        long long minKey, long long maxKey, bool checkBalance,
        int& actualHeight, int& actualSize);
    bool validate(bool checkBalance);
    void checkStoredRec(AVLNode* node, int& h, int& s,
        int& badSizeId, int& badHeightId);

    int  rankLL(long long key);
    void collectUnbalancedRec(AVLNode* node, int out[], int maxCount, int& count);
    AVLNode* lowestUnbalancedRec(AVLNode* node);
    AVLNode* cloneRec(const AVLNode* src, AVLNode* parent);
    AVLNode* parseRec(const std::string& s, int& pos, AVLNode* parent,
        bool& ok, int& count, int& idCounter, int depth);

public:
    AVLTree();

    // ---- BST functions that build plain BSTNodes: NOT allowed on an AVLTree ----
    bool insertNode(int id, int value) = delete;
    int  buildFromKeys(const int keys[], int n) = delete;
    void generateRandom(unsigned seed, int n, int skewPercent = 0) = delete;
    void generateLazyArchitect(int count) = delete;

    AVLNode* getAVLRoot();
    AVLNode* getAVLNodeById(int id);
    int  nodeCount();
    int  treeHeight();
    bool containsKey(int key);
    void resetTree();                      

    void setRecordEvents(bool on);
    bool getRecordEvents();
    int  getLastRotationCount();
    int  getTotalRotations();
    void resetRotationCounters();

    bool insert(int key);
    bool insertValue(int value);          
    bool deleteNode(int key);

    AVLNode* rotateLeft(AVLNode* node);
    AVLNode* rotateRight(AVLNode* node);
    AVLNode* rebalance(AVLNode* node);      
    int  balanceFactor(AVLNode* node);

    // ---- order statistics ----
    AVLNode* kth(int k);
    int  rank(int key);                     // number of keys <= key
    int  countInRange(int lo, int hi);      // rank(hi) - rank(lo - 1)
    int  kthPath(int k, int out[], int maxLen);     // ids visited, returns TRUE count (0 if k invalid)
    int  rankPath(int key, int out[], int maxLen);  // ids visited, returns TRUE count

    // ---- validation ----
    bool isBalanced();                      // structure + balance
    bool validateStructure();               // everything except balance (for 3.1 - 3.3 start trees)
    int  findWrongSizeId();                 // -1 if every stored size is correct
    int  findWrongHeightId();               // -1 if every stored height is correct
    bool forceStoredSize(int id, int size); // debug / demo: corrupt a size so the boss can flag it
    int  subtreeSizeOf(int id);             // stored size, 0 if not found

    // ---- Level 3.1 ----
    int  getBalanceFactorById(int id);
    bool isNodeUnbalanced(int id);
    int  getUnbalancedIds(int out[], int maxCount);  // returns TOTAL unbalanced, writes up to maxCount ids
    std::string badgeText(int id);                    // "+1", "0", "-2", "?" if missing

    // ---- Levels 3.2 / 3.3 ----
    int  detectRotationCase(AVLNode* node);          // ROT_*, tree unchanged
    int  detectRotationCaseById(int id);
    AVLNode* findLowestUnbalanced();                 // the correct pivot, NULL if none
    bool getHint(int& pivotId, int& rotCase);
    bool applyManualRotation(int nodeId, char dir);  // dir 'L' or 'R', clears events first
    int  repairNode(int nodeId);                     // auto-fix one room, returns case applied
    int  repairAll();                                // auto-fix whole tree, returns primitive rotations
    static int expectedRotationSteps(int rotCase);   // 1 single, 2 double, 0 none
    static const char* rotationName(int rotCase);    // "LL", "RR", "LR", "RL", "NONE"

    // ---- Level 3.4 (prediction) ----
    int  predictInsert(int key);            // ROT_*, tree unchanged
    int  predictDelete(int key);            // ROT_NONE also if key is missing
    bool wouldBeDuplicate(int key);
    int  findInsertKeyFor(int wantCase, unsigned& rngState);   // -1 if none found

    // ---- level data / generation (seeded) ----
    static unsigned nextRandom(unsigned& state);
    int  buildUnbalanced(const int keys[], int count);         // plain BST order, no rebalancing
    bool insertRaw(int key);                                   // plain insert, no rebalancing
    int  generateCase(int caseType, unsigned seed, int keysOut[], int maxKeys);
    bool loadCase(int caseType, unsigned seed);                // each of LL/RR/LR/RL has 3 templates
    int  loadRandomUnbalanced(unsigned seed, int n);           // Level 3.1 (n 3..15)
    int  buildRandomAVL(unsigned seed, int n, int maxKey = 99);

    // ---- endless mode ----
    bool randomOperation(unsigned& rngState, int& keyOut, bool& wasInsert);
    void renumberIds();                     // ids = idBase.. in level order (call between operations)

    // ---- snapshots / save / load ----
    void cloneFrom(const AVLTree& other);   // exact copy, ids preserved
    bool loadFromString(const std::string& text);   // preorder with '#', validates before installing
    // saving uses the inherited treeToString()

    // ---- SFML-friendly data ----
    int  collectAVLNodes(AVLNodeInfo out[], int maxN);   // level order, returns count
    std::string describeAVLEvent(EventNode* e);          // call right after the operation
};