# @unused
load("//tutil:variables.bzl", "COPTS", "COVERAGE", "OPTIMIZE")

##################################################
config_setting(
    name = "coverage",
    values = {"define": "coverage=true"},
)

cc_library(
    name = "base",
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
    visibility = ["//visibility:public"],
)
