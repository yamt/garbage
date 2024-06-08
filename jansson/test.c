#include <jansson.h>
#include <stdio.h>
#include <stdlib.h>

int
main(int argc, char **argv)
{
        json_t *j;
        j = json_object();
        if (j == NULL) {
                fprintf(stderr, "json_object failed\n");
                exit(1);
        }
        json_t *jint = json_integer(argc);
        if (jint == NULL) {
                fprintf(stderr, "json_inteder failed\n");
                exit(1);
        }
        if (json_object_set_new(j, "argc", jint)) {
                fprintf(stderr, "json_object_set_new failed\n");
                exit(1);
        }
        json_t *a = json_array();
        int i;
        for (i = 0; i < argc; i++) {
                json_t *s = json_string(argv[i]);
                if (s == NULL) {
                        fprintf(stderr, "json_string failed\n");
                        exit(1);
                }
                if (json_array_append_new(a, s)) {
                        fprintf(stderr, "json_array_append_new failed\n");
                        exit(1);
                }
        }
        if (json_object_set_new(j, "argv", a)) {
                fprintf(stderr, "json_object_set_new failed\n");
                exit(1);
        }
        if (json_dumpf(j, stdout, JSON_INDENT(4))) {
                fprintf(stderr, "json_dumpf failed\n");
                exit(1);
        }
        /*
         * the output of json_dumpf does't have a newline at the end of
         * the last line.
         */
        printf("\n");
}
