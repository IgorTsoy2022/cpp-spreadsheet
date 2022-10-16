#include "cell.h"
#include "sheet.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>
#include <unordered_set>

//---------------------------------------------------------------
// class Cell public
//---------------------------------------------------------------
Cell::Cell(Sheet& sheet)
    : impl_(std::make_unique<EmptyImpl>()), sheet_(sheet)
{}

void Cell::Set(std::string text) {
    std::unique_ptr<Impl> impl;
    if (text.empty()) {
        impl = std::make_unique<EmptyImpl>();
    }
    if (text.size() > 1 && text.at(0) == '=') {
        impl = std::make_unique<FormulaImpl>(std::move(text),
                                             sheet_);
    }
    else {
        impl = std::make_unique<TextImpl>(text);
    }

    if (HasCircularReferences(*impl)) {
        throw CircularDependencyException("Circular References");
    }

    impl_ = std::move(impl);

    UpdateReferences();
}

void Cell::Clear() {
    impl_ = std::make_unique<EmptyImpl>();
}

Cell::Value Cell::GetValue() const {
    return impl_->GetValue();
}

std::string Cell::GetText() const {
    return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
    if (impl_ == nullptr)
    {
        return {};
    }
    return impl_->GetReferencedCells();
}

bool Cell::IsReferenced() const {
    return !prev_cells_.empty();
}

Cell::~Cell() {}

//---------------------------------------------------------------
// class Cell private
//---------------------------------------------------------------

    //-----------------------------------------------------------
    // class EmptyImpl
    //-----------------------------------------------------------
Cell::EmptyImpl::EmptyImpl() {}

Cell::Value Cell::EmptyImpl::GetValue() const {
    return { "" };
}

std::string Cell::EmptyImpl::GetText() const {
    return "";
}

    //-----------------------------------------------------------
    // class TextImpl
    //-----------------------------------------------------------
Cell::TextImpl::TextImpl(std::string text)
    : text_(text)
{}

std::string Cell::TextImpl::GetText() const {
    return text_;
}

Cell::Value Cell::TextImpl::GetValue() const {
    if (!text_.empty() && text_[0] == '\'') {
        return { text_.substr(1) };
    }
    return { text_ };
}

    //-----------------------------------------------------------
    // class FormulaImpl
    //-----------------------------------------------------------
Cell::FormulaImpl::FormulaImpl(std::string text,
                               Sheet& sheet)
    : sheet_(sheet)
{
    if (!text.empty() && text[0] == '=') {
        formula_ = std::move(ParseFormula(text.substr(1)));
    }
    else {
        throw FormulaException("Invalid formula");
    }
}

std::string Cell::FormulaImpl::GetText() const {
    return "=" + formula_->GetExpression();
}

Cell::Value Cell::FormulaImpl::GetValue() const {
    FormulaInterface::Value val = formula_->Evaluate(sheet_);
    if (std::holds_alternative<double>(val)) {
        return { std::get<double>(val) };
    }
    else {
        return { std::get<FormulaError>(val) };
    }
}

std::vector<Position> 
Cell::FormulaImpl::GetReferencedCells() const {
    return formula_->GetReferencedCells();
}

//---------------------------------------------------------------
// class Cell private
//---------------------------------------------------------------
bool Cell::HasCircularReferences(Impl& impl) {
    if (impl.GetReferencedCells().empty()) {
        return false;
    }

    std::unordered_set<Cell*> references;
    for (auto& pos : impl.GetReferencedCells()) {
        references.insert(sheet_.GetSheetCell(pos));
    }

    std::unordered_set<Cell*> visited;
    std::vector<Cell*> cells;
    cells.push_back(this);
    while (!cells.empty()) {
        Cell* current = cells.back();
        cells.pop_back();
        visited.insert(current);
        if (references.find(current) != references.end()) {
            return true;
        }
        for (Cell* prev_cell : current->prev_cells_) {
            if (visited.find(prev_cell) == visited.end()) {
                cells.push_back(prev_cell);
            }
        }
    }

    return false;
}

void Cell::UpdateReferences() {
    for (Cell* next_cell : next_cells_) {
        next_cell->prev_cells_.erase(this);
    }

    next_cells_.clear();
    for (const auto& pos : impl_->GetReferencedCells()) {
        Cell* next_cell = sheet_.GetSheetCell(pos);
        if (!next_cell) {
            sheet_.SetCell(pos, "");
            next_cell = sheet_.GetSheetCell(pos);
        }
        next_cells_.insert(next_cell);
        next_cell->prev_cells_.insert(this);
    }
}