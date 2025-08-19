#pragma once

// Ensure we build without HarfBuzz/SheenBidi/Freetype on Windows runners while keeping juce_graphics
#ifndef JUCE_USE_HARFBUZZ
#define JUCE_USE_HARFBUZZ 0
#endif

#ifndef JUCE_USE_SHEENBIDI
#define JUCE_USE_SHEENBIDI 0
#endif

#ifndef JUCE_USE_FREETYPE
#define JUCE_USE_FREETYPE 0
#endif

#ifndef JUCE_USE_CORETEXT_LAYOUT
#define JUCE_USE_CORETEXT_LAYOUT 0
#endif

#ifndef JUCE_USE_DIRECTWRITE
#define JUCE_USE_DIRECTWRITE 0
#endif

#ifndef JUCE_USE_WIN32_FONTS
#define JUCE_USE_WIN32_FONTS 1
#endif


