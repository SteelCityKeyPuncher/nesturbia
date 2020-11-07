#include "catch2/catch_all.hpp"

#include "nesturbia/nesturbia.hpp"

TEST_CASE("dummy") { CHECK(nesturbia::nesturbia() == 123); }
