FROM yamt/ubuntu as build

WORKDIR /root/src
RUN git clone --depth 1 https://github.com/bytecodealliance/wasm-micro-runtime

WORKDIR wasm-micro-runtime/wamr-compiler
RUN ./build_llvm.sh
WORKDIR b
RUN cmake ..
RUN make
RUN install wamrc /usr/local/bin

WORKDIR /root/src
WORKDIR wasm-micro-runtime/core/deps
RUN rm -r llvm

WORKDIR /root/src
WORKDIR wasm-micro-runtime/wamr-compiler
RUN ./build_llvm_xtensa.sh
WORKDIR b.xtensa
RUN cmake ..
RUN make
RUN install wamrc /usr/local/bin/wamrc-xtensa

FROM scratch
COPY --from=build /usr/local/bin /usr/local/bin
