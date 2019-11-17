/*
====================================================================
$File: $
$Date: $
$Revision: $
$Createor: Fuzes Marcell $
$Notice: (C) Copyright 2018 by Fuzes Marcell, All Rights Reserved. $
====================================================================
*/

#include "brainfuck_jit.h"
#include "brainfuck_asm.cpp"

#include <windows.h>
#include <stdio.h>

#include "brainfuck_parser.h"
#include "brainfuck_parser.cpp"

PUT_CHARINT(PutCharInt)
{
    putchar(x);
}

struct file
{
    char *Contents;
    int Size;
};

static file
ReadEntireFileAndZeroTerminate(char *FileName)
{
    file Result;
    
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

static void
Usage(void)
{
    printf("Brainfuck jit compiler by Fuzes Marcell\n");
    printf("Usage:\n");
    printf(" brainfuck_jit [brainfuck source file] [...]\n");
    printf(" -dump [filename]    Dump jit compiled byte code to file\n");
    printf(" -print              Print the jit compiled byte code\n");
}

int main(int ArgCount, char **Args)
{
    if(ArgCount >= 2)
    {
        bool DumpToFile = false;
        bool PrintOut = false;
        if(ArgCount == 3)
        {
            if(strcmp(Args[2], "-dump") == 0)
            {
                DumpToFile = true;
            }
            if(strcmp(Args[2], "-print") == 0)
            {
                PrintOut = true;
            }
        }
        
        file SourceFile = ReadEntireFileAndZeroTerminate(Args[1]);
        
        // TODO(fuzes): How should we calculate what our buffer size should be
        // or do we want it to simply grow ?
        jit_code_buffer JitBuffer;
        JitBuffer.Size = Megabytes(1);
        JitBuffer.Memory = (unsigned char *)VirtualAlloc(0, JitBuffer.Size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
        JitBuffer.Index = 0;
        
        AsmMov(&JitBuffer, Register_ID_Rbx, Register_ID_Rdx);
        AsmMov(&JitBuffer, Register_ID_R12, Register_ID_Rcx);
        
        brainfuck_parser Parser;
        Parser.At = SourceFile.Contents;
        
        while(ParseExpressions(&Parser, &JitBuffer))
        {
            
        }
        
        AsmRet(&JitBuffer);
        
        DWORD Old;
        VirtualProtect(JitBuffer.Memory, JitBuffer.Size, PAGE_EXECUTE_READ, &Old);
        jit_function *JitFunction = (jit_function *)JitBuffer.Memory;
        
        if(DumpToFile)
        {
            FILE *File = fopen(Args[2], "wb");
            fwrite(JitBuffer.Memory, JitBuffer.Index, 1, File);
            fclose(File);
            printf("Dumped contents to %s (%d bytes)", Args[3], JitBuffer.Index);
        }
        else if(PrintOut)
        {
            printf("Jit compiled byte code (%d bytes):\n", JitBuffer.Index);
            for(int Index = 0;
                Index < JitBuffer.Index;
                ++Index)
            {
                printf("%02X ", JitBuffer.Memory[Index]);
                if((((Index + 1) % 8) == 0) && (Index != 0))
                {
                    printf("\n");
                }
            }
        }
        else
        {
            long long BrainFuckBuffer[30000] = {};
            JitFunction(BrainFuckBuffer, PutCharInt);
        }
    }
    else
    {
        Usage();
    }
    
    return 0;
}