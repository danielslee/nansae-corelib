package(
    default_visibility = ["//visibility:public"],
)

cc_library(
    name = "core",
    srcs = [
        "character.cc",
        "hash_table.cc",
        "string.cc",
        "trie.cc"
        ],
    hdrs = [
        "character.h",
        "hash_table.h",
        "stream_binary_io.h",
        "string.h",
        "trie.h",
        "segmentations.h",
        ],
    deps = ["@boost//:core"]
    )

cc_test(
    name = "character_test",
    timeout = "short",
    srcs = ["character_test.cc"],
    copts = ["-Iexternal/gtest/include"],
    deps = ["//nansae/core", "@gtest//:main"]
)

cc_test(
    name = "hash_table_test",
    timeout = "short",
    srcs = ["hash_table_test.cc"],
    copts = ["-Iexternal/gtest/include"],
    deps = ["//nansae/core", "@gtest//:main"]
)

cc_test(
    name = "string_test",
    timeout = "short",
    srcs = ["string_test.cc"],
    copts = ["-Iexternal/gtest/include"],
    deps = ["//nansae/core", "@gtest//:main"]
)

cc_test(
    name = "trie_test",
    timeout = "short",
    srcs = ["trie_test.cc"],
    copts = ["-Iexternal/gtest/include"],
    deps = ["//nansae/core", "@gtest//:main"]
)

cc_test(
    name = "segmentations_test",
    timeout = "short",
    srcs = ["segmentations_test.cc"],
    copts = ["-Iexternal/gtest/include"],
    deps = ["//nansae/core", "@gtest//:main"]
)
