#include "node.hpp"

// Node implementation
Node::Node(GST state, int move) : 
    state(state), 
    move(move), 
    wins(0), 
    visits(0), 
    parent(nullptr) 
{}
