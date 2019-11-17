/*
====================================================================
$File: $
$Date: $
$Revision: $
$Createor: Fuzes Marcell $
$Notice: (C) Copyright 2018 by Fuzes Marcell, All Rights Reserved. $
====================================================================
*/

static bool
ParseExpressions(brainfuck_parser *Parser, jit_code_buffer *JitBuffer)
{
    bool Parsing = true;
    bool ParsingWhileLoop = false;
    
    while(Parsing)
    {
        char Token = *Parser->At++;
        
        switch(Token)
        {
            case '+':
            {
                AsmMov(JitBuffer, Register_ID_Rcx, Register_ID_R12, false, true);
                WriteByte(JitBuffer, 0x24);
                
                unsigned int Counter = 1;
                while(*Parser->At == '+')
                {
                    ++Counter;
                    Parser->At++;
                }
                
                AsmAddR32Imm32(JitBuffer, Register_ID_Ecx, Counter);
                
                AsmMov(JitBuffer, Register_ID_R12, Register_ID_Rcx, true, false);
                WriteByte(JitBuffer, 0x24);
                
            } break;
            
            case '-':
            {
                AsmMov(JitBuffer, Register_ID_Rcx, Register_ID_R12, false, true);
                WriteByte(JitBuffer, 0x24);
                
                unsigned int Counter = 1;
                while(*Parser->At == '-')
                {
                    ++Counter;
                    Parser->At++;
                }
                
                AsmSubR32Imm32(JitBuffer, Register_ID_Ecx, Counter);
                
                AsmMov(JitBuffer, Register_ID_R12, Register_ID_Rcx, true, false);
                WriteByte(JitBuffer, 0x24);
                
            } break;
            
            // TODO(fuzes): Altough during runtime we can not track this
            // we can certainly check this during parsing.
            case '>':
            {
                // TODO(fuzes): Just use one add instruction here!
                // inc r12
                WriteByte(JitBuffer, 0x49);
                WriteByte(JitBuffer, 0xFF);
                WriteByte(JitBuffer, 0xC4);
                
                WriteByte(JitBuffer, 0x49);
                WriteByte(JitBuffer, 0xFF);
                WriteByte(JitBuffer, 0xC4);
                
                WriteByte(JitBuffer, 0x49);
                WriteByte(JitBuffer, 0xFF);
                WriteByte(JitBuffer, 0xC4);
                
                WriteByte(JitBuffer, 0x49);
                WriteByte(JitBuffer, 0xFF);
                WriteByte(JitBuffer, 0xC4);
                
                WriteByte(JitBuffer, 0x49);
                WriteByte(JitBuffer, 0xFF);
                WriteByte(JitBuffer, 0xC4);
                
                WriteByte(JitBuffer, 0x49);
                WriteByte(JitBuffer, 0xFF);
                WriteByte(JitBuffer, 0xC4);
                
                WriteByte(JitBuffer, 0x49);
                WriteByte(JitBuffer, 0xFF);
                WriteByte(JitBuffer, 0xC4);
                
                WriteByte(JitBuffer, 0x49);
                WriteByte(JitBuffer, 0xFF);
                WriteByte(JitBuffer, 0xC4);
                
            } break;
            
            case '<':
            {
                // TODO(fuzes): Just use one add instruction here!
                // dec r12
                WriteByte(JitBuffer, 0x49);
                WriteByte(JitBuffer, 0xFF);
                WriteByte(JitBuffer, 0xCC);
                
                WriteByte(JitBuffer, 0x49);
                WriteByte(JitBuffer, 0xFF);
                WriteByte(JitBuffer, 0xCC);
                
                WriteByte(JitBuffer, 0x49);
                WriteByte(JitBuffer, 0xFF);
                WriteByte(JitBuffer, 0xCC);
                
                WriteByte(JitBuffer, 0x49);
                WriteByte(JitBuffer, 0xFF);
                WriteByte(JitBuffer, 0xCC);
                
                WriteByte(JitBuffer, 0x49);
                WriteByte(JitBuffer, 0xFF);
                WriteByte(JitBuffer, 0xCC);
                
                WriteByte(JitBuffer, 0x49);
                WriteByte(JitBuffer, 0xFF);
                WriteByte(JitBuffer, 0xCC);
                
                WriteByte(JitBuffer, 0x49);
                WriteByte(JitBuffer, 0xFF);
                WriteByte(JitBuffer, 0xCC);
                
                WriteByte(JitBuffer, 0x49);
                WriteByte(JitBuffer, 0xFF);
                WriteByte(JitBuffer, 0xCC);
            } break;
            
            case '.':
            {
                AsmMov(JitBuffer, Register_ID_Rcx, Register_ID_R12, false, true);
                WriteByte(JitBuffer, 0x24);
                
                AsmPushRbp(JitBuffer);
                AsmMovRbpRsp(JitBuffer);
                
                AsmCall(JitBuffer, Register_Ebx);
                
                AsmMovRspRbp(JitBuffer);
                AsmPopRbp(JitBuffer);
            } break;
            
            case ',':
            {
                
            } break;
            
            case '[':
            {
                ParsingWhileLoop = true;
                unsigned int LoopStartIndex = JitBuffer->Index;
                
                AsmMov(JitBuffer, Register_ID_Rcx, Register_ID_R12, false, true);
                WriteByte(JitBuffer, 0x24);
                
                // test rcx rcx
                WriteByte(JitBuffer, 0x48);
                WriteByte(JitBuffer, 0x85);
                WriteByte(JitBuffer, 0xC9);
                
                unsigned int JumpWriteIndex = JitBuffer->Index;
                AsmJeRel32(JitBuffer, 0);
                unsigned int JumpIndex = JitBuffer->Index;
                
                ParseExpressions(Parser, JitBuffer);
                
                int OffsetToStart = -((int)(JitBuffer->Index - LoopStartIndex));
                int SizeOfJumpInstruction = 5;
                OffsetToStart -= SizeOfJumpInstruction;
                AsmJump(JitBuffer, OffsetToStart);
                
                unsigned int CurrentIndex = JitBuffer->Index;
                unsigned int OffsetEndOfWhileLoop = ((int)(JitBuffer->Index - JumpIndex));
                
                JitBuffer->Index = JumpWriteIndex;
                AsmJeRel32(JitBuffer, OffsetEndOfWhileLoop);
                
                JitBuffer->Index = CurrentIndex;
                
            } break;
            
            case ']':
            {
                ParsingWhileLoop = false;
                Parsing = false;
            } break;
            
            case '\0':
            {
                Parsing = false;
            } break;
        }
    }
    
    return false;
}