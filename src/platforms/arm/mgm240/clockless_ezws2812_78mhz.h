#pragma once

// IWYU pragma: private

/// @file clockless_ezws2812_78mhz.h
/// @brief FastLED ezWS2812 GPIO controller optimized for 78MHz Silicon Labs MGM240/MG24
///
/// This controller provides cycle-accurate WS2812 timing using direct GPIO manipulation
/// specifically optimized for 78MHz CPU frequency. All timing calculations are
/// pre-computed to avoid runtime overhead.

#include "controller.h"
#include "pixel_controller.h"
#include "eorder.h"
#include "fl/stl/compiler_control.h"
#include "fl/system/fastpin.h"

// Check if we're on the right platform
#include "platforms/arm/silabs/is_silabs.h"
#if !defined(FL_IS_SILABS)
#error "ezWS2812 GPIO 78MHz controller is only available for Silicon Labs MGM240/MG24 platforms"
#endif
namespace fl {
/// @brief ezWS2812 GPIO controller optimized for 78MHz CPUs
///
/// This controller uses direct GPIO manipulation with pre-computed timing
/// optimized specifically for 78MHz CPU frequency. It processes entire
/// byte arrays in tight loops for maximum performance.
///
/// Current implementation: WS2812-specific timing
/// - '0' bit: 0.4µs high, 0.85µs low (1.25µs total)
/// - '1' bit: 0.8µs high, 0.45µs low (1.25µs total)
///
/// At 78MHz: 1 cycle = ~12.82ns
/// - '0' high: ~31.2 cycles, '0' low: ~66.3 cycles
/// - '1' high: ~62.4 cycles, '1' low: ~35.1 cycles
///
/// @todo FUTURE IMPROVEMENT: Make this controller generic with T1/T2/T3 timing parameters
/// This would allow it to support all clockless LED chipsets (SK6812, TM1809, UCS1903, etc.)
/// instead of being WS2812-specific. The generic ClocklessController template parameters
/// could be used: T1 (high time for '1'), T2 (high time for '0'), T3 (low time for both).
/// Pre-computed cycle counts would be calculated at compile-time from T1/T2/T3 values.
/// Example: template<uint8_t DATA_PIN, int T1, int T2, int T3, EOrder RGB_ORDER = GRB>
///
/// @tparam DATA_PIN GPIO pin number for LED data
/// @tparam RGB_ORDER Color channel ordering (typically GRB for WS2812)
template<u8 DATA_PIN, EOrder RGB_ORDER = GRB>
class ClocklessController_ezWS2812_GPIO_78MHz : public CPixelLEDController<RGB_ORDER> {

    /// @note IMPLEMENTATION ROADMAP for generic T1/T2/T3 support:
    /// 1. Add template parameters: template<DATA_PIN, T1, T2, T3, RGB_ORDER>
    /// 2. Calculate cycles at compile-time:
    ///    - constexpr int T1_CYCLES = (T1 * F_CPU) / 1000000000; // T1 in ns
    ///    - constexpr int T2_CYCLES = (T2 * F_CPU) / 1000000000; // T2 in ns
    ///    - constexpr int T3_CYCLES = (T3 * F_CPU) / 1000000000; // T3 in ns
    /// 3. Generate NOP sequences dynamically based on calculated cycles
    /// 4. Replace hardcoded send1()/send0() with parameterized versions
    /// This would enable support for SK6812 (T1=300ns, T2=300ns, T3=600ns),
    /// TM1809 (T1=350ns, T2=350ns, T3=450ns), and all other clockless chipsets.

private:
    u16 mNumLeds;
    typename FastPin<DATA_PIN>::port_ptr_t mPort;
    typename FastPin<DATA_PIN>::port_t mPinMask;
    typename FastPin<DATA_PIN>::port_t mHi;
    typename FastPin<DATA_PIN>::port_t mLo;

    /// @brief Send '1' bit - optimized for 78MHz
    /// Slightly shorter high pulse to reduce false '1' reads on MG24.
    /// 0.75µs high (~58 cycles), 0.50µs low (~39 cycles)
    FASTLED_FORCE_INLINE void send1() const {
        *mPort = mHi;
        asm volatile(
            "nop; nop; nop; nop; nop; nop; nop; nop;"  // 8
            "nop; nop; nop; nop; nop; nop; nop; nop;"  // 16
            "nop; nop; nop; nop; nop; nop; nop; nop;"  // 24
            "nop; nop; nop; nop; nop; nop; nop; nop;"  // 32
            "nop; nop; nop; nop; nop; nop; nop; nop;"  // 40
            "nop; nop; nop; nop; nop; nop; nop; nop;"  // 48
            "nop; nop; nop; nop; nop; nop;"            // 54
            "nop; nop; nop; nop;"                      // 58
        );
        *mPort = mLo;
        asm volatile(
            "nop; nop; nop; nop; nop; nop; nop; nop;"  // 8
            "nop; nop; nop; nop; nop; nop; nop; nop;"  // 16
            "nop; nop; nop; nop; nop; nop; nop; nop;"  // 24
            "nop; nop; nop; nop; nop; nop; nop; nop;"  // 32
            "nop; nop; nop; nop; nop; nop; nop;"       // 39
        );
    }

    /// @brief Send '0' bit - optimized for 78MHz
    /// Slightly shorter high pulse to reduce false '1' reads on MG24.
    /// 0.35µs high (~27 cycles), 0.90µs low (~70 cycles)
    FASTLED_FORCE_INLINE void send0() const {
        *mPort = mHi;
        asm volatile(
            "nop; nop; nop; nop; nop; nop; nop; nop;"  // 8
            "nop; nop; nop; nop; nop; nop; nop; nop;"  // 16
            "nop; nop; nop; nop; nop; nop; nop; nop;"  // 24
            "nop; nop; nop;"                           // 27
        );
        *mPort = mLo;
        asm volatile(
            "nop; nop; nop; nop; nop; nop; nop; nop;"  // 8
            "nop; nop; nop; nop; nop; nop; nop; nop;"  // 16
            "nop; nop; nop; nop; nop; nop; nop; nop;"  // 24
            "nop; nop; nop; nop; nop; nop; nop; nop;"  // 32
            "nop; nop; nop; nop; nop; nop; nop; nop;"  // 40
            "nop; nop; nop; nop; nop; nop; nop; nop;"  // 48
            "nop; nop; nop; nop; nop; nop; nop; nop;"  // 56
            "nop; nop; nop; nop; nop; nop; nop; nop;"  // 64
            "nop; nop; nop; nop; nop; nop;"            // 70
        );
    }

    /// @brief Send byte with MSB first - optimized tight loop
    /// @param byte_value 8-bit value to send
    FASTLED_FORCE_INLINE void sendByte(u8 byte_value) const {
        // Unrolled loop for maximum performance
        if (byte_value & 0x80) send1(); else send0(); // bit 7
        if (byte_value & 0x40) send1(); else send0(); // bit 6
        if (byte_value & 0x20) send1(); else send0(); // bit 5
        if (byte_value & 0x10) send1(); else send0(); // bit 4
        if (byte_value & 0x08) send1(); else send0(); // bit 3
        if (byte_value & 0x04) send1(); else send0(); // bit 2
        if (byte_value & 0x02) send1(); else send0(); // bit 1
        if (byte_value & 0x01) send1(); else send0(); // bit 0
    }

    /// @brief Send pixel data in output order
    /// @param c0 Byte 0 in RGB_ORDER
    /// @param c1 Byte 1 in RGB_ORDER
    /// @param c2 Byte 2 in RGB_ORDER
    FASTLED_FORCE_INLINE void sendPixel(u8 c0, u8 c1, u8 c2) const {
        sendByte(c0);
        sendByte(c1);
        sendByte(c2);
    }

public:
    /// @brief Constructor
    ClocklessController_ezWS2812_GPIO_78MHz() : mNumLeds(0) {}

    /// @brief Initialize the controller
    virtual void init() override {
        FastPin<DATA_PIN>::setOutput();
        mPinMask = FastPin<DATA_PIN>::mask();
        mPort = FastPin<DATA_PIN>::port();
        mHi = *mPort | mPinMask;
        mLo = *mPort & ~mPinMask;
        *mPort = mLo;
        // Keep default slew rate (do not override port CTRL here)
    }

    /// @brief Get maximum refresh rate
    virtual u16 getMaxRefreshRate() const override {
        return 500; // Slightly higher rate for faster CPU
    }

public:
    /// @brief Output pixels to LED strip - optimized for bulk processing
    /// @param pixels FastLED pixel controller with RGB data
    virtual void showPixels(PixelController<RGB_ORDER>& pixels) override {
        // Force line low before the reset/latch period
        *mPort = *mPort & ~mPinMask;
        // Ensure a clean reset/latch before sending a new frame
        delayMicroseconds(300);

        // Disable all interrupts for precise timing - critical for WS2812
        __disable_irq();

        // Deterministic low gap before first bit (>= 50us)
        {
            // 100us at 78MHz ~= 7800 cycles
            constexpr u32 kResetCycles = (78000000UL / 1000000UL) * 100UL;
            for (u32 i = 0; i < kResetCycles; i++) {
                asm volatile("nop");
            }
        }

        // Prepare cached port values for this frame (with interrupts off)
        mHi = *mPort | mPinMask;
        mLo = *mPort & ~mPinMask;
        *mPort = mLo;

        // Prime dithering for the first byte
        pixels.preStepFirstByteDithering();

        // Process all pixels in tight loop
        while (pixels.has(1)) {
            u8 c0 = pixels.loadAndScale0();
            u8 c1 = pixels.loadAndScale1();
            u8 c2 = pixels.loadAndScale2();

            sendPixel(c0, c1, c2);

            pixels.advanceData();
            pixels.stepDithering();
        }

        // Re-enable interrupts
        __enable_irq();

        // WS2812 reset/latch time (>50µs low)
        delayMicroseconds(300);
    }
};

/// @brief Convenient typedef for 78MHz MGM240/MG24 GPIO controller
/// @tparam DATA_PIN GPIO pin number for LED data
/// @tparam RGB_ORDER Color channel ordering (typically GRB for WS2812)
template<u8 DATA_PIN, EOrder RGB_ORDER = GRB>
using EZWS2812_GPIO_78MHz = ClocklessController_ezWS2812_GPIO_78MHz<DATA_PIN, RGB_ORDER>;
}  // namespace fl
