/*
====================================================================
$File: $
$Date: $
$Revision: $
$Createor: Fuzes Marcell $
$Notice: (C) Copyright 2018 by Fuzes Marcell, All Rights Reserved. $
====================================================================
*/

#include "brainf_jit.h"

#include <windows.h>
#include <stdio.h>

internal LARGE_INTEGER
Win32GetWallClock()
{
    LARGE_INTEGER Result;
    QueryPerformanceCounter(&Result);
    
    return Result;
}

static s64 GlobalPerfCountFrequency;
internal f32
Win32GetSecondsElapsed(LARGE_INTEGER Start, LARGE_INTEGER End)
{
    f32 Result;
    s64 CounterElapsed = End.QuadPart - Start.QuadPart;
    Result = ((f32)CounterElapsed) / ((f32)GlobalPerfCountFrequency);
    return Result;
}

internal loaded_file
ReadEntireFileAndZeroTerminate(char *FileName)
{
    loaded_file Result;
    
    FILE *File = fopen(FileName, "rb");
    fseek(File, 0, SEEK_END);
    Result.Size = ftell(File) + 1;
    Result.Contents = (char *)malloc(Result.Size);
    
    fseek(File, 0, SEEK_SET);
    
    fread(Result.Contents, Result.Size, 1, File);
    fclose(File);
    
    Result.Contents[Result.Size - 1] = 0;
    
    return Result;
}

internal jit_code_buffer
InitializeJitBuffer(memory_index BufferSize)
{
    jit_code_buffer JitBuffer;
    JitBuffer.Size = BufferSize;
    JitBuffer.Memory = (u8 *)VirtualAlloc(0, JitBuffer.Size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    JitBuffer.Index = 0;
    JitBuffer.Overflow = FALSE;
    JitBuffer.NumberOfInstructions = 0;
    
    return JitBuffer;
}

#define WriteByteStream(Buffer, ByteArray) WriteByteStream_(Buffer, ByteArray, sizeof(ByteArray))

internal void
WriteByteStream_(jit_code_buffer *Buffer, u8 *Bytes, memory_index Size)
{
    Buffer->Overflow = (Buffer->Index + Size) > Buffer->Size;
    Assert(!Buffer->Overflow);
    if(!Buffer->Overflow)
    {
        for(u32 Index = 0;
            Index < Size;
            ++Index)
        {
            *(Buffer->Memory + Buffer->Index++) = Bytes[Index];
        }
    }
    else
    {
        Buffer->Index += Size;
    }
}

internal void
WriteU32(jit_code_buffer *Buffer, u32 Value)
{
    Buffer->Overflow = (Buffer->Index + sizeof(Value)) > Buffer->Size;
    Assert(!Buffer->Overflow);
    if(!Buffer->Overflow)
    {
        *((u32 *)(Buffer->Memory + Buffer->Index)) = Value;
    }
    
    Buffer->Index += sizeof(Value);
}

internal void
WriteS32(jit_code_buffer *Buffer, s32 Value)
{
    Buffer->Overflow = (Buffer->Index + sizeof(Value)) > Buffer->Size;
    Assert(!Buffer->Overflow);
    if(!Buffer->Overflow)
    {
        *((s32 *)(Buffer->Memory + Buffer->Index)) = Value;
    }
    
    Buffer->Index += sizeof(Value);
}

internal void
WriteU8(jit_code_buffer *Buffer, u8 Value)
{
    Buffer->Overflow = (Buffer->Index + sizeof(Value)) > Buffer->Size;
    Assert(!Buffer->Overflow);
    if(!Buffer->Overflow)
    {
        *((u8 *)(Buffer->Memory + Buffer->Index)) = Value;
    }
    
    Buffer->Index += sizeof(Value);
}

internal b32
ParseAndEmitInstructions(brainf_parser *Parser, jit_code_buffer *Buffer)
{
    b32 Parsing = TRUE;
    
    while(Parsing)
    {
        u8 C = *Parser->At++;
        switch(C)
        {
            case '+':
            {
                // mov rcx, qword ptr[r12]
                u8 BufferMovRcxDerefR12[] = {0x49, 0x8B, 0x0C, 0x24};
                WriteByteStream(Buffer, BufferMovRcxDerefR12);
                
                u32 Counter = 1;
                while(*Parser->At == '+')
                {
                    ++Counter;
                    ++Parser->At;
                }
                
                // add rcx, counter
                u8 BufferAddInstruction[] = {0x48, 0x81, 0xC1};
                WriteByteStream(Buffer, BufferAddInstruction);
                WriteU32(Buffer, Counter);
                
                // mov qword ptr [r12],rcx
                u8 BufferMovDerefR12Rcx[] = {0x49, 0x89, 0x0C, 0x24};
                WriteByteStream(Buffer, BufferMovDerefR12Rcx);
                
                Buffer->NumberOfInstructions += 3;
                
            } break;
            
            case '-':
            {
                // mov rcx, qword ptr[r12]
                u8 BufferMovRcxDerefR12[] = {0x49, 0x8B, 0x0C, 0x24};
                WriteByteStream(Buffer, BufferMovRcxDerefR12);
                
                u32 Counter = 1;
                while(*Parser->At == '-')
                {
                    ++Counter;
                    ++Parser->At;
                }
                
                // sub rcx, counter
                u8 BufferSubInstruction[] = {0x48, 0x81, 0xE9};
                WriteByteStream(Buffer, BufferSubInstruction);
                WriteU32(Buffer, Counter);
                
                // mov qword ptr [r12],rcx
                u8 BufferMovDerefR12Rcx[] = {0x49, 0x89, 0x0C, 0x24};
                WriteByteStream(Buffer, BufferMovDerefR12Rcx);
                
                Buffer->NumberOfInstructions += 3;
            } break;
            
            case '>':
            {
                u32 Counter = 1;
                while(*Parser->At == '>')
                {
                    ++Counter;
                    ++Parser->At;
                }
                
                // add r12, counter
                u8 BufferAddInstruction[] = {0x49, 0x81, 0xC4};
                WriteByteStream(Buffer, BufferAddInstruction);
                WriteU32(Buffer, Counter * sizeof(u64));
                
                ++Buffer->NumberOfInstructions;
            } break;
            
            case '<':
            {
                u32 Counter = 1;
                while(*Parser->At == '<')
                {
                    ++Counter;
                    ++Parser->At;
                }
                
                // sub r12, counter
                u8 BufferSubInstruction[] = {0x49, 0x81, 0xEC};
                WriteByteStream(Buffer, BufferSubInstruction);
                WriteU32(Buffer, Counter * sizeof(u64));
                
                ++Buffer->NumberOfInstructions;
            } break;
            
            case '.':
            {
                // mov rcx, qword ptr[r12]
                u8 BufferMovRcxDerefR12[] = {0x49, 0x8B, 0x0C, 0x24};
                WriteByteStream(Buffer, BufferMovRcxDerefR12);
                
                // NOTE(fuzes): We have to conform to the cdecl calling convention.
                // See: https://en.wikipedia.org/wiki/X86_calling_conventions
                
                // NOTE(fuzes): make new call frame
                // push rbp
                // mov rbp, rsp
                u8 BufferCallFrame[] = {0x55, 0x48, 0x8B, 0xEC};
                WriteByteStream(Buffer, BufferCallFrame);
                
                u8 BufferCallRbx[] = {0xFF, 0xD3};
                WriteByteStream(Buffer, BufferCallRbx);
                
                // NOTE(fuzes): Clean the stack
                // mov rsp,rbp
                // pop rbp
                u8 BufferCleanUp[] = {0x48, 0x8B, 0xE5, 0x5D};
                WriteByteStream(Buffer, BufferCleanUp);
                
                Buffer->NumberOfInstructions += 6;
                
            } break;
            
            case ',':
            {
                // NOTE(fuzes): make new call frame
                // push rbp
                // mov rbp, rsp
                u8 BufferCallFrame[] = {0x55, 0x48, 0x8B, 0xEC};
                WriteByteStream(Buffer, BufferCallFrame);
                
                u8 BufferCallR13[] = {0x41, 0xFF,0xD5};
                WriteByteStream(Buffer, BufferCallR13);
                
                // NOTE(fuzes): The result is in rax. If mov the result into the
                // cellbuffer.
                // mov qword ptr [r12],rax
                u8 BufferMovDerefR12Rax[] = {0x49, 0x89, 0x04, 0x24};
                WriteByteStream(Buffer, BufferMovDerefR12Rax);
                
                // NOTE(fuzes): Clean the stack
                // mov rsp,rbp
                // pop rbp
                u8 BufferCleanUp[] = {0x48, 0x8B, 0xE5, 0x5D};
                WriteByteStream(Buffer, BufferCleanUp);
                
                Buffer->NumberOfInstructions += 5;
            } break;
            
            case '[':
            {
                memory_index LoopStartIndex = Buffer->Index;
                
                // mov rcx, qword ptr[r12]
                u8 BufferMovRcxDerefR12[] = {0x49, 0x8B, 0x0C, 0x24};
                WriteByteStream(Buffer, BufferMovRcxDerefR12);
                
                // test rcx, rcx
                u8 BufferTestRcxRcx[] = {0x48, 0x85, 0xC9};
                WriteByteStream(Buffer, BufferTestRcxRcx);
                
                memory_index JeWriteIndex = Buffer->Index;
                // je end of the while loop
                u8 BufferJe[] = {0x0F, 0x84};
                WriteByteStream(Buffer, BufferJe);
                WriteS32(Buffer, 0x0);
                memory_index JumpIndex = Buffer->Index;
                
                ParseAndEmitInstructions(Parser, Buffer);
                
                // NOTE(fuzes): This is the instruction at the end of the loop.
                // We just calculate the offset to the start of the loop and jump
                // that amount.
                s32 OffsetToStart = -((s32)(Buffer->Index - LoopStartIndex));
                s32 SizeOfJumpInstruction = 5;
                OffsetToStart -= SizeOfJumpInstruction;
                WriteU8(Buffer, 0xE9);
                WriteS32(Buffer, OffsetToStart);
                
                // NOTE(fuzes): Only now we know how big the je jump must be.
                // So we overwrite the instruction with the correct offset.
                memory_index CurrentIndex = Buffer->Index;
                s32 OffsetEndOfWhileLoop = ((s32)(Buffer->Index - JumpIndex));
                
                Buffer->Index = JeWriteIndex;
                // je end of the while loop
                WriteByteStream(Buffer, BufferJe);
                WriteS32(Buffer, OffsetEndOfWhileLoop);
                
                Buffer->Index = CurrentIndex;
                
                Buffer->NumberOfInstructions += 4;
            } break;
            
            case ']':
            {
                Parsing = FALSE;
            } break;
            
            case '\0':
            {
                Parsing = FALSE;
            } break;
        }
    }
    
    return FALSE;
}

internal void
FinalizeJitBufferForExecution(jit_code_buffer *Buffer)
{
    DWORD Old;
    VirtualProtect(Buffer->Memory, Buffer->Size, PAGE_EXECUTE_READ, &Old);
}

void PutCharWrapper(s32 X)
{
    putchar(X);
}

char GetCharWrapper(void)
{
    return getchar();
}

internal void
Usage()
{
    printf("Brainfuck jit compiler written by Fuzes Marcell.\n");
    printf("Usage: brainf_jit [file] [options]\n");
    printf("Options:\n");
    printf("       -debug Print out the byte code and other debug statistics.\n");
    printf("       -cellsize [number (1, 2, 4, 8)-byte] Set the cell size of the buffer.\n");
    printf("       -buffercount [number] Set the count of the buffer e.g cellsize * buffersize\n");
    printf("       -codebuffersize [number e.g(32b, 43kb, 12mb)] Set the size of the jit generated buffer. Default: 128kb\n");
    printf("       -perf Print performance statistics.\n");
}

internal u32
ConvertStringToU32(u8 *Text, u32 Length)
{
    u32 Result = 0;
    
    u32 Base = 1;
    u32 Cof = 10;
    for(u32 Index = 0;
        Index < Length;
        ++Index)
    {
        u32 NumberValue = (u32)(Text[Length - (Index + 1)] - '0');
        Result += NumberValue * Base;
        Base *= Cof;
    }
    
    return Result;
}

internal b32
StringEndsWith(u8 *Text, u8 *Suffix)
{
    b32 Result = FALSE;
    
    memory_index TextLength = strlen(Text);
    memory_index SuffixLength = strlen(Suffix);
    
    if(SuffixLength <= TextLength)
    {
        Result = strncmp(Text + TextLength - SuffixLength, Suffix, SuffixLength) == 0;
    }
    
    return Result;
}

internal b32
ParseArguments(cmd_arguments *Arguments, int ArgCount, char **Args)
{
    b32 Success = TRUE;
    for(u32 ArgumentIndex = 2;
        ArgumentIndex < ArgCount;
        ++ArgumentIndex)
    {
        if(strcmp("-debug", Args[ArgumentIndex]) == 0)
        {
            Arguments->Debug = TRUE;
        }
        else if(strcmp("-cellsize", Args[ArgumentIndex]) == 0)
        {
            if((ArgumentIndex + 1) < ArgCount)
            {
                u32 CellSize = ConvertStringToU32(Args[ArgumentIndex + 1],
                                                  strlen(Args[ArgumentIndex + 1]));
                if(CellSize != 8  ||
                   CellSize != 16 ||
                   CellSize != 32 ||
                   CellSize != 64)
                {
                    printf("Invalid cell size %d.\n", CellSize);
                    Success = FALSE;
                }
            }
            else
            {
                printf("No argument for cellsize parameter\n");
                Success = FALSE;
            }
        }
        else if(strcmp("-buffercount", Args[ArgumentIndex]) == 0)
        {
            if((ArgumentIndex + 1) < ArgCount)
            {
                Arguments->BufferSize = ConvertStringToU32(Args[ArgumentIndex + 1],
                                                           strlen(Args[ArgumentIndex + 1]));
            }
            else
            {
                printf("No argument for buffercount parameter\n");
                Success = FALSE;
            }
        }
        else if(strcmp("-codebuffersize", Args[ArgumentIndex]) == 0)
        {
            if((ArgumentIndex + 1) < ArgCount)
            {
                // NOTE(fuzes): First start parsing from behind to check
                // which size the user defined.
                
                u8 *Text = Args[ArgumentIndex + 1];
                memory_index TextLength = strlen(Text);
                if(StringEndsWith(Text, "b"))
                {
                    Arguments->CodeBufferSize = ConvertStringToU32(Text, TextLength - 1);
                }
                else if(StringEndsWith(Text, "kb"))
                {
                    Arguments->CodeBufferSize = ConvertStringToU32(Text, TextLength - 2) * 1024;
                }
                else if(StringEndsWith(Text, "mb"))
                {
                    Arguments->CodeBufferSize = ConvertStringToU32(Text, TextLength - 2) * 1024 * 1024;
                }
                else
                {
                    printf("Invalid suffix for codebuffersize paramater\n");
                    Success = FALSE;
                }
            }
            else
            {
                printf("No argument for codebuffersize parameter\n");
                Success = FALSE;
            }
        }
        else if(strcmp("-perf", Args[ArgumentIndex]) == 0)
        {
            Arguments->Perf = TRUE;
        }
    }
    
    return Success;
}

int main(int ArgCount, char **Args)
{
    if(ArgCount >= 2)
    {
        cmd_arguments Arguments;
        Arguments.Debug = FALSE;
        Arguments.Perf = FALSE;
        Arguments.CellSize = 64;
        Arguments.BufferSize = Kilobytes(30);
        Arguments.CodeBufferSize = Kilobytes(128);
        
        if(ParseArguments(&Arguments, ArgCount, Args))
        {
            jit_code_buffer JitBuffer = InitializeJitBuffer(Arguments.CodeBufferSize);
            
            // NOTE(fuzes): We move the putchar function into our non volatile register.
            // mov rbx, rdx
            u8 BufferMovRbxRdx[] = {0x48, 0x8B, 0xDA};
            WriteByteStream(&JitBuffer, BufferMovRbxRdx);
            ++JitBuffer.NumberOfInstructions;
            
            // NOTE(fuzes): The brainf main buffer is passed as the first argument so in windows it is in
            // rcx so we move this into a non volatile register.
            // mov rbx, rdx
            u8 BufferMovR12Rcx[] = {0x4C, 0x8B, 0xE1};
            WriteByteStream(&JitBuffer, BufferMovR12Rcx);
            ++JitBuffer.NumberOfInstructions;
            
            // NOTE(fuzes): The third argument is the getchar function so we save it in r13
            u8 BufferMovR13R8[] = {0x4D, 0x8B, 0xE8};
            WriteByteStream(&JitBuffer, BufferMovR13R8);
            ++JitBuffer.NumberOfInstructions;
            
            loaded_file BrainFSourceFile = ReadEntireFileAndZeroTerminate(Args[1]);
            
            brainf_parser Parser;
            Parser.At = BrainFSourceFile.Contents;
            
            while(ParseAndEmitInstructions(&Parser, &JitBuffer)) {}
            
            // ret
            WriteU8(&JitBuffer, 0xC3);
            ++JitBuffer.NumberOfInstructions;
            
            FinalizeJitBufferForExecution(&JitBuffer);
            
            void *BrainfBuffer = calloc(1, Arguments.CellSize * Arguments.BufferSize);
            
            if(!JitBuffer.Overflow)
            {
                if(Arguments.Perf)
                {
                    // NOTE(fuzes): This is required on startup for the windows
                    // query performance counter.
                    LARGE_INTEGER PerfCountFrequencyResult;
                    QueryPerformanceFrequency(&PerfCountFrequencyResult);
                    GlobalPerfCountFrequency = PerfCountFrequencyResult.QuadPart;
                    
                    LARGE_INTEGER StartCounter = Win32GetWallClock();
                    
                    jit_function *JitFunction = (jit_function *)JitBuffer.Memory;
                    s32 Result = JitFunction(BrainfBuffer, PutCharWrapper, GetCharWrapper);
                    
                    LARGE_INTEGER EndCounter = Win32GetWallClock();
                    f32 SecondsElapsed = Win32GetSecondsElapsed(StartCounter, EndCounter);
                    printf("Done in %.03fsec\n", SecondsElapsed);
                }
                else
                {
                    jit_function *JitFunction = (jit_function *)JitBuffer.Memory;
                    s32 Result = JitFunction(BrainfBuffer, PutCharWrapper, GetCharWrapper);
                }
                
                if(Arguments.Debug)
                {
                    printf("Instructions: %d\n", JitBuffer.NumberOfInstructions);
                    printf("Size: %lld bytes", JitBuffer.Index);
                }
            }
            else
            {
                printf("Fatal error: Overflow in instruction buffer.\nConsider increasing the codebuffer.\nRequired size: %lldbytes", JitBuffer.Index);
            }
        }
    }
    else
    {
        Usage();
    }
    
    return 0;
}