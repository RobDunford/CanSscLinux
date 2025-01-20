// project/platfrom specific defines
/*Definition of printf (const char *__restrict __fmt, ...) required*/

#include <stdio.h> // included to provide implementation of printf

// Include files for required methods added here so that implementation specific
// paths can be used

#include "can.h"
#include "datastore.h"
#include "tick.h"

#define CAN_SSC_NODE_ID 0