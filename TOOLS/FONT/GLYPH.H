#ifndef BITGLYPH_H
#define BITGLYPH_H 1

typedef void* BITfont;
typedef void* BITglyphDC;

BITfont     BITglyphCreateFont      (char* _name = "Arial", int _width = 0, int _height = -(16 * 96) / 72 /* 16-pt for default*/, int _weight = 400 /* 400 = normal, 700 = bold */, int _italic = 0 /* 0 = normal, 255 = italic */);
void        BITglyphDestroyFont     (BITfont _font);

BITglyphDC  BITglyphCreateDC        (BITsurface* _surface);
void        BITglyphDestroyDC       (BITglyphDC _dc);
void        BITglyphSetFont         (BITglyphDC _dc, BITfont _font);
void        BITglyphUpdate          (BITglyphDC _dc);
void        BITglyphText            (BITglyphDC _dc, int _sx, int _sy, char* _text, bool _outline, bool _shadow);
void        BITglyphDestroyDC       (BITglyphDC _dc);

#endif
