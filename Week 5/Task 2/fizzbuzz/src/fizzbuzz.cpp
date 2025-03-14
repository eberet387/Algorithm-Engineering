#include <string>

std::string fizzbuzz(int const number)
{
   if (number != 0)
   {
      auto m3 = number % 3;
      auto m5 = number % 5;
      if (!m5 && !m3) { return "fizzbuzz"; }
      else if (!m5) { return "buzz"; }
      else if (!m3) { return "fizz"; }
   }
   return std::to_string(number);
}
