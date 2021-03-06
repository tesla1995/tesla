COPTS = [
    "-Wall",
    "-Wextra",
    # disable some warnings enabled by Wextra
    "-Wno-unused-but-set-variable",
    "-Wno-unused-parameter",
    "-Wno-unused-local-typedefs",
    "-Wno-missing-field-initializers",
    # other useful warnings
    "-Wendif-labels",
    "-Wfloat-equal",
    "-Wformat=2",
    #"-Wframe-larger-than=69632", # A 64k buffer and other small vars
    #"-Wmissing-include-dirs",
    "-Wpointer-arith",
    "-Wwrite-strings",
    # error flags
    "-Werror=char-subscripts",
    "-Werror=comments",
    "-Werror=conversion-null",
    "-Werror=empty-body",
    "-Werror=endif-labels",
    "-Werror=format",
    #"-Werror=format-nonliteral",
    #"-Werror=missing-include-dirs",
    "-Werror=overflow",
    "-Werror=parentheses",
    "-Werror=reorder",
    "-Werror=return-type",
    "-Werror=sequence-point",
    "-Werror=sign-compare",
    "-Werror=switch",
    "-Werror=type-limits",
    "-Werror=uninitialized",
    # Masked it at first
    # "-Werror=unused-function",
    "-Werror=unused-label",
    "-Werror=unused-result",
    "-Werror=unused-value",
    "-Werror=unused-variable",
    "-Werror=write-strings",
    # Machine
    #"-march=native",
    #"-mcx16",
    # C++ only warning flags
    "-Wno-invalid-offsetof",
    "-Wnon-virtual-dtor",
    "-Woverloaded-virtual",
    "-Wvla",
    "-Werror=non-virtual-dtor",
    "-Werror=overloaded-virtual",
    "-Werror=vla",
]

OPTIMIZE = [
    "-O2",
    "-DNDEBUG",
]

COVERAGE = [
    "-fprofile-arcs",
    "-ftest-coverage",
]
