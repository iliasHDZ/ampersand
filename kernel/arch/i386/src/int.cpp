#include "int.hpp"

#include <arch/exception.hpp>

#include "thread.hpp"

#include "drv/pic.hpp"

#define ISR_TARGET_CODE_SEG 0x8

int_regs* int_isr_regs;

static const char* int_isr_names[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",

    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",

    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",

    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};

struct PACKED_STRUCT int_idt_gate {
    u16 offset_0_15;
    u16 selector;
    u8  zero;
    u8  attribs;
    u16 offset_16_31;
};

static int_idt_gate idt_gates[INT_ID_COUNT];

static void int_set_gate(u32 id, u32 offset, u16 selector, u8 reqPl) {
    if (id > 255)
        return;

    int_idt_gate& gate = idt_gates[id];

    gate.offset_0_15  = offset & 0xffff;
    gate.offset_16_31 = (offset >> 16) & 0xffff;

    gate.selector = selector;
    gate.attribs  = 0x8E;// | (reqPl << 5);
    gate.zero     = 0;
}

namespace isr {
    extern "C" void isr_0();
    extern "C" void isr_1();
    extern "C" void isr_2();
    extern "C" void isr_3();
    extern "C" void isr_4();
    extern "C" void isr_5();
    extern "C" void isr_6();
    extern "C" void isr_7();
    extern "C" void isr_8();
    extern "C" void isr_9();
    extern "C" void isr_10();
    extern "C" void isr_11();
    extern "C" void isr_12();
    extern "C" void isr_13();
    extern "C" void isr_14();
    extern "C" void isr_15();
    extern "C" void isr_16();
    extern "C" void isr_17();
    extern "C" void isr_18();
    extern "C" void isr_19();
    extern "C" void isr_20();
    extern "C" void isr_21();
    extern "C" void isr_22();
    extern "C" void isr_23();
    extern "C" void isr_24();
    extern "C" void isr_25();
    extern "C" void isr_26();
    extern "C" void isr_27();
    extern "C" void isr_28();
    extern "C" void isr_29();
    extern "C" void isr_30();
    extern "C" void isr_31();
    extern "C" void isr_32();
    extern "C" void isr_33();
    extern "C" void isr_34();
    extern "C" void isr_35();
    extern "C" void isr_36();
    extern "C" void isr_37();
    extern "C" void isr_38();
    extern "C" void isr_39();
    extern "C" void isr_40();
    extern "C" void isr_41();
    extern "C" void isr_42();
    extern "C" void isr_43();
    extern "C" void isr_44();
    extern "C" void isr_45();
    extern "C" void isr_46();
    extern "C" void isr_47();
    extern "C" void isr_48();
    extern "C" void isr_49();
    extern "C" void isr_50();
    extern "C" void isr_51();
    extern "C" void isr_52();
    extern "C" void isr_53();
    extern "C" void isr_54();
    extern "C" void isr_55();
    extern "C" void isr_56();
    extern "C" void isr_57();
    extern "C" void isr_58();
    extern "C" void isr_59();
    extern "C" void isr_60();
    extern "C" void isr_61();
    extern "C" void isr_62();
    extern "C" void isr_63();
    extern "C" void isr_64();
    extern "C" void isr_65();
    extern "C" void isr_66();
    extern "C" void isr_67();
    extern "C" void isr_68();
    extern "C" void isr_69();
    extern "C" void isr_70();
    extern "C" void isr_71();
    extern "C" void isr_72();
    extern "C" void isr_73();
    extern "C" void isr_74();
    extern "C" void isr_75();
    extern "C" void isr_76();
    extern "C" void isr_77();
    extern "C" void isr_78();
    extern "C" void isr_79();
    extern "C" void isr_80();
    extern "C" void isr_81();
    extern "C" void isr_82();
    extern "C" void isr_83();
    extern "C" void isr_84();
    extern "C" void isr_85();
    extern "C" void isr_86();
    extern "C" void isr_87();
    extern "C" void isr_88();
    extern "C" void isr_89();
    extern "C" void isr_90();
    extern "C" void isr_91();
    extern "C" void isr_92();
    extern "C" void isr_93();
    extern "C" void isr_94();
    extern "C" void isr_95();
    extern "C" void isr_96();
    extern "C" void isr_97();
    extern "C" void isr_98();
    extern "C" void isr_99();
    extern "C" void isr_100();
    extern "C" void isr_101();
    extern "C" void isr_102();
    extern "C" void isr_103();
    extern "C" void isr_104();
    extern "C" void isr_105();
    extern "C" void isr_106();
    extern "C" void isr_107();
    extern "C" void isr_108();
    extern "C" void isr_109();
    extern "C" void isr_110();
    extern "C" void isr_111();
    extern "C" void isr_112();
    extern "C" void isr_113();
    extern "C" void isr_114();
    extern "C" void isr_115();
    extern "C" void isr_116();
    extern "C" void isr_117();
    extern "C" void isr_118();
    extern "C" void isr_119();
    extern "C" void isr_120();
    extern "C" void isr_121();
    extern "C" void isr_122();
    extern "C" void isr_123();
    extern "C" void isr_124();
    extern "C" void isr_125();
    extern "C" void isr_126();
    extern "C" void isr_127();
    extern "C" void isr_128();
    extern "C" void isr_129();
    extern "C" void isr_130();
    extern "C" void isr_131();
    extern "C" void isr_132();
    extern "C" void isr_133();
    extern "C" void isr_134();
    extern "C" void isr_135();
    extern "C" void isr_136();
    extern "C" void isr_137();
    extern "C" void isr_138();
    extern "C" void isr_139();
    extern "C" void isr_140();
    extern "C" void isr_141();
    extern "C" void isr_142();
    extern "C" void isr_143();
    extern "C" void isr_144();
    extern "C" void isr_145();
    extern "C" void isr_146();
    extern "C" void isr_147();
    extern "C" void isr_148();
    extern "C" void isr_149();
    extern "C" void isr_150();
    extern "C" void isr_151();
    extern "C" void isr_152();
    extern "C" void isr_153();
    extern "C" void isr_154();
    extern "C" void isr_155();
    extern "C" void isr_156();
    extern "C" void isr_157();
    extern "C" void isr_158();
    extern "C" void isr_159();
    extern "C" void isr_160();
    extern "C" void isr_161();
    extern "C" void isr_162();
    extern "C" void isr_163();
    extern "C" void isr_164();
    extern "C" void isr_165();
    extern "C" void isr_166();
    extern "C" void isr_167();
    extern "C" void isr_168();
    extern "C" void isr_169();
    extern "C" void isr_170();
    extern "C" void isr_171();
    extern "C" void isr_172();
    extern "C" void isr_173();
    extern "C" void isr_174();
    extern "C" void isr_175();
    extern "C" void isr_176();
    extern "C" void isr_177();
    extern "C" void isr_178();
    extern "C" void isr_179();
    extern "C" void isr_180();
    extern "C" void isr_181();
    extern "C" void isr_182();
    extern "C" void isr_183();
    extern "C" void isr_184();
    extern "C" void isr_185();
    extern "C" void isr_186();
    extern "C" void isr_187();
    extern "C" void isr_188();
    extern "C" void isr_189();
    extern "C" void isr_190();
    extern "C" void isr_191();
    extern "C" void isr_192();
    extern "C" void isr_193();
    extern "C" void isr_194();
    extern "C" void isr_195();
    extern "C" void isr_196();
    extern "C" void isr_197();
    extern "C" void isr_198();
    extern "C" void isr_199();
    extern "C" void isr_200();
    extern "C" void isr_201();
    extern "C" void isr_202();
    extern "C" void isr_203();
    extern "C" void isr_204();
    extern "C" void isr_205();
    extern "C" void isr_206();
    extern "C" void isr_207();
    extern "C" void isr_208();
    extern "C" void isr_209();
    extern "C" void isr_210();
    extern "C" void isr_211();
    extern "C" void isr_212();
    extern "C" void isr_213();
    extern "C" void isr_214();
    extern "C" void isr_215();
    extern "C" void isr_216();
    extern "C" void isr_217();
    extern "C" void isr_218();
    extern "C" void isr_219();
    extern "C" void isr_220();
    extern "C" void isr_221();
    extern "C" void isr_222();
    extern "C" void isr_223();
    extern "C" void isr_224();
    extern "C" void isr_225();
    extern "C" void isr_226();
    extern "C" void isr_227();
    extern "C" void isr_228();
    extern "C" void isr_229();
    extern "C" void isr_230();
    extern "C" void isr_231();
    extern "C" void isr_232();
    extern "C" void isr_233();
    extern "C" void isr_234();
    extern "C" void isr_235();
    extern "C" void isr_236();
    extern "C" void isr_237();
    extern "C" void isr_238();
    extern "C" void isr_239();
    extern "C" void isr_240();
    extern "C" void isr_241();
    extern "C" void isr_242();
    extern "C" void isr_243();
    extern "C" void isr_244();
    extern "C" void isr_245();
    extern "C" void isr_246();
    extern "C" void isr_247();
    extern "C" void isr_248();
    extern "C" void isr_249();
    extern "C" void isr_250();
    extern "C" void isr_251();
    extern "C" void isr_252();
    extern "C" void isr_253();
    extern "C" void isr_254();
    extern "C" void isr_255();

    static void int_init_isr_gates() {
        int_set_gate(0,   (u32)isr_0,   ISR_TARGET_CODE_SEG, 0);
        int_set_gate(1,   (u32)isr_1,   ISR_TARGET_CODE_SEG, 0);
        int_set_gate(2,   (u32)isr_2,   ISR_TARGET_CODE_SEG, 0);
        int_set_gate(3,   (u32)isr_3,   ISR_TARGET_CODE_SEG, 0);
        int_set_gate(4,   (u32)isr_4,   ISR_TARGET_CODE_SEG, 0);
        int_set_gate(5,   (u32)isr_5,   ISR_TARGET_CODE_SEG, 0);
        int_set_gate(6,   (u32)isr_6,   ISR_TARGET_CODE_SEG, 0);
        int_set_gate(7,   (u32)isr_7,   ISR_TARGET_CODE_SEG, 0);
        int_set_gate(8,   (u32)isr_8,   ISR_TARGET_CODE_SEG, 0);
        int_set_gate(9,   (u32)isr_9,   ISR_TARGET_CODE_SEG, 0);
        int_set_gate(10,  (u32)isr_10,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(11,  (u32)isr_11,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(12,  (u32)isr_12,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(13,  (u32)isr_13,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(14,  (u32)isr_14,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(15,  (u32)isr_15,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(16,  (u32)isr_16,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(17,  (u32)isr_17,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(18,  (u32)isr_18,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(19,  (u32)isr_19,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(20,  (u32)isr_20,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(21,  (u32)isr_21,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(22,  (u32)isr_22,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(23,  (u32)isr_23,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(24,  (u32)isr_24,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(25,  (u32)isr_25,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(26,  (u32)isr_26,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(27,  (u32)isr_27,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(28,  (u32)isr_28,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(29,  (u32)isr_29,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(30,  (u32)isr_30,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(31,  (u32)isr_31,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(32,  (u32)isr_32,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(33,  (u32)isr_33,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(34,  (u32)isr_34,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(35,  (u32)isr_35,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(36,  (u32)isr_36,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(37,  (u32)isr_37,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(38,  (u32)isr_38,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(39,  (u32)isr_39,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(40,  (u32)isr_40,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(41,  (u32)isr_41,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(42,  (u32)isr_42,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(43,  (u32)isr_43,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(44,  (u32)isr_44,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(45,  (u32)isr_45,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(46,  (u32)isr_46,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(47,  (u32)isr_47,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(48,  (u32)isr_48,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(49,  (u32)isr_49,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(50,  (u32)isr_50,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(51,  (u32)isr_51,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(52,  (u32)isr_52,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(53,  (u32)isr_53,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(54,  (u32)isr_54,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(55,  (u32)isr_55,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(56,  (u32)isr_56,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(57,  (u32)isr_57,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(58,  (u32)isr_58,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(59,  (u32)isr_59,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(60,  (u32)isr_60,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(61,  (u32)isr_61,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(62,  (u32)isr_62,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(63,  (u32)isr_63,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(64,  (u32)isr_64,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(65,  (u32)isr_65,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(66,  (u32)isr_66,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(67,  (u32)isr_67,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(68,  (u32)isr_68,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(69,  (u32)isr_69,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(70,  (u32)isr_70,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(71,  (u32)isr_71,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(72,  (u32)isr_72,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(73,  (u32)isr_73,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(74,  (u32)isr_74,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(75,  (u32)isr_75,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(76,  (u32)isr_76,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(77,  (u32)isr_77,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(78,  (u32)isr_78,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(79,  (u32)isr_79,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(80,  (u32)isr_80,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(81,  (u32)isr_81,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(82,  (u32)isr_82,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(83,  (u32)isr_83,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(84,  (u32)isr_84,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(85,  (u32)isr_85,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(86,  (u32)isr_86,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(87,  (u32)isr_87,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(88,  (u32)isr_88,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(89,  (u32)isr_89,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(90,  (u32)isr_90,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(91,  (u32)isr_91,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(92,  (u32)isr_92,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(93,  (u32)isr_93,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(94,  (u32)isr_94,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(95,  (u32)isr_95,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(96,  (u32)isr_96,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(97,  (u32)isr_97,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(98,  (u32)isr_98,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(99,  (u32)isr_99,  ISR_TARGET_CODE_SEG, 0);
        int_set_gate(100, (u32)isr_100, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(101, (u32)isr_101, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(102, (u32)isr_102, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(103, (u32)isr_103, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(104, (u32)isr_104, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(105, (u32)isr_105, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(106, (u32)isr_106, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(107, (u32)isr_107, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(108, (u32)isr_108, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(109, (u32)isr_109, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(110, (u32)isr_110, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(111, (u32)isr_111, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(112, (u32)isr_112, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(113, (u32)isr_113, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(114, (u32)isr_114, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(115, (u32)isr_115, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(116, (u32)isr_116, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(117, (u32)isr_117, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(118, (u32)isr_118, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(119, (u32)isr_119, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(120, (u32)isr_120, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(121, (u32)isr_121, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(122, (u32)isr_122, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(123, (u32)isr_123, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(124, (u32)isr_124, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(125, (u32)isr_125, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(126, (u32)isr_126, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(127, (u32)isr_127, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(128, (u32)isr_128, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(129, (u32)isr_129, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(130, (u32)isr_130, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(131, (u32)isr_131, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(132, (u32)isr_132, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(133, (u32)isr_133, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(134, (u32)isr_134, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(135, (u32)isr_135, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(136, (u32)isr_136, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(137, (u32)isr_137, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(138, (u32)isr_138, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(139, (u32)isr_139, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(140, (u32)isr_140, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(141, (u32)isr_141, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(142, (u32)isr_142, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(143, (u32)isr_143, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(144, (u32)isr_144, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(145, (u32)isr_145, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(146, (u32)isr_146, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(147, (u32)isr_147, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(148, (u32)isr_148, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(149, (u32)isr_149, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(150, (u32)isr_150, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(151, (u32)isr_151, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(152, (u32)isr_152, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(153, (u32)isr_153, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(154, (u32)isr_154, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(155, (u32)isr_155, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(156, (u32)isr_156, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(157, (u32)isr_157, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(158, (u32)isr_158, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(159, (u32)isr_159, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(160, (u32)isr_160, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(161, (u32)isr_161, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(162, (u32)isr_162, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(163, (u32)isr_163, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(164, (u32)isr_164, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(165, (u32)isr_165, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(166, (u32)isr_166, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(167, (u32)isr_167, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(168, (u32)isr_168, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(169, (u32)isr_169, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(170, (u32)isr_170, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(171, (u32)isr_171, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(172, (u32)isr_172, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(173, (u32)isr_173, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(174, (u32)isr_174, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(175, (u32)isr_175, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(176, (u32)isr_176, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(177, (u32)isr_177, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(178, (u32)isr_178, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(179, (u32)isr_179, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(180, (u32)isr_180, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(181, (u32)isr_181, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(182, (u32)isr_182, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(183, (u32)isr_183, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(184, (u32)isr_184, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(185, (u32)isr_185, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(186, (u32)isr_186, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(187, (u32)isr_187, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(188, (u32)isr_188, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(189, (u32)isr_189, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(190, (u32)isr_190, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(191, (u32)isr_191, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(192, (u32)isr_192, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(193, (u32)isr_193, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(194, (u32)isr_194, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(195, (u32)isr_195, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(196, (u32)isr_196, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(197, (u32)isr_197, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(198, (u32)isr_198, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(199, (u32)isr_199, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(200, (u32)isr_200, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(201, (u32)isr_201, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(202, (u32)isr_202, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(203, (u32)isr_203, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(204, (u32)isr_204, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(205, (u32)isr_205, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(206, (u32)isr_206, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(207, (u32)isr_207, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(208, (u32)isr_208, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(209, (u32)isr_209, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(210, (u32)isr_210, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(211, (u32)isr_211, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(212, (u32)isr_212, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(213, (u32)isr_213, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(214, (u32)isr_214, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(215, (u32)isr_215, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(216, (u32)isr_216, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(217, (u32)isr_217, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(218, (u32)isr_218, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(219, (u32)isr_219, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(220, (u32)isr_220, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(221, (u32)isr_221, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(222, (u32)isr_222, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(223, (u32)isr_223, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(224, (u32)isr_224, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(225, (u32)isr_225, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(226, (u32)isr_226, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(227, (u32)isr_227, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(228, (u32)isr_228, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(229, (u32)isr_229, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(230, (u32)isr_230, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(231, (u32)isr_231, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(232, (u32)isr_232, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(233, (u32)isr_233, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(234, (u32)isr_234, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(235, (u32)isr_235, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(236, (u32)isr_236, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(237, (u32)isr_237, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(238, (u32)isr_238, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(239, (u32)isr_239, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(240, (u32)isr_240, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(241, (u32)isr_241, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(242, (u32)isr_242, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(243, (u32)isr_243, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(244, (u32)isr_244, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(245, (u32)isr_245, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(246, (u32)isr_246, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(247, (u32)isr_247, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(248, (u32)isr_248, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(249, (u32)isr_249, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(250, (u32)isr_250, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(251, (u32)isr_251, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(252, (u32)isr_252, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(253, (u32)isr_253, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(254, (u32)isr_254, ISR_TARGET_CODE_SEG, 0);
        int_set_gate(255, (u32)isr_255, ISR_TARGET_CODE_SEG, 0);
    }
};

struct PACKED_STRUCT int_idt_ptr {
    u16 limit;
    u32 base;
};

static int_idt_ptr idt_ptr = {
    sizeof(idt_gates) - 1,
    (u32)&idt_gates
};

static bool int_load_idt() {
    __asm__ __volatile__("lidtl (%0)" : : "r" (&idt_ptr));
    return true;
}

void int_enable() {
    asm("sti");
}

void int_disable() {
    asm("cli");
}

struct int_handler_entry {
    InterruptHandler func;
    void* param;
};

static int_handler_entry int_irq_handlers[INT_ID_COUNT - INT_EXCPT_COUNT];

void int_regs_to_cpu_state(CPUState* dst, int_regs* src) {
    dst->regs[ARCH_REG_EAX] = src->eax;
    dst->regs[ARCH_REG_ECX] = src->ecx;
    dst->regs[ARCH_REG_EDX] = src->edx;
    dst->regs[ARCH_REG_EBX] = src->ebx;

    dst->regs[ARCH_REG_ESP] = src->esp;
    dst->regs[ARCH_REG_EBP] = src->ebp;
    dst->regs[ARCH_REG_ESI] = src->esi;
    dst->regs[ARCH_REG_EDI] = src->edi;

    dst->regs[ARCH_REG_CS] = src->cs & 0xffff;
    dst->regs[ARCH_REG_DS] = src->ds & 0xffff;
    dst->regs[ARCH_REG_SS] = src->ss & 0xffff;

    dst->regs[ARCH_REG_EIP]    = src->eip;
    dst->regs[ARCH_REG_EFLAGS] = src->eflags;
}

bool int_register_irq(u32 id, InterruptHandler handler, void* param) {
    if (id >= (INT_ID_COUNT - INT_EXCPT_COUNT))
        return false;

    if (int_irq_handlers[id].func)
        return false;

    int_irq_handlers[id].func  = handler;
    int_irq_handlers[id].param = param;

    return true;
}

extern "C" void int_isr_handler() {
    u32 id = int_isr_regs->int_no;

    arch_cpu_was_idling = arch_cpu_is_idling;
    arch_cpu_is_idling  = false;

    if (id < INT_EXCPT_COUNT) {
        CPUState state;
        int_regs_to_cpu_state(&state, int_isr_regs);

        Exception excpt(int_isr_names[id], &state);

        arch_thread_invoke_exception(&excpt);
        return;
    }
    
    u32 irq = id - INT_EXCPT_COUNT;

    PIC8259Pair::get()->endOfInterrupt(id);

    if (int_irq_handlers[irq].func)
        int_irq_handlers[irq].func(int_irq_handlers[irq].param);

    // If cpu was previously idling and nothing changed after interrupt, continue idling
    if (arch_cpu_was_idling)
        arch_idle_cpu();
}

u32 isr_src_stack_esp;
u32 isr_src_stack_ebp;

u32 int_get_esp() {
    return isr_src_stack_esp;
}

u32 int_get_ebp() {
    return isr_src_stack_ebp;
}

extern "C" void NO_RETURN isr_return();

void NO_RETURN int_return(u32 esp, u32 ebp) {
    isr_src_stack_esp = esp;
    isr_src_stack_ebp = ebp;

    isr_return();
}

bool int_init() {
    memset(idt_gates,        0, sizeof(idt_gates));
    memset(int_irq_handlers, 0, sizeof(int_irq_handlers));

    isr::int_init_isr_gates();

    if (!int_load_idt())
        return false;

    return true;
}