/*
 * usage example:
 *
 *   uma% sudo ./kval _kernel_lock_cacheline+8,8
 *   _kernel_lock_cacheline+8, addr=0xffffffff81ab5200+8
 *   00 20 2d 50 92 9c ff ff
 *   uint64_t: 18446634750957133824 0xffff9c92502d2000
 *   uma%
 */

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <kvm.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static size_t
str_to_size(const char *cp)
{
	char *ep;
	errno = 0;
	uintmax_t um = strtoumax(cp, &ep, 0);
	if (ep == cp || *ep != 0 || errno != 0 || um > SIZE_MAX) {
		fprintf(stderr, "invalid size: %s", cp);
		exit(1);
	}
	return (size_t)um;
}

int
main(int argc, char *argv[])
{
	kvm_t *kd;
	char buf[_POSIX2_LINE_MAX];
	struct nlist nl[2];

	kd = kvm_openfiles(NULL, NULL, NULL, 0, buf);
	if (kd == NULL) {
		fprintf(stderr, "kvm_openfiles: %s\n", buf);
		exit(1);
	}

	nl[1].n_name = NULL;
	argv++;
	argc--;
	while (argc > 0) {
		int ninval;
		unsigned long addr;
		size_t offset = 0;
		size_t size = 4;
		const char *name = strtok(*argv, "+");
		const char *offstr = strtok(NULL, ",");
		const char *sizestr = strtok(NULL, ",");
		if (offstr != NULL) {
			offset = str_to_size(offstr);
		}
		if (sizestr != NULL) {
			size = str_to_size(sizestr);
		}
		nl[0].n_name = name;
		argv++;
		argc--;

		ninval = kvm_nlist(kd, nl);
		if (ninval == -1) {
			fprintf(stderr, "kvm_nlist: %s\n", kvm_geterr(kd));
			exit(1);
		}
		if (ninval != 0) {
			fprintf(stderr, "kvm_nlist: returns %d\n", ninval);
			exit(1);
		}

		addr = nl[0].n_value;

		{
			static uint8_t *p = NULL;
			ssize_t nread;
			size_t want;

			want = size;
			p = realloc(p, size);
			if (p == NULL) {
				fprintf(stderr, "realloc failed\n");
				exit(1);
			}
			nread = kvm_read(kd, addr + offset, p, want);
			if (nread == -1) {
				fprintf(stderr, "kvm_read: %s\n",
				    kvm_geterr(kd));
				exit(1);
			}
			if (nread != want) {
				fprintf(stderr, "kvm_read: short read\n");
				exit(1);
			}
			printf("%s+%zu, addr=0x%lx+%zu\n",
			    nl[0].n_name, offset, addr, offset);

			const char *sep = "";
			size_t i;
			for (i = 0; i < size; i++) {
				printf("%s%02x", sep, p[i]);
				sep = " ";
			}
			printf("\n");
			switch (size) {
			case 4:
				printf("uint32_t: %" PRIu32 " 0x%" PRIx32 "\n",
				    *(uint32_t *)p,
				    *(uint32_t *)p);
				break;
			case 8:
				printf("uint64_t: %" PRIu64 " 0x%" PRIx64 "\n",
				    *(uint64_t *)p,
				    *(uint64_t *)p);
				break;
			}
		}
	}

	if (kvm_close(kd)) {
		fprintf(stderr, "kvm_close: %s\n", kvm_geterr(kd));
		exit(1);
	}
	exit(0);
}
