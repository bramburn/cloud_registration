#ifndef PROFILING_MACROS_H
#define PROFILING_MACROS_H

#include "core/performance_profiler.h"

/**
 * @file profiling_macros.h
 * @brief Convenience macros for easy performance profiling instrumentation
 * 
 * This header provides RAII-based macros that make it easy to instrument
 * code with performance profiling. The macros automatically handle the
 * start/end timing calls and ensure proper cleanup even if exceptions occur.
 * 
 * Usage:
 * @code
 * #include "core/profiling_macros.h"
 * 
 * void myFunction() {
 *     PROFILE_FUNCTION(); // Profiles the entire function
 *     
 *     {
 *         PROFILE_SECTION("Database Query");
 *         // Database operations here
 *     } // Timing automatically ends when scope exits
 *     
 *     {
 *         PROFILE_SECTION("Data Processing");
 *         // Processing operations here
 *     }
 * }
 * @endcode
 */

/**
 * @brief Profile a named section using RAII
 * @param name Name of the section to profile (QString or string literal)
 * 
 * This macro creates a scoped timer that automatically starts timing when
 * the macro is called and stops timing when the current scope exits.
 * The timer is automatically cleaned up even if exceptions occur.
 */
#define PROFILE_SECTION(name) PerformanceProfiler::SectionTimer __profiler_timer(name)

/**
 * @brief Profile the current function using RAII
 * 
 * This macro automatically generates a section name based on the current
 * function name, making it easy to profile entire functions without
 * manually specifying names.
 */
#define PROFILE_FUNCTION() PROFILE_SECTION(QString("%1").arg(__FUNCTION__))

/**
 * @brief Profile a named section with detailed context information
 * @param name Custom name for the section
 * 
 * This macro includes file name and function name in the profiling section
 * name, providing more detailed context for debugging and analysis.
 * Use this for more detailed profiling when you need to distinguish
 * between similar operations in different files or functions.
 */
#define PROFILE_SECTION_DETAILED(name) \
    PROFILE_SECTION(QString("%1::%2::%3").arg(__FUNCTION__).arg(__FILE__).arg(name))

/**
 * @brief Conditionally profile a section based on profiler state
 * @param name Name of the section to profile
 * 
 * This macro only creates the profiling timer if profiling is currently
 * enabled, providing zero overhead when profiling is disabled.
 * Note: The PerformanceProfiler already checks enabled state internally,
 * so this is mainly for documentation purposes.
 */
#define PROFILE_SECTION_CONDITIONAL(name) \
    do { \
        if (PerformanceProfiler::instance().isEnabled()) { \
            PROFILE_SECTION(name); \
        } \
    } while(0)

#endif // PROFILING_MACROS_H
