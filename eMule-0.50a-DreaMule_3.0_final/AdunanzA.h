#ifndef _ADUNANZA_H_
#define _ADUNANZA_H_


#define ADUTIP_FAKE              0x0
#define ADUTIP_TRANSFER           0x1
#define ADUTIP_PRIMEIRARODADA    0x2
#define ADUTIP_A3EW              0x4
#define ADUTIP_FORMATOFONTI      0x8
#define ADUTIP_BROKENKAD        0x10
#define ADUTIP_LOWUPLIMITS      0x20
#define ADUTIP_LOWDOWNLIMITS    0x40


extern void AduTipBlock(uint32 adutip);
extern bool AduTipShow(uint32 adutip);
extern void AduTipLowUp(void);



#endif

