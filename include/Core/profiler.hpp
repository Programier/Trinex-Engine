
#ifdef TRACY_ENABLE

#include <tracy/Tracy.hpp>

#define trinex_profile_cpu() ZoneScopedN(__FUNCTION__)
#define trinex_profile_cpu_n(name) ZoneScopedN(name)
#define trinex_profile_frame_mark() FrameMark

#else
#define trinex_profile_cpu()
#define trinex_profile_cpu_n(name)
#define trinex_profile_frame_mark()
#endif
