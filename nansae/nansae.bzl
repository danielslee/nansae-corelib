def _py_wrap_cc_impl(ctx):
  srcs = ctx.files.srcs
  if len(srcs) != 1:
    fail("Exactly one SWIG source file label must be specified.", "srcs")
  module_name = ctx.attr.module_name
  src = ctx.files.srcs[0]
  inputs = set([src])
  inputs += ctx.files.swig_includes
  for dep in ctx.attr.deps:
    inputs += dep.cc.transitive_headers
  inputs += ctx.files._swiglib
  swig_include_dirs = set(_get_repository_roots(ctx, inputs))
  swig_include_dirs += sorted([f.dirname for f in ctx.files._swiglib])
  args = ["-c++",
          "-python",
          "-module", module_name,
          "-o", ctx.outputs.cc_out.path,
          "-outdir", ctx.outputs.py_out.dirname]
  args += ["-l" + f.path for f in ctx.files.swig_includes]
  args += ["-I" + i for i in swig_include_dirs]
  args += [src.path]
  outputs = [ctx.outputs.cc_out,
             ctx.outputs.py_out]
  ctx.action(executable=ctx.executable._swig,
             arguments=args,
             inputs=list(inputs),
             outputs=outputs,
             mnemonic="PythonSwig",
             progress_message="SWIGing " + src.path)
  return struct(files=set(outputs))

def _get_repository_roots(ctx, files):
  """Returns abnormal root directories under which files reside.
  When running a ctx.action, source files within the main repository are all
  relative to the current directory; however, files that are generated or exist
  in remote repositories will have their root directory be a subdirectory,
  e.g. bazel-out/local-fastbuild/genfiles/external/jpeg_archive. This function
  returns the set of these devious directories, ranked and sorted by popularity
  in order to hopefully minimize the number of I/O system calls within the
  compiler, because includes have quadratic complexity.
  """
  result = {}
  for f in files:
    root = f.root.path
    if root:
      if root not in result:
        result[root] = 0
      result[root] -= 1
    work = f.owner.workspace_root
    if work:
      if root:
        root += "/"
      root += work
    if root:
      if root not in result:
        result[root] = 0
      result[root] -= 1
  return [k for v, k in sorted([(v, k) for k, v in result.items()])]

_py_wrap_cc = rule(
    attrs = {
        "srcs": attr.label_list(
            mandatory = True,
            allow_files = True,
        ),
        "swig_includes": attr.label_list(
            cfg = "data",
            allow_files = True,
        ),
        "deps": attr.label_list(
            allow_files = True,
            providers = ["cc"],
        ),
        "module_name": attr.string(mandatory = True),
        "py_module_name": attr.string(mandatory = True),
        "_swig": attr.label(
            default = Label("@swig//:swig"),
            executable = True,
            cfg = "host",
        ),
        "_swiglib": attr.label(
            default = Label("@swig//:templates"),
            allow_files = True,
        ),
    },
    outputs = {
        "cc_out": "%{module_name}.cc",
        "py_out": "%{py_module_name}.py",
    },
    implementation = _py_wrap_cc_impl,
)

def nsl_py_wrap_cc(name, srcs, swig_includes=[], deps=[], copts=[], **kwargs):
  module_name = name.split("/")[-1]
  # Convert a rule name such as foo/bar/baz to foo/bar/_baz.so
  # and use that as the name for the rule producing the .so file.
  cc_library_name = "/".join(name.split("/")[:-1] + ["_" + module_name + ".so"])
  cc_library_pyd_name = "/".join(name.split("/")[:-1] + ["_" + module_name + ".pyd"])
  extra_deps = []
  _py_wrap_cc(name=name + "_py_wrap",
              srcs=srcs,
              swig_includes=swig_includes,
              deps=deps + extra_deps,
              module_name=module_name,
              py_module_name=name)

  native.cc_binary(
      name=cc_library_name,
      srcs=[module_name + ".cc"],
      copts=(copts + ["-Wno-self-assign",
                      "-Wno-sign-compare",
                      "-Wno-write-strings"]),
      linkstatic=1,
      linkshared=1,
      deps=deps + extra_deps)
  native.genrule(
      name = "gen_" + cc_library_pyd_name,
      srcs = [":" + cc_library_name],
      outs = [cc_library_pyd_name],
      cmd = "cp $< $@",
  )
  native.py_library(name=name,
                    srcs=[":" + name + ".py"],
                    srcs_version="PY2AND3",
                    data=[":"+cc_library_name])
                    #select({
                    #  "//tensorflow:windows": [":" + cc_library_pyd_name],
                    #  "//conditions:default": [":" + cc_library_name],
                    #}))
