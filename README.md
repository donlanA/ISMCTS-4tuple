## Playing
### move direction:
N: north
E: east
W: west
S: south

## Compile
g++ -std=c++11 -O2 src/gst.cpp src/ismcts.cpp src/mcts.cpp src/node.cpp -o game

./game

## Endgame Mode
To compile and play from a specific board position:

```
g++ -std=c++11 -O2 src/gst-endgame.cpp src/ismcts.cpp src/mcts.cpp src/node.cpp -o endgame
./endgame
```

Choose option 2 to load a custom board position.
Input the positions of pieces according to the prompts.

## Code settings
### pieces:
 0   h   g   f   e   0
 0   d   c   b   a   0
 0   0   0   0   0   0
 0   0   0   0   0   0
 0   A   B   C   D   0
 0   E   F   G   H   0

### board position:
 0   1   2   3   4   5
 6   7   8   9  10  11
12  13  14  15  16  17
18  19  20  21  22  23
24  25  26  27  28  29
30  31  32  33  34  35

-1: dead

this program(x,y) = (pos/row, pos%col)
GPW used(x,y) = (pos%row, pos/col)

### move direction:
0: up
1: left
2: right
3: down

   0
 1 P 2
   3


### computer view:
color board: |1|:red |2|:blue
 0  -2  -2  -2  -1   0
 0  -1  -1  -1  -2   0
 0   0   0   0   0   0
 0   0   0   0   0   0
 0   1   1   1   2   0
 0   2   2   2   1   0
### piece board:
-1  15  14  13  12  -1
-1  11  10   9   8  -1
-1  -1  -1  -1  -1  -1
-1  -1  -1  -1  -1  -1
-1   0   1   2   3  -1
-1   4   5   6   7  -1

### final user view:
 <   h   g   f   e   >
 -   d   c   b   a   - 
 -   -   -   -   -   -
 -   -   -   -   -   -
 -   A   B   C   D   -
 -   E   F   G   H   -

 有動 ismcts.cpp ismcts.hpp gst-endgame.cpp