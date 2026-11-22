
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include "helpers.h"
#include "screen.h"
#include "Act3level1.h"
#include "Act3level2.h"
#include "Act3level3.h"
#include "Act3Level4.h"
#include "Act3Level5.h"
#include "Act2Level1.h"
#include "Act2Level2.h"
#include "Act2Level3.h"
#include "Act2Level4.h"
#include "Act2Level5.h"
using namespace std;
using namespace sf;
#include <cmath>


int main()
{
    RenderWindow window(VideoMode(1280, 720),"Vault Heist");

    window.setFramerateLimit(60);
    Font test;
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
    instructionWindow inst(window);
    SandboxScreen sandbox(window);
    Level1Screen level(window);
    Level2Screen level2(window);
    Level3Screen level3(window);
    Level4Screen level4(window);
    Level5Screen level5(window);
    Act2Level1Screen act2l1(window);
    Act2Level2Screen act2l2(window);
    Act2Level3Screen act2l3(window);
    Act2Level4Screen act2l4(window);
    Act2Level5Screen act2l5(window);
    Act3Level1Screen act3l1(window);
    Act3Level2Screen act3l2(window);
    Act3Level3Screen act3l3(window);
    Act3Level4Screen act3l4(window);
    Act3Level5Screen act3l5(window);
    Act4Level1Screen act4level1(window);
    Act4Level2Screen act4level2(window);
    Act4Level3Screen act4level3(window);
    Act4Level4Screen act4level4(window);
    Act4Level5Screen act4level5(window);

    Map map_ (window);
    ResultPopup popup(window);
    LeaderboardScreen board(window);
    bool resultShown = false;
    blackScreen bl(window);

    Screen* current = &menu;
    int currentId = 1;
    int pausedId = 0;
    bool transitioning = false;
    bool switched = false;

    float transitionTime = 0;

    const float transitionDuration = 1.2f;

    int pendingScreen = currentId;

    current->onEnter();


    Clock clock;
    while (window.isOpen())
    {
        float dt = clock.restart().asSeconds();

        if (!transitioning)
        {
            if (popup.isOpen()) popup.handleInput();
            else                current->handleInput();
        }

        int act = popup.takeAction();
        int next = current->getNextScreen();

        if (next == 99 || act == 99)
        {
            window.close();
            break;
        }

        if (!transitioning && !popup.isOpen() && currentId >= 2 && currentId <= 21)
        {
            int st, sc, al, pe;
            if (current->getResult(st, sc, al, pe))
            {
                if (!resultShown)
                {
                    resultShown = true;
                    popup.show(currentId, st, sc, al, pe);
                }
            }
            else
                resultShown = false;
        }

        if (act == 1)                        
        {
            popup.close();
            next = (currentId == 21) ? 26 : currentId + 1;
        }
        else if (act == 2)                   // L on the popup
        {
            popup.close();
            next = 26;
        }
        if (next == 99)
        {
            window.close();
            break;
        }
        if (next == 25 && currentId >= 2 && currentId <= 21)
        {
            bl.setTarget(currentId);
            pausedId = currentId;
        }

        if (next != currentId &&
            next >= 2 && next <= 21 &&
            currentId != 25)
        {
            bl.setTarget(next);
            next = 25;
        }
        if (!transitioning && next != currentId)
        {
            pendingScreen = next;
            transitioning = true;
            switched = false;
            transitionTime = 0;
        }
        if (transitioning)
        {
            transitionTime += dt;
            if (!switched && transitionTime >= transitionDuration / 2)
            {
                current->onExit();

                if (pendingScreen == 1)
                {
                    current = &menu;
                    pausedId = 0;
                }
                else if (pendingScreen == 2)
                    current = &level;
                else if (pendingScreen == 3)
                    current = &level2;
                else if (pendingScreen == 4)
                    current = &level3;
                else if (pendingScreen == 5)
                    current = &level4;
                else if (pendingScreen == 6)
                    current = &level5;
                else if (pendingScreen == 7)
                    current = &act2l1;
                else if (pendingScreen == 8)
                    current = &act2l2;
                else if (pendingScreen == 9)
                    current = &act2l3;
                else if (pendingScreen == 10)
                    current = &act2l4;
                else if (pendingScreen == 11)
                    current = &act2l5;
                else if (pendingScreen == 12)
                    current = &act3l1;
                else if (pendingScreen == 13)
                    current = &act3l2;
                else if (pendingScreen == 14)
                    current = &act3l3;
                else if (pendingScreen == 15)
                    current = &act3l4;
                else if (pendingScreen == 16)
                    current = &act3l5;
                else if (pendingScreen == 17)
                    current = &act4level1;
                else if (pendingScreen == 18)
                    current = &act4level2;
                else if (pendingScreen == 19)
                    current = &act4level3;
                else if (pendingScreen == 20)
                    current = &act4level4;
                else if (pendingScreen == 21)
                    current = &act4level5;
                else if (pendingScreen == 22)
                    current = &inst;
                else if (pendingScreen == 23)
                    current = &sandbox;
                
                else if (pendingScreen == 24)
                    current = &map_;
                else if (pendingScreen == 25)
                    current = &bl;
                else if (pendingScreen == 26)
                    current = &board;

                currentId = pendingScreen;

                if (currentId == pausedId)
                {
                    current->onResume();
                    pausedId = 0;
                }
                else
                {
                    current->onEnter();
                }

                switched = true;
            }

            if (transitionTime >= transitionDuration)
            {
                transitioning = false;
            }
        }

        current->update(dt);

        current->draw();
        popup.draw();


        if (transitioning)
        {
            float alpha;

            if (transitionTime < transitionDuration / 2)
            {
                alpha = (transitionTime / (transitionDuration / 2)) * 255;
            }
            else
            {
                alpha = (1 - (transitionTime - transitionDuration / 2)
                    / (transitionDuration / 2)) * 255;
            }

            RectangleShape fade(Vector2f(1280, 720));
            fade.setPosition(0, 0);
            fade.setFillColor(Color(0, 0, 0, (int)alpha));

            window.draw(fade);
        }


        window.display();
    }


    return 0;
}
