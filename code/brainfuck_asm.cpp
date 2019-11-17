/*
====================================================================
$File: $
$Date: $
$Revision: $
$Createor: Fuzes Marcell $
$Notice: (C) Copyright 2018 by Fuzes Marcell, All Rights Reserved. $
====================================================================
*/

static void
WriteByte(jit_code_buffer *Buffer, unsigned char Number)
{
    Assert((Buffer->Index + sizeof(unsigned char)) <= Buffer->Size);
    
    *(Buffer->Memory + Buffer->Index) = Number;
    Buffer->Index += sizeof(unsigned char);
}

static void
WriteInt(jit_code_buffer *Buffer, unsigned int Number)
{
    Assert((Buffer->Index + sizeof(unsigned int)) <= Buffer->Size);
    
    *((unsigned int *)(Buffer->Memory + Buffer->Index)) = Number;
    Buffer->Index += sizeof(unsigned int);
}

static void
WriteShort(jit_code_buffer *Buffer, unsigned short Number)
{
    Assert((Buffer->Index + sizeof(unsigned short)) <= Buffer->Size);
    
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
AsmMovR64R64(jit_code_buffer *Buffer, register_type DestRegister, register_type SourceRegister)
{
    unsigned char Prefix = 0x48;
    unsigned char OpCode = 0x8B;
    unsigned char ModRM = EncodeModRM(0x3, DestRegister, SourceRegister);
    WriteByte(Buffer, Prefix);
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

enum register_size
{
    Register_Size_Invalid,
    
    Register_Size_QWORD,
    Register_Size_DWORD,
    Register_Size_WORD,
    Register_Size_BYTE,
};

static register_size
GetRegisterSize(register_id Register)
{
    register_size Result = Register_Size_Invalid;
    
    if(Register < Register_ID_QWORD_MAX)
    {
        Result = Register_Size_QWORD;
    }
    else if(Register < Register_ID_DWORD_MAX &&
            Register >= Register_ID_Eax)
    {
        Result = Register_Size_DWORD;
    }
    else if(Register < Register_ID_WORD_MAX &&
            Register >= Register_ID_Ax)
    {
        Result = Register_Size_WORD;
    }
    else if(Register < Register_ID_BYTE_MAX &&
            Register >= Register_ID_Al)
    {
        Result = Register_Size_BYTE;
    }
    
    return Result;
}

static bool
IsRegisterExtended(register_id Register)
{
    bool Result = false;
    
    if(Register >= Register_ID_R8 &&
       Register < Register_ID_QWORD_MAX)
    {
        Result = true;
    }
    else if(Register >= Register_ID_R8D &&
            Register < Register_ID_DWORD_MAX)
    {
        Result = true;
    }
    else if(Register >= Register_ID_R8W &&
            Register < Register_ID_WORD_MAX)
    {
        Result = true;
    }
    else if(Register >= Register_ID_R8B &&
            Register < Register_ID_BYTE_MAX)
    {
        Result = true;
    }
    
    return Result;
}

static char
GetRegisterByteValue(register_id Register, bool IsExtendedRegister, register_size Size)
{
    register_id MinExtendedID = Register_Invalid;
    register_id MinID = Register_Invalid;
    switch(Size)
    {
        case Register_Size_QWORD:
        {
            MinExtendedID = Register_ID_R8;
            MinID = Register_ID_Rax;
        } break;
        
        case Register_Size_DWORD:
        {
            MinExtendedID = Register_ID_R8D;
            MinID = Register_ID_Eax;
        } break;
        
        case Register_Size_WORD:
        {
            MinExtendedID = Register_ID_R8W;
            MinID = Register_ID_Ax;
        } break;
        
        case Register_Size_BYTE:
        {
            MinExtendedID = Register_ID_R8B;
            MinID = Register_ID_Al;
        } break;
    }
    
    char Result = 0xFF;
    
    if(IsExtendedRegister)
    {
        Result = Register - MinExtendedID;
    }
    else
    {
        Result = Register - MinID;
    }
    
    return Result;
}

#define XORSwap(A, B)       \
((A) ^= (B));               \
((B) ^= (A));               \
((A) ^= (B));

static void
AsmMov(jit_code_buffer *Buffer, register_id DestRegID, register_id SourceRegID,
       bool IsDestDereference = false, bool IsSourceDereference = false)
{
    // TODO(fuzes): Assert this!
    register_size DestRegSize = GetRegisterSize(DestRegID);
    register_size SourceRegSize = GetRegisterSize(SourceRegID);
    
    Assert(DestRegSize == SourceRegSize);
    Assert(!((IsDestDereference == true) && (IsSourceDereference == true)));
    
    bool ShouldBeDereferenced = IsDestDereference || IsSourceDereference;
    char ModValue = ShouldBeDereferenced ? 0x0 : 0x3;
    bool IsDestRegExtension = IsRegisterExtended(DestRegID);
    bool IsSourceRegExtension = IsRegisterExtended(SourceRegID);
    
    unsigned char R1Value = GetRegisterByteValue(DestRegID, IsDestRegExtension, DestRegSize);
    unsigned char R2Value = GetRegisterByteValue(SourceRegID, IsSourceRegExtension, SourceRegSize);
    
    switch(DestRegSize)
    {
        case Register_Size_QWORD:
        {
            unsigned char OpCode = 0x8B;
            if(IsDestDereference)
            {
                XORSwap(IsDestRegExtension, IsSourceRegExtension);
                XORSwap(R1Value, R2Value);
                OpCode = 0x89;
            }
            
            unsigned char Prefix = EncodePrefix(0x1, (int)IsDestRegExtension, 0x0, (int)IsSourceRegExtension);
            
            unsigned char ModRM = EncodeModRM(ModValue, R1Value, R2Value);
            
            WriteByte(Buffer, Prefix);
            WriteByte(Buffer, OpCode);
            WriteByte(Buffer, ModRM);
            
        } break;
        
        case Register_Size_DWORD:
        case Register_Size_WORD:
        {
            if(IsDestRegExtension || IsSourceRegExtension)
            {
                unsigned char Prefix = EncodePrefix(0x0, (int)IsDestRegExtension, 0x0, (int)IsSourceRegExtension);
                WriteByte(Buffer, Prefix);
            }
            
            unsigned char OpCode = 0x8B;
            unsigned char ModRM = EncodeModRM(0x3, R1Value, R2Value);
            
            WriteByte(Buffer, OpCode);
            WriteByte(Buffer, ModRM);
            
        } break;
        
        case Register_Size_BYTE:
        {
            if(IsDestRegExtension || IsSourceRegExtension)
            {
                unsigned char Prefix = EncodePrefix(0x0, (int)IsDestRegExtension, 0x0, (int)IsSourceRegExtension);
                WriteByte(Buffer, Prefix);
            }
            
            unsigned char OpCode = 0x8A;
            unsigned char ModRM = EncodeModRM(0x3, R1Value, R2Value);
            
            WriteByte(Buffer, OpCode);
            WriteByte(Buffer, ModRM);
        } break;
    }
}

static void
AsmAddR32Imm32(jit_code_buffer *Buffer, register_id RegisterID, unsigned int Value)
{
    register_size RegisterSize = GetRegisterSize(RegisterID);
    bool IsExtension = IsRegisterExtended(RegisterID);
    unsigned char RegisterValue = GetRegisterByteValue(RegisterID, IsExtension, RegisterSize);
    
    Assert(RegisterSize == Register_Size_DWORD);
    
    if(IsExtension)
    {
        unsigned char Prefix = EncodePrefix(0x0, 0x0, 0x0, (int)IsExtension);
        WriteByte(Buffer, Prefix);
    }
    
    unsigned char OpCode = 0x81;
    unsigned char ModRM = EncodeModRM(0x3, 0x0, RegisterValue);
    
    WriteByte(Buffer, OpCode);
    WriteByte(Buffer, ModRM);
    WriteInt(Buffer, Value);
}

static void
AsmSubR32Imm32(jit_code_buffer *Buffer, register_id RegisterID, unsigned int Value)
{
    register_size RegisterSize = GetRegisterSize(RegisterID);
    bool IsExtension = IsRegisterExtended(RegisterID);
    unsigned char RegisterValue = GetRegisterByteValue(RegisterID, IsExtension, RegisterSize);
    
    Assert(RegisterSize == Register_Size_DWORD);
    if(IsExtension)
    {
        unsigned char Prefix = EncodePrefix(0x0, 0x0, 0x0, (int)IsExtension);
        WriteByte(Buffer, Prefix);
    }
    
    unsigned char OpCode = 0x81;
    unsigned char ModRM = EncodeModRM(0x3, 0x5, RegisterValue);
    
    WriteByte(Buffer, OpCode);
    WriteByte(Buffer, ModRM);
    WriteInt(Buffer, Value);
}

static void
AsmJump(jit_code_buffer *Buffer, unsigned int Value)
{
    unsigned char OpCode = 0xE9;
    WriteByte(Buffer, OpCode);
    WriteInt(Buffer, Value);
}