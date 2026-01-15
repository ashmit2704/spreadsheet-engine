#pragma once
#include <unordered_map>
#include "cell.h"

using namespace std;

class Spreadsheet {
public:
    void setCell(const CellID& id, const string& input);
    double getCellValue(const CellID& id);
    bool hasError(const CellID& id);

private:
    unordered_map<CellID, Cell, CellIDHash> cells;

    void recalculateFrom(const CellID& start);
};
