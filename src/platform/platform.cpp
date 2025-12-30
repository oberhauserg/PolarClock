#include "platform.h"

#ifdef __EMSCRIPTEN__
#include "emscripten_platform.h"
#elif defined(__ANDROID__)
#include "android_platform.h"
#else
#include "desktop_platform.h"
#endif

namespace polarclock {

Platform* Platform::create() {
#ifdef __EMSCRIPTEN__
    return new EmscriptenPlatform();
#elif defined(__ANDROID__)
    return new AndroidPlatform();
#else
    return new DesktopPlatform();
#endif
}

} // namespace polarclock
