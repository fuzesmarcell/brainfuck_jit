@echo off

IF NOT EXIST ..\build mkdir ..\build

pushd ..\build

cl /nologo /FC /Z7 ..\code\brainf_jit.c

popd