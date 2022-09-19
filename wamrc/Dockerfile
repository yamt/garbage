FROM yamt/ubuntu as base

WORKDIR /root/src
RUN git clone --depth 1 https://github.com/bytecodealliance/wasm-micro-runtime
RUN git -C wasm-micro-runtime log -1 > wamr-version.txt

FROM base as build

WORKDIR wasm-micro-runtime/wamr-compiler
RUN ./build_llvm.sh --project clang lldb
WORKDIR b
RUN cmake -DWAMR_BUILD_DEBUG_AOT=1 ..
RUN make
RUN install wamrc /usr/local/bin

FROM base as build-xtensa

WORKDIR wasm-micro-runtime/wamr-compiler
RUN ./build_llvm_xtensa.sh --project clang lldb
WORKDIR b
RUN cmake -DWAMR_BUILD_DEBUG_AOT=1 ..
RUN make
RUN install wamrc /usr/local/bin/wamrc-xtensa

FROM scratch
COPY --from=base /root/src/wamr-version.txt /wamr-version.txt
COPY --from=build /usr/local/bin /usr/local/bin
COPY --from=build-xtensa /usr/local/bin /usr/local/bin
