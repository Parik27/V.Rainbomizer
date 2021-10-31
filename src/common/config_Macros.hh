#define RB_C__NUM_ARGS(X100, X99, X98, X97, X96, X95, X94, X93, X92, X91, X90, \
                       X89, X88, X87, X86, X85, X84, X83, X82, X81, X80, X79,  \
                       X78, X77, X76, X75, X74, X73, X72, X71, X70, X69, X68,  \
                       X67, X66, X65, X64, X63, X62, X61, X60, X59, X58, X57,  \
                       X56, X55, X54, X53, X52, X51, X50, X49, X48, X47, X46,  \
                       X45, X44, X43, X42, X41, X40, X39, X38, X37, X36, X35,  \
                       X34, X33, X32, X31, X30, X29, X28, X27, X26, X25, X24,  \
                       X23, X22, X21, X20, X19, X18, X17, X16, X15, X14, X13,  \
                       X12, X11, X10, X9, X8, X7, X6, X5, X4, X3, X2, X1, N,   \
                       ...)                                                    \
    N
#define RB_C_NUM_ARGS(...)                                                     \
    RB_C__NUM_ARGS (__VA_ARGS__, 100, 99, 98, 97, 96, 95, 94, 93, 92, 91, 90,  \
                    89, 88, 87, 86, 85, 84, 83, 82, 81, 80, 79, 78, 77, 76,    \
                    75, 74, 73, 72, 71, 70, 69, 68, 67, 66, 65, 64, 63, 62,    \
                    61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48,    \
                    47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34,    \
                    33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20,    \
                    19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4,  \
                    3, 2, 1)

#define RB_C_EXPAND(X) X
#define RB_C_FIRSTARG(X, ...) (X)
#define RB_C_RESTARGS(X, ...) (__VA_ARGS__)
#define RB_C_FOREACH(MACRO, LIST)                                              \
    RB_C_FOREACH_ (RB_C_NUM_ARGS LIST, MACRO, LIST)
#define RB_C_FOREACH_(N, M, LIST) RB_C_FOREACH__ (N, M, LIST)
#define RB_C_FOREACH__(N, M, LIST) RB_C_FOREACH_##N (M, LIST)

// clang-format off
#define RB_C_FOREACH_1(M, LIST) M LIST
#define RB_C_FOREACH_2(M, LIST) RB_C_EXPAND (M RB_C_FIRSTARG LIST), RB_C_FOREACH_1 (M, RB_C_RESTARGS LIST)
#define RB_C_FOREACH_3(M, LIST) RB_C_EXPAND (M RB_C_FIRSTARG LIST), RB_C_FOREACH_2 (M, RB_C_RESTARGS LIST)
#define RB_C_FOREACH_4(M, LIST) RB_C_EXPAND (M RB_C_FIRSTARG LIST), RB_C_FOREACH_3 (M, RB_C_RESTARGS LIST)
#define RB_C_FOREACH_5(M, LIST) RB_C_EXPAND (M RB_C_FIRSTARG LIST), RB_C_FOREACH_4 (M, RB_C_RESTARGS LIST)
#define RB_C_FOREACH_6(M, LIST) RB_C_EXPAND (M RB_C_FIRSTARG LIST), RB_C_FOREACH_5 (M, RB_C_RESTARGS LIST)
#define RB_C_FOREACH_7(M, LIST) RB_C_EXPAND (M RB_C_FIRSTARG LIST), RB_C_FOREACH_6 (M, RB_C_RESTARGS LIST)
#define RB_C_FOREACH_8(M, LIST) RB_C_EXPAND (M RB_C_FIRSTARG LIST), RB_C_FOREACH_7 (M, RB_C_RESTARGS LIST)
#define RB_C_FOREACH_9(M, LIST) RB_C_EXPAND (M RB_C_FIRSTARG LIST), RB_C_FOREACH_8 (M, RB_C_RESTARGS LIST)
#define RB_C_FOREACH_10(M, LIST) RB_C_EXPAND (M RB_C_FIRSTARG LIST), RB_C_FOREACH_9 (M, RB_C_RESTARGS LIST)
#define RB_C_FOREACH_11(M, LIST) RB_C_EXPAND (M RB_C_FIRSTARG LIST), RB_C_FOREACH_10 (M, RB_C_RESTARGS LIST)
#define RB_C_FOREACH_12(M, LIST) RB_C_EXPAND (M RB_C_FIRSTARG LIST), RB_C_FOREACH_11 (M, RB_C_RESTARGS LIST)
#define RB_C_FOREACH_13(M, LIST) RB_C_EXPAND (M RB_C_FIRSTARG LIST), RB_C_FOREACH_12 (M, RB_C_RESTARGS LIST)
#define RB_C_FOREACH_14(M, LIST) RB_C_EXPAND (M RB_C_FIRSTARG LIST), RB_C_FOREACH_13 (M, RB_C_RESTARGS LIST)
#define RB_C_FOREACH_15(M, LIST) RB_C_EXPAND (M RB_C_FIRSTARG LIST), RB_C_FOREACH_14 (M, RB_C_RESTARGS LIST)

// clang-format on

#define RB_C_OPTION(Option) std::make_pair (#Option, &Config ().Option)

#define RB_C_DO_CONFIG(Name, ...)                                              \
    if (!ConfigManager::ReadConfig (Name, RB_C_FOREACH (RB_C_OPTION,           \
                                                        (__VA_ARGS__))))       \
        return;

#define RB_C_DO_CONFIG_NO_OPTIONS(Name)                                        \
    if (!ConfigManager::ReadConfig (Name)     \
        return;

#define RB_C_CONFIG_START                                                      \
    static auto &Config ()                                                     \
    {                                                                          \
        static struct Config

#define RB_C_CONFIG_END                                                        \
    s_Config;                                                                  \
    return s_Config;                                                           \
    }
