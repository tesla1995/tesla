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
  name = "register_user_cc_proto",
  deps = [
    ":register_user_proto",
  ],
)

proto_library(
  name = "register_user_proto",
  srcs = [
    "register_user.proto",
  ],
  deps = [
    ":general_response_proto",
  ],
)

proto_library(
  name = "general_response_proto",
  srcs = [
    "general_response.proto",
  ],
)

cc_binary(
  name = "authen_server",
  srcs = ["server.cc"],
  deps = [
    ":register_user_cc_proto",
  ],
  linkopts = [
    "-lgflags",
    "-lbrpc",
    "-ltcmalloc_and_profiler",
  ],
  #data = [
  #  "//test:certs",
  #]
)

cc_binary(
  name = "authen_client",
  srcs = ["client.cc"],
  deps = [
    ":register_user_cc_proto",
  ],
  linkopts = [
    "-lgflags",
    "-lbrpc",
  ],
  #data = [
  #  "//test:certs",
  #]
)
