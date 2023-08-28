const pe = @cImport({
    @cInclude("pe/pe.h");
});
const std = @import("std");
const coff = std.coff;
const io = std.io;
const mem = std.mem;

pub const PackerError = error{
    BinaryMappingError,
    ImportLoadingError,
    RelocationError,
    ExportLoadingError,
};

fn buildVirtualImage(binary: coff.Coff, allocator: mem.Allocator) ![]u8 {
    var data: []const u8 = binary.data;
    var imageSize: u32 = binary.getOptionalHeader64().size_of_image;
    var image: []u8 = try allocator.alloc(u8, imageSize);
    errdefer allocator.free(image);
    var imageStream = io.fixedBufferStream(image);
    var imageWriter = imageStream.writer();
    var imageHeader = data[0..binary.getOptionalHeader64().size_of_headers];
    _ = try imageWriter.write(imageHeader);

    for (binary.getSectionHeaders()) |header| {
        var section = binary.getSectionData(&header);
        var virtualPointer = header.virtual_address;
        try imageStream.seekTo(virtualPointer);
        _ = try imageWriter.write(section);
    }

    return image;
}

pub fn unpack(binary: coff.Coff, allocator: mem.Allocator) !void {
    var image = try buildVirtualImage(binary, allocator);
    defer allocator.free(image);

    var virtual = pe.ImageLoadToMemory(image.ptr, image.len);
    if (virtual == null) {
        return PackerError.BinaryMappingError;
    }
    if (pe.ImageLoadImports(virtual) == pe.FALSE) {
        return PackerError.ImportLoadingError;
    }
    if (pe.ImageRelocate(virtual) == pe.FALSE) {
        return PackerError.RelocationError;
    }
    pe.ImageRunEntrypoint(virtual);
}
