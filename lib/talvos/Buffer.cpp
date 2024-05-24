#include "talvos/Buffer.h"
#include "talvos/Memory.h"
#include <cassert>
#include <iostream>

namespace talvos
{

template <typename T>
void dump(const Memory &Mem, uint64_t BaseAddr, const std::string &Name,
          size_t NumBytes, unsigned VecWidth = 1)
{
  for (uint64_t i = 0; i < NumBytes / sizeof(T); i += VecWidth)
  {
    std::cout << "  " << Name << "[" << (i / VecWidth) << "] = ";

    if (VecWidth > 1)
      std::cout << "(";
    for (unsigned v = 0; v < VecWidth; v++)
    {
      if (v > 0)
        std::cout << ", ";

      if (i + v >= NumBytes / sizeof(T))
        break;

      T Value;
      Mem.load((uint8_t *)&Value, BaseAddr + (i + v) * sizeof(T), sizeof(T));
      std::cout << Value;
    }
    if (VecWidth > 1)
      std::cout << ")";

    std::cout << std::endl;
  }
}

void dump(const Memory &Mem, uint64_t BaseAddr, const Buffer &B)
{
  assert(B.Ty->isPointer());
  Type const *ElemTy = B.Ty->getElementType();
  // TODO[seth]: better handling for vectors, pointers to structs, etc
  while (ElemTy->isArray() || ElemTy->isRuntimeArray())
    ElemTy = ElemTy->getElementType();

  const auto &Name = B.Name.value_or("<unnamed buffer>");
  const size_t NumBytes = B.Size;

  std::cout << std::endl << "Buffer '" << Name << "'";
  if (!B.Name)
    std::cout << "@0x" << std::hex << BaseAddr << std::dec;
  std::cout << " (" << NumBytes << " bytes):" << std::endl;

  switch (ElemTy->getTypeId())
  {
  case Type::VOID:
    // TODO[seth]: untested (if this is even possible)
    Mem.dump(BaseAddr, NumBytes);
    break;
  case Type::INT:
  {
    const auto BW = ElemTy->getBitWidth();
    // TODO signed-ness
    if (false)
    {
      if (BW == 8)
        dump<uint8_t>(Mem, BaseAddr, Name, NumBytes);
      else if (BW == 16)
        dump<uint16_t>(Mem, BaseAddr, Name, NumBytes);
      else if (BW == 32)
        dump<uint32_t>(Mem, BaseAddr, Name, NumBytes);
      else if (BW == 64)
        dump<uint64_t>(Mem, BaseAddr, Name, NumBytes);
      else
        goto err;
    }
    else
    {
      if (BW == 8)
        dump<int8_t>(Mem, BaseAddr, Name, NumBytes);
      else if (BW == 16)
        dump<int16_t>(Mem, BaseAddr, Name, NumBytes);
      else if (BW == 32)
        dump<int32_t>(Mem, BaseAddr, Name, NumBytes);
      else if (BW == 64)
        dump<int64_t>(Mem, BaseAddr, Name, NumBytes);
      else
        goto err;
    }
    break;
  err:
    std::cerr << "cannot dump integer: unhandled bit width: " << BW
              << std::endl;
  }
  break;

  case Type::FLOAT:
  {
    const auto BW = ElemTy->getBitWidth();

    if (BW == 32)
      dump<float>(Mem, BaseAddr, Name, NumBytes);
    else if (BW == 64)
      dump<double>(Mem, BaseAddr, Name, NumBytes);
    else
      std::cerr << "cannot dump float: unhandled bit width: " << BW
                << std::endl;
  }
  break;

  case Type::POINTER:
  case Type::BOOL:
  case Type::VECTOR:
  case Type::MATRIX:
  case Type::IMAGE:
  case Type::SAMPLER:
  case Type::SAMPLED_IMAGE:
  case Type::STRUCT:
  case Type::FUNCTION:
    std::cerr << "cannot dump: unhandled type: " << B.Ty;
    break;

  case Type::ARRAY:
  case Type::RUNTIME_ARRAY:
  default:
    __builtin_unreachable();
  }
}
} // namespace talvos
