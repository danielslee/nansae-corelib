new_http_archive(
    name = "boost",
    url = "http://downloads.sourceforge.net/project/boost/boost/1.62.0/boost_1_62_0.tar.bz2",
    build_file = "third_party/boost/boost.BUILD",
    type = "tar.bz2",
    strip_prefix = "boost_1_62_0/",
    sha256 = "36c96b0f6155c98404091d8ceb48319a28279ca0333fba1ad8611eb90afb2ca0",
  )

new_http_archive(
    name = "gtest",
    url = "https://github.com/google/googletest/archive/release-1.8.0.zip",
    sha256 = "f3ed3b58511efd272eb074a3a6d6fb79d7c2e6a0e374323d1e6bcbcc1ef141bf",
    build_file = "third_party/gtest/gtest.BUILD",
    strip_prefix = "googletest-release-1.8.0/googletest",
)

new_http_archive(
    name = "swig",
    sha256 = "58a475dbbd4a4d7075e5fe86d4e54c9edde39847cdb96a3053d87cb64a23a453",
    url = "http://cdimage.debian.org/mirror/xbmc.org/build-deps/sources/swig-3.0.8.tar.gz",
    strip_prefix = "swig-3.0.8",
    build_file = "third_party/swig/swig.BUILD"
  )

new_http_archive(
    name = "pcre",
    sha256 = "ccdf7e788769838f8285b3ee672ed573358202305ee361cfec7a4a4fb005bbc7",
    url = "http://ftp.exim.org/pub/pcre/pcre-8.39.tar.gz",
    strip_prefix = "pcre-8.39",
    build_file = "third_party/pcre/pcre.BUILD",
  )

local_repository(
  name = "org_tensorflow",
  path = "tensorflow",
  )

load('@org_tensorflow//tensorflow:workspace.bzl', 'tf_workspace')
tf_workspace()
