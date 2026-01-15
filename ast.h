#pragma once
#include <memory>
#include "cell_id.h"

using namespace std;

struct ASTNode {
    enum Type { NUMBER, CELL, ADD, SUB, MUL, DIV } type;

    double numVal = 0.0;
    CellID cell;

    unique_ptr<ASTNode> left;
    unique_ptr<ASTNode> right;

    ASTNode(Type t) : type(t) {}
};
