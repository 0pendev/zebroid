const std = @import("std");
const coff = std.coff;
const mem = std.mem;

const embeded = @embedFile("main.exe");

pub fn getBinary(allocator: mem.Allocator) !coff.Coff {
    var binary: coff.Coff = undefined;
    binary.allocator = allocator;
    try binary.parse(embeded);
    return binary;
}
