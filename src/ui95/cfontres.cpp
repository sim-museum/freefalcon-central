#include <windows.h>
#include "chandler.h"



//XX
extern DWORD RGB565toRGB8(WORD sc);


C_Fontmgr::C_Fontmgr()
{
    ID_ = 0;

    name_[0] = 0;
    height_ = 0;
    first_ = 0;
    last_ = 0;

    bytesperline_ = 0;

    fNumChars_ = 0;
    fontTable_ = NULL;

    dSize_ = 0;
    fontData_ = NULL;

    kNumKerns_ = 0;
    kernList_ = NULL;
}

C_Fontmgr::~C_Fontmgr()
{
    if (fontTable_ or fontData_ or kernList_)
        Cleanup();
}

void C_Fontmgr::Setup(long ID, char *fontfile)
{
    FILE *fp;

    ID_ = ID;

    fp = fopen(fontfile, "rb");

    if ( not fp)
    {
        MonoPrint("FONT error: %s not opened\n", fontfile);
        return;
    }

    fread(&name_, 32, 1, fp);
#ifdef FF_LINUX
    // FF_LINUX: font files store 4-byte values; long is 8 bytes on 64-bit
    {
        int32_t tmp32;
        fread(&tmp32, sizeof(int32_t), 1, fp); height_ = tmp32;
        fread(&first_, sizeof(short), 1, fp);
        fread(&last_, sizeof(short), 1, fp);
        fread(&tmp32, sizeof(int32_t), 1, fp); bytesperline_ = tmp32;
        fread(&tmp32, sizeof(int32_t), 1, fp); fNumChars_ = tmp32;
        fread(&tmp32, sizeof(int32_t), 1, fp); kNumKerns_ = tmp32;
        fread(&tmp32, sizeof(int32_t), 1, fp); dSize_ = tmp32;
    }
#else
    fread(&height_, sizeof(long), 1, fp);
    fread(&first_, sizeof(short), 1, fp);
    fread(&last_, sizeof(short), 1, fp);
    fread(&bytesperline_, sizeof(long), 1, fp);
    fread(&fNumChars_, sizeof(long), 1, fp);
    fread(&kNumKerns_, sizeof(long), 1, fp);
    fread(&dSize_, sizeof(long), 1, fp);
#endif

    if (fNumChars_)
    {
        fontTable_ = new CharStr[fNumChars_];
        fread(fontTable_, sizeof(CharStr), fNumChars_, fp);
    }

    if (kNumKerns_)
    {
        kernList_ = new KerningStr[kNumKerns_];
        fread(kernList_, sizeof(KerningStr), kNumKerns_, fp);
    }

    if (dSize_)
    {
#ifdef USE_SH_POOLS
        fontData_ = (char*)MemAllocPtr(UI_Pools[UI_GENERAL_POOL], sizeof(char) * (dSize_), FALSE);
#else
        fontData_ = new char[dSize_];
#endif
        fread(fontData_, dSize_, 1, fp);
    }

    fclose(fp);
#ifdef FF_LINUX
    /* FF-TEXT-2: OCR-A has no ae/AE. The game's OCR-A files (the chat font) were built on Windows, which stored a
       copy of a/A, or an a squeezed into the a's own cell, in those slots, so the chat (ocrabt12) showed
       "hej_\xe6r\xf8" as "hej_ar\xf8". A glyph no wider than its first letter cannot show two letters, so build
       it: the first letter, then the second sharing one column. FF_NO_FONTFIX=1 keeps the files' glyphs. */
    if (fontTable_ and fontData_ and bytesperline_ > 0 and bytesperline_ <= 16 and not getenv("FF_NO_FONTFIX"))
    {
        static const unsigned char lig[][3] = { {0xE6, 'a', 'e'}, {0xC6, 'A', 'E'} };
        const long per = bytesperline_ * height_;

        for (const auto &L : lig)
        {
            if (L[0] > last_ or L[1] < first_ or L[2] < first_ or (L[0] - first_ + 1) * per > dSize_)
                continue;

            CharStr &c = fontTable_[L[0] - first_];
            const CharStr &a = fontTable_[L[1] - first_], &e = fontTable_[L[2] - first_];
            const int w = a.w + e.w - 1;

            if (c.w > a.w or a.w == 0 or e.w == 0 or w > bytesperline_ * 8)
                continue;

            unsigned char *dc = (unsigned char *)fontData_ + (L[0] - first_) * per;
            const unsigned char *da = (unsigned char *)fontData_ + (L[1] - first_) * per;
            const unsigned char *de = (unsigned char *)fontData_ + (L[2] - first_) * per;

            for (long r = 0; r < height_; r++)
            {
                unsigned char row[16] = {0};
                const unsigned char *ra = da + r * bytesperline_, *re = de + r * bytesperline_;

                for (int j = 0; j < a.w; j++)
                    if (ra[j >> 3] & (1 << (j & 7))) row[j >> 3] |= (unsigned char)(1 << (j & 7));

                for (int j = 0; j < e.w; j++)
                    if (re[j >> 3] & (1 << (j & 7))) { int k = a.w - 1 + j; row[k >> 3] |= (unsigned char)(1 << (k & 7)); }

                memcpy(dc + r * bytesperline_, row, bytesperline_);
            }

            c.w = (unsigned char)w;
            c.lead = a.lead;
            c.trail = e.trail;
            fprintf(stderr, "[font] %.32s: built the %c%c ligature at 0x%02X (width %d)\n", name_, L[1], L[2], L[0], w);
        }
    }
#endif
}

void C_Fontmgr::Save(char *filename)
{
    FILE *fp;

    fp = fopen(filename, "wb");

    if ( not fp)
    {
        MonoPrint("FONT error: can't create %s\n", filename);
        return;
    }

    fwrite(&name_, 32, 1, fp);
    fwrite(&height_, sizeof(long), 1, fp);
    fwrite(&first_, sizeof(short), 1, fp);
    fwrite(&last_, sizeof(short), 1, fp);
    fwrite(&bytesperline_, sizeof(long), 1, fp);
    fwrite(&fNumChars_, sizeof(long), 1, fp);
    fwrite(&kNumKerns_, sizeof(long), 1, fp);
    fwrite(&dSize_, sizeof(long), 1, fp);

    if (fNumChars_)
    {
        fwrite(fontTable_, sizeof(CharStr), fNumChars_, fp);
    }

    if (kNumKerns_)
    {
        fwrite(kernList_, sizeof(KerningStr), kNumKerns_, fp);
    }

    if (dSize_)
    {
        fwrite(fontData_, dSize_, 1, fp);
    }

    fclose(fp);
}

void C_Fontmgr::Cleanup()
{
    ID_ = 0;
    first_ = 0;
    last_ = 0;
    bytesperline_ = 0;

    fNumChars_ = 0;

    if (fontTable_)
    {
        delete[] fontTable_;  // FF_LINUX: new CharStr[fNumChars_]
        fontTable_ = NULL;
    }

    kNumKerns_ = 0;

    if (kernList_)
    {
        delete[] kernList_;  // FF_LINUX: new KerningStr[kNumKerns_]
        kernList_ = NULL;
    }

    dSize_ = 0;

    if (fontData_)
    {
#ifdef USE_SH_POOLS
        MemFreePtr(fontData_);
#else
        delete[] fontData_;  // FF_LINUX: new char[dSize_]
#endif
        fontData_ = NULL;
    }
}

long C_Fontmgr::Width(_TCHAR *str)
{
    long i;
    long size;
    long thechar;

    if ( not str)
        return(0);

    size = 0;
    i = 0;

    while (str[i])
        //while( not F4IsBadReadPtr(&(str[i]), sizeof(_TCHAR)) and str[i]) // JB 010401 CTD (too much CPU)
    {
        thechar = str[i] bitand 0xff;

        if (thechar >= first_ and thechar <= last_)
        {
            thechar -= first_;
            size += fontTable_[thechar].lead + fontTable_[thechar].w + fontTable_[thechar].trail;
        }

        i++;
    }

    return(size + 1);
}

long C_Fontmgr::Width(_TCHAR *str, long len)
{
    long i;
    long size;
    long thechar;

    if ( not str)
        return(0);

    size = 0;
    i = 0;

    while (str[i] and i < len)
    {
        thechar = str[i] bitand 0xff;

        if (thechar >= first_ and thechar <= last_)
        {
            thechar -= first_;
            size += fontTable_[thechar].lead + fontTable_[thechar].w + fontTable_[thechar].trail;
        }

        i++;
    }

    return(size + 1);
}

long C_Fontmgr::Height()
{
    return(height_);
}

CharStr *C_Fontmgr::GetChar(short ID)
{
    if (fontTable_ and ID >= first_ and ID <= last_)
        return(&fontTable_[ID - first_]);

    return(NULL);
}


void C_Fontmgr::Draw(SCREEN *surface, _TCHAR *str, long length, WORD color, long x, long y)
{
    long idx, i, j;
    long xoffset, yoffset;
    unsigned long thechar;
    unsigned char *sstart, *sptr, seg = 0;
    WORD *dstart, *dptr, *dendh, *dendv;
    bool b32 = surface->bpp == 32;//XX

    if ( not fontData_)
        return;

    if ( not str)
        return;

    idx = 0;
    xoffset = x;
    yoffset = y;

    if (b32) //XX
        dendv = surface->mem + 2 * (surface->width * surface->height);
    else
        dendv = surface->mem + surface->width * surface->height; // Make sure we don't go past the end of the surface


    while (str[idx] and idx < length)
    {
        thechar = str[idx] bitand 0xff;

        if (thechar >= (unsigned long)first_ and thechar <= (unsigned long)last_) 
        {
            thechar -= first_;
            xoffset += fontTable_[thechar].lead;

            sstart = (unsigned char *)(fontData_ + (thechar * bytesperline_ * height_));

            if (b32)
            {
                dstart = surface->mem + 2 * ((yoffset * surface->width) + xoffset);
                dendh = surface->mem + 2 * ((yoffset * surface->width) + surface->width);
            }
            else
            {
                dstart = surface->mem + (yoffset * surface->width) + xoffset;
                dendh = surface->mem + (yoffset * surface->width) + surface->width;
            }


            for (i = 0; i < height_ and dstart < dendv; i++)
            {
                dptr = dstart;
                sptr = sstart;

                for (j = 0; j < fontTable_[thechar].w; j++)
                {
                    if (dptr < dendh)
                    {
                        if ( not (j bitand 0x7))
                            seg = *sptr++;

                        //XX
                        //if(seg bitand 1)
                        // *dptr++=color;
                        //else
                        // dptr++;

                        if (seg bitand 1)
                        {
                            if (b32)
                                *((DWORD*)(dptr)) = RGB565toRGB8(color);
                            else
                                *dptr = color;
                        }

                        seg >>= 1;
                        ++dptr;

                        if (b32) //XX
                            ++dptr;
                    }
                }

                sstart += bytesperline_;
                dstart += surface->width;
                dendh += surface->width;

                if (b32)  //XX
                {
                    dstart += surface->width;
                    dendh += surface->width;
                }
            }

            xoffset += fontTable_[thechar].w + fontTable_[thechar].trail;
        }

        idx++;
    }
}

void C_Fontmgr::DrawSolid(SCREEN *surface, _TCHAR *str, long length, WORD color, WORD bgcolor, long x, long y)
{
    long idx, i, j;
    long xoffset, yoffset;
    unsigned long thechar;
    unsigned char *sstart, *sptr, seg = 0;
    WORD *dstart, *dptr, *dendh, *dendv;

    if ( not fontData_)
        return;

    if ( not str)
        return;

    bool b32 = surface->bpp == 32;//XX

    idx = 0;
    xoffset = x;
    yoffset = y;

    if (b32) //XX
        dendv = surface->mem + 2 * (surface->width * surface->height);
    else
        dendv = surface->mem + surface->width * surface->height; // Make sure we don't go past the end of the surface

    while (str[idx] and idx < length)
    {
        thechar = str[idx] bitand 0xff;

        if (thechar >= (unsigned long)first_ and thechar <= (unsigned long)last_) 
        {
            thechar -= first_;

            sstart = (unsigned char *)(fontData_ + (thechar * bytesperline_ * height_));

            if (b32) //XX word->dword
            {
                dstart = surface->mem + 2 * ((yoffset * surface->width) + xoffset);
                dendh = surface->mem + 2 * ((yoffset * surface->width) + surface->width);
            }
            else
            {
                dstart = surface->mem + (yoffset * surface->width) + xoffset;
                dendh = surface->mem + (yoffset * surface->width) + surface->width;
            }

            for (i = 0; i < height_ and dstart < dendv; i++)
            {
                dptr = dstart;
                sptr = sstart;

                for (j = 0; j < fontTable_[thechar].lead; j++)
                    if (dptr < dendh)
                    {
                        if (b32)
                        {
                            *((DWORD*)(dptr)) = RGB565toRGB8(bgcolor);
                            dptr += 2;
                        }
                        else
                            *dptr++ = bgcolor;
                    }

                for (j = 0; j < fontTable_[thechar].w; j++)
                {
                    if ( not (j bitand 0x7))
                        seg = *sptr++;

                    if (dptr < dendh)
                    {
                        if (b32) //XX
                        {
                            if (seg bitand 1)
                                *((DWORD*)(dptr)) = RGB565toRGB8(color);
                            else
                                *((DWORD*)(dptr)) = RGB565toRGB8(bgcolor);

                            dptr += 2;
                        }
                        else
                        {
                            if (seg bitand 1)
                                *dptr++ = color;
                            else
                                *dptr++ = bgcolor;
                        }

                        seg >>= 1;
                    }
                }

                for (j = 0; j < fontTable_[thechar].trail; j++)
                    if (dptr < dendh)
                    {
                        if (b32) //XX
                        {
                            *((DWORD*)(dptr)) = RGB565toRGB8(bgcolor);
                            dptr += 2;
                        }
                        else
                            *dptr++ = bgcolor;
                    }

                sstart += bytesperline_;
                dstart += surface->width;
                dendh += surface->width;

                if (b32) //XX
                {
                    dstart += surface->width;
                    dendh += surface->width;
                }
            }

            xoffset += fontTable_[thechar].lead + fontTable_[thechar].w + fontTable_[thechar].trail;
        }

        idx++;
    }
}

void C_Fontmgr::Draw(SCREEN *surface, _TCHAR *str, WORD color, long x, long y)
{
    if (str) Draw(surface, str, _tcsclen(str), color, x, y);
}

void C_Fontmgr::DrawSolid(SCREEN *surface, _TCHAR *str, WORD color, WORD bgcolor, long x, long y)
{
    if (str) DrawSolid(surface, str, _tcsclen(str), color, bgcolor, x, y);
}

void C_Fontmgr::_Draw16(SCREEN *surface, _TCHAR *str, long length, WORD color, long x, long y, UI95_RECT *cliprect)
// not void C_Fontmgr::Draw(SCREEN *surface,_TCHAR *str,short length,WORD color,long x,long y,UI95_RECT *cliprect)
{
    long idx, i, j;
    long xoffset, yoffset;
    unsigned long thechar;
    unsigned char *sstart, *sptr, seg = 0;
    WORD *dstart, *dptr;
    WORD *dendh, *dendv;
    WORD *dclipx, *dclipy;

    if ( not fontData_)
        return;

    if ( not str)
        return;

    idx = 0;
    xoffset = x;
    yoffset = y;
    dclipy = surface->mem + (cliprect->top * surface->width);
    dendv = surface->mem + (cliprect->bottom * surface->width); // Make sure we don't go past the end of the surface

    while (str[idx] and idx < length)
    {
        thechar = str[idx] bitand 0xff;

        if (thechar >= (unsigned long)first_ and thechar <= (unsigned long)last_)
        {
            thechar -= first_;
            xoffset += fontTable_[thechar].lead;

            sstart = (unsigned char *)(fontData_ + (thechar * bytesperline_ * height_));
            dstart = surface->mem + (yoffset * surface->width) + xoffset;
            dclipx = surface->mem + (yoffset * surface->width) + cliprect->left;
            dendh = dclipx + (cliprect->right - cliprect->left);

            for (i = 0; i < height_ and dstart < dendv; i++)
            {
                if (dstart >= dclipy)
                {
                    dptr = dstart;
                    sptr = sstart;

                    for (j = 0; j < fontTable_[thechar].w; j++)
                    {
                        if ( not (j bitand 0x7))
                        {
                            seg = *sptr++;

                            if ( not seg)
                            {
                                j += 7;
                                dptr += 8;
                                continue;
                            }
                        }

                        if (dptr < dendh)
                        {
                            if (dptr >= dclipx)
                            {
                                if (seg bitand 1)
                                    *dptr++ = color;
                                else
                                    dptr++;
                            }
                            else
                                dptr++;

                            seg >>= 1;
                        }
                    }
                }

                sstart += bytesperline_;
                dclipx += surface->width;
                dstart += surface->width;
                dendh += surface->width;
            }

            xoffset += fontTable_[thechar].w + fontTable_[thechar].trail;
        }

        idx++;
    }
}

void C_Fontmgr::_Draw32(SCREEN *surface, _TCHAR *str, long length, DWORD dwColor, long x, long y, UI95_RECT *cliprect)
{
    long idx, i, j;
    long xoffset, yoffset;
    unsigned long thechar;
    unsigned char *sstart, *sptr, seg = 0;

    DWORD *dstart, *dptr;
    DWORD *dendh, *dendv;
    DWORD *dclipx, *dclipy;

    if ( not fontData_)
        return;

    if ( not str)
        return;

    DWORD *surfmem = (DWORD*) surface->mem;

    idx = 0;
    xoffset = x;
    yoffset = y;
    dclipy = surfmem + (cliprect->top * surface->width);
    dendv = surfmem + (cliprect->bottom * surface->width); // Make sure we don't go past the end of the surface

    while (str[idx] and idx < length)
    {
        thechar = str[idx] bitand 0xff;

        if (thechar >= (unsigned long)first_ and thechar <= (unsigned long)last_)
        {
            thechar -= first_;
            xoffset += fontTable_[thechar].lead;

            sstart = (unsigned char *)(fontData_ + (thechar * bytesperline_ * height_));

            dstart = surfmem + (yoffset * surface->width) + xoffset;
            dclipx = surfmem + (yoffset * surface->width) + cliprect->left;
            dendh = dclipx + (cliprect->right - cliprect->left);

            for (i = 0; i < height_ and dstart < dendv; i++)
            {
                if (dstart >= dclipy)
                {
                    dptr = dstart;
                    sptr = sstart;

                    for (j = 0; j < fontTable_[thechar].w; j++)
                    {
                        if ( not (j bitand 0x7))
                        {
                            seg = *sptr++;

                            if ( not seg)
                            {
                                j += 7;
                                dptr += 8;
                                continue;
                            }
                        }

                        if (dptr < dendh)
                        {
                            if (dptr >= dclipx)
                            {
                                if (seg bitand 1)
                                    *dptr++ = dwColor;
                                else
                                    dptr++;
                            }
                            else
                                dptr++;

                            seg >>= 1;
                        }
                    }
                }

                sstart += bytesperline_;
                dclipx += surface->width;
                dstart += surface->width;
                dendh += surface->width;
            }

            xoffset += fontTable_[thechar].w + fontTable_[thechar].trail;
        }

        idx++;
    }
}

//XX
void C_Fontmgr::DrawSolid(SCREEN *surface, _TCHAR *str, long length, WORD color, WORD bgcolor, long x, long y, UI95_RECT *cliprect)
{
    if (surface->bpp == 32)
        _DrawSolid32(surface, str, length, RGB565toRGB8(color), RGB565toRGB8(bgcolor), x, y, cliprect);
    else
        _DrawSolid16(surface, str, length, color, bgcolor, x, y, cliprect);
}
//XX
void C_Fontmgr::_DrawSolid16(SCREEN *surface, _TCHAR *str, long length, WORD color, WORD bgcolor, long x, long y, UI95_RECT *cliprect)
// not void C_Fontmgr::DrawSolid(SCREEN *surface,_TCHAR *str,short length,WORD color,WORD bgcolor,long x,long y,UI95_RECT *cliprect)
{
    long idx, i, j;
    long xoffset, yoffset;
    unsigned long thechar;
    unsigned char *sstart, *sptr, seg = 0;
    WORD *dstart, *dptr;
    WORD *dendh, *dendv;
    WORD *dclipx, *dclipy;

    if ( not fontData_)
        return;

    if ( not str)
        return;

    idx = 0;
    xoffset = x;
    yoffset = y;
    dclipy = surface->mem + (cliprect->top * surface->width);
    dendv = surface->mem + (cliprect->bottom * surface->width); // Make sure we don't go past the end of the surface

    while (str[idx] and idx < length)
    {
        thechar = str[idx] bitand 0xff;

        if (thechar >= (unsigned long)first_ and thechar <= (unsigned long)last_)
        {
            thechar -= first_;

            sstart = (unsigned char *)(fontData_ + (thechar * bytesperline_ * height_));
            dstart = surface->mem + (yoffset * surface->width) + xoffset;
            dclipx = surface->mem + (yoffset * surface->width) + cliprect->left;
            dendh = dclipx + (cliprect->right - cliprect->left);

            for (i = 0; i < height_ and dstart < dendv; i++)
            {
                if (dstart >= dclipy)
                {
                    dptr = dstart;
                    sptr = sstart;

                    for (j = 0; j < fontTable_[thechar].lead; j++)
                    {
                        if (dptr < dendh)
                        {
                            if (dptr >= dclipx)
                                *dptr++ = bgcolor;
                            else
                                dptr++;
                        }
                    }

                    for (j = 0; j < fontTable_[thechar].w; j++)
                    {
                        if (dptr < dendh)
                        {
                            if ( not (j bitand 0x7))
                                seg = *sptr++;

                            if (dptr >= dclipx)
                            {
                                if (seg bitand 1)
                                    *dptr++ = color;
                                else
                                    *dptr++ = bgcolor;
                            }
                            else
                                dptr++;

                            seg >>= 1;
                        }
                    }

                    for (j = 0; j < fontTable_[thechar].trail; j++)
                    {
                        if (dptr < dendh)
                        {
                            if (dptr >= dclipx)
                                *dptr++ = bgcolor;
                            else
                                dptr++;
                        }
                    }
                }

                sstart += bytesperline_;
                dclipx += surface->width;
                dstart += surface->width;
                dendh += surface->width;
            }

            xoffset += fontTable_[thechar].lead + fontTable_[thechar].w + fontTable_[thechar].trail;
        }

        idx++;
    }
}

//XX
void C_Fontmgr::_DrawSolid32(SCREEN *surface, _TCHAR *str, long length, DWORD color, DWORD bgcolor, long x, long y, UI95_RECT *cliprect)
{
    long idx, i, j;
    long xoffset, yoffset;
    unsigned long thechar;
    unsigned char *sstart, *sptr, seg = 0;
    DWORD *dstart, *dptr;
    DWORD *dendh, *dendv;
    DWORD *dclipx, *dclipy;

    if ( not fontData_)
        return;

    if ( not str)
        return;

    DWORD *surfmem = (DWORD*)surface->mem;

    idx = 0;
    xoffset = x;
    yoffset = y;
    dclipy = surfmem + (cliprect->top * surface->width);
    dendv = surfmem + (cliprect->bottom * surface->width); // Make sure we don't go past the end of the surface

    while (str[idx] and idx < length)
    {
        thechar = str[idx] bitand 0xff;

        if (thechar >= (unsigned long)first_ and thechar <= (unsigned long)last_)
        {
            thechar -= first_;

            sstart = (unsigned char *)(fontData_ + (thechar * bytesperline_ * height_));
            dstart = surfmem + (yoffset * surface->width) + xoffset;
            dclipx = surfmem + (yoffset * surface->width) + cliprect->left;
            dendh = dclipx + (cliprect->right - cliprect->left);

            for (i = 0; i < height_ and dstart < dendv; i++)
            {
                if (dstart >= dclipy)
                {
                    dptr = dstart;
                    sptr = sstart;

                    for (j = 0; j < fontTable_[thechar].lead; j++)
                    {
                        if (dptr < dendh)
                        {
                            if (dptr >= dclipx)
                                *dptr++ = bgcolor;
                            else
                                dptr++;
                        }
                    }

                    for (j = 0; j < fontTable_[thechar].w; j++)
                    {
                        if (dptr < dendh)
                        {
                            if ( not (j bitand 0x7))
                                seg = *sptr++;

                            if (dptr >= dclipx)
                            {
                                if (seg bitand 1)
                                    *dptr++ = color;
                                else
                                    *dptr++ = bgcolor;
                            }
                            else
                                dptr++;

                            seg >>= 1;
                        }
                    }

                    for (j = 0; j < fontTable_[thechar].trail; j++)
                    {
                        if (dptr < dendh)
                        {
                            if (dptr >= dclipx)
                                *dptr++ = bgcolor;
                            else
                                dptr++;
                        }
                    }
                }

                sstart += bytesperline_;
                dclipx += surface->width;
                dstart += surface->width;
                dendh += surface->width;
            }

            xoffset += fontTable_[thechar].lead + fontTable_[thechar].w + fontTable_[thechar].trail;
        }

        idx++;
    }
}



void C_Fontmgr::Draw(SCREEN *surface, _TCHAR *str, WORD color, long x, long y, UI95_RECT *cliprect)
{
    if (str)
    {
        //XX
        if (surface->bpp == 32)
            _Draw32(surface, str, _tcsclen(str), RGB565toRGB8(color), x, y, cliprect);
        else
            _Draw16(surface, str, _tcsclen(str), color, x, y, cliprect);
    }
}

void C_Fontmgr::DrawSolid(SCREEN *surface, _TCHAR *str, WORD color, WORD bgcolor, long x, long y, UI95_RECT *cliprect)
{
    if (str)
        DrawSolid(surface, str, _tcsclen(str), color, bgcolor, x, y, cliprect);
}

//XX
void C_Fontmgr::Draw(SCREEN *surface, _TCHAR *str, long length, WORD color, long x, long y, UI95_RECT *cliprect)
{
    if (str)
    {
        //XX
        if (surface->bpp == 32)
            _Draw32(surface, str, length, RGB565toRGB8(color), x, y, cliprect);
        else
            _Draw16(surface, str, length, color, x, y, cliprect);
    }
}
