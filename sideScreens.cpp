#include "screen.h"
using namespace std;



string getString(int level)
{
    string s;
    if (level == 1) return "The guards follow a fixed route.\nPredict their movements,\nor they will find you first.";
    if (level == 2) return "The architect never left\nhis secrets in plain sight.\nThe blueprint can only be restored\n by understanding its hidden order.";
    if (level == 3)return "Security breach detected.\nThe alarm is spreading through every connected level.\nCalculate how far it can reach before the entire vault locks down.";
    if (level == 4) return "Every path has an origin.\nFind the chamber where two journeys become one.";
    if (level == 5) return "What stands on the left must stand on the right.\nWhat stands on the right must stand on the left.\nOnly then will the path reveal itself.";

    return "The vault awaits...";
}
void blackScreen::setTarget(int screenId)
{
    targetScreen = screenId;
    setMessage(screenId - 1);
}
void blackScreen::setMessage(int level)
{
    msg.setString(getString(level));

    FloatRect b = msg.getLocalBounds();
    msg.setOrigin(b.left + b.width / 2, b.top + b.height / 2);
    msg.setPosition(640, 360);
}

blackScreen::blackScreen(RenderWindow& window) : Screen(window)
{
	font.loadFromFile("assets/fonts/MedievalSharp.ttf");
    if (backgroundTexture.loadFromFile("assets/pictures/circle.jfif"))
    {
        background.setTexture(backgroundTexture);
        background.setScale(1280.0f / backgroundTexture.getSize().x, 720.0f / backgroundTexture.getSize().y
        );
    }
    nextScreen = 25;
    targetScreen = 2;

    msg.setFont(font);
    msg.setFillColor(Color::White);
    msg.setCharacterSize(25);
    msg.setPosition(500, 200);
    setMessage(targetScreen - 1);
    
    


}
void blackScreen::update(float time)
{
  }
void blackScreen::onEnter()
{
    isActive = true;
    nextScreen = 25;
    setMessage(targetScreen - 1);

}
void blackScreen::handleInput()
{
    Event event;
    while (window.pollEvent(event))
    {
        if (event.type == Event::Closed) nextScreen = 99;

        if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left)
        {
            nextScreen = targetScreen;
        }
    }

}

void blackScreen::draw()
{
    window.clear();
    window.draw(background);
    window.draw(msg);
}

//-----------------------------------------MAP--------------------------------------------------
Map::Map(RenderWindow& window) : Screen(window)
{
    font.loadFromFile("assets/fonts/SuperShiny.ttf");
    if (backgroundTexture.loadFromFile("assets/pictures/map.jfif"))
    {
        background.setTexture(backgroundTexture);
        background.setScale(1280.0f / backgroundTexture.getSize().x, 720.0f / backgroundTexture.getSize().y
        );
    }

    nextScreen = 24;

}

void Map::onEnter()
{
    isActive = true;
    nextScreen = 24;
}

void Map::handleInput()
{
    Event e;
    while (window.pollEvent(e))
    {
        if (e.type == Event::Closed) nextScreen = 99;

        if (e.type == Event::MouseButtonPressed && e.mouseButton.button == Mouse::Left)
        {
            nextScreen = 2;
        }
    }
}
void Map::update(float time) {}

void Map::draw()
{
    window.clear();
    window.draw(background);
}
