load("//tutil:variables.bzl", "COPTS", "OPTIMIZE", "COVERAGE")

config_setting(
    name = "coverage",
    values = {"define": "coverage=true"},
)

########################################################

## test data
#filegroup(
#    name = "certs",
#    srcs = glob([
#        "certs/*.cer",
#        "certs/*.pem",
#        "certs/*.cert",
#    ]),
#)

########################################################

cc_library(
  name = "mysql_client",
  srcs = ["mysql_client.cc"],
  hdrs = glob(
             ["*.h"],
             ),
  deps = [
    "//external:gflags",
    "//log:tlog",
  ],
  visibility = ["//visibility:public"],
)

cc_binary(
  name = "mysql_client_test",
  srcs = ["mysql_client_test.cc"],
  deps = [
    ":mysql_client",
  ],
  linkopts = [
    "-lpthread",
    "-lmysqlcppconn8",
  ],
)
