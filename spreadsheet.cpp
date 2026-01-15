#include "spreadsheet.h"
#include "parser.h"
#include "cell_id.h"

#include <queue>
#include <stdexcept>
#include <cctype>
#include <unordered_set>
#include <unordered_map>

using namespace std;


// CellID Helper Implementation
int colLettersToIdx(const string& letters) {
    int col = 0;
    for(char ch : letters) {
        if(!isupper(ch)) {
            throw invalid_argument("Invalid column letter");
        }
        col = col * 26 + (ch - 'A' + 1);
    }
    return col - 1;  // Converting to base 0
}


string colIdxToLetters(int col) {
    if(col < 0) 
        throw invalid_argument("Negative column index");
    
    string result;
    col += 1;  // Convert to 1-based
    while(col > 0) {
        int rem = (col - 1) % 26;
        result.insert(result.begin(), char('A' + rem));
        col = (col - 1) / 26;
    }
    return result;
}


CellID parseCellID(const string& ref) {
    if(ref.empty())
        throw invalid_argument("Empty    Reference");
    
    int i = 0;
    while(i < ref.size() && isupper(ref[i]))
        i++;
    
    if(i == 0 || i == ref.size()) 
        throw invalid_argument("Invalid cell reference format");

    string colPart = ref.substr(0, i);
    string rowPart = ref.substr(i);

    for(char c : rowPart) {
        if(!isdigit(c))
            throw invalid_argument("Invalid row number");
    }

    int col = colLettersToIdx(colPart);
    int row = stoi(rowPart) - 1;

    if(row < 0) 
        throw invalid_argument("Row index must be >= 1");

    return CellID(row, col);
}


string cellIDToStr(const CellID& id) {
    if(id.row < 0 || id.col < 0) 
        throw invalid_argument("Invalid CellID");

    string col = colIdxToLetters(id.col);
    string row = to_string(id.row + 1);

    return col + row;
}


// Lexer
Lexer::Lexer(const string& s) : input(s) {}


Token Lexer::nextToken() {
    while(pos < input.size() && isspace(input[pos])) pos++;

    if(pos >= input.size()) return {TokenType::END, ""};

    char c = input[pos];

    // For number
    if(isdigit(c)) {
        int start = pos;
        while(pos < input.size() && (isdigit(input[pos]) || input[pos] == '.'))
            pos++;
        return {TokenType::NUMBER, input.substr(start, pos - start)};
    }

    // Cell reference
    if(isupper(c)) {
        int start = pos;
        while (pos < input.size() && isupper(input[pos])) pos++;
        while (pos < input.size() && isdigit(input[pos])) pos++;
        return {TokenType::CELL, input.substr(start, pos - start)};
    }
    pos++;

    switch (c) {
        case '+': return {TokenType::PLUS, "+"};
        case '-': return {TokenType::MINUS, "-"};
        case '*': return {TokenType::MUL, "*"};
        case '/': return {TokenType::DIV, "/"};
        case '(': return {TokenType::LPAREN, "("};
        case ')': return {TokenType::RPAREN, ")"};
        default:
            throw runtime_error("Invalid character in formula");
    }
}


// Parser
Parser::Parser(const string& input) : lexer(input) {
    current = lexer.nextToken();
}


unique_ptr<ASTNode> Parser::parse() {
    return parseExpression();
}


unique_ptr<ASTNode> Parser::parseExpression() {
    auto node = parseTerm();

    while(current.type == TokenType::PLUS || current.type == TokenType::MINUS) {
        TokenType op = current.type;
        current = lexer.nextToken();

        auto right = parseTerm();
        auto parent = make_unique<ASTNode>(op == TokenType::PLUS ? ASTNode::ADD : ASTNode::SUB);
        parent->left = move(node);
        parent->right = move(right);
        node = move(parent);
    }
    return node;
}


unique_ptr<ASTNode> Parser::parseTerm() {
    auto node = parseFactor();

    while (current.type == TokenType::MUL || current.type == TokenType::DIV) {
        TokenType op = current.type;
        current = lexer.nextToken();

        auto right = parseFactor();
        auto parent = make_unique<ASTNode>(
            op == TokenType::MUL ? ASTNode::MUL : ASTNode::DIV
        );
        parent->left = move(node);
        parent->right = move(right);
        node = move(parent);
    }
    return node;
}


unique_ptr<ASTNode> Parser::parseFactor() {
    if (current.type == TokenType::NUMBER) {
        auto node = make_unique<ASTNode>(ASTNode::NUMBER);
        node->numVal = stod(current.text);
        current = lexer.nextToken();
        return node;
    }

    if (current.type == TokenType::CELL) {
        auto node = make_unique<ASTNode>(ASTNode::CELL);
        node->cell = parseCellID(current.text);
        current = lexer.nextToken();
        return node;
    }

    if (current.type == TokenType::LPAREN) {
        current = lexer.nextToken();
        auto node = parseExpression();
        if (current.type != TokenType::RPAREN)
            throw runtime_error("Missing )");
        current = lexer.nextToken();
        return node;
    }

    throw runtime_error("Invalid factor");
}


// AST Evaluation & Dependency
double evalAST(const ASTNode* node, Spreadsheet& sheet, bool& hasError) {
    if(!node)
        return 0;
    
    switch(node->type) {
        case ASTNode::NUMBER:
            return node->numVal;
        
        case ASTNode::CELL:
            if(sheet.hasError(node->cell)) {
                hasError = true;
                return 0;
            }
            return sheet.getCellValue(node->cell);
        
        case ASTNode::ADD: {
            double l = evalAST(node->left.get(), sheet, hasError);
            double r = evalAST(node->right.get(), sheet, hasError);
            return l + r;
        }

        case ASTNode::SUB: {
            double l = evalAST(node->left.get(), sheet, hasError);
            double r = evalAST(node->right.get(), sheet, hasError);
            return l - r;
        }

        case ASTNode::MUL: {
            double l = evalAST(node->left.get(), sheet, hasError);
            double r = evalAST(node->right.get(), sheet, hasError);
            return l * r;
        }

        case ASTNode::DIV: {
            double l = evalAST(node->left.get(), sheet, hasError);
            double r = evalAST(node->right.get(), sheet, hasError);
            if (r == 0) {
                hasError = true;
                return 0;
            }
            return l / r;
        }
    }
    return 0;
}


// Only to Inspect Structure
void extractDependencies(const ASTNode* node, unordered_set<CellID, CellIDHash>& deps) {
    if(!node)
        return;
    if(node->type == ASTNode::CELL) {
        deps.insert(node->cell);
    }
    extractDependencies(node->left.get(), deps);
    extractDependencies(node->right.get(), deps);
}


// Cycle Detection Helper
bool hasCycle(const CellID& current, const CellID& target, unordered_map<CellID, Cell, CellIDHash>& cells, unordered_set<CellID, CellIDHash>& visited) {
    if(current == target) return true;

    if(visited.count(current)) return false;

    visited.insert(current);

    if(!cells.count(current)) return false;

    for(const CellID& dep : cells[current].dependencies) {
        if(hasCycle(dep, target, cells, visited)) 
            return true;
    }

    return false;
}


bool createsCycle(const CellID& cell, const unordered_set<CellID, CellIDHash>& newDeps, unordered_map<CellID, Cell, CellIDHash>& cells) {
    for (const CellID& dep : newDeps) {
        unordered_set<CellID, CellIDHash> visited;
        if (hasCycle(dep, cell, cells, visited))
            return true;
    }
    return false;
}


// Topological Recalculation
void collectAffected(const CellID& start, unordered_map<CellID, Cell, CellIDHash>& cells, unordered_set<CellID, CellIDHash>& affected) {
    if(affected.count(start))
        return;
    affected.insert(start);

    for(const CellID& dep : cells[start].dependents) {
        collectAffected(dep, cells, affected);
    }
}


unordered_map<CellID, int, CellIDHash> buildIndegree(const unordered_set<CellID, CellIDHash>& affected, unordered_map<CellID, Cell, CellIDHash>& cells) {
    unordered_map<CellID, int, CellIDHash> indegree;

    for (const CellID& id : affected) {
        indegree[id] = 0;
    }

    for (const CellID& id : affected) {
        for (const CellID& dep : cells[id].dependencies) {
            if (affected.count(dep)) {
                indegree[id]++;
            }
        }
    }

    return indegree;
}


void Spreadsheet::recalculateFrom(const CellID& start) {
    unordered_set<CellID, CellIDHash> affected;
    collectAffected(start, cells, affected);

    auto indegree = buildIndegree(affected, cells);

    queue<CellID> q;
    for (auto& it : indegree) {
        auto id = it.first;
        auto deg = it.second;
        if (deg == 0) q.push(id);
    }

    while (!q.empty()) {
        CellID cur = q.front();
        q.pop();

        Cell& cell = cells[cur];

        if (cell.ast) {
            bool hasErr = false;
            double val = evalAST(cell.ast.get(), *this, hasErr);

            cell.hasError = hasErr;
            if (!hasErr) {
                cell.value = val;
            }
        }

        // Reduce indegree of dependents
        for (const CellID& dep : cell.dependents) {
            if (!indegree.count(dep)) continue;
            indegree[dep]--;
            if (indegree[dep] == 0) {
                q.push(dep);
            }
        }
    }
}


// Spreadsheet API
double Spreadsheet::getCellValue(const CellID& id) {
    if (!cells.count(id)) return 0.0;   // empty cell = 0
    if (cells[id].error != CellError::NONE) return 0.0;
    return cells[id].value;
}


bool Spreadsheet::hasError(const CellID& id) {
    if (!cells.count(id)) return false;
    return cells[id].hasError;
}


void Spreadsheet::setCell(const CellID& id, const string& input) {
    Cell& cell = cells[id];
    cell.rawInput = input;

    // Remove old dependencies
    for (const CellID& d : cell.dependencies)
        cells[d].dependents.erase(id);
    cell.dependencies.clear();

    if (!input.empty() && input[0] == '=') {
        Parser parser(input.substr(1));
        auto ast = parser.parse();

        unordered_set<CellID, CellIDHash> newDeps;
        extractDependencies(ast.get(), newDeps);

        if (createsCycle(id, newDeps, cells)) {
            cell.ast.reset();
            cell.dependencies.clear();
            cell.error = CellError::CYCLE;
            cell.hasError = true;
            return;
        }

        cell.ast = move(ast); 
        cell.dependencies = newDeps;

        for (const CellID& d : newDeps)
            cells[d].dependents.insert(id);

        recalculateFrom(id);
    } else {
        cell.value = stod(input);
        cell.hasError = false;
        recalculateFrom(id);
    }
}