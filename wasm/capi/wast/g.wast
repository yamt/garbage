
(module (global (export "g1") (mut i32) (i32.const 100)))

(register "g")

(module
(global (import "g" "g1") (mut i32))
(func (export "inc") (global.set 0 (i32.add (i32.const 1) (global.get 0))))
(func (export "get") (result i32) (global.get 0))
)

(assert_return (invoke "inc"))
(assert_return (invoke "get") (i32.const 101))

(register "sub")

(module
(global (import "g" "g1") (mut i32))
(func (export "inc") (import "sub" "inc"))
(func (export "get") (result i32) (global.get 0))
)

(assert_return (invoke "get") (i32.const 101))
(assert_return (invoke "inc"))
(assert_return (invoke "get") (i32.const 102))
