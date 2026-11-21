#pragma once
#include <cstdlib>
#include "header.h"
using namespace std;

inline bool getNextstep(const int arr[], int s, int hint[])
{
	for (int i = s / 2 - 1; i >= 0; i--)
	{
		int larger = largerChildIndex(i, s, arr);
		if (larger != -1)
		{
			hint[0] = i;
			hint[1] = larger;
			return true;
		}
	}
	return false;
}

inline void generateRandomArray(int* arr, int& s, int minS, int maxS)
{
	s = minS + rand() % (maxS - minS + 1);

	bool used[100] = { false };
	for (int i = 0; i < s; i++)
	{
		int v;
		do
		{
			v = 1 + rand() % 99;
		} while (used[v]);

		used[v] = true;
		arr[i] = v;
	}
}
inline void makeRandomHeap(int* arr, int& n, int minN, int maxN)
{
	int heap[15];
	generateRandomArray(heap, n, minN, maxN);

	maxHeap h(n);
	for (int i = 0; i < n; i++) h.insert(heap[i]);
	h.copyTo(arr);
}

inline bool sameArray(const int* a, const int* b, int n)
{
	for (int i = 0; i < n; i++)
		if (a[i] != b[i]) return false;
	return true;
}
inline int act4Capacity(int optimal)
{
	return (3 * optimal + 1) / 2 + 2;
}

inline int act4Stars(int alarm, int optimal, int penalties)
{
	if (alarm <= optimal && penalties == 0) return 3;
	if (alarm * 4 <= 5 * optimal && penalties <= 1) return 2;
	return 1;
}

inline int act4Score(int alarm, int penalties)
{
	int s = 1000 - 10 * alarm - 50 * penalties;
	return s < 0 ? 0 : s;
}

inline int buildHeapSwaps(int arr[], int s)
{
	int total = 0;
	for (int i = s / 2 - 1; i >= 0; i--)
		total += shiftDownSwaps(i, s, arr);
	return total;
}

inline int heapSortSteps(int heap[], int s)
{
	int steps = 0;
	for (int end = s - 1; end >= 1; end--)
	{
		int t = heap[0]; heap[0] = heap[end]; heap[end] = t;
		steps++;
		steps += shiftDownSwaps(0, end, heap);
	}
	return steps;
}



class act4Level1
{
public:
	static const int MAXN = 15;
	static const int PLAYING = 0;
	static const int WON = 1;
	static const int LOST = 2;

	static const int LEFT = 0;
	static const int RIGHT = 1;
	static const int HINT_UNLINK = 2;   

	static const int CAPACITY = 5;     
	static const int CHECK_IGNORED = 0;
	static const int CHECK_NOT_CONNECTED = 1;
	static const int CHECK_NOT_COMPLETE = 2;
	static const int CHECK_NOT_HEAP = 3;
	static const int CHECK_OK = 4;

private:
	int n;
	int val[MAXN];
	int child[MAXN][2];
	int par[MAXN];
	int checkedArr[MAXN];
	int checkedCount;
	int alarm;
	int state;

	void resetLinks()
	{
		for (int i = 0; i < MAXN; i++)
		{
			child[i][0] = child[i][1] = -1;
			par[i] = -1;
		}
	}

public:
	act4Level1()
	{
		n = 0; alarm = 0; state = LOST; checkedCount = 0;
		for (int i = 0; i < MAXN; i++) { val[i] = 0; checkedArr[i] = 0; }
		resetLinks();
	}

	void start()
	{
		generateRandomArray(val, n, 7, 10);
		resetLinks();
		checkedCount = 0;
		alarm = 0;
		state = PLAYING;
	}

	void clearLinks()
	{
		if (state != PLAYING) return;
		resetLinks();
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


	int evaluate(int* outArr, int& outCount) const
	{
		outCount = 0;
		if (linkCount() != n - 1) return CHECK_NOT_CONNECTED;

		TreeNode* t[MAXN];
		for (int i = 0; i < n; i++)
		{
			t[i] = new TreeNode;
			t[i]->id = i;
			t[i]->value = val[i];
		}

		TreeNode* root = NULL;
		for (int i = 0; i < n; i++)
		{
			if (child[i][0] != -1)
			{
				t[i]->left = t[child[i][0]];
				t[child[i][0]]->parent = t[i];
			}
			if (child[i][1] != -1)
			{
				t[i]->right = t[child[i][1]];
				t[child[i][1]]->parent = t[i];
			}
			if (par[i] == -1) root = t[i];
		}

		int result;

		if (!isCompleteTree(root))
		{
			result = CHECK_NOT_COMPLETE;
		}
		else
		{
			List order;
			levelOrder(root, &order);

			TreeNode* c = order.getHead();
			while (c != NULL && outCount < MAXN)
			{
				outArr[outCount++] = c->value;
				c = c->right;
			}

			result = isMaxHeap(outArr, outCount) ? CHECK_OK : CHECK_NOT_HEAP;
		}

		for (int i = 0; i < n; i++) delete t[i];
		return result;
	}

	int check()
	{
		if (state != PLAYING) return CHECK_IGNORED;

		int arr[MAXN], cnt;
		int result = evaluate(arr, cnt);
		if (result == CHECK_NOT_CONNECTED) return result;      

		checkedCount = cnt;
		for (int i = 0; i < cnt; i++) checkedArr[i] = arr[i];

		if (result == CHECK_OK)
		{
			state = WON;
		}
		else
		{
			alarm++;
			if (alarm >= CAPACITY) state = LOST;
		}
		return result;
	}

	bool getHint(int& p, int& side, int& c) const
	{
		if (state != PLAYING) return false;

		int tmp[MAXN], cnt;
		if (evaluate(tmp, cnt) == CHECK_OK) return false;

		int idx[MAXN];
		for (int i = 0; i < n; i++) idx[i] = i;
		for (int i = 0; i < n; i++)                      
		{
			int best = i;
			for (int j = i + 1; j < n; j++)
				if (val[idx[j]] > val[idx[best]]) best = j;
			int t = idx[i]; idx[i] = idx[best]; idx[best] = t;
		}

		int pos[MAXN];                                    
		for (int i = 0; i < n; i++) pos[idx[i]] = i;

		for (int i = 0; i < n; i++)
		{
			for (int s = 0; s < 2; s++)
			{
				int k = 2 * i + 1 + s;
				if (k >= n) continue;

				if (child[idx[i]][s] == idx[k]) continue;

				p = idx[i];
				side = s;
				c = idx[k];

				for (int a = p; a != -1; a = par[a])
				{
					if (a != c) continue;

					int x = p;
					while (x != c)
					{
						int pa = par[x];
						int xs = (child[pa][0] == x) ? 0 : 1;
						bool inTarget = pos[x] > 0 &&
							idx[(pos[x] - 1) / 2] == pa && (pos[x] - 1) % 2 == xs;
						if (!inTarget)
						{
							p = x;
							side = HINT_UNLINK;
							c = -1;
							return true;
						}
						x = pa;
					}
				}
				return true;
			}
		}
		return false;
	}

	int getN() const { return n; }
	int getState() const { return state; }
	int getVal(int i) const { return val[i]; }
	int getChild(int i, int side) const { return child[i][side]; }
	int getParent(int i) const { return par[i]; }
	int getAlarm() const { return alarm; }
	int getCapacity() const { return CAPACITY; }
	int getCheckedCount() const { return checkedCount; }
	int getChecked(int i) const { return checkedArr[i]; }

	int stars() const
	{
		if (state != WON) return 0;
		if (alarm == 0) return 3;
		if (alarm == 1) return 2;
		return 1;
	}
	int score() const
	{
		if (state != WON) return 0;
		int s = 1000 - 150 * alarm;
		return s < 0 ? 0 : s;
	}
};

//-----------------level 2---------------------------------------

class act4Level2
{

public:
	static const int PLAYING = 0;
	static const int WON = 1;
	static const int LOST = 2;

private:
	int arr[15];
	int initialArr[15];
	int n;
	int capacity;
	int state; 
	int alarm;
	int optimalSwaps;
	int penalities;


public:

	act4Level2()
	{
		reset();
	}

	void reset()
	{
		n = 0;
		alarm = 0;
		state = LOST;
		optimalSwaps = 0;
		penalities = 0;
	}
	void start()
	{
		alarm = 0;
		penalities = 0;
		capacity = act4Capacity(optimalSwaps); 
		state = PLAYING;
		do {
			generateRandomArray(arr, n, 7, 15);
		} while (isMaxHeap(arr, n));

		int temp[15];
		for (int i = 0; i < n; i++)
		{
			temp[i] = arr[i];
			initialArr[i] = arr[i];
		}

		optimalSwaps = buildHeapSwaps(temp, n);
	}

	void swapNodes(int i, int j)
	{

		if (state != PLAYING) return;
		if (i < 0 || i >= n || j < 0 || j >= n || i == j) return;

		int temp = arr[i];
		arr[i] = arr[j];
		arr[j] = temp;
		alarm++;

		if (isMaxHeap(arr, n))
			state = WON;
		else if (alarm >= capacity)
			state = LOST;

		

	}

	bool getHint(int& a, int& b) const
	{
		if (state != PLAYING) return false;
		int hint_[2];
		if (!getNextstep(arr, n, hint_)) return false;
		a = hint_[0];
		b = hint_[1];
		return true;
	}

	void resetPuzzle()
	{
		if (state != PLAYING) return;
		for (int i = 0; i < n; i++)
			arr[i] = initialArr[i];
	}


	int getN() const { return n; }
	int getVal(int i) const { return arr[i]; }
	int getState() const { return state; }
	int getAlarm() const { return alarm; }
	int getCapacity() const { return capacity; }
	int stars() const { return (state == WON) ? act4Stars(alarm, optimalSwaps, penalities) : 0; }
	int score() const { return (state == WON) ? act4Score(alarm, penalities) : 0; }

};
//---------------------------------level 3-------------------------------------------------
class act4Level3
{
public:
	static const int PLAYING = 0;
	static const int WON = 1;
	static const int LOST = 2;

private:
	int state;
	int penalities;
	int arr[15];
	int initialarr[15];
	int n;
	int initialN;
	int curIdx;
	bool rootRemoved;
	int optimaltakes;
	int alarm;
	int cap;

public:
	act4Level3()
	{
		reset();
	}

	void reset()
	{
		state = LOST;
		penalities = 0;
		n = 0;
		initialN = 0;
		curIdx = -1;
		rootRemoved = false;
		alarm = 0;
		optimaltakes = 0;
		cap = 0;
	}

	void start()
	{
		reset();
		makeRandomHeap(arr, n, 7, 12);
		initialN = n;

		for (int i = 0; i < n; i++)
		{
			initialarr[i] = arr[i];
		}
		int copy[15];
		for (int i = 0; i < n; i++) copy[i] = arr[i];
		copy[0] = copy[n - 1];
		optimaltakes = 1 + shiftDownSwaps(0, n - 1, copy);

		cap = act4Capacity(optimaltakes);
		rootRemoved = false;
		curIdx = -1;
		state = PLAYING;
	}

	void resetPuzzle()
	{
		if (state != PLAYING) return;
		n = initialN;
		for (int i = 0; i < n; i++)
		{
			arr[i] = initialarr[i];
		}
		rootRemoved = false;
		curIdx = -1;
	}

	bool removeRoot()
	{
		if (state != PLAYING || rootRemoved) return false;

		arr[0] = arr[n - 1];
		n--;
		rootRemoved = true;
		curIdx = 0;
		alarm++;

		if (largerChildIndex(0, n, arr) == -1)
			state = WON;
		else if (alarm >= cap)
			state = LOST;

		return true;
	}

	bool swapNodes(int i, int j)
	{
		if (state != PLAYING) return false;
		if (i < 0 || i >= n || j < 0 || j >= n || i == j) return false;

		if (!rootRemoved)
		{
			alarm++;
			penalities++;
			if (alarm >= cap) state = LOST;
			return false;
		}

		int target = largerChildIndex(curIdx, n, arr);

		if ((i == curIdx && j == target) || (j == curIdx && i == target))
		{
			int temp = arr[i];
			arr[i] = arr[j];
			arr[j] = temp;
			curIdx = target;
			alarm++;

			if (largerChildIndex(curIdx, n, arr) == -1)
				state = WON;
			else if (alarm >= cap)
				state = LOST;

			return true;
		}
		alarm++;
		penalities++;
		if (alarm >= cap) state = LOST;
		return false;
	}

	bool getHint(int& a, int& b) const
	{
		if (state != PLAYING) return false;

		if (!rootRemoved)
		{
			a = 0;
			b = n - 1;
			return true;
		}

		int target = largerChildIndex(curIdx, n, arr);
		if (target == -1) return false;

		a = curIdx;
		b = target;
		return true;
	}

	int getN() const { return n; }
	int getVal(int i) const { return arr[i]; }
	int getCurIdx() const { return curIdx; }
	bool isRootRemoved() const { return rootRemoved; }
	int getState() const { return state; }
	int getAlarm() const { return alarm; }
	int getCapacity() const { return cap; }
	int stars() const { return (state == WON) ? act4Stars(alarm, optimaltakes, penalities) : 0; }
	int score() const { return (state == WON) ? act4Score(alarm, penalities) : 0; }
};


class act4Level4
{
public:
	static const int PLAYING = 0;
	static const int WON = 1;
	static const int LOST = 2;

private:
	int state;
	int penalities;
	int arr[15];
	int initialarr[15];
	int n;
	int heapEnd;
	int curIdx;
	bool needExtract;
	int optimaltakes;
	int alarm;
	int cap;

public:
	act4Level4()
	{
		reset();
	}

	void reset()
	{
		state = LOST;
		penalities = 0;
		n = 0;
		heapEnd = 0;
		curIdx = -1;
		needExtract = true;
		alarm = 0;
		optimaltakes = 0;
		cap = 0;
	}

	void start()
	{
		reset();
		makeRandomHeap(arr, n, 6, 9);

		for (int i = 0; i < n; i++)
		{
			initialarr[i] = arr[i];
		}

		int copy[15];
		for (int i = 0; i < n; i++) copy[i] = arr[i];
		optimaltakes = heapSortSteps(copy, n);

		cap = act4Capacity(optimaltakes);
		heapEnd = n;
		curIdx = -1;
		needExtract = true;
		state = PLAYING;
	}

	void resetPuzzle()
	{
		if (state != PLAYING) return;
		for (int i = 0; i < n; i++)
		{
			arr[i] = initialarr[i];
		}
		heapEnd = n;
		curIdx = -1;
		needExtract = true;
	}

	bool swapNodes(int i, int j)
	{
		if (state != PLAYING) return false;
		if (i < 0 || i >= n || j < 0 || j >= n || i == j) return false;

		if (needExtract)
		{
			int last = heapEnd - 1;
			if ((i == 0 && j == last) || (j == 0 && i == last))
			{
				int temp = arr[0];
				arr[0] = arr[last];
				arr[last] = temp;
				heapEnd--;
				alarm++;

				if (heapEnd <= 1)
				{
					state = WON;
				}
				else if (largerChildIndex(0, heapEnd, arr) == -1)
				{
					needExtract = true;
					curIdx = -1;
				}
				else
				{
					needExtract = false;
					curIdx = 0;
				}

				if (state == PLAYING && alarm >= cap)
					state = LOST;

				return true;
			}
			alarm++;
			penalities++;
			if (alarm >= cap) state = LOST;
			return false;
		}
		else
		{
			int target = largerChildIndex(curIdx, heapEnd, arr);
			if ((i == curIdx && j == target) || (j == curIdx && i == target))
			{
				int temp = arr[i];
				arr[i] = arr[j];
				arr[j] = temp;
				curIdx = target;
				alarm++;

				if (largerChildIndex(curIdx, heapEnd, arr) == -1)
				{
					needExtract = true;
					curIdx = -1;
					if (heapEnd <= 1) state = WON;
				}

				if (state == PLAYING && alarm >= cap)
					state = LOST;

				return true;
			}
			alarm++;
			penalities++;
			if (alarm >= cap) state = LOST;
			return false;
		}
	}

	bool getHint(int& a, int& b) const
	{
		if (state != PLAYING) return false;

		if (needExtract)
		{
			a = 0;
			b = heapEnd - 1;
			return true;
		}

		int target = largerChildIndex(curIdx, heapEnd, arr);
		if (target == -1) return false;

		a = curIdx;
		b = target;
		return true;
	}

	int getN() const { return n; }
	int getVal(int i) const { return arr[i]; }
	int getHeapEnd() const { return heapEnd; }
	int getCurIdx() const { return curIdx; }
	bool isNeedExtract() const { return needExtract; }
	int getState() const { return state; }
	int getAlarm() const { return alarm; }
	int getCapacity() const { return cap; }
	int stars() const { return (state == WON) ? act4Stars(alarm, optimaltakes, penalities) : 0; }
	int score() const { return (state == WON) ? act4Score(alarm, penalities) : 0; }
};
//-----------------level 5------------------

inline bool isSortedAscending(const int arr[], int s)
{
	for (int i = 0; i < s - 1; i++)
	{
		if (arr[i] > arr[i + 1]) return false;
	}
	return true;
}

class act4Level5
{
public:
	static const int PLAYING = 0;
	static const int WON = 1;
	static const int LOST = 2;

private:
	int state;
	int penalities;
	int arr[15];
	int initialarr[15];
	int n;
	bool heapBuilt;
	int optimaltakes;
	int alarm;
	int cap;

public:
	act4Level5()
	{
		reset();
	}

	void reset()
	{
		state = LOST;
		penalities = 0;
		n = 0;
		heapBuilt = false;
		alarm = 0;
		optimaltakes = 0;
		cap = 0;
	}

	void start()
	{
		reset();
		do {
			generateRandomArray(arr, n, 6, 9);
		} while (isMaxHeap(arr, n) || isSortedAscending(arr, n));

		for (int i = 0; i < n; i++)
		{
			initialarr[i] = arr[i];
		}

		// Optimal steps = Build Max Heap swaps + Heap Sort steps
		int copy[15];
		for (int i = 0; i < n; i++) copy[i] = arr[i];
		int buildSteps = buildHeapSwaps(copy, n);
		int sortSteps = heapSortSteps(copy, n);
		optimaltakes = buildSteps + sortSteps;

		cap = act4Capacity(optimaltakes);
		heapBuilt = false;
		state = PLAYING;
	}

	void resetPuzzle()
	{
		if (state != PLAYING) return;
		for (int i = 0; i < n; i++)
		{
			arr[i] = initialarr[i];
		}
		heapBuilt = false;
	}

	void swapNodes(int i, int j)
	{
		if (state != PLAYING) return;
		if (i < 0 || i >= n || j < 0 || j >= n || i == j) return;

		int temp = arr[i];
		arr[i] = arr[j];
		arr[j] = temp;
		alarm++;

		if (!heapBuilt && isMaxHeap(arr, n))
		{
			heapBuilt = true;
		}

		if (isSortedAscending(arr, n))
		{
			state = WON;
		}
		else if (alarm >= cap)
		{
			state = LOST;
		}
	}

	bool checkVault()
	{
		if (state != PLAYING) return false;

		if (isSortedAscending(arr, n))
		{
			state = WON;
			return true;
		}

		alarm++;
		penalities++;
		if (alarm >= cap) state = LOST;
		return false;
	}

	int getN() const { return n; }
	int getVal(int i) const { return arr[i]; }
	bool isHeapBuilt() const { return heapBuilt; }
	int getState() const { return state; }
	int getAlarm() const { return alarm; }
	int getCapacity() const { return cap; }
	int stars() const { return (state == WON) ? act4Stars(alarm, optimaltakes, penalities) : 0; }
	int score() const { return (state == WON) ? act4Score(alarm, penalities) : 0; }
};