#ifndef _GVM_TICKER_H_
#define _GVM_TICKER_H_

#include <functional>

namespace gvm {
class Ticker {
 public:
  Ticker(uint32_t hertz, std::function<void(void)> callback);

  uint64_t Start();
  void Stop() { stop_ = true; }

 private:
  uint32_t hertz_;
  std::function<void(void)> callback_;
  bool stop_;
};

}  // namespace gvm

#endif
