#include "cell.h"
#include "sheet.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>
#include <stack>

class Cell::Impl{
public:
    Impl() = default;
    virtual ~Impl() = default;

    virtual Value GetValue() const = 0;
    virtual std::string GetText() const = 0;

    virtual std::vector<Position> GetReferencedCells() const{
        return {};
    }

    virtual FormulaInterface* GetFormula() {
        return nullptr;
    }

    virtual bool IsCacheValid() const {
        return true;
    }

    virtual void InvalidateCache() {}
};

std::vector<Position> Cell::GetReferencedCells() const{
    return impl_->GetReferencedCells();
}

bool Cell::IsReferenced() const{
    return !cells_depend_on_this_.empty();
}

class Cell::EmptyImpl : public Impl{
public:
    Value GetValue() const override{
        return "";
    }

    std::string GetText() const override{
        return "";
    }
};

class Cell::TextImpl : public Impl{
public:
    TextImpl(std::string text) : text_(std::move(text)){}

    Value GetValue() const override{
        if(!text_.empty() && text_[0] == ESCAPE_SIGN){
            return text_.substr(1);
        }

        return text_;
    }

    std::string GetText() const override{
        return text_;
    }

private:
    std::string text_ = "";
};

class Cell::FormulaImpl : public Impl{
public:
    FormulaImpl(std::string text, const SheetInterface &sheet) : sheet_(sheet), formula_(ParseFormula(text.substr(1))){}

    Value GetValue() const override{
        if(!cached_value_.has_value()){
            cached_value_ = formula_->Evaluate(sheet_);
        }

        return std::visit(
            [](const auto& value){
                return Value(value);
            },
            *cached_value_
        );
    }

    std::vector<Position> GetReferencedCells() const override {
        return formula_->GetReferencedCells();
    }

    FormulaInterface* GetFormula() override {
        return formula_.get();
    }

    bool IsCacheValid() const override {
        return cached_value_.has_value();
    }

    void InvalidateCache() override {
        cached_value_.reset();
    }

    std::string GetText() const override{
        return FORMULA_SIGN + formula_->GetExpression();
    }

private:
    const SheetInterface& sheet_;
    std::unique_ptr<FormulaInterface> formula_;
    mutable std::optional<FormulaInterface::Value> cached_value_;
};



Cell::Cell(Sheet& sheet) : impl_(std::make_unique<EmptyImpl>()), sheet_(sheet){}

Cell::~Cell() {

}

void Cell::Set(std::string text) {
    std::unique_ptr<Impl> new_impl;

    if(text.empty()){
        new_impl = std::make_unique<EmptyImpl>();
    } else if (text.size() > 1 && text[0] == FORMULA_SIGN) {
        new_impl = std::make_unique<FormulaImpl>(std::move(text), sheet_);
    } else {
        new_impl = std::make_unique<TextImpl>(std::move(text));
    }

    if (CheckCircularDependency(*new_impl)){ 
        throw CircularDependencyException("Introduce Circular Dependency");
    }

    impl_.swap(new_impl);

    UpdateDependencies();

    InvalidateCache(true);
}

bool Cell::CheckCircularDependency(const Impl& newImpl) const{
    
    if (newImpl.GetReferencedCells().empty()) {
        return false;
    }

    std::unordered_set<const Cell*> referenced_cells;

    for (const auto& pos : newImpl.GetReferencedCells()) {
        referenced_cells.insert(sheet_.GetCellPtr(pos));
    }

    std::unordered_set<const Cell*> visited_cells;
    std::stack<const Cell*> to_visit_cells;
    to_visit_cells.push(this);

    while (!to_visit_cells.empty()) {
        const Cell *current = to_visit_cells.top();
        to_visit_cells.pop();
        visited_cells.insert(current);

        if (referenced_cells.find(current) != referenced_cells.end()) {
            return true;
        }

        for (const Cell* ref : current->depends_on_cells_) {
            if (visited_cells.find(ref) == visited_cells.end()) {
                to_visit_cells.push(ref);
            }
        }
    }

    return false;
}

void Cell::UpdateDependencies(){

    for (Cell* cell : cells_depend_on_this_) {
        cell->depends_on_cells_.erase(this);
    }

    cells_depend_on_this_.clear();

    for (const auto& pos : impl_->GetReferencedCells()) {
        
        Cell* cell = sheet_.GetCellPtr(pos);

        if (!cell) {
            sheet_.SetCell(pos, "");
            cell = sheet_.GetCellPtr(pos);
        }

        cells_depend_on_this_.insert(cell);
        cell->depends_on_cells_.insert(this);
    }
}

void Cell::InvalidateCache(bool forced_update){
    if (impl_->IsCacheValid() || forced_update) {
        impl_->InvalidateCache();

        for (Cell *cell : depends_on_cells_) {
            cell->InvalidateCache();
        }
    }
}

void Cell::Clear() {
    Set("");
}

Cell::Value Cell::GetValue() const {
    return impl_->GetValue();
}

std::string Cell::GetText() const {
    return impl_->GetText();
}