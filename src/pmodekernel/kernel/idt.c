#include <stdint.h>
#include "./kmemmgt.h"
#include "./logging.h"

#define NULL ((void*)0)

#pragma pack(push, 1)
struct InterruptDescriptor {
   uint16_t offset1;        // offset bits 0..15
   uint16_t selector;        // a code segment selector in GDT or LDT
   uint8_t  zero;            // unused, set to 0
   uint8_t  type_attributes; // gate type, dpl, and p fields
   uint16_t offset2;        // offset bits 16..31
};

struct IdtPtr {
    uint16_t limit;
    uint32_t base;
};

#pragma pack(pop)

struct InterruptDescriptor idt[256] __attribute__((aligned(16)));
struct IdtPtr idtr;

extern void int0(void);
extern void int1(void);
extern void int2(void);
extern void int3(void);
extern void int4(void);
extern void int5(void);
extern void int6(void);
extern void int7(void);
extern void int8(void);
extern void int9(void);
extern void intA(void);
extern void intB(void);
extern void intC(void);
extern void intD(void);
extern void intE(void); // SKIP F
extern void int10(void);
extern void int11(void);
extern void int12(void);
extern void int13(void);
extern void int14(void);
extern void int15(void);
extern void int16(void);
extern void int17(void);
extern void int18(void);
extern void int19(void);
extern void int1A(void);
extern void int1B(void);
extern void int1C(void);
extern void int1D(void);
extern void int1E(void);
extern void int1F(void);
extern void int20(void);
extern void int21(void);
extern void int22(void);
extern void int23(void);
extern void int24(void);
extern void int25(void);
extern void int26(void);
extern void int27(void);
extern void int28(void);
extern void int29(void);
extern void int2A(void);
extern void int2B(void);
extern void int2C(void);
extern void int2D(void);
extern void int2E(void);
extern void int2F(void);
extern void int30(void);
extern void int31(void);
extern void int32(void);
extern void int33(void);
extern void int34(void);
extern void int35(void);
extern void int36(void);
extern void int37(void);
extern void int38(void);
extern void int39(void);
extern void int3A(void);
extern void int3B(void);
extern void int3C(void);
extern void int3D(void);
extern void int3E(void);
extern void int3F(void);
extern void int40(void);
extern void int41(void);
extern void int42(void);
extern void int43(void);
extern void int44(void);
extern void int45(void);
extern void int46(void);
extern void int47(void);
extern void int48(void);
extern void int49(void);
extern void int4A(void);
extern void int4B(void);
extern void int4C(void);
extern void int4D(void);
extern void int4E(void);
extern void int4F(void);
extern void int50(void);
extern void int51(void);
extern void int52(void);
extern void int53(void);
extern void int54(void);
extern void int55(void);
extern void int56(void);
extern void int57(void);
extern void int58(void);
extern void int59(void);
extern void int5A(void);
extern void int5B(void);
extern void int5C(void);
extern void int5D(void);
extern void int5E(void);
extern void int5F(void);
extern void int60(void);
extern void int61(void);
extern void int62(void);
extern void int63(void);
extern void int64(void);
extern void int65(void);
extern void int66(void);
extern void int67(void);
extern void int68(void);
extern void int69(void);
extern void int6A(void);
extern void int6B(void);
extern void int6C(void);
extern void int6D(void);
extern void int6E(void);
extern void int6F(void);
extern void int70(void);
extern void int71(void);
extern void int72(void);
extern void int73(void);
extern void int74(void);
extern void int75(void);
extern void int76(void);
extern void int77(void);
extern void int78(void);
extern void int79(void);
extern void int7A(void);
extern void int7B(void);
extern void int7C(void);
extern void int7D(void);
extern void int7E(void);
extern void int7F(void);
extern void int80(void);
extern void int81(void);
extern void int82(void);
extern void int83(void);
extern void int84(void);
extern void int85(void);
extern void int86(void);
extern void int87(void);
extern void int88(void);
extern void int89(void);
extern void int8A(void);
extern void int8B(void);
extern void int8C(void);
extern void int8D(void);
extern void int8E(void);
extern void int8F(void);
extern void int90(void);
extern void int91(void);
extern void int92(void);
extern void int93(void);
extern void int94(void);
extern void int95(void);
extern void int96(void);
extern void int97(void);
extern void int98(void);
extern void int99(void);
extern void int9A(void);
extern void int9B(void);
extern void int9C(void);
extern void int9D(void);
extern void int9E(void);
extern void int9F(void);
extern void intA0(void);
extern void intA1(void);
extern void intA2(void);
extern void intA3(void);
extern void intA4(void);
extern void intA5(void);
extern void intA6(void);
extern void intA7(void);
extern void intA8(void);
extern void intA9(void);
extern void intAA(void);
extern void intAB(void);
extern void intAC(void);
extern void intAD(void);
extern void intAE(void);
extern void intAF(void);
extern void intB0(void);
extern void intB1(void);
extern void intB2(void);
extern void intB3(void);
extern void intB4(void);
extern void intB5(void);
extern void intB6(void);
extern void intB7(void);
extern void intB8(void);
extern void intB9(void);
extern void intBA(void);
extern void intBB(void);
extern void intBC(void);
extern void intBD(void);
extern void intBE(void);
extern void intBF(void);
extern void intC0(void);
extern void intC1(void);
extern void intC2(void);
extern void intC3(void);
extern void intC4(void);
extern void intC5(void);
extern void intC6(void);
extern void intC7(void);
extern void intC8(void);
extern void intC9(void);
extern void intCA(void);
extern void intCB(void);
extern void intCC(void);
extern void intCD(void);
extern void intCE(void);
extern void intCF(void);
extern void intD0(void);
extern void intD1(void);
extern void intD2(void);
extern void intD3(void);
extern void intD4(void);
extern void intD5(void);
extern void intD6(void);
extern void intD7(void);
extern void intD8(void);
extern void intD9(void);
extern void intDA(void);
extern void intDB(void);
extern void intDC(void);
extern void intDD(void);
extern void intDE(void);
extern void intDF(void);
extern void intE0(void);
extern void intE1(void);
extern void intE2(void);
extern void intE3(void);
extern void intE4(void);
extern void intE5(void);
extern void intE6(void);
extern void intE7(void);
extern void intE8(void);
extern void intE9(void);
extern void intEA(void);
extern void intEB(void);
extern void intEC(void);
extern void intED(void);
extern void intEE(void);
extern void intEF(void);
extern void intF0(void);
extern void intF1(void);
extern void intF2(void);
extern void intF3(void);
extern void intF4(void);
extern void intF5(void);
extern void intF6(void);
extern void intF7(void);
extern void intF8(void);
extern void intF9(void);
extern void intFA(void);
extern void intFB(void);
extern void intFC(void);
extern void intFD(void);
extern void intFE(void);
extern void intFF(void);

void (*idtRequestDispatchers[256])(void);
void (*idtRequestHandlers[256])(uint32_t*);

void setIDTValue(uint8_t intNum, uint32_t address, uint8_t attr, uint16_t selector);
void noHandlerDefined(uint32_t* stack);

void setIDTHandler(uint8_t intNum, uint32_t address) {
    idtRequestHandlers[intNum] = (void(*)(uint32_t*)) address;
}

// initialize idt variable and set cpu's idt register to point to it
void initIDTStructures() {   
    idtRequestDispatchers[0] = int0;
    idtRequestDispatchers[1] = int1;
    idtRequestDispatchers[2] = int2;
    idtRequestDispatchers[3] = int3;
    idtRequestDispatchers[4] = int4;
    idtRequestDispatchers[5] = int5;
    idtRequestDispatchers[6] = int6;
    idtRequestDispatchers[7] = int7;
    idtRequestDispatchers[8] = int8;
    idtRequestDispatchers[9] = int9;
    idtRequestDispatchers[10] = intA;
    idtRequestDispatchers[11] = intB;
    idtRequestDispatchers[12] = intC;
    idtRequestDispatchers[13] = intD;
    idtRequestDispatchers[14] = intE;
    // idtRequestDispatchers[15] = intF; SKIP F; RESERVED
    idtRequestDispatchers[16] = int10;
    idtRequestDispatchers[17] = int11;
    idtRequestDispatchers[18] = int12;
    idtRequestDispatchers[19] = int13;
    idtRequestDispatchers[20] = int14;
    idtRequestDispatchers[21] = int15;
    idtRequestDispatchers[22] = int16;
    idtRequestDispatchers[23] = int17;
    idtRequestDispatchers[24] = int18;
    idtRequestDispatchers[25] = int19;
    idtRequestDispatchers[26] = int1A;
    idtRequestDispatchers[27] = int1B;
    idtRequestDispatchers[28] = int1C;
    idtRequestDispatchers[29] = int1D;
    idtRequestDispatchers[30] = int1E;
    idtRequestDispatchers[31] = int1F;
    idtRequestDispatchers[32] = int20;
    idtRequestDispatchers[33] = int21;
    idtRequestDispatchers[34] = int22;
    idtRequestDispatchers[35] = int23;
    idtRequestDispatchers[36] = int24;
    idtRequestDispatchers[37] = int25;
    idtRequestDispatchers[38] = int26;
    idtRequestDispatchers[39] = int27;
    idtRequestDispatchers[40] = int28;
    idtRequestDispatchers[41] = int29;
    idtRequestDispatchers[42] = int2A;
    idtRequestDispatchers[43] = int2B;
    idtRequestDispatchers[44] = int2C;
    idtRequestDispatchers[45] = int2D;
    idtRequestDispatchers[46] = int2E;
    idtRequestDispatchers[47] = int2F;
    idtRequestDispatchers[48] = int30;
    idtRequestDispatchers[49] = int31;
    idtRequestDispatchers[50] = int32;
    idtRequestDispatchers[51] = int33;
    idtRequestDispatchers[52] = int34;
    idtRequestDispatchers[53] = int35;
    idtRequestDispatchers[54] = int36;
    idtRequestDispatchers[55] = int37;
    idtRequestDispatchers[56] = int38;
    idtRequestDispatchers[57] = int39;
    idtRequestDispatchers[58] = int3A;
    idtRequestDispatchers[59] = int3B;
    idtRequestDispatchers[60] = int3C;
    idtRequestDispatchers[61] = int3D;
    idtRequestDispatchers[62] = int3E;
    idtRequestDispatchers[63] = int3F;
    idtRequestDispatchers[64] = int40;
    idtRequestDispatchers[65] = int41;
    idtRequestDispatchers[66] = int42;
    idtRequestDispatchers[67] = int43;
    idtRequestDispatchers[68] = int44;
    idtRequestDispatchers[69] = int45;
    idtRequestDispatchers[70] = int46;
    idtRequestDispatchers[71] = int47;
    idtRequestDispatchers[72] = int48;
    idtRequestDispatchers[73] = int49;
    idtRequestDispatchers[74] = int4A;
    idtRequestDispatchers[75] = int4B;
    idtRequestDispatchers[76] = int4C;
    idtRequestDispatchers[77] = int4D;
    idtRequestDispatchers[78] = int4E;
    idtRequestDispatchers[79] = int4F;
    idtRequestDispatchers[80] = int50;
    idtRequestDispatchers[81] = int51;
    idtRequestDispatchers[82] = int52;
    idtRequestDispatchers[83] = int53;
    idtRequestDispatchers[84] = int54;
    idtRequestDispatchers[85] = int55;
    idtRequestDispatchers[86] = int56;
    idtRequestDispatchers[87] = int57;
    idtRequestDispatchers[88] = int58;
    idtRequestDispatchers[89] = int59;
    idtRequestDispatchers[90] = int5A;
    idtRequestDispatchers[91] = int5B;
    idtRequestDispatchers[92] = int5C;
    idtRequestDispatchers[93] = int5D;
    idtRequestDispatchers[94] = int5E;
    idtRequestDispatchers[95] = int5F;
    idtRequestDispatchers[96] = int60;
    idtRequestDispatchers[97] = int61;
    idtRequestDispatchers[98] = int62;
    idtRequestDispatchers[99] = int63;
    idtRequestDispatchers[100] = int64;
    idtRequestDispatchers[101] = int65;
    idtRequestDispatchers[102] = int66;
    idtRequestDispatchers[103] = int67;
    idtRequestDispatchers[104] = int68;
    idtRequestDispatchers[105] = int69;
    idtRequestDispatchers[106] = int6A;
    idtRequestDispatchers[107] = int6B;
    idtRequestDispatchers[108] = int6C;
    idtRequestDispatchers[109] = int6D;
    idtRequestDispatchers[110] = int6E;
    idtRequestDispatchers[111] = int6F;
    idtRequestDispatchers[112] = int70;
    idtRequestDispatchers[113] = int71;
    idtRequestDispatchers[114] = int72;
    idtRequestDispatchers[115] = int73;
    idtRequestDispatchers[116] = int74;
    idtRequestDispatchers[117] = int75;
    idtRequestDispatchers[118] = int76;
    idtRequestDispatchers[119] = int77;
    idtRequestDispatchers[120] = int78;
    idtRequestDispatchers[121] = int79;
    idtRequestDispatchers[122] = int7A;
    idtRequestDispatchers[123] = int7B;
    idtRequestDispatchers[124] = int7C;
    idtRequestDispatchers[125] = int7D;
    idtRequestDispatchers[126] = int7E;
    idtRequestDispatchers[127] = int7F;
    idtRequestDispatchers[128] = int80;
    idtRequestDispatchers[129] = int81;
    idtRequestDispatchers[130] = int82;
    idtRequestDispatchers[131] = int83;
    idtRequestDispatchers[132] = int84;
    idtRequestDispatchers[133] = int85;
    idtRequestDispatchers[134] = int86;
    idtRequestDispatchers[135] = int87;
    idtRequestDispatchers[136] = int88;
    idtRequestDispatchers[137] = int89;
    idtRequestDispatchers[138] = int8A;
    idtRequestDispatchers[139] = int8B;
    idtRequestDispatchers[140] = int8C;
    idtRequestDispatchers[141] = int8D;
    idtRequestDispatchers[142] = int8E;
    idtRequestDispatchers[143] = int8F;
    idtRequestDispatchers[144] = int90;
    idtRequestDispatchers[145] = int91;
    idtRequestDispatchers[146] = int92;
    idtRequestDispatchers[147] = int93;
    idtRequestDispatchers[148] = int94;
    idtRequestDispatchers[149] = int95;
    idtRequestDispatchers[150] = int96;
    idtRequestDispatchers[151] = int97;
    idtRequestDispatchers[152] = int98;
    idtRequestDispatchers[153] = int99;
    idtRequestDispatchers[154] = int9A;
    idtRequestDispatchers[155] = int9B;
    idtRequestDispatchers[156] = int9C;
    idtRequestDispatchers[157] = int9D;
    idtRequestDispatchers[158] = int9E;
    idtRequestDispatchers[159] = int9F;
    idtRequestDispatchers[160] = intA0;
    idtRequestDispatchers[161] = intA1;
    idtRequestDispatchers[162] = intA2;
    idtRequestDispatchers[163] = intA3;
    idtRequestDispatchers[164] = intA4;
    idtRequestDispatchers[165] = intA5;
    idtRequestDispatchers[166] = intA6;
    idtRequestDispatchers[167] = intA7;
    idtRequestDispatchers[168] = intA8;
    idtRequestDispatchers[169] = intA9;
    idtRequestDispatchers[170] = intAA;
    idtRequestDispatchers[171] = intAB;
    idtRequestDispatchers[172] = intAC;
    idtRequestDispatchers[173] = intAD;
    idtRequestDispatchers[174] = intAE;
    idtRequestDispatchers[175] = intAF;
    idtRequestDispatchers[176] = intB0;
    idtRequestDispatchers[177] = intB1;
    idtRequestDispatchers[178] = intB2;
    idtRequestDispatchers[179] = intB3;
    idtRequestDispatchers[180] = intB4;
    idtRequestDispatchers[181] = intB5;
    idtRequestDispatchers[182] = intB6;
    idtRequestDispatchers[183] = intB7;
    idtRequestDispatchers[184] = intB8;
    idtRequestDispatchers[185] = intB9;
    idtRequestDispatchers[186] = intBA;
    idtRequestDispatchers[187] = intBB;
    idtRequestDispatchers[188] = intBC;
    idtRequestDispatchers[189] = intBD;
    idtRequestDispatchers[190] = intBE;
    idtRequestDispatchers[191] = intBF;
    idtRequestDispatchers[192] = intC0;
    idtRequestDispatchers[193] = intC1;
    idtRequestDispatchers[194] = intC2;
    idtRequestDispatchers[195] = intC3;
    idtRequestDispatchers[196] = intC4;
    idtRequestDispatchers[197] = intC5;
    idtRequestDispatchers[198] = intC6;
    idtRequestDispatchers[199] = intC7;
    idtRequestDispatchers[200] = intC8;
    idtRequestDispatchers[201] = intC9;
    idtRequestDispatchers[202] = intCA;
    idtRequestDispatchers[203] = intCB;
    idtRequestDispatchers[204] = intCC;
    idtRequestDispatchers[205] = intCD;
    idtRequestDispatchers[206] = intCE;
    idtRequestDispatchers[207] = intCF;
    idtRequestDispatchers[208] = intD0;
    idtRequestDispatchers[209] = intD1;
    idtRequestDispatchers[210] = intD2;
    idtRequestDispatchers[211] = intD3;
    idtRequestDispatchers[212] = intD4;
    idtRequestDispatchers[213] = intD5;
    idtRequestDispatchers[214] = intD6;
    idtRequestDispatchers[215] = intD7;
    idtRequestDispatchers[216] = intD8;
    idtRequestDispatchers[217] = intD9;
    idtRequestDispatchers[218] = intDA;
    idtRequestDispatchers[219] = intDB;
    idtRequestDispatchers[220] = intDC;
    idtRequestDispatchers[221] = intDD;
    idtRequestDispatchers[222] = intDE;
    idtRequestDispatchers[223] = intDF;
    idtRequestDispatchers[224] = intE0;
    idtRequestDispatchers[225] = intE1;
    idtRequestDispatchers[226] = intE2;
    idtRequestDispatchers[227] = intE3;
    idtRequestDispatchers[228] = intE4;
    idtRequestDispatchers[229] = intE5;
    idtRequestDispatchers[230] = intE6;
    idtRequestDispatchers[231] = intE7;
    idtRequestDispatchers[232] = intE8;
    idtRequestDispatchers[233] = intE9;
    idtRequestDispatchers[234] = intEA;
    idtRequestDispatchers[235] = intEB;
    idtRequestDispatchers[236] = intEC;
    idtRequestDispatchers[237] = intED;
    idtRequestDispatchers[238] = intEE;
    idtRequestDispatchers[239] = intEF;
    idtRequestDispatchers[240] = intF0;
    idtRequestDispatchers[241] = intF1;
    idtRequestDispatchers[242] = intF2;
    idtRequestDispatchers[243] = intF3;
    idtRequestDispatchers[244] = intF4;
    idtRequestDispatchers[245] = intF5;
    idtRequestDispatchers[246] = intF6;
    idtRequestDispatchers[247] = intF7;
    idtRequestDispatchers[248] = intF8;
    idtRequestDispatchers[249] = intF9;
    idtRequestDispatchers[250] = intFA;
    idtRequestDispatchers[251] = intFB;
    idtRequestDispatchers[252] = intFC;
    idtRequestDispatchers[253] = intFD;
    idtRequestDispatchers[254] = intFE;
    idtRequestDispatchers[255] = intFF;

    for (int i = 0; i < 256; i++) {
        if (i == 0x0F) continue; // 0x0f is reserved
        setIDTValue(i, (uint32_t) idtRequestDispatchers[i], 0x8E, 0X08);
        setIDTHandler(i, (uint32_t) noHandlerDefined);
    }

    idtr.limit = (sizeof(struct InterruptDescriptor) * 256) - 1;
    idtr.base = (uint32_t) &idt;
}

void setIDTValue(uint8_t intNum, uint32_t address, uint8_t attr, uint16_t selector) {
    // if (i/dt==NULL) initIDTStructures();
    
    struct InterruptDescriptor new = {
        (uint16_t)(address & 0xFFFF),
        selector,
        0x00,
        attr,
        (uint16_t)((address >> 16) & 0xFFFF) 
        
    };

    idt[intNum] = new;
}

void commonHandler(uint32_t* stack) {
    idtRequestHandlers[stack[12]](stack);
}


void noHandlerDefined(uint32_t* stack) {
    kprint("Uninitialized interrupt ");
    kprint_hex(stack[12]);
    kprint(" with (potential) error code ");
    kprint_hex(stack[13]);
    kprint("\n");
    // error code is stack[13]
    return;
}

void enableIDT() {
    asm volatile (
        "lidt %0\n\t"
        : 
        : "m" (idtr)
        : 
    );
}