load("//tutil:variables.bzl", "COPTS", "OPTIMIZE", "COVERAGE")

cc_library(
  name = "test_comm",
  srcs = [
    "test_comm.cc",
  ],
  hdrs = [
    "test_comm.h"
  ],
  deps = [
    "//tutil:tutil",
  ],
)

config_setting(
    name = "coverage",
    values = {"define": "coverage=true"},
)

########################################################

# test data
filegroup(
    name = "certs",
    srcs = glob([
        "certs/*.cer",
        "certs/*.pem",
        "certs/*.cert",
    ]),
)

########################################################
cc_test(
  name = "string_view_test",
  srcs = ["string_view_test.cc"],
  deps = [
    "//tutil:tutil",
    "//external:glog",
    "//external:gtest",
  ],
)

cc_test(
  name = "underscore_name_test",
  srcs = ["underscore_name_test.cc"],
  deps = [
    "//tutil:tutil",
    "//external:glog",
    "//external:gtest",
    "//external:gtest_main",
  ],
)

cc_test(
  name = "streambuf_test",
  srcs = ["streambuf_test.cc"],
  deps = [
    "//tutil:tutil",
    "//external:glog",
    "//external:gtest",
  ],
)

cc_test(
  name = "likely_test",
  srcs = ["likely_test.cc"],
  deps = [
    "//tutil:tutil",
    "//external:glog",
  ],
  copts = COPTS,
  linkopts = OPTIMIZE,
)

cc_test(
  name = "string_splitter_test",
  srcs = ["string_splitter_test.cc"],
  deps = [
    "//tutil:tutil",
    "//external:gtest",
    "//external:glog",
  ],
)

cc_test(
  name = "wildcard_matcher_test",
  srcs = ["wildcard_matcher_test.cc"],
  deps = [
    "//tutil:tutil",
    "//external:gtest",
    "//external:glog",
  ],
)

cc_test(
  name = "file_path_test",
  srcs = ["file_path_test.cc"],
  deps = [
    "//tutil:tutil",
    "//external:gtest",
    "//external:glog",
  ],
)

cc_binary(
  name = "move_ctor_test",
  srcs = ["move_ctor_test.cc"],
  deps = [
    "//tutil:tutil",
    "//external:gtest",
    "//external:glog",
  ],
)

cc_test(
  name = "string_split_test",
  srcs = ["string_split_test.cc"],
  deps = [
    "//tutil:tutil",
    "//external:gtest",
    "//external:glog",
  ],
)

cc_test(
  name = "type_traits_test",
  srcs = ["type_traits_test.cc"],
  deps = [
    "//tutil:tutil",
    "//external:gtest",
    "//external:glog",
  ],
)

cc_test(
  name = "compiler_specific_test",
  srcs = ["compiler_specific_test.cc"],
  deps = [
    "//tutil:tutil",
    "//external:gtest",
    "//external:glog",
  ],
)

cc_binary(
  name = "agent_group_test",
  srcs = ["agent_group_test.cc"],
  deps = [
    "//tutil:tutil",
    "//tvar:tvar",
    "//external:gtest",
    "//log:tlog",
  ],
  linkopts = [
    "-lpthread",
  ],
)

cc_binary(
  name = "tlog_test",
  srcs = ["tlog_test.cc"],
  deps = [
    "//log:tlog",
  ],
  linkopts = [
    "-lpthread",
  ],
)

cc_binary(
  name = "devapi_test",
  srcs = ["devapi_test.cc"],
  linkopts = [
    "-lpthread",
    "-lmysqlcppconn8",
  ],
)

cc_test(
  name = "timestamp_test",
  srcs = ["timestamp_test.cc"],
  deps = [
    "//tutil:tutil",
    "//external:gtest",
    "//external:glog",
  ],
)

cc_test(
  name = "duration_test",
  srcs = ["duration_test.cc"],
  deps = [
    "//tutil:tutil",
    "//external:gtest",
    "//external:glog",
  ],
)

cc_test(
  name = "timer_heap_test",
  srcs = ["timer_heap_test.cc"],
  deps = [
    "//tutil:tutil",
    "//external:gtest",
    "//external:glog",
  ],
)

cc_binary(
  name = "lock_test", 
  srcs = ["lock_test.cc"],
  deps = [
    "//tutil:tutil",
    "//tvar:tvar",
    "//wait_free:wait_free",
    "//external:gtest",
    "//log:tlog",
  ],
  linkopts = [
    "-lpthread",
  ],
)

cc_test(
  name = "bounded_queue_test",
  srcs = ["bounded_queue_test.cc"],
  deps = [
    "//tutil:tutil",
    "//external:gtest",
    "//external:glog",
  ],
  copts = COPTS + select({
    ":coverage": COVERAGE,
    "//conditions:default": [],
  })
)

cc_binary(
  name = "adder_test",
  srcs = ["adder_test.cc"],
  deps = [
    "//tutil:tutil",
    "//tvar:tvar",
    "//log:tlog",
    "//external:gtest",
  ],
  linkopts = [
    "-lpthread",
  ],
)

cc_binary(
  name = "digest_test",
  srcs = ["digest_test.cc"],
  deps = [
    "//tutil:tutil",
    "//log:tlog",
    "//crypto:tcrypto",
  ],
  linkopts = [
    "-lpthread",
    "-lcrypto",
    "-lssl",
  ],
)

cc_binary(
  name = "ssl_test",
  srcs = ["ssl_test.cc"],
  deps = [
    "//tutil:tutil",
    "//log:tlog",
    "//crypto:tcrypto",
  ],
  linkopts = [
    "-lpthread",
    "-lcrypto",
    "-lssl",
  ],
  data = [
    "//test:certs",
  ]
)

cc_binary(
  name = "singleton_test",
  srcs = ["singleton_test.cc"],
  deps = [
    "//tutil:tutil",
    "//external:gtest",
  ],
  linkopts = [
    "-lpthread",
    "-lbrpc",
  ],
)

cc_binary(
  name = "extension_test",
  srcs = ["extension_test.cc"],
  deps = [
    "//tutil:tutil",
  ],
)

cc_binary(
  name = "doubly_buffered_data_test",
  srcs = ["doubly_buffered_data_test.cc"],
  deps = [
    "//tutil:tutil",
    "//log:tlog",
    "//external:gflags",
  ],
  copts = [
    "-DBRPC_ENABLE_CPU_PROFILER",
  ],
  linkopts = [
    "-ltcmalloc_and_profiler",
    "-lpthread",
    "-lbrpc",
  ],
)

cc_binary(
  name = "rw_lock_data_test",
  srcs = ["rw_lock_data_test.cc"],
  deps = [
    "//tutil:tutil",
    "//log:tlog",
    "//external:gflags",
  ],
  copts = [
    "-DBRPC_ENABLE_CPU_PROFILER",
  ],
  linkopts = [
    "-ltcmalloc_and_profiler",
    "-lpthread",
    "-lbrpc",
  ],
)

cc_binary(
  name = "mutex_lock_data_test",
  srcs = ["mutex_lock_data_test.cc"],
  deps = [
    "//tutil:tutil",
    "//log:tlog",
    "//external:gflags",
  ],
  copts = [
    "-DBRPC_ENABLE_CPU_PROFILER",
  ],
  linkopts = [
    "-ltcmalloc_and_profiler",
    "-lpthread",
    "-lbrpc",
  ],
)

cc_binary(
  name = "fast_pthread_mutex_test",
  srcs = ["fast_pthread_mutex_test.cc"],
  deps = [
    "//tutil:tutil",
    "//log:tlog",
    "//external:gflags",
  ],
  copts = [
    "-DBRPC_ENABLE_CPU_PROFILER",
  ],
  linkopts = [
    "-ltcmalloc_and_profiler",
    "-lpthread",
    "-lbrpc",
  ],
)

cc_test(
  name = "explicitly_construct_test",
  srcs = ["explicitly_construct_test.cc"],
  deps = [
    "//tutil:tutil",
    "//external:gtest",
  ],
)

cc_test(
  name = "packed_cache_test",
  srcs = ["packed_cache_test.cc"],
  deps = [
    "//tutil:tutil",
    "//external:gtest",
    "//external:glog",
  ],
)

cc_test(
  name = "system_alloc_test",
  srcs = ["system_alloc_test.cc"],
  deps = [
    "//allocator:allocator",
    "//external:gtest",
  ],
)

cc_test(
  name = "object_allocator_test",
  srcs = ["object_allocator_test.cc"],
  deps = [
    "//allocator:allocator",
    "//external:gtest",
    "//tutil:tutil",
  ],
)

cc_binary(
  name = "poxis_mutex_test",
  srcs = ["poxis_mutex_test.cc"],
  copts = [
    "-DBRPC_ENABLE_CPU_PROFILER",
  ],
  linkopts = [
    "-ltcmalloc_and_profiler",
    "-lpthread",
    "-lgflags",
    "-lbrpc",
  ],
)

cc_binary(
  name = "poxis_spinlock_test",
  srcs = ["poxis_spinlock_test.cc"],
  copts = [
    "-DBRPC_ENABLE_CPU_PROFILER",
  ],
  linkopts = [
    "-ltcmalloc_and_profiler",
    "-lpthread",
    "-lgflags",
    "-lbrpc",
  ],
)

cc_test(
  name = "object_pool_test",
  srcs = ["object_pool_test.cc"],
  deps = [
    "//allocator:allocator",
    "//external:gtest",
    "//tutil:tutil",
  ],
  copts = COPTS + OPTIMIZE,
  linkopts = [
    "-lpthread",
    "-ltcmalloc_and_profiler",
  ],
)
