#include "catch.hpp"
#include "fizzbuzz.h"

TEST_CASE("Test positives", "[classic]")
{
   SECTION("Test all up to 10") {
      REQUIRE(fizzbuzz(1) == "1"); 
      REQUIRE(fizzbuzz(2) == "2");
      REQUIRE(fizzbuzz(3) == "fizz");
      REQUIRE(fizzbuzz(4) == "4");
      REQUIRE(fizzbuzz(5) == "buzz");
      REQUIRE(fizzbuzz(6) == "fizz");
      REQUIRE(fizzbuzz(7) == "7");
      REQUIRE(fizzbuzz(8) == "8");
      REQUIRE(fizzbuzz(9) == "fizz");
      REQUIRE(fizzbuzz(10) == "buzz");
   }

   SECTION("Test all multiples of 3 only up to 100") {
      for (int i = 3; i <= 100; i+=3) {
         if (i % 5) REQUIRE(fizzbuzz(i) == "fizz");
      }
   }

   SECTION("Test all multiples of 5 only up to 100") {
      for (int i = 5; i <= 100; i += 5) {
         if (i % 3) REQUIRE(fizzbuzz(i) == "buzz");
      }
   }

   SECTION("Test all multiples of 3 and 5 up to 100") {
      for (int i = 15; i <= 100; i += 15) {
         REQUIRE(fizzbuzz(i) == "fizzbuzz");
      }
   }
}