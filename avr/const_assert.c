// Ralph Doncaster 2020 public domain software
// compile-time argument checks

#include <avr/io.h>

int LED = 1;

int err;

void badArgument(const char*) __attribute((error("")));

#define CHECK_PIN_ARG(condition) \
    if (!(condition)) badArgument("")

#define ASSERT_CONST(pin)               \
    if (!__builtin_constant_p(pin))     \
        badArgument("pin must be a constant");

int main()
{
    ASSERT_CONSTANT(LED);
    CHECK_PIN_ARG(LED < 10);
    PINB = LED; 
    return err;
}

