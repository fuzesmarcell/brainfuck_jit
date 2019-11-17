/*
====================================================================
$File: $
$Date: $
$Revision: $
$Createor: Fuzes Marcell $
$Notice: (C) Copyright 2018 by Fuzes Marcell, All Rights Reserved. $
====================================================================
*/

#pragma once
#define SWAP_SHORT(Value) ((Value << 8) | ((Value & 0xFF00) >> 8))
#define SWAP_INT(Value) (((Value & 0xFF000000) >> 24) | ((Value & 0x00FF0000) >> 8) | ((Value & 0x0000FF00) << 8) | (Value << 24))

#define Bytes(V) V
#define Kilobytes(V) (Bytes(V) * 1024LL)
#define Megabytes(V) (Kilobytes(V) * 1024LL)
#define Gigabytes(V) (Megabytes(V) * 1024LL)
#define Terabytes(V) (Gigabytes(V) * 1024LL)

#define Assert(Expression) if(!(Expression)){int *AFFFFFF = 0; *AFFFFFF = 69;}

//
// 
//

#define EncodeModRM(Mod, Reg, RM) (((Mod) << 6) | ((Reg) << 3) | (RM))
#define EncodePrefix(W, R, X, B) ((0x4 << 4) | ((W) << 3) | ((R) << 2) | ((X) << 1) | ((B)))

#define PUT_CHARINT(Name) void Name(int x)
typedef PUT_CHARINT(put_char_int);

typedef int jit_function(void *Buffer, put_char_int *PutCharF);


struct jit_code_buffer
{
    unsigned char *Memory;
    unsigned int Index;
    unsigned int Size;
};

enum register_type : unsigned char
{
    Register_Eax,
    Register_Ecx,
    Register_Edx,
    Register_Ebx,
    Register_Esp,
    Register_Ebp,
    Register_Esi,
    Register_Edi,
};

enum register_mod_type : unsigned char
{
    Register_Mod_Eax = 0xC0,
    Register_Mod_Ecx = 0xC1,
    Register_Mod_Edx = 0xC2,
    Register_Mod_Ebx = 0xC3,
};

enum register_id
{
    Register_ID_Rax,
    Register_ID_Rcx,
    Register_ID_Rdx,
    Register_ID_Rbx,
    Register_ID_R8,
    Register_ID_R9,
    Register_ID_R10,
    Register_ID_R11,
    Register_ID_R12,
    Register_ID_R13,
    Register_ID_R14,
    Register_ID_R15,
    
    Register_ID_QWORD_MAX,
    
    Register_ID_Eax,
    Register_ID_Ecx,
    Register_ID_Edx,
    Register_ID_Ebx,
    Register_ID_R8D,
    Register_ID_R9D,
    Register_ID_R10D,
    Register_ID_R11D,
    Register_ID_R12D,
    Register_ID_R13D,
    Register_ID_R14D,
    Register_ID_R15D,
    
    Register_ID_DWORD_MAX,
    
    Register_ID_Ax,
    Register_ID_Cx,
    Register_ID_Dx,
    Register_ID_Bx,
    Register_ID_R8W,
    Register_ID_R9W,
    Register_ID_R10W,
    Register_ID_R11W,
    Register_ID_R12W,
    Register_ID_R13W,
    Register_ID_R14W,
    Register_ID_R15W,
    
    Register_ID_WORD_MAX,
    
    Register_ID_Al,
    Register_ID_Cl,
    Register_ID_Dl,
    Register_ID_Bl,
    Register_ID_R8B,
    Register_ID_R9B,
    Register_ID_R10B,
    Register_ID_R11B,
    Register_ID_R12B,
    Register_ID_R13B,
    Register_ID_R14B,
    Register_ID_R15B,
    
    Register_ID_BYTE_MAX,
    
    
    Register_Invalid,
};