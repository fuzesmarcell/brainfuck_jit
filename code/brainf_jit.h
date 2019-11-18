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
#include <stdint.h>

#define internal static

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef s32 b32;

typedef float f32;
typedef double f64;

typedef size_t memory_index;

//
// NOTE(fuzes): Some simple macros
//

#define SWAP_SHORT(Value) ((Value << 8) | ((Value & 0xFF00) >> 8))
#define SWAP_INT(Value) (((Value & 0xFF000000) >> 24) | ((Value & 0x00FF0000) >> 8) | ((Value & 0x0000FF00) << 8) | (Value << 24))

#define Bytes(V) V
#define Kilobytes(V) (Bytes(V) * 1024LL)
#define Megabytes(V) (Kilobytes(V) * 1024LL)
#define Gigabytes(V) (Megabytes(V) * 1024LL)
#define Terabytes(V) (Gigabytes(V) * 1024LL)

#define TRUE 1
#define FALSE 0

#if DEBUG_BUILD
#define Assert(Expression) if(!(Expression)){int *AFFFFFF = 0; *AFFFFFF = 69;}
#else
#define Assert(Expression)
#endif

//
//
//

typedef struct loaded_file loaded_file;
struct loaded_file
{
    u8 *Contents;
    memory_index Size;
};

typedef struct brainf_parser brainf_parser;
struct brainf_parser
{
    u8 *At;
};

typedef int jit_function(void *Buffer, void *PutChar, void *GetChar);

typedef struct jit_code_buffer jit_code_buffer;
struct jit_code_buffer
{
    u8 *Memory;
    memory_index Size;
    memory_index Index;
    
    b32 Overflow;
    
    u32 NumberOfInstructions;
};

typedef struct cmd_arguments cmd_arguments;
struct cmd_arguments
{
    b32 Debug;
    b32 Perf;
    memory_index CellSize;
    memory_index BufferSize;
    memory_index CodeBufferSize;
};