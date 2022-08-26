#pragma once

// --------------------------------------------------------------------------------------------------------
// RHI
// --------------------------------------------------------------------------------------------------------

// Default: 0
#define ENABLE_OPENGL_RHI 0
// Default: 1
#define ENABLE_D3D10_RHI 1
// Default: 1
#define ENABLE_D3D11_RHI 1

// --------------------------------------------------------------------------------------------------------
// Renderer
// --------------------------------------------------------------------------------------------------------
// Default: Debugging enabled in Debug builds
// --------------------------------------------------------------------------------------------------------

/**
 * Enables shader debugging and disables optimizations in Debug builds
 * Default: 1
 */
#define ION_ENABLE_SHADER_DEBUG 1
/**
 * Forces shader debugging and disables optimizations on all configurations
 * Default: 0
 */
#define ION_FORCE_SHADER_DEBUG 0
