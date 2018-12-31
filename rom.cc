#include "rom.h"

#include <cassert>

namespace gvm {

/* static */
const Rom* Rom::FromFile(std::ifstream& in) {
  const std::string header("1987gvm");

  char hBuffer[7];
  in.read(hBuffer, 7);
  assert(header.compare(0, 7, hBuffer, 7) == 0);

  auto* rom = new Rom();
  while (in) {
    uint32_t addr;
    in.read(reinterpret_cast<char*>(&addr), 4);
    if (!in) break;
    uint32_t wCount;
    in.read(reinterpret_cast<char*>(&wCount), 4);
    assert(wCount <= (256 << 20));
    uint32_t* buffer = new uint32_t[wCount];
    in.read(reinterpret_cast<char*>(buffer), wCount*4);
    rom->Load(addr, buffer, wCount);
  }
  in.close();
  return rom;
}

Rom::Rom(uint32_t memaddr, const std::vector<Word>& words) {
  Load(memaddr, words);
}

void Rom::Load(uint32_t memaddr, const std::vector<Word>&  words) {
  assert(!words.empty());
  assert(memaddr % kWordSize == 0);
  rom_[memaddr] = words;
}

void Rom::Load(uint32_t memaddr, const uint32_t* words, uint32_t size) {
  std::vector<Word>  program(words, words + size);
  Load(memaddr, program);
}

void Rom::ToFile(std::ofstream& out) const {
  std::vector<uint32_t> buffer;

  for (const auto& kv : rom_) {
    buffer.push_back(kv.first);
    buffer.push_back(kv.second.size());
    buffer.insert(buffer.end(), kv.second.begin(), kv.second.end());
  }

  out.write("1987gvm", 7);
  out.write(reinterpret_cast<const char*>(buffer.data()),
            buffer.size() * sizeof(uint32_t));
  out.close();
}

}  // namespace gvm
