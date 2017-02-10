#include <iostream>
#include <cmath>
#include <fstream>
#include "chip8.hpp"

// the built-in fonts of the CHIP-8
unsigned char fontset[80] =
{
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

Chip8::Chip8():
halt(false) {
    reset();
	timerClock.restart();
	cycleClock.restart();
    
    // initialise the keyboard mapping
    keys[0] = sf::Keyboard::X;
    keys[1] = sf::Keyboard::Num1;
    keys[2] = sf::Keyboard::Num2;
    keys[3] = sf::Keyboard::Num3;
    keys[4] = sf::Keyboard::Q;
    keys[5] = sf::Keyboard::W;
    keys[6] = sf::Keyboard::E;
    keys[7] = sf::Keyboard::A;
    keys[8] = sf::Keyboard::S;
    keys[9] = sf::Keyboard::D;
    keys[10] = sf::Keyboard::Y;
    keys[11] = sf::Keyboard::C;
    keys[12] = sf::Keyboard::Num4;
    keys[13] = sf::Keyboard::R;
    keys[14] = sf::Keyboard::F;
    keys[15] = sf::Keyboard::V;

	// fill the sine buffer
	const unsigned AMP = 30000;
	const double TWO_PI = 6.28318;
	const double INC = 440. / 44100;
	double x = 0;

	for (unsigned i = 0; i < 44100; i++) {
		sine[i] = AMP * sin(x*TWO_PI);
		x += INC;
	}

	// load sound into buffer
	if (!soundBuffer.loadFromSamples(sine, 44100, 1, 44100)) {
		std::cerr << "failed to load sound" << std::endl;
	}

	// assign buffer to sound
	sound.setBuffer(soundBuffer);
	sound.setLoop(true);
}

Chip8::~Chip8() {
    
}

void Chip8::reset() {
    memset(&V, 0, 16);
    memset(&s, 0, sizeof(unsigned short)*16);
    memset(&display, 0, 64*32*4);
    memset(&ram, 0, 4096);
    memcpy(&ram[0x50], &fontset, 80); // copy font to the right space
    I = DT = ST = SP = 0;
    PC = 0x200; // start of a CHIP-8 program
}

void Chip8::dumpMemory() {
    printf("+----------------------------+\n");
    printf("|         MEMORY DUMP        |\n");
    printf("+----------------------------+\n");
    for(int i = 0; i < 4096;++i) {
        if(i == 0) {
            printf("0x%.3X: ", 0);
        }
        if(i > 0 && (i % 8) == 0) {
            printf("\n");
            printf("0x%.3X: ", i);
        }
        printf("%.2X ", ram[i]);
        
    }
    printf("\n\n");
}

int Chip8::loadProgram(std::string path) {
    int size = 0;
    
    // try to open the specified file
    std::ifstream input(path, std::ios::binary);
    if(!input.is_open()) {
        std::cout << "ERROR: could not open file: " << path << std::endl;
        return -1;
    }
    
    // save the file size
    input.seekg(0, std::ios::end);
    size = input.tellg();
    
    // check if the program is too large to fit into the ram
    if(size > 3584) {
        std::cout << "ERROR: program is too large" << std::endl;
        return -1;
    }
    
    // inform the user about the next step and write the file into the ram
    std::cout << "Loading file: " << path << std::endl;
    input.seekg(0, std::ios::beg);
    input.read((char*)&ram[0x200], size);
    return 0;
}

unsigned short Chip8::fetchOpcode() {
    
    // if program counter exceeds memory
    if(PC+1 > 4096) {
        std::cout << "ERROR: program counter exceeds memory" << std::endl;
        exit(0);
    }
    
    // fetch the current opcode
    unsigned short opcode = ram[PC] << 8 | ram[PC+1];
    
    // return the opcode swapped to Big-Endian
    return opcode;
}

void Chip8::decode(unsigned short opcode) {
    
   //std::cout << "current opcode: 0x" << std::hex << opcode << std::dec << std::endl;
    
    switch(opcode & 0xF000) {
        case 0x0000:
            switch(opcode & 0x00FF) {
                case 0xE0:
                    memset(&display, 0, 64*32*4);
                    PC += 2;
                    break;
                    
                case 0xEE:
                    PC = s[(--SP)] + 2;
                    break;
                    
                default:
                    std::cout << "unknown opcode" << std::endl;
                    halt = true;
                    break;
            }
            break;
            
        case 0x1000:
            PC = (opcode & 0x0FFF);
            break;
            
        case 0x2000:
            s[(SP++)] = PC;
            PC = (opcode & 0x0FFF);
            break;
            
        case 0x3000:
            if(V[(opcode & 0x0F00)>>8] == (opcode & 0x00FF)) {
                PC += 4;
            } else {
                PC += 2;
            }
            break;
            
        case 0x4000:
            if(V[(opcode & 0x0F00)>>8] != (opcode & 0x00FF)) {
                PC += 4;
            } else {
                PC += 2;
            }
            break;
            
        case 0x5000:
            if(V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0)>>4]) {
                PC += 4;
            } else {
                PC += 2;
            }
            break;
            
        case 0x6000:
            V[(opcode & 0x0F00)>>8] = (opcode & 0x00FF);
            PC += 2;
            break;
            
        case 0x7000:
            V[(opcode & 0x0F00)>>8] += (opcode & 0x00FF);
            PC += 2;
            break;
            
        case 0x8000:
            
            switch(opcode & 0x000F) {
                    
                case 0x0:
                    V[(opcode & 0x0F00)>>8] = V[(opcode & 0x00F0)>>4];
                    PC += 2;
                    break;

				case 0x1:
					V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
					PC += 2;
					break;
                    
                case 0x2:
                    V[(opcode & 0x0F00)>>8] &= V[(opcode & 0x00F0)>>4];
                    PC += 2;
                    break;
                    
                case 0x3:
                    V[(opcode & 0x0F00)>>8] ^= V[(opcode & 0x00F0)>>4];
                    PC += 2;
                    break;
                    
                case 0x4:
                    V[(opcode & 0x0F00)>>8] += V[(opcode & 0x00F0)>>4];
                    if(V[(opcode & 0x00F0)>>4] > (0xFF - V[(opcode & 0x0F00)>>8])) {
                        V[0xF] = 1;
                    } else {
                        V[0xF] = 0;
                    }
                    PC += 2;
                    break;
                    
                case 0x5:
                    if(V[(opcode & 0x0F00)>>8] > V[(opcode & 0x00F0)>>4]) {
                        V[0xF] = 1;
                    } else {
                        V[0xF] = 0;
                    }
                    V[(opcode & 0x0F00)>>8] -= V[(opcode & 0x00F0)>>4];
                    PC += 2;
                    break;
                    
                case 0x6:
                    V[0xF] = V[(opcode & 0x0F00)>>8] & 0x1;
                    V[(opcode & 0x0F00)>>8] >>= 1;
                    PC += 2;
                    break;

				case 0x7:
					if (V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4])
						V[0xF] = 0;
					else
						V[0xF] = 1;
					V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
					PC += 2;
					break;

				case 0x000E:
					V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;
					V[(opcode & 0x0F00) >> 8] <<= 1;
					PC += 2;
					break;
                    
                default:
                    std::cout << "unknown opcode" << std::endl;
                    halt = true;
                    break;
                    
            }
            
            break;

		case 0x9000:
			if (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4]) {
				PC += 4;
			}
			else {
				PC += 2;
			}
			break;
            
        case 0xA000:
            I = (opcode & 0x0FFF);
            PC += 2;
            break;

		case 0xB000:
			PC = (opcode & 0x0FFF) + V[0];
			break;

		case 0xC000:
			V[(opcode & 0x0F00) >> 8] = (rand() % 0xFF) & (opcode & 0x00FF);
			PC += 2;
			break;
            
        case 0xD000:
        {
            
            unsigned short xS = V[(opcode & 0x0F00) >> 8];
            unsigned short yS = V[(opcode & 0x00F0) >> 4];
            unsigned short h = opcode & 0x000F;
            unsigned short p;
            
            V[0xF] = 0;
            for (int y = 0; y < h; y++)
            {
                p = ram[I + y];
                for(int x = 0; x < 8; x++)
                {
                    if((p & (0x80 >> x)) != 0)
                    {
                        if(display[((((yS+y)*64)*4)+((xS+x)*4))] == 0xFF)
                        {
                            V[0xF] = 1;
                        }
                        for (int i = 0; i < 4; i++) {
                            display[((((yS+y)*64)*4)+((xS+x)*4))+i] ^= 0xFF;
                        }
                    }
                }
            }
            PC += 2;
        }
            break;
            
        case 0xE000:
            
            switch(opcode & 0x00FF) {
                    
                case 0x9E:
                    if(sf::Keyboard::isKeyPressed(keys[V[(opcode & 0x0F00)>>8]])) {
                        PC += 4;
                    } else {
                        PC += 2;
                    }
                    break;
                    
                case 0xA1:
                    if(!sf::Keyboard::isKeyPressed(keys[V[(opcode & 0x0F00)>>8]])) {
                        PC += 4;
                    } else {
                        PC += 2;
                    }
                    break;
                    
                default:
                    std::cout << "unknown opcode" << std::endl;
                    halt = true;
                    break;
            }
            
            break;

		case 0xF000:

			switch (opcode & 0x00FF) {

                case 0x07:
                    V[(opcode & 0x0F00) >> 8] = DT;
                    PC += 2;
                    break;
                    
                case 0x0A:
                {
                    bool key_pressed = false;
                    
                    while(!key_pressed) {
                        for(unsigned char i = 0; i < 16; i++) {
                            if(sf::Keyboard::isKeyPressed(keys[i])) {
                                V[(opcode & 0x0F00)>>8] = i;
                                key_pressed = true;
                            }
                        }
                    }
                    PC += 2;
                }
                    break;

                case 0x15:
                    DT = V[(opcode & 0x0F00)>>8];
                    PC += 2;
                    break;
                    
                case 0x18:
                    ST = V[(opcode & 0x0F00)>>8];
                    PC += 2;
                    break;
                    
                case 0x1E:
                    if(I + V[(opcode & 0x0F00) >> 8] > 0xFFF)
                        V[0xF] = 1;
                    else
                        V[0xF] = 0;
                    I += V[(opcode & 0x0F00) >> 8];
                    PC += 2;
                    break;

				case 0x29:
					I = 0x50+V[(opcode & 0x0F00) >> 8] * 0x5;
					PC += 2;
					break;

				case 0x0033:
					ram[I] = V[(opcode & 0x0F00) >> 8] / 100;
					ram[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
					ram[I + 2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;
					PC += 2;
					break;

				case 0x55:
					for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i) {
						ram[I + i] = V[i];
					}
					I += ((opcode & 0x0F00) >> 8) + 1;
					PC += 2;
					break;
                    
                case 0x65:
                    for(int i = 0; i <= ((opcode & 0x0F00)>>8); ++i) {
                        V[i] = ram[I+i];
                    }
                    I += ((opcode & 0x0F00) >> 8) + 1;
                    PC += 2;
                    break;

                default:
                    std::cout << "unknown opcode" << std::endl;
                    halt = true;
                    break;
			}
			break;
            
        default:
            std::cout << "unknown opcode" << std::endl;
            halt = true;
            break;
    }
}

void Chip8::cycle() {

	if ((sound.getStatus() == sf::SoundSource::Playing)  && ST == 0) {
		sound.stop();
	}

	if ((sound.getStatus() == sf::SoundSource::Stopped) && ST != 0) {
		sound.play();
	}

	// count the timers down at about 60 Hz
	if (timerClock.getElapsedTime().asMilliseconds() > 16) {
		if (timerClock.getElapsedTime().asMilliseconds() > 18) {
			//std::cout << "timer timing missed badly" << std::endl;
		}
		if (DT > 0) DT--;
		if (ST > 0) ST--;
		timerClock.restart();
	}

	// cycle the cpu at about 500 Hz
	if (cycleClock.getElapsedTime().asMilliseconds() > 1) {
        if(cycleClock.getElapsedTime().asMilliseconds() > 3) {
            //std::cout << "clock timing missed badly" << std::endl;
        }
		decode(fetchOpcode());
		cycleClock.restart();
	}
}
