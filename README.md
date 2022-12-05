# Forest programming language
## NOTE: This language is still very much in the fetus stages of development

Forest is a programming language that is greatly inspired by both C++ and Rust. It is designed to use very little RAM as it contains types that are very memory efficient as well as offering utility to calculate complex mathematics with the help of the `std::math` library. 

My goal for this language is to be able to use it in my own small projects first but to expand it into a proper language that can do the same as C++. 

### Why?
Because I wanna have a go at making my own language, like every developer has wanted at some point in their life.

## Goals
- [ ] Compiled language (Though interpreted is also fine for now)
- [ ] Turing complete
- [ ] Statically typed
- [ ] Self-hosted (Eventually)
- [ ] Optimised

## Examples

Hello World:
```rust
i32 main(string[] args) {
    stdout.writeln("Hello, World!");
    return 0;
}
```

Printing from 1-99 (inclusive):
```rust
i32 main(string[] argv) {
    loop i, 1..99 {
        stdout.writeln(i);
    }
    return 0;
}
```

Fibonacci sequence:
```rust
ui32 fibonacci(ui8 n) {
	if (n <= 1) return n;
	return fibonacci(n - 2) + fibonacci(n - 1);
}

i32 main(string[] argv) {
    ui32 fibo = fibonacci(10);
    stdout.write(fibo); // Should print 55
    return 0;
}
```

## Language Reference
The syntax of the language itself 

### Primitives
- Integers: `i8, i16, i32, i64, ui8, ui16, ui32, ui64`
- Floats: `f8, f16, f32, f64`
- Boolean: `bool`
- Char: `char` (UTF-8)
- Pointer/Reference: `ref<T>`
- Enum: `enum`

### Non-Primitives
- String: `string` (Essentially a character array that keeps track of the size)

(included in `std::math`)
- Vectors: `vec2<T>, vec3<T>, vec4<T>` where T is one of the number primitives
    - Example: 

        `vec2<i8>` would be a Vector2 of 8-bit signed integers,

        `vec3<f32>` would be a Vector3 of 32-bit floating points.

        etc...

- Matrices: `mat[2-4]x[2-4]<T>` (all dimensions from 2x2 to 4x4 where T is one of the number primitives)
    - Example: 

        `mat2x4<ui8>` would be a 2x4 matrix of 8-bit unsigned integers

        `mat4x3<f64>` would be a 4x3 matrix of 64-bit floating points.

        etc...
- Lines: `line` (internally this is a `vec3<f16>`, but it has other methods such as checking for intersections, in the format of ax + by + c)

(included in `std::collections`)
- Array: `array<T>` or `T[]`
- Queue: `queue<T>` (FIFO, First in, First out)
- Stack: `stack<T>` (LIFO, Last in, First out)
- LinkedList: `linkedlist<T>` (An infinitely expandable list of values, keeps track of beginning node and ending node for reduced complexity)
- Set: `set<T>` (Array of unique values, can perform UNION, INTERSECT, EXCEPT)
- Tree: `tree<T>` (Hierarchical structure of values, useful for binary search trees)
- Graph: `graph<T>` (A graph of nodes, all nodes can be linked to each other, useful for pathfinding algorithms)

### Control characters
- Conditionals: `if (), else if (), else`
- Loops: `loop, until (condition), loop <variable>, begin..end` (`loop i, 0..10` = `for (ui8 i = 0; i <= 10; i++)`, type of i is inferred to most-memory friendly option) 
- Statements: `skip, break, return` (skip = continue)

### Operators
- Assignment: `=`
- Comparison: `==` (everything compared by value, for reference checking you have get reference first: `\string1 == \string2`)
- Arithmetic: `+, -, /, *, %, ^ (power of)` (arithmetic and assignment can be combined)
- Boolean logic: `|, ||, &, &&, ~ (xor), ! (not), >, <, <=, >=`
- References: `\ (get address), @ (dereference)`
- Calling operator: `.`

### Other
- Functions: `<returntype> <name>(params) {}` (`i8 add(i8 a, i8 b) { return a+b; }`)
- OOP design: `class, interface, namespace, struct`
- Inheritance: `:`
- Comments: `//single line, /*multi line*/`
- Import statements: `use namespace::package::function/type` (function/type is optional)

### I/O
- Console:
    - Writing: `stdout.write("text here")` or `stdout.writeln("Text here")` for automatic line breaks
    - Reading: `stdin.read()` or `stdin.readline()`. `read()` returns a ui32 and `readline()` returns a ui32[] of the bytes that were read (UTF-8 encoding)
- Filesystem: 
    - Writing: `fs.write(path, bytes)`
    - Reading: `ui8[] fs.read(path)`
    - Step reading: `ui32 fd = fs.open(path); f32 fs.readLE<f32>(fd)`

#### Note: 
UTF-8 encodes characters as they're needed with multiple bytes. The common ASCII characters only take 1 byte each, but the rest of the international alphabets needs more than 1 byte. Because I wanna support international files and code, I want this language to also allow for that.

Don't worry, all of the difficulty of encoding is abstracted to the user of this language. You can easily convert between strings and byte-arrays.
