#ifndef _GVM_TEXT_MODE_ROM_H_
#define _GVM_TEXT_MODE_ROM_H_

#include "rom.h"

namespace gvm {
namespace rom {

// Functions and constants for text mode.
Rom* Textmode(uint32_t memaddr);

}  // namespace rom
}  // namespace gvm

#endif  // _GVM_TEXT_MODE_ROM_H_
