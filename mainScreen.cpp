#include <iostream>
#include <cmath>
#include "screen.h"
#include "helpers.h"
// ------------------------------------- MainMenuScreen ----------------
MainMenuScreen::MainMenuScreen(RenderWindow& win) : Screen(win), nextScreen(1)
{
    font.loadFromFile("assets/fonts/SuperShiny.ttf");
    font2.loadFromFile("assets/fonts/MedievalSharp.ttf");

    title.setFont(font);
    title.setString("Vault heist");
    title.setCharacterSize(100);
    title.setFillColor(Color::White);
    title.setOutlineColor(Color::Black);
    title.setOutlineThickness(1);
    title.setPosition(400, 60);

    const float innerLeft = 45, innerTop = 45, innerRight = 543, innerBottom = 363;
    float innerW = innerRight - innerLeft;
    float innerH = innerBottom - innerTop;

    storyBox.setSize(Vector2f(560, 560 * innerH / innerW));
    storyBox.setPosition(90, 210);
    storyBox.setFillColor(Color(0, 0, 0, 150));

    borderTexture.loadFromFile("assets/pictures/textborder.png");
    borderSprite.setTexture(borderTexture);
    float s = storyBox.getSize().x / innerW;     
    borderSprite.setScale(s, s);
    borderSprite.setPosition(storyBox.getPosition().x - innerLeft * s,
        storyBox.getPosition().y - innerTop * s);

    storyLine.setFont(font2);
    storyLine.setString("Legends speak of an ancient vault\nhidden beneath a forgotten kingdom.\nIt holds treasures beyond imagination,\nguarded by puzzles no one has ever solved.\n\nCan you find it ?");
    storyLine.setFillColor(Color::White);
    storyLine.setCharacterSize(24);
    storyLine.setOutlineColor(Color::Black);
    storyLine.setOutlineThickness(1);

    FloatRect tb = storyLine.getLocalBounds();
    storyLine.setOrigin(tb.left + tb.width / 2.0f, tb.top + tb.height / 2.0f);
    storyLine.setPosition(storyBox.getPosition().x + storyBox.getSize().x / 2.0f,
        storyBox.getPosition().y + storyBox.getSize().y / 2.0f);

    button = makeRoundedRect(260, 55, 12);
    button.setPosition(720, 210);
    button.setFillColor(Color(212, 110, 194));

    instructionButton = makeRoundedRect(260, 55, 12);
    instructionButton.setPosition(720, 277);
    instructionButton.setFillColor(Color(212, 110, 194));

    leaderboardButton = makeRoundedRect(260, 55, 12);
    leaderboardButton.setPosition(720, 344);
    leaderboardButton.setFillColor(Color(212, 110, 194));

    extraButton = makeRoundedRect(260, 55, 12);
    extraButton.setPosition(720, 411);
    extraButton.setFillColor(Color(212, 110, 194));

    buttonText.setFont(font);
    buttonText.setString("START GAME");
    buttonText.setCharacterSize(22);
    buttonText.setFillColor(Color::Black);
    centerText(buttonText, button);

    buttontext2.setFont(font);
    buttontext2.setString("Instructions");
    buttontext2.setCharacterSize(22);
    buttontext2.setFillColor(Color::Black);
    centerText(buttontext2, instructionButton);

    leaderboardText.setFont(font);
    leaderboardText.setString("Leaderboard");    
    leaderboardText.setCharacterSize(22);
    leaderboardText.setFillColor(Color::Black);
    centerText(leaderboardText, leaderboardButton);

    extraText.setFont(font);
    extraText.setString("Sand Box");             
    extraText.setCharacterSize(22);
    extraText.setFillColor(Color::Black);
    centerText(extraText, extraButton);

    backgroundTexture.loadFromFile("assets/pictures/Hmain.jfif");
    background.setTexture(backgroundTexture);
    background.setScale(1280.0f / backgroundTexture.getSize().x, 720.0f / backgroundTexture.getSize().y);
}

void MainMenuScreen::onEnter()
{
    isActive = true;
    nextScreen = 1;
    levelNumber = 0;
}

void MainMenuScreen::handleInput()
{
    Event e;
    while (window.pollEvent(e))
    {
        if (e.type == Event::Closed) nextScreen = 99;

        if (e.type == Event::MouseButtonPressed && e.mouseButton.button == Mouse::Left)
        {
            Vector2f click((float)e.mouseButton.x, (float)e.mouseButton.y);

            if (button.getGlobalBounds().contains(click))
                nextScreen = 24;
            if (instructionButton.getGlobalBounds().contains(click))
                nextScreen = 22;
            if (leaderboardButton.getGlobalBounds().contains(click))
                nextScreen = 1;   
            if (extraButton.getGlobalBounds().contains(click))
                nextScreen = 23;   
            if (leaderboardButton.getGlobalBounds().contains(click))
                nextScreen = 26;

            levelNumber = 1;
        }
    }
}

void MainMenuScreen::update(float time)
{
    Vector2f mouse = (Vector2f)Mouse::getPosition(window);

    applyHover(button, mouse);
    applyHover(instructionButton, mouse);
    applyHover(leaderboardButton, mouse);
    applyHover(extraButton, mouse);
}

void MainMenuScreen::draw()
{
    window.clear();
    window.draw(background);

    window.draw(storyBox);       
    window.draw(borderSprite);   
    window.draw(storyLine);      

    window.draw(title);

    window.draw(button);
    window.draw(buttonText);
    window.draw(instructionButton);
    window.draw(buttontext2);
    window.draw(leaderboardButton);
    window.draw(leaderboardText);
    window.draw(extraButton);
    window.draw(extraText);
    
}