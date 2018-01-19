// Copyright (c) 2018 the Talvos developers. All rights reserved.
//
// This file is distributed under a three-clause BSD license. For full license
// terms please see the LICENSE file distributed with this source code.

#ifndef TALVOS_INVOCATION_H
#define TALVOS_INVOCATION_H

#include <vector>

#include "talvos/Object.h"

namespace talvos
{

struct Function;
struct Instruction;
class Memory;
class Module;

class Invocation
{
public:
  enum State { READY, FINISHED };

public:
  Invocation(const Module *M, const Function *F);
  ~Invocation();
  State getState() const;
  void step();

  // Instruction handlers.
  void executeAccessChain(const Instruction *Inst);
  void executeLoad(const Instruction *Inst);
  void executeStore(const Instruction *Inst);

private:
  const Instruction *CurrentInstruction;
  std::vector<Object> Objects;
  Memory *GlobalMemory;
};

} // namespace talvos

#endif