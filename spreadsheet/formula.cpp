#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << fe.ToString();
}

struct GetDoubleValue{
    double operator()(const std::string& value){
        double ans = 0;

        if (!value.empty()) {
            char* end;
            double d = std::strtod(value.c_str(), &end);
            (*end == '\0') ? ans = d : throw FormulaError(FormulaError::Category::Value);
        }

        return ans;
    }

    double operator()(double value){
        return value;
    }

    double operator()(FormulaError value){
        throw FormulaError(value);
    }
};


namespace {
class Formula : public FormulaInterface {
public:
    explicit Formula(std::string expression) try
        : ast_(ParseFormulaAST(std::move(expression))) {
    } catch (const std::exception& exc) {
        throw FormulaException(exc.what());    
    }

    Value Evaluate(const SheetInterface& sheet) const override {
        try{
            auto func_check_value_cell = [&sheet](const Position& position) -> double{
                if(!position.IsValid()){
                    throw FormulaError(FormulaError::Category::Ref);
                }

                const auto* cell = sheet.GetCell(position);

                return (cell) ? std::visit(GetDoubleValue(), cell->GetValue()) : 0.0;
            };
        
            return ast_.Execute(func_check_value_cell);
            
        }catch(FormulaError& excp){
            return excp;
        }
    }

    std::string GetExpression() const override {
        std::stringstream ss;
        ast_.PrintFormula(ss); 
        return ss.str();
    }

    std::vector<Position> GetReferencedCells() const override{
        std::vector<Position> cells;

        for (auto cell : ast_.GetCells()) {
            if (cell.IsValid()) {
                cells.push_back(cell);
            }
        }

        cells.erase(std::unique(cells.begin(), cells.end()), cells.end());
        return cells;
    }

private:
    FormulaAST ast_;
};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}