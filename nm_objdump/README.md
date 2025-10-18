# ELF Format and Command Line Tools

## General

This repository provides an overview of the Executable and Linkable Format (ELF), commonly used in Unix-like operating systems for executables, object code, shared libraries, and core dumps. It also covers the usage of the `nm` and `objdump` commands, which are essential tools for examining ELF files.

## What is the ELF Format?

ELF (Executable and Linkable Format) is a binary file format that allows for the efficient storage and execution of programs. It consists of various segments and sections that define how the executable should be loaded into memory and executed by the operating system.

### Key Characteristics:
- **Modular**: Supports dynamic linking and loading.
- **Flexible**: Can represent different architectures.
- **Portable**: Can be used across various platforms.

## What are the Commands `nm` and `objdump`?

### `nm`

The `nm` command is used to list symbols from object files. It can help developers understand the symbols (functions, variables) defined and referenced in ELF files.

### `objdump`

`objdump` provides detailed information about object files. It can display headers, disassemble machine code, and show the contents of sections.

## How to Use Them Properly

### Using `nm`

To display symbols from an ELF file:

```bash
nm [options] <file>
