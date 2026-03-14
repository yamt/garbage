/*
 * a motivation: test /dev/drum using libkvm.
 *
 * cc -Wall -o minips minips.c -lkvm
 * ./minips
 */

#define _KMEMUSER
#include <sys/param.h>
#include <sys/sysctl.h>

#include <errno.h>
#include <fcntl.h>
#include <kvm.h>
#include <limits.h>
#include <paths.h>
#include <stdio.h>
#include <stdlib.h>

void
print_args(const char *label, char **args)
{
        if (args == NULL) {
                printf("\t%s: failed\n", label);
        } else if (args[0] == NULL) {
                printf("\t%s: empty\n", label);
        } else {
                char *cp;
                int j;
                for (j = 0, cp = args[j]; cp != NULL; cp = args[++j]) {
                        printf("\t%s[%d] %s\n", label, j, cp);
                }
        }
}

int
main(int argc, char **argv)
{
        const char *execfile = _PATH_UNIX;
        const char *corefile = _PATH_MEM;
        const char *swapfile = _PATH_DRUM;
        char errbuf[_POSIX2_LINE_MAX];
        kvm_t *kd;

        kd = kvm_openfiles(execfile, corefile, swapfile, O_RDONLY, errbuf);
        if (kd == NULL) {
                fprintf(stderr, "kvm_openfiles failed: %s\n", errbuf);
                exit(1);
        }
        struct kinfo_proc *ps;
        int cnt;
        ps = kvm_getprocs(kd, KERN_PROC_ALL, 0, &cnt);
        if (ps == NULL) {
                fprintf(stderr, "kvm_getprocs KERN_PROC_ALL failed\n");
                exit(1);
        }
        int i;
        for (i = 0; i < cnt; i++) {
                const struct kinfo_proc *kp = &ps[i];
                const struct proc *p = &kp->kp_proc;
                printf("pid %5u comm %.*s stat 0x%x\n", (unsigned int)p->p_pid,
                       MAXCOMLEN + 1, p->p_comm, p->p_stat);
                char **argv = kvm_getargv(kd, kp, 0);
                print_args("argv", argv);
                char **envv = kvm_getenvv(kd, kp, 0);
                print_args("envv", envv);
        }
}
