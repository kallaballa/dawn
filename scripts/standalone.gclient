# Copy this file to <dawn clone dir>/.gclient to bootstrap gclient in a
# standalone checkout of Dawn.

solutions = [
  { "name"        : ".",
    "url"         : "https://github.com/kallaballa/dawn.git",
    "deps_file"   : "DEPS",
    "managed"     : False,
  },
]
