
#ifdef TRACY_ENABLE

#include <tracy/Tracy.hpp>

#define trinex_profile_cpu() ZoneScopedN(__FUNCTION__)
#define trinex_profile_cpu_n(name) ZoneScopedN(name)
#define trinex_profile_transient_cpu() ZoneTransientN(___tracy_transient_zone, __FUNCTION__, true)
#define trinex_profile_transient_cpu_n(name) ZoneTransientN(___tracy_transient_zone, name, true)

#define trinex_profile_frame_mark() FrameMark
#define trinex_profile_frame_mark_n(name) FrameMarkNamed(name)
#define trinex_profile_frame_mark_start(name) FrameMarkStart(name)
#define trinex_profile_frame_mark_end(name) FrameMarkEnd(name)

#else
#define trinex_profile_cpu()
#define trinex_profile_cpu_n(name)
#define trinex_profile_transient_cpu()
#define trinex_profile_transient_cpu_n(name)
#define trinex_profile_frame_mark()
#define trinex_profile_frame_mark_n(name)
#define trinex_profile_frame_mark_start(name)
#define trinex_profile_frame_mark_end(name)
#endif
