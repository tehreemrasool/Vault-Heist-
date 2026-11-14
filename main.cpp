
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include "screen.h"
using namespace std;
using namespace sf;

int main()
{
    // CREATE WINDOW
    RenderWindow window(VideoMode(1280, 720),"Vault Heist");
    window.setFramerateLimit(60);


    
    Font test;

    if (!loadFont(test))
    {
        cout << "Font not found!" << std::endl;
        return 1;
    }

    Music music;

    if (music.openFromFile("assets/themeSong.mp3"))
    {
        music.setLoop(true);
        music.play();
    }
    else
    {
        cout << "Music not loaded!" <<endl;
    }


    MainMenuScreen menu(window);
    Level1Screen level(window);
    Level2Screen level2(window);
    Map map_ (window);


    // START WITH MAIN MENU
    Screen* current = &menu;
    int currentId = 1;

    current->onEnter();


    // CLOCK
    Clock clock;


    // MAIN GAME LOOP
    while (window.isOpen())
    {
        float dt = clock.restart().asSeconds();
        current->handleInput();


        // CHECK WHICH SCREEN SHOULD OPEN
        int next = current->getNextScreen();


        // EXIT GAME
        if (next == 99)
        {
            window.close();
            break;
        }


        // CHANGE SCREEN
        if (next != currentId)
        {
            current->onExit();

            if (next == 1)
            {
                current = &menu;
            }
            else if (next == 2)
            {
                current = &level;
            }
            else if (next == 3)
            {
                current = &level2;
            }
            
            else if (next == 24)
            {
                current = &map_;
            }
            

            currentId = next;

            current->onEnter();
        }


        // UPDATE SCREEN
        current->update(dt);


        // DRAW SCREEN
        current->draw();


        // DISPLAY EVERYTHING
        window.display();
    }

    return 0;
}
