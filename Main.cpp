#include "Vec2.hpp"

#include <cmath>
#include <iostream>
#include <vector>
#include <unistd.h>

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>

using namespace std;

int maxBallRadius = 50;
int ballAmount = 0;
int substeps = 4;
float dt;

sf::RenderWindow window(sf::VideoMode(0, 0), "MY Physics Engine!", sf::Style::Close | sf::Style::Fullscreen);
sf::Vector2u size = window.getSize();
Vec2 WINDOW_SIZE = Vec2(size.x, size.y);

struct Slider
{
    sf::RectangleShape middleRect;
    sf::RectangleShape frontRect;
    sf::RectangleShape backRect;
    float sidesOffset;
    float maxWidth;
    float maxValue;
    float minValue;
    float value;
    Vec2 size;

    Slider(int index, float maxValue, float startingValue, float minValue = 0)
    {
        this->maxValue = maxValue;
        this->minValue = minValue;
        value = startingValue;

        size = Vec2(100, 20);
        Vec2 position = Vec2(10, 10 + ((size.y + 10) * index));
        sidesOffset = 4;

        backRect.setSize(sf::Vector2f(size.x, size.y));
        backRect.setPosition(position.x, position.y);
        backRect.setFillColor(sf::Color(50, 50, 50));

        maxWidth = size.x - sidesOffset * 2;

        middleRect.setSize(sf::Vector2f(maxWidth, size.y - sidesOffset * 2));
        middleRect.setPosition(position.x + sidesOffset, position.y + sidesOffset);
        middleRect.setFillColor(sf::Color(100, 100, 100));

        frontRect = middleRect;
        frontRect.setSize(sf::Vector2f(maxWidth * ((value - minValue) / (maxValue - minValue)), size.y - sidesOffset * 2));
        frontRect.setFillColor(sf::Color(255, 240, 31));
    }

    void changeValue(float val)
    {
        value = val;
        frontRect.setSize(sf::Vector2f(maxWidth * ((value - minValue) / (maxValue - minValue)), size.y - sidesOffset * 2));
    }

    void draw(sf::RenderWindow &window)
    {
        window.draw(backRect);
        window.draw(middleRect);
        window.draw(frontRect);
    }

    void add(int n)
    {
        if (value < maxValue)
        {
            value += n;
        }
        frontRect.setSize(sf::Vector2f(maxWidth * ((value - minValue) / (maxValue - minValue)), size.y - sidesOffset * 2));
    }

    void subtract(int n)
    {
        if (value > minValue)
        {
            value -= n;
        }
        frontRect.setSize(sf::Vector2f(maxWidth * ((value - minValue) / (maxValue - minValue)), size.y - sidesOffset * 2));
    }
};

struct Ball
{
    Vec2 position;
    Vec2 oldPosition;
    Vec2 acceleration;
    sf::CircleShape sprite;
    float radius;

    Ball(Vec2 position, float radius, sf::Color color)
    {
        this->radius = radius;
        this->position = position;
        this->oldPosition = position;

        sprite = sf::CircleShape();
        sprite.setPosition(sf::Vector2f(position.x, position.y));
        sprite.setFillColor(color);
        sprite.setRadius(radius);
    }

    void update()
    {
        Vec2 velocity = position - oldPosition;

        oldPosition = position;

        position = position + velocity + acceleration;

        acceleration = Vec2();

        sprite.setPosition(sf::Vector2f(position.x - radius, position.y - radius));
    }
};

Slider radiusSlider = Slider(0, maxBallRadius, 1);
vector<Ball> balls;

sf::Color randomColor()
{
    return sf::Color(rand() % 155 + 100, rand() % 155 + 100, rand() % 155 + 100);
}

float mySqrt(float x)
{
    unsigned int i = *(unsigned int *)&x;
    i += 127 << 23;
    i >>= 1;
    return *(float *)&i;
}

void loop()
{
    Vec2 gravity = {0, 0.01};

    float constraintRadius = 450;
    Vec2 constraintPosition = WINDOW_SIZE / 2;
    sf::CircleShape constraintSprite = sf::CircleShape();
    constraintSprite.setPosition(sf::Vector2f(constraintPosition.x - constraintRadius, constraintPosition.y - constraintRadius));
    constraintSprite.setFillColor(sf::Color::Black);
    constraintSprite.setRadius(constraintRadius);

    int ballsToBeSpawned = 1000;
    ballAmount = ballsToBeSpawned;
    float radius = 450;
    float angle = 0;

    ballsToBeSpawned /= 2;
    for (int i = 0; i < ballsToBeSpawned; i++)
    {
        balls.push_back(Ball(Vec2(WINDOW_SIZE.x / 2 - cos(angle) * radius, WINDOW_SIZE.y / 2 - sin(angle) * radius), 5, sf::Color(randomColor())));
        balls.push_back(Ball(Vec2(WINDOW_SIZE.x / 2 + cos(angle) * radius, WINDOW_SIZE.y / 2 + sin(angle) * radius), 5, sf::Color(randomColor())));
        angle += 0.1;
        radius -= 0.5;
    }

    sf::Clock clock;
    while (window.isOpen())
    {
        dt = clock.restart().asSeconds();

        for (int _ = 0; _ < substeps; _++)
        {
            for (int i = 0; i < ballAmount; i++)
            {
                balls[i].acceleration += gravity;

                float smallerRadius = constraintRadius - balls[i].radius;
                Vec2 vectorDistance = balls[i].position - constraintPosition;
                float absDistance = mySqrt(vectorDistance.x * vectorDistance.x + vectorDistance.y * vectorDistance.y);

                if (absDistance > smallerRadius)
                {
                    Vec2 direction = (vectorDistance / absDistance);
                    balls[i].position -= direction * (absDistance - smallerRadius);
                }

                for (int j = i + 1; j < ballAmount; j++)
                {
                    float combinedRadius = balls[i].radius + balls[j].radius;
                    Vec2 vectorDist = balls[i].position - balls[j].position;
                    float absDist = mySqrt(vectorDist.x * vectorDist.x + vectorDist.y * vectorDist.y);

                    if (absDist < combinedRadius)
                    {
                        float nudge = combinedRadius - absDist;
                        balls[i].position += (vectorDist / absDist) * (nudge / (combinedRadius / balls[j].radius));
                        balls[j].position -= (vectorDist / absDist) * (nudge / (combinedRadius / balls[i].radius));
                    }
                }

                balls[i].update();
            }
        }
    }
}

int main()
{
    window.setFramerateLimit(60);

    Vec2 gravity = {0, 0.01};

    float constraintRadius = 450;
    Vec2 constraintPosition = WINDOW_SIZE / 2;
    sf::CircleShape constraintSprite = sf::CircleShape();
    constraintSprite.setPosition(sf::Vector2f(constraintPosition.x - constraintRadius, constraintPosition.y - constraintRadius));
    constraintSprite.setFillColor(sf::Color::Black);
    constraintSprite.setRadius(constraintRadius);

    sf::Thread loop1(&loop);
    loop1.launch();

    sf::Event ev;
    sf::Clock clock;
    while (window.isOpen())
    {
        while (window.pollEvent(ev))
        {
            if (ev.type == sf::Event::Closed)
            {
                window.close();
            }
            if (ev.type == sf::Event::KeyPressed)
                if (ev.key.code == sf::Keyboard::Escape)
                {
                    window.close();
                }
        }

        if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
        {
            sf::Vector2i mouse = sf::Mouse::getPosition();
            balls.push_back(Ball(Vec2(mouse.x, mouse.y), radiusSlider.value, randomColor()));
            cout << 1 / (dt) << endl;
            ballAmount++;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
        {
            radiusSlider.add(1);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
        {
            radiusSlider.subtract(1);
        }

        window.clear(sf::Color(30, 30, 30));

        window.draw(constraintSprite);

        for (int i = 0; i < ballAmount; i++)
        {
            window.draw(balls[i].sprite);
        }

        radiusSlider.draw(window);

        window.display();
    }

    return 0;
}

/*

#include "Vec2.hpp"

#include <cmath>
#include <iostream>
#include <vector>
#include <unistd.h>

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>

using namespace std;

struct Slider
{
    sf::RectangleShape middleRect;
    sf::RectangleShape frontRect;
    sf::RectangleShape backRect;
    float sidesOffset;
    float maxWidth;
    float maxValue;
    float minValue;
    float value;
    Vec2 size;

    Slider(int index, float maxValue, float startingValue, float minValue = 0)
    {
        this->maxValue = maxValue;
        this->minValue = minValue;
        value = startingValue;

        size = Vec2(100, 20);
        Vec2 position = Vec2(10, 10 + ((size.y + 10) * index));
        sidesOffset = 4;

        backRect.setSize(sf::Vector2f(size.x, size.y));
        backRect.setPosition(position.x, position.y);
        backRect.setFillColor(sf::Color(50, 50, 50));

        maxWidth = size.x - sidesOffset * 2;

        middleRect.setSize(sf::Vector2f(maxWidth, size.y - sidesOffset * 2));
        middleRect.setPosition(position.x + sidesOffset, position.y + sidesOffset);
        middleRect.setFillColor(sf::Color(100, 100, 100));

        frontRect = middleRect;
        frontRect.setSize(sf::Vector2f(maxWidth * ((value - minValue) / (maxValue - minValue)), size.y - sidesOffset * 2));
        frontRect.setFillColor(sf::Color(255, 240, 31));
    }

    void changeValue(float val)
    {
        value = val;
        frontRect.setSize(sf::Vector2f(maxWidth * ((value - minValue) / (maxValue - minValue)), size.y - sidesOffset * 2));
    }

    void draw(sf::RenderWindow &window)
    {
        window.draw(backRect);
        window.draw(middleRect);
        window.draw(frontRect);
    }
};

struct Ball
{
    Vec2 position;
    Vec2 oldPosition;
    Vec2 acceleration;
    sf::CircleShape sprite;
    float radius;

    Ball(Vec2 position, float radius, sf::Color color)
    {
        this->radius = radius;
        this->position = position;
        this->oldPosition = position;

        sprite = sf::CircleShape();
        sprite.setPosition(sf::Vector2f(position.x, position.y));
        sprite.setFillColor(color);
        sprite.setRadius(radius);
    }

    void update(float dt)
    {
        Vec2 velocity = position - oldPosition;

        oldPosition = position;

        position = position + velocity + acceleration;

        acceleration = Vec2();

        sprite.setPosition(sf::Vector2f(position.x - radius, position.y - radius));
    }
};

vector<Ball> updateBalls(vector<Ball> balls, Vec2 gravity, float constraintRadius, Vec2 constraintPosition, int ballAmount, int substeps, float dt)
{
    for (int _ = 0; _ < substeps; _++)
    {
        for (int i = 0; i < ballAmount; i++)
        {
            balls[i].acceleration += gravity;

            float smallerRadius = constraintRadius - balls[i].radius;
            Vec2 vectorDistance = balls[i].position - constraintPosition;
            float absDistance = sqrt(vectorDistance.x * vectorDistance.x + vectorDistance.y * vectorDistance.y);

            if (absDistance > smallerRadius)
            {
                Vec2 direction = (vectorDistance / absDistance);
                balls[i].position -= direction * (absDistance - smallerRadius);
            }

            for (int j = i + 1; j < ballAmount; j++)
            {
                float combinedRadius = balls[i].radius + balls[j].radius;
                Vec2 vectorDist = balls[i].position - balls[j].position;
                float absDist = sqrt(vectorDist.x * vectorDist.x + vectorDist.y * vectorDist.y);

                if (absDist < combinedRadius)
                {
                    float nudge = combinedRadius - absDist;
                    balls[i].position += (vectorDist / absDist) * (nudge / (combinedRadius / balls[j].radius));
                    balls[j].position -= (vectorDist / absDist) * (nudge / (combinedRadius / balls[i].radius));
                }
            }

            balls[i].update(dt);
        }
    }

    return balls;
}

sf::Color randomColor()
{
    return sf::Color(rand() % 155 + 100, rand() % 155 + 100, rand() % 155 + 100);
}

int main()
{
    sf::RenderWindow window(sf::VideoMode(0, 0), "MY Physics Engine!", sf::Style::Titlebar | sf::Style::Close | sf::Style::Fullscreen);
    sf::Vector2u size = window.getSize();
    Vec2 WINDOW_SIZE = Vec2(size.x, size.y);
    window.setFramerateLimit(60);
    sf::Event ev;

    Vec2 gravity = {0, 0.01};
    int substeps = 4;

    float constraintRadius = 450;
    Vec2 constraintPosition = WINDOW_SIZE / 2;
    sf::CircleShape constraintSprite = sf::CircleShape();
    constraintSprite.setPosition(sf::Vector2f(constraintPosition.x - constraintRadius, constraintPosition.y - constraintRadius));
    constraintSprite.setFillColor(sf::Color::Black);
    constraintSprite.setRadius(constraintRadius);

    vector<Ball> balls;
    int ballsToBeSpawned = 1200;
    int maxBallRadius = 50;
    int ballRadius = 1;
    float radius = 450;
    float angle = 0;
    bool mouseDown;

    Slider radiusSlider = Slider(0, maxBallRadius, 1);

    int ballAmount = ballsToBeSpawned;

    ballsToBeSpawned /= 2;
    for (int i = 0; i < ballsToBeSpawned; i++)
    {
        balls.push_back(Ball(Vec2(WINDOW_SIZE.x / 2 - cos(angle) * radius, WINDOW_SIZE.y / 2 - sin(angle) * radius), 5, sf::Color(randomColor())));
        balls.push_back(Ball(Vec2(WINDOW_SIZE.x / 2 + cos(angle) * radius, WINDOW_SIZE.y / 2 + sin(angle) * radius), 5, sf::Color(randomColor())));
        angle += 0.1;
        radius -= 0.5;
    }

    sf::Clock clock;
    while (true)
    {
        float dt = clock.restart().asSeconds() / substeps;

        while (window.pollEvent(ev))
        {
            if (ev.type == sf::Event::Closed)
                return 0;
            if (ev.type == sf::Event::KeyPressed)
                if (ev.key.code == sf::Keyboard::Escape)
                    return 0;
            if (ev.type == sf::Event::MouseButtonPressed)
                mouseDown = true;
            if (ev.type == sf::Event::MouseButtonReleased)
                mouseDown = false;
        }

        if (mouseDown && ballRadius != 0)
        {
            sf::Vector2i mouse = sf::Mouse::getPosition();
            balls.push_back(Ball(Vec2(mouse.x, mouse.y), ballRadius, sf::Color(randomColor())));
            cout << 1 / (dt * substeps) << endl;
            ballAmount += 1;
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) && ballRadius < maxBallRadius)
        {
            ballRadius += 1;
            radiusSlider.changeValue(ballRadius);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) && ballRadius > 1)
        {
            ballRadius -= 1;
            radiusSlider.changeValue(ballRadius);
        }

        window.clear(sf::Color(30, 30, 30));

        window.draw(constraintSprite);

        balls = updateBalls(balls, gravity, constraintRadius, constraintPosition, ballAmount, substeps, dt);

        for (int i = 0; i < ballAmount; i++)
        {
            window.draw(balls[i].sprite);
        }

        radiusSlider.draw(window);

        window.display();
    }

    return 0;
}


*/