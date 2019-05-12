#include <cstring>
#include <iostream>

#define MEM_SIZE_BYTES (128 << 10)
#define MEM_SIZE_WORDS (MEM_SIZE_BYTES/sizeof(uint32_t))
#define PAGE_SIZE_BYTES 64
static uint32_t mem[MEM_SIZE_WORDS];
static uint32_t* heap = &mem[0];
static uint32_t** heap_curr = &heap;

static uint32_t* const heap_start = mem;
static uint32_t* const heap_end = (mem + MEM_SIZE_WORDS);

void init_heap(uint32_t* heap_start, uint32_t* const heap_end) {
  std::cerr <<  &mem << " | " << heap_end << std::endl;
  const uint32_t size = static_cast<uint32_t>(heap_end - heap_start);
  std::cerr << size << std::endl;
  if (size > 0) {
    std::memset(heap_start, 0, size);
  }
}


uint32_t* brk(int32_t words, uint32_t** curr, uint32_t* const start, uint32_t* const end) {
  if (words == 0) {
    return *curr;
  }

  uint32_t* next = (*curr + words);
  if (words < 0) {
    if (next < start) {
      next = start;
    }
    *curr = next;
    return next;
  }

  if (next <= end) {
    *curr = next;
    return next;
  }
  return nullptr;
}

uint32_t* kbrk(int32_t words) {
  return brk(words, heap_curr, heap_start, heap_end);
}

typedef struct MHeader{
   uint32_t size;
   uint32_t* next;
} MHeader;
const uint32_t kMHsize = sizeof(MHeader);
const uint32_t kMHwords = kMHsize / sizeof(uint32_t);


uint32_t* alloc(int32_t bytes, uint32_t** curr, uint32_t* const start, uint32_t* const end) {
  if (bytes <= 0) {
    return nullptr;
  }

  // Total bytes = bytes + header_size and must be aligned to page boundary.
  std::cerr << "Requested " << bytes << std::endl;
  bytes += kMHsize;
  const int32_t pages = bytes / PAGE_SIZE_BYTES;
  const int32_t page_bytes = pages * PAGE_SIZE_BYTES;
  if (bytes != page_bytes) bytes = page_bytes + PAGE_SIZE_BYTES;
  std::cerr << "Will allocate " << bytes << std::endl;

  MHeader mh;
  uint32_t* heap = start;

  while (true) {
    memcpy(&mh, heap, kMHsize);
    if (mh.size == 0) {
      // First page ever.
      uint32_t* next = brk(bytes/sizeof(uint32_t), curr, start, end);
      if (next == nullptr) {
        return nullptr;
      }
      // Got the memory. Write the header and return ptr to the block.
      mh.size = bytes;
      mh.next = next;
      memcpy(heap, &mh, kMHsize);
      return &heap[kMHwords];

    } else if (mh.size < 0) {
      mh.size *= -1;
      // We found a free page with enough memory.
      memcpy(heap, &mh, kMHsize);
      return &heap[kMHwords];

    } else {
      // Current page is not good. Go to the next one or allocate memory.
      if (mh.next != nullptr) {
        heap = mh.next;
      } else {
        uint32_t* next = brk(bytes/sizeof(uint32_t), curr, start, end);
        if (next == nullptr) {
          return nullptr;
        }
        // Got the memory. Write the header and return ptr to the block.
        mh.size = bytes;
        mh.next = next;
        memcpy(heap, &mh, kMHsize);
        return &heap[kMHwords];
      }
    }
  }

  return nullptr;
}

uint32_t* kalloc(int32_t bytes) {
  return alloc(bytes, heap_curr, heap_start, heap_end);
}

int main(void) {
  init_heap(*heap_curr, heap_end);

  // Test the kbrk is working.
  std::cerr << kbrk(0) << std::endl;
  std::cerr << kbrk(16) << std::endl;
  std::cerr << kbrk(-1) << std::endl;
  std::cerr << kbrk(-15) << std::endl;
  std::cerr << kbrk(MEM_SIZE_WORDS) << std::endl;
  std::cerr << kbrk(1) << std::endl;
  std::cerr << kbrk(-(1<<20)) << std::endl;

  // Test that kalloc is working.
  std::cerr << kalloc(16) << std::endl;
  std::cerr << kalloc(16) << std::endl;
  std::cerr << kalloc(48) << std::endl;
  std::cerr << kalloc(49) << std::endl;

  return 0;
}
