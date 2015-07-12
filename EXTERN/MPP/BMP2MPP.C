// -------------------------------------------------------------------
// BMP to MPP file converter.
// by Zerkman / Sector One
//--------------------------------------------------------------------

/* This program is free software. It comes without any warranty, to
* the extent permitted by applicable law. You can redistribute it
* and/or modify it under the terms of the Do What The Fuck You Want
* To Public License, Version 2, as published by Sam Hocevar. See
* http://sam.zoy.org/wtfpl/COPYING for more details. */

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <limits.h>
#include <time.h>

#define MAX_NCOLORS 64
#define MAX_WIDTH 416
#define MAX_HEIGHT 274
#define MAX_XDELTA 160

#define ANNEAL_INIT_T_RATIO 1


static void __mem_error(const void *ptr, const char *name, const char* file, int line) 
{
    if (!ptr) 
    {
        fprintf(stderr, "%s:%d:allocation for pointer `%s' failed\n", file, line, name);
        exit(1);
    }
}

#define MEM_ERROR(ptr) __mem_error(ptr, #ptr, __FILE__, __LINE__)

typedef struct 
{
    int id;
    int ncolors;
    int nfixed;
    int scancolors;
    int x0;
    int (*xinc)(int);
    int width;
    int height;
} 
Mode;

static int xinc0(int c) { return ((c==15)?88:((c==31)?12:((c==37)?100:4))); }
static int xinc1(int c) { return (((c)&1)?16:4); }
static int xinc2(int c) { return 8; }
static int xinc3(int c) { return ((c==15)?112:((c==31)?12:((c==37)?100:4))); }

Mode modes[4] = 
{
    { 0, 54, 0, 54, 33, xinc0, 320, 200 },
    { 1, 48, 0, 48,  9, xinc1, 320, 200 },
    { 2, 64, 0, 58,  4, xinc2, 336, 200 },
    { 3, 54, 6, 54, 69, xinc3, 416, 274 },
};

#define IMAX(x) (16-((x>0)&&(first[x-1]!=first[x])))

#define STR_EX(x) #x
#define STR(x) STR_EX(x)

#define EXTC(a,s) ((a>>s)&0x1f)

static void write16(void *ptr, unsigned short x) 
{
    unsigned char *p = (unsigned char*) ptr;
    p[0] = (unsigned char) (x>>8);
    p[1] = (unsigned char) x;
}

static void write32(void *ptr, unsigned int x) 
{
    unsigned char *p = ptr;
    p[0] = x>>24;
    p[1] = x>>16;
    p[2] = x>>8;
    p[3] = x;
}

static int cdist(unsigned short a, unsigned short b) 
{
    int dr = EXTC(a,10) - EXTC(b,10);
    int dg = EXTC(a,5) - EXTC(b,5);
    int db = EXTC(a,0) - EXTC(b,0);
    /*	if (dr<0) dr = -dr;
    if (dg<0) dg = -dg;
    if (db<0) db = -db;
    int d = dr;
    if (dg>d) d = dg;
    if (db>d) d = db;
    return d*(dr+dg+db); */
    return dr*dr+dg*dg+db*db;
    /*	return dr*19+dg*55+db*26; */
}

int greedy(unsigned char *ppix, unsigned short *ppal, const Mode *mode, const unsigned short *line, int special)
{
    static int first[MAX_WIDTH];
    static int init = 0;
    int i, j, x;
    int cost = 0;

    if (!init) 
    {
        int nx = mode->x0;
        int idx = 0;
        int c = 0;
        for (x=0; x<mode->width; ++x) 
        {
            if (x == nx) 
            {
                ++idx;
                nx += mode->xinc(c);
                ++c;
            }
            first[x] = idx;
        }
        init=1;
    }

    if (special) 
    {
        memset(ppal+mode->nfixed, -1, (12-mode->nfixed)*sizeof(unsigned short));
        memset(ppal+16, -1, (mode->ncolors-16)*sizeof(unsigned short));
    } 
    else
        memset(ppal+mode->nfixed, -1, (mode->ncolors-mode->nfixed)*sizeof(unsigned short));
    
    memset(ppix, -1, mode->width);
    if (mode->id != 3)
        ppal[0] = 0;
    
    ppal[(mode->ncolors-1) & -16] = 0;

    for (i=0; i<mode->width; ++i) 
    {
        int jmax, idx;
        unsigned short *p, c;
        x = ((i*4)+(i/(mode->width/4))+1)%mode->width;
        jmax = IMAX(x);
        p = ppal + first[x];
        c = line[x];
        idx = -1;
        for (j=0; j<jmax; ++j) 
        {
            if (p[j] == c) 
            {
                idx = j;
                break;
            }
        }
        if (idx == -1) 
        {
            for (j=0; j<jmax; ++j) 
            {
                if (p[j] == 0xffff) 
                {
                    idx = j;
                    p[idx] = c;
                    break;
                }
            }
        }
        if (idx == -1) 
        {
            int dist = cdist(c, p[0]);
            idx = 0;
            for (j=1; j<jmax; ++j) 
            {
                int d = cdist(c, p[j]);
                if (d < dist) 
                {
                    dist = d;
                    idx = j;
                }
            }
            cost += dist;
        }
        ppix[x] = (idx + first[x]) & 0xf;
    }
    for (i=mode->nfixed; i<mode->ncolors+mode->nfixed; ++i)
        if (ppal[i] == 0xffff)
            ppal[i] = 0;
    return cost;
}

struct instance 
{
    /* instance-independent data */
    int nc;
    int xmax[MAX_NCOLORS];
    int first[MAX_WIDTH];

    /* variable data */
    const unsigned short *line;
};

struct solution 
{
    unsigned short pal[MAX_NCOLORS];
    unsigned char pix[MAX_WIDTH];
    int eval;
};

struct bbstr 
{
    /* problem and solution pointers */
    int *ppix;
    unsigned short *ppal;

    /* internal data */
    struct instance inst;

    /* current solution */
    struct solution sol;
    int bound;
    int ttl;
};

void instance_init(struct instance *inst, const Mode *mode) 
{
    int i, x;
    int nx = mode->x0;
    int nc = 0;
    int c = 0;

    for (x=0; x<mode->width; ++x) 
    {
        if (x == nx) 
        {
            inst->xmax[nc] = x;
            ++nc;
            nx += mode->xinc(c);
            ++c;
        }
        inst->first[x] = nc;
    }
    for (i=0; i<16; ++i)
        inst->xmax[nc++] = x;
    inst->nc = nc;
}

void solution_init(struct solution *sol, const Mode *mode) 
{
    memset(sol->pal, -1, mode->ncolors*sizeof(unsigned short));
    memset(sol->pix, -1, mode->width*sizeof(int));
    sol->eval = 0;
}

/* Compute distance array of a solution */
void solution_computedist(const Mode *mode, int *dist, struct solution *sol, const struct instance *inst)
{
    int x;
    sol->eval = 0;
    for (x=0; x<mode->width; ++x) 
    {
        unsigned short *pal = sol->pal + inst->first[x];
        unsigned short c = inst->line[x];
        unsigned short ca = pal[(sol->pix[x]-inst->first[x]) & 0xf];
        dist[x] = cdist(c, ca);
        sol->eval += dist[x];
    }
}

/* Cost of changing palette entry palidx to color col */
static int compute_delta(const Mode *mode, const struct solution *sol, const struct instance *inst, int *dist, int palidx, unsigned short col)
{
    int delta = 0;
    int x0 = (palidx<16)?0:inst->xmax[palidx-16];
    int x1 = inst->xmax[palidx];
    int x, i;
    const int *first = inst->first;
    for (x=x0; x<x1; ++x) 
    {
        if (sol->pix[x] == (palidx&0xf) || dist[x] != 0) 
        {
            /* cost of changing color palidx to col for pixel x */
            int f = first[x];
            unsigned short c = inst->line[x];
            int cost = cdist(c, col);
            int imax = IMAX(x);
            for (i=0; i<imax; ++i) {
                int d, idx = i+f;
                if ((idx&0xf) == (palidx&0xf))	/* ? */
                    continue;
                d = cdist(c, sol->pal[idx]);
                if (d < cost)
                    cost = d;
            }
            delta += cost - dist[x];
        }
    }
    return delta;
}

/* Change palette entry palidx to color col */
static void apply_delta(struct solution *sol, const struct instance *inst, int dist[MAX_WIDTH], int palidx, unsigned short col)
{
    int x0 = (palidx<16)?0:inst->xmax[palidx-16];
    int x1 = inst->xmax[palidx];
    int x, i;
    const int *first = inst->first;
    sol->pal[palidx] = col;
    palidx &= 0xf;
    for (x=x0; x<x1; ++x) 
    {
        if (sol->pix[x] == palidx || dist[x] != 0) 
        {
            /* minimize cost of changing color palidx to col for pixel x */
            int f = first[x];
            unsigned short c = inst->line[x];
            int cost = cdist(c, sol->pal[f]);
            int best = 0;
            int imax = IMAX(x);
            for (i=1; i<imax; ++i) {
                int d = cdist(c, sol->pal[i+f]);
                if (d < cost) {
                    cost = d;
                    best = i;
                }
            }
            sol->pix[x] = (best+f)&0xf;
            sol->eval += cost - dist[x];
            dist[x] = cost;
        }
    }
}


int anneal(unsigned char *ppix, unsigned short *ppal, const Mode *mode, const unsigned short *line, int bnd, int opt_level, int special)
{
    struct instance inst;
    struct solution sol, best;
    int dist[MAX_WIDTH];
    int colorcnt[MAX_NCOLORS];
    unsigned short colors[MAX_NCOLORS][MAX_XDELTA];
    int colornum[MAX_NCOLORS];
    int i, j;
    int orig_eval, worst_eval;
    int threshold = opt_level * opt_level * mode->ncolors / 16;
    int nval;
    double t;
    int cool_cnt;

    instance_init(&inst, mode);
    inst.line = line;
    memcpy(&sol.pal, ppal, mode->ncolors*sizeof(unsigned short));
    memcpy(&sol.pix, ppix, mode->width);
    solution_computedist(mode, dist, &sol, &inst);
    best = sol;
    orig_eval = best.eval;
    worst_eval = orig_eval;

    /* init color arrays */
    nval = 0;
    for (i=mode->nfixed; i<inst.nc; ++i) 
    {
        unsigned char have[32768/8];
        int x0 = (i<16)?0:inst.xmax[i-16];
        int x1 = inst.xmax[i];
        int x;
        int nh = 0;
        memset(have, 0, sizeof(have));
        for (x=x0; x<x1; ++x) 
        {
            unsigned short c = line[x];
            int mask = 1<<(c&7);
            int pos = c/8;
            if (!(have[pos]&mask)) 
            {
                have[pos] |= mask;
                colors[i][nh++] = c;
            }
        }
        colorcnt[i] = nh;
        if (nh > nval)
            nval = nh;
    }
    for (i=0; i<mode->nfixed; ++i) 
    {
        colors[i][0] = sol.pal[i];
        colorcnt[i] = 1;
    }
    if (special) 
    {
        for(i=12; i<16; ++i) 
        {
            colors[i][0] = sol.pal[i];
            colorcnt[i] = 1;
        }
    }
    for (i=0; i<inst.nc; ++i) 
    {
        colornum[i] = -1;
        for (j=0; j<colorcnt[i]; ++j) 
        {
            if (colors[i][j] == sol.pal[i])
                colornum[i] = j;
        }
    }
    
    if (mode->id != 3) 
    {
        /* Force left border color to black in non-overscan modes */
        colors[0][0] = 0;
        colorcnt[0] = 1;
        colornum[0] = 0;
    }
    colors[(mode->ncolors-1) & -16][0] = 0;
    colorcnt[(mode->ncolors-1) & -16] = 1;
    colornum[(mode->ncolors-1) & -16] = 0;

    /* simulated annealing loop */
    t = ANNEAL_INIT_T_RATIO*sol.eval;
    cool_cnt = 0;
    
    for (;;) 
    {
        int palidx, num, delta;
        
        do 
        {
            palidx = rand()%inst.nc;
        } 
        while (colorcnt[palidx] < 2);

        num = rand()%(colorcnt[palidx]-1);
        num += num >= colornum[palidx];

        delta = compute_delta(mode, &sol, &inst, dist, palidx, colors[palidx][num]);

        if (rand()*(1./RAND_MAX) < exp(-delta/t)) 
        {
            /* update solution */

            apply_delta(&sol, &inst, dist, palidx, colors[palidx][num]);

            if (sol.eval < best.eval) 
            {
                best = sol;
                if (sol.eval == 0)
                    break;
            }
        } 
        else 
        {
            int eval = sol.eval + delta;
            if (eval > worst_eval)
                worst_eval = eval;
        }
        ++cool_cnt;
        if (cool_cnt == threshold) 
        {
            double tf = 0.02*(worst_eval-best.eval)/(nval*.693-3);
            t = t / (1. + 100.*t/(worst_eval+1));
            if (t < tf)
                break;
            cool_cnt = 0;
        }
    }

    if (best.eval < orig_eval) {
        memcpy(ppal, &best.pal, mode->ncolors*sizeof(unsigned short));
        memcpy(ppix, &best.pix, mode->width);
        return best.eval;
    }
    return orig_eval;
}

int file_error(const char *file,  const char *msg) 
{
    fprintf(stderr, "%s: %s\n", file, msg);
    return 1;
}

static int readle2(const char *ptr) 
{
    const signed char *sptr = (const signed char *)ptr;
    const unsigned char *uptr = (const unsigned char *)ptr;
    int x;
    x = uptr[0];
    x |= sptr[1]<<8;
    return x;
}

static int readle4(const char *ptr) 
{
    const signed char *sptr = (const signed char *)ptr;
    const unsigned char *uptr = (const unsigned char *)ptr;
    int x;
    x = uptr[0];
    x |= uptr[1]<<8;
    x |= uptr[2]<<16;
    x |= sptr[3]<<24;
    return x;
}


void convert(FILE *fd, const unsigned char *bmp, const Mode *mode, int bits, int flick, int opt, int raw_palette, int err)
{
    unsigned char *pixels, *ppix;
    unsigned short *palette;
    int i, x, y;
    int penalty0 = 0, penalty1 = 0;
    int npal, nbits, bpp;
    unsigned short *bitmap, *pbitmap;
    unsigned int tmpcol;


    pixels  = malloc(mode->width*mode->height);
    palette = malloc(mode->height*(mode->ncolors-mode->nfixed)*sizeof(unsigned short));
    bitmap  = malloc(mode->width/4*(mode->height-1)*sizeof(unsigned short));

    MEM_ERROR(pixels);
    MEM_ERROR(palette);
    MEM_ERROR(bitmap);

    memset(palette, 0, (mode->ncolors-mode->nfixed)*sizeof(palette[0]));

    for (y=1; y<mode->height; ++y) 
    {
        const unsigned char *pbmp = &bmp[mode->width*3*(mode->height-1-y)];
        unsigned short *ppal = &palette[y*(mode->ncolors-mode->nfixed)-mode->nfixed];
        unsigned short line[MAX_WIDTH];
        int val, special = 0;

        for (x=0; x<mode->width; ++x) 
        {
            int r, g, b, t;
            int mask = 0;
            if (flick && ((flick^y)&1))
                mask = 1<<(7-bits);
            t = *pbmp++;
            b = t>>(8-bits);
            b += b < (1<<bits)-1 && (t&mask) != 0;
            t = *pbmp++;
            g = t>>(8-bits);
            g += g < (1<<bits)-1 && (t&mask) != 0;
            t = *pbmp++;
            r = t>>(8-bits);
            r += r < (1<<bits)-1 && (t&mask) != 0;
            line[x] = (r << 10) | (g << 5) | b;
        }
        
        if (y==229) 
        {
            memcpy(ppal+12, ppal-4, 4*sizeof(unsigned short));
            special = 1;
        }

        val = greedy(pixels+y*mode->width, ppal, mode, line, special);
        penalty0 += val;
        if (val && opt) 
        {
            val = anneal(pixels+y*mode->width, ppal, mode, line, val, opt, special);
            penalty1 += val;
        }

        if (val && err)
            printf("y=%d error=%d          \n", y, val);
        
        if (opt) 
        {
            char buf[16];
            if (penalty1)
                sprintf(buf, "%.02f", (double)penalty0/penalty1);
            else
                strcpy(buf, "*");
            printf("y=%d gain=%s       \r", y, buf);
            fflush(stdout);
        }
    }

    if (opt) 
    {
        char buf[16];
        if (penalty1)
            sprintf(buf, "%.02f", (double)penalty0/penalty1);
        else
            strcpy(buf, "inf");
        printf("Error penalty: initial %d, final %d. Gain ratio: %s\n",
            penalty0, penalty1, buf);
    }
    else
    {
        printf("Error penalty: %d.\n", penalty0);
    }

    ppix = &pixels[mode->width];
    pbitmap = bitmap;
    npal = 0;
    nbits = 0;
    bpp = 3*bits;
    tmpcol = 0;

    for (y=1; y<mode->height; ++y) 
    {
        for (x=0; x<mode->width; x+=16) 
        {
            unsigned short b0=0, b1=0, b2=0, b3=0;
            for (i=0; i<16; ++i) 
            {
                int c = *ppix++;
                b0 |= (c&1) << (15-i);
                b1 |= ((c&2)>>1) << (15-i);
                b2 |= ((c&4)>>2) << (15-i);
                b3 |= ((c&8)>>3) << (15-i);
            }
            write16(pbitmap+0, b0);
            write16(pbitmap+1, b1);
            write16(pbitmap+2, b2);
            write16(pbitmap+3, b3);
            pbitmap += 4;
        }
    
        for (x=0; x<(mode->ncolors-mode->nfixed); ++x) 
        {
            unsigned short c = palette[y*(mode->ncolors-mode->nfixed)+x], ex;
            switch (bits) 
            {
            case 3:
                if (raw_palette)
                    c = ((c>>10)&0x7)<<8 | ((c>>5)&0x7)<<4 | (c&0x7);
                else
                    c = ((c>>10)&0x7)<<6 | ((c>>5)&0x7)<<3 | (c&0x7);
                break;
            case 4:
                c = ((c>>10)&0xf)<<8 | ((c>>5)&0xf)<<4 | (c&0xf);
                c = ((c&0xeee)>>1) | ((c&0x111)<<3);
                break;
            case 5:
                ex = ((c>>10)&1)<<14 | ((c>>5)&1)<<13 | (c&1)<<12;
                c = ((c>>11)&0xf)<<8 | ((c>>6)&0xf)<<4 | ((c>>1)&0xf);
                c = ex | ((c&0xeee)>>1) | ((c&0x111)<<3);
                break;
            default:
                fprintf(stderr, "wrong number of bits\n");
                abort();
            }
            if (raw_palette)
                write16(&palette[y*(mode->ncolors-mode->nfixed)+x], c);
            else if (mode->id != 3 && (x == 0 || x == ((mode->ncolors-1) & -16)))
                continue;
            else if (mode->id == 2 && x >= mode->scancolors)
                break;
            else 
            {
                nbits += bpp;
                tmpcol = (tmpcol << bpp) | c;
                if (nbits >= 16) 
                {
                    nbits -= 16;
                    write16(&palette[npal++], tmpcol >> nbits);
                }
            }        
        }

        if (raw_palette && mode->id == 0) 
        {
            /* specific color reordering for mode 0 */
            unsigned short tmp[6];
            memcpy(tmp, palette+y*mode->ncolors+48, 6*2);
            memmove(palette+y*mode->ncolors+22, palette+y*mode->ncolors+16, 32*2);
            memcpy(palette+y*mode->ncolors+16, tmp, 6*2);
        }
    }

    if (raw_palette) 
    {
        unsigned short blackcolor2 = (mode->scancolors - 1) & 0xFFF0;

        for (y = 1 ; y < mode->height ; y++)
        {
            unsigned short* scanline = &palette[y*(mode->ncolors-mode->nfixed)];


            fwrite(&scanline[1],  2, blackcolor2 - 1, fd);
            fwrite(&scanline[blackcolor2 + 1], 2, mode->scancolors - blackcolor2 - 1, fd);
        }

        fwrite(bitmap, 2, (mode->width/4)*(mode->height-1), fd);
    }
    else 
    {
        if (nbits != 0)
            write16(&palette[npal++], tmpcol << (16-nbits));

        fwrite(palette, 2, npal, fd);
        fwrite(bitmap, 2, (mode->width/4)*(mode->height-1), fd);
    }

    free(bitmap);
    free(palette);
    free(pixels);
}

// optlevel 0 to 9
int bmp2mpp(const char* filename, int _mode, int ste, int extra, int doubl, int optlevel, int randseed, const char* outfilename, int raw_palette) 
{
    char header[14+20];
    FILE *fd;
    int bits;
    const Mode *mode = &modes[0];
    int width, height, offset, bpp, compression;
    int err = 1;
    unsigned char *bmp;


    if (_mode == 2)
    {
        ste = 1;
    }

    if ( randseed == 0xFFFFFFFF )
    {
        randseed = 42;
    }

    mode = &modes[_mode];
    
    assert (!(extra && doubl));
    assert(!(extra && !ste));

    fd = fopen(filename, "rb");
    if (!fd) 
    {
        perror(filename);
        return 1;
    }

    /* Analyze file header */
    fread(header, 14+20, 1, fd);
    if (strncmp(header, "BM", 2))
        return file_error(filename, "must be in BMP format");

    offset = readle4(header+10);
    width = readle4(header+18);
    height = readle4(header+22);
    bpp = readle2(header+28);
    compression = readle4(header+30);
    
    if (bpp != 24 || compression != 0)
        return file_error(filename, "BMP format must be uncompressed 24-bit");
    
    if (width != mode->width || height != mode->height) 
    {
        char buf[256];
        sprintf(buf, "image size must be %dx%d", mode->width, mode->height);
        return file_error(filename, buf);
    }

    bmp = malloc(mode->width*mode->height*3);
    MEM_ERROR(bmp);
    fseek(fd, offset, SEEK_SET);
    fread(&bmp[0], 3, mode->width*mode->height, fd);
    fclose(fd);

    fd = fopen(outfilename, "wb");
    if (fd == NULL) 
    {
        perror(outfilename);
        return 1;
    }

    srand(randseed);
    bits = 3 + ste + extra;
    
    if (doubl) 
    {
        convert(fd, bmp, mode, bits, 1, optlevel, raw_palette, err);
        convert(fd, bmp, mode, bits, 2, optlevel, raw_palette, err);
    }
    else
    {
        convert(fd, bmp, mode, bits, 0, optlevel, raw_palette, err);
    }

    fclose(fd);

    printf("Successfully written to output file `%s'.\n", outfilename);

    free(bmp);

    return 0;
}
