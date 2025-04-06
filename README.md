# ♟️🐒 Chimp

Chimp is a UCI-compatible chess engine following a depth-first search approach.

Meaning, given a chess position (the root), it generates a move tree and explores as far as possible along each branch before backtracking. The engine searches for the best move to play amongst the root's children, using optimization techniques to efficiently prune branches that are unlikely to lead to favorable outcomes.


# Features

### Search
- Negamax Search
- Alpha-beta Pruning
- Iterative Deepening
- Move Ordering
  - Transposition Table
  - MVV-LVA for capture moves
  - Killers for quiet moves
- Quiescence Search
- Mate Distance Pruning
- Principal Variation Search

### Evaluation
- Piece-Square Tables
- Game Phase Interpolation

# Building

Chimp requires the following tools to build and run:

- **Compiler**: `clang++` (version >=20). Note: `g++` is currently not supported.  
- **Build System**: `make`.

To build Chimp, you can run the following commands:

```bash
git clone https://github.com/fsoonaye/chimp.git
cd chimp
make
./engine
```

# UCI Instructions

Chimp supports the following UCI (Universal Chess Interface) commands:

```
uci  
isready  
ucinewgame
quit  
stop  
  
position startpos
position fen <fen-string> moves <move1> <move2> ...    
go depth <>  
go nodes <>  
go perft <>  
go wtime <> btime <> winc <> binc <> movestogo <>
go movetime <>    
go mate <>
eval  
```

Further explanations of these commands can be found in `uci.h` and on the internet, for instance [here](https://wbec-ridderkerk.nl/html/UCIProtocol.html) or in the official [stockfish documentation](https://official-stockfish.github.io/docs/stockfish-wiki/UCI-&-Commands.html).


# History

This chess engine is the culmination of my ongoing exploration into the rich and complex niche that is chess programming.

In its current state, Chimp has, for now, absolutely no pretentions of revolutionizing the field of chess programming. Instead, its purpose is to serve as a personal milestone: a proof of my journey and dedication to grasp the fundamentals of this fascinating domain. 

I have only implemented techniques and concepts that I *believe* to understand, one at a time, prioritizing clarity and simplicity. In many ways, this repository embodies the kind of resource I wish I'd had as an entry point.

This project also served as a playground for improving my C++ skills and documenting what I learned. Over time, I’ve built a significant collection of notes. One day, I might turn these notes into a guide to chess programming.  

One thing I *definitely* plan to write soon is a comprehensive guide to **chess engine testing**. Having all this information gathered in one place and thoroughly explained would have saved me so much time!


# Resources  

While the guides I plan to write are still in progress, here’s a list of useful resources I’ve used or depended on to deepen my knowledge.

### Useful Tools: 
- [OpenBench](https://github.com/fsoonaye/OpenBench): a Distributed SPRT framework.
- [CuteChess](https://cutechess.com/): a GUI and cli testing tool for engine matches (also the more recent [FastChess](https://github.com/Disservin/fastchess?tab=readme-ov-file)).
- [martinnovaak/enginetest](https://github.com/martinnovaak/enginetest): tests your engine against Lichess puzzles.
- [TerjeKir/EngineTests](https://github.com/TerjeKir/EngineTests): tests your engine against "mate in x" problems, also has a speed-up comparison tool.  

### Wikis:
- [Chess Programming Wiki](https://www.chessprogramming.org/Main_Page): the ultimate reference for everything related to chess engine programming.  
  - [Connorpasta](https://www.chessprogramming.org/Search_Progression): a practical progression guide for search implementation.
  - [PeSTO](https://www.chessprogramming.org/PeSTO%27s_Evaluation_Function): a reasonable evaluation function when NNUEs are beyond the scope of your engine.
- [Bruce Moreland's Programming Topics](https://web.archive.org/web/20070707012511/http://www.brucemo.com/compchess/programming/index.htm): an older yet remarkably pedagogical resource.
- [The art of Chess Programming in Rust](https://rustic-chess.org/front_matter/about_book.html): a more modern, well-documented journey into creating a chess engine.


# Acknowledgements and Credit 

I have annotated concepts and arrays in my source code to properly credit the sources where I’ve borrowed ideas or implementations directly from other engines. However, if I’ve missed an attribution, I encourage the authors of these works to reach out for additional recognition or to request removal, if necessary.

### Engines that have been notable sources of inspiration:
- [SmallBrain](https://github.com/Disservin/Smallbrain)  
- [Rice](https://github.com/rafid-dev/rice/tree/main) 
- [Weiss](https://github.com/cutechess/weiss)
- [Stormphrax](https://github.com/Ciekce/Stormphrax)
- [Stockfish](https://github.com/official-stockfish/Stockfish): the strongest open-source chess engine in the world.
- [Vice](https://github.com/bluefeversoft/vice): I have not looked at it for quite some time but I had to give credit.
  

### Special thanks to:  
- The [Stockfish Discord Server](https://discord.gg/GWDRS3kU6R) and mainly the [Engine Programming Discord Server](https://discord.com/invite/F6W6mMsTGN), whose active members answered countless questions and helped me progress on this journey.  
- **[Disservin](https://github.com/Disservin)**, the author of [SmallBrain](https://github.com/Disservin/Smallbrain), for the brilliant **[Chess Library in C++](https://github.com/Disservin/chess-library)** used in Chimp (primarily for move generation and board data structures).
- **[Andrew Grant](https://github.com/AndyGrant)**, the author of [OpenBench](https://github.com/fsoonaye/OpenBench), for his great framework and his availability on the [OpenBench Discord server](https://discord.com/invite/9MVg7fBTpM).
