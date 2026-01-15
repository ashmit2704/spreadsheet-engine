#pragma once
#include <memory>
#include <unordered_set>
#include "ast.h"
#include "cell_id.h"

using namespace std;

enum class CellError {
    NONE,
    DIV_ZERO,
    REF,
    VALUE,
    CYCLE
};

struct Cell {
    string rawInput;
    double value = 0.0;

    bool hasError = false;
    CellError error = CellError::NONE;

    unique_ptr<ASTNode> ast;

    unordered_set<CellID, CellIDHash> dependencies;
    unordered_set<CellID, CellIDHash> dependents;
};
