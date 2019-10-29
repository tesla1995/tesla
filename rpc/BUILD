load("//tutil:variables.bzl", "COPTS", "OPTIMIZE", "COVERAGE")

config_setting(
    name = "coverage",
    values = {"define": "coverage=true"},
)

########################################################

# test data
#filegroup(
#    name = "certs",
#    srcs = glob([
#        "certs/*.cer",
#        "certs/*.pem",
#        "certs/*.cert",
#    ]),
#)

########################################################
cc_proto_library(
  name = "echo_cc_proto",
  deps = [
    ":echo_proto",
  ],
)

proto_library(
  name = "echo_proto",
  srcs = [
    "echo.proto",
  ],
)

cc_binary(
  name = "echo_client",
  srcs = ["client.cc"],
  deps = [
    ":echo_cc_proto",
  ],
  linkopts = [
    "-lgflags",
    "-lbrpc",
  ],
  #data = [
  #  "//test:certs",
  #]
)

cc_binary(
  name = "echo_server",
  srcs = ["server.cc"],
  deps = [
    ":echo_cc_proto",
  ],
  linkopts = [
    "-lgflags",
    "-lbrpc",
  ],
  #data = [
  #  "//test:certs",
  #]
)
