#pragma once
#include <cstdlib>
#include "header.h"
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
	// traversal types
	static const int PREORDER = 0;
	static const int INORDER = 1;
	static const int POSTORDER = 2;
	static const int LEVELORDER = 3;

	// level states
	static const int PLAYING = 0;
	static const int WON = 1;
	static const int LOST = 2;

	// click results
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

	// modes
	static const int INPRE = 0;    // inorder + preorder
	static const int INPOST = 1;   // inorder + postorder

	// states
	static const int PLAYING = 0;
	static const int WON = 1;

	// child sides
	static const int LEFT = 0;
	static const int RIGHT = 1;

private:
	int n;
	int mode;
	int inSeq[MAXN];
	int otherSeq[MAXN];
	int val[MAXN];          // value of piece i (shuffled order)
	int child[MAXN][2];     // piece index of left/right child, -1 = none
	int par[MAXN];          // piece index of parent, -1 = none
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

		// pieces in shuffled order
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

	// cut piece c from its parent
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

	// make c the left/right child of p. returns false if not allowed
	bool connect(int p, int side, int c)
	{
		if (state != PLAYING) return false;
		if (p < 0 || p >= n || c < 0 || c >= n || p == c) return false;
		if (side != LEFT && side != RIGHT) return false;

		// c must not be an ancestor of p (would make a cycle)
		for (int a = p; a != -1; a = par[a])
			if (a == c) return false;

		detach(c);                         // c leaves its old parent
		if (child[p][side] != -1)          // slot taken: old child is cut loose
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