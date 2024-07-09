#if !defined( STRINGS_H )
#define STRINGS_H

#define STR_WIN_WINDOWTITLE               "Testing"

#define STR_WINERR_HEADER                 "Windows Error"
#define STR_WINERR_PERFORMANCEFREQUENCY   "Failed to query performance frequency."
#define STR_WINERR_TIMERRESOLUTION        "Failed to set timer resolution."
#define STR_WINERR_REGISTERWINDOW         "Failed to register main window class."
#define STR_WINERR_CREATEWINDOW           "Failed to create main window."
#define STR_WINERR_CLIENTRECT             "Failed to get window client rect."
#define STR_WINERR_NOPIXELFORMAT          "No OpenGL pixel formats available."
#define STR_WINERR_UNSUITABLEPIXELFORMAT  "No suitable OpenGL pixel format found."
#define STR_WINERR_RENDERINGCONTEXT       "Failed to make OpenGL rendering context current."
#define STR_WINERR_ADJUSTCLIENTRECT       "Failed to adjust the main window client size."
#define STR_WINERR_LOGMESSAGE             "Windows error: %s"
#define STR_WINERR_APPDIRECTORY           "Failed to get the app's base directory."
#define STR_WINERR_INITGAME               "Failed to initialize game."
#define STR_WINERR_CREATELOGFILE          "Failed to open log file."
#define STR_WINERR_WRITELOGFILE           "Failed to write message to log file."

#define STR_FILEERR_OPENFILEFAILED        "File error: could not open file: %s"
#define STR_FILEERR_GETFILESIZEFAILED     "File error: could not get file size: %s"

#define STR_BMPERR_FILECORRUPT            "BMP error: file is corrupt: %s"
#define STR_BMPERR_INVALIDHEADERTYPE      "BMP error: invalid header type, only BM is allowed: %s"
#define STR_BMPERR_HEADERCORRUPT          "BMP error: header section is corrupt: %s"
#define STR_BMPERR_INVALIDDIBHEADERTYPE   "BMP error: invalid DIB header type, on BITMAPINFOHEADER is allowed: %s"
#define STR_BMPERR_INVALIDCOLORPLANES     "BMP error: invalid number of color planes: %s"
#define STR_BMPERR_INVALIDBPP             "BMP error: invalid number of bits per pixel: %s"
#define STR_BMPERR_ISCOMPRESSED           "BMP error: compression is not currently supported: %s"
#define STR_BMPERR_INVALIDIMAGESIZE       "BMP error: invalid image size: %s"
#define STR_BMPERR_INVALIDPALETTECOUNT    "BMP error: invalid number of palette colors: %s"
#define STR_BMPERR_PALETTECORRUPT         "BMP error: palette is corrupt: %s"
#define STR_BMPERR_INVALIDCOLORINDEXING   "BMP error: color indexing is not currently supported: %s"

#define STR_SPRITEERR_FRAMEDIMENSIONS     "Sprite error: frame dimensions don't match texture"

#endif

