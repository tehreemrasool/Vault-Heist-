#pragma once
#include <cstdlib>
#include "header.h"
#include <cmath>
#include <ctime>
#include <iostream>
using namespace std;

class act1Level1
{	

private:
	List* answerKey = new List;
	int n = 0;
	int next = 0;
	int alarm = 0;
	int penalties = 0;
	int capacity = 0;
	int state = LOST;       
	int type = PREORDER;
	int visitOrder[101] = { 0 };

public:
	static const int PREORDER = 0;
	static const int INORDER = 1;
	static const int POSTORDER = 2;
	static const int LEVELORDER = 3;

	static const int PLAYING = 0;
	static const int WON = 1;
	static const int LOST = 2;

	static const int IGNORED = 0;
	static const int CORRECT = 1;
	static const int WRONG = 2;
	~act1Level1() { delete answerKey; }

	void start(TreeNode* root, int t)
	{
		answerKey->clear();
		type = t;

		if (type == PREORDER)        preOrder(root, answerKey);
		else if (type == INORDER)    inOrder(root, answerKey);
		else if (type == POSTORDER)  postOrder(root, answerKey);
		else                         levelOrder(root, answerKey);

		n = answerKey->getCount();
		next = 0;
		alarm = 0;
		penalties = 0;
		state = (n > 0) ? PLAYING : LOST;

		capacity = (3 * n + 1) / 2 + 2;
		for (int i = 0; i < 101; i++) visitOrder[i] = 0;

		answerKey->setCurr(answerKey->getHead());
	}

	int checkClick(int id)
	{
		if (state != PLAYING) return IGNORED;
		if (id < 0 || id > 100) return IGNORED;

		alarm++;
		int r;

		if (id == answerKey->getCurrId())
		{
			visitOrder[id] = ++next;
			answerKey->moveCurr();
			r = CORRECT;
		}
		else
		{
			penalties++;
			r = WRONG;
		}

		if (next == n) state = WON;
		else if (alarm >= capacity) state = LOST;
		return r;
	}

	int getN() const { return n; }
	int getNext() const { return next; }
	int getAlarm() const { return alarm; }
	int getPenalties() const { return penalties; }
	int getCapacity() const { return capacity; }
	int getState() const { return state; }
	int getType() const { return type; }
	int orderOf(int id) const { return visitOrder[id]; }

	int stars() const
	{
		if (state != WON) return 0;
		if (alarm <= n && penalties == 0) return 3;
		if (alarm * 4 <= 5 * n && penalties <= 1) return 2;
		return 1;
	}
	int score() const
	{
		if (state != WON) return 0;
		int s = 1000 - 10 * alarm;
		return s < 0 ? 0 : s;
	}
};



//          level 2

class act1Level2
{
public:
	static const int MAXN = 15;
	static const int INPRE = 0;   
	static const int INPOST = 1;  
	static const int PLAYING = 0;
	static const int WON = 1;
	static const int LEFT = 0;
	static const int RIGHT = 1;

private:
	int n;
	int mode;
	int inSeq[MAXN];
	int otherSeq[MAXN];
	int val[MAXN];          
	int child[MAXN][2];     
	int par[MAXN];          
	TreeNode* answer;
	int state;
	int checks;
	int wrongChecks;

	void fillArray(List& l, int* out)
	{
		TreeNode* c = l.getHead();
		int i = 0;
		while (c != NULL && i < MAXN)
		{
			out[i++] = c->value;
			c = c->right;
		}
	}

	bool same(int i, TreeNode* t) const
	{
		if (i == -1 && t == NULL) return true;
		if (i == -1 || t == NULL) return false;
		if (val[i] != t->value) return false;
		return same(child[i][0], t->left) && same(child[i][1], t->right);
	}

public:
	act1Level2()
	{
		n = 0; mode = INPRE; answer = NULL; state = PLAYING;
		checks = 0; wrongChecks = 0;
		for (int i = 0; i < MAXN; i++)
		{
			inSeq[i] = otherSeq[i] = val[i] = 0;
			child[i][0] = child[i][1] = -1;
			par[i] = -1;
		}
	}
	act1Level2(const act1Level2&) = delete;
	act1Level2& operator=(const act1Level2&) = delete;

	~act1Level2() { deleteTree(answer); }

	void start(int m, int count)
	{
		deleteTree(answer);
		answer = NULL;

		mode = m;
		if (count < 2) count = 2;
		if (count > MAXN) count = MAXN;
		n = count;

		TreeNode* t = constructBinaryTree(n);
		List a, b;
		inOrder(t, &a);
		if (mode == INPRE) preOrder(t, &b);
		else               postOrder(t, &b);
		fillArray(a, inSeq);
		fillArray(b, otherSeq);
		deleteTree(t);

		if (mode == INPRE) answer = buildPre(otherSeq, inSeq, n);
		else               answer = buildPost(otherSeq, inSeq, n);
		for (int i = 0; i < n; i++) val[i] = inSeq[i];
		for (int i = n - 1; i > 0; i--)
		{
			int j = rand() % (i + 1);
			int tmp = val[i]; val[i] = val[j]; val[j] = tmp;
		}

		for (int i = 0; i < MAXN; i++)
		{
			child[i][0] = child[i][1] = -1;
			par[i] = -1;
		}
		state = PLAYING;
		checks = 0;
		wrongChecks = 0;
	}

	void clearLinks()
	{
		if (state != PLAYING) return;
		for (int i = 0; i < MAXN; i++)
		{
			child[i][0] = child[i][1] = -1;
			par[i] = -1;
		}
	}
	void detach(int c)
	{
		if (state != PLAYING) return;
		if (c < 0 || c >= n) return;
		int p = par[c];
		if (p == -1) return;
		if (child[p][0] == c) child[p][0] = -1;
		if (child[p][1] == c) child[p][1] = -1;
		par[c] = -1;
	}

	bool connect(int p, int side, int c)
	{
		if (state != PLAYING) return false;
		if (p < 0 || p >= n || c < 0 || c >= n || p == c) return false;
		if (side != LEFT && side != RIGHT) return false;

		for (int a = p; a != -1; a = par[a])
			if (a == c) return false;

		detach(c);                        
		if (child[p][side] != -1)         
			detach(child[p][side]);

		child[p][side] = c;
		par[c] = p;
		return true;
	}

	int linkCount() const
	{
		int k = 0;
		for (int i = 0; i < n; i++) if (par[i] != -1) k++;
		return k;
	}

	bool check()
	{
		if (state != PLAYING) return false;
		checks++;
		int roots = 0, rootIdx = -1;
		for (int i = 0; i < n; i++)
			if (par[i] == -1) { roots++; rootIdx = i; }

		bool ok = (roots == 1) && same(rootIdx, answer);
		if (ok) state = WON;
		else wrongChecks++;
		return ok;
	}

	int getN() const { return n; }
	int getMode() const { return mode; }
	int getState() const { return state; }
	int getVal(int i) const { return val[i]; }
	int getChild(int i, int side) const { return child[i][side]; }
	int getParent(int i) const { return par[i]; }
	int getInSeq(int i) const { return inSeq[i]; }
	int getOtherSeq(int i) const { return otherSeq[i]; }
	int getWrongChecks() const { return wrongChecks; }

	int stars() const
	{
		if (state != WON) return 0;
		if (wrongChecks == 0) return 3;
		if (wrongChecks == 1) return 2;
		return 1;
	}
	int score() const
	{
		if (state != WON) return 0;
		int s = 1000 - 150 * wrongChecks;
		return s < 0 ? 0 : s;
	}
};

class act1Level3
{
public:
	static const int PLAYING = 0;
	static const int WON = 1;
	static const int LOST = 2;
	static const int IGNORED = 0;
	static const int CORRECT = 1;
	static const int WRONG = 2;
	static const int TIMEOUT = 3;

private:
	TreeNode* byId[101];    
	bool solved[101];       
	int order[101];         
	int n;
	int next;              
	int alarm;
	int penalties;           
	int timeouts;
	int capacity;
	int state;

	void collect(TreeNode* t)
	{
		if (t == NULL) return;
		if (t->id >= 0 && t->id < 101)
		{
			byId[t->id] = t;
			n++;
		}
		collect(t->left);
		collect(t->right);
	}
	void buildOrder()
	{
		bool placed[101];
		for (int i = 0; i < 101; i++) placed[i] = false;

		for (int step = 0; step < n; step++)
		{
			int ready[101];
			int r = 0;

			for (int id = 0; id < 101; id++)
			{
				TreeNode* t = byId[id];
				if (t == NULL || placed[id]) continue;

				bool leftOk = (t->left == NULL) || placed[t->left->id];
				bool rightOk = (t->right == NULL) || placed[t->right->id];

				if (leftOk && rightOk) ready[r++] = id;
			}

			if (r == 0) { n = step; break; }   

			int pick = ready[rand() % r];
			placed[pick] = true;
			order[step] = pick;
		}
	}

	void updateState()
	{
		if (next == n) state = WON;
		else if (alarm >= capacity) state = LOST;
	}

public:
	act1Level3()
	{
		n = 0; next = 0; alarm = 0; penalties = 0; timeouts = 0; capacity = 0;
		state = LOST;
		for (int i = 0; i < 101; i++)
		{
			byId[i] = NULL;
			solved[i] = false;
			order[i] = -1;
		}
	}
	act1Level3(const act1Level3&) = delete;
	act1Level3& operator=(const act1Level3&) = delete;

	void start(TreeNode* root)
	{
		n = 0; next = 0; alarm = 0; penalties = 0; timeouts = 0;
		for (int i = 0; i < 101; i++)
		{
			byId[i] = NULL;
			solved[i] = false;
			order[i] = -1;
		}

		collect(root);
		buildOrder();

		capacity = (3 * n + 1) / 2 + 2;   
		state = (n > 0) ? PLAYING : LOST;
	}

	int submit(int guess)
	{
		if (state != PLAYING) return IGNORED;

		alarm++;
		int r;

		if (guess == getCurrentHeight())
		{
			solved[order[next]] = true;
			next++;
			r = CORRECT;
		}
		else
		{
			penalties++;
			r = WRONG;
		}

		updateState();
		return r;
	}

	int timeUp()
	{
		if (state != PLAYING) return IGNORED;

		alarm++;
		penalties++;
		timeouts++;

		updateState();
		return TIMEOUT;
	}

	int getN() const { return n; }
	int getSolved() const { return next; }
	int getAlarm() const { return alarm; }
	int getPenalties() const { return penalties; }
	int getTimeouts() const { return timeouts; }
	int getCapacity() const { return capacity; }
	int getState() const { return state; }
	int getCurrentId() const { return (next < n) ? order[next] : -1; }

	int getCurrentHeight() const
	{
		if (next >= n) return 0;
		return findHeight(byId[order[next]]);
	}

	bool isSolved(int id) const { return id >= 0 && id < 101 && solved[id]; }

	int stars() const
	{
		if (state != WON) return 0;
		if (alarm <= n && penalties == 0) return 3;
		if (alarm * 4 <= 5 * n && penalties <= 1) return 2;
		return 1;
	}
	int score() const
	{
		if (state != WON) return 0;
		int s = 1000 - 10 * alarm;
		return s < 0 ? 0 : s;
	}
};


//          level 4

class act1Level4
{
public:
	static const int PLAYING = 0;
	static const int WON = 1;
	static const int LOST = 2;

	static const int IGNORED = 0;
	static const int CORRECT = 1;
	static const int WRONG = 2;

private:
	TreeNode* nodes[101];
	int n;
	int totalRounds;
	int currentRound;
	int nodeAIdx;
	int nodeBIdx;
	int alarm;
	int penalties;
	int capacity;
	int state;

	void collect(TreeNode* t)
	{
		if (t == NULL || n >= 101) return;
		nodes[n++] = t;
		collect(t->left);
		collect(t->right);
	}

	void pickPair()
	{
		if (n < 2)
		{
			nodeAIdx = 0;
			nodeBIdx = 0;
			return;
		}
		nodeAIdx = rand() % n;
		do
		{
			nodeBIdx = rand() % n;
		} while (nodeBIdx == nodeAIdx);
	}

	// Aligns depths first so header.h's findLCA() receives equal-depth nodes
	TreeNode* computeLCA(TreeNode* one, TreeNode* two) const
	{
		int d1 = depth(one);
		int d2 = depth(two);
		while (d1 > d2) { one = one->parent; d1--; }
		while (d2 > d1) { two = two->parent; d2--; }
		return findLCA(one, two);
	}

	int computePathLength(TreeNode* one, TreeNode* two) const
	{
		TreeNode* lca = computeLCA(one, two);
		if (lca == NULL) return 0;
		return depth(one) + depth(two) - 2 * depth(lca);
	}

public:
	act1Level4()
	{
		n = 0; totalRounds = 3; currentRound = 0;
		nodeAIdx = -1; nodeBIdx = -1;
		alarm = 0; penalties = 0; capacity = 0;
		state = LOST;
		for (int i = 0; i < 101; i++) nodes[i] = NULL;
	}
	act1Level4(const act1Level4&) = delete;
	act1Level4& operator=(const act1Level4&) = delete;

	void start(TreeNode* root, int rounds = 3)
	{
		n = 0;
		for (int i = 0; i < 101; i++) nodes[i] = NULL;
		collect(root);

		totalRounds = rounds;
		currentRound = 0;
		alarm = 0;
		penalties = 0;
		capacity = (3 * totalRounds + 1) / 2 + 2;

		if (n >= 2)
		{
			state = PLAYING;
			pickPair();
		}
		else
		{
			state = LOST;
		}
	}

	int submit(int selectedLcaId, int guessedPathLen)
	{
		if (state != PLAYING) return IGNORED;

		alarm++;
		TreeNode* a = nodes[nodeAIdx];
		TreeNode* b = nodes[nodeBIdx];
		TreeNode* actualLca = computeLCA(a, b);
		int actualLen = computePathLength(a, b);

		int r;
		if (actualLca != NULL && selectedLcaId == actualLca->id && guessedPathLen == actualLen)
		{
			currentRound++;
			r = CORRECT;
			if (currentRound >= totalRounds)
				state = WON;
			else
				pickPair();
		}
		else
		{
			penalties++;
			r = WRONG;
			if (alarm >= capacity)
				state = LOST;
		}

		return r;
	}

	int getN() const { return n; }
	int getTotalRounds() const { return totalRounds; }
	int getCurrentRound() const { return currentRound; }
	int getAlarm() const { return alarm; }
	int getPenalties() const { return penalties; }
	int getCapacity() const { return capacity; }
	int getState() const { return state; }

	TreeNode* getNodeA() const { return (nodeAIdx >= 0 && nodeAIdx < n) ? nodes[nodeAIdx] : NULL; }
	TreeNode* getNodeB() const { return (nodeBIdx >= 0 && nodeBIdx < n) ? nodes[nodeBIdx] : NULL; }

	int stars() const
	{
		if (state != WON) return 0;
		if (alarm <= totalRounds && penalties == 0) return 3;
		if (alarm * 4 <= 5 * totalRounds && penalties <= 1) return 2;
		return 1;
	}
	int score() const
	{
		if (state != WON) return 0;
		int s = 1000 - 10 * alarm;
		return s < 0 ? 0 : s;
	}
};


//          level 5

class act1Level5
{
public:
	static const int PLAYING = 0;
	static const int WON = 1;
	static const int LOST = 2;

private:
	TreeNode* original;
	TreeNode* working;
	TreeNode* target;
	int n;
	int swaps;
	int alarm;
	int penalties;
	int capacity;
	int state;

	TreeNode* copyTree(TreeNode* root, TreeNode* parent = NULL)
	{
		if (root == NULL) return NULL;
		TreeNode* copy = new TreeNode;
		copy->id = root->id;
		copy->value = root->value;
		copy->x = root->x;
		copy->y = root->y;
		copy->parent = parent;
		copy->left = copyTree(root->left, copy);
		copy->right = copyTree(root->right, copy);
		return copy;
	}

	bool treesMatch(TreeNode* a, TreeNode* b) const
	{
		if (a == NULL && b == NULL) return true;
		if (a == NULL || b == NULL) return false;
		if (a->value != b->value) return false;
		return treesMatch(a->left, b->left) && treesMatch(a->right, b->right);
	}

	int countNodes(TreeNode* t) const
	{
		if (t == NULL) return 0;
		return 1 + countNodes(t->left) + countNodes(t->right);
	}

	TreeNode* findById(TreeNode* t, int id)
	{
		if (t == NULL) return NULL;
		if (t->id == id) return t;
		TreeNode* l = findById(t->left, id);
		if (l != NULL) return l;
		return findById(t->right, id);
	}

public:
	act1Level5()
	{
		original = NULL; working = NULL; target = NULL;
		n = 0; swaps = 0; alarm = 0; penalties = 0; capacity = 0;
		state = LOST;
	}
	act1Level5(const act1Level5&) = delete;
	act1Level5& operator=(const act1Level5&) = delete;

	~act1Level5()
	{
		deleteTree(original);
		deleteTree(working);
		deleteTree(target);
	}

	void start(TreeNode* srcRoot)
	{
		deleteTree(original);
		deleteTree(working);
		deleteTree(target);

		original = copyTree(srcRoot);
		working = copyTree(srcRoot);
		target = mirror(copyTree(srcRoot)); // uses your mirror() from header.h

		n = countNodes(original);
		swaps = 0;
		alarm = 0;
		penalties = 0;
		capacity = (3 * n + 1) / 2 + 2;
		state = (n > 0) ? PLAYING : LOST;
	}

	bool swapChildren(int nodeId)
	{
		if (state != PLAYING) return false;
		TreeNode* node = findById(working, nodeId);
		if (node == NULL) return false;
		if (node->left == NULL && node->right == NULL) return false;

		TreeNode* temp = node->left;
		node->left = node->right;
		node->right = temp;
		swaps++;
		return true;
	}

	void resetWorking()
	{
		if (state != PLAYING) return;
		deleteTree(working);
		working = copyTree(original);
		swaps = 0;
	}

	bool submit()
	{
		if (state != PLAYING) return false;
		alarm++;

		if (treesMatch(working, target))
		{
			state = WON;
			return true;
		}
		else
		{
			penalties++;
			if (alarm >= capacity)
				state = LOST;
			return false;
		}
	}

	TreeNode* getWorkingRoot() const { return working; }
	int getN() const { return n; }
	int getSwaps() const { return swaps; }
	int getAlarm() const { return alarm; }
	int getPenalties() const { return penalties; }
	int getCapacity() const { return capacity; }
	int getState() const { return state; }

	int stars() const
	{
		if (state != WON) return 0;
		if (penalties == 0) return 3;
		if (penalties == 1) return 2;
		return 1;
	}
	int score() const
	{
		if (state != WON) return 0;
		int s = 1000 - 10 * alarm - 50 * penalties;
		return s < 0 ? 0 : s;
	}
};