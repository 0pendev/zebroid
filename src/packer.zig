const std = @import("std");
const coff = std.coff;
const mem = std.mem;

pub fn getBinary(allocator: mem.Allocator) !coff.Coff {
    var binary: coff.Coff = undefined;
    // TODO: once zig 0.11.0 is release use json.Reader to
    // parse a json file at comptime
    var embeded = @embedFile("data.raw");
    var data: []u8 = undefined;
    const key: u8 = 0x42;

    data = try allocator.alloc(u8, embeded.len);
    for (embeded) |raw, index| {
        data[index] = raw ^ key;
    }
    binary.allocator = allocator;
    try binary.parse(data);
    return binary;
}
