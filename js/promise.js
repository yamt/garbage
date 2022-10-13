// new Promise puts it on microtask
// https://developer.mozilla.org/en-US/docs/Web/API/HTML_DOM_API/Microtask_guide
// https://developer.mozilla.org/en-US/docs/Web/API/HTML_DOM_API/Microtask_guide/In_depth
// https://javascript.info/microtask-queue
// https://javascript.info/event-loop
p = new Promise(resolve => {
    setTimeout(() => {
        console.log("timer")
        resolve("resolved")
    }, 1000)
})
p.then(() => {console.log("promise then")})

// https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Statements/async_function
async function foo()
{
    await console.log("foo1")
    console.log("foo2")
}
foo().then(() => {console.log("async then")})

// the above foo() is basically same as:
async function foo()
{
    return Promise.resolve(console.log("foo1")).then(() => console.log("foo2"))
}
foo().then(() => {console.log("async then")})

console.log("main thread exiting")
