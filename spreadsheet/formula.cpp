#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream &operator<<(std::ostream &output, FormulaError fe)
{
    return output << "#ARITHM!";
}

namespace
{
    class Formula : public FormulaInterface
    {
    public:
        explicit Formula(std::string expression)
        try
            : ast_(ParseFormulaAST(expression))
        {
        }
        catch (...)
        {
            throw FormulaException("Invalid formula");
        }

        Value Evaluate(const SheetInterface &sheet) const override
        {
            try
            {
                return ast_.Execute(sheet);
            }
            catch (const FormulaError &formula_error)
            {
                return formula_error;
            }
        }

        std::string GetExpression() const override
        {
            std::stringstream ss;
            ast_.PrintFormula(ss);
            return ss.str();
        }

        std::vector<Position> GetReferencedCells() const override
        {
            std::vector<Position> result;
            for (const Position &cell : ast_.GetCells())
            {
                result.push_back(cell);
            }

            result.erase(std::unique(result.begin(), result.end()), result.end());
            return result;
        }

    private:
        FormulaAST ast_;
    };
} // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression)
{
    return std::make_unique<Formula>(std::move(expression));
}