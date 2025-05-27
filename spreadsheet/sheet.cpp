#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

Sheet::~Sheet() = default;

void Sheet::SetCell(Position pos, std::string text)
{
    if (!pos.IsValid())
    {
        throw InvalidPositionException("Invalid position");
    }

    const auto it = table_.find(pos);
    if (it == table_.end())
    {
        table_[pos] = std::make_unique<Cell>(*this);
    }

    table_.at(pos)->Set(std::move(text));
}

const CellInterface *Sheet::GetCell(Position pos) const
{
    if (!pos.IsValid())
    {
        throw InvalidPositionException("Invalid position");
    }

    const auto &cell = table_.find(pos);
    if (cell != table_.end())
    {
        return cell->second.get();
    }

    return nullptr;
}

CellInterface *Sheet::GetCell(Position pos)
{
    if (!pos.IsValid())
    {
        throw InvalidPositionException("Invalid position");
    }

    const auto &cell = table_.find(pos);
    if (cell != table_.end())
    {
        return cell->second.get();
    }

    return nullptr;
}

void Sheet::ClearCell(Position pos)
{
    if (!pos.IsValid())
    {
        throw InvalidPositionException("Invalid position");
    }

    table_.erase(pos);
}

Size Sheet::GetPrintableSize() const
{
    Size result;
    for (auto it = table_.begin(); it != table_.end(); ++it)
    {
        if (it->second != nullptr)
        {
            const int col = it->first.col;
            const int row = it->first.row;
            result.cols = std::max(col + 1, result.cols);
            result.rows = std::max(row + 1, result.rows);
        }
    }

    return result;
}

void Sheet::PrintValues(std::ostream &output) const
{
    Size table_size = GetPrintableSize();
    for (int row = 0; row < table_size.rows; ++row)
    {
        for (int col = 0; col < table_size.cols; ++col)
        {
            if (col > 0)
            {
                output << '\t';
            }

            const auto it = table_.find({row, col});
            if (it != table_.end() && it->second != nullptr)
            {
                std::visit([&output](const auto value)
                           { output << value; }, it->second->GetValue());
            }
        }

        output << '\n';
    }
}

void Sheet::PrintTexts(std::ostream &output) const
{
    Size table_size = GetPrintableSize();
    for (int row = 0; row < table_size.rows; ++row)
    {
        for (int col = 0; col < table_size.cols; ++col)
        {
            if (col > 0)
            {
                output << '\t';
            }

            const auto it = table_.find({row, col});
            if (it != table_.end() && it->second != nullptr)
            {
                output << it->second->GetText();
            }
        }

        output << '\n';
    }
}

std::unique_ptr<SheetInterface> CreateSheet()
{
    return std::make_unique<Sheet>();
}