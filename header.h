#pragma once
#include <ctime>
#include <cstdlib>
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
	void setPositions(TreeNode* root, int depth, int& position)
	{
		if (root == NULL)
			return;

		setPositions(root->left, depth + 1, position);

		root->x = 200 + position * 150;
		root->y = 250 + depth * 130;

		position++;

		setPositions(root->right, depth + 1, position);
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
//------------------------level 1--------------------------
inline void preOrder(TreeNode* root, List* h)
{
	if (root == NULL) return;
	h->push(root);
	preOrder(root->left, h);
	preOrder(root->right, h);

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
	TreeNode* q[101]; 
	int f = 0, b = 0;
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
//----------------------level 2-----------------------------------
inline int findIndex(int inorder[], int left, int right, int val)
{
	for (int i = left; i <= right; i++)
	{
		if (inorder[i] == val)
			return i;
	}
	return -1;
}

inline TreeNode* helperPre(int pre[], int in[], int& preIdx, int left, int right)
{
	if (left > right)
		return NULL;

	TreeNode* root = new TreeNode(pre[preIdx]);
	int indx = findIndex(in, left, right, pre[preIdx]);
	preIdx++;

	root->left = helperPre(pre, in, preIdx, left, indx - 1);
	root->right = helperPre(pre, in, preIdx, indx + 1, right);
	return root;
}

inline TreeNode* buildPre(int pre[], int in[], int s)
{
	int preIdx = 0;
	return helperPre(pre, in, preIdx, 0, s - 1);
}

inline TreeNode* helperPost(int post[], int in[], int& postIdx, int left, int right)
{
	if (left > right)
		return NULL;

	TreeNode* root = new TreeNode(post[postIdx]);
	int indx = findIndex(in, left, right, post[postIdx]);
	postIdx--;

	root->right = helperPost(post, in, postIdx, indx + 1, right);
	root->left = helperPost(post, in, postIdx, left, indx - 1);
	return root;
}

inline TreeNode* buildPost(int post[], int in[], int s)
{
	int postIdx = s - 1;
	return helperPost(post, in, postIdx, 0, s - 1);
}

//-------------------------level3---------------------------

inline int findHeight(TreeNode* root)
{
	if (root == NULL)return 0;
	int l = 1 + findHeight(root->left);
	int r = 1 + findHeight(root->right);

	return (r > l) ? r : l;
}

inline int findDiameter(TreeNode* root)
{
	if (root == NULL)return 0;

	int l = findHeight(root->left);
	int r = findHeight(root->right);

	int tot = l + r;

	int leftD = findDiameter(root->left);
	int rightD = findDiameter(root->right);

	return max(tot, max(leftD, rightD));
}

// ---------------------level 4--------------------------------------------

inline int depth(TreeNode* root)
{
	int d = 0;
	while (root != NULL)
	{
		root = root->parent;
		d++;
	}
	return d;
}

inline TreeNode* findLCA(TreeNode*one, TreeNode*two)
{
	int d1 = depth(one);
	int d2 = depth(two);

	while (d1 > d2)
	{
		one = one->parent;
	}
	while (d2 > d1)
	{
		two = two->parent;
	}

	while (one != two)
	{
		one = one->parent;
		two = two->parent;
	}
	return one;
}


// -----------------------level 5---------------------------------

inline TreeNode* mirror(TreeNode* root)
{

	if (root == NULL) return NULL;
	root ->left = mirror(root->left);
	root->right = mirror(root->right);
	TreeNode* temp = root->left;
	root->left = root->right;
	root->right = temp;

	return root;
}

