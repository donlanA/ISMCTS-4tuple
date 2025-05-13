## Description
1. 4-tuple Server版
2. 把server資料夾放在4-tuple資料夾下編譯即可使用
3. 要把使用的data移進去server下面

## 資料夾架構示意：
> 4-tuple_1.3.7
  |> server (在這裡編譯)
    |> data
    |main.cpp
    |MyAI.cpp
    |MyAI.h
    |Tomorin.exe
  |DATA_impl.cpp
  |GST_impl.cpp
  |其他原本的檔案(pcg, main, header.h...)

## Modification
1. 修改成直接打包的模式
2. 把重複定義的define拿掉

## Playing
### move direction:
N: north
E: east
W: west
S: south

## Compile
g++ -o Tomorin main.cpp MyAI.cpp ../4T_GST_impl.cpp ../4T_DATA_impl.cpp -std=c++11

g++ -o Tomorin_ismcts main.cpp MyAI.cpp ../4T_GST_impl.cpp ../4T_DATA_impl.cpp ../ismcts.cpp ../node.cpp -std=c++11

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
 0   2   1   1   1   0
 0   1   2   2   2   0
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
 <   E   F   G   H   >