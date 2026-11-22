# **Vault Heist**

A 2D puzzle game built in C++ with SFML that teaches data structures through 20 levels across four acts. You infiltrate an ancient vault whose rooms form a tree. Every wrong move raises the **Alarm Meter**, and if it fills up the vault locks down.

| Act | Theme | Data structure / algorithms |
| ----- | ----- | ----- |
| 1 | The Blueprint | Binary trees: traversals (pre/in/post/level order), tree reconstruction, height, LCA and path length, mirroring |
| 2 | The Vault Rooms | Binary Search Tree: search, insert, delete (3 cases), range query with pruning, degenerate-tree boss |
| 3 | The Self-Healing Vault | AVL tree: balance factors, LL/RR/LR/RL rotations, insert prediction, order statistics (k-th, rank) |
| 4 | The Hierarchy | Max-heap: build, heapify, extract-max, heap sort |

There is also a **Sandbox** mode (Binary tree / BST / AVL / Max Heap) and a **Leaderboard** that uses the project's own heap sort.

---

## **1\. Requirements**

* C++14 or newer (tested design target: C++17)  
* SFML 2.5.x (Graphics, Window, System, Audio)  
* Windows (Visual Studio 2019/2022) or Linux/macOS with g++/clang++

## **2\. Project layout**

```
header.h              Tree nodes, List, traversals, builders, LCA, mirror, maxHeap, heap helpers
act1.h                Logic classes for Act 1 levels 1-5 (no graphics)
Act2.h / Act2.cpp     BST class (insert/delete/search/range/layout/serialization)
Act3.h / Act3.cpp     AVLTree class (extends BST: rotations, rank/kth, case generators)
act4.h                Logic classes for Act 4 levels 1-5 (heap puzzles)
Act2LevelN.*, Act3LevelN.*, act4Screens.cpp, screen.cpp
                      SFML screens (input, drawing) for each level
helpers.h             Drawing helpers (text, tree lines, rounded rects, letterbox view)
mainScreen.cpp        Main menu
sideScreens.cpp       Briefing (black) screen, map, instructions
sandbox.cpp           Sandbox mode
leaderBoard.cpp       Result popup + leaderboard (leaderboard.txt)
main.cpp              Window, music, screen manager and transitions
test_main.cpp         Unit test suite (no SFML required)
Makefile              Builds the test suite
assets/               fonts/, pictures/, themeSong.mp3
```

Design note: game rules (logic classes such as `act1Level1`, `BST`, `AVLTree`, `act4Level2`) are kept separate from the SFML screens. That is why the logic can be unit tested without opening a window.

## **3\. Assets**

The game loads these relative to the working directory, so **run it from the folder that contains `assets/`**:

```
assets/fonts/ARIAL.ttf
assets/fonts/SuperShiny.ttf
assets/fonts/MedievalSharp.ttf
assets/themeSong.mp3
assets/pictures/*.jpeg / *.jfif / *.png   (backgrounds for every level, menu, map, acts)
```

Missing pictures simply leave the background blank. A missing theme song prints `Music not loaded!`.

## **4\. Build and run the game**

### **Visual Studio**

1. Create an empty C++ project and add all `.cpp` and `.h` files (except `test_main.cpp`).  
2. Link SFML: `sfml-graphics`, `sfml-window`, `sfml-system`, `sfml-audio` (use the `-d` variants in Debug).  
3. Copy the SFML DLLs next to the executable, and set the debugging working directory to the folder that contains `assets/`.  
4. Build and run.

## **5\. Game controls**

### **Global (every level)**

| Key | Action |
| ----- | ----- |
| Esc | Pause and show the level briefing (click to resume) |
| H | Hint (or the on-screen HINT button). Using a hint caps stars at 2 in most levels |
| Space  | Reset / retry the current puzzle |
| R | Generate a new puzzle |
| N | Skip to the next level (debug / testing shortcut) |
| Enter | Submit / continue after winning |
| Mouse |  Select rooms and buttons |

### **Act 1: Binary Trees**

| Level | Goal | Controls |
| ----- | ----- | ----- |
| 1 | Click rooms in the order the guard visits them | Click rooms. `1` preorder, `2` inorder, `3` postorder, `4` level order |
| 2 | Rebuild a tree from inorder \+ preorder/postorder | Click parent, then Left-click (left child) or Right-click (right child). Click the same room twice to unlink. `1`/`2` switch mode. Enter checks |
| 3 | Type the height of the highlighted room (5 s per room) | Digits, Backspace, Enter |
| 4 | Click the Lowest Common Ancestor and type the edge distance | Click room, digits, Tab switches box, Enter submits |
| 5 | Mirror the tree | Click a room to swap its children, Enter submits |

### **Act 2: Binary Search Tree**

| Level | Goal | Controls |
| ----- | ----- | ----- |
| 1 | Walk from the root to the treasure value | Left/Right (or A/D) or click a child |
| 2 | Insert 3 new keys at the correct empty slot | Same as above; step toward an empty slot to stash |
| 3 | Name the delete case (leaf / one child / two children) | `1`, `2`, `3` or click the buttons |
| 4 | Range query: loot every room in `[lo, hi]` and prune the rest | Click rooms adjacent to ones you entered |
| 5 (Boss) | Search a degenerate (sorted) tree with a shrinking alarm | Left/Right or click |

### **Act 3: AVL Tree**

| Level | Goal | Controls |
| ----- | ----- | ----- |
| 1 | Find every room with balance factor beyond \+/-1 | Click rooms |
| 2 | Fix LL / RR imbalances with a single rotation | Drag the lowest unbalanced room sideways (right \= right rotation) |
| 3 | Identify LR / RL, then fix with two rotations | `1`\-`4` choose case, then drag rooms |
| 4 | Predict which rotation an insert will trigger | `1`\-`5` (None, LL, RR, LR, RL) |
| 5 (Boss) | k-th smallest / k-th largest and size audit | Click rooms down the path; click the wrong-size room in audit rounds |

### **Act 4: Max Heap**

| Level | Goal | Controls |
| ----- | ----- | ----- |
| 1 | Connect rooms into a complete tree that is a max-heap | Click parent, then Left/Right-click child. Enter checks (5 mistakes allowed) |
| 2 | Heapify an array by swapping | Click two rooms to swap |
| 3 | Remove root, then sift down | REMOVE ROOT button, then swap with larger child |
| 4 | Full heap sort | Swap root with last active room, then sift down |
| 5 (Boss) | Heapify and sort ascending, no hints | Swap rooms, then UNLOCK VAULT (Enter) |

### **Sandbox**

Type a number, then use the buttons (Insert / Delete / Search or Mirror / Extract Max / Heap Sort, Random, Clear, Switch Type). Enter \= Insert, Esc \= menu.

### **Menu screens**

Backspace returns to the menu from Instructions and Leaderboard.

## **6\. Scoring**

* **Alarm capacity** \= `(3 * optimal + 1) / 2 + 2`, where `optimal` is the minimum number of steps.  
* **Stars:** 3 if alarm \<= optimal and no penalties; 2 if alarm \<= 1.25 x optimal and at most 1 penalty; otherwise 1\. Hints cap stars at 2\.  
* **Score:** about `1000 - 10 * alarm` (some levels also subtract 50 per penalty or add a pruning bonus).  
* Every win is appended to `leaderboard.txt` (`levelId stars score`). The Leaderboard screen shows the top 10 and the total of the best score per level.

## **7\. Screen IDs (used by `main.cpp`)**

| ID | Screen | ID | Screen |
| ----- | ----- | ----- | ----- |
| 1 | Main menu | 14 | Act 3 L3 |
| 2-6 | Act 1 L1-L5 | 15 | Act 3 L4 |
| 7-11 | Act 2 L1-L5 | 16 | Act 3 L5 |
| 12 | Act 3 L1 | 17-21 | Act 4 L1-L5 |
| 13 | Act 3 L2 | 22 | Instructions |
| 23 | Sandbox | 24 | Map |
| 25 | Briefing screen | 26 | Leaderboard |
| 99 | Quit |  |  |

## **8\. Unit tests**

The test suite covers the pure logic (no SFML): tree traversals, reconstruction, height/diameter/LCA/mirror, max-heap and heap sort, all Act 1 and Act 4 level classes, the BST (insert/delete/search/range/LCA/k-th/serialization) and the AVL tree (all four rotation cases, balance, rank/k-th, prediction, loading).

```sh
make test          # builds and runs, writes test_report.txt
# or manually:
g++ -std=c++17 -Wall -I. test_main.cpp Act2.cpp Act3.cpp -o run_tests && ./run_tests
```

Exit code is 0 only if every check passes. `test_report.txt` lists each suite with pass/fail counts and every failed check with its file and line. Use that file as the test report deliverable.

