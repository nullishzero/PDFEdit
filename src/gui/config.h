/** @file
 * File with common configuration for entire program
 * \brief Configuration header
 */
#ifndef __CONFIG_H__
#define __CONFIG_H__

#ifndef CONFIG_DATA_PATH
#error CONFIG_DATA_PATH not defined
#else

/* Ugly macros to force compiler to transform CONFIG_DATA_PATH
 * given by command line define to the string.
 * Note that we can't do
 * #define DATA_PATH S_DATA_PATH(CONFIG_DATA_PATH)
 * directly, because # operator doesn't expand parameter so we
 * would get "CONFIG_DATA_PATH" as the result. So one more level is necessary
 * to force expansion (this is done by CONFIG_PATH.
 *
 * FIXME: consolidate this code - don't use macros if possible
 */
#define S_DATA_PATH(path) #path
#define EXPAND_CONFIG_PATH(path) S_DATA_PATH(path)

/** Directory where all data files of pdfedit would be stored
 *  (icons, default config, etc ...).
 *  Value is set from CONFIG_DATA_PATH configuration value.
 */
#define DATA_PATH EXPAND_CONFIG_PATH(CONFIG_DATA_PATH)

#endif

/** config directory name (will be used relative to $HOME) */
#define CONFIG_DIR ".pdfedit"

#endif

