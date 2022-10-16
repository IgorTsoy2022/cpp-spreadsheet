#include "formula.h"
#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, 
                         FormulaError fe) {
    return output << "#DIV/0!";
}

double GetDoubleValue(const std::string& str) {
    double value = 0;
    if (!str.empty()) {
        std::istringstream in(str);
        if (!(in >> value) || !in.eof()) {
            throw FormulaError(FormulaError::Category::Value);
        }
    }
    return value;
}

double GetDoubleValue(double value) {
    return value;
}

double GetDoubleValue(FormulaError error) {
    throw FormulaError(error);
}

double GetVariantValue(const CellInterface* cell) {
    if (cell == nullptr) return 0;
    return std::visit([](const auto& value) { 
            return GetDoubleValue(value);
        }, cell->GetValue());
}

namespace {

    //-----------------------------------------------------------
    // class Formula
    //-----------------------------------------------------------
    class Formula : public FormulaInterface {
    public:
        explicit Formula(std::string expression)
            try
            : ast_(ParseFormulaAST(expression))
        {}
        catch (const std::exception& exc) {
            throw FormulaException(exc.what());
        }

        Value Evaluate(const SheetInterface& 
                       sheet) const override {
            try {
                auto lambda = 
                    [&sheet](Position position) -> double {
                        if (!position.IsValid()) {
                            throw FormulaError(
                                FormulaError::Category::Ref);
                        }
                        const auto* cell = 
                            sheet.GetCell(position);
                        return GetVariantValue(cell);
                    };

                return ast_.Execute(lambda);
            }
            catch (const FormulaError& fe) {
                return fe;
            }
        }

        std::string GetExpression() const override {
            std::ostringstream out;
            ast_.PrintFormula(out);
            return out.str();
        }

        std::vector<Position> 
        GetReferencedCells() const override {
            std::vector<Position> cells;
            for (auto cell : ast_.GetCells()) {
                if (cell.IsValid()) {
                    cells.push_back(cell);
                }
            }
            auto it = std::unique(cells.begin(), cells.end());
            cells.resize(it - cells.begin());
            return cells;
        }

    private:
        FormulaAST ast_;
    };

} // namespace

std::unique_ptr<FormulaInterface> 
ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}