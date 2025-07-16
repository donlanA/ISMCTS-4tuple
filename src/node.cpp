#include "node.hpp"

// Node 實現
Node::Node(GST state, int move) : 
    state(state), 
    move(move), 
    wins(0), 
    visits(0), 
    parent(nullptr) 
{}
