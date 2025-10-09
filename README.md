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