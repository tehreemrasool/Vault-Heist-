#pragma once
#include <ctime>
#include <cstdlib>
#include <iostream>

struct TreeNode
{
	int id;
	TreeNode* left;
	TreeNode* right;
	TreeNode* parent;
	int value;
	TreeNode()
	{
		id = 0;
		value = 0;
		left = NULL;
		right = NULL;
		parent = NULL;
	}
};

class List
{
	TreeNode* head = NULL;
	TreeNode* tail = NULL;
	TreeNode* curr = NULL;
	int count = 0;


public:
	void clear()
	{
		while (head != NULL)
		{
			TreeNode* next = head->right;
			delete head;
			head = next;
		}
		head = tail = curr = NULL;
		count = 0;
	}
	
	~List()
	{
		clear();
		
	}

	TreeNode* getHead()
	{
		return head;
	}

	void moveCurr()
	{
		if (curr != NULL) curr = curr->right;
	}
	void setCurr(TreeNode* n)
	{
		curr = n;
	}
	int getCurrId()
	{
		return curr ? curr->id : -1; 
	}
	int getCount()
	{
		return count;
	}
	void push(TreeNode* n) {
		TreeNode* x = new TreeNode;
		x->value = n->value;
		x->id = n->id;
		x->parent = n->parent;

		if (head == NULL) head = tail = curr = x;
		else { tail->right = x; tail = x; }
		count++;
	}
	
};

inline void preOrder(TreeNode* root, List* h)
{
	if (root == NULL) return;
	h->push(root);
	preOrder(root->left, h);
	preOrder(root->right, h);

}

inline void addLeft(TreeNode* n, int d, int id)
{
	n->left = new TreeNode;
	n->left->value = d;
	n->left->id = id;
	n->left->parent = n;
}
inline void addright(TreeNode* n, int d,int id)
{
	n->right = new TreeNode;
	n->right->value = d;
	n->right->id = id;
	n->right->parent = n;
}

inline void postOrder(TreeNode* root, List* h)
{
	if (root == NULL) return;
	postOrder(root->left, h);
	postOrder(root->right, h);
	h->push(root);

}

inline void inOrder(TreeNode* root, List* h)
{
	if (root == NULL) return;
	inOrder(root->left, h);
	h->push(root);
	inOrder(root->right, h);
}

inline void levelOrder(TreeNode* root, List* h)
{
	if (root == NULL) return;
	TreeNode* q[101]; int f = 0, b = 0;
	q[b++] = root;
	while (f < b)
	{
		TreeNode* c = q[f++];
		h->push(c);
		if (c->left)  q[b++] = c->left;
		if (c->right) q[b++] = c->right;
	}
}


inline void insertRandom(TreeNode* parent, int num, int index)
{
	int dir = rand() % 2;

	if (dir == 0)
	{
		if (parent->left == NULL)
			addLeft(parent, num, index);
		else
			insertRandom(parent->left, num, index);
	}
	else
	{
		if (parent->right == NULL)
			addright(parent, num, index);
		else
			insertRandom(parent->right, num, index);
	}
}


inline TreeNode* constructBinaryTree(int numberOfRooms)
{
	if (numberOfRooms < 1 || numberOfRooms > 101)
		return NULL;

	bool used[101] = { false };
	TreeNode* root = NULL;

	for (int i = 0; i < numberOfRooms; i++)
	{
		int num;
		do
		{
			num = rand() % 101;
		} while (used[num]);

		used[num] = true;

		if (root == NULL)
		{
			root = new TreeNode;
			root->value = num;
			root->id = i;
		}
		else
			insertRandom(root, num, i);
	}
	return root;
}
inline void deleteTree(TreeNode* root)
{
	if (root == NULL) return;
	deleteTree(root->left);
	deleteTree(root->right);
	delete root;
}
