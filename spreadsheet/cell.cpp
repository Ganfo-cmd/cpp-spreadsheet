#include "cell.h"

#include <cassert>
#include <iostream>
#include <queue>
#include <string>
#include <optional>

Cell::Cell(SheetInterface &sheet) :  sheet_(sheet), impl_(std::make_unique<EmptyImpl>()) {}

Cell::~Cell() = default;

void Cell::Set(std::string text)
{
    std::unique_ptr<Impl> local_impl;
    if (text.size() > 1 && text.at(0) == FORMULA_SIGN)
    {
        local_impl = std::make_unique<FormulaImpl>(text);

        const auto &referenced_cells = local_impl->GetReferencedCells();

        if (HasCircularDependencyException(referenced_cells))
        {
            throw CircularDependencyException("Circular Dependency Exception");
        }

        // обновление зависимостей
        UpdateDependencies(referenced_cells);

        // инвалидация кеша
        CacheInvalidation();
    }
    else
    {
        if (text.empty())
        {
            local_impl = std::make_unique<EmptyImpl>();
        }
        else
        {
            local_impl = std::make_unique<TextImpl>(text);
        }
    }

    impl_ = std::move(local_impl);
}

void Cell::Clear()
{
    impl_ = std::make_unique<EmptyImpl>();
}

Cell::Value Cell::GetValue() const
{
    return impl_->GetValue(sheet_);
}

std::string Cell::GetText() const
{
    return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const
{
    return impl_->GetReferencedCells();
}

bool Cell::IsReferenced() const
{
    return !depends_cells_.empty();
}

std::vector<Position> Cell::Impl::GetReferencedCells() const
{
    return {};
}

void Cell::Impl::ClearCache()
{
}

Cell::Value Cell::EmptyImpl::GetValue(const SheetInterface &sheet) const
{
    return "";
}

std::string Cell::EmptyImpl::GetText() const
{
    return "";
}

Cell::TextImpl::TextImpl(std::string text) : value_(std::move(text)) {}

Cell::Value Cell::TextImpl::GetValue(const SheetInterface &sheet) const
{
    if (value_.at(0) == ESCAPE_SIGN)
    {
        return value_.substr(1);
    }

    return value_;
}

std::string Cell::TextImpl::GetText() const
{
    return value_;
}

Cell::FormulaImpl::FormulaImpl(std::string text)
    : formula_(ParseFormula(text.substr(1))) {}

Cell::Value Cell::FormulaImpl::GetValue(const SheetInterface &sheet) const
{
    const auto &result = formula_->Evaluate(sheet);
    if (std::holds_alternative<double>(result))
    {
        return std::get<double>(result);
    }
    else
    {
        return std::get<FormulaError>(result);
    }
}

std::string Cell::FormulaImpl::GetText() const
{
    return FORMULA_SIGN + formula_->GetExpression();
}

std::vector<Position> Cell::FormulaImpl::GetReferencedCells() const
{
    return formula_->GetReferencedCells();
}

void Cell::FormulaImpl::ClearCache()
{
    cache_.reset();
}

void Cell::UpdateDependencies(const std::vector<Position> &referenced_cells)
{
    depends_cells_.clear();
    for (const auto &cell : referenced_cells)
    {
        Cell *it = static_cast<Cell *>(sheet_.GetCell(cell));
        if (it == nullptr)
        {
            sheet_.SetCell(cell, "");
            it = static_cast<Cell *>(sheet_.GetCell(cell));
        }

        depends_cells_.insert(it);
        it->dependent_cells_.insert(this);
    }
}

bool Cell::HasCircularDependencyException(const std::vector<Position> &referenced_cells) const
{
    if (referenced_cells.empty())
    {
        return false;
    }

    std::queue<const Cell *> queue;
    std::unordered_set<const Cell *> ref_set;
    for (const auto &cell : referenced_cells)
    {
        ref_set.insert(static_cast<Cell *>(sheet_.GetCell(cell)));
    }

    queue.push(this);
    while (!queue.empty())
    {
        const auto *current_cell = queue.front();
        queue.pop();

        if (this != current_cell)
        {
            ref_set = current_cell->depends_cells_;
        }

        if (ref_set.count(current_cell) > 0)
        {
            return true;
        }

        for (const auto *cell : ref_set)
        {
            if (cell != nullptr)
            {
                queue.push(cell);
            }
        }
    }

    return false;
}

void Cell::CacheInvalidation()
{
    for (const auto cell : dependent_cells_)
    {
        cell->impl_->ClearCache();
    }
}