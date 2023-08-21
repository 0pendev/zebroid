const packer = @import("packer.zig");
const unpacker = @import("unpacker.zig");
const std = @import("std");
const debug = std.debug;
const time = std.time;

pub fn main() !void {
    // to prevent conflicts with the C code we are using
    var allocator = std.heap.c_allocator;
    // gets the binary from memory
    var binary = try packer.getBinary(allocator);
    try unpacker.unpack(binary, allocator);
}
