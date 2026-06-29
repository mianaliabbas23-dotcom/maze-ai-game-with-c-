#include "Player.h"
#include "Grid.h"
#include "Config.h"
Player::Player(int startX, int startY) : x(startX), y(startY), score(0), speed(0.2f) {
    shape.setSize(sf::Vector2f(CELL_SIZE-6, CELL_SIZE-6));
    shape.setFillColor(sf::Color(60,180,255));
    shape.setOutlineThickness(1.5f);
    shape.setOutlineColor(sf::Color(160,230,255));
}
bool Player::move(int dx, int dy, Grid& grid) {
    int newX=x+dx, newY=y+dy;
    if(newX>=0&&newX<Grid::SIZE&&newY>=0&&newY<Grid::SIZE&&!grid.getCell(newX,newY).isWall){x=newX;y=newY;return true;}
    return false;
}
void Player::update() {
    shape.setPosition(sf::Vector2f(x*CELL_SIZE+OFFSET_X+3, y*CELL_SIZE+OFFSET_Y+3));
}
void Player::render(sf::RenderWindow& window) { window.draw(shape); }
int  Player::getX()     const { return x; }
int  Player::getY()     const { return y; }
int  Player::getScore() const { return score; }
void Player::addScore(int pts) { score+=pts; }
