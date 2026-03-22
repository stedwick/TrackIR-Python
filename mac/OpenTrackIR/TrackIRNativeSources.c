// Temporary Xcode-side bridge so the macOS app can compile the shared C sources
// without duplicating the protocol or frame logic in Swift.

#include "../../c/src/tir5_protocol.c"
#include "../../c/src/tir5_frame.c"
#include "../../c/src/tir5_device.c"
#include "../../c/src/tir5_session.c"
