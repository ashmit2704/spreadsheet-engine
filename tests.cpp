#include "spreadsheet.h"
#include "cell_id.h"
#include<cassert>
#include<iostream>

using namespace std;

#define TEST(name) \
    cout << "Running " << name << "...\n";

#define PASS() \
    cout << "[X] Passed\n\n";


void test_literals() {
    TEST("Literal values")

    Spreadsheet sheet;
    sheet.setCell(parseCellID("A1"), "10");
    sheet.setCell(parseCellID("B1"), "20");

    assert(sheet.getCellValue(parseCellID("A1")) == 10);
    assert(sheet.getCellValue(parseCellID("B1")) == 20);

    PASS();
}


void test_simple_formula() {
    TEST("Simple formula")

    Spreadsheet sheet;
    sheet.setCell(parseCellID("A1"), "10");
    sheet.setCell(parseCellID("B1"), "20");
    sheet.setCell(parseCellID("C1"), "=A1+B1");

    assert(sheet.getCellValue(parseCellID("C1")) == 30);

    PASS();
}


void test_operator_precedence() {
    TEST("Operator precedence")

    Spreadsheet sheet;
    sheet.setCell(parseCellID("A1"), "10");
    sheet.setCell(parseCellID("B1"), "5");
    sheet.setCell(parseCellID("C1"), "=A1+B1*2");

    assert(sheet.getCellValue(parseCellID("C1")) == 20);

    PASS();
}


void test_parentheses() {
    TEST("Parentheses")

    Spreadsheet sheet;
    sheet.setCell(parseCellID("A1"), "10");
    sheet.setCell(parseCellID("B1"), "5");
    sheet.setCell(parseCellID("C1"), "=(A1+B1)*2");

    assert(sheet.getCellValue(parseCellID("C1")) == 30);

    PASS();
}


void test_dependency_chain() {
    TEST("Dependency chain")

    Spreadsheet sheet;
    sheet.setCell(parseCellID("A1"), "10");
    sheet.setCell(parseCellID("B1"), "=A1+5");
    sheet.setCell(parseCellID("C1"), "=B1*2");

    assert(sheet.getCellValue(parseCellID("C1")) == 30);

    sheet.setCell(parseCellID("A1"), "20");
    assert(sheet.getCellValue(parseCellID("C1")) == 50);

    PASS();
}


void test_diamond_dependency() {
    TEST("Diamond dependency")

    Spreadsheet sheet;
    sheet.setCell(parseCellID("A1"), "10");
    sheet.setCell(parseCellID("B1"), "=A1+5");
    sheet.setCell(parseCellID("C1"), "=A1*2");
    sheet.setCell(parseCellID("D1"), "=B1+C1");

    assert(sheet.getCellValue(parseCellID("D1")) == 35);

    sheet.setCell(parseCellID("A1"), "20");
    assert(sheet.getCellValue(parseCellID("D1")) == 65);

    PASS();
}


void test_div_zero() {
    TEST("Division by zero")

    Spreadsheet sheet;
    sheet.setCell(parseCellID("A1"), "10");
    sheet.setCell(parseCellID("B1"), "0");
    sheet.setCell(parseCellID("C1"), "=A1/B1");

    assert(sheet.hasError(parseCellID("C1")));

    PASS();
}


void test_cycle_detection() {
    TEST("Cycle detection")

    Spreadsheet sheet;
    sheet.setCell(parseCellID("A1"), "=B1+1");
    sheet.setCell(parseCellID("B1"), "=A1+1");

    assert(sheet.hasError(parseCellID("B1")));

    PASS();
}


void test_error_propagation_chain() {
    TEST("Error propagation through chain")

    Spreadsheet sheet;

    sheet.setCell(parseCellID("A1"), "10");
    sheet.setCell(parseCellID("B1"), "0");
    sheet.setCell(parseCellID("C1"), "=A1/B1");  // DIV_ZERO
    sheet.setCell(parseCellID("D1"), "=C1+5");
    sheet.setCell(parseCellID("E1"), "=D1*2");

    assert(sheet.hasError(parseCellID("C1")));
    assert(sheet.hasError(parseCellID("D1")));
    assert(sheet.hasError(parseCellID("E1")));

    PASS();
}


void test_error_propagation_diamond() {
    TEST("Error propagation diamond")

    Spreadsheet sheet;

    sheet.setCell(parseCellID("A1"), "0");
    sheet.setCell(parseCellID("B1"), "=10/A1");  // error
    sheet.setCell(parseCellID("C1"), "=A1+5");   // ok
    sheet.setCell(parseCellID("D1"), "=B1+C1");  // error via B1

    assert(sheet.hasError(parseCellID("B1")));
    assert(!sheet.hasError(parseCellID("C1")));
    assert(sheet.hasError(parseCellID("D1")));

    PASS();
}


void test_error_recovery() {
    TEST("Error recovery")

    Spreadsheet sheet;

    sheet.setCell(parseCellID("A1"), "0");
    sheet.setCell(parseCellID("B1"), "=10/A1");
    assert(sheet.hasError(parseCellID("B1")));

    // Fix the root cause
    sheet.setCell(parseCellID("A1"), "2");

    assert(!sheet.hasError(parseCellID("B1")));
    assert(sheet.getCellValue(parseCellID("B1")) == 5);

    PASS();
}


int main() {
    test_literals();
    test_simple_formula();
    test_operator_precedence();
    test_parentheses();
    test_dependency_chain();
    test_diamond_dependency();
    test_div_zero();
    test_cycle_detection();
    test_error_propagation_chain();
    test_error_propagation_diamond();
    test_error_recovery();

    cout << "All tests passed.\n";
    return 0;
}
