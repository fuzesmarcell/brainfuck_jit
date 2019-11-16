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

#include <windows.h>
#include <stdio.h>

static void
WriteByte(jit_code_buffer *Buffer, unsigned char Number)
{
    *(Buffer->Memory + Buffer->Index) = Number;
    Buffer->Index += sizeof(unsigned char);
}

static void
WriteInt(jit_code_buffer *Buffer, unsigned int Number)
{
    *((unsigned int *)(Buffer->Memory + Buffer->Index)) = Number;
    Buffer->Index += sizeof(unsigned int);
}

static void
WriteShort(jit_code_buffer *Buffer, unsigned short Number)
{
    *((unsigned short *)(Buffer->Memory + Buffer->Index)) = Number;
    Buffer->Index += sizeof(unsigned short);
}

static unsigned char
AddRegisterToOpcode(unsigned char OpCode, register_type Reg)
{
    unsigned char Result;
    Result = (OpCode & 0xF8) | ((unsigned char)Reg);
    return Result;
}

static void
AsmMovR32Imm32(jit_code_buffer *Buffer, register_type Register, unsigned int Number)
{
    unsigned char OpCode = AddRegisterToOpcode(0xB8, Register);
    WriteByte(Buffer, OpCode);
    WriteInt(Buffer, Number);
}

static void
AsmMovR32R32RM(jit_code_buffer *Buffer, 
               register_type DestinationRegister, register_mod_type SourceRegister)
{
    unsigned char OpCode = 0x8B;
    unsigned char ModRM = (unsigned char)SourceRegister | (DestinationRegister << 3);
    WriteByte(Buffer, OpCode);
    WriteByte(Buffer, ModRM);
}

static void
AsmMovZx(jit_code_buffer *Buffer, register_type DestRegister, register_type SourceRegister)
{
    unsigned short OpCode = SWAP_SHORT(0x0FB6);
    unsigned char ModRM = SourceRegister | (DestRegister << 3);
    WriteShort(Buffer, OpCode);
    WriteByte(Buffer, ModRM);
}

static void
AsmMovR8R8MR(jit_code_buffer *Buffer, register_type DestRegister, register_type SourceRegister)
{
    unsigned char OpCode = 0x88;
    unsigned char ModRM =  DestRegister | (SourceRegister << 3);
    WriteByte(Buffer, OpCode);
    WriteByte(Buffer, ModRM);
}

AsmMovR64R64(jit_code_buffer *Buffer, register_type DestRegister, register_type SourceRegister)
{
    unsigned char Prefix = 0x48;
    unsigned char OpCode = 0x8B;
    unsigned char ModRM = EncodeModRM(0x3, DestRegister, SourceRegister);
    WriteByte(Buffer, Prefix);
    WriteByte(Buffer, OpCode);
    WriteByte(ModRM);
}

static void
AsmRet(jit_code_buffer *Buffer)
{
    WriteByte(Buffer, 0xC3);
}

static void
AsmIncR32(jit_code_buffer *Buffer, register_mod_type Register)
{
    unsigned char OpCode = 0xFF;
    unsigned char ModRM = (unsigned char)Register;
    WriteByte(Buffer, OpCode);
    WriteByte(Buffer, ModRM);
}

static void
AsmIncR64(jit_code_buffer *Buffer, register_mod_type Register)
{
    // TODO(fuzes): Why is this the prefix I do not get it.
    unsigned char Prefix = 0x48;
    unsigned char OpCode = 0xFF;
    unsigned char ModRM = (unsigned char)Register;
    WriteByte(Buffer, Prefix);
    WriteByte(Buffer, OpCode);
    WriteByte(Buffer, ModRM);
}

static void
AsmDecR32(jit_code_buffer *Buffer, register_mod_type Register)
{
    unsigned char OpCode = 0xFF;
    unsigned char ModRM = (unsigned char)Register | (1 << 3);
    WriteByte(Buffer, OpCode);
    WriteByte(Buffer, ModRM);
}

static void
AsmDecR64(jit_code_buffer *Buffer, register_mod_type Register)
{
    unsigned char Prefix = 0x48;
    unsigned char OpCode = 0xFF;
    unsigned char ModRM = (unsigned char)Register | (1 << 3);
    WriteByte(Buffer, Prefix);
    WriteByte(Buffer, OpCode);
    WriteByte(Buffer, ModRM);
}

static void
AsmJmp32(jit_code_buffer *Buffer, int Offset)
{
    unsigned char OpCode = 0xE9;
    WriteByte(Buffer, OpCode);
    
    // NOTE(fuzes): The offset is calculated based on the instruction pointer
    // which is residing after this instruction so we have to add the size of the instruction
    // to the offset which we get passed.
    if(Offset <= 0)
    {
        Offset -= 5;
    }
    else
    {
        Offset += 5;
    }
    
    // NOTE(fuzes): For some reason I do not understand why there is no need to swap the Offset
    // We had to swap the opcode...
    WriteInt(Buffer, Offset);
}

static void
AsmJeRel32(jit_code_buffer *Buffer, int Offset)
{
    unsigned int SizeOfInstruction = sizeof(int) + sizeof(short);
    unsigned short OpCode = SWAP_SHORT(0x0F84);
    if(Offset <= 0)
    {
        Offset -= SizeOfInstruction;
    }
    else
    {
        Offset += SizeOfInstruction;
    }
    
    WriteShort(Buffer, OpCode);
    WriteInt(Buffer, Offset);
}

static void
AsmTestR8R8(jit_code_buffer *Buffer, register_type R1, register_type R2)
{
    unsigned char OpCode = 0x84;
    unsigned char ModRM = EncodeModRM(0x3, R1, R2);
    WriteByte(Buffer, OpCode);
    WriteByte(Buffer, ModRM);
}

static void
AsmCall(jit_code_buffer *Buffer, register_type Register)
{
    unsigned char OpCode = 0xFF;
    unsigned char ModRM = EncodeModRM(0x3, 0x2, Register);
    WriteByte(Buffer, OpCode);
    WriteByte(Buffer, ModRM);
}

static void
AsmPush(jit_code_buffer *Buffer, register_type Register)
{
    unsigned char OpCode = 0x55;
    WriteByte(Buffer, OpCode);
}

static void
AsmPushRbp(jit_code_buffer *Buffer)
{
    WriteByte(Buffer, 0x55);
}

static void
AsmMovRbpRsp(jit_code_buffer *Buffer)
{
    WriteByte(Buffer, 0x48);
    WriteByte(Buffer, 0x8B);
    WriteByte(Buffer, 0xEC);
}

static void
AsmMovRspRbp(jit_code_buffer *Buffer)
{
    WriteByte(Buffer, 0x48);
    WriteByte(Buffer, 0x8B);
    WriteByte(Buffer, 0xE5);
}

static void
AsmPopRbp(jit_code_buffer *Buffer)
{
    WriteByte(Buffer, 0x5D);
}

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
    
    fread(Result.Contents, Result.Size, 1, File);
    fclose(File);
    
    Result.Contents[Result.Size - 1] = 0;
    
    return Result;
}

int main(int ArgCount, char **Args)
{
    
#if 1    
    jit_code_buffer JitBuffer;
    JitBuffer.Size = 2048;
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
    
    AsmPushRbp(&JitBuffer);
    AsmMovRbpRsp(&JitBuffer);
    
    AsmMovR32Imm32(&JitBuffer, Register_Ecx, 0x41);
    AsmCall(&JitBuffer, Register_Edx);
    
    AsmMovRspRbp(&JitBuffer);
    AsmPopRbp(&JitBuffer);
    
    AsmRet(&JitBuffer);
    
    if(ArgCount == 3)
    {
        if(strcmp(Args[1], "-dump") == 0)
        {
            FILE *File = fopen(Args[2], "wb");
            fwrite(JitBuffer.Memory, JitBuffer.Index, 1, File);
            fclose(File);
            printf("Dumped contents to %s (%d bytes)", Args[2], JitBuffer.Index);
        }
    }
    else
    {
        DWORD Old;
        VirtualProtect(JitBuffer.Memory, JitBuffer.Size, PAGE_EXECUTE_READ, &Old);
        jit_function *JitFunction = (jit_function *)JitBuffer.Memory;
        
        unsigned char BrainFuckBuffer[30000] = {};
        int Result = JitFunction(BrainFuckBuffer, PutCharInt);
        printf("%i", BrainFuckBuffer[0]);
    }
#endif
    
#if 0    
    if(ArgCount >= 2)
    {
        file SourceFile = ReadEntireFileAndZeroTerminate(Args[1]);
    }
#endif
    
    return 0;
}