#include <sys/stat.h>

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <ie/c_api/ie_c_api.h>

int
read_file(const char *path, bool add_nul, void **pp, size_t *sizep)
{
        struct stat st;
        void *p;
        size_t size;
        ssize_t ssz;
        int fd;
        int ret;

        fd = open(path, O_RDONLY);
        if (fd == -1) {
                return errno;
        }
        ret = fstat(fd, &st);
        if (ret == -1) {
                close(fd);
                return errno;
        }
        size = st.st_size;
        p = malloc(size + add_nul);
        if (p == NULL) {
                close(fd);
                return ENOMEM;
        }
        ssz = read(fd, p, size);
        if (ssz != size) {
                close(fd);
                return EIO;
        }
        if (add_nul) {
                ((char *)p)[size] = 0;
        }
        *pp = p;
        *sizep = size;
        return 0;
}

IEStatusCode
make_blob(const tensor_desc_t *tensor_desc, const void *data, size_t data_size,
          ie_blob_t **blobp)
{
        ie_blob_t *blob = NULL;
        ie_blob_buffer_t buffer;
        int buffer_size;
        IEStatusCode status;
        status = ie_blob_make_memory(tensor_desc, &blob);
        if (status != OK) {
                fprintf(stderr, "ie_blob_make_memory failed\n");
                goto fail;
        }
        status = ie_blob_get_buffer(blob, &buffer);
        if (status != OK) {
                fprintf(stderr, "ie_blob_get_buffer failed\n");
                goto fail;
        }
        status = ie_blob_byte_size(blob, &buffer_size);
        if (status != OK) {
                fprintf(stderr, "ie_blob_byte_size failed\n");
                goto fail;
        }
        assert(buffer_size == data_size);
        memcpy(buffer.buffer, data, data_size);
        *blobp = blob;
        return OK;
fail:
        if (blob != NULL) {
                ie_blob_free(&blob);
        }
        return status;
}

void
print_result(float *f, size_t n)
{
        int i;
        for (i = 0; i < n; i++) {
                printf("%u %f\n", i, f[i]);
        }
}

int
main(int argc, char **argv)
{
        int exit_status = 1;
        int ret;
        IEStatusCode status;
        void *graph_xml = NULL;
        size_t graph_xml_size;
        void *graph_weights = NULL;
        size_t graph_weights_size;
        void *tensor = NULL;
        size_t tensor_size;
        ie_core_t *core = NULL;
        ie_network_t *net = NULL;
        ie_executable_network_t *exec = NULL;
        ie_blob_t *blob_weights = NULL;
        ie_blob_t *blob_tensor = NULL;
        ie_infer_request_t *req = NULL;

        ret = read_file("../testfiles/model.xml", false, &graph_xml,
                        &graph_xml_size);
        if (ret != 0) {
                fprintf(stderr, "failed to read model.xml\n");
                goto fail;
        }
        ret = read_file("../testfiles/model.bin", false, &graph_weights,
                        &graph_weights_size);
        if (ret != 0) {
                fprintf(stderr, "failed to read model.bin\n");
                goto fail;
        }

        /* load */

        const char *plugins_xml = "/opt/intel/openvino_2022/runtime/lib/"
                                  "intel64/Debug/plugins.xml";
        // const char *plugins_xml =
        // "/opt/intel/openvino_2022/runtime/lib/intel64/Release/plugins.xml";
        status = ie_core_create(plugins_xml, &core);
        if (status != OK) {
                fprintf(stderr, "ie_core_create failed\n");
                goto fail;
        }
        tensor_desc_t tensor_desc = {
                .layout = ANY,
                .dims =
                        {
                                .ranks = 1,
                                .dims =
                                        {
                                                graph_weights_size,
                                        },
                        },
                .precision = U8,
        };
        status = make_blob(&tensor_desc, graph_weights, graph_weights_size,
                           &blob_weights);
        if (status != OK) {
                fprintf(stderr, "make_blob failed\n");
                goto fail;
        }
        status = ie_core_read_network_from_memory(
                core, graph_xml, graph_xml_size, blob_weights, &net);
        if (status != OK) {
                fprintf(stderr, "ie_core_read_network_from_memory failed\n");
                goto fail;
        }
#if 1 /* */
        size_t size;
        status = ie_network_get_inputs_number(net, &size);
        if (status != OK) {
                fprintf(stderr, "ie_network_get_inputs_number failed\n");
                goto fail;
        }
        size_t i;
        for (i = 0; i < size; i++) {
                char *name;
                status = ie_network_get_input_name(net, i, &name);
                if (status != OK) {
                        fprintf(stderr, "ie_network_get_input_name failed\n");
                        goto fail;
                }
                status = ie_network_set_input_layout(net, name, NHWC);
                if (status != OK) {
                        fprintf(stderr,
                                "ie_network_set_input_layout failed\n");
                        goto fail;
                }
        }
#endif
        ie_config_t conf = {
                NULL,
                NULL,
                NULL,
        };
        status = ie_core_load_network(core, net, "CPU", &conf, &exec);
        if (status != OK) {
                fprintf(stderr, "ie_core_load_network failed\n");
                goto fail;
        }
        fprintf(stderr, "load succeeded\n");

        ret = read_file("../testfiles/tensor.bgr", false, &tensor,
                        &tensor_size);
        if (ret != 0) {
                fprintf(stderr, "failed to read tensor.bgr\n");
                goto fail;
        }

        /* set_input */
        status = ie_exec_network_create_infer_request(exec, &req);
        if (status != OK) {
                fprintf(stderr,
                        "ie_exec_network_create_infer_request failed\n");
                goto fail;
        }
        tensor_desc_t tensor_desc2 = {
                .layout = NHWC,
                .dims =
                        {
                                .ranks = 4,
                                .dims =
                                        {
                                                1,
                                                3,
                                                224,
                                                224,
                                        },
                        },
                .precision = FP32,
        };
        status = make_blob(&tensor_desc2, tensor, tensor_size, &blob_tensor);
        if (ret != 0) {
                fprintf(stderr, "make_blob (tensor) failed\n");
                goto fail;
        }
        char *name;
        status = ie_network_get_input_name(net, 0, &name);
        if (status != OK) {
                fprintf(stderr, "ie_network_get_input_name failed\n");
                goto fail;
        }
        status = ie_infer_request_set_blob(req, name, blob_tensor);
        if (status != OK) {
                fprintf(stderr, "ie_infer_request_set_blob failed\n");
                goto fail;
        }
        fprintf(stderr, "set_input succeeded\n");

        /* compute */
        status = ie_infer_request_infer(req);
        if (status != OK) {
                fprintf(stderr, "ie_infer_request_infer failed\n");
                goto fail;
        }
        fprintf(stderr, "compute succeeded\n");

        /* get_output */
        status = ie_network_get_output_name(net, 0, &name);
        if (status != OK) {
                fprintf(stderr, "ie_network_get_output_name failed\n");
                goto fail;
        }
        ie_blob_t *blob_result = NULL;
        status = ie_infer_request_get_blob(req, name, &blob_result);
        if (status != OK) {
                fprintf(stderr, "ie_infer_request_get_blob failed\n");
                goto fail;
        }
        fprintf(stderr, "get_output succeeded\n");

        /* print result */
        ie_blob_buffer_t buffer;
        int buffer_size;
        status = ie_blob_get_buffer(blob_result, &buffer);
        if (status != OK) {
                fprintf(stderr, "ie_blob_get_buffer failed\n");
                goto fail;
        }
        status = ie_blob_byte_size(blob_result, &buffer_size);
        if (status != OK) {
                fprintf(stderr, "ie_blob_byte_size failed\n");
                goto fail;
        }
        float *f = (float *)buffer.buffer;
        print_result(f + 1, buffer_size / sizeof(*f) - 1);

        exit_status = 0;
fail:
        if (req != NULL) {
                ie_infer_request_free(&req);
        }
        if (blob_tensor != NULL) {
                ie_blob_free(&blob_tensor);
        }
        if (exec != NULL) {
                ie_exec_network_free(&exec);
        }
        if (net != NULL) {
                ie_network_free(&net);
        }
        if (blob_weights != NULL) {
                ie_blob_free(&blob_weights);
        }
        if (core != NULL) {
                ie_core_free(&core);
        }
        if (tensor != NULL) {
                free(tensor);
        }
        if (graph_weights != NULL) {
                free(graph_weights);
        }
        if (graph_xml != NULL) {
                free(graph_xml);
        }
        return exit_status;
}
