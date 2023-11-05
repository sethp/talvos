
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include "CommandFile.h"
#include "talvos/Module.h"
#include "talvos/PipelineExecutor.h"

#include <emscripten.h>
#include <emscripten/console.h>

extern "C"
{
  using PhyCoord = talvos::PipelineExecutor::PhyCoord;
  using LogCoord = talvos::PipelineExecutor::LogCoord;

  // TODO pin this representation?
  enum LaneState
  {
    Active = talvos::PipelineExecutor::LaneState::Active,
    Inactive = talvos::PipelineExecutor::LaneState::Inactive,
    AtBarrier = talvos::PipelineExecutor::LaneState::AtBarrier,
    AtBreakpoint = talvos::PipelineExecutor::LaneState::AtBreakpoint,
    AtAssert = talvos::PipelineExecutor::LaneState::AtAssert,
    AtException = talvos::PipelineExecutor::LaneState::AtException,
    NotLaunched = talvos::PipelineExecutor::LaneState::NotLaunched,
    Exited = talvos::PipelineExecutor::LaneState::Exited,
  };
  // TODO it'd be nice to check if ^ matches talvos::PipelineExecutor::LaneState

  struct ExecutionUniverse
  {
    // TODO bigger count type (requires C++ bitset-alike; i.e. roaring?)
    uint8_t Cores, Lanes;
    talvos::PipelineExecutor::StepResult Result;
    uint64_t SteppedCores, SteppedLanes;

    struct
    {
      PhyCoord PhyCoord;
      LogCoord LogCoord;
      LaneState State;
    } LaneStates[64 /* TODO */];
  };

  static_assert(sizeof(ExecutionUniverse) == 1304);
  static_assert(offsetof(ExecutionUniverse, Cores) == 0);
  static_assert(offsetof(ExecutionUniverse, Lanes) == 1);
  static_assert(offsetof(ExecutionUniverse, Result) == 2);
  static_assert(offsetof(ExecutionUniverse, SteppedCores) == 8);
  static_assert(offsetof(ExecutionUniverse, SteppedLanes) == 16);
  static_assert(offsetof(ExecutionUniverse, LaneStates) == 24);

  static_assert((sizeof(ExecutionUniverse{}.LaneStates) /
                 sizeof(ExecutionUniverse{}.LaneStates[0])) == 64);

  static_assert(sizeof(ExecutionUniverse{}.LaneStates[0]) == 20);
  static_assert(offsetof(ExecutionUniverse, LaneStates[0].PhyCoord) == 24);
  static_assert(offsetof(ExecutionUniverse, LaneStates[0].LogCoord) == 28);
  static_assert(offsetof(ExecutionUniverse, LaneStates[0].State) == 40);
  static_assert(offsetof(ExecutionUniverse, LaneStates[1]) == 44);

  static_assert(offsetof(ExecutionUniverse, LaneStates[0].PhyCoord) -
                    offsetof(ExecutionUniverse, LaneStates) ==
                0);
  static_assert(offsetof(ExecutionUniverse, LaneStates[0].LogCoord) -
                    offsetof(ExecutionUniverse, LaneStates) ==
                4);
  static_assert(offsetof(ExecutionUniverse, LaneStates[0].State) -
                    offsetof(ExecutionUniverse, LaneStates) ==
                16);

  static_assert(sizeof(PhyCoord) == 2);
  static_assert(offsetof(PhyCoord, Core) == 0);
  static_assert(offsetof(PhyCoord, Lane) == 1);

  static_assert(sizeof(LogCoord) == 12);
  static_assert(offsetof(LogCoord, X) == 0);
  static_assert(offsetof(LogCoord, Y) == 4);
  static_assert(offsetof(LogCoord, Z) == 8);

  static_assert(sizeof(LaneState) == 4);
}

class Chatty
{
public:
  Chatty() { emscripten_console_log("hello"); }
  ~Chatty() { emscripten_console_log("goodbye"); }
};

class Session
{
public:
  std::stringstream CmdStream;
  CommandFile CF;

  Session(const char *module, const char *commands)
      : CmdStream(commands), CF(module, CmdStream){};

  void run() { CF.run(); }
  ExecutionUniverse start()
  {
    CF.run(CommandFile::DEBUG);
    // run one step
    return makeU(step());
  }

  talvos::PipelineExecutor::StepResult step(uint64_t StepMask = -1)
  {
    auto res = CF.Device->getPipelineExecutor().step(StepMask);
    if (res == talvos::PipelineExecutor::FINISHED)
      // run more COMMANDs (e.g. DUMP)
      // TODO another DISPATCH won't work here (for two reasons)
      CF.run(CommandFile::DEBUG);
    return res;
  }

  talvos::PipelineExecutor::StepResult cont()
  {
    // since we don't support breakpoints yet...
    while (step() != talvos::PipelineExecutor::FINISHED)
      ;
    return talvos::PipelineExecutor::FINISHED;
  }

  void print(const std::vector<std::string> &Args)
  {
    CF.Device->getPipelineExecutor().print(Args);
  }

  void swtch(const std::vector<std::string> &Args)
  {
    CF.Device->getPipelineExecutor().swtch(Args);
  }

  void printContext()
  {
    auto &PE = CF.Device->getPipelineExecutor();
    if (PE.getCurrentInvocation())
      PE.printContext();
  }

  inline ExecutionUniverse makeU(talvos::PipelineExecutor::StepResult result,
                                 uint64_t steppedMask = -1)
  {
    ExecutionUniverse ret = {
        .Cores = static_cast<uint8_t>(CF.Device->Cores),
        .Lanes = static_cast<uint8_t>(CF.Device->Lanes),
        .Result = result,

        // TODO remove/rename this... (lolsob)
        // .SteppedCores = steppedCores,
    };

    auto getOrDef = [](const PhyCoord &k) {
      auto res = talvos::PipelineExecutor::Assignments.find(k);
      return res == talvos::PipelineExecutor::Assignments.end() ? LogCoord{}
                                                                : res->second;
    };

    int n = 0;
    for (const auto &[phyCoord, state] : talvos::PipelineExecutor::Lanes)
    {
      assert(n < (sizeof(ret.LaneStates) / sizeof(ret.LaneStates[0])));
      steppedMask &=
          ~(state != talvos::PipelineExecutor::NotLaunched ? 0x0 : (0x1 << n));
      ret.LaneStates[n++] = {.PhyCoord = phyCoord,
                             // TODO oh no, now this might not exist :(
                             .LogCoord = getOrDef(phyCoord),
                             // TODO reinterpret_cast ?
                             .State = static_cast<LaneState>(state)};
    }

    ret.SteppedLanes = steppedMask;

    assert(n == ret.Cores * ret.Lanes);
    return ret;
  }
};

// lets manually mangle some names yey
extern "C"
{
  EMSCRIPTEN_KEEPALIVE Session *Session__create__(const char *module,
                                                  const char *commands)
  {
    return new Session(module, commands);
  };
  EMSCRIPTEN_KEEPALIVE void Session__destroy__(Session *self) { delete self; };

  EMSCRIPTEN_KEEPALIVE void Session_run(Session *self) { self->run(); };
  EMSCRIPTEN_KEEPALIVE void Session_start(Session *self, ExecutionUniverse *out)
  {
    assert(out);

    *out = self->start();
  };
  EMSCRIPTEN_KEEPALIVE void Session_printContext(Session *self)
  {
    self->printContext();
  };

  // interactions
  EMSCRIPTEN_KEEPALIVE talvos::PipelineExecutor::StepResult
  Session_step(Session *self, uint64_t *laneMask, ExecutionUniverse *out)
  {
    assert(out);
    assert(laneMask);

    auto ret = self->step(*laneMask);
    *out = self->makeU(ret, *laneMask);
    return ret;
  }
  EMSCRIPTEN_KEEPALIVE void Session_continue(Session *self,
                                             ExecutionUniverse *out)
  {
    *out = self->makeU(self->cont());
  }

  // inspection (NB: switch as an inspection-only concept is novel here)
  EMSCRIPTEN_KEEPALIVE void Session_print(Session *self, int argc, char *argv[])
  {
    std::vector<std::string> Args(argv, argv + argc);
    self->print(Args);
  }
  EMSCRIPTEN_KEEPALIVE void Session_switch(Session *self, int argc,
                                           char *argv[])
  {
    std::vector<std::string> Args(argv, argv + argc);
    self->swtch(Args);
  }
};

extern "C"
{
  EMSCRIPTEN_KEEPALIVE bool validate_wasm(const char *module)
  {
    std::vector<uint8_t> ModuleText((const uint8_t *)module,
                                    (const uint8_t *)module + strlen(module));

    return talvos::Module::load(ModuleText) != nullptr;
  }

  EMSCRIPTEN_KEEPALIVE Session *run_wasm(const char *module,
                                         const char *commands)
  {
    return Session__create__(module, commands);
  }

  EMSCRIPTEN_KEEPALIVE Session *debug_wasm(const char *module,
                                           const char *commands)
  {
    return Session__create__(module, commands);
  }

  EMSCRIPTEN_KEEPALIVE
  void Session_fetch_shrubbery(Session *self, uint64_t *out) { *out = -1; }

  // this kind of exception doesn't work so well in WASM-land
  // EMSCRIPTEN_KEEPALIVE void exception() { throw "hello: it's an
  // exception!";
  // }
  EMSCRIPTEN_KEEPALIVE void exception()
  {
    throw std::runtime_error("hello: it's an exception!");
  }

  EMSCRIPTEN_KEEPALIVE void assertion()
  {
    assert(false && "hello: it's an assertion!");
  }
}
