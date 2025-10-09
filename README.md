# CodeQL_demo
A demo for the CodeQL tool - converts source code into a database of code structures which are query-able and helpful for vulnerability discovery.

A few programs with vulnerabilities:
- program1.c
- program2.c
- program3.c

## Setup

### Setup for Mac
Using homebrew to install the command line interface for CodeQL:
```
brew install codeql
```
This will enable you to exercise all CodeQL tools in the command line, as demonstrated with the demo.

### Setup for Windows
[Download CodeQL CLI binaries](https://github.com/github/codeql-cli-binaries/releases)
- codeql-osx64.zip for mac OS, codeql-win64.zip for windows, and codeql-linux64.zip for linux

Ensure to add this to your PATH environment variable in order to reference the program upon running the `codeql` command.

### Verify your installation
```
codeql --version
```

### Motivation Behind CodeQL
CodeQL was developed by Semmle to address fundamental challenges in code security and quality analysis.

Firstly, it enhances and scales the manual code review to handle, at large scale, security risks and issues that may exist across hundreds of dependencies to thousands of lines of code.

Generated queries can effectively target certain patterns and behaviors (i.e. buffer overflow, SQL injection, use-after-free) which can automatically be generated.

#### `sample_query.ql`
```ql
/**
 * @name Array access with literal index out of bounds
 * @description Direct array access with a constant index that exceeds array size
 * @kind problem
 * @problem.severity error
 * @id cpp/array-literal-index-out-of-bounds
 */

import cpp

from ArrayExpr access, ArrayType arrayType, int index, int size
where
  arrayType = access.getArrayBase().getType().getUnspecifiedType() and
  arrayType.getArraySize() = size and
  access.getArrayOffset().getValue().toInt() = index and
  index >= size
select access, "Array access at index " + index + " exceeds array size of " + size
```

This query effectively allows you to track potential errors within programs where there exists an explicit indexing issue that is over the intended bounds of the array.

### Creating a CodeQL Database
This will allow you use CodeQL to query the code built within in order to find security vulnerabilities or any specific queries.
```
# Create database while observing the build
codeql database create my-database \
  --language=c-cpp \
  --command="make" \
  --source-root=/path/to/your/project

# Or for projects without a build system
codeql database create my-database \
  --language=c-cpp \
  --command="gcc -c *.c" \
  --source-root=.

# for projects without compilation
codeql database create my-database \
  --language=javascript \
  --source-root=. \
  --no-run-unnecessary-builds

    alternatively use if above fails: 
      --build-mode none
```

| Aspect | `--command` | `--no-run-unnecessary-builds` |
|--------|-------------|-------------------------------|
| **Purpose** | Specifies what to monitor | Optimization to skip builds |
| **When used** | Compiled languages primarily | Interpreted languages |
| **Effect** | Limits extraction to compiled code | Extracts all discoverable source files |
| **Build runs** | Yes, always | Only if necessary |

Under the hood, CodeQL effectively parses the source code using language-specific extractors
and directly builds an Abstract Syntax Tree (AST) and Control Flow Graph (CFG) for the program. 
This process extracts semantic information like type hierarchies, variable scopes, and data flow while storing all this information as relational data in a CodeQL database.

### Running Standard Suite (C/C++)
```bash
# Run all standard C/C++ security queries
codeql database analyze my-database \
  path/to/codeql-repo/cpp/ql/src/codeql-suites/cpp-security-and-quality.qls \
  --format=sarif-latest \
  --output=results.sarif

# View results in human-readable format
codeql database analyze my-database \
  path/to/codeql-repo/cpp/ql/src/codeql-suites/cpp-security-and-quality.qls \
  --format=csv \
  --output=results.csv
```

### Install Query Pack
To run custom queries, install the query pack with the following commands:
```bash
cd codeql-custom-queries-cpp
codeql pack install
```

### Running Custom Queries
```bash
codeql query run <your_query_name>.ql \
  --database=<your_database_name> \
  --output=results.bqrs

# Convert to readable format
codeql bqrs decode results.bqrs \
  --format=text
```

This will allow you to run queries above and decode them in a human-readable format.

CodeQL uses a declarative, object-oriented query language similar to SQL but designed for code analysis. 

The language includes:
- Predicates (logical relations) that define patterns in code.
```ql
// Find all static functions
predicate isStatic(Function f) {
  f.isStatic()
}

from Function f
where isStatic(f)
select f, "Static function"
```
- Classes that model code elements (functions, variables, expressions, etc.).
```
// Define a custom class for buffer allocation functions
class BufferAllocation extends FunctionCall {
  BufferAllocation() {
    this.getTarget().getName() in ["malloc", "calloc", "alloca"]
  }
  
  Expr getSize() {
    result = this.getArgument(0)
  }
}

from BufferAllocation alloc
select alloc, "Buffer allocated with size: " + alloc.getSize()
```

- Recursive queries for complex analysis like dataflow tracking.
```
// Find all functions reachable from main()
predicate reachable(Function source, Function target) {
  exists(FunctionCall call |
    call.getEnclosingFunction() = source and
    call.getTarget() = target
  )
  or
  exists(Function mid |
    reachable(source, mid) and
    reachable(mid, target)
  )
}

from Function main, Function f
where main.getName() = "main" and reachable(main, f)
select f, "Function reachable from main()"
```

___
___

# Rough CodeQL Syntax Guide for Beginners

## 1. Basic Query Structure

Every CodeQL query follows this pattern:

```ql
from [variable declarations]
where [conditions]
select [what to return]
```

**Example:**
```ql
from Function f
where f.getName() = "main"
select f, "This is the main function"
```

## 2. Variable Declarations

Declare variables with their type:

```ql
from Function f, FunctionCall fc, int lineNum
```

Multiple variables of the same type:
```ql
from Function f1, Function f2
```

## 3. Predicates (Boolean Functions)

### Simple Predicate
```ql
predicate isPublic(Function f) {
  f.isPublic()
}
```

### Predicate with Multiple Conditions
```ql
predicate isMainFunction(Function f) {
  f.getName() = "main" and
  f.getNumberOfParameters() = 0
}
```

## 4. Logical Operators

### AND - `and`
```ql
where f.isPublic() and f.getName() = "foo"
```

### OR - `or`
```ql
where f.getName() = "malloc" or f.getName() = "calloc"
```

### NOT - `not`
```ql
where not f.isStatic()
```

### Parentheses for Grouping
```ql
where (f.getName() = "foo" or f.getName() = "bar") and f.isPublic()
```

## 5. Comparisons

```ql
=     // equals
!=    // not equals
<     // less than
>     // greater than
<=    // less than or equal
>=    // greater than or equal
```

**Examples:**
```ql
where f.getNumberOfParameters() > 3
where f.getName() != "main"
```

## 6. The `exists` Quantifier

"There exists at least one entity that satisfies these conditions"

### Basic Syntax
```ql
exists(Type variable | condition)
```

**Example:**
```ql
// Find functions that contain a call to malloc
from Function f
where exists(FunctionCall call |
  call.getEnclosingFunction() = f and
  call.getTarget().getName() = "malloc"
)
select f
```

### With Multiple Variables
```ql
exists(FunctionCall call, Function target |
  call.getEnclosingFunction() = f and
  target = call.getTarget() and
  target.getName() = "free"
)
```

## 7. The `forall` Quantifier

"For all entities, this condition holds"

```ql
forall(Type variable | condition1 | condition2)
```

**Example:**
```ql
// Find functions where ALL parameters are pointers
from Function f
where forall(Parameter p |
  p = f.getAParameter() |
  p.getType() instanceof PointerType
)
select f
```

## 8. Classes

### Define a Class
```ql
class ClassName extends BaseType {
  ClassName() {
    // Constructor: defines membership
    [conditions]
  }
  
  // Member predicates
  predicate someMethod() {
    ...
  }
}
```

**Example:**
```ql
class UnsafeFunction extends Function {
  UnsafeFunction() {
    this.getName() in ["strcpy", "gets", "sprintf"]
  }
  
  predicate isDeprecated() {
    this.getName() = "gets"
  }
}
```

### Using Classes
```ql
from UnsafeFunction f
select f, "Unsafe function used"
```

## 9. Special Keywords

### `this`
Refers to the current object in a class:
```ql
class LargeFunction extends Function {
  LargeFunction() {
    this.getNumberOfLines() > 100
  }
}
```

### `result`
The return value in non-predicate functions:
```ql
int getSize(Function f) {
  result = f.getNumberOfLines()
}
```

Can return multiple values:
```ql
Function getACalledFunction(Function f) {
  exists(FunctionCall call |
    call.getEnclosingFunction() = f and
    result = call.getTarget()
  )
}
```

## 10. Common Accessors (Method Calls)

These are methods on CodeQL types:

```ql
f.getName()                    // Get function name
f.getAParameter()              // Get a parameter
f.getParameter(0)              // Get first parameter
f.getNumberOfParameters()      // Count parameters
fc.getTarget()                 // Get called function
fc.getArgument(0)              // Get first argument
e.getEnclosingFunction()       // Get containing function
f.getACallTo()                 // Get a call to this function
```

## 11. Type Checks with `instanceof`

Check if something is of a specific type:

```ql
where p.getType() instanceof PointerType
where expr instanceof FunctionCall
```

## 12. String Operations

```ql
.matches("pattern")     // Pattern matching with wildcards
.regexpMatch("regex")   // Regular expression matching
.indexOf("substring")   // Find substring position
.prefix(n)              // First n characters
.suffix(n)              // Last n characters
```

**Examples:**
```ql
where f.getName().matches("get%")        // Starts with "get"
where f.getName().regexpMatch("test.*")  // Regex pattern
```

## 13. Collections and Aggregates

### `in` - Membership Test
```ql
where f.getName() in ["malloc", "calloc", "free"]
```

### `count()` - Count Results
```ql
where count(f.getAParameter()) > 5
```

### `sum()`, `avg()`, `min()`, `max()`
```ql
select sum(int i | i = f.getNumberOfLines() | i)
```

### `any()` - Get Any Element
```ql
where f.getName() = any(string s | s in ["foo", "bar"])
```

## 14. Recursion (Transitive Closure)

### Basic Recursion
```ql
predicate calls(Function caller, Function callee) {
  // Base case
  exists(FunctionCall fc |
    fc.getEnclosingFunction() = caller and
    fc.getTarget() = callee
  )
  or
  // Recursive case
  exists(Function mid |
    calls(caller, mid) and
    calls(mid, callee)
  )
}
```

### Transitive Closure Operator `+` and `*`
```ql
// + means "one or more steps"
f.getASupertype+()

// * means "zero or more steps"
f.getASupertype*()
```

## 15. Imports

Import libraries at the top of your query:

```ql
import cpp                    // For C/C++
import java                   // For Java
import javascript             // For JavaScript
import python                 // For Python

// Import specific modules
import semmle.code.cpp.dataflow.TaintTracking
```

## 16. Select Statement Variations

### Simple Select
```ql
select f
```

### Select with Message
```ql
select f, "This is a function"
```

### Select with Multiple Columns
```ql
select f, f.getName(), f.getNumberOfParameters()
```

### Select with Links
```ql
select f, "Function $@ is unsafe", f, f.getName()
```

## 17. Comments

```ql
// Single line comment

/*
 * Multi-line
 * comment
 */

/** 
 * QLDoc comment for documentation
 */
```

## 18. Complete Example Putting It All Together

```ql
/**
 * @name Buffer overflow vulnerability
 * @description Finds uses of strcpy that may cause buffer overflows
 * @kind problem
 */

import cpp

// Define a class for dangerous functions
class DangerousBufferFunction extends Function {
  DangerousBufferFunction() {
    this.getName() in ["strcpy", "strcat", "sprintf"]
  }
}

// Predicate to check if size is verified
predicate hasLengthCheck(FunctionCall call, Variable destVar) {
  exists(FunctionCall sizeCheck |
    sizeCheck.getTarget().getName() in ["strlen", "sizeof"] and
    sizeCheck.getAnArgument().(VariableAccess).getTarget() = destVar
  )
}

// Main query
from FunctionCall call, DangerousBufferFunction dangerous, Variable dest
where
  // Call is to a dangerous function
  call.getTarget() = dangerous and
  
  // Get the destination variable
  dest = call.getArgument(0).(VariableAccess).getTarget() and
  
  // No length check found
  not hasLengthCheck(call, dest)
  
select call, 
  "Potentially unsafe call to $@ without length check",
  dangerous, dangerous.getName()
```

## 19. Common Patterns Cheat Sheet

### Find All Calls to a Function
```ql
from FunctionCall call
where call.getTarget().getName() = "malloc"
select call
```

### Find Functions That Call Another Function
```ql
from Function f, FunctionCall call
where call.getEnclosingFunction() = f and
      call.getTarget().getName() = "free"
select f
```

### Find Variables of Specific Type
```ql
from Variable v
where v.getType().getName() = "int*"
select v
```

### Follow Data Through Assignment
```ql
from Variable v, Expr source, Expr use
where source = v.getAnAssignedValue() and
      use = v.getAnAccess()
select source, use
```

### Check Function Parameters
```ql
from Function f, Parameter p
where p = f.getAParameter() and
      p.getType() instanceof PointerType
select f, p
```

## 20. Tips for Learning

1. **Start simple**: Begin with basic `from-where-select` queries
2. **Use the explorer**: Look at existing queries in the CodeQL repository
3. **Check the docs**: Each type has methods - explore them in the API docs
4. **Test incrementally**: Build queries step by step, testing each part
5. **Use `select` for debugging**: Print intermediate results to understand data flow

## Quick Reference Card

```ql
// Basic query
from Type var
where condition
select var
```

```ql
// With predicate
predicate name(Type param) { condition }
```

```ql
// With class
class Name extends BaseType {
  Name() { this.property() = value }
}
```

```ql
// With exists
exists(Type var | condition)
```

```ql
// Recursion
predicate transitive(Type a, Type b) {
  base_case(a, b)
  or
  exists(Type mid | transitive(a, mid) and transitive(mid, b))
}
```

```ql
// Operators
and, or, not
=, !=, <, >, <=, >=
in, instanceof
```