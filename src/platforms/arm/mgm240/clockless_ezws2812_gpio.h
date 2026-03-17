#pragma once

// IWYU pragma: private

/// @file clockless_ezws2812_gpio.h
/// @brief FastLED ezWS2812 GPIO controller with automatic frequency selection
///
/// This controller automatically selects the optimal timing implementation
/// based on the CPU frequency (F_CPU). It includes both 39MHz and 78MHz
/// optimized implementations and chooses the correct one at compile time.

#include "eorder.h"

// Check if we're on the right platform
#include "platforms/arm/silabs/is_silabs.h"
#if !defined(FL_IS_SILABS)
#error "ezWS2812 GPIO controllers are only available for Silicon Labs MGM240/MG24 platforms"
#endif

// Include both the ezWS2812 GPIO implementations and the DWT-based controller
#include "platforms/arm/mgm240/clockless_ezws2812_39mhz.h"
#include "platforms/arm/mgm240/clockless_ezws2812_78mhz.h"
#include "platforms/arm/mgm240/clockless_arm_mgm240.h"
#include "fl/chipsets/led_timing.h"

extern "C" {
extern uint32_t SystemCoreClock;
}
namespace fl {
/// @brief Auto-selecting ezWS2812 GPIO controller
///
/// This controller template automatically selects the optimal implementation
/// based on the CPU frequency defined by F_CPU:
/// - F_CPU >= 78MHz: Uses 78MHz optimized timing
/// - F_CPU < 78MHz: Uses 39MHz optimized timing (default)
///
/// This provides the best performance without requiring manual selection
/// for the current WS2812-specific implementation.
///
/// @todo FUTURE IMPROVEMENT: When the underlying 39MHz/78MHz controllers
/// are made generic with T1/T2/T3 parameters, this auto-selector could also
/// become generic and support all clockless chipsets with optimal timing
/// selection based on CPU frequency.
///
/// @tparam DATA_PIN GPIO pin number for LED data
/// @tparam RGB_ORDER Color channel ordering (typically GRB for WS2812)
template<u8 DATA_PIN, EOrder RGB_ORDER = GRB>
class ClocklessController_ezWS2812_GPIO_Auto : public CPixelLEDController<RGB_ORDER> {
public:
private:
    ClocklessController_ezWS2812_GPIO_39MHz<DATA_PIN, RGB_ORDER> mImpl39;
    ClocklessController_ezWS2812_GPIO_78MHz<DATA_PIN, RGB_ORDER> mImpl78;
    bool mUse78 = false;

    static uint32_t cpuFreqHz() {
        uint32_t hz = SystemCoreClock;
        if (hz == 0) {
            #ifdef F_CPU
            hz = static_cast<uint32_t>(F_CPU);
            #endif
        }
        return hz;
    }

public:
    /// @brief Constructor
    ClocklessController_ezWS2812_GPIO_Auto() = default;

    /// @brief Initialize the controller
    virtual void init() override {
        mUse78 = (cpuFreqHz() >= 70000000u);
        if (mUse78) {
            mImpl78.init();
        } else {
            mImpl39.init();
        }
    }

    /// @brief Get maximum refresh rate
    virtual u16 getMaxRefreshRate() const override {
        return mUse78 ? mImpl78.getMaxRefreshRate() : mImpl39.getMaxRefreshRate();
    }

    /// @brief Get selected frequency mode for debugging
    const char* getFrequencyMode() const { return mUse78 ? "78MHz" : "39MHz"; }

protected:
    /// @brief Show pixels (used by FastLED internally)
    virtual void showPixels(PixelController<RGB_ORDER>& pixels) override {
        if (mUse78) {
            mImpl78.showPixels(pixels);
        } else {
            mImpl39.showPixels(pixels);
        }
    }
};

/// @brief Main ezWS2812 GPIO controller typedef
///
/// Uses the MGM240 DWT-based clockless controller for accurate timing on M33.
/// This path is more robust against instruction timing and flash wait states
/// than the fixed-NOP implementations.
///
/// Usage:
/// @code
/// FastLED.addLeds<EZWS2812_GPIO, 7, GRB>(leds, NUM_LEDS);
/// @endcode
///
/// @tparam DATA_PIN GPIO pin number for LED data
/// @tparam RGB_ORDER Color channel ordering (typically GRB for WS2812)
template<u8 DATA_PIN, EOrder RGB_ORDER = GRB>
using EZWS2812_GPIO = ClocklessController<DATA_PIN, TIMING_WS2812B_V5, RGB_ORDER>;

/// @brief Direct access to frequency-specific controllers
///
/// These aliases use the MGM240 DWT-based controller for reliable timing.
/// They remain named for compatibility but do not depend on fixed NOP counts.

/// @brief 39MHz optimized GPIO controller (DWT-based)
template<u8 DATA_PIN, EOrder RGB_ORDER = GRB>
using EZWS2812_GPIO_39MHZ = ClocklessController<DATA_PIN, TIMING_WS2812B_V5, RGB_ORDER>;

/// @brief 78MHz optimized GPIO controller (fixed-NOP implementation)
template<u8 DATA_PIN, EOrder RGB_ORDER = GRB>
using EZWS2812_GPIO_78MHZ = ClocklessController_ezWS2812_GPIO_78MHz<DATA_PIN, RGB_ORDER>;
}  // namespace fl
