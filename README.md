# MSRS - an LKM to Find Undocumented MSRs on x86_64
MSRS (MSR scan) is a high performance Linux kernel module that can be used to find undocumented model-specific registers (MSRs) on x86_64 CPUs.

## What are MSRs?
Model-specific registers (MSRs) are a fundamental part of the x86_64 instruction set
architecture.
They are used, among others, to toggle CPU features, debugging, and monitoring.
As an example, the IA32_LSTAR_MSR shall be noted. It contains the instruction pointer of the system-call handler, that is executed by the syscall instruction.
Model-specific in this context means that each micro-architecture may have a different set of MSRs. Both AMD and Intel provide a substantial list of documented MSRs as part of their developer manuals.

## How do I use this Module?
You need a C++ compiler that supports C++20, make, and CMake version 3.16 or above.

In the module folder execute the following to build and load the MSRS module:
```bash
make
sudo insmod msrs.ko
```

MSRS comes with a few tools to make working with it easier.
Head into the tools folder and execute
```bash
mkdir build
cd build
cmake ..
make
```

In the bin folder you can now find three executables:
* msrs_detect - this can be used to trigger a scan
* msrs_ls - prints all found MSRs
* msrs_sample - can be used to sample MSR values

## Where do I get more Information?
For further information about the file format, more information about the kernel module and the tools, already conducted research, and background information feel free to check out my bachelor thesis which you can find in the thesis.pdf file.
