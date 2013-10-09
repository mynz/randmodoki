.PATH: ${.CURDIR}

KMOD= zeromodoki
SRCS= zeromodoki.c device_if.h bus_if.h

.include <bsd.kmod.mk>

