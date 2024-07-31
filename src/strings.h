#if !defined( STRINGS_H )
#define STRINGS_H

#define STR_WIN_WINDOWTITLE                     "Open GL Game"

#define STR_WINERR_HEADER                       "Windows Error"
#define STR_WINERR_PERFORMANCEFREQUENCY         "Failed to query performance frequency."
#define STR_WINERR_TIMERRESOLUTION              "Failed to set timer resolution."
#define STR_WINERR_REGISTERWINDOW               "Failed to register main window class."
#define STR_WINERR_CREATEWINDOW                 "Failed to create main window."
#define STR_WINERR_CLIENTRECT                   "Failed to get window client rect."
#define STR_WINERR_NOPIXELFORMAT                "No OpenGL pixel formats available."
#define STR_WINERR_UNSUITABLEPIXELFORMAT        "No suitable OpenGL pixel format found."
#define STR_WINERR_RENDERINGCONTEXT             "Failed to make OpenGL rendering context current."
#define STR_WINERR_ADJUSTCLIENTRECT             "Failed to adjust the main window client size."
#define STR_WINERR_LOGMESSAGE                   "Windows error: %s"
#define STR_WINERR_APPDIRECTORY                 "Failed to get the app's base directory."
#define STR_WINERR_INITGAME                     "Failed to initialize game."
#define STR_WINERR_CREATELOGFILE                "Failed to open log file."
#define STR_WINERR_WRITELOGFILE                 "Failed to write message to log file."
#define STR_WINERR_INITTHREADS                  "Failed to initialize threads."

#define STR_FILEERR_OPENFILEFAILED              "File error: could not open file: %s"
#define STR_FILEERR_GETFILESIZEFAILED           "File error: could not get file size: %s"

#define STR_BMPERR_MEMORYCORRUPT                "BMP error: bitmap memory is corrupt, image ID %u"
#define STR_BMPERR_INVALIDHEADERTYPE            "BMP error: invalid header type, only BM is allowed, image ID %u"
#define STR_BMPERR_HEADERCORRUPT                "BMP error: header section is corrupt, image ID %u"
#define STR_BMPERR_INVALIDDIBHEADERTYPE         "BMP error: invalid DIB header type, on BITMAPINFOHEADER is allowed, image ID %u"
#define STR_BMPERR_INVALIDCOLORPLANES           "BMP error: invalid number of color planes, image ID %u"
#define STR_BMPERR_INVALIDBPP                   "BMP error: invalid number of bits per pixel, image ID %u"
#define STR_BMPERR_ISCOMPRESSED                 "BMP error: compression is not currently supported, image ID %u"
#define STR_BMPERR_INVALIDIMAGESIZE             "BMP error: invalid image size, image ID %u"
#define STR_BMPERR_INVALIDPALETTECOUNT          "BMP error: invalid number of palette colors, image ID %u"
#define STR_BMPERR_PALETTECORRUPT               "BMP error: palette is corrupt, image ID %u"
#define STR_BMPERR_INVALIDCOLORINDEXING         "BMP error: color indexing is not currently supported, image ID %u"

#define STR_GDFERR_FILECORRUPT                  "Game data file error: file is corrupt: %s"
#define STR_GDFERR_NOCHUNKSFOUNDFORID           "Game data file error: no chunks found for chunk ID: %u"
#define STR_GDFERR_SPRITEBASECORRUPT            "Game data file error: sprite base memory is corrupt, sprite base ID %u"
#define STR_GDFERR_SPRITEBASEIMAGENOTFOUND      "Game data file error: sprite base image ID not found: %u"
#define STR_GDFWARN_UNKNOWNCHUNKID              "Game data file warning: unknown chunk ID encountered: %u"
#define STR_GDFWARN_UNKNOWNBITMAPENTRYID        "Game data file warning: unknown bitmap entry ID encountered: %u"
#define STR_GDFWARN_UNKNOWNFONTENTRYID          "Game data file warning: unknown font entry ID encountered: %u"
#define STR_GDFWARN_UNKNOWNSPRITEBASEID         "Game data file warning: unknown sprite base ID encountered: %u"

#define STR_SPRITEERR_FRAMEDIMENSIONS           "Sprite error: frame dimensions don't match texture, image ID %u"
#define STR_SPRITEERR_MEMORYCORRUPT             "Sprite error: sprite memory is corrupt, sprite base ID %u"

#define STR_FONTERR_MEMORYCORRUPT               "Font error: font memory is corrupt, font ID %u"

#define STR_DIAG_FRAMETARGETMICRO               "Target frame microseconds: %lld"
#define STR_DIAG_FRAMEDURATIONMICRO             "Last frame microseconds: %lld"
#define STR_DIAG_LAGFRAMES                      "Lag frames: %u"
#define STR_DIAG_THREADCOUNT                    "Threads: %u (click for details)"
#define STR_DIAG_THREADJOBSDONE                 "   Jobs done by thread %u: %lld"
#define STR_DIAG_MOUSEPOS                       "Mouse position: %d, %d"

#define STR_BRUSHTEETH                          "(...But don't forget to brush your teeth!)"

#define STR_MENU_KEEPPLAYING                    "Keep Playing"
#define STR_MENU_QUIT                           "Quit"

#endif

