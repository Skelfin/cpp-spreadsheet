#pragma once

#include "cell.h"
#include "common.h"

#include <functional>
#include <map>

class Sheet : public SheetInterface {
public:
    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

    const Cell* GetCellPtr(Position pos) const;
    Cell* GetCellPtr(Position pos);

private:
    enum class ActionWithCoordinates{
        kAddToIndex,
        kRemoveFromIndex
    };

    std::map<int, int> row_number_to_counter_;
    std::map<int, int> col_number_to_counter_;

    void UpdateCoordinateCounter(Position pos, ActionWithCoordinates action);
    void PreparingSheetForInsert(Position pos);

    bool IsPositionValid(Position pos) const;
    bool HasCell(Position pos) const;
    bool HasCoordinateCounter(int row, int col) const;

    template<typename FunctionOutStream>
    void PrintCells(std::ostream& output, FunctionOutStream print_cell) const;

    std::vector<std::vector<std::unique_ptr<Cell>>> cells_;
};
