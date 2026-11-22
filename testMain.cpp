// Vault Heist - unit test suite (no SFML, no STL containers, no global variables)
// Build: g++ -std=c++17 -Wall -I. test_main.cpp Act2.cpp Act3.cpp -o run_tests
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include "helpers.h"
#include "header.h"
#include "act1.h"
#include "act4.h"
#include "Act2.h"
#include "Act3.h"
using namespace std;

// ---------------------------------------------------------------- framework
// All state lives inside this object (created in main and passed to every test).
class Tester
{
    static const int MAXLINES = 400;
    int pass, fail;
    int suitePass, suiteFail;
    string failures[MAXLINES];
    int failCount;
    string report[MAXLINES];
    int reportCount;

    void addReport(const string& s)
    {
        if (reportCount < MAXLINES) report[reportCount++] = s;
    }

public:
    Tester()
    {
        pass = fail = 0;
        suitePass = suiteFail = 0;
        failCount = 0;
        reportCount = 0;
    }

    void record(bool ok, const char* expr, const char* file, int line)
    {
        if (ok) { pass++; suitePass++; return; }
        fail++; suiteFail++;
        if (failCount < MAXLINES)
            failures[failCount++] = string("    FAIL ") + file + ":" + to_string(line) + "  " + expr;
    }

    void run(const string& name, void (*fn)(Tester&))
    {
        suitePass = suiteFail = 0;
        int before = failCount;
        fn(*this);
        string head = string(suiteFail == 0 ? "[PASS] " : "[FAIL] ") + name +
            "  (" + to_string(suitePass) + " passed, " + to_string(suiteFail) + " failed)";
        addReport(head);
        cout << head << "\n";
        for (int i = before; i < failCount; i++)
        {
            addReport(failures[i]);
            cout << failures[i] << "\n";
        }
    }

    int failed() const { return fail; }

    void finish()
    {
        cout << "\nTOTAL: " << pass << " passed, " << fail << " failed\n";
        ofstream rep("test_report.txt");
        rep << "Vault Heist - Unit Test Report\n==============================\n";
        for (int i = 0; i < reportCount; i++) rep << report[i] << "\n";
        rep << "\nTOTAL CHECKS: " << (pass + fail) << "\nPASSED: " << pass
            << "\nFAILED: " << fail << "\nRESULT: "
            << (fail == 0 ? "ALL TESTS PASSED" : "SOME TESTS FAILED") << "\n";
        rep.close();
        cout << "Report written to test_report.txt\n";
    }
};

// every test function has a parameter named T
#define CHECK(c) T.record((c), #c, __FILE__, __LINE__)
// values are cast to long long so static const members are not ODR-used
#define CHECK_EQ(a, b) T.record(((long long)(a)) == ((long long)(b)), #a " == " #b, __FILE__, __LINE__)

// ---------------------------------------------------------------- helpers
//        1
//       / \
//      2   3
//     / \
//    4   5
static TreeNode* sampleTree()
{
    TreeNode* r = new TreeNode(1); r->id = 1;
    addLeft(r, 2, 2);
    addright(r, 3, 3);
    addLeft(r->left, 4, 4);
    addright(r->left, 5, 5);
    return r;
}
static string seq(List& l)
{
    string s;
    for (TreeNode* c = l.getHead(); c != NULL; c = c->right)
        s += (s.empty() ? "" : " ") + to_string(c->value);
    return s;
}
static string inorderStr(TreeNode* t)
{
    List l; inOrder(t, &l); return seq(l);
}
static int indexOfVal(act1Level2& l, int v)
{
    for (int i = 0; i < l.getN(); i++) if (l.getVal(i) == v) return i;
    return -1;
}
static void linkRec(act1Level2& l, TreeNode* t)
{
    if (!t) return;
    int p = indexOfVal(l, t->value);
    if (t->left) { l.connect(p, act1Level2::LEFT, indexOfVal(l, t->left->value));   linkRec(l, t->left); }
    if (t->right) { l.connect(p, act1Level2::RIGHT, indexOfVal(l, t->right->value)); linkRec(l, t->right); }
}

// ================================================================ TREE BASICS
static void test_traversals(Tester& T)
{
    TreeNode* r = sampleTree();
    List a, b, c, d;
    preOrder(r, &a); inOrder(r, &b); postOrder(r, &c); levelOrder(r, &d);
    CHECK(seq(a) == "1 2 4 5 3");
    CHECK(seq(b) == "4 2 5 1 3");
    CHECK(seq(c) == "4 5 2 3 1");
    CHECK(seq(d) == "1 2 3 4 5");
    CHECK_EQ(a.getCount(), 5);
    List e; preOrder(NULL, &e);
    CHECK_EQ(e.getCount(), 0);
    deleteTree(r);
}

static void test_reconstruction(Tester& T)
{
    int in[] = { 4, 2, 5, 1, 3 };
    int pre[] = { 1, 2, 4, 5, 3 };
    int post[] = { 4, 5, 2, 3, 1 };
    TreeNode* t1 = buildPre(pre, in, 5);
    TreeNode* t2 = buildPost(post, in, 5);
    List p1, p2;
    preOrder(t1, &p1); postOrder(t2, &p2);
    CHECK(seq(p1) == "1 2 4 5 3");
    CHECK(seq(p2) == "4 5 2 3 1");
    CHECK(inorderStr(t1) == "4 2 5 1 3");
    CHECK(inorderStr(t2) == "4 2 5 1 3");
    deleteTree(t1); deleteTree(t2);
}

static void test_tree_metrics(Tester& T)
{
    TreeNode* r = sampleTree();
    CHECK_EQ(findHeight(r), 3);
    CHECK_EQ(findHeight(NULL), 0);
    CHECK_EQ(findHeight(r->right), 1);
    CHECK_EQ(findDiameter(r), 3);
    CHECK_EQ(depth(r), 1);
    CHECK_EQ(depth(r->left->left), 3);
    CHECK_EQ(getMaxDepth(r), 3);
    CHECK(findLCA(r->left->left, r->left->right) == r->left);
    CHECK(findLCA(r->left->left, r->right) == r);
    CHECK(findLCA(r->left, r->left->left) == r->left);
    deleteTree(r);
}

static void test_mirror(Tester& T)
{
    TreeNode* r = sampleTree();
    r = mirror(r);
    CHECK(inorderStr(r) == "3 1 5 2 4");
    List pre; preOrder(r, &pre);
    CHECK(seq(pre) == "1 3 2 5 4");
    r = mirror(r);
    CHECK(inorderStr(r) == "4 2 5 1 3");
    CHECK(mirror(NULL) == NULL);
    deleteTree(r);
}

static void test_random_tree_builder(Tester& T)
{
    srand(11);
    TreeNode* t = constructBinaryTree(10);
    List l; preOrder(t, &l);
    CHECK_EQ(l.getCount(), 10);
    bool seen[101] = { false };
    bool unique = true;
    for (TreeNode* c = l.getHead(); c; c = c->right)
    {
        if (seen[c->value]) unique = false;
        seen[c->value] = true;
    }
    CHECK(unique);
    CHECK(constructBinaryTree(0) == NULL);
    CHECK(constructBinaryTree(102) == NULL);
    deleteTree(t);
}

// ================================================================ HEAP
static void test_max_heap_class(Tester& T)
{
    maxHeap h(10);
    int vals[] = { 5, 17, 3, 42, 8, 21 };
    for (int i = 0; i < 6; i++) h.insert(vals[i]);
    CHECK_EQ(h.getSize(), 6);
    CHECK_EQ(h.getValue(0), 42);
    int arr[10]; h.copyTo(arr);
    CHECK(isMaxHeap(arr, 6));

    int expect[] = { 42, 21, 17, 8, 5, 3 };
    bool ok = true;
    for (int i = 0; i < 6; i++) if (h.extractMax() != expect[i]) ok = false;
    CHECK(ok);
    CHECK_EQ(h.extractMax(), -1);

    maxHeap full(2); full.insert(1); full.insert(2); full.insert(3);
    CHECK_EQ(full.getSize(), 2);

    maxHeap a(5); a.insert(4); a.insert(9);
    maxHeap b = a;                 // copy ctor must deep copy
    b.insert(100);
    CHECK_EQ(a.getSize(), 2);
    CHECK_EQ(b.getSize(), 3);
}

static void test_heap_functions(Tester& T)
{
    int bad[] = { 1, 5, 3 };
    int good[] = { 9, 5, 3, 1, 2 };
    CHECK(!isMaxHeap(bad, 3));
    CHECK(isMaxHeap(good, 5));
    CHECK(isMaxHeap(bad, 1));

    int arr[] = { 4, 10, 3, 5, 1 };
    for (int i = 5 / 2 - 1; i >= 0; i--) shiftDown(i, 5, arr);
    CHECK(isMaxHeap(arr, 5));
    CHECK_EQ(arr[0], 10);

    int s[] = { 9, 3, 7, 1, 8, 2, 6 };
    heapSort(s, 7);
    CHECK(isSortedAscending(s, 7));
    CHECK_EQ(s[0], 1); CHECK_EQ(s[6], 9);

    int one[] = { 5 }; heapSort(one, 1); CHECK_EQ(one[0], 5);

    int h[] = { 9, 5, 3, 1, 2 };
    CHECK_EQ(largerChildIndex(0, 5, h), -1);
    int h2[] = { 1, 5, 3 };
    CHECK_EQ(largerChildIndex(0, 3, h2), 1);
    int h3[] = { 1, 3, 5 };
    CHECK_EQ(largerChildIndex(0, 3, h3), 2);

    int sw[] = { 1, 5, 3, 4, 2 };       // 1 sinks: swap with 5, then with 4
    CHECK_EQ(shiftDownSwaps(0, 5, sw), 2);
    CHECK(isMaxHeap(sw, 5));

    int b[] = { 1, 2, 3, 4, 5, 6, 7 };
    int swaps = buildHeapSwaps(b, 7);
    CHECK(swaps > 0);
    CHECK(isMaxHeap(b, 7));
    int hs[] = { 9, 5, 8, 1, 2, 3, 4 };
    CHECK(isMaxHeap(hs, 7));
    int steps = heapSortSteps(hs, 7);
    CHECK(steps >= 6);
    CHECK(isSortedAscending(hs, 7));
}

static void test_complete_tree(Tester& T)
{
    TreeNode* r = sampleTree();
    CHECK(isCompleteTree(r));
    TreeNode* g = new TreeNode(1);
    addright(g, 2, 2);
    CHECK(!isCompleteTree(g));
    CHECK(isCompleteTree(NULL));
    deleteTree(r); deleteTree(g);
}

static void test_act4_helpers(Tester& T)
{
    CHECK_EQ(act4Capacity(0), 2);
    CHECK_EQ(act4Capacity(4), 8);
    CHECK_EQ(act4Stars(4, 4, 0), 3);
    CHECK_EQ(act4Stars(5, 4, 1), 2);
    CHECK_EQ(act4Stars(9, 4, 3), 1);
    CHECK_EQ(act4Score(5, 1), 900);
    CHECK_EQ(act4Score(200, 0), 0);
    srand(2);
    int arr[15], n;
    generateRandomArray(arr, n, 7, 10);
    CHECK(n >= 7 && n <= 10);
    bool uniq = true;
    for (int i = 0; i < n; i++) for (int j = i + 1; j < n; j++) if (arr[i] == arr[j]) uniq = false;
    CHECK(uniq);
    int h[15], hn;
    makeRandomHeap(h, hn, 6, 9);
    CHECK(isMaxHeap(h, hn));
    int a[] = { 1, 2, 3 }, b[] = { 1, 2, 3 }, c[] = { 1, 3, 2 };
    CHECK(sameArray(a, b, 3));
    CHECK(!sameArray(a, c, 3));
    int hint[2];
    int t[] = { 1, 5, 3 };
    CHECK(getNextstep(t, 3, hint));
    CHECK_EQ(hint[0], 0); CHECK_EQ(hint[1], 1);
    int good[] = { 5, 1, 3 };
    CHECK(!getNextstep(good, 3, hint));
}

// ================================================================ ACT 1
static void test_act1_level1(Tester& T)
{
    TreeNode* r = sampleTree();
    act1Level1 L;
    CHECK_EQ(L.getState(), act1Level1::LOST);          // before start()
    L.start(r, act1Level1::PREORDER);
    CHECK_EQ(L.getState(), act1Level1::PLAYING);
    CHECK_EQ(L.getN(), 5);
    CHECK_EQ(L.getCapacity(), 10);
    CHECK_EQ(L.getNextId(), 1);
    int order[] = { 1, 2, 4, 5, 3 };
    bool allCorrect = true;
    for (int i = 0; i < 5; i++) if (L.checkClick(order[i]) != act1Level1::CORRECT) allCorrect = false;
    CHECK(allCorrect);
    CHECK_EQ(L.getState(), act1Level1::WON);
    CHECK_EQ(L.stars(), 3);
    CHECK_EQ(L.score(), 950);
    CHECK_EQ(L.checkClick(1), act1Level1::IGNORED);    // finished

    L.start(r, act1Level1::INORDER);
    CHECK_EQ(L.getNextId(), 4);
    CHECK_EQ(L.checkClick(1), act1Level1::WRONG);
    CHECK_EQ(L.getPenalties(), 1);
    CHECK_EQ(L.checkClick(500), act1Level1::IGNORED);  // out of range
    CHECK_EQ(L.orderOf(4), 0);
    L.checkClick(4);
    CHECK_EQ(L.orderOf(4), 1);

    L.start(r, act1Level1::POSTORDER);
    CHECK_EQ(L.getNextId(), 4);
    L.start(r, act1Level1::LEVELORDER);
    CHECK_EQ(L.getNextId(), 1);
    L.checkClick(1); CHECK_EQ(L.getNextId(), 2);

    L.start(r, act1Level1::PREORDER);
    for (int i = 0; i < 10; i++) L.checkClick(3);      // always wrong first click
    CHECK_EQ(L.getState(), act1Level1::LOST);
    CHECK_EQ(L.stars(), 0);
    CHECK_EQ(L.score(), 0);

    L.start(NULL, act1Level1::PREORDER);
    CHECK_EQ(L.getState(), act1Level1::LOST);
    deleteTree(r);
}

static void test_act1_level2(Tester& T)
{
    for (int mode = 0; mode < 2; mode++)
    {
        srand(7 + mode);
        act1Level2 L;
        L.start(mode == 0 ? act1Level2::INPRE : act1Level2::INPOST, 6);
        int n = L.getN();
        CHECK_EQ(n, 6);

        // wrong check first
        CHECK(!L.check());
        CHECK_EQ(L.getWrongChecks(), 1);

        int in[15], other[15];
        for (int i = 0; i < n; i++) { in[i] = L.getInSeq(i); other[i] = L.getOtherSeq(i); }
        TreeNode* ans = (mode == 0) ? buildPre(other, in, n) : buildPost(other, in, n);

        linkRec(L, ans);
        CHECK_EQ(L.linkCount(), n - 1);
        CHECK(L.check());
        CHECK_EQ(L.getState(), act1Level2::WON);
        CHECK_EQ(L.stars(), 2);                        // 1 wrong check before
        CHECK_EQ(L.score(), 850);
        deleteTree(ans);
    }

    // connect() rules
    srand(3);
    act1Level2 L; L.start(act1Level2::INPRE, 5);
    CHECK(L.connect(0, act1Level2::LEFT, 1));
    CHECK(!L.connect(1, act1Level2::LEFT, 0));         // would create a cycle
    CHECK(!L.connect(0, act1Level2::LEFT, 0));         // self link
    CHECK(!L.connect(0, 5, 2));                        // bad side
    CHECK_EQ(L.getParent(1), 0);
    L.detach(1);
    CHECK_EQ(L.getParent(1), -1);
    L.connect(0, act1Level2::LEFT, 1); L.clearLinks();
    CHECK_EQ(L.linkCount(), 0);

    // hint walk should eventually reproduce the solution
    L.start(act1Level2::INPRE, 7);
    int p, s, c, guard = 0;
    while (guard++ < 60 && L.getHint(p, s, c)) L.connect(p, s, c);
    CHECK(L.check());
}

static void test_act1_level3(Tester& T)
{
    TreeNode* r = sampleTree();
    srand(5);
    act1Level3 L;
    CHECK_EQ(L.getState(), act1Level3::LOST);
    L.start(r);
    CHECK_EQ(L.getN(), 5);
    CHECK_EQ(L.getCapacity(), 10);
    int guard = 0;
    while (L.getState() == act1Level3::PLAYING && guard++ < 50)
        L.submit(L.getCurrentHeight());
    CHECK_EQ(L.getState(), act1Level3::WON);
    CHECK_EQ(L.stars(), 3);
    CHECK_EQ(L.getSolved(), 5);
    CHECK_EQ(L.submit(1), act1Level3::IGNORED);

    L.start(r);
    int wrong = L.getCurrentHeight() + 1;
    CHECK_EQ(L.submit(wrong), act1Level3::WRONG);
    CHECK_EQ(L.getPenalties(), 1);
    CHECK_EQ(L.timeUp(), act1Level3::TIMEOUT);
    CHECK_EQ(L.getTimeouts(), 1);
    CHECK_EQ(L.getPenalties(), 2);
    CHECK_EQ(L.getAlarm(), 2);
    for (int i = 0; i < 20; i++) L.timeUp();
    CHECK_EQ(L.getState(), act1Level3::LOST);

    // order must respect children-before-parent
    L.start(r);
    bool childFirst = true;
    bool done[101] = { false };
    for (int i = 0; i < 5; i++)
    {
        int id = L.getCurrentId();
        TreeNode* nd = findNodeById(r, id);
        if (nd->left && !done[nd->left->id]) childFirst = false;
        if (nd->right && !done[nd->right->id]) childFirst = false;
        done[id] = true;
        L.submit(L.getCurrentHeight());
    }
    CHECK(childFirst);
    deleteTree(r);
}

static void test_act1_level4(Tester& T)
{
    TreeNode* r = sampleTree();
    srand(1);
    act1Level4 L;
    L.start(r, 3);
    CHECK_EQ(L.getState(), act1Level4::PLAYING);
    CHECK_EQ(L.getTotalRounds(), 3);
    CHECK_EQ(L.getCapacity(), 7);
    int guard = 0;
    while (L.getState() == act1Level4::PLAYING && guard++ < 20)
    {
        TreeNode* a = L.getNodeA(); TreeNode* b = L.getNodeB();
        TreeNode* lca = findLCA(a, b);
        int len = depth(a) + depth(b) - 2 * depth(lca);
        L.submit(lca->id, len);
    }
    CHECK_EQ(L.getState(), act1Level4::WON);
    CHECK_EQ(L.stars(), 3);
    CHECK_EQ(L.getCurrentRound(), 3);

    L.start(r, 3);
    TreeNode* a = L.getNodeA(); TreeNode* b = L.getNodeB();
    TreeNode* lca = findLCA(a, b);
    int len = depth(a) + depth(b) - 2 * depth(lca);
    CHECK_EQ(L.submit(lca->id, len + 1), act1Level4::WRONG);
    CHECK_EQ(L.getPenalties(), 1);
    CHECK_EQ(L.submit(lca->id, len), act1Level4::CORRECT);

    TreeNode* single = new TreeNode(1); single->id = 1;
    L.start(single, 3);
    CHECK_EQ(L.getState(), act1Level4::LOST);
    deleteTree(single); deleteTree(r);
}

static void test_act1_level5(Tester& T)
{
    TreeNode* r = sampleTree();
    act1Level5 L;
    L.start(r);
    CHECK_EQ(L.getN(), 5);
    CHECK_EQ(L.getCapacity(), 10);
    CHECK(!L.swapChildren(4));                          // leaf
    CHECK(!L.swapChildren(77));                         // missing
    CHECK(!L.submit());                                 // not mirrored yet
    CHECK_EQ(L.getPenalties(), 1);
    CHECK(L.getHintId() != -1);
    CHECK(L.swapChildren(1));
    CHECK(L.swapChildren(2));
    CHECK_EQ(L.getSwaps(), 2);
    CHECK(L.submit());
    CHECK_EQ(L.getState(), act1Level5::WON);
    CHECK_EQ(L.stars(), 2);                             // one penalty

    L.start(r);
    L.swapChildren(1);
    L.resetWorking();
    CHECK_EQ(L.getSwaps(), 0);
    CHECK(inorderStr(L.getWorkingRoot()) == "4 2 5 1 3");
    CHECK(inorderStr(r) == "4 2 5 1 3");                // original untouched
    for (int i = 0; i < 10; i++) L.submit();
    CHECK_EQ(L.getState(), act1Level5::LOST);
    deleteTree(r);
}

// ================================================================ ACT 2: BST
static void fillBST(BST& b)
{
    int keys[] = { 50, 30, 70, 20, 40, 60, 80 };
    for (int i = 0; i < 7; i++) b.insertValue(keys[i]);
}

static void test_bst_basic(Tester& T)
{
    BST b;
    CHECK(b.isEmpty());
    fillBST(b);
    CHECK(!b.isEmpty());
    CHECK_EQ(b.countNodes(), 7);
    CHECK_EQ(b.height(b.getRoot()), 3);
    CHECK(b.isValidBST());
    CHECK(!b.insertValue(50));                          // duplicate
    CHECK_EQ(b.countNodes(), 7);

    int alarm = 0;
    BSTNode* n = b.searchNode(60, alarm);
    CHECK(n != NULL && n->value == 60);
    CHECK_EQ(alarm, 3);
    CHECK(b.searchNode(55, alarm) == NULL);
    CHECK_EQ(b.searchPathLength(60), 3);
    CHECK_EQ(b.searchPathLength(50), 1);
    CHECK_EQ(b.insertPathLength(65), 3);
    CHECK_EQ(b.insertPathLength(50), -1);

    int ids[10];
    CHECK_EQ(b.pathTo(60, ids, 10), 3);
    CHECK_EQ(b.pathTo(99, ids, 10), -1);

    BSTNode* root = b.getRoot();
    CHECK_EQ(b.compareAt(root, 10), 1);                 // key smaller, go left
    CHECK_EQ(b.compareAt(root, 90), -1);
    CHECK_EQ(b.compareAt(root, 50), 0);
    CHECK(b.childToward(root, 10) == root->left);
    CHECK(b.childToward(root, 90) == root->right);
    CHECK(b.childToward(root, 50) == NULL);

    CHECK_EQ(BST::capacityFor(4), 8);
    CHECK_EQ(BST::capacityFor(0), 2);
}

static void test_bst_navigation(Tester& T)
{
    BST b; fillBST(b);
    int a = 0;
    BSTNode* fifty = b.searchNode(50, a);
    CHECK(b.successor(fifty)->value == 60);
    CHECK(b.predecessor(fifty)->value == 40);
    CHECK(b.findMinimum(b.getRoot())->value == 20);
    CHECK(b.findMaximum(b.getRoot())->value == 80);
    CHECK(b.successor(b.findMaximum(b.getRoot())) == NULL);
    CHECK(b.kthSmallest(1)->value == 20);
    CHECK(b.kthSmallest(3)->value == 40);
    CHECK(b.kthSmallest(7)->value == 80);
    CHECK(b.kthSmallest(8) == NULL);
    CHECK(b.kthSmallest(0) == NULL);
    CHECK(b.lcaBST(20, 40)->value == 30);
    CHECK(b.lcaBST(20, 80)->value == 50);
    CHECK(b.lcaBST(60, 80)->value == 70);
    CHECK(b.lcaBST(20, 99) == NULL);
}

static void test_bst_delete(Tester& T)
{
    BST b; fillBST(b);
    CHECK_EQ(b.classifyDelete(20), 1);                  // leaf
    CHECK_EQ(b.classifyDelete(30), 3);                  // two children
    CHECK_EQ(b.classifyDelete(99), 0);                  // missing
    CHECK(b.deleteNode(20));                            // leaf
    CHECK(b.isValidBST());
    CHECK_EQ(b.classifyDelete(30), 2);                  // now one child
    CHECK(b.deleteNode(30));                            // one child
    CHECK(b.isValidBST());
    CHECK_EQ(b.countNodes(), 5);
    CHECK(b.deleteNode(50));                            // two children (root)
    CHECK(b.isValidBST());
    CHECK_EQ(b.getRoot()->value, 60);                   // inorder successor
    CHECK(!b.deleteNode(999));
    CHECK_EQ(b.countNodes(), 4);
    while (!b.isEmpty()) b.deleteNode(b.getRoot()->value);
    CHECK_EQ(b.countNodes(), 0);
    CHECK(b.isValidBST());
}

static void test_bst_range_query(Tester& T)
{
    BST b; fillBST(b);
    RangeResult* r = b.rangeQuery(35, 65);
    int got[20];
    int cnt = 0;
    for (; r && cnt < 20; r = r->next) got[cnt++] = r->data->value;
    CHECK_EQ(cnt, 3);
    CHECK(cnt == 3 && got[0] == 40 && got[1] == 50 && got[2] == 60);
    CHECK_EQ(b.getRangeInRange(), 3);
    CHECK_EQ(b.getRangeVisited(), 5);
    CHECK_EQ(b.getRangeSkippedNodes(), 2);
    CHECK_EQ(b.getRangeSkippedSubtrees(), 2);
    CHECK_EQ(b.rangeOptimalSteps(35, 65), 5);
    CHECK(b.rangeQuery(65, 35) == NULL);                // lo > hi
    CHECK_EQ(b.rangeOptimalSteps(0, 100), 7);           // nothing to prune
}

static void test_bst_serialization_and_generators(Tester& T)
{
    BST b; fillBST(b);
    string s = b.treeToString();
    BST c;
    CHECK(c.loadFromString(s));
    CHECK_EQ(c.countNodes(), 7);
    CHECK(c.treeToString() == s);
    CHECK(c.isValidBST());

    BST bad;
    CHECK(!bad.loadFromString("5 3"));                  // truncated
    CHECK(bad.isEmpty());
    CHECK(!bad.loadFromString("5 # # 9"));              // extra token
    CHECK(!bad.loadFromString("5 9 # # #"));            // violates BST order
    CHECK(!bad.loadFromString("abc"));

    BST g1, g2;
    g1.generateRandom(42, 12, 0);
    g2.generateRandom(42, 12, 0);
    CHECK_EQ(g1.countNodes(), 12);
    CHECK(g1.isValidBST());
    CHECK(g1.treeToString() == g2.treeToString());      // deterministic per seed

    BST g3; g3.generateRandom(7, 200, 0);               // clamped to 99
    CHECK_EQ(g3.countNodes(), 99);

    BST lazy;
    lazy.generateLazyArchitect(10);
    CHECK_EQ(lazy.countNodes(), 10);
    CHECK_EQ(lazy.height(lazy.getRoot()), 10);          // a corridor
    CHECK(lazy.isValidBST());

    BST built;
    int keys[] = { 5, 3, 8, 3 };
    CHECK_EQ(built.buildFromKeys(keys, 4), 3);          // duplicate dropped

    NodeInfo info[20];
    BST l; fillBST(l);
    l.computeLayout(100, 1180, 175, 605);
    CHECK_EQ(l.collectNodes(info, 20), 7);
    CHECK_EQ(info[0].depth, 1);
    CHECK_EQ(info[0].parentId, -1);
    float minX = 1e9f, maxX = -1e9f;
    for (int i = 0; i < 7; i++)
    {
        if (info[i].x < minX) minX = info[i].x;
        if (info[i].x > maxX) maxX = info[i].x;
    }
    CHECK(minX >= 99.0f && maxX <= 1181.0f);
}

static void test_bst_events(Tester& T)
{
    BST b; fillBST(b);
    int a = 0;
    b.searchNode(60, a);
    CHECK(b.getEventCount() > 0);
    CHECK(!b.describeEvent(b.getEvents()).empty());
    b.clearEvents();
    CHECK_EQ(b.getEventCount(), 0);
    int remaining, alarm = 0;
    BSTNode* hit = b.bossSearch(60, alarm, remaining, 0, 0);
    CHECK(hit != NULL);
    CHECK(b.bossSearch(61, alarm, remaining, 0, 0) == NULL);
}

// ================================================================ ACT 3: AVL
static void test_avl_basic_rotations(Tester& T)
{
    AVLTree t;
    t.insert(3); t.insert(2); t.insert(1);              // LL
    CHECK_EQ(t.getAVLRoot()->value, 2);
    CHECK_EQ(t.getLastRotationCount(), 1);
    CHECK(t.isBalanced());

    AVLTree rr; rr.insert(1); rr.insert(2); rr.insert(3);
    CHECK_EQ(rr.getAVLRoot()->value, 2);

    AVLTree lr; lr.insert(3); lr.insert(1); lr.insert(2);
    CHECK_EQ(lr.getAVLRoot()->value, 2);
    CHECK_EQ(lr.getLastRotationCount(), 2);

    AVLTree rl; rl.insert(1); rl.insert(3); rl.insert(2);
    CHECK_EQ(rl.getAVLRoot()->value, 2);
    CHECK_EQ(rl.getLastRotationCount(), 2);

    CHECK(!t.insert(2));                                // duplicate
    CHECK(t.containsKey(1));
    CHECK(!t.containsKey(9));
}

static void test_avl_stress(Tester& T)
{
    AVLTree t;
    for (int i = 1; i <= 31; i++) t.insert(i);          // worst-case sorted input
    CHECK_EQ(t.nodeCount(), 31);
    CHECK_EQ(t.treeHeight(), 5);                        // perfectly balanced
    CHECK(t.isBalanced());
    CHECK(t.validateStructure());

    bool alwaysBalanced = true;
    for (int i = 1; i <= 20; i++)
    {
        t.deleteNode(i);
        if (!t.isBalanced()) alwaysBalanced = false;
    }
    CHECK(alwaysBalanced);
    CHECK_EQ(t.nodeCount(), 11);
    CHECK(!t.deleteNode(1));

    AVLTree big;
    unsigned st = 99;
    for (int i = 0; i < 300; i++) big.insert((int)(AVLTree::nextRandom(st) % 1000));
    CHECK(big.isBalanced());
    int n = big.nodeCount();
    CHECK(big.treeHeight() <= 15);                      // ~1.44 * log2(n)
    CHECK(n > 200);
}

static void test_avl_order_statistics(Tester& T)
{
    AVLTree t;
    for (int i = 1; i <= 31; i++) t.insert(i);
    CHECK_EQ(t.kth(10)->value, 10);
    CHECK_EQ(t.kth(1)->value, 1);
    CHECK_EQ(t.kth(31)->value, 31);
    CHECK(t.kth(32) == NULL);
    CHECK(t.kth(0) == NULL);
    CHECK_EQ(t.rank(10), 10);
    CHECK_EQ(t.rank(0), 0);
    CHECK_EQ(t.rank(100), 31);
    CHECK_EQ(t.countInRange(5, 15), 11);
    CHECK_EQ(t.countInRange(15, 5), 0);
    int path[20];
    int len = t.kthPath(10, path, 20);
    CHECK(len >= 1 && len <= 6);
    CHECK(t.rankPath(10, path, 20) >= 1);
    CHECK_EQ(t.subtreeSizeOf(t.getAVLRoot()->id), 31);
}

static void test_avl_all_four_cases(Tester& T)
{
    int cases[4] = { AVLTree::ROT_LL, AVLTree::ROT_RR, AVLTree::ROT_LR, AVLTree::ROT_RL };
    for (int k = 0; k < 4; k++)
    {
        for (unsigned seed = 0; seed < 6; seed++)
        {
            AVLTree a;
            CHECK(a.loadCase(cases[k], seed));
            CHECK(!a.isBalanced());
            int pivot = -1, rc = AVLTree::ROT_NONE;
            CHECK(a.getHint(pivot, rc));
            CHECK_EQ(rc, cases[k]);
            CHECK(a.isNodeUnbalanced(pivot));
            int rot = a.repairAll();
            if (seed % 3 == 0) CHECK_EQ(rot, AVLTree::expectedRotationSteps(cases[k]));
            CHECK(rot >= 1);
            CHECK(a.isBalanced());
            CHECK(a.validateStructure());
        }
    }
    CHECK_EQ(AVLTree::expectedRotationSteps(AVLTree::ROT_LL), 1);
    CHECK_EQ(AVLTree::expectedRotationSteps(AVLTree::ROT_RL), 2);
    CHECK_EQ(AVLTree::expectedRotationSteps(AVLTree::ROT_NONE), 0);
    CHECK(string(AVLTree::rotationName(AVLTree::ROT_LR)) == "LR");
    CHECK(string(AVLTree::rotationName(AVLTree::ROT_NONE)) == "NONE");
}

static void test_avl_manual_rotation_and_unbalanced_build(Tester& T)
{
    AVLTree t;
    int keys[] = { 1, 2, 3 };
    CHECK_EQ(t.buildUnbalanced(keys, 3), 3);
    CHECK(!t.isBalanced());
    int ids[10];
    CHECK_EQ(t.getUnbalancedIds(ids, 10), 1);
    CHECK_EQ(t.detectRotationCaseById(t.getAVLRoot()->id), AVLTree::ROT_RR);
    CHECK(t.applyManualRotation(t.getAVLRoot()->id, 'L'));
    CHECK_EQ(t.getAVLRoot()->value, 2);
    CHECK(t.isBalanced());
    CHECK(!t.applyManualRotation(t.getAVLRoot()->id, 'X'));

    AVLTree u;
    CHECK(u.loadRandomUnbalanced(5, 9) > 0);
    CHECK(u.nodeCount() >= 3);
    u.repairAll();
    CHECK(u.isBalanced());
}

static void test_avl_prediction(Tester& T)
{
    AVLTree t; t.insert(1); t.insert(2);
    CHECK_EQ(t.predictInsert(3), AVLTree::ROT_RR);
    CHECK_EQ(t.nodeCount(), 2);                          // real tree untouched
    AVLTree l; l.insert(3); l.insert(2);
    CHECK_EQ(l.predictInsert(1), AVLTree::ROT_LL);
    AVLTree lr; lr.insert(3); lr.insert(1);
    CHECK_EQ(lr.predictInsert(2), AVLTree::ROT_LR);
    AVLTree rl; rl.insert(1); rl.insert(3);
    CHECK_EQ(rl.predictInsert(2), AVLTree::ROT_RL);
    CHECK_EQ(t.predictInsert(1), AVLTree::ROT_NONE);     // duplicate
    CHECK_EQ(t.predictInsert(0), AVLTree::ROT_NONE);     // (1,2)+0 stays balanced
    CHECK_EQ(t.predictDelete(99), AVLTree::ROT_NONE);

    unsigned rng = 123;
    AVLTree g; g.buildRandomAVL(4, 8, 99);
    int key = g.findInsertKeyFor(AVLTree::ROT_NONE, rng);
    CHECK(key == -1 || !g.containsKey(key));
}

static void test_avl_build_and_load(Tester& T)
{
    AVLTree t;
    CHECK_EQ(t.buildRandomAVL(1, 20, 99), 20);
    CHECK_EQ(t.nodeCount(), 20);
    CHECK(t.isBalanced());

    string s = t.treeToString();
    AVLTree u;
    CHECK(u.loadFromString(s));
    CHECK_EQ(u.nodeCount(), 20);
    CHECK(u.validateStructure());
    CHECK(u.treeToString() == s);

    AVLTree bad;
    CHECK(!bad.loadFromString("5 3"));
    CHECK(!bad.loadFromString("5 9 # # #"));
    CHECK(bad.loadFromString("   "));                    // blank = empty tree
    CHECK_EQ(bad.nodeCount(), 0);

    AVLTree c;
    c.cloneFrom(t);
    c.insert(1000);
    CHECK_EQ(c.nodeCount(), 21);
    CHECK_EQ(t.nodeCount(), 20);                         // clone is independent

    AVLNodeInfo info[40];
    CHECK_EQ(t.collectAVLNodes(info, 40), 20);
    bool flagsOk = true;
    for (int i = 0; i < 20; i++) if (info[i].unbalanced) flagsOk = false;
    CHECK(flagsOk);

    AVLTree ops; ops.buildRandomAVL(3, 10, 50);
    unsigned rng = 5; int key; bool wasInsert;
    for (int i = 0; i < 50; i++) ops.randomOperation(rng, key, wasInsert);
    CHECK(ops.isBalanced());
}

static void test_avl_size_audit(Tester& T)
{
    AVLTree t; t.buildRandomAVL(8, 10, 99);
    CHECK_EQ(t.findWrongSizeId(), -1);
    int rootId = t.getAVLRoot()->id;
    int trueSize = t.subtreeSizeOf(rootId);
    CHECK(t.forceStoredSize(rootId, trueSize + 2));
    CHECK_EQ(t.findWrongSizeId(), rootId);
    t.forceStoredSize(rootId, trueSize);
    CHECK_EQ(t.findWrongSizeId(), -1);
}

// ================================================================ ACT 4 levels
static void test_act4_level1(Tester& T)
{
    srand(3);
    act4Level1 L; L.start();
    int n = L.getN();
    CHECK(n >= 7 && n <= 10);
    CHECK_EQ(L.getState(), act4Level1::PLAYING);
    CHECK_EQ(L.check(), act4Level1::CHECK_NOT_CONNECTED);
    CHECK_EQ(L.getAlarm(), 0);                           // not counted

    int idx[15];
    for (int i = 0; i < n; i++) idx[i] = i;
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++)
            if (L.getVal(idx[j]) > L.getVal(idx[i])) { int tmp = idx[i]; idx[i] = idx[j]; idx[j] = tmp; }
    for (int i = 0; i < n; i++)
        for (int s = 0; s < 2; s++)
        {
            int k = 2 * i + 1 + s;
            if (k < n) L.connect(idx[i], s, idx[k]);
        }
    CHECK_EQ(L.linkCount(), n - 1);
    CHECK_EQ(L.check(), act4Level1::CHECK_OK);
    CHECK_EQ(L.getState(), act4Level1::WON);
    CHECK_EQ(L.stars(), 3);
    CHECK_EQ(L.score(), 1000);

    // wrong shape: a chain is connected but not complete
    L.start();
    n = L.getN();
    for (int i = 0; i + 1 < n; i++) L.connect(i, act4Level1::LEFT, i + 1);
    CHECK_EQ(L.check(), act4Level1::CHECK_NOT_COMPLETE);
    CHECK_EQ(L.getAlarm(), 1);
    for (int i = 0; i < 4; i++) L.check();
    CHECK_EQ(L.getState(), act4Level1::LOST);

    // cycles are rejected
    L.start();
    CHECK(L.connect(0, act4Level1::LEFT, 1));
    CHECK(!L.connect(1, act4Level1::LEFT, 0));

    // hint-driven solution
    L.start();
    int p, side, c, guard = 0;
    while (guard++ < 100 && L.getHint(p, side, c))
    {
        if (side == act4Level1::HINT_UNLINK) L.detach(p);
        else L.connect(p, side, c);
    }
    CHECK_EQ(L.check(), act4Level1::CHECK_OK);
}

static void test_act4_level2(Tester& T)
{
    srand(5);
    act4Level2 L; L.start();
    int n = L.getN();
    CHECK(n >= 7 && n <= 15);
    int copy[15];
    for (int i = 0; i < n; i++) copy[i] = L.getVal(i);
    CHECK(!isMaxHeap(copy, n));
    int needed = buildHeapSwaps(copy, n);
    CHECK(L.getCapacity() >= needed);                    // capacity must allow an optimal solve

    int a, b, guard = 0;
    while (L.getState() == act4Level2::PLAYING && guard++ < 100 && L.getHint(a, b))
        L.swapNodes(a, b);
    CHECK_EQ(L.getState(), act4Level2::WON);
    int arr[15];
    for (int i = 0; i < n; i++) arr[i] = L.getVal(i);
    CHECK(isMaxHeap(arr, n));

    srand(6); L.start();
    int v0 = L.getVal(0);
    L.swapNodes(0, 0);                                   // ignored
    CHECK_EQ(L.getAlarm(), 0);
    L.swapNodes(0, 1);
    CHECK_EQ(L.getAlarm(), 1);
    L.resetPuzzle();
    CHECK_EQ(L.getVal(0), v0);
}

static void test_act4_level3(Tester& T)
{
    srand(9);
    act4Level3 L; L.start();
    int n = L.getN();
    CHECK(n >= 7 && n <= 12);
    CHECK(!L.isRootRemoved());
    CHECK(!L.swapNodes(1, 2));                           // must remove root first
    CHECK_EQ(L.getPenalties(), 1);
    int a, b;
    CHECK(L.getHint(a, b));
    CHECK_EQ(a, 0); CHECK_EQ(b, n - 1);
    CHECK(L.removeRoot());
    CHECK(L.isRootRemoved());
    CHECK_EQ(L.getN(), n - 1);
    CHECK(!L.removeRoot());                              // only once
    int guard = 0;
    while (L.getState() == act4Level3::PLAYING && guard++ < 50 && L.getHint(a, b))
        L.swapNodes(a, b);
    CHECK_EQ(L.getState(), act4Level3::WON);
    int arr[15];
    for (int i = 0; i < L.getN(); i++) arr[i] = L.getVal(i);
    CHECK(isMaxHeap(arr, L.getN()));
    CHECK(L.stars() >= 1);
}

static void test_act4_level4(Tester& T)
{
    srand(12);
    act4Level4 L; L.start();
    int n = L.getN();
    CHECK(n >= 6 && n <= 9);
    CHECK(L.isNeedExtract());
    CHECK_EQ(L.getHeapEnd(), n);
    int a, b, guard = 0;
    while (L.getState() == act4Level4::PLAYING && guard++ < 200 && L.getHint(a, b))
        L.swapNodes(a, b);
    CHECK_EQ(L.getState(), act4Level4::WON);
    int arr[15];
    for (int i = 0; i < n; i++) arr[i] = L.getVal(i);
    CHECK(isSortedAscending(arr, n));
    CHECK_EQ(L.stars(), 3);

    srand(13); L.start();
    CHECK(!L.swapNodes(1, 2));                           // wrong: needs root <-> last
    CHECK_EQ(L.getPenalties(), 1);
    L.resetPuzzle();
    CHECK_EQ(L.getHeapEnd(), L.getN());
}

static void test_act4_level5(Tester& T)
{
    srand(21);
    act4Level5 L; L.start();
    int n = L.getN();
    CHECK(n >= 6 && n <= 9);
    int arr[15];
    for (int i = 0; i < n; i++) arr[i] = L.getVal(i);
    CHECK(!isSortedAscending(arr, n));
    CHECK(!isMaxHeap(arr, n));
    CHECK(!L.isHeapBuilt());
    CHECK(!L.checkVault());                              // not sorted yet
    CHECK_EQ(L.getPenalties(), 1);
    CHECK_EQ(L.getAlarm(), 1);

    // selection-sort swaps; may run out of alarm, so only check the logic is consistent
    act4Level5 M; srand(22); M.start();
    int m = M.getN();
    for (int i = 0; i < m - 1 && M.getState() == act4Level5::PLAYING; i++)
    {
        int mn = i;
        for (int j = i + 1; j < m; j++) if (M.getVal(j) < M.getVal(mn)) mn = j;
        if (mn != i) M.swapNodes(i, mn);
    }
    CHECK(M.getState() == act4Level5::WON || M.getState() == act4Level5::LOST);
    if (M.getState() == act4Level5::WON)
    {
        int s[15]; for (int i = 0; i < m; i++) s[i] = M.getVal(i);
        CHECK(isSortedAscending(s, m));
    }
    M.resetPuzzle();
}

// ================================================================ main
int main()
{
    cout << "=== Vault Heist unit tests ===\n";
    Tester T;
    T.run("Tree traversals", test_traversals);
    T.run("Tree reconstruction (inorder+pre/post)", test_reconstruction);
    T.run("Tree metrics: height, diameter, depth, LCA", test_tree_metrics);
    T.run("Mirror", test_mirror);
    T.run("Random tree builder", test_random_tree_builder);
    T.run("maxHeap class", test_max_heap_class);
    T.run("Heap functions: heapify, heap sort, swaps", test_heap_functions);
    T.run("Complete tree check", test_complete_tree);
    T.run("Act 4 helpers (capacity, stars, score)", test_act4_helpers);
    T.run("Act 1 Level 1 (traversal order)", test_act1_level1);
    T.run("Act 1 Level 2 (reconstruction)", test_act1_level2);
    T.run("Act 1 Level 3 (height)", test_act1_level3);
    T.run("Act 1 Level 4 (LCA / path length)", test_act1_level4);
    T.run("Act 1 Level 5 (mirror)", test_act1_level5);
    T.run("BST basics", test_bst_basic);
    T.run("BST navigation (successor, k-th, LCA)", test_bst_navigation);
    T.run("BST delete (3 cases)", test_bst_delete);
    T.run("BST range query", test_bst_range_query);
    T.run("BST serialization and generators", test_bst_serialization_and_generators);
    T.run("BST events / boss search", test_bst_events);
    T.run("AVL rotations on insert", test_avl_basic_rotations);
    T.run("AVL stress and delete rebalancing", test_avl_stress);
    T.run("AVL order statistics", test_avl_order_statistics);
    T.run("AVL all four rotation cases (LL/RR/LR/RL)", test_avl_all_four_cases);
    T.run("AVL manual rotation", test_avl_manual_rotation_and_unbalanced_build);
    T.run("AVL insert prediction", test_avl_prediction);
    T.run("AVL build, clone, serialization", test_avl_build_and_load);
    T.run("AVL size audit", test_avl_size_audit);
    T.run("Act 4 Level 1 (build heap links)", test_act4_level1);
    T.run("Act 4 Level 2 (heapify)", test_act4_level2);
    T.run("Act 4 Level 3 (extract + sift down)", test_act4_level3);
    T.run("Act 4 Level 4 (heap sort)", test_act4_level4);
    T.run("Act 4 Level 5 (boss)", test_act4_level5);

    T.finish();
    return T.failed() == 0 ? 0 : 1;
}