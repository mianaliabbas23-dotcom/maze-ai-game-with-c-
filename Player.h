#pragma once
#include <SFML/Graphics.hpp>
class Grid;
class Player {
public:
    Player(int startX, int startY);
    bool move(int dx, int dy, Grid& grid);
    void update();
    void render(sf::RenderWindow& window);
    int getX() const;
    int getY() const;
    int getScore() const;
    void addScore(int pts);
private:
    int x, y, score;
    float speed;
    sf::RectangleShape shape;
};
