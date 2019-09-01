load("//tutil:variables.bzl", "COPTS")

cc_library(
    name = "tlog",
    srcs = [
        "asynclogging.cc",
        "current_thread.cc",
        "fileutil.cc",
        "logfile.cc",
        "logging.cc",
        "logstream.cc",
        "timestamp.cc",
    ],
    hdrs = [
        "asynclogging.h",
        "count_down_latch.h",
        "current_thread.h",
        "fileutil.h",
        "fixedbuffer.h",
        "logfile.h",
        "logging.h",
        "logstream.h",
        "noncopyable.h",
        "timestamp.h",
    ],
    copts = COPTS,
    visibility = ["//visibility:public"],
)
