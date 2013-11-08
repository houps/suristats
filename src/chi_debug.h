#ifndef __CHI_DEBUG_H__
#   define __CHI_DEBUG_H__

/*****************/
/* Useful Macros */
/*****************/

#   define UNUSED_PARAMETER(_param_)   (void)(_param_)

#   ifdef DEBUG
#       define DEBUG_TEST 1
#   else
#       define DEBUG_TEST 0
#   endif

#   define debug_print(fmt, ...) \
               do { if (DEBUG_TEST) fprintf(stderr, fmt, __VA_ARGS__); } while (0)

#endif
