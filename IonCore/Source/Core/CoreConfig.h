#pragma once

#pragma region Tracing

// --------------------------------------------------------------------------------------------------------
// Tracing
// --------------------------------------------------------------------------------------------------------
// Default: Tracing enabled on Debug and Release configuration
// --------------------------------------------------------------------------------------------------------

/**
 * Forces tracing on each configuration.
 * Tracing will be enabled even if ION_NO_TRACING is 1.
 * Default: 0
 */
#define ION_FORCE_TRACING 0
/**
 * Enables tracing on Release mode.
 * Default 1
 */
#define ION_RELEASE_TRACING 1
/**
 * Disables tracing on all configurations.
 * Default: 0
 */
#define ION_NO_TRACING 0
/**
 * Specifies a maximum number of cached results.
 * Once this number is reached the tracer dumps the cached results
 * to file and clears the cache.
 * One result size is sizeof(DebugTracing::TraceResult).
 * Default: 10000
 */
#define ION_TRACE_DUMP_THRESHOLD 10000

#pragma endregion

#pragma region Assertion Macros

// --------------------------------------------------------------------------------------------------------
// Assertion Macros
// --------------------------------------------------------------------------------------------------------

/**
 * Enables Debug assertion macros.
 * Debug assertions usually check for user error or unexpected edge cases.
 * This only affects the ionassert macro.
 * Default: 1
 */
#define ION_ENABLE_DEBUG_ASSERTS 1
/**
 * Enables debug break on error throws, so they can be investigated.
 * This only affects the ionthrow / ionthrowif macros, not the fwdthrow.
 * Default: 0
 */
#define ION_BREAK_ON_THROW 0
/**
 * Always show a platform message box on assertions.
 * Normally, they don't show on debug builds.
 * Default: 0
 */
#define ION_FORCE_ABORT_MSGBOX 0

#pragma endregion
