#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include "header.h"
using namespace sf;
using namespace std;

void generateRandomArray(int* arr, int& s)
{
	int num = rand() % 10;
	s = num;
	arr = new int[s];
	for (int i = 0; i < s; i++)
	{
		arr[i] = rand() % 101;
	}
}

class act4level1
{
	int penalities;
	int state = 0;         // 0 if lost 1 if win 2 if still playing
	maxHeap* answerKey;
	int* arr;
	int size;
	


public:
	static const int PLAYING = 0;
	static const int WON = 1;
	static const int LOST = 2;

	act4level1()
	{
		start();

	}



	void start()
	{
		generateRandomArray(arr, size);
		answerKey = new maxHeap(size);
		for (int i = 0; i < size; i++)
		{
			answerKey->insert(arr[i]);
		}
	}

	bool checkResults(int arr[])
	{
		return isMaxHeap(arr, size);


	}

};