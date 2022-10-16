#pragma once

#include "FormulaLexer.h"
#include "common.h"

#include <forward_list>
#include <functional>
#include <stdexcept>

namespace ASTImpl {
    class Expr;
}

class ParsingError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

class FormulaAST {
public:
    explicit FormulaAST(std::unique_ptr<ASTImpl::Expr> root_expr,
        std::forward_list<Position> cells);
    FormulaAST(FormulaAST&&) = default;
    FormulaAST& operator=(FormulaAST&&) = default;

    void PrintCells(std::ostream& out) const;
    void Print(std::ostream& out) const;
    void PrintFormula(std::ostream& out) const;

    double Execute(std::function<double(Position)> cell) const;

    std::forward_list<Position>& GetCells() {
        return cells_;
    }

    const std::forward_list<Position>& GetCells() const {
        return cells_;
    }

    ~FormulaAST();

private:
    std::unique_ptr<ASTImpl::Expr> root_expr_;

    // physically stores cells so that they can be
    // efficiently traversed without going through
    // the whole AST
    std::forward_list<Position> cells_;
};

FormulaAST ParseFormulaAST(std::istream& in);
FormulaAST ParseFormulaAST(const std::string& in_str);