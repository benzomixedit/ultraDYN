#pragma once

// Enable Windows text stack with DirectWrite and HarfBuzz/SheenBidi where needed
#ifndef JUCE_USE_HARFBUZZ
#define JUCE_USE_HARFBUZZ 1
#endif

#ifndef JUCE_USE_SHEENBIDI
#define JUCE_USE_SHEENBIDI 1
#endif

#ifndef JUCE_USE_FREETYPE
#define JUCE_USE_FREETYPE 0
#endif

#ifndef JUCE_USE_CORETEXT_LAYOUT
#define JUCE_USE_CORETEXT_LAYOUT 0
#endif

#ifndef JUCE_USE_DIRECTWRITE
#define JUCE_USE_DIRECTWRITE 1
#endif

#ifndef JUCE_USE_WIN32_FONTS
#define JUCE_USE_WIN32_FONTS 1
#endif


