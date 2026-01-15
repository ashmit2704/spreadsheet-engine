#pragma once
#include <string>
#include <unordered_set>

using namespace std;

struct CellID {
    int row;
    int col;

    CellID(int r = 0, int c = 0) : row(r), col(c) {}
    bool operator==(const CellID& other) const {
        return row == other.row && col == other.col;
    }
};

struct CellIDHash {
    size_t operator()(const CellID& id) const {
        return hash<int>()(id.row) ^ (hash<int>()(id.col) << 1);
    }
};

int colLettersToIdx(const string& letters);
string colIdxToLetters(int col);
CellID parseCellID(const string& ref);
string cellIDToStr(const CellID& id);
