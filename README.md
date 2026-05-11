# Evil Hangman

A modular implementation of the classic Evil Hangman game written in C.
    
Unlike traditional Hangman, Evil Hangman continuously changes the hidden word by maintaining the largest possible family of candidate words after each guess, making the game intentionally deceptive and significantly harder for the player.
    
This project was developed as part of a Computer Science curriculum and focuses heavily on custom data structures, dynamic memory management, abstraction, and algorithmic problem solving.

---

## Features

- Dynamic “Evil” word-family generation
- AVL tree associative array for partitioning word families
- Custom generic vector implementation
- Custom string ADT implementation
- File-driven dictionary loading
- Configurable word length and guess count
- Optional running total of remaining candidate words
- Real-time guessed-letter tracking
- Win/loss replay loop
- Modular multi-file project architecture

---

## Demo

### Example Gameplay

```
Enter the desired word length: 5
Enter the number of guesses: 13
Would you like to see the number of words remaining? (y/n): y

Remaining guesses: 13
Guessed letters:
Current word family: -----
Words remaining: 8548
Enter guess: a
Sorry, there are no a's

Remaining guesses: 12
Guessed letters: a
Current word family: -----
Words remaining: 5089
Enter guess: e
Sorry, there are no e's

Remaining guesses: 11
Guessed letters: ae
Current word family: -----
Words remaining: 2481
Enter guess: b
Sorry, there are no b's

Remaining guesses: 10
Guessed letters: aeb
Current word family: -----
Words remaining: 2173
Enter guess:
```

--- 

## How to Build / Run

### Requirements

- C compiler with C89/C90 support
- Microsoft Visual Studio 2022/2026
- GCC / Clang

No external libraries are required.

### Build Instructions (Visual Studio)

1. Clone the repository:
`git clone https://github.com/jasonbower/Evil-Hangman.git`
2. Open the .sln file in Visual Studio.
3. Build the project:
`Build -> Build Solution`
4. Run the executable.

### Build Instructions (GCC)

```
gcc *.c -o evil_hangman
./evil_hangman
```

---

## Usage

When the program starts, the player is prompted for:

- Desired word length
- Number of guesses
- Whether to display the running total of remaining words

The game then repeatedly:

- Displays guessed letters
- Displays remaining guesses
- Accepts a letter guess
- Reorganizes candidate words into word families
- Selects the largest remaining family
- Updates the visible pattern

The player wins by revealing the full word before running out of guesses.

---

## Project Structure

```
Evil-Hangman/
│
├── main.c
├── my_string.c
├── my_string.h
├── generic_vector.c
├── generic_vector.h
├── associative_array.c
├── associative_array.h
├── status.h
├── dictionary.txt
└── README.md
```

### Important Components

- `main.c`
    - Main gameplay loop and game control logic
- `my_string.*`
    - Custom string ADT implementation
- `generic_vector.*`
    - Resizable generic vector implementation
- `associative_array.*`
    - AVL-tree-based associative array used for word family partitioning
- `dictionary.txt`
    - Source dictionary used to generate candidate words

---

## Concepts Used / What I Learned

This project helped reinforce several core Computer Science concepts, including:

- Dynamic memory management
- Abstract Data Types (ADTs)
- Generic programming in C
- AVL trees and self-balancing binary search trees
- Recursive algorithms
- File I/O
- Pointers and opaque object handling
- Modular software design
- Algorithmic problem solving
- Runtime efficiency considerations
- Defensive programming and error handling

---

## Technical Highlights

- Implemented a self-balancing AVL tree from scratch
- Designed generic reusable vector and string ADTs
- Used opaque pointers to enforce abstraction boundaries
- Built recursive tree insertion and balancing logic
- Managed dynamic memory manually without external libraries

---

## Known Issues / Future Improvements

### Current Limitations

- Console-only interface
- Dictionary must be present in the executable directory
- Limited input sanitization for unusual user input

### Planned Improvements
- Additional code refactoring and cleanup
- Improved UI formatting
- Smarter tie-breaking strategies for word family selection
- Better replay statistics
- Optional colorized terminal output
- Cross-platform Makefile support
- Potential graphical version in the future

---

## Credits

Dictionary word list sourced from public English word datasets.

Project developed independently for educational purposes.

--- 

## License

This project is licensed under the MIT License.

You are free to use, modify, and distribute this project with proper attribution.

--- 

Author: Jason Bower

Project Started: 1/26/26
