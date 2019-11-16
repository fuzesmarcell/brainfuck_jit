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

//
// 
//

#define EncodeModRM(Mod, Reg, RM) (((Mod) << 6) | ((Reg) << 3) | (RM))

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
};

enum register_mod_type : unsigned char
{
    Register_Mod_Eax = 0xC0,
    Register_Mod_Ecx = 0xC1,
    Register_Mod_Edx = 0xC2,
    Register_Mod_Ebx = 0xC3,
};
