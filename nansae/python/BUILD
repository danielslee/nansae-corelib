load("//nansae:nansae.bzl", "nsl_py_wrap_cc")

nsl_py_wrap_cc(
    name = "pywrap_nansae",
    srcs = ["nansae.i"],
    default_python_version = "PY3",
    swig_includes = [
        "character.i",
        "hash_table.i",
        "segmentations.i",
        "string.i",
        "trie.i"
    ],
    deps = [
        "//nansae/core",
        "//third_party/python:python_headers"
    ]
)

#py_library(
#    name = "nansae",
#    srcs = [
#        "corpus.py",
#         ...
#    ],
#    deps = [
#        ":pywrap_nansae",
#        "@org_tensorflow//tensorflow:tensorflow_py"
#    ],
#)

#py_binary(
#    name = "trainer",
#    srcs = ["trainer.py"],
#    #data = [":transform"],  # a cc_binary which we invoke at run time
#    srcs_version = "PY3",
#    default_python_version = "PY3",
#    deps = [
#        ":nansae",
#        ":pywrap_nansae"
#    ],
#)

#py_test(
#  name = "lattice_test",
#  srcs = ["lattice_test.py"],
#  srcs_version = "PY3",
#  default_python_version = "PY3",
#  deps = [
#        ":pywrap_nansae",
#        ":nansae"
#  ],
#  size = "small"
#)

# (...) More tests
