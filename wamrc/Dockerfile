FROM yamt/ubuntu as base

WORKDIR /root/src
RUN git clone --depth 1 https://github.com/bytecodealliance/wasm-micro-runtime

FROM base as build

WORKDIR wasm-micro-runtime/wamr-compiler
RUN ./build_llvm.sh
WORKDIR b
RUN cmake ..
RUN make
RUN install wamrc /usr/local/bin

FROM base as build-xtensa

WORKDIR wasm-micro-runtime/wamr-compiler
RUN ./build_llvm_xtensa.sh
WORKDIR b
RUN cmake ..
RUN make
RUN install wamrc /usr/local/bin/wamrc-xtensa

FROM scratch
COPY --from=build /usr/local/bin /usr/local/bin
COPY --from=build-xtensa /usr/local/bin /usr/local/bin
