# @unused
load("//tutil:variables.bzl", "COPTS", "COVERAGE", "OPTIMIZE")

##################################################
config_setting(
    name = "coverage",
    values = {"define": "coverage=true"},
)

cc_library(
    name = "allocator",
    srcs = glob([
        "*.cc",
    ]),
    hdrs = glob([
        "*.h",
    ]),
    copts = COPTS + select({
        ":coverage": COVERAGE,
        "//conditions:default": [],
    }),
    linkopts = OPTIMIZE,
    visibility = ["//visibility:public"],
)
