# CodeQL_demo
A demo for the CodeQL tool - converts source code into a database of code structures which are query-able and helpful for vulnerability discovery.

A few programs with vulnerabilities:
- program1.c
- program2.c
- program3.c

## Setup

[Download CodeQL CLI binaries](https://github.com/github/codeql-cli-binaries/releases)
- codeql-osx64.zip for mac OS, codeql-win64.zip for windows, and codeql-linux64.zip for linux

Clone our repo with:
  git clone 

  codeql database create test_db --language=c --source-root=./vulnerable_programs

