# @unused
load("//tutil:variables.bzl", "COPTS", "COVERAGE", "OPTIMIZE")

##################################################
config_setting(
    name = "coverage",
    values = {"define": "coverage=true"},
)

cc_library(
    name = "wait_free",
    srcs = glob([
        "*.cc",
    ],
    exclude = [
      "*test*.cc",
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

cc_binary(
  name = "lifo_test",
  srcs = ["lifo_test.cc"],
  deps = [
    ":wait_free",
    "//tutil:tutil",
  ],
  copts = COPTS + select({
      ":coverage": COVERAGE,
      "//conditions:default": [],
  }),
  linkopts = [
      '-lpthread',
  ],
)

cc_binary(
  name = "lifo_test_v2",
  srcs = ["lifo_test_v2.cc"],
  deps = [
    ":wait_free",
    "//tutil:tutil",
  ],
  copts = COPTS + select({
      ":coverage": COVERAGE,
      "//conditions:default": [],
  }),
  linkopts = [
      '-lpthread',
      '-latomic',
  ],
)
