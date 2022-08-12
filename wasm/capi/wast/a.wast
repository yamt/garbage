;; https://github.com/bytecodealliance/wasm-micro-runtime/issues/1353

(module (memory (export "m1") 1))

(register "mem")

(module
(memory (import "mem" "m1") 0)
(func (export "inc") (param $i i32) (i32.store (local.get $i) (i32.add (i32.const 1) (i32.load (local.get $i)))))
)

(register "sub")

(module
(memory (import "mem" "m1") 0)
(func (export "inc") (import "sub" "inc") (param $i i32))
(func (export "get") (param $i i32) (result i32) (i32.load (local.get $i)))
)

(assert_return (invoke "get" (i32.const 0)) (i32.const 0))
(assert_return (invoke "inc" (i32.const 0)))
(assert_return (invoke "get" (i32.const 0)) (i32.const 1))
(assert_return (invoke "inc" (i32.const 0)))
(assert_return (invoke "get" (i32.const 0)) (i32.const 2))
