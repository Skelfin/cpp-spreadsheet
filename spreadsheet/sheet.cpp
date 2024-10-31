#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

Sheet::~Sheet() {
}

void Sheet::SetCell(Position pos, std::string text) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid position in SetCell()");
    }

    PreparingSheetForInsert(pos);

    auto& cell = cells_[pos.row][pos.col];

    if (!cell) {
        cell = std::make_unique<Cell>(*this);
    }

    cell->Set(std::move(text));

    UpdateCoordinateCounter(pos, ActionWithCoordinates::kAddToIndex);
}

void Sheet::PreparingSheetForInsert(Position pos){
    cells_.resize(std::max(size_t(pos.row + 1), cells_.size()));   
    cells_[pos.row].resize(std::max(size_t(pos.col + 1), cells_[pos.row].size()));    
}

const CellInterface* Sheet::GetCell(Position pos) const {
    return GetCellPtr(pos);     
}

CellInterface* Sheet::GetCell(Position pos) {
    return GetCellPtr(pos);    
}

const Cell* Sheet::GetCellPtr(Position pos) const{
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid position in ClearCell()");
    }

    if (size_t(pos.row) >= cells_.size() || size_t(pos.col) >= cells_[pos.row].size()) {
        return nullptr;
    }

    return cells_[pos.row][pos.col].get();        
}

Cell* Sheet::GetCellPtr(Position pos){
    return const_cast<Cell*>(static_cast<const Sheet&>(*this).GetCellPtr(pos));        
}

void Sheet::ClearCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid position in ClearCell()");
    }

    if ((size_t(pos.row) < cells_.size()) && (size_t(pos.col) < cells_[pos.row].size())) {
        if (auto &cell = cells_[pos.row][pos.col]; cell) {
            cell.reset();
            UpdateCoordinateCounter(pos, ActionWithCoordinates::kRemoveFromIndex);
        }
    }
}

Size Sheet::GetPrintableSize() const {

    if(row_number_to_counter_.empty() || col_number_to_counter_.empty()){
        return {0,0};
    }

    return {row_number_to_counter_.rbegin()->first + 1, col_number_to_counter_.rbegin()->first + 1};
}

void Sheet::PrintValues(std::ostream& output) const {
    PrintCells(output, [&output](const CellInterface* cell){
        if(cell){
            std::visit([&output](const auto& value) { output << value; }, cell->GetValue());
        }
        
    });    
}

void Sheet::PrintTexts(std::ostream& output) const {
    PrintCells(output, [&output](const CellInterface* cell) {
        if(cell){
            output << cell->GetText(); 
        } 
        
    });
}

template<typename FunctionOutStream>
void Sheet::PrintCells(std::ostream& output, FunctionOutStream print_cell) const{

    auto size = GetPrintableSize();
    for (int row = 0; row < size.rows; ++row) {
        for (int col = 0; col < size.cols; ++col) {
            
            if(col > 0){
                output << '\t';
            }

            print_cell(GetCell({row, col}));
            
        }

        output << '\n';
    }   
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}

void Sheet::UpdateCoordinateCounter(Position pos, ActionWithCoordinates action){

    if(action == ActionWithCoordinates::kAddToIndex){
        ++row_number_to_counter_[pos.row];
        ++col_number_to_counter_[pos.col];
        return;
    }

    if(action == ActionWithCoordinates::kRemoveFromIndex){
        auto row_number_iter = row_number_to_counter_.find(pos.row);
        auto col_number_iter = col_number_to_counter_.find(pos.col);

        if(row_number_iter != row_number_to_counter_.end() && col_number_iter != col_number_to_counter_.end()){
            if(--row_number_iter->second <= 0){
                row_number_to_counter_.erase(row_number_iter);
            }

            if(--col_number_iter->second <= 0){
                col_number_to_counter_.erase(col_number_iter);
            }
        }
    }
}