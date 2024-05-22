// Copyright (c) 2018 the Talvos developers. All rights reserved.
//
// This file is distributed under a three-clause BSD license. For full license
// terms please see the LICENSE file distributed with this source code.

/// \file Instruction.cpp
/// This file defines the Instruction class.

#include <cassert>
#include <cstring>
#include <iostream>

#define SPV_ENABLE_UTILITY_CODE
#include <spirv/unified1/spirv.h>
#undef SPV_ENABLE_UTILITY_CODE

#include "talvos/Instruction.h"

namespace talvos
{

Instruction::Instruction(uint16_t Opcode, uint16_t NumOperands,
                         const uint32_t *Operands, const Type *ResultType)
{
  this->Opcode = Opcode;
  this->NumOperands = NumOperands;
  this->ResultType = ResultType;
  this->Next = nullptr;
  this->Previous = nullptr;

  this->Operands = new uint32_t[NumOperands];
  memcpy(this->Operands, Operands, NumOperands * sizeof(uint32_t));
}

void Instruction::insertAfter(Instruction *I)
{
  assert(Opcode != SpvOpLabel);

  this->Previous = I;
  this->Next = std::move(I->Next);
  if (this->Next)
    this->Next->Previous = this;
  I->Next = std::unique_ptr<Instruction>(this);
}

void Instruction::print(std::ostream &O, bool Align) const
{
  if (Align)
  {
    if (ResultType)
    {
      // TODO: Adapt extra whitespace based on module ID bound.
      if (Operands[1] < 100)
        O << " ";
      if (Operands[1] < 10)
        O << " ";
    }
    else
      O << "       ";
  }

  if (ResultType)
    O << "%" << Operands[1] << " = ";

  O << opcodeToString(Opcode);
  for (unsigned i = 0; i < NumOperands; i++)
  {
    if (ResultType && i == 1)
      continue;
    // TODO: Omit the '%' if this is a literal operand.
    O << " %" << Operands[i];
  }
}

const char *Instruction::opcodeToString(uint16_t Opcode)
{
  return SpvOpToString((const SpvOp)Opcode);
}

} // namespace talvos
