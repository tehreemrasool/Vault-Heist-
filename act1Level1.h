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