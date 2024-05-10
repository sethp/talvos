# Talvos (wasm-experimental)

Hi, Seth here, sorry for the source dump. I'm still learning my way through a whole bunch of layers at once, so things are gonna be weird for a bit.

## What you need to know

This is being actively co-developed as part of the https://github.com/sethp/learn-gpgpu project.

It's not really stand-alone right now, it's probably better for you to only look at it through the lens of that project. Or, if you've got an idea in mind for how it *should* stand on its own and want to help, just let me know.

## Previous notes

The two most important things I'm getting out of this stuff right now are:

1. A relatively convenient workflow for pushing/pulling things across the browser/WASM boundary; that's driven entirely from a different repo (https://github.com/sethp/learn-gpgpu), though, so there's no guarantees on this project making much sense stand-alone.
2. Debugging via Chrome's DWARF extension: this implies being able to find the C++ sources at a path baked into the debug files.


Things that I know are broken (PRs welcome!):

1. Many of the tests; I changed the "step" semantic to step all SIMT lanes once (previously it was doing a single instruction for a single lane), and this had some Consequences that I haven't fully unwound. Configuring the "interactive" tests to run with a single-core, single-lane simulated GPU might get them working again?
2. The emscripten build is more than a little weird, and ctest hangs when you try and run it.
3. Some unknown amount of the vulkan stuff, especially in emscripten-land. I haven't dug into it too much.
4. The docs here aren't kept particularly up-to-date.
5. This depends on a forked SPIRV-Headers so that I could more rapidly prototype extensions. See the big warning at the top for how to build it yourself.

Things that sure seem like they could work?

1. The build ought to reliably produce a WASM output that's able to be copied "elsewhere"
2. Debugging specific tests with `./hack/debug.sh`, if you take care to avoid using the emscripten toolchain (I have a `./build/host` cmake directory for this purpose)



# Talvos

This project provides a SPIR-V interpreter and Vulkan device emulator, with the
aim of providing an extensible dynamic analysis framework and debugger for
SPIR-V shaders.
Talvos provides an implementation of the Vulkan API to enable it to execute
real Vulkan applications.
Linux, macOS, and Windows are supported.

Talvos is distributed under a three-clause BSD license. For full license
terms please see the LICENSE file distributed with this source code.


## Status

Talvos is still in the early stages of development.
Compute shaders are the current focus, and Talvos is currently capable of
executing various SPIR-V shaders generated from OpenCL kernels compiled with
[Clspv](https://github.com/google/clspv).
Talvos can also handle vertex and fragment shaders, with basic support for
offscreen rendering currently in progress.
Talvos currently passes around 15% of the
[Vulkan conformance test suite](https://github.com/KhronosGroup/VK-GL-CTS).

The codebase is changing relatively quickly as new features are added, so the
internal and external APIs are all subject to change until we reach the 1.0
release (suggestions for improvements always welcome).

Future work may involve extending the emulator to support tessellation and
geometry shaders, onscreen rendering, and implementing missing Vulkan API
functions to reach conformance.
Contributions in these (or other) areas would be extremely welcome.


## Building

More detailed build instructions are provided
[here](https://talvos.github.io/building.html).

Building Talvos requires a compiler that supports C++17, and Python to enable
the internal test suite.
Talvos depends on
[SPIRV-Headers](https://github.com/KhronosGroup/SPIRV-Headers) and
[SPIRV-Tools](https://github.com/KhronosGroup/SPIRV-Tools), and uses GNU
readline to enhance the interactive debugger.

Configure with CMake, indicating where SPIRV-Headers and SPIRV-Tools are
installed if necessary:

    cmake <path_to_talvos_source>                           \
          -DCMAKE_BUILD_TYPE=Debug                          \
          -DCMAKE_INSTALL_PREFIX=<target_install_directory> \
          -DSPIRV_INCLUDE_DIR=<...>                         \
          -DSPIRV_TOOLS_INCLUDE_DIR=<...>                   \
          -DSPIRV_TOOLS_LIBRARY_DIR=<...>

Using the `Debug` build type is strongly recommended while Talvos is still in
the early stages of development, to enable assertions that guard unimplemented
features.

Once configured, use `make` (or your preferred build tool) to build and
install. Run `make test` to run the internal test suite.


## Usage

More detailed usage information is provided
[here](https://talvos.github.io/usage.html).

Talvos provides an implementation of the Vulkan API which allows existing
Vulkan applications to be executed through the emulator without modification.
Simply linking an application against `libtalvos-vulkan.so` or
`talvos-vulkan.lib` is enough for it to use Talvos.

Alternatively, the `talvos-cmd` command provides a simple interface to the
emulator.
An example that runs a simple N-Body simulation can be found in
`test/misc/nbody.tcf`.

The following command is used to execute a Talvos command file:

    talvos-cmd nbody.tcf

The command file contains commands that allocate and initialize storage buffers
and launch SPIR-V shaders.
There are also commands that dump the output of storage buffers, to verify
that the shader executed as expected.

To enable the interactive debugger, set the environment variable
`TALVOS_INTERACTIVE=1`.
This will drop to a prompt when a shader begins executing.
Use `help` to list the available commands, which include `step` to advance
through the interpreter, `print` to view an instruction result, and `switch` to
change the current invocation.


## More information

Raise issues and ask questions via
[GitHub](https://github.com/talvos/talvos/issues).

The [Talvos documentation](https://talvos.github.io) provides more detailed
information about using Talvos, as well as information for developers.

[Doxygen documentation](https://talvos.github.io/api) for the source code is
also available.
