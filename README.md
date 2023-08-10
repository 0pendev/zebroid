# Zebroid
A simple COFF/PE packer base on [this tutorial](https://github.com/frank2/packer-tutorial) and [this blogpost for TLS](https://kaimi.io/en/2012/09/developing-pe-file-packer-step-by-step-step-6-tls/), and [this blogpost for exports](https://kaimi.io/en/2012/09/developing-pe-file-packer-step-by-step-step-8-dlls-and-exports/).

Also, not perfect but [this documentation](https://learn.microsoft.com/en-us/windows/win32/debug/pe-format).

It is pretty unusable and is just a POC to try out zig build scripts and to learn about the PE format.

# Usage
Update the path to the binary to embed in build.zig and run:

```bash
zig build -Dtarget=x86_64-windows -Drelease-fast
``` 