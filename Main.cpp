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
sf::Time dt;

sf::RenderWindow window(sf::VideoMode(0, 0), "MY Physics Engine!", sf::Style::Fullscreen);
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

    Slider(int index, float _maxValue, float startingValue, float _minValue = 0)
    {
        maxValue = _maxValue;
        minValue = _minValue;
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
        frontRect.setSize(sf::Vector2f(maxWidth * ((value - minValue - 1) / (maxValue - minValue - 1)), size.y - sidesOffset * 2));
        frontRect.setFillColor(sf::Color(255, 240, 31));
    }

    void changeValue(float val)
    {
        value = val;
        frontRect.setSize(sf::Vector2f(maxWidth * ((value - minValue - 1) / (maxValue - minValue - 1)), size.y - sidesOffset * 2));
    }

    void add(int n)
    {
        if (value < maxValue)
        {
            value += n;
        }
        frontRect.setSize(sf::Vector2f(maxWidth * ((value - minValue - 1) / (maxValue - minValue - 1)), size.y - sidesOffset * 2));
    }

    void subtract(int n)
    {
        if (value > minValue)
        {
            value -= n;
        }
        frontRect.setSize(sf::Vector2f(maxWidth * ((value - minValue - 1) / (maxValue - minValue - 1)), size.y - sidesOffset * 2));
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

    bool isPivot;
    float radius;

    Ball()
    {
    }

    Ball(Vec2 _position, float _radius, sf::Color _color, bool _isPivot = false)
    {
        radius = _radius;
        position = _position;
        oldPosition = _position;
        isPivot = _isPivot;

        sprite = sf::CircleShape();
        sprite.setPosition(sf::Vector2f(position.x, position.y));
        sprite.setFillColor(_color);
        sprite.setRadius(radius);
    }

    void update()
    {
        if (isPivot)
            return;

        Vec2 velocity = position - oldPosition;

        oldPosition = position;

        position = position + velocity + acceleration;

        acceleration = Vec2();

        sprite.setPosition(sf::Vector2f(position.x - radius, position.y - radius));
    }
};

struct Link
{
    float targetDist;
    Ball *objPtr1;
    Ball *objPtr2;
    bool pivot;

    Link(Ball &object1, Ball &object2, float _targetDist)
    {
        targetDist = _targetDist;
        objPtr1 = &object1;
        objPtr2 = &object2;
    }

    void update()
    {
        Vec2 dist = objPtr1->position - objPtr2->position;
        float absDist = dist.euclideanDistance();
        Vec2 direction = dist / absDist;
        Vec2 nudge = direction * ((targetDist - absDist) / 2);

        if (objPtr1->isPivot)
        {
            objPtr2->position -= nudge;
        }
        else if (objPtr2->isPivot)
        {
            objPtr1->position += nudge;
        }
        else
        {
            objPtr1->position += nudge;
            objPtr2->position -= nudge;
        }
    }
};

Slider radiusSlider = Slider(0, maxBallRadius, 1, 1);
vector<Ball> tempballs;
vector<Ball> balls;

sf::Color randomColor()
{
    return sf::Color(rand() % 155 + 100, rand() % 155 + 100, rand() % 155 + 100);
}

void loop()
{
    Vec2 gravity = {0, 0.01};

    float constraintRadius = 450;
    Vec2 constraintPosition = WINDOW_SIZE / 2;

    int ballsToBeSpawned = 10;
    ballAmount = ballsToBeSpawned;
    float radius = 440;
    float angle = 0;

    // ballsToBeSpawned /= 2;
    // for (int i = 0; i < ballsToBeSpawned; i++)
    // {
    //     balls.push_back(Ball(Vec2(WINDOW_SIZE.x / 2 - cos(angle) * radius, WINDOW_SIZE.y / 2 - sin(angle) * radius), 5, sf::Color(randomColor())));
    //     balls.push_back(Ball(Vec2(WINDOW_SIZE.x / 2 + cos(angle) * radius, WINDOW_SIZE.y / 2 + sin(angle) * radius), 5, sf::Color(randomColor())));
    //     angle += 0.1;
    //     radius -= 0.5;
    // }

    for (int i = 0; i < ballsToBeSpawned; i++)
    {
        balls.push_back(Ball(Vec2(WINDOW_SIZE.x / 2 + i * 20, WINDOW_SIZE.y / 2), 10, sf::Color(randomColor())));
    }

    vector<Link> links;

    for (int i = 0; i < ballsToBeSpawned - 1; i++)
    {
        links.push_back(Link(balls[i], balls[i + 1], 30));
    }

    balls[0].isPivot = true;
    balls[ballAmount - 1].isPivot = true;
    int linkAmount = links.size();

    tempballs = balls;

    sf::Clock clock;
    while (window.isOpen())
    {
        dt = clock.restart();

        sf::sleep(sf::milliseconds(10));

        for (int i = 0; ballAmount < tempballs.size(); i++)
        {
            balls.push_back(tempballs[ballAmount + i]);
            ballAmount += 1;
        }

        for (int _ = 0; _ < substeps; _++)
        {
            for (int i = 0; i < linkAmount; i++)
            {
                links[i].update();
            }

            for (int i = 0; i < ballAmount; i++)
            {
                balls[i].acceleration += gravity;

                float smallerRadius = constraintRadius - balls[i].radius;
                Vec2 vectorDistance = balls[i].position - constraintPosition;
                float absDistance = vectorDistance.euclideanDistance();

                if (absDistance > smallerRadius)
                    balls[i].position -= (vectorDistance / absDistance) * (absDistance - smallerRadius);

                for (int j = i + 1; j < ballAmount; j++)
                {
                    float combinedRadius = balls[i].radius + balls[j].radius;
                    Vec2 vectorDist = balls[i].position - balls[j].position;
                    float absDist = vectorDist.euclideanDistance();

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
            tempballs.push_back(Ball(Vec2(mouse.x, mouse.y), radiusSlider.value, randomColor()));
            cout << 1 / dt.asSeconds() << endl;
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

    delete[] & balls;
    delete[] & tempballs;

    return 0;
}
