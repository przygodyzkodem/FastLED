/// @file fastpin_arm_mgm240.h
/// @brief FastPin implementation for Silicon Labs MGM240 ARM Cortex-M33
///
/// This implementation provides hardware-accelerated GPIO operations for the
/// MGM240SD22VNA microcontroller using Silicon Labs EMLIB.
///
/// Key features:
/// - Atomic GPIO operations using DOUTSET/DOUTCLR registers
/// - Race condition-free pin manipulation in interrupt environments
/// - Direct Silicon Labs EMLIB integration for optimal performance
/// - Template-based compile-time optimization

#pragma once

// IWYU pragma: private

#include "fl/stl/stdint.h"

// Include Silicon Labs EMLIB GPIO for direct register access
// IWYU pragma: begin_keep
#include <em_gpio.h>
#include <em_cmu.h>
// IWYU pragma: end_keep

#include "led_sysdefs.h"
#include "fl/stl/compiler_control.h"
#include "fl/system/fastpin_base.h"
namespace fl {
/// Forward declaration of base FastPin template
template<u8 PIN> class FastPin;

/// Initialize GPIO clock (required for Silicon Labs EFM32/EFR32 devices)
void _mgm240_gpio_init();

/// @brief GPIO port accessor structures
/// These provide compile-time port resolution for template-based FastPin operations
struct __generated_struct_GPIO_PORT_A {
    static constexpr GPIO_Port_TypeDef port() { return gpioPortA; }
};

struct __generated_struct_GPIO_PORT_B {
    static constexpr GPIO_Port_TypeDef port() { return gpioPortB; }
};

struct __generated_struct_GPIO_PORT_C {
    static constexpr GPIO_Port_TypeDef port() { return gpioPortC; }
};

struct __generated_struct_GPIO_PORT_D {
    static constexpr GPIO_Port_TypeDef port() { return gpioPortD; }
};

/// @brief Hardware pin template for MGM240 GPIO operations
/// @tparam _MASK Bit mask for the pin within the port (1 << pin_number)
/// @tparam _PORT_STRUCT Port accessor structure (e.g., __generated_struct_GPIO_PORT_A)
/// @tparam _PORT_NUMBER Port number (0=A, 1=B, 2=C, 3=D)
/// @tparam _PIN_NUMBER Pin number within the port (0-7 typically)
template<u32 _MASK, typename _PORT_STRUCT, u8 _PORT_NUMBER, u8 _PIN_NUMBER> class _ARMPIN : public ValidPinBase {
public:
    typedef volatile u32 * port_ptr_t;
    typedef u32 port_t;

    /// Configure pin as push-pull output
    FASTLED_FORCE_INLINE static void setOutput() {
        _mgm240_gpio_init();
        GPIO_PinModeSet(_PORT_STRUCT::port(), _PIN_NUMBER, gpioModePushPull, 0);
    }

    /// Configure pin as input
    FASTLED_FORCE_INLINE static void setInput() {
        _mgm240_gpio_init();
        GPIO_PinModeSet(_PORT_STRUCT::port(), _PIN_NUMBER, gpioModeInput, 0);
    }

    /// Set pin output high
    FASTLED_FORCE_INLINE static void hi() {
        GPIO_PinOutSet(_PORT_STRUCT::port(), _PIN_NUMBER);
    }

    /// Set pin output low
    FASTLED_FORCE_INLINE static void lo() {
        GPIO_PinOutClear(_PORT_STRUCT::port(), _PIN_NUMBER);
    }

    /// Set pin output based on value (non-zero = high, zero = low)
    FASTLED_FORCE_INLINE static void set(port_t val) {
        if(val) hi(); else lo();
    }

    /// Generate a brief pulse by toggling twice
    FASTLED_FORCE_INLINE static void strobe() { toggle(); toggle(); }

    /// Toggle pin output state
    FASTLED_FORCE_INLINE static void toggle() {
        GPIO_PinOutToggle(_PORT_STRUCT::port(), _PIN_NUMBER);
    }

    /// Set pin high (port parameter ignored for compatibility)
    FASTLED_FORCE_INLINE static void hi(port_ptr_t port) { FL_UNUSED(port); hi(); }

    /// Set pin low (port parameter ignored for compatibility)
    FASTLED_FORCE_INLINE static void lo(port_ptr_t port) { FL_UNUSED(port); lo(); }

    /// Get port value with this pin set high
    FASTLED_FORCE_INLINE static port_t hival() {
        return GPIO_PortOutGet(_PORT_STRUCT::port()) | _MASK;
    }

    /// Get port value with this pin set low
    FASTLED_FORCE_INLINE static port_t loval() {
        return GPIO_PortOutGet(_PORT_STRUCT::port()) & ~_MASK;
    }

    /// Get pointer to GPIO port output register (for direct port manipulation)
    FASTLED_FORCE_INLINE static port_ptr_t port() {
        return &(GPIO->P[_PORT_STRUCT::port()].DOUT);
    }

    /// Get pointer to atomic SET register for race-free high operations
    FASTLED_FORCE_INLINE static port_ptr_t sport() {
        return &(GPIO->P[_PORT_STRUCT::port()].DOUTSET);
    }

    /// Get pointer to atomic CLEAR register for race-free low operations
    FASTLED_FORCE_INLINE static port_ptr_t cport() {
        return &(GPIO->P[_PORT_STRUCT::port()].DOUTCLR);
    }

    /// Fast port write operation (used by timing-critical code)
    FASTLED_FORCE_INLINE static void fastset(port_ptr_t port, port_t val) {
        *port = val;
    }

    /// Get the bit mask for this pin within its port
    FASTLED_FORCE_INLINE static port_t mask() { return _MASK; }

    /// Get numeric port index (0=A, 1=B, 2=C, 3=D)
    FASTLED_FORCE_INLINE static u8 port_number() { return _PORT_NUMBER; }

    /// Read pin input state
    FASTLED_FORCE_INLINE static bool isset() {
        return GPIO_PinInGet(_PORT_STRUCT::port(), _PIN_NUMBER) != 0;
    }
};

/// @brief Pin definition macro for MGM240 FastPin specializations
/// Creates a template specialization of FastPin for a specific Arduino pin number
/// @param PIN Arduino pin number (0-25)
/// @param BIT Pin number within the port (0-7)
/// @param PORT_LETTER Port letter (A, B, C, D)
/// @param MASK Bit mask for the pin (1 << BIT)
#define _FL_DEFPIN(PIN, BIT, PORT_LETTER, MASK) template<> class FastPin<PIN> : public _ARMPIN<MASK, __generated_struct_GPIO_PORT_##PORT_LETTER, PORT_NUM_##PORT_LETTER, BIT> {};

// Define port numbers for template parameters
#define PORT_NUM_A 0
#define PORT_NUM_B 1
#define PORT_NUM_C 2
#define PORT_NUM_D 3

#if defined(ARDUINO_BOARD_XIAO_MG24)
// Pin mappings for Seeed Studio XIAO MG24 (Sense)
// Based on Silicon Labs core variant: variants/xiao_mg24/arduino_variant.cpp
_FL_DEFPIN(0,  0, C, (1 << 0));   // D0  - PC0
_FL_DEFPIN(1,  1, C, (1 << 1));   // D1  - PC1
_FL_DEFPIN(2,  2, C, (1 << 2));   // D2  - PC2
_FL_DEFPIN(3,  3, C, (1 << 3));   // D3  - PC3
_FL_DEFPIN(4,  4, C, (1 << 4));   // D4  - PC4
_FL_DEFPIN(5,  5, C, (1 << 5));   // D5  - PC5
_FL_DEFPIN(6,  6, C, (1 << 6));   // D6  - PC6
_FL_DEFPIN(7,  7, C, (1 << 7));   // D7  - PC7
_FL_DEFPIN(8,  3, A, (1 << 3));   // D8  - PA3
_FL_DEFPIN(9,  4, A, (1 << 4));   // D9  - PA4
_FL_DEFPIN(10, 5, A, (1 << 5));   // D10 - PA5
_FL_DEFPIN(11, 9, A, (1 << 9));   // D11 - PA9
_FL_DEFPIN(12, 8, A, (1 << 8));   // D12 - PA8
_FL_DEFPIN(13, 2, B, (1 << 2));   // D13 - PB2
_FL_DEFPIN(14, 3, B, (1 << 3));   // D14 - PB3
_FL_DEFPIN(15, 0, B, (1 << 0));   // D15 - PB0
_FL_DEFPIN(16, 1, B, (1 << 1));   // D16 - PB1
_FL_DEFPIN(17, 0, A, (1 << 0));   // D17 - PA0
_FL_DEFPIN(18, 2, D, (1 << 2));   // D18 - PD2
_FL_DEFPIN(19, 5, D, (1 << 5));   // D19 - PD5
_FL_DEFPIN(20, 7, A, (1 << 7));   // D20 - PA7 (LED)
_FL_DEFPIN(21, 6, A, (1 << 6));   // D21 - PA6
_FL_DEFPIN(22, 8, C, (1 << 8));   // D22 - PC8
_FL_DEFPIN(23, 9, C, (1 << 9));   // D23 - PC9
_FL_DEFPIN(24, 4, D, (1 << 4));   // D24 - PD4
_FL_DEFPIN(25, 3, D, (1 << 3));   // D25 - PD3
_FL_DEFPIN(26, 4, B, (1 << 4));   // D26 - PB4
_FL_DEFPIN(27, 5, B, (1 << 5));   // D27 - PB5
#else
// Pin mappings for Arduino Nano Matter (MGM240SD22VNA)
// Based on Arduino Nano form factor - verify against hardware documentation

// Digital pins 0-13 (Arduino Nano standard layout)
_FL_DEFPIN(0,  0, A, (1 << 0));   // D0/RX  - PA00
_FL_DEFPIN(1,  1, A, (1 << 1));   // D1/TX  - PA01
_FL_DEFPIN(2,  2, A, (1 << 2));   // D2     - PA02
_FL_DEFPIN(3,  3, A, (1 << 3));   // D3/PWM - PA03
_FL_DEFPIN(4,  4, A, (1 << 4));   // D4     - PA04
_FL_DEFPIN(5,  5, A, (1 << 5));   // D5/PWM - PA05
_FL_DEFPIN(6,  6, A, (1 << 6));   // D6/PWM - PA06
_FL_DEFPIN(7,  7, A, (1 << 7));   // D7     - PA07
_FL_DEFPIN(8,  0, B, (1 << 0));   // D8     - PB00
_FL_DEFPIN(9,  1, B, (1 << 1));   // D9/PWM - PB01
_FL_DEFPIN(10, 2, B, (1 << 2));   // D10/SS - PB02
_FL_DEFPIN(11, 3, B, (1 << 3));   // D11/MOSI - PB03
_FL_DEFPIN(12, 4, B, (1 << 4));   // D12/MISO - PB04
_FL_DEFPIN(13, 5, B, (1 << 5));   // D13/SCK/LED - PB05

// Analog pins A0-A7 (mapped to port C)
_FL_DEFPIN(14, 0, C, (1 << 0));   // A0 - PC00
_FL_DEFPIN(15, 1, C, (1 << 1));   // A1 - PC01
_FL_DEFPIN(16, 2, C, (1 << 2));   // A2 - PC02
_FL_DEFPIN(17, 3, C, (1 << 3));   // A3 - PC03
_FL_DEFPIN(18, 4, C, (1 << 4));   // A4/SDA - PC04
_FL_DEFPIN(19, 5, C, (1 << 5));   // A5/SCL - PC05
_FL_DEFPIN(20, 6, C, (1 << 6));   // A6 - PC06
_FL_DEFPIN(21, 7, C, (1 << 7));   // A7 - PC07

// Additional GPIO pins for extended functionality (port D)
_FL_DEFPIN(22, 0, D, (1 << 0));   // D22 - PD00
_FL_DEFPIN(23, 1, D, (1 << 1));   // D23 - PD01
_FL_DEFPIN(24, 2, D, (1 << 2));   // D24 - PD02
_FL_DEFPIN(25, 3, D, (1 << 3));   // D25 - PD03
#endif

#define HAS_HARDWARE_PIN_SUPPORT
}  // namespace fl
