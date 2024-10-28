
const std = @import("std");

pub fn build(b : *std.Build) !void {

    const exe = b.addExecutable(.{
        .name = "editor",
        .target = b.standardTargetOptions(.{}),
    });

    const flags = [_][]const u8{
        "-Wall", "-Wextra", "-ggdb",
        "-I/usr/include/freetype2",
        "-I/usr/include/libpng16"
    };


    {
        var dir = try std.fs.cwd().openDir("src", .{ .iterate = true });

        var walker = try dir.walk(b.allocator);
        defer walker.deinit();

        while (try walker.next()) |entry| {
            const ext = std.fs.path.extension(entry.basename);
            var include_file = false;
            if(std.mem.eql(u8, ext, ".c")) {
                include_file = true;
            }
            if (include_file) {
                exe.addCSourceFile(.{
                    .file = b.path(b.pathJoin(&.{"src", entry.path})),
                    .flags = &flags,
                });
            }
        }
    }

    exe.linkSystemLibrary("c");
    exe.linkSystemLibrary("glfw");
    exe.linkSystemLibrary("GLEW");
    exe.linkSystemLibrary("freetype");
    exe.linkSystemLibrary("m");
    b.default_step.dependOn(&exe.step);


    b.installArtifact(exe);
}



