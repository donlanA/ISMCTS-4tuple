### 目標

合併ISMCTS 4tuple

讓 ISMCTS 在模擬時不要完全隨機走步，而是根據 4tuple 選擇，以提升模擬成效

### 合併思路

1. (完成) 確認 gst 程式可以讓 4tuple 正常運作 (可以連接 server)

2. 確認 gst 程式可以讓 ismcts 正常運作 (可以連接 server)

3. 修改 ISMCTS.cpp 當中 simulation 的部分，讓 ISMCTS 考慮 4tuple

4. 修改 gst-endgame，測試是否能在殘局有更好的表現 (gst-endgame 與 gst 只差在 init_board 的部分)

### 進度

ISMCTS、4tuple 各自都可以運作

gst、gst-endgame目前都是讓mcts與ismcts對打，需要修改成連接server

應該會合併 ISMCTS 跟 4tuple 的 GST

## Playing
### move direction:
N: north
E: east
W: west
S: south

## Compile (gst)
g++ -std=c++11 -O2 src/gst.cpp src/ismcts.cpp src/mcts.cpp src/node.cpp -o game
./game

## Endgame Compile (gst-endgame)
g++ -std=c++11 -O2 src/gst-endgame.cpp src/ismcts.cpp src/mcts.cpp src/node.cpp -o endgame
./endgame


## Code settings


### pieces:
```
 0   h   g   f   e   0
 0   d   c   b   a   0
 0   0   0   0   0   0
 0   0   0   0   0   0
 0   A   B   C   D   0
 0   E   F   G   H   0
```
### board position:
```
 0   1   2   3   4   5
 6   7   8   9  10  11
12  13  14  15  16  17
18  19  20  21  22  23
24  25  26  27  28  29
30  31  32  33  34  35
```
-1: dead

this program(x,y) = (pos/row, pos%col)
GPW used(x,y) = (pos%row, pos/col)

### move direction:
0: up
1: left
2: right
3: down
```
   0
 1 P 2
   3
```

### computer view:
color board: |1|:red |2|:blue
```
 0  -2  -2  -2  -1   0
 0  -1  -1  -1  -2   0
 0   0   0   0   0   0
 0   0   0   0   0   0
 0   1   1   1   2   0
 0   2   2   2   1   0
```
### piece board:
```
-1  15  14  13  12  -1
-1  11  10   9   8  -1
-1  -1  -1  -1  -1  -1
-1  -1  -1  -1  -1  -1
-1   0   1   2   3  -1
-1   4   5   6   7  -1
```
### final user view:
```
 <   h   g   f   e   >
 -   d   c   b   a   - 
 -   -   -   -   -   -
 -   -   -   -   -   -
 -   A   B   C   D   -
 -   E   F   G   H   -
```
