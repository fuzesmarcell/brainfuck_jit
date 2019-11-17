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
                
                AsmAddR32Imm32(JitBuffer, Register_ID_Ecx, 0x1);
                
                AsmMov(JitBuffer, Register_ID_R12, Register_ID_Rcx, true, false);
                WriteByte(JitBuffer, 0x24);
                
            } break;
            
            case '-':
            {
                AsmMov(JitBuffer, Register_ID_Rcx, Register_ID_R12, false, true);
                WriteByte(JitBuffer, 0x24);
                
                AsmSubR32Imm32(JitBuffer, Register_ID_Ecx, 0x1);
                
                AsmMov(JitBuffer, Register_ID_R12, Register_ID_Rcx, true, false);
                WriteByte(JitBuffer, 0x24);
                
            } break;
            
            // TODO(fuzes): Altough during runtime we can not track this
            // we can certainly check this during parsing.
            case '>':
            {
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
            } break;
            
            case '<':
            {
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
            } break;
            
            case '.':
            {
                
            } break;
            
            case ',':
            {
                
            } break;
            
            case '[':
            {
                ParsingWhileLoop = true;
                ParseExpressions(Parser, JitBuffer);
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
        
        if(ParsingWhileLoop)
        {
            printf("Unmatched closing bracket\n");
        }
    }
    
    return false;
}