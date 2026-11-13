#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include "screen.h"
using namespace sf;
using namespace std;

int main()
{
	RenderWindow window(VideoMode(1280, 720), "Vault Heist");
	window.setFramerateLimit(60);

	Music music;
	if (music.openFromFile("assets/themeSong.mp3"))
	{
		music.setLoop(true);
		music.play();
	}
	else
		cout << "Could not load assets/themeSong.mp3 (continuing without music)\n";

	MainMenuScreen menu(window);    // screen id 1
	Level1Screen level(window);     // screen id 2

	Screen* current = &menu;
	int currentId = 1;
	current->onEnter();

	Clock clock;
	while (window.isOpen())
	{
		float dt = clock.restart().asSeconds();

		current->handleInput();
		int next = current->getNextScreen();

		if (next == 99) { window.close(); break; }

		if (next != currentId)
		{
			current->onExit();
			current = (next == 1) ? (Screen*)&menu : (Screen*)&level;
			currentId = next;
			current->onEnter();
		}

		current->update(dt);
		current->draw();
		window.display();
	}
	return 0;
}