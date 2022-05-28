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

struct Ball
{
    Vec2 position;
    Vec2 oldPosition;
    Vec2 acceleration;
    sf::CircleShape sprite;
    float radius;

    Ball(Vec2 position, float radius)
    {
        this->radius = radius;
        this->position = position;
        this->oldPosition = position;

        sprite = sf::CircleShape();
        sprite.setPosition(sf::Vector2f(position.x, position.y));
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
    int ballsToBeSpawned = 500;
    int ballRadius = 20;
    float radius = 300;
    float angle = 0;
    bool mouseDown;

    sf::Clock clock;
    while (ballsToBeSpawned > balls.size())
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

        if (mouseDown)
        {
            sf::Vector2i mouse = sf::Mouse::getPosition();
            balls.push_back(Ball(Vec2(mouse.x, mouse.y), ballRadius));
            cout << 1 / (dt * substeps) << endl;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
            ballRadius += 1;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
            ballRadius -= 1;

        // balls.push_back(Ball(Vec2(WINDOW_SIZE.x / 2 - cos(angle) * radius, WINDOW_SIZE.y / 2 - sin(angle) * radius), 5));
        // balls.push_back(Ball(Vec2(WINDOW_SIZE.x / 2 + cos(angle) * radius, WINDOW_SIZE.y / 2 + sin(angle) * radius), 5));
        // radius -= 1;
        // angle += 0.5;

        window.clear(sf::Color(30, 30, 30));

        window.draw(constraintSprite);

        int ballAmount = balls.size();

        balls = updateBalls(balls, gravity, constraintRadius, constraintPosition, ballAmount, substeps, dt);

        for (int i = 0; i < ballAmount; i++)
        {
            window.draw(balls[i].sprite);
        }

        window.display();
    }

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

        if (mouseDown)
        {
            sf::Vector2i mouse = sf::Mouse::getPosition();
            balls.push_back(Ball(Vec2(mouse.x, mouse.y), ballRadius));
            cout << 1 / (dt * substeps) << endl;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
            ballRadius += 1;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
            ballRadius -= 1;

        window.clear(sf::Color(30, 30, 30));

        window.draw(constraintSprite);

        int ballAmount = balls.size();

        balls = updateBalls(balls, gravity, constraintRadius, constraintPosition, ballAmount, substeps, dt);

        for (int i = 0; i < ballAmount; i++)
        {
            window.draw(balls[i].sprite);
        }

        window.display();
    }

    return 0;
}
