# @unused
load("//tutil:variables.bzl", "COPTS", "COVERAGE", "OPTIMIZE")

##################################################
config_setting(
    name = "coverage",
    values = {"define": "coverage=true"},
)

cc_library(
    name = "fcontext",
    srcs = glob([
        "asm/*.S",
    ]),
    hdrs = glob([
        "fcontext.h",
    ]),
    copts = COPTS,
    visibility = ["//visibility:public"],
)

cc_binary(
  name = "fcontext_test",
  srcs = ["fcontext_test.cc"],
  deps = [
    ":fcontext",
    "//allocator:allocator",
  ],
)
