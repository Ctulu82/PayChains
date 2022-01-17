/**
 * @brief Example C-based BPF program that prints out the parameters
 * passed to it
 */
#include <paychains_sdk.h>
#include "helper.h"

void helper_function(void) {
  pay_log(__FILE__);
}
