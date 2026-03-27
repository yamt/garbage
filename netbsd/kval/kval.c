
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <kvm.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
		const char *name = strtok(*argv, "+");
		const char *offstr = strtok(NULL, "+");
		if (offstr != NULL) {
			char *ep;
			errno = 0;
			uintmax_t um = strtoumax(offstr, &ep, 0);
			if (ep == offstr || *ep != 0 || errno != 0) {
				fprintf(stderr, "invalid offset: %s", offstr);
				exit(1);
			}
			offset = um;
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
			uint32_t v;
			ssize_t nread;
			size_t want;

			want = sizeof(v);
			nread = kvm_read(kd, addr + offset, &v, want);
			if (nread == -1) {
				fprintf(stderr, "kvm_read: %s\n",
				    kvm_geterr(kd));
				exit(1);
			}
			if (nread != want) {
				fprintf(stderr, "kvm_read: short read\n");
				exit(1);
			}
			printf("%s+%zu, addr=0x%lx+%zu, val=0x%x (%d)\n",
			    nl[0].n_name, offset, addr, offset, v, v);
		}
	}

	if (kvm_close(kd)) {
		fprintf(stderr, "kvm_close: %s\n", kvm_geterr(kd));
		exit(1);
	}
	exit(0);
}
