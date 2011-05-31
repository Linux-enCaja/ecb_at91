// *----------------------------------------------------------------------------
// *         ATMEL Microcontroller Software Support  -  ROUSSET  -
// *----------------------------------------------------------------------------
// * The software is delivered "AS IS" without warranty or condition of any
// * kind, either express, implied or statutory. This includes without
// * limitation any warranty or condition with respect to merchantability or
// * fitness for any particular purpose, or against the infringements of
// * intellectual property rights of others.
// *----------------------------------------------------------------------------
// * File Name           : ohci.h
// * Object              : OHCI ED, TD HCCA definitions + inlined functions
// * Creation            : ODi   01/06/2003
// *
// *----------------------------------------------------------------------------
#ifndef AT91_OHCI_H
#define AT91_OHCI_H

typedef struct _AT91S_UHP_ED {
	volatile unsigned int Control;
	volatile unsigned int TailP;
	volatile unsigned int HeadP;
	volatile unsigned int NextEd;
} AT91S_UHP_ED, *AT91PS_UHP_ED;

typedef struct _AT91S_UHP_TD {
	volatile unsigned int Control;
	volatile unsigned int CBP;
	volatile unsigned int NextTD;
	volatile unsigned int BE;
} AT91S_UHP_TD, *AT91PS_UHP_TD;

typedef struct _AT91S_UHP_ITD {
	volatile unsigned int Control;
	volatile unsigned int BP0;
	volatile unsigned int NextTD;
	volatile unsigned int BE;
	volatile unsigned int PSW0;
	volatile unsigned int PSW1;
	volatile unsigned int PSW2;
	volatile unsigned int PSW3;
	volatile unsigned int PSW4;
	volatile unsigned int PSW5;
	volatile unsigned int PSW6;
	volatile unsigned int PSW7;
} AT91S_UHP_ITD, *AT91PS_UHP_ITD;

typedef struct _AT91S_UHP_HCCA {
	volatile unsigned int    UHP_HccaInterruptTable[32];
	volatile unsigned short  UHP_HccaFrameNumber;
	volatile unsigned short  UHP_HccaPad1;
	volatile unsigned int    UHP_HccaDoneHead;
	volatile unsigned char   reserved[116];
} AT91S_UHP_HCCA, *AT91PS_UHP_HCCA;

// *----------------------------------------------------------------------------
// * \fn    AT91F_CreateEd
// * \brief Init a pre-allocated endpoint descriptor. !!! TD must be aligned on a 16 bytes boundary
// *----------------------------------------------------------------------------
__inline void AT91F_CreateEd(
	unsigned int EDAddr,
	unsigned int MaxPacket,
	unsigned int TDFormat,
	unsigned int Skip,
	unsigned int Speed,
	unsigned int Direction,
	unsigned int EndPt,
	unsigned int FuncAddress,
	unsigned int TDQTailPntr,
	unsigned int TDQHeadPntr,
	unsigned int ToggleCarry,
	unsigned int NextED)
{
	AT91PS_UHP_ED pED = (AT91PS_UHP_ED) EDAddr;
	pED->Control = (MaxPacket << 16) | (TDFormat << 15) |
	               (Skip << 14) | (Speed << 13) | (Direction << 11) |
	               (EndPt << 7) | FuncAddress;
	pED->TailP   = (TDQTailPntr & 0xFFFFFFF0);
	pED->HeadP   = (TDQHeadPntr & 0xFFFFFFF0) | (ToggleCarry << 1);
	pED->NextEd  = (NextED & 0xFFFFFFF0);
}

// *----------------------------------------------------------------------------
// * \fn    AT91F_CreateGenTd
// * \brief Init a pre-allocated transfer descriptor. !!! TD must be aligned on a 16 bytes boundary
// *----------------------------------------------------------------------------
__inline void AT91F_CreateGenTd(
	unsigned int GenTdAddr,
	unsigned int DataToggle,
	unsigned int DelayInterrupt,
	unsigned int Direction,
	unsigned int BufRnding,
	unsigned int CurBufPtr,
	unsigned int NextTD,
	unsigned int BuffLen)
{
	AT91PS_UHP_TD pTD = (AT91PS_UHP_TD) GenTdAddr;
	pTD->Control = (DataToggle << 24) | (DelayInterrupt << 21) | (Direction << 19) | (BufRnding << 18);
	pTD->CBP     = CurBufPtr;
	pTD->NextTD  = (NextTD & 0xFFFFFFF0);
	pTD->BE      = (BuffLen) ? CurBufPtr + BuffLen - 1 : CurBufPtr;
}

// *----------------------------------------------------------------------------
// * \fn    AT91F_CreateGenITd
// * \brief Init a pre-allocated periodic transfer descriptor. !!! TD must be aligned on a 16 bytes boundary
// *----------------------------------------------------------------------------
__inline void AT91F_CreateGenITd(
	unsigned int GenTdAddr,
	unsigned int CondCode,
	unsigned int FrameCount,
	unsigned int DelayInterrupt,
	unsigned int StartFrame,
	unsigned int BuffPage0,
	unsigned int NextTD,
	unsigned int BufEnd,
	unsigned int PswOffset0,
	unsigned int PswOffset1,
	unsigned int PswOffset2,
	unsigned int PswOffset3,
	unsigned int PswOffset4,
	unsigned int PswOffset5,
	unsigned int PswOffset6,
	unsigned int PswOffset7)
{
	AT91PS_UHP_ITD pITD = (AT91PS_UHP_ITD) GenTdAddr;

	pITD->Control = (CondCode << 28) | (FrameCount << 24) | (DelayInterrupt << 21) | StartFrame;
	pITD->BP0 = (BuffPage0 << 12);
	pITD->NextTD = (NextTD << 4);
	pITD->BE   = BufEnd;
	pITD->PSW0 = PswOffset0;
	pITD->PSW1 = PswOffset1;
	pITD->PSW2 = PswOffset2;
	pITD->PSW3 = PswOffset3;
	pITD->PSW4 = PswOffset4;
	pITD->PSW5 = PswOffset5;
	pITD->PSW6 = PswOffset6;
	pITD->PSW7 = PswOffset7;

}

#endif
