#include "common.h"

#include <cctype>
#include <sstream>
#include <algorithm>

const int LETTERS = 26;
const int MAX_POSITION_LENGTH = 17;
const int MAX_POS_LETTER_COUNT = 3;

const Position Position::NONE = {-1, -1};

bool Position::operator==(const Position rhs) const {
    return (row == rhs.row) && (col == rhs.col);
}

bool Position::operator<(const Position rhs) const {
    return std::tie(row, col) < std::tie(rhs.row, rhs.col);
}

bool Position::IsValid() const {
    return (row >= 0 && row < MAX_ROWS) && (col >= 0 && col < MAX_COLS);
}

std::string Position::ToString() const {
    if (!IsValid()) {
        return "";
    }

    int remainder = col;
    std::string ans;

    while (remainder >= 0) {
        auto result_div = std::div(remainder, LETTERS);
        ans += 'A' + static_cast<char>(result_div.rem);

        if (result_div.quot == 0) {
            remainder = -1; 
        } else {
            remainder = result_div.quot - 1;
        }
    }

    std::reverse(ans.begin(), ans.end());
    ans += std::to_string(row + 1);

    return ans;
}

Position Position::FromString(std::string_view str) {
    auto separation = std::find_if(str.begin(), str.end(), [](const auto& symbol) -> bool {
        return !(std::isalpha(symbol) && std::isupper(symbol));
    });

    std::string symbols{str.begin(), separation};
    std::string numbers{separation, str.end()};

    if (symbols.empty() || numbers.empty() || symbols.size() > MAX_POS_LETTER_COUNT || !std::isdigit(numbers[0])) {
        return Position::NONE;
    }

    int row = 0; 
    for (const auto& digit : numbers) {
        if (!std::isdigit(digit) || row >= MAX_ROWS) {
            return Position::NONE;
        }
        row = row * 10 + int(digit - '0');
    }

    int col = 0;
    for (const auto& symbol : symbols) {
        col = col * LETTERS + int(symbol - 'A') + 1;
    }

    return {row - 1, col - 1};
}
