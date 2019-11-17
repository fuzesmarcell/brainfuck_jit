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

int main(int ArgCount, char **Args)
{
    
    jit_code_buffer JitBuffer;
    JitBuffer.Size = 16384;
    JitBuffer.Memory = (unsigned char *)VirtualAlloc(0, JitBuffer.Size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    JitBuffer.Index = 0;
    
    // TODO(fuzes): We should not have to use the mod register in the parameters it just feels
    // uneccassary for the user to distinguish these two things.
    
#if 0    
    // NOTE(fuzes): Dereference the ecx pointer which contains our buffer which
    // we pass from the JitFunction().
    AsmMovZx(&JitBuffer, Register_Ebx, Register_Ecx);
    
    
    // NOTE(fuzes): This is were we are actually are doing
    // ++*ptr;
    AsmIncR32(&JitBuffer, Register_Mod_Ebx);
    AsmMovR8R8MR(&JitBuffer, Register_Ecx, Register_Ebx);
    
    // ++ptr;
    AsmIncR64(&JitBuffer, Register_Mod_Ecx);
    
    // ++*ptr;
    AsmMovZx(&JitBuffer, Register_Ebx, Register_Ecx);
    AsmIncR32(&JitBuffer, Register_Mod_Ebx);
    AsmMovR8R8MR(&JitBuffer, Register_Ecx, Register_Ebx);
    
    // --ptr;
    AsmDecR64(&JitBuffer, Register_Mod_Ecx);
    
    // ++*ptr;
    AsmMovZx(&JitBuffer, Register_Ebx, Register_Ecx);
    AsmIncR32(&JitBuffer, Register_Mod_Ebx);
    AsmMovR8R8MR(&JitBuffer, Register_Ecx, Register_Ebx);
    
    //AsmJmp32(&JitBuffer, (-JitBuffer.Index));
    AsmMovR32Imm32(&JitBuffer, Register_Ebx, 0x1);
    AsmTestR8R8(&JitBuffer, Register_Ebx, Register_Ebx);
    AsmJeRel32(&JitBuffer, (-JitBuffer.Index));
#endif
    
#if 0    
    AsmMov(&JitBuffer, Register_ID_R12, Register_ID_R8);
    AsmMov(&JitBuffer, Register_ID_Rax, Register_ID_Rbx);
    AsmMov(&JitBuffer, Register_ID_R12D, Register_ID_R8D);
    AsmMov(&JitBuffer, Register_ID_Eax, Register_ID_R12D);
    AsmMov(&JitBuffer, Register_ID_Ax, Register_ID_R12W);
    AsmMov(&JitBuffer, Register_ID_Al, Register_ID_R12B);
#endif
    
#if 0    
    // NOTE(fuzes): We store in RDX: The putchar function pointer
    // We store in r12 the buffer of the brainfuck programm.
    AsmMov(&JitBuffer, Register_ID_Rbx, Register_ID_Rdx);
    AsmMov(&JitBuffer, Register_ID_R12, Register_ID_Rcx);
    
    AsmMov(&JitBuffer, Register_ID_Rcx, Register_ID_R12, false, true);
    WriteByte(&JitBuffer, 0x24);
    
    AsmAddR32Imm32(&JitBuffer, Register_ID_Ecx, 'R');
    AsmSubR32Imm32(&JitBuffer, Register_ID_Ecx, 1);
    
    AsmPushRbp(&JitBuffer);
    AsmMovRbpRsp(&JitBuffer);
    
    AsmCall(&JitBuffer, Register_Ebx);
    
    AsmMovRspRbp(&JitBuffer);
    AsmPopRbp(&JitBuffer);
    
    AsmRet(&JitBuffer);
#endif
    
    if(ArgCount >= 2)
    {
        if(strcmp(Args[1], "-dump") == 0)
        {
            FILE *File = fopen(Args[2], "wb");
            fwrite(JitBuffer.Memory, JitBuffer.Index, 1, File);
            fclose(File);
            printf("Dumped contents to %s (%d bytes)", Args[2], JitBuffer.Index);
        }
        else if(strcmp(Args[1], "-print") == 0)
        {
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
            file SourceFile = ReadEntireFileAndZeroTerminate(Args[1]);
            
            AsmMov(&JitBuffer, Register_ID_Rbx, Register_ID_Rdx);
            AsmMov(&JitBuffer, Register_ID_R12, Register_ID_Rcx);
            
            brainfuck_parser Parser;
            Parser.At = SourceFile.Contents;
            
            while(ParseExpressions(&Parser, &JitBuffer))
            {
                
            }
            
            AsmPushRbp(&JitBuffer);
            AsmMovRbpRsp(&JitBuffer);
            
            AsmCall(&JitBuffer, Register_Ebx);
            
            AsmMovRspRbp(&JitBuffer);
            AsmPopRbp(&JitBuffer);
            
            AsmRet(&JitBuffer);
            
            DWORD Old;
            VirtualProtect(JitBuffer.Memory, JitBuffer.Size, PAGE_EXECUTE_READ, &Old);
            jit_function *JitFunction = (jit_function *)JitBuffer.Memory;
            
            int BrainFuckBuffer[30000] = {};
            int Result = JitFunction(BrainFuckBuffer, PutCharInt);
            printf("%d\n", BrainFuckBuffer[0]);
            printf("%d\n", BrainFuckBuffer[1]);
            printf("%d\n", BrainFuckBuffer[2]);
            printf("%d\n", BrainFuckBuffer[3]);
        }
    }
    else
    {
        DWORD Old;
        VirtualProtect(JitBuffer.Memory, JitBuffer.Size, PAGE_EXECUTE_READ, &Old);
        jit_function *JitFunction = (jit_function *)JitBuffer.Memory;
        
        unsigned char BrainFuckBuffer[30000] = {};
        int Result = JitFunction(BrainFuckBuffer, PutCharInt);
        printf("%x", BrainFuckBuffer[0]);
    }
    
    return 0;
}