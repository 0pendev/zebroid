const libunpack = @cImport({
    @cInclude("unpacker.h");
});
const std = @import("std");
const coff = std.coff;
const io = std.io;

pub const PackerError = error{
    BinaryMappingError,
    ImportLoadingError,
    RelocationError,
};

fn buildVirtualImage(binary: coff.Coff) ![]u8 {
    var data: []const u8 = binary.data;
    var imageSize: u32 = binary.getOptionalHeader64().size_of_image;
    var image: []u8 = try binary.allocator.alloc(u8, imageSize);
    errdefer binary.allocator.free(image);
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
        var buffer = try binary.allocator.alloc(u8, rawSize);
        defer binary.allocator.free(buffer);
        _ = try dataReader.read(buffer);
        _ = try imageWriter.write(buffer);
    }

    return image;
}

pub fn unpack(binary: coff.Coff) !void {
    const FALSE: c_int = 0;
    var image = try buildVirtualImage(binary);
    defer binary.allocator.free(image);
    var virtual = libunpack.ImageLoadToMemory(image.ptr, image.len);
    if (virtual == null) {
        return PackerError.BinaryMappingError;
    }
    if (libunpack.ImageLoadImports(virtual) == FALSE) {
        return PackerError.ImportLoadingError;
    }
    if (libunpack.ImageRelocate(virtual) == FALSE) {
        return PackerError.RelocationError;
    }
    libunpack.ImageRunEntrypoint(virtual);
}
