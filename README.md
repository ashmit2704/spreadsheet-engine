# Spreadsheet Engine (C++)

A spreadsheet computation engine built from scratch in **C++**, focusing on
formula parsing, dependency graphs, topological recalculation, and error handling.

This project implements the **core logic** behind spreadsheet applications
(Excel / Google Sheets) without a UI.


## Features

- Cell addressing (`A1`, `AA10`, etc.)
- Formula parsing with operator precedence and parentheses
- Arithmetic expressions (`+`, `-`, `*`, `/`)
- Cell references inside formulas
- Dependency graph between cells
- Topological recalculation (efficient updates)
- Circular dependency detection
- Error propagation and recovery
- Memory-safe AST using `std::unique_ptr`
- Unit tests for correctness verification


## Project Structure

```
spreadsheet-engine/
│
├── cell_id.h
├── lexer.h
├── ast.h
├── parser.h
├── cell.h
│
├── spreadsheet.h
├── spreadsheet.cpp
│
├── tests.cpp
└── README.md
```

## Building and Running Locally

### Requirements
- C++17 compatible compiler
- g++

### Compile

```
g++ -std=c++17 spreadsheet.cpp tests.cpp -o tests
```
### Run Tests

```
./tests
```
### Expected Output
```
All tests passed
```
### Testing Strategy

The project includes unit tests that verify:
Literal cell values
Formula evaluation
Operator precedence and parentheses
Dependency propagation
Diamond dependency graphs
Circular dependency detection
Error propagation through dependent cells
Error recovery when inputs are corrected

Tests are located in tests.cpp and can be run locally using the steps above.

## Architecture Diagram
```
                   +------------------+
                   |    User Input    |
                   |   ("=A1+B1*2")   |
                   +--------+---------+
                            |
                            v
                   +------------------+
                   |      Lexer       |
                   |  (Tokenization)  |
                   +--------+---------+
                            |
                            v
                   +------------------+
                   |      Parser      |
                   | (Recursive Desc) |
                   +--------+---------+
                            |
                            v
                   +------------------+
                   |       AST        |
                   | (Expression Tree)|
                   +--------+---------+
                            |
            +---------------+----------------+
            |                                |
            v                                v
+----------------------+        +----------------------+
| Dependency Extractor |        |      Evaluator       |
|  (Cell References)   |        |   (Compute Value)    |
+----------+-----------+        +----------+-----------+
           |                                 |
           v                                 |
+----------------------+                     |
|  Dependency Graph    |<--------------------+
|  (Cells as Nodes)    |
+----------+-----------+
           |
           v
+----------------------+
|  Topological Sort    |
|   (Correct Order)    |
+----------+-----------+
           |
           v
+----------------------+
| Recalculation Engine |
| (Propagate Updates)  |
+----------------------+
```

## Example Usage
```
Spreadsheet sheet;

sheet.setCell(parseCellID("A1"), "10");
sheet.setCell(parseCellID("B1"), "20");
sheet.setCell(parseCellID("C1"), "=A1+B1*2");

double result = sheet.getCellValue(parseCellID("C1")); // 50
```

## Future Work
+ Future Work
+ Formula functions (SUM, AVG, IF)
+ Undo / Redo support
+ Persistence / serialization
+ UI layer (web-based)
