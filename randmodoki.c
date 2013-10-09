#include <sys/cdefs.h>
__FBSDID("$FreeBSD: release/9.1.0/sys/dev/null/null.c 230320 2012-01-18 21:54:34Z gnn $");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/conf.h>
#include <sys/uio.h>
#include <sys/kernel.h>
#include <sys/malloc.h>
#include <sys/module.h>
#include <sys/priv.h>
#include <sys/disk.h>
#include <sys/bus.h>
#include <sys/filio.h>

#include <machine/bus.h>
#include <machine/vmparam.h>


#include <sys/selinfo.h>
#include "randomdev.h"

/* For use with destroy_dev(9). */
static struct cdev *randmodoki_dev;

static d_write_t randmodoki_write;
static d_ioctl_t randmodoki_ioctl;
static d_read_t randmodoki_read;

static struct cdevsw randmodoki_cdevsw = {
	.d_version =	D_VERSION,
	.d_read =	randmodoki_read,
	.d_write =	randmodoki_write,
	.d_ioctl =	randmodoki_ioctl,
	.d_name =	"randmodoki",
	.d_flags =	D_MMAP_ANON,
};

/* ARGSUSED */
static int
randmodoki_write(struct cdev *dev __unused, struct uio *uio, int flags __unused)
{
	uio->uio_resid = 0;

	return (0);
}

/* ARGSUSED */
static int
randmodoki_ioctl(struct cdev *dev __unused, u_long cmd, caddr_t data __unused,
	   int flags __unused, struct thread *td)
{
	int error;
	error = 0;

	switch (cmd) {
	case FIONBIO:
		break;
	case FIOASYNC:
		if (*(int *)data != 0)
			error = EINVAL;
		break;
	default:
		error = ENOIOCTL;
	}
	return (error);
}


/* ARGSUSED */
static int
randmodoki_read(struct cdev *dev __unused, struct uio *uio, int flag)
{
#if 1
	int c, error = 0;
	void *random_buf;

	/* Blocking logic */
	if (!random_systat.seeded)
		error = (*random_systat.block)(flag);

	/* The actual read */
	if (!error) {

		random_buf = (void *)malloc(PAGE_SIZE, M_TEMP, M_WAITOK);

		while (uio->uio_resid > 0 && !error) {
			c = MIN(uio->uio_resid, PAGE_SIZE);
			c = (*random_systat.read)(random_buf, c);
			error = uiomove(random_buf, c, uio);
		}
		free(random_buf, M_TEMP);
	}
	return (error);
#else
	void *zbuf;
	ssize_t len;
	int error = 0;

	KASSERT(uio->uio_rw == UIO_READ,
	    ("Can't be in %s for write", __func__));
	zbuf = __DECONST(void *, zero_region);
	while (uio->uio_resid > 0 && error == 0) {
		len = uio->uio_resid;
		if (len > ZERO_REGION_SIZE)
			len = ZERO_REGION_SIZE;
		error = uiomove(zbuf, len, uio);
	}

	return (error);
#endif
}

/* ARGSUSED */
static int
null_modevent(module_t mod __unused, int type, void *data __unused)
{
	switch(type) {
	case MOD_LOAD:
		if (bootverbose)
			printf("randmodoki: <randmodoki device>\n");
		randmodoki_dev = make_dev_credf(MAKEDEV_ETERNAL_KLD, &randmodoki_cdevsw, 0,
		    NULL, UID_ROOT, GID_WHEEL, 0666, "randmodoki");
		break;

	case MOD_UNLOAD:
		destroy_dev(randmodoki_dev);
		break;

	case MOD_SHUTDOWN:
		break;

	default:
		return (EOPNOTSUPP);
	}

	return (0);
}

DEV_MODULE(randmodoki, null_modevent, NULL);
MODULE_VERSION(randmodoki, 1);
