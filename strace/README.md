# Strace and Process Tracing Guide

This README provides an overview of key concepts in Linux process tracing, focusing on `strace` and the underlying `ptrace` system call. It covers the fundamentals, purposes, differences between tracing modes, and techniques for memory access in traced processes. This guide is intended for developers, system administrators, and security researchers interested in debugging, monitoring, or analyzing running processes.

## Table of Contents
- [What is strace, and what is its purpose?](#what-is-strace-and-what-is-its-purpose)
- [What is ptrace, and what possibilities it offers?](#what-is-ptrace-and-what-possibilities-it-offers)
- [Differences between PTRACE_SINGLESTEP and PTRACE_SYSCALL](#differences-between-ptrace_singlestep-and-ptrace_syscall)
- [How to read from and/or write to the memory of a process being traced](#how-to-read-from-andor-write-to-the-memory-of-a-process-being-traced)
- [Further Reading](#further-reading)

## What is strace, and what is its purpose?

`strace` is a powerful debugging and diagnostic tool available on Linux and other Unix-like systems. It is used to trace system calls and signals made by a running process or command.

### Purpose
- **System Call Tracing**: `strace` intercepts and logs all system calls (e.g., `open`, `read`, `write`, `fork`) invoked by the target process, including their arguments and return values. This helps diagnose issues like file access failures, permission errors, or performance bottlenecks caused by kernel interactions.
- **Signal Handling**: It also traces signals received by the process, showing how the program responds to interrupts or terminations.
- **Debugging and Profiling**: Ideal for reverse-engineering binaries, understanding program behavior without source code, or identifying why an application crashes or hangs.
- **Common Use Cases**:
  - Running `strace -f -e trace=file ./myprogram` to trace file-related calls in a multi-threaded program.
  - Output is typically logged to stderr or a file (e.g., `strace -o trace.log ./myprogram`).

`strace` works by forking a child process and using the `ptrace` system call to attach to it (for commands) or an existing process (via PID). It's non-intrusive for short-lived processes but can slow down execution due to the overhead of tracing.

## What is ptrace, and what possibilities it offers?

`ptrace` (Process Trace) is a Linux system call that enables one process (the "tracer") to observe and control the execution of another process (the "tracee"). It is the foundational mechanism behind tools like `strace`, `gdb`, and `ltrace`.

### How ptrace Works
- The tracer attaches to the tracee using `PTRACE_ATTACH` (for existing processes) or by forking and setting `PTRACE_TRACEME` in the child.
- Once attached, the tracee stops at key points (e.g., system call entry/exit), allowing the tracer to inspect or modify its state.
- Key operations are performed via the `ptrace` function: `ptrace(request, pid, addr, data)`, where `request` specifies the action (e.g., `PTRACE_PEEKDATA` for reading memory).

### Possibilities Offered by ptrace
- **Execution Control**: Single-step through instructions (`PTRACE_SINGLESTEP`), continue execution (`PTRACE_CONT`), or stop at signals.
- **Register and Memory Access**: Read/write CPU registers (`PTRACE_GETREGS`/`PTRACE_SETREGS`) or memory regions (`PTRACE_PEEKDATA`/`PTRACE_POKEDATA`).
- **System Call Interception**: Monitor or modify system calls before/after they occur (`PTRACE_SYSCALL`).
- **Signal Injection**: Send signals to the tracee (`PTRACE_SYSCALL` or `PTRACE_CONT` with signal parameters).
- **Advanced Use Cases**:
  - **Debugging**: GDB uses `ptrace` to set breakpoints and inspect stack traces.
  - **Security Tools**: Sandboxing (e.g., seccomp filters) or malware analysis by hooking syscalls.
  - **Process Injection**: Inject code or libraries into running processes (e.g., for hot-patching).
  - **Reverse Engineering**: Trace execution flow in closed-source binaries.

**Limitations**: Requires root privileges for attaching to arbitrary processes (unless the tracee opts in). It can be detected by anti-debugging techniques, and multi-threaded processes add complexity due to thread-specific tracing.

## Differences between PTRACE_SINGLESTEP and PTRACE_SYSCALL

Both `PTRACE_SINGLESTEP` and `PTRACE_SYSCALL` are `ptrace` requests used to control tracee execution, but they differ in granularity and focus:

| Aspect                  | PTRACE_SINGLESTEP                                                                 | PTRACE_SYSCALL                                                                    |
|-------------------------|-----------------------------------------------------------------------------------|-----------------------------------------------------------------------------------|
| **Purpose**            | Advances the tracee by exactly one machine instruction (single-stepping).         | Stops the tracee at system call entry and exit points.                            |
| **Granularity**        | Fine-grained: Steps through every CPU instruction, including user code and syscalls. | Coarse-grained: Only pauses at syscall boundaries, allowing normal execution in between. |
| **Use Case**           | Instruction-level debugging (e.g., in GDB for breakpoints or disassembly analysis). | Syscall monitoring (e.g., in `strace` to log kernel interactions without tracing every line of code). |
| **Overhead**           | High: Slows down execution significantly due to frequent stops.                   | Lower: Faster for long-running processes, as it skips user-space instructions.    |
| **Behavior on Syscalls**| Treats syscalls as regular instructions; tracer must handle them manually if needed. | Automatically stops before (`entry`) and after (`exit`) each syscall, providing args and return values. |
| **Example Scenario**   | Debugging assembly code or detecting subtle bugs in low-level operations.         | Auditing file I/O or network calls in a server application.                       |

In practice, `PTRACE_SINGLESTEP` is used for precise control, while `PTRACE_SYSCALL` is more efficient for high-level tracing. Tools like `strace` internally loop on `PTRACE_SYSCALL` to capture syscall details.

## How to read from and/or write to the memory of a process being traced

Accessing a tracee's memory via `ptrace` is done using `PTRACE_PEEKDATA` (read) and `PTRACE_POKEDATA` (write). These operate on word-sized units (typically 4 or 8 bytes, depending on architecture). The tracee must be stopped (e.g., via `PTRACE_ATTACH` or a tracing request).

### Reading Memory
1. Attach to the process: `ptrace(PTRACE_ATTACH, pid, NULL, NULL); waitpid(pid, &status, 0);`
2. Use `PTRACE_PEEKDATA`: Read from address `addr` into a local word variable.
   - Example (C code snippet):
     ```c
     #include <sys/ptrace.h>
     #include <sys/wait.h>
     #include <stdio.h>

     long read_memory(pid_t pid, void *addr) {
         return ptrace(PTRACE_PEEKDATA, pid, addr, NULL);
     }

     // Usage: long value = read_memory(pid, (void*)0x7fff12345678);
     // Handle errors: if (errno) perror("ptrace read failed");
     ```
   - To read larger blocks, loop over addresses (e.g., read 1KB by incrementing `addr` in 8-byte steps).
3. Detach: `ptrace(PTRACE_DETACH, pid, NULL, NULL);`

**Notes**: The address must be valid in the tracee's virtual address space (e.g., obtained via `/proc/<pid>/maps`). Reading invalid memory returns -1 and sets `errno` to `EFAULT`.

### Writing Memory
1. Similar attachment as above.
2. Use `PTRACE_POKEDATA`: Write a word from `data` to address `addr`.
   - Example (C code snippet):
     ```c
     long write_memory(pid_t pid, void *addr, long data) {
         return ptrace(PTRACE_POKEDATA, pid, addr, data);
     }

     // Usage: write_memory(pid, (void*)0x7fff12345678, 0xDEADBEEF);
     // Verify: long check = read_memory(pid, addr); // Should match 0xDEADBEEF
     ```
3. Detach as above.

**Security Considerations**:
- Writing to memory can crash the tracee if invalid (e.g., overwriting code or stack).
- Requires permissions: Tracer often needs to be root or the same user as the tracee.
- For 64-bit systems, use `long` (8 bytes); align addresses to avoid faults.
- Advanced: Use `PTRACE_PEEKUSER`/`PTRACE_POKEUSER` for registers, or `/proc/<pid>/mem` as an alternative (but `ptrace` is more portable).

**Example Full Program**: A simple tracer might attach to a child process, read its stack, modify a variable, and continue execution. Always include error handling for production use.

## Further Reading
- **Man Pages**: `man strace`, `man ptrace`, `man wait` (for status handling).
- **Documentation**: Linux kernel source (`Documentation/trace/ptrace.txt`) or the `ptrace` man page for request constants.
- **Tools**: Experiment with `gdb` (uses `ptrace` under the hood) or `ltrace` (for library calls).
- **Resources**: 
  - [Strace Official Page](https://strace.io/)
  - Books: "Linux System Programming" by Robert Love for deeper `ptrace` examples.
  - Online: Tutorials on using `ptrace` for process injection (e.g., on GitHub repos like simple tracers).

This guide is based on Linux specifics (kernel 2.6+). For other OSes (e.g., BSD), equivalents like `ktrace` may apply. If you have code examples or specific scenarios, feel free to contribute via pull requests!
