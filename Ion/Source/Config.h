#pragma once

// --------------------------------------------------------------------------------------------------------
// Tracing
// --------------------------------------------------------------------------------------------------------
// Default: Tracing enabled on Debug configuration
// --------------------------------------------------------------------------------------------------------

/* Forces tracing on each configuration
 * Tracing will be enabled even if ION_NO_TRACING is 1
 * Default: 0 */
#define ION_FORCE_TRACING 0
/* Disables tracing on all configurations
 * Default: 0 */
#define ION_NO_TRACING 0

// --------------------------------------------------------------------------------------------------------
// Assertion
// --------------------------------------------------------------------------------------------------------
// Default: ionassert enabled on Debug, ionassertnd enabled everywhere, ionexcept enabled everywhere
// --------------------------------------------------------------------------------------------------------

/* Enables Debug assertions
 * Debug assertions usually check for user error or unexpected edge cases.
 * Default: 1 */
#define ION_ENABLE_DEBUG_ASSERTS 1
/* Disables all assertions and removes ionexcept reporting
 * Not recommended as it may generate unexpected behavior
 * Default: 0 */
#define ION_FORCE_NO_ASSERTS 0
/* Enables debug break on exceptions, so they can be investigated
 * Default: 0 */
#define ION_BREAK_ON_EXCEPT 0
/* Always shows platform message box on assertions
 * Default: 0 */
#define ION_FORCE_ASSERT_MSGBOX 0
