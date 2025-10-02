#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "ConfigParserTest.cpp"
#include "RequestTest.cpp"

volatile sig_atomic_t g_running = 1;
