#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>
#include <variant>

using namespace std::literals;

//---------------------------------------------------------------
// class Sheet
//---------------------------------------------------------------
void Sheet::SetCell(Position pos, std::string text) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid Position");
    }

    if (pos.row >= printable_size_.rows) {
        printable_size_.rows = pos.row + 1;
    }

    if (pos.col >= printable_size_.cols) {
        printable_size_.cols = pos.col + 1;
    }

    auto& cell = sheet_[pos];
    if (cell == nullptr) {
        cell = std::make_unique<Cell>(*this);
    }
    cell->Set(std::move(text));
}

const CellInterface* Sheet::GetCell(Position pos) const {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid Position");
    }

    if (sheet_.count(pos) > 0) {
        CellInterface* ptr = &(*sheet_.at(pos));
        return ptr;
    }

    return nullptr;
}

CellInterface* Sheet::GetCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid Position");
    }

    if (sheet_.count(pos) > 0) {
        CellInterface* ptr = &(*sheet_.at(pos));
        return ptr;
    }

    return nullptr;
}

void Sheet::ClearCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid Position");
    }

    if (sheet_.count(pos) > 0) {
        sheet_.at(pos) = nullptr;
    }

    PrintResize();
}

Size Sheet::GetPrintableSize() const {
    return printable_size_;
}

void Sheet::PrintValues(std::ostream& output) const {
    for (int i = 0; i < printable_size_.rows; ++i) {
        bool first_col = true;
        for (int j = 0; j < printable_size_.cols; ++j) {
            if (first_col) {
                first_col = false;
            }
            else {
                output << "\t";
            }

            Position pos = { i, j };
            if (sheet_.count(pos) > 0) {
                const auto val = (*(sheet_.at(pos))).GetValue();
                if (std::holds_alternative<double>(val)) {
                    output << std::get<double>(val);
                }
                else if (std::holds_alternative<std::string>(val)) {
                    output << std::get<std::string>(val);
                }
                else {
                    output << std::get<FormulaError>(val);
                }
            }
        }
        output << "\n";
    }
}

void Sheet::PrintTexts(std::ostream& output) const {
    for (int i = 0; i < printable_size_.rows; ++i) {
        bool first_col = true;
        for (int j = 0; j < printable_size_.cols; ++j) {
            if (first_col) {
                first_col = false;
            }
            else {
                output << "\t";
            }

            Position pos = { i, j };
            if (sheet_.count(pos) > 0) {
                output << (*sheet_.at(pos)).GetText();
            }
            else {
                output << "";
            }
        }
        output << "\n";
    }
}

Cell* Sheet::GetSheetCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid Position");
    }

    if (sheet_.count(pos) > 0) {
        return sheet_.at(pos).get();
    }

    return nullptr;
}

void Sheet::PrintResize() {
    int max_row = 0;
    int max_col = 0;

    for (const auto& [pos, _] : sheet_) {
        max_row = (max_row > pos.row) ? max_row : pos.row;
        max_col = (max_col > pos.col) ? max_col : pos.col;
    }

    printable_size_.rows = max_row + 1;
    printable_size_.cols = max_col + 1;
}

Sheet::~Sheet()
{}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}