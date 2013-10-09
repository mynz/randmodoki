.PATH: ${.CURDIR}

KMOD= randmodoki
SRCS= randmodoki.c device_if.h bus_if.h

.include <bsd.kmod.mk>

