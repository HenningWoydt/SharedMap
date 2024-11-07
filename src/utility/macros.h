#ifndef SHAREDMAP_MACROS_H
#define SHAREDMAP_MACROS_H

#include <iostream>
#include <string>

namespace SharedMap {
#ifndef ASSERT_ENABLED
#define ASSERT_ENABLED false
#endif

#if ASSERT_ENABLED
#define ASSERT(condition) if(!(condition)) {std::cerr << "Error in file " << __FILE__ << " in function " << __FUNCTION__ << " at line " << __LINE__ << "!" << std::endl; abort(); } ((void)0)
#else
#define ASSERT(condition) ((void)0)
#endif

#define dump(x) std::cout << (#x) << " is " << (x) << std::endl
}

#endif //SHAREDMAP_MACROS_H
