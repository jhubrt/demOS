#define _CRT_SECURE_NO_WARNINGS

#include <algorithm>
#include <assert.h>

#include <windows.h>

#undef MEM_FREE

extern "C"
{
#   include "DEMOSDK/BASTYPES.H"
#   include "DEMOSDK/PC/SURFACE.H"
#   include "DEMOSDK/PC/WINDOW.H"
}

#include "TOOLS/FONT/GLYPH.H"

constexpr COLORREF COLOR_WHITE = RGB(255, 255, 255);
constexpr COLORREF COLOR_BLACK = RGB(0, 0, 0);
constexpr COLORREF COLOR_BLUE  = RGB(0, 0, 255);

struct bitGlyphFont
{
    HFONT    m_font = NULL;
    LOGFONT  m_logFont;

    bitGlyphFont(char* _name, int _width, int _height, int _weight, int _italic)
    {
        memset(&m_logFont, 0, sizeof(m_logFont));

        strcpy(m_logFont.lfFaceName, _name); 
        m_logFont.lfWidth  = _width;
        m_logFont.lfHeight = _height;
        m_logFont.lfWeight = _weight;        
        m_logFont.lfItalic = _italic;        
        m_logFont.lfQuality = PROOF_QUALITY | NONANTIALIASED_QUALITY;

        m_font = CreateFontIndirect(&m_logFont);
    }
};


class bitGlyphDC
{
    HDC      m_dc;
    HBITMAP  m_bitmap;
    HRGN     m_rgn;

    BITsurface* m_surface;

    int m_bitcount;

public:

    bitGlyphDC(BITsurface* _surface)
        : m_surface(_surface)
    {
        ASSERT(_surface->format == BITformat_x888);

        m_bitcount = 32;
        m_bitmap = CreateBitmap(_surface->width, _surface->height, 1, m_bitcount, _surface->buffer);
        m_dc = CreateCompatibleDC(NULL);
        m_rgn = CreateRectRgn(0, 0, _surface->width, _surface->height);

        SelectObject(m_dc, m_bitmap);
        SelectClipRgn(m_dc, m_rgn);
    }

    void SetFont(bitGlyphFont* _font)
    {
        SelectObject(m_dc, _font->m_font);
    }

    void OutText(int _x, int _y, char* _text, bool _outline, bool _shadow)
    {
        auto lentext = strlen(_text);

        SetTextAlign(m_dc, TA_LEFT | TA_TOP | TA_UPDATECP);
        SetMapMode(m_dc, MM_TEXT);
        SetBkMode(m_dc, TRANSPARENT);
        SetBkColor(m_dc, COLOR_BLUE);


        // Determine padding for outline and shadow effects
        int left_padding = (_outline ? 1 : 0);
        int right_padding = (_outline ? (_shadow ? 2 : 1) : (_shadow ? 2 : 0));
        int top_padding = (_outline ? 1 : 0);
        int bottom_padding = (_outline ? (_shadow ? 2 : 1) : (_shadow ? 2 : 0));


        if (_outline)
        {
            SetTextColor(m_dc, COLOR_BLACK);
            MoveToEx(m_dc, _x + 0, _y + 0, NULL); ExtTextOut(m_dc, 0, 0, ETO_OPAQUE, NULL, _text, lentext, NULL);
            MoveToEx(m_dc, _x + 1, _y + 0, NULL); ExtTextOut(m_dc, 0, 0, ETO_OPAQUE, NULL, _text, lentext, NULL);
            MoveToEx(m_dc, _x + 2, _y + 0, NULL); ExtTextOut(m_dc, 0, 0, ETO_OPAQUE, NULL, _text, lentext, NULL);
            MoveToEx(m_dc, _x + 0, _y + 1, NULL); ExtTextOut(m_dc, 0, 0, ETO_OPAQUE, NULL, _text, lentext, NULL);
            MoveToEx(m_dc, _x + 1, _y + 1, NULL); ExtTextOut(m_dc, 0, 0, ETO_OPAQUE, NULL, _text, lentext, NULL);
            MoveToEx(m_dc, _x + 2, _y + 1, NULL); ExtTextOut(m_dc, 0, 0, ETO_OPAQUE, NULL, _text, lentext, NULL);
            MoveToEx(m_dc, _x + 0, _y + 2, NULL); ExtTextOut(m_dc, 0, 0, ETO_OPAQUE, NULL, _text, lentext, NULL);
            MoveToEx(m_dc, _x + 1, _y + 2, NULL); ExtTextOut(m_dc, 0, 0, ETO_OPAQUE, NULL, _text, lentext, NULL);
            MoveToEx(m_dc, _x + 2, _y + 2, NULL); ExtTextOut(m_dc, 0, 0, ETO_OPAQUE, NULL, _text, lentext, NULL);

            if (_shadow)
            {
                MoveToEx(m_dc, _x + 3, _y + 3, NULL); ExtTextOut(m_dc, 0, 0, ETO_OPAQUE, NULL, _text, lentext, NULL);
            }

            SetTextColor(m_dc, COLOR_WHITE);
            MoveToEx(m_dc, _x + 1, _y + 1, NULL); ExtTextOut(m_dc, _x, _y, ETO_OPAQUE, NULL, _text, lentext, NULL);
        }
        else
        {
            if (_shadow)
            {
                SetTextColor(m_dc, COLOR_BLACK);
                MoveToEx(m_dc, _x + 2, _y + 2, NULL); ExtTextOut(m_dc, 0, 0, ETO_OPAQUE, NULL, _text, lentext, NULL);
            }

            SetTextColor(m_dc, COLOR_WHITE);
            MoveToEx(m_dc, _x, _y, NULL); ExtTextOut(m_dc, _x, _y, ETO_OPAQUE, NULL, _text, lentext, NULL);
        }
    }

    void UpdateSurface()
    {
        GetBitmapBits(m_bitmap, m_surface->width * m_surface->height * m_bitcount / 8, m_surface->buffer);
    }

    ~bitGlyphDC()
    {
        DeleteObject(m_dc);
        DeleteObject(m_bitmap);
        DeleteObject(m_rgn);
    }

};


BITfont     BITglyphCreateFont      (char* _name, int _w, int _h, int _weight, int _italic) { return new bitGlyphFont(_name, _w, _h, _weight, _italic);  }
void        BITglyphDestroyFont     (BITfont _font)                                         { delete static_cast<bitGlyphFont*>(_font); }

BITglyphDC  BITglyphCreateDC        (BITsurface* _surface)                                  { return new bitGlyphDC(_surface); }
void        BITglyphDestroyDC       (BITglyphDC _dc)                                        { delete static_cast<bitGlyphDC*>(_dc); }
void        BITglyphSetFont         (BITglyphDC _dc, BITfont _font)                         { static_cast<bitGlyphDC*>(_dc)->SetFont( static_cast<bitGlyphFont*>(_font) ); }
void        BITglyphUpdate          (BITglyphDC _dc)                                        { static_cast<bitGlyphDC*>(_dc)->UpdateSurface(); }
void        BITglyphText            (BITglyphDC _dc, int _sx, int _sy, char* _text, bool _outline, bool _shadow) { static_cast<bitGlyphDC*>(_dc)->OutText(_sx, _sy, _text, _outline, _shadow); }
