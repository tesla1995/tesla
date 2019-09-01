# This name is used for the directory that the repository's runfiles are stored in.
workspace(name = "com_github_test")

#####################################################################
# Depending on other Bazel projects.
local_repository(
    name = "stage3",
    path = "/home/tesla/develop/stage3",
)

#####################################################################
# Depending on external packages

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

git_repository(
    name = "com_github_gflags_gflags",
    remote = "https://github.com/gflags/gflags.git",
    tag = "v2.2.2",
)

bind(
    name = "gflags",
    actual = "@com_github_gflags_gflags//:gflags",
)

git_repository(
    name = "com_github_google_googletest",
    remote = "https://github.com/google/googletest.git",
    tag = "release-1.8.1",
)

bind(
    name = "gtest",
    actual = "@com_github_google_googletest//:gtest",
)

bind(
    name = "gtest_main",
    actual = "@com_github_google_googletest//:gtest_main",
)

git_repository(
    name = "com_github_google_glog",
    commit = "0e4ce7c0c0f7cda7cc86017abd775cecf04074e0",
    remote = "https://github.com/google/glog.git",
)

bind(
    name = "glog",
    actual = "@com_github_google_glog//:glog",
)

#git_repository(
#    name = "com_github_protocolbuffers_protobuf",
#    remote = "https://github.com/protocolbuffers/protobuf.git",
#    tag = "v3.6.1.3",
#)
#
#bind(
#    name = "protobuf",
#    actual = "@com_github_protocolbuffers_protobuf//:protobuf",
#)

http_archive(
  name = "com_google_protobuf",
  strip_prefix = "protobuf-3.6.1.3",
  urls = ["https://github.com/protocolbuffers/protobuf/archive/v3.6.1.3.tar.gz"],
)

http_archive(
  name = "com_google_gpreftools",
  strip_prefix = "gperftools-2.7",
  urls = ["https://github.com/gperftools/gperftools/releases/download/gperftools-2.7/gperftools-2.7.tar.gz"],
)

#git_repository(
#    name = "com_github_google_benchmark",
#    remote = "https://github.com/google/benchmark.git",
#    tag = "v1.4.1"
#)
#bind(
#    name = "benchmark",
#    actual = "@com_github_google_benchmark//:benchmark",
#)

git_repository(
    name = "com_github_google_cctz",
    commit = "b820d31e80948a64ad68b88799759e098bc3a879",
    remote = "https://github.com/google/cctz.git",
)

bind(
    name = "civil_time",
    actual = "@com_github_google_cctz//:civil_time",
)

bind(
    name = "time_zone",
    actual = "@com_github_google_cctz//:time_zone",
)
