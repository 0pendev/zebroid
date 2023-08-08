const std = @import("std");

fn ObfuscateEmbeded() void {
    var embeded = @embedFile("examples/example-c.exe");
    var xored: [embeded.len]u8 = undefined;
    for (embeded) |val, index| {
        xored[index] = val ^ 0x42;
    }

    var dir = std.fs.cwd();
    dir.deleteFile("src/data.raw") catch {};
    dir.writeFile("src/data.raw", &xored) catch {
        std.debug.print("Unable to write file src/data.raw", .{});
    };
}

pub fn build(b: *std.build.Builder) void {
    const target = b.standardTargetOptions(.{});
    const mode = b.standardReleaseOptions();

    ObfuscateEmbeded();
    const exe = b.addExecutable("zebroid", "src/main.zig");
    exe.setTarget(target);
    exe.setBuildMode(mode);
    const cflags = [_][]const u8{
        "-Wall",
    };
    const csource = [_][]const u8{
        "src/pe/get_nt_headers.c",
        "src/pe/image_load_exports.c",
        "src/pe/image_load_imports.c",
        "src/pe/image_load_to_memory.c",
        "src/pe/image_relocate.c",
        "src/pe/image_run_entrypoint.c",
    };
    exe.addIncludePath("src");
    exe.addCSourceFiles(&csource, &cflags);
    exe.linkLibC();
    exe.install();

    const run_cmd = exe.run();
    run_cmd.step.dependOn(b.getInstallStep());
    if (b.args) |args| {
        run_cmd.addArgs(args);
    }

    const run_step = b.step("run", "Run the app");
    run_step.dependOn(&run_cmd.step);

    const exe_tests = b.addTest("src/main.zig");
    exe_tests.setTarget(target);
    exe_tests.setBuildMode(mode);

    const test_step = b.step("test", "Run unit tests");
    test_step.dependOn(&exe_tests.step);
}
