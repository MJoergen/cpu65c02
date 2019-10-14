#include <iostream>
#include <iomanip>
#include "cpu6502.h"

// List of all the possible instructions.
typedef enum
{
    I_RES,  // Reserved
    I_ORA,
    I_AND,
    I_EOR,
    I_ADC,
    I_STA,
    I_LDA,
    I_CMP,
    I_SBC,
    I_CLC,
    I_SEC,
    I_CLI,
    I_SEI,
    I_CLV,
    I_CLD,
    I_SED,
    I_BPL,
    I_BMI,
    I_BVC,
    I_BVS,
    I_BCC,
    I_BCS,
    I_BNE,
    I_BEQ,
    I_JMP,
    I_PHP,
    I_JSR,
    I_PLP,
    I_PHA,
    I_RTS,
    I_PLA,
    I_ASLA, 
    I_ASL, 
    I_ROL,
    I_LSR,
    I_ROR,
    I_DEC, 
    I_INC,
    I_BIT
} instruction_t;

// List of all the possible addressing modes.
typedef enum
{
    AM_NONE,
    AM_IMM,
    AM_ABS,
    AM_ZP,
    AM_REL
} addrMode_t;

typedef enum
{
    ALU_ORA,
    ALU_AND,
    ALU_EOR,
    ALU_ADC,
    ALU_LDA,
    ALU_CMP,
    ALU_SBC,
    ALU_ASL,
    ALU_ROL,
    ALU_LSR,
    ALU_ROR,
    ALU_DEC,
    ALU_INC,
    ALU_BIT
} aluMode_t;


static const addrMode_t addrModes[256] = 
{
    AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_ZP,   AM_ZP,   AM_NONE,  AM_NONE, AM_IMM,  AM_NONE, AM_NONE, AM_NONE, AM_ABS,  AM_ABS,  AM_NONE, // 0x00
    AM_REL,  AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE,  AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE, // 0x10
    AM_ABS,  AM_NONE, AM_NONE, AM_NONE, AM_ZP,   AM_ZP,   AM_ZP,   AM_NONE,  AM_NONE, AM_IMM,  AM_NONE, AM_NONE, AM_ABS,  AM_ABS,  AM_ABS,  AM_NONE, // 0x20
    AM_REL,  AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE,  AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE, // 0x30
    AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_ZP,   AM_ZP,   AM_NONE,  AM_NONE, AM_IMM,  AM_NONE, AM_NONE, AM_ABS,  AM_ABS,  AM_ABS,  AM_NONE, // 0x40
    AM_REL,  AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE,  AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE, // 0x50
    AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_ZP,   AM_ZP,   AM_NONE,  AM_NONE, AM_IMM,  AM_NONE, AM_NONE, AM_NONE, AM_ABS,  AM_ABS,  AM_NONE, // 0x60
    AM_REL,  AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE,  AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE, // 0x70
    AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_ZP,   AM_NONE, AM_NONE,  AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_ABS,  AM_NONE, AM_NONE, // 0x80
    AM_REL,  AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE,  AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE, // 0x90
    AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_ZP,   AM_NONE, AM_NONE,  AM_NONE, AM_IMM,  AM_NONE, AM_NONE, AM_NONE, AM_ABS,  AM_NONE, AM_NONE, // 0xA0
    AM_REL,  AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE,  AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE, // 0xB0
    AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_ZP,   AM_ZP,   AM_NONE,  AM_NONE, AM_IMM,  AM_NONE, AM_NONE, AM_NONE, AM_ABS,  AM_ABS,  AM_NONE, // 0xC0
    AM_REL,  AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE,  AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE, // 0xD0
    AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_ZP,   AM_ZP,   AM_NONE,  AM_NONE, AM_IMM,  AM_NONE, AM_NONE, AM_NONE, AM_ABS,  AM_ABS,  AM_NONE, // 0xE0
    AM_REL,  AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE,  AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE, AM_NONE  // 0xF0
}; // addrModes

static const instruction_t instructions[256] = 
{
    I_RES, I_RES, I_RES, I_RES, I_RES, I_ORA, I_ASL, I_RES,  I_PHP, I_ORA, I_ASLA,I_RES, I_RES, I_ORA, I_ASL, I_RES, // 0x00
    I_BPL, I_RES, I_RES, I_RES, I_RES, I_RES, I_RES, I_RES,  I_CLC, I_RES, I_RES, I_RES, I_RES, I_RES, I_RES, I_RES, // 0x10
    I_JSR, I_RES, I_RES, I_RES, I_BIT, I_AND, I_ROL, I_RES,  I_PLP, I_AND, I_RES, I_RES, I_BIT, I_AND, I_ROL, I_RES, // 0x20
    I_BMI, I_RES, I_RES, I_RES, I_RES, I_RES, I_RES, I_RES,  I_SEC, I_RES, I_RES, I_RES, I_RES, I_RES, I_RES, I_RES, // 0x30
    I_RES, I_RES, I_RES, I_RES, I_RES, I_EOR, I_LSR, I_RES,  I_PHA, I_EOR, I_RES, I_RES, I_JMP, I_EOR, I_LSR, I_RES, // 0x40
    I_BVC, I_RES, I_RES, I_RES, I_RES, I_RES, I_RES, I_RES,  I_CLI, I_RES, I_RES, I_RES, I_RES, I_RES, I_RES, I_RES, // 0x50
    I_RTS, I_RES, I_RES, I_RES, I_RES, I_ADC, I_ROR, I_RES,  I_PLA, I_ADC, I_RES, I_RES, I_RES, I_ADC, I_ROR, I_RES, // 0x60
    I_BVS, I_RES, I_RES, I_RES, I_RES, I_RES, I_RES, I_RES,  I_SEI, I_RES, I_RES, I_RES, I_RES, I_RES, I_RES, I_RES, // 0x70
    I_RES, I_RES, I_RES, I_RES, I_RES, I_STA, I_RES, I_RES,  I_RES, I_RES, I_RES, I_RES, I_RES, I_STA, I_RES, I_RES, // 0x80
    I_BCC, I_RES, I_RES, I_RES, I_RES, I_RES, I_RES, I_RES,  I_RES, I_RES, I_RES, I_RES, I_RES, I_RES, I_RES, I_RES, // 0x90
    I_RES, I_RES, I_RES, I_RES, I_RES, I_LDA, I_RES, I_RES,  I_RES, I_LDA, I_RES, I_RES, I_RES, I_LDA, I_RES, I_RES, // 0xA0
    I_BCS, I_RES, I_RES, I_RES, I_RES, I_RES, I_RES, I_RES,  I_CLV, I_RES, I_RES, I_RES, I_RES, I_RES, I_RES, I_RES, // 0xB0
    I_RES, I_RES, I_RES, I_RES, I_RES, I_CMP, I_DEC, I_RES,  I_RES, I_CMP, I_RES, I_RES, I_RES, I_CMP, I_DEC, I_RES, // 0xC0
    I_BNE, I_RES, I_RES, I_RES, I_RES, I_RES, I_RES, I_RES,  I_CLD, I_RES, I_RES, I_RES, I_RES, I_RES, I_RES, I_RES, // 0xD0
    I_RES, I_RES, I_RES, I_RES, I_RES, I_SBC, I_INC, I_RES,  I_RES, I_SBC, I_RES, I_RES, I_RES, I_SBC, I_INC, I_RES, // 0xE0
    I_BEQ, I_RES, I_RES, I_RES, I_RES, I_RES, I_RES, I_RES,  I_SED, I_RES, I_RES, I_RES, I_RES, I_RES, I_RES, I_RES  // 0xF0
}; // instructions


void Cpu6502::reset()
{
    m_pc = (m_memory.read(0xFFFD) << 8) | m_memory.read(0xFFFC);
    std::cout << "Resetting CPU. PC=" << std::hex << std::setw(4) << m_pc;
    std::cout << std::dec << std::endl;
    m_sp = 0xFF;
} // reset

static uint8_t alu(uint8_t opcode, uint8_t arg1, uint8_t arg2, Cpu6502::t_flags& flags)
{
    uint16_t tmp16;
    uint8_t  tmp8 = arg1;

    switch (opcode)
    {
        case ALU_ORA:
            tmp8 = arg1 | arg2;
            flags.sign = (tmp8 >> 7);
            flags.zero = (tmp8 == 0);
            break;
        case ALU_AND:
            tmp8 = arg1 & arg2;
            flags.sign = (tmp8 >> 7);
            flags.zero = (tmp8 == 0);
            break;
        case ALU_EOR:
            tmp8 = arg1 ^ arg2;
            flags.sign = (tmp8 >> 7);
            flags.zero = (tmp8 == 0);
            break;
        case ALU_ADC:
            tmp16 = arg1 + arg2 + flags.carry;
            // Set S, Z, C, and V
            flags.carry    = (tmp16 >> 8);
            flags.overflow = ~((arg1 >> 7) ^ (arg2 >> 7)) & ((arg1 >> 7) ^ ((tmp16 >> 7) & 1));
            tmp8 = tmp16 & 0xFF;
            flags.sign  = (tmp8 >> 7);
            flags.zero  = (tmp8 == 0);
            break;
        case ALU_LDA:
            tmp8 = arg2;
            // Set S and Z
            flags.sign = (tmp8 >> 7);
            flags.zero = (tmp8 == 0);
            break;
        case ALU_CMP:
            tmp16 = arg1 + ((~arg2) & 0xFF) + 1;
            // Set S, Z, and C
            flags.carry = (tmp16 >> 8);
            tmp8 = tmp16 & 0xFF;
            flags.sign  = (tmp8 >> 7);
            flags.zero  = (tmp8 == 0);
            break;
        case ALU_SBC:
            tmp16 = arg1 + ~arg2 + flags.carry;
            // Set S, Z, C, and V
            flags.carry    = (tmp16 >> 8);
            flags.overflow = ~((arg1 >> 7) ^ (~arg2 >> 7)) & ((arg1 >> 7) ^ ((tmp16 >> 7) & 1));
            tmp8 = tmp16 & 0xFF;
            flags.sign  = (tmp8 >> 7);
            flags.zero  = (tmp8 == 0);
            break;
        case ALU_ASL:
            // Set S, Z, and C
            tmp8 = (arg2 << 1) & 0xFF;
            flags.carry = (arg2 >> 7);
            flags.sign  = (tmp8 >> 7);
            flags.zero  = (tmp8 == 0);
            break;
        case ALU_ROL:
            // Set S, Z, and C
            tmp8 = ((arg2 << 1) + flags.carry) & 0xFF;
            flags.carry = (arg2 >> 7);
            flags.sign  = (tmp8 >> 7);
            flags.zero  = (tmp8 == 0);
            break;
        case ALU_LSR:
            // Set S, Z, and C
            tmp8 = (arg2 >> 1) & 0xFF;
            flags.carry = (arg2 & 1);
            flags.sign  = (tmp8 >> 7);
            flags.zero  = (tmp8 == 0);
            break;
        case ALU_ROR:
            // Set S, Z, and C
            tmp8 = (arg2 >> 1) & 0xFF;
            if (flags.carry)
                tmp8 |= 0x80;
            flags.carry = (arg2 & 1);
            flags.sign  = (tmp8 >> 7);
            flags.zero  = (tmp8 == 0);
            break;
        case ALU_DEC:
            // Set S, and Z
            tmp8 = (arg2 - 1) & 0xFF;
            flags.sign  = (tmp8 >> 7);
            flags.zero  = (tmp8 == 0);
            break;
        case ALU_INC:
            // Set S, and Z
            tmp8 = (arg2 + 1) & 0xFF;
            flags.sign  = (tmp8 >> 7);
            flags.zero  = (tmp8 == 0);
            break;
        case ALU_BIT:
            // Set S, Z, and V
            tmp8 = arg1 & arg2;
            flags.sign = arg2 >> 7;
            flags.overflow = (arg2 >> 6) & 1;
            flags.zero  = (tmp8 == 0);
    }

    return tmp8;
} // alu

static uint16_t sign_extend(uint8_t arg)
{
    return (arg < 0x80) ? arg : 0xFF00 | arg;
}

void Cpu6502::singleStep()
{
    uint8_t inst = m_memory.read(m_pc);
    uint16_t pArg = 0;
    switch (addrModes[inst])
    {
        case AM_NONE : m_pc += 1; break;
        case AM_IMM  : pArg = m_pc+1; m_pc += 2; break;
        case AM_ABS  : pArg = m_memory.read(m_pc+1) | (m_memory.read(m_pc+2) << 8); m_pc += 3; break;
        case AM_ZP   : pArg = m_memory.read(m_pc+1); m_pc += 2; break;
        case AM_REL  : pArg = m_pc + sign_extend(m_memory.read(m_pc+1)) + 2; m_pc += 2; break;
    } // switch (addrModes[inst])

    m_memory.trace(true);
    switch (instructions[inst])
    {
        case I_RES: std::cerr << "Unimplemented instruction" << std::endl; exit(-1); break;

        case I_ORA: m_areg = alu(ALU_ORA, m_areg, m_memory.read(pArg), m_flags); break;
        case I_AND: m_areg = alu(ALU_AND, m_areg, m_memory.read(pArg), m_flags); break;
        case I_EOR: m_areg = alu(ALU_EOR, m_areg, m_memory.read(pArg), m_flags); break;
        case I_ADC: m_areg = alu(ALU_ADC, m_areg, m_memory.read(pArg), m_flags); break;
        case I_STA: m_memory.write(pArg, m_areg); break;
        case I_LDA: m_areg = alu(ALU_LDA, m_areg, m_memory.read(pArg), m_flags); break;
        case I_CMP: alu(ALU_CMP, m_areg, m_memory.read(pArg), m_flags); break;
        case I_SBC: m_areg = alu(ALU_SBC, m_areg, m_memory.read(pArg), m_flags); break;

        case I_CLC: m_flags.carry = 0; break;
        case I_SEC: m_flags.carry = 1; break;
        case I_CLI: m_flags.intmask = 0; break;
        case I_SEI: m_flags.intmask = 1; break;
        case I_CLV: m_flags.overflow = 0; break;
        case I_CLD: m_flags.decimal = 0; break;
        case I_SED: m_flags.decimal = 1; break;

        case I_BPL: if (m_flags.sign == 0) m_pc = pArg; break;
        case I_BMI: if (m_flags.sign == 1) m_pc = pArg; break;
        case I_BVC: if (m_flags.overflow == 0) m_pc = pArg; break;
        case I_BVS: if (m_flags.overflow == 1) m_pc = pArg; break;
        case I_BCC: if (m_flags.carry == 0) m_pc = pArg; break;
        case I_BCS: if (m_flags.carry == 1) m_pc = pArg; break;
        case I_BNE: if (m_flags.zero == 0) m_pc = pArg; break;
        case I_BEQ: if (m_flags.zero == 1) m_pc = pArg; break;

        case I_PHP: m_memory.write(0x0100 | m_sp, *(uint8_t*) &m_flags); m_sp -= 1; break;
        case I_JSR: m_memory.write(0x0100 | m_sp, (m_pc-1) >> 8); m_memory.write(0x0100 | (m_sp-1), (m_pc-1) & 0xFF); m_sp -= 2; m_pc = pArg; break;
        case I_PLP: m_sp += 1; *(uint8_t*) &m_flags = m_memory.read(0x0100 | m_sp); break;
        case I_PHA: m_memory.write(0x0100 | m_sp, m_areg); m_sp -= 1; break;
        case I_RTS: m_sp += 2; m_pc = ((m_memory.read(0x0100 | m_sp) << 8) | m_memory.read(0x0100 | (m_sp - 1))) + 1; break;
        case I_PLA: m_sp += 1; m_areg = m_memory.read(0x0100 | m_sp); break;

        case I_ASLA: m_areg = alu(ALU_ASL, 0, m_areg, m_flags); break;
        case I_ASL: m_memory.write(pArg, alu(ALU_ASL, m_areg, m_memory.read(pArg), m_flags)); break;
        case I_ROL: m_memory.write(pArg, alu(ALU_ROL, m_areg, m_memory.read(pArg), m_flags)); break;
        case I_LSR: m_memory.write(pArg, alu(ALU_LSR, m_areg, m_memory.read(pArg), m_flags)); break;
        case I_ROR: m_memory.write(pArg, alu(ALU_ROR, m_areg, m_memory.read(pArg), m_flags)); break;
        case I_DEC: m_memory.write(pArg, alu(ALU_DEC, m_areg, m_memory.read(pArg), m_flags)); break;
        case I_INC: m_memory.write(pArg, alu(ALU_INC, m_areg, m_memory.read(pArg), m_flags)); break;
        case I_BIT: alu(ALU_BIT, m_areg, m_memory.read(pArg), m_flags); break;

        case I_JMP: m_pc = pArg; break;
    } // switch (instructions[inst])
    m_memory.trace(false);
} // singleStep

void Cpu6502::show() const
{
    std::cout << "PC: " << std::hex << std::setw(4) << m_pc;
    std::cout << "  AREG : " << std::hex << std::setw(2) << (uint16_t) m_areg;
    std::cout << "  FLAGS : ";
    if (m_flags.sign)     std::cout << "S"; else std::cout << ".";
    if (m_flags.overflow) std::cout << "V"; else std::cout << ".";
    if (m_flags.reserved) std::cout << "R"; else std::cout << ".";
    if (m_flags.brk)      std::cout << "B"; else std::cout << ".";
    if (m_flags.decimal)  std::cout << "D"; else std::cout << ".";
    if (m_flags.intmask)  std::cout << "I"; else std::cout << ".";
    if (m_flags.zero)     std::cout << "Z"; else std::cout << ".";
    if (m_flags.carry)    std::cout << "C"; else std::cout << ".";

    std::cout << "   ";
    disas();
} // show

// Disassemble the current instruction.
void Cpu6502::disas() const
{
    uint8_t inst = m_memory.read(m_pc);
    switch (instructions[inst])
    {
        case I_RES: std::cout << "???"; break;
        case I_ORA: std::cout << "ORA"; break;
        case I_AND: std::cout << "AND"; break;
        case I_EOR: std::cout << "EOR"; break;
        case I_ADC: std::cout << "ADC"; break;
        case I_STA: std::cout << "STA"; break;
        case I_LDA: std::cout << "LDA"; break;
        case I_CMP: std::cout << "CMP"; break;
        case I_SBC: std::cout << "SBC"; break;
        case I_CLC: std::cout << "CLC"; break;
        case I_SEC: std::cout << "SEC"; break;
        case I_CLI: std::cout << "CLI"; break;
        case I_SEI: std::cout << "SEI"; break;
        case I_CLV: std::cout << "CLV"; break;
        case I_CLD: std::cout << "CLD"; break;
        case I_SED: std::cout << "SED"; break;
        case I_BPL: std::cout << "BPL"; break;
        case I_BMI: std::cout << "BMI"; break;
        case I_BVC: std::cout << "BVC"; break;
        case I_BVS: std::cout << "BVS"; break;
        case I_BCC: std::cout << "BCC"; break;
        case I_BCS: std::cout << "BCS"; break;
        case I_BNE: std::cout << "BNE"; break;
        case I_BEQ: std::cout << "BEQ"; break;
        case I_JMP: std::cout << "JMP"; break;
        case I_PHP: std::cout << "PHP"; break;
        case I_JSR: std::cout << "JSR"; break;
        case I_PLP: std::cout << "PLP"; break;
        case I_PHA: std::cout << "PHA"; break;
        case I_RTS: std::cout << "RTS"; break;
        case I_PLA: std::cout << "PLA"; break;
        case I_ASL: std::cout << "ASL"; break;
        case I_ASLA:std::cout << "ASL A"; break;
        case I_ROL: std::cout << "ROL"; break;
        case I_LSR: std::cout << "LSR"; break;
        case I_ROR: std::cout << "ROR"; break;
        case I_DEC: std::cout << "DEC"; break;
        case I_INC: std::cout << "INC"; break;
        case I_BIT: std::cout << "BIT"; break;
    }
    switch (addrModes[inst])
    {
        case AM_NONE : break;
        case AM_IMM  : std::cout << " #$" << std::hex << std::setfill('0') << std::setw(2) << (uint16_t) m_memory.read(m_pc+1); break;
        case AM_ABS  : std::cout << " $" << std::hex << std::setfill('0') << std::setw(4) << (m_memory.read(m_pc+1) | (m_memory.read(m_pc+2) << 8)); break;
        case AM_ZP   : std::cout << " $" << std::hex << std::setfill('0') << std::setw(2) << (uint16_t) m_memory.read(m_pc+1); break;
        case AM_REL  : std::cout << " $" << std::hex << std::setfill('0') << std::setw(4) << m_pc + 2 + sign_extend(m_memory.read(m_pc+1)); break;
    } // switch (addrModes[inst])

    std::cout << std::dec << std::endl;
} // disas

