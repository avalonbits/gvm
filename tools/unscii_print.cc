#include <cstdint>
#include <iostream>


static const uint32_t kLetter[4] = {
  0x3c180000, 0x7e666666, 0x66666666, 0x00000066
};

void PrintPart(uint32_t letter) {
  for (int i = 0; i < sizeof(uint32_t)*8; ++i) {
    if (i % 8 == 0) std::cerr << "\n";
    auto c = letter & 0x01;

    if (c == 0) {
      std::cerr << " ";
    } else {
      std::cerr << "*";
    }
    letter = letter >> 1;
  }
}

void Print(const uint32_t* letter, uint32_t words) {
  for (int i = 0; i < words; ++i) {
    PrintPart(letter[i]);
  }
}

int main(void) {
  Print(kLetter, 4);
  return 0;
}
