# Zebroid
A simple COFF/PE packer base on [this tutorial](https://github.com/frank2/packer-tutorial) and [this blogposts](https://kaimi.io/).

# Usage
Update the path to the binary to embed in build.zig and run:

```bash
zig build -Dtarget=x86_64-windows -Drelease-fast
```