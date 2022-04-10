#pragma once

// --------------------------------------------------------------------------------------------------------
// Tracing
// --------------------------------------------------------------------------------------------------------
// Default: Tracing enabled on Debug and Release configuration
// --------------------------------------------------------------------------------------------------------

/* Forces tracing on each configuration
 * Tracing will be enabled even if ION_NO_TRACING is 1
 * Default: 0 */
#define ION_FORCE_TRACING 0
/* Enables tracing on Release mode 
   Default 1 */
#define ION_RELEASE_TRACING 1
/* Disables tracing on all configurations
 * Default: 0 */
#define ION_NO_TRACING 0
/* Specifies a maximum number of cached results.
 * Once this number is reached the tracer dumps the cached results
 * to file and clears the cache.
 * One result size is sizeof(DebugTracing::TraceResult)
 * Default: 10000 */
#define ION_TRACE_DUMP_THRESHOLD 10000

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

// --------------------------------------------------------------------------------------------------------
// Renderer
// --------------------------------------------------------------------------------------------------------
// Default: Debugging enabled in Debug builds
// --------------------------------------------------------------------------------------------------------

/* Enables shader debugging and disables optimizations in Debug builds
   Default: 1 */
#define ION_ENABLE_SHADER_DEBUG 1
/* Forces shader debugging and disables optimizations on all configurations
   Default: 0 */
#define ION_FORCE_SHADER_DEBUG 0
