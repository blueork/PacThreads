// Current Task
// Display the PacMan
// Move the PacMan with respect to Collision
#include <iostream>
#include <SFML/Graphics.hpp>
#include <pthread.h>

#define s std
#define height 31
#define width 28

// Declare all the Global Variables here for the all the threads to access
// the underlying 2D Maze
sf::RenderWindow window;
sf::Texture pacManTex;
sf::Sprite pacManSprite;

int maze[height][width] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,1,1,1,0,0,1,1,1,0,0,1,1,0,0,1,1,1,0,0,1,1,1,0,0,1},
    {1,0,0,1,1,1,0,0,1,1,1,0,0,1,1,0,0,1,1,1,0,0,1,1,1,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,1},
    {1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,1},
    {1,0,0,1,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,1,0,0,1},
    {1,0,0,1,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,1,0,0,1},
    {1,0,0,1,1,0,0,1,1,1,0,0,0,0,0,0,0,0,1,1,1,0,0,1,1,0,0,1},
    {1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,1,0,0,0,1,1,1,1,1,1,0,0,0,1,0,0,0,0,0,0,1},
    {1,1,1,1,1,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,1,1,1,1,1},
    {0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0},
    {1,1,1,1,1,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,1,1,1,1,1},
    {1,0,0,0,0,0,0,1,0,0,0,1,1,1,1,1,1,0,0,0,1,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1},
    {1,0,0,1,1,0,0,1,1,1,0,0,0,0,0,0,0,0,1,1,1,0,0,1,1,0,0,1},
    {1,0,0,1,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,1,0,0,1},
    {1,0,0,1,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,1,0,0,1},
    {1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,1},
    {1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,1,1,1,0,0,1,1,1,0,0,1,1,0,0,1,1,1,0,0,1,1,1,0,0,1},
    {1,0,0,1,1,1,0,0,1,1,1,0,0,1,1,0,0,1,1,1,0,0,1,1,1,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1} };  


// the pacMan thread
// responsible for everything pacMan related
// shall move the PacMan with respect to Walls 

void pacMan(int direction) {

  if(direction) {
    
    sf::Vector2f currPos = pacManSprite.getPosition();
    int xPos = (currPos.x/16), yPos = (currPos.y/16);
    if(direction == 1) {
      if(maze[yPos-1][xPos] != 1)
        pacManSprite.move(0, -16);
    }
    else if(direction == 2) {
      if(maze[yPos+1][xPos] != 1)
        pacManSprite.move(0, 16);
    }
    else if(direction == 3) {
      if(maze[yPos][xPos-1] != 1)
        pacManSprite.move(-16, 0);
    }
    else if(direction == 4) {
      if(maze[yPos][xPos+1] != 1)
        pacManSprite.move(16, 0);
    }
  }
  
}

// the userInterface thread
// display the grid
// rendering the graphics
// responsible for taking input


void userInterface() {

  window.create(sf::VideoMode(448, 496), "Pac-Man");

  // Rectangle Blocks to make up the wall
  sf::RectangleShape rectangle;
  rectangle.setSize(sf::Vector2f (16, 16));
  rectangle.setOutlineThickness(0);
  rectangle.setFillColor(sf::Color::Blue);

  sf::Clock delay;
  int direction = 0;
  // to check for user inputs
  sf::Event event;
  while(window.isOpen()) {

    while(window.pollEvent(event)) {
      if(event.type == sf::Event::Closed)
        window.close();
      if(event.type == sf::Event::KeyPressed) {
        if(event.key.code == sf::Keyboard::W) {
          //delta_y = -1;
          // s::cout<<"W\n";
          direction = 1;
        }
        else if(event.key.code == sf::Keyboard::S) {
          // delta_y = 1;
          // s::cout<<"S\n";
          direction = 2;
        }
        else if(event.key.code == sf::Keyboard::A) {
          //delta_x = -1;
          // s::cout<<"A\n";
          direction = 3;
        }
        else if(event.key.code == sf::Keyboard::D) {
          //delta_x = 1;
          // s::cout<<"D\n";
          direction = 4;
        }
      }
    }

    pacMan(direction);

    window.clear(sf::Color::Black);

    // Display the grid
    for(int i = 0; i < height; ++i) {
      for(int j = 0; j < width; ++j) {
        if(maze[i][j] == 1) {
          rectangle.setPosition(16*j,16*i);
          window.draw(rectangle); 
        }
        // else if(maze[i][j] == 2) {
        //   coinSprite.setPosition(16*j, 16*i);
        //   window.draw(coinSprite);
        // }
      }
    }

      window.draw(pacManSprite);

    // draw everything on the screen
    // renders everything
    window.display();

    delay.restart();
    while(delay.getElapsedTime().asSeconds() < 0.1) {}


  }


}


int main() {

  //std::cout<<"Hello World"<<std::endl;
  
  // load the texture for PacMan
  pacManTex.loadFromFile("./Resources/Images/player.png");
  pacManSprite.setTexture(pacManTex);
  // assign the PacMan sprite the correct texture
  pacManSprite.setTextureRect(sf::IntRect(16,0,16,16));
  pacManSprite.setPosition(16, 16);

  pacManSprite.setPosition(16,16);
  userInterface();


  return 0;
}
