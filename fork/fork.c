#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern char *optarg;
extern int optind;

int
main(int argc, char **argv)
{
        int njobs = 2;

        int ch;
        while ((ch = getopt(argc, argv, "j:")) != -1) {
                switch (ch) {
                case 'j':
                        njobs = atoi(optarg);
                        break;
                }
        }
        argc -= optind;
        argv += optind;
        if (argc == 0) {
                fprintf(stderr, "not enough arguments\n");
                exit(2);
        }

        int devnull = open("/dev/null", O_RDWR);
        if (devnull == -1) {
                fprintf(stderr, "open(/dev/null) failed with %d (%s)\n", errno,
                        strerror(errno));
                exit(1);
        }

        pid_t children[njobs];
        int nforked = 0;
        int nterminated = 0;

        while (nforked < njobs) {
                pid_t pid = fork();
                if (pid == -1) {
                        fprintf(stderr, "fork failed with %d (%s)\n", errno,
                                strerror(errno));
                        /* XXX cleanup */
                        exit(1);
                }
                if (pid == 0) {
                        /* child */
                        dup2(devnull, STDOUT_FILENO);
                        close(devnull);
                        execv(argv[0], argv);
                        fprintf(stderr, "execv failed with %d (%s)\n", errno,
                                strerror(errno));
                        /* XXX cleanup */
                        exit(1);
                }
                /* parent */
                children[nforked++] = pid;
        }

        int exit_status = 0;
        while (nterminated < nforked) {
                int status;
                pid_t pid = waitpid(-1, &status, 0);
                if (pid == -1) {
                        fprintf(stderr, "waitpid failed with %d (%s)\n", errno,
                                strerror(errno));
                        exit(1);
                }
                if (WIFEXITED(status)) {
                        int s = WEXITSTATUS(status);
                        printf("child (pid=%u) terminated: exit-status=%d\n",
                               (unsigned int)pid, s);
                        if (exit_status == 0) {
                                exit_status = s;
                        }
                        continue;
                }
                if (WIFSIGNALED(status)) {
                        int sig = WTERMSIG(status);
                        printf("child (pid=%u) signaled: signal=%d (%s) "
                               "dump=%d\n",
                               (unsigned int)pid, sig, strsignal(sig),
                               (int)WCOREDUMP(status));
                        if (exit_status == 0) {
                                exit_status = 1; /* XXX */
                        }
                        continue;
                }
                /* WIFSTOPPED? */
        }
        exit(exit_status);
}
