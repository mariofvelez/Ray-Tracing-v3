#pragma once

#define DEBUG_LOG

// debug commands
#ifdef DEBUG_LOG
#define dlog(x) std::cout << x
#define dlogln(x) std::cout << x << std::endl
#define derr(x) std::cerr << x
#define derrln(x) std::cerr << x << std::endl
float start_times[10];
float end_times[10];
#define debug_start(x, y) start_times[y] = x
#define debug_end(x, y) end_times[y] = x
#define debug_time(x) end_times[x] - start_times[x]
#else
#define dlog(x)
#define dlogln(x)
#define derr(x)
#define cerrln(x)
#define debug_start(x, y)
#define debug_end(x, y)
#define debug_time(x) 0
#endif