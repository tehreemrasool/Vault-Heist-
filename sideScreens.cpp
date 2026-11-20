#include "screen.h"
using namespace std;
string getString(int level)
{
    if (level == 1) return "The guards follow a fixed route.\nPredict their movements,\nor they will find you first.";
    if (level == 2) return "The architect never left\nhis secrets in plain sight.\nThe blueprint can only be restored\n by understanding its hidden order.";
    if (level == 3)return "Security breach detected.\nThe alarm is spreading\nthrough every connected level.\nCalculate how far it can reach\n before the entire vault locks down.";
    if (level == 4) return "Every path has an origin.\nFind the chamber where two\n journeys become one.";
    if (level == 5) return "\tWhat stands on the left\n\tmust stand on the right.\n\tWhat stands on the right\n\t must stand on the left.\nOnly then will the path reveal itself.";
    if (level == 16) return "The strongest treasure must always\n stand above the weaker ones. ";
    if (level == 17) return "The hierarchy has been corrupted.\n Restore the order before the guards arrive.";
    if (level == 18) return "The greatest must leave first.\n But removing the strongest\n will disturb the entire hierarchy.";
    if (level == 19) return "Your time is running out.\n Every unnecessary movement brings\n the guards closer.\n Complete the sorting before\n they reach you.";
    if (level == 20) return "You have followed the paths,\n restored the trees,\n and mastered the hierarchy.\n Now there is nothing left to teach you.";
    return "The vault awaits...";
}

string getHint(int level)
{
    if (level == 1) return "hint1";
    if (level == 2) return "hint1";
    if (level == 3) return "hint1";
    if (level == 4) return "hint1";
    if (level == 5) return "hint1";
    if (level == 6) return "hint1";
    if (level == 7) return "hint1";
    if (level == 8) return "hint1";
    if (level == 9) return "hint1";
    if (level == 10) return "hint1";
    if (level == 11) return "hint1";
    if (level == 12) return "hint1";
    if (level == 13) return "hint1";
    if (level == 14) return "hint1";
    if (level == 15) return "hint1";
    if (level == 16) return "hint1";
    if (level == 17) return "hint1";
    if (level == 18) return "hint1";
    if (level == 19) return "hint1";
    if (level == 20) return "hint1";
    
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
        background.setScale(1280.0f / backgroundTexture.getSize().x, 720.0f / backgroundTexture.getSize().y);
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


//---------------------------Instructions-----------------------------

instructionWindow::instructionWindow(RenderWindow& window) : Screen(window), square(Vector2f(1280.0f, 720.0f))
{
    font.loadFromFile("assets/fonts/SuperShiny.ttf");
    instructions.setFont(font);
    instructions.setString("\t\t\t\t\tTHE HEIST\n"
        "Infiltrate an ancient vault whose rooms form a living tree.\n"
        "Decode the blueprints and solve each chamber to advance.\n\n"
        "\t\t\t\t\tTHE ALARM\n"
        "Every wrong move or timeout raises the Alarm Meter.\n"
        "If the alarm fills up, the vault locks down and you lose!\n"
        "Finish with fewer alarm steps to earn up to 3 Stars.\n\n"
        "\t\t\t\t\tCONTROLS\n"
        "Left Click : Select rooms & boxes\n"
        "0-9 / Enter : Type & submit answers\n"
        "Space : Reset puzzle   |   R : New tree\n"
        "Esc : Pause & level briefing"
        "\n\n\n\n\t\t\t\t\t\t\t\t\t\t\tPress backspace to return to menu");



    if (backgroundTexture.loadFromFile("assets/pictures/Hmain.jfif"))
    {
        background.setTexture(backgroundTexture);
        background.setScale(1280.0f / backgroundTexture.getSize().x, 720.0f / backgroundTexture.getSize().y
        );
    }
    instructions.setCharacterSize(22);
    instructions.setFillColor(Color::White);

    FloatRect tb = instructions.getLocalBounds();
    instructions.setOrigin(tb.left + tb.width / 2.0f, tb.top + tb.height / 2.0f);
    instructions.setPosition(640, 360);

    const float pad = 40;
    square.setSize(Vector2f(tb.width + pad * 2, tb.height + pad * 2));
    square.setOrigin(square.getSize().x / 2.0f, square.getSize().y / 2.0f);
    square.setPosition(640, 360);
    square.setFillColor(Color(0, 0, 0, 220));
    square.setOutlineThickness(4);
    square.setOutlineColor(Color(212, 110, 194));
}

void instructionWindow::update(float time) {}


void instructionWindow::draw()
{
    window.draw(background);
    window.draw(square);
    window.draw(instructions);
}

void instructionWindow::onEnter()
{
    isActive = true;
    nextScreen = 21;

}

void instructionWindow::handleInput()
{
    Event e;
    while (window.pollEvent(e))
    {
        if (e.type == Event::Closed) nextScreen = 99;
        if (e.type == Event::KeyPressed)
        {
            if (e.key.code == Keyboard::BackSpace)
            {
                nextScreen = 1;
            }
        }

    }
}

