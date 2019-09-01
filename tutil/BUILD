# @unused
load("//tutil:variables.bzl", "COPTS", "COVERAGE", "OPTIMIZE")

##################################################
config_setting(
    name = "coverage",
    values = {"define": "coverage=true"},
)

cc_library(
    name = "tutil",
    srcs = glob([
        "*.cc",
        "strings/*.cc",
        "files/*.cc",
        "synchronization/*.cc",
    ]),
    hdrs = glob([
        "*.h",
        "strings/*.h",
        "files/*.h",
        "containers/*.h",
        "synchronization/*.h",
    ]),
    copts = COPTS + select({
        ":coverage": COVERAGE,
        "//conditions:default": [],
    }),
    visibility = ["//visibility:public"],
    deps = [
        "//log:tlog",
    ],
)
