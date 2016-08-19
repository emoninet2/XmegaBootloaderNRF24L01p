/*
 * xmega_core.h
 *
 * Created: 18-Aug-16 9:58:53 PM
 *  Author: Emon
 */ 


#ifndef XMEGA_CORE_H_
#define XMEGA_CORE_H_



#define _MOV(toreg, fromreg)		\
(__extension__({					\
	__asm__ __volatile__			\
	(								\
	"mov %0, %1\n\t"			\
	:							\
	:	"i" (toreg),			\
	"i" (fromreg)			\
	);								\
}))

#define _TEST(toreg, val)			\
(__extension__({					\
	__asm__ __volatile__			\
	(								\
	"ldi r16, %1\n\t"				\
	"ldi r17, %0\n\t"				\
	"ldi r%2, 0x55\n\t"				\
	:								\
	:	"i" (0x99),					\
	"i" (0x69),					\
	"i" (toreg)					\
	);								\
}))

#define _LPM()				\
(__extension__({				\
	__asm__ __volatile__		\
	(							\
	"lpm \n\t"					\
	:							\
	:							\
	);							\
}))

#define _LPM_toReg(regD)		\
(__extension__({				\
	__asm__ __volatile__		\
	(							\
	"lpm r%0 ,Z \n\t"				\
	:							\
	:	"i" (regD)				\
	);							\
}))

#define _LPM_toReg_PostInc(regD)		\
(__extension__({				\
	__asm__ __volatile__		\
	(							\
	"lpm r%0 ,Z+ \n\t"				\
	:							\
	:	"i" (regD)				\
	);							\
}))


#define _ELPM()				\
(__extension__({				\
	__asm__ __volatile__		\
	(							\
	"elpm \n\t"					\
	:							\
	:							\
	);							\
}))

#define _ELPM_toReg(regD)		\
(__extension__({				\
	__asm__ __volatile__		\
	(							\
	"elpm r%0 ,Z \n\t"				\
	:							\
	:	"i" (regD)				\
	);							\
}))

#define _ELPM_toReg_PostInc(regD)		\
(__extension__({				\
	__asm__ __volatile__		\
	(							\
	"elpm r%0 ,Z+ \n\t"				\
	:							\
	:	"i" (regD)				\
	);							\
}))


#define _SPM()				\
(__extension__({				\
	__asm__ __volatile__		\
	(							\
	"spm \n\t"					\
	:							\
	:							\
	);							\
}))


#define _SPM_PostIncBy2()				\
(__extension__({				\
	__asm__ __volatile__		\
	(							\
	"spm Z+\n\t"					\
	:							\
	:							\
	);							\
}))










#endif /* XMEGA_CORE_H_ */