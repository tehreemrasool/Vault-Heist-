
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include "screen.h"
using namespace std;
using namespace sf;


#include <cmath>

void drawSmoke(RenderWindow& window, float progress)
{
    int x[] = { 100, 250, 400, 550, 700, 850, 1000, 1150,
                180, 350, 520, 690, 860, 1030, 1200 };

    int y[] = { 150, 500, 250, 600, 180, 450, 200, 550,
                350, 100, 400, 100, 550, 350, 100 };

    float opacity;

    if (progress < 0.5f)
        opacity = progress * 2;
    else
        opacity = (1 - progress) * 2;

    int alpha = (int)(opacity * 170);

    for (int i = 0; i < 15; i++)
    {
        float radius = 90 + progress * 100 + (i % 3) * 20;

        CircleShape smoke(radius);

        smoke.setPosition(
            x[i] - radius + sin(progress * 5 + i) * 25,
            y[i] - radius - progress * 40
        );

        smoke.setFillColor(Color(180, 180, 190, alpha));

        window.draw(smoke);
    }
}
int main()
{
    RenderWindow window(VideoMode(1280, 720),"Vault Heist");
    window.setFramerateLimit(60);
    Font test;

    //if (!loadFont(test))
    //{
    //    cout << "Font not found!" <<endl;
    //    return 1;
    //}

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
    Level1Screen level(window);
    Level2Screen level2(window);
    Level3Screen level3(window);
    Level4Screen level4(window);
    Level5Screen level5(window);
    Map map_ (window);
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
            current->handleInput();
        }

        int next = current->getNextScreen();
        if (next == 99)
        {
            window.close();
            break;
        }
        if (next == 25 && currentId >= 2 && currentId <= 17)
        {
            bl.setTarget(currentId);
            pausedId = currentId;
        }

        if (next != currentId &&
            next >= 2 && next <= 17 &&
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
                    current = &menu;
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
                else if (pendingScreen == 21)
                    current = &inst;
                else if (pendingScreen == 24)
                    current = &map_;
                else if (pendingScreen == 25)
                    current = &bl;

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
