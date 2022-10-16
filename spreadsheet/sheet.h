#pragma once

#include "cell.h"
#include "common.h"

#include <functional>
#include <unordered_map>

const size_t PRIME_NUMBER = 37;

struct PositionHasher {
    size_t operator()(const Position pos) const {
        return pos.row + pos.col * PRIME_NUMBER;
    }
};

class Sheet : public SheetInterface {
public:
    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

    Cell* GetSheetCell(Position pos);

    ~Sheet();

private:
    void PrintResize();

private:
    Size printable_size_ = {0,0};
    std::unordered_map<Position, std::unique_ptr<Cell>,
                       PositionHasher> sheet_;
};