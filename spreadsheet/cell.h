#pragma once

#include "common.h"
#include "formula.h"

#include <optional>
#include <functional>
#include <unordered_set>

class Sheet;

class Cell : public CellInterface
{
public:
    Cell(SheetInterface &sheet);
    ~Cell();

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;

    bool IsReferenced() const;

private:
    class Impl
    {
    public:
        virtual ~Impl() = default;

        virtual Value GetValue(const SheetInterface &sheet) const = 0;
        virtual std::string GetText() const = 0;

        virtual std::vector<Position> GetReferencedCells() const;
        virtual void ClearCache();
    };

    class EmptyImpl : public Impl
    {
    public:
        Value GetValue(const SheetInterface &sheet) const override;
        std::string GetText() const override;
    };

    class TextImpl : public Impl
    {
    public:
        explicit TextImpl(std::string text);

        Value GetValue(const SheetInterface &sheet) const override;
        std::string GetText() const override;

    private:
        std::string value_;
    };

    class FormulaImpl : public Impl
    {
    public:
        explicit FormulaImpl(std::string text);

        Value GetValue(const SheetInterface &sheet) const override;
        std::string GetText() const override;

        std::vector<Position> GetReferencedCells() const override;

        void ClearCache() override;

    private:
        std::unique_ptr<FormulaInterface> formula_;
        mutable std::optional<FormulaInterface::Value> cache_;
    };

    SheetInterface &sheet_;
    std::unique_ptr<Impl> impl_;

    std::unordered_set<const Cell *> depends_cells_;   // ячейки на которые ссылается данная ячейка (для поиска циклических зависимостей)
    std::unordered_set<const Cell *> dependent_cells_; // ячейки ссылающиеся на данную ячейку (для инвалидации кеша)

    void UpdateDependencies(const std::vector<Position> &referenced_cells);
    bool HasCircularDependencyException(const std::vector<Position> &referenced_cells) const;
    void CacheInvalidation();
};
