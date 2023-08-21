const std = @import("std");
const coff = std.coff;
const mem = std.mem;

pub fn getBinary(allocator: mem.Allocator) !coff.Coff {
    var binary: coff.Coff = undefined;
    var embeded = @embedFile("data.raw");
    var data: []u8 = undefined;
    const key: u8 = 0x42;

    data = try allocator.alloc(u8, embeded.len);
    for (embeded, 0..) |raw, index| {
        data[index] = raw ^ key;
    }
    binary = try coff.Coff.init(data);
    return binary;
}
