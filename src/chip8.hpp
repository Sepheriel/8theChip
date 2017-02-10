#ifndef cpu_hpp
#define cpu_hpp

#include <stdio.h>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

class Chip8 {
    
public:
    Chip8();
    ~Chip8();
    
	// timer clocks
	sf::Clock timerClock;
	sf::Clock cycleClock;

    // halt flag
    bool halt;
        
    // real registers
    unsigned char V[16];	// general purpose register
    unsigned short I;		// I register
    unsigned char DT;		// delay timer
    unsigned char ST;		// sound timer
        
    // pseudo registers
    unsigned short PC;		// program counter
    unsigned char SP;		// stack pointer
    unsigned short s[16];	// stack
    
    /* a maximum sized ram
     +------------------+
     |    MEMORY MAP    |
     +------------------+ 0xFFF - end of addressable space
     |                  |
     |                  |
     |                  |
     |                  |
     +------------------+ 0x200 - start of CHIP-8 programs
     +------------------+ 0x1FF - end of CHIP-8 interpreter
     |                  |
     +------------------+ 0x0A0 - end of built-in font space
     |                  |
     +------------------+ 0x050 - start of built-in font space
     |                  |
     +------------------+ 0x000 - start of CHIP-8 interpreter
     */
    unsigned char ram[4096];
    
    // 64 * 32 pixel CHIP-8 display mapped to (RGBA)
    unsigned char display[64*32*4];
    
    // mapping of the keys
    sf::Keyboard::Key keys[16];

	// the sound
	sf::Sound sound;

	// the sound buffer
	sf::SoundBuffer soundBuffer;

	// sine buffer for the beep
	short sine[44100];
    
    // load program into the ram
    int loadProgram(std::string path);
    
    // dump the memory
    void dumpMemory();
    
    // opcode fetching function
    unsigned short fetchOpcode();
    
    // decode a opcode
    void decode(unsigned short opcode);
    
    // execute next cycle on specified memory
    void cycle();
    
    // reset the cpu
    void reset();
};

#endif /* cpu_hpp */
