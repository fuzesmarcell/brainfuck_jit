# Brainf jit compiler (x64)
## Description
A simple brainf*** jit compiler which generates x64 instructions on the fly and then executes the code.
Currently only working on windows.
## Usage
To use the brainf jit compiler you have to do the following steps
1. Get brainf jit compiler
2. Build from source
3. Run
### Step 1: Get brainf jit compiler
1. Run the command `git clone https://github.com/fuzesmarcell/brainfuck_jit.git`
### Step 2: Build from source
1. `cd brainfuck_jit/code`
2. Run `build.bat`
NOTE: The `build.bat` script on Windows expects to find `cl` (MSVC). Your environment should know about this. The easiest way to do this is to use one of the Visual Studio command prompts (titled `x64 Native Tools Command Prompt for VS<version>`, or `x86 Native Tools Command Prompt for VS<version>`). Otherwise, you can call `vcvarsall.bat` in your terminal environment, which is packaged with Visual Studio.
### Step 3: Run
1. To run a brainf program simple execute the command in the build directory `brainf_jit brainfsource.b`
2. You can specify different parameters with addtional command line arguments. Simply run `brainf_jit` and the usage will be printed
NOTE: The -cellsize option currently is not working and can be only 64-bit.
## Examples
In the `examples` directory you will find some brainf programms you can try out.