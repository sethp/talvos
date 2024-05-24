#ifndef TALVOS_BUFFER_H
#define TALVOS_BUFFER_H

#include <cstddef>
#include <optional>
#include <string>

#include "talvos/Type.h"

namespace talvos
{

class Memory;

/// Buffer represents a managed shared memory area that's allocated
/// by the runtime at dispatch/exec time.
///
/// TODO: define more semantics around storage classes/memory models & lifecycle
struct Buffer
{
  const uint32_t Id;
  const Type *Ty;
  const size_t Size; ///< Size (in bytes)
  const uint32_t StorageClass;
  const std::optional<std::string> Name;
};

void dump(const Memory &Mem, uint64_t BaseAddr, const Buffer &B);

#ifdef __EMSCRIPTEN__
static_assert(sizeof(talvos::Buffer) == 32);
static_assert(offsetof(talvos::Buffer, Id) == 0);
static_assert(offsetof(talvos::Buffer, Size) == 8);
static_assert(offsetof(talvos::Buffer, Name) == 16);
#endif

} // namespace talvos

#endif
