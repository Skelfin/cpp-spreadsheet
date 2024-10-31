#pragma once

#include "common.h"
#include "formula.h"

#include <functional>
#include <unordered_set>

class Sheet;

class Cell : public CellInterface {
public:
    Cell(Sheet& sheet);
    ~Cell();

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;

    bool IsReferenced() const;


private:
    class Impl;
    class EmptyImpl;
    class TextImpl;
    class FormulaImpl;

    std::unique_ptr<Impl> impl_;
    Sheet& sheet_;

    std::unordered_set<Cell*> depends_on_cells_;
    std::unordered_set<Cell*> cells_depend_on_this_;
    
    bool CheckCircularDependency(const Impl& newImpl) const;
    void UpdateDependencies();
    void InvalidateCache(bool forced_update = false);
};