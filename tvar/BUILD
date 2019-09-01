load("//tutil:variables.bzl", "COPTS")
##################################################
cc_library(
  name = "tvar",

  srcs = glob([
    "detail/*.cc",
    "*.cc",
  ]),

  hdrs = glob([
    "detail/*.h",
    "*.h"
  ]),

  deps = [
    "//tutil:tutil",
    "//external:gflags",
  ],

  visibility = ["//visibility:public"],

  copts = COPTS,
)
