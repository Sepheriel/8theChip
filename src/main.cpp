#include <iostream>
#include <fstream>
#include <thread>

#include <SFML/Graphics.hpp>

#include "chip8.hpp"

int scale = 10;

// a texture representing the display
sf::Texture texture;

void renderThread(sf::RenderWindow* window, Chip8* chip) {

	// create the sprite for drawing the display
	sf::Sprite sprite;
	texture.create(64, 32);
	sprite.setTexture(texture);
	sprite.setScale(scale, scale);

	sf::Clock clock;
	sf::Time time;

	// the rendering loop
	while (window->isOpen()) {

		// print the display
		window->clear();
        
        texture.update(chip->display);
		window->draw(sprite);
		window->display();

		// print FPS
		//time = clock.getElapsedTime();
		//std::cout << 1.0f / time.asSeconds() << std::endl;
		//clock.restart().asSeconds();
	}


}


int main(int argc, char** argv) {

    if(argc < 3) {
        std::cout << "Usage is: program -r <path to rom>" << std::endl;
        return 0;
    }
    
    if(strcmp(argv[1], "-r") != 0) {
        std::cout << "Usage is: program -r <path to rom>" << std::endl;
        return 0;
    }
    
    // create the CHIP-8
    Chip8 chip;
    chip.reset();
    
    // create the window for drawing the screen
    sf::RenderWindow window(sf::VideoMode(64*scale, 32*scale), "I 8 the Chip");
	window.setVerticalSyncEnabled(true);
    
    // load the program and dump the ram to console
    chip.loadProgram(argv[2]);

	// detach the window to another thread for drawing
	window.setActive(false);
	std::thread render(&renderThread, &window, &chip);

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();

			if (event.type == sf::Event::KeyPressed) {
				if (event.key.code == sf::Keyboard::Escape) {
					window.close();
				}
			}
            
		}

        // if cpu is not halted: cycle
		if(!chip.halt) chip.cycle();

	}
	render.join();
	return 0;
}

