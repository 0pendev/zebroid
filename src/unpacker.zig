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
    var headers = binary.getSectionHeaders();
    var dataStream = io.fixedBufferStream(data);
    var dataReader = dataStream.reader();
    var imageStream = io.fixedBufferStream(image);
    var imageWriter = imageStream.writer();
    var imageHeader = data[0..binary.getOptionalHeader64().size_of_headers];
    _ = try imageWriter.write(imageHeader);

    for (headers) |header| {
        var virtualPointer = header.virtual_address;
        var rawPointer = header.pointer_to_raw_data;
        var rawSize = header.size_of_raw_data;
        try dataStream.seekTo(rawPointer);
        try imageStream.seekTo(virtualPointer);
        var buffer = try allocator.alloc(u8, rawSize);
        defer allocator.free(buffer);
        _ = try dataReader.read(buffer);
        _ = try imageWriter.write(buffer);
    }

    return image;
}

pub fn unpack(binary: coff.Coff, allocator: mem.Allocator) !void {
    const FALSE: c_int = 0;
    var image = try buildVirtualImage(binary, allocator);
    defer allocator.free(image);

    var virtual = pe.ImageLoadToMemory(image.ptr, image.len);
    if (virtual == null) {
        return PackerError.BinaryMappingError;
    }
    if (pe.ImageLoadImports(virtual) == FALSE) {
        return PackerError.ImportLoadingError;
    }
    if (pe.ImageRelocate(virtual) == FALSE) {
        return PackerError.RelocationError;
    }
    pe.ImageRunEntrypoint(virtual);
}
