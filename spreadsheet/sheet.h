#pragma once

#include "cell.h"
#include "common.h"

#include <functional>

struct PositionHasher
{
    size_t operator()(const Position &pos) const
    {
        size_t h1 = std::hash<int>{}(pos.row);
        size_t h2 = std::hash<int>{}(pos.col);
        return h1 ^ (h2 << 1) ^ (h1 >> 3);
    }
};

class Sheet : public SheetInterface
{
public:
    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface *GetCell(Position pos) const override;
    CellInterface *GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream &output) const override;
    void PrintTexts(std::ostream &output) const override;

private:
    std::unordered_map<Position, std::unique_ptr<Cell>, PositionHasher> table_;
};