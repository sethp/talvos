#ifndef TALVOS_BUFFER_H
#define TALVOS_BUFFER_H

#include <cstddef>
#include <optional>
#include <string>

#include "talvos/Type.h"

namespace talvos
{

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

} // namespace talvos

#endif
