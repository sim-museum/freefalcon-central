/* ff_acmitext.cpp -- Tacview TEXT ACMI written alongside FreeFalcon's own recording (REPLAY-LAB-1 S3).
 *
 * PO: "whenever an ff session is recorded, have it output both its usual file, and a file in the tacview text
 * format used by ma and BoB". FreeFalcon's recorder writes binary .flt records that are imported into a
 * TAPEnnnn.vhs; this file tees the same position records, as they are written, into acmibin/acmiNNNN.txt.acmi, and
 * the import renames it to TAPEnnnn.txt.acmi so the pair travel together. The .flt / .vhs path is untouched.
 *
 * FORMAT -- the same as MiG Alley's and Battle of Britain's exports (Tacview 2.2 text, transform syntax #4):
 *   T=Lon|Lat|Alt|Roll|Pitch|Yaw|U|V|Heading
 *   Lon/Lat  from the sim's own ApproxLatLong() (what the UFC shows), as offsets from the theatre origin, which
 *            is written as ReferenceLongitude/ReferenceLatitude
 *   Alt      -z, metres
 *   U/V      the theatre's flat metres, east (sim y) and north (sim x) -- so sorties in one theatre share a frame
 *   angles   degrees; FreeFalcon's yaw is already clockwise from north (x north, y east, z down), as Tacview's is
 * Objects: aircraft (Type=Air+FixedWing, Name = the vehicle class name, CallSign = the drawn label, Color from the
 * team, Pilot=Player for the player's jet) and missiles (Type=Weapon+Missile). Commas in values are escaped (\,),
 * as the format requires.
 *
 * FF_ACMI_TEXT=0 disables it.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <map>
#include <string>

extern float FALCON_ORIGIN_LAT, FALCON_ORIGIN_LONG;
void ApproxLatLong(float x, float y, float* latitude, float* longitude);
char* GetClassName(int ID);
extern int NumEntities;
extern "C" FILE* fopen_nocase(const char* filepath, const char* mode);   /* compat: case-insensitive paths */

static FILE* g_txt = NULL;
static char g_txt_path[512];
static double g_lastT = -1e300;
static long g_samples = 0;
static std::map<long, int> g_seen;          // ACMI id -> 1 once its static properties have been written

static const double FT2M = 0.3048;
static const double R2D = 57.29577951308232;

static void esc(char* out, size_t n, const char* s)
{
    size_t k = 0;
    for (; s and *s and k + 2 < n; s++) {
        if (*s == ',' or *s == '\\') out[k++] = '\\';
        if (*s == '\r' or *s == '\n') continue;
        out[k++] = *s;
    }
    out[k] = 0;
}

extern "C" int ff_acmitext_active(void) { return g_txt != NULL; }

/* has this ACMI id's static description been written? (the recorder looks names up only on first sighting) */
int ff_acmitext_known(long uid) { return g_txt == NULL or g_seen.count(uid) != 0; }

/* fltname: the .flt the recorder just opened, e.g. "acmibin\\acmi0003.flt" (backslashes as the game writes them) */
void ff_acmitext_start(const char* fltname)
{
    const char* e = getenv("FF_ACMI_TEXT");
    if ((e and e[0] == '0') or g_txt)
        return;
    snprintf(g_txt_path, sizeof g_txt_path, "%s", fltname);
    for (char* p = g_txt_path; *p; p++) if (*p == '\\') *p = '/';
    char* dot = strrchr(g_txt_path, '.');
    if (dot) *dot = 0;
    strncat(g_txt_path, ".txt.acmi", sizeof g_txt_path - strlen(g_txt_path) - 1);
    g_txt = fopen_nocase(g_txt_path, "wb");
    if (not g_txt) {
        fprintf(stderr, "[acmitext] cannot write %s\n", g_txt_path);
        return;
    }
    g_lastT = -1e300;
    g_samples = 0;
    g_seen.clear();
    fprintf(g_txt, "FileType=text/acmi/tacview\r\nFileVersion=2.2\r\n");
    fprintf(g_txt, "0,ReferenceTime=2000-01-01T00:00:00Z\r\n");
    fprintf(g_txt, "0,ReferenceLongitude=%.6f\r\n", (double)FALCON_ORIGIN_LONG);
    fprintf(g_txt, "0,ReferenceLatitude=%.6f\r\n", (double)FALCON_ORIGIN_LAT);
    fprintf(g_txt, "0,DataSource=FreeFalcon 6 (Linux port)\r\n");
    fprintf(g_txt, "0,DataRecorder=ff_acmitext (REPLAY-LAB-1)\r\n");
    fprintf(g_txt, "0,Title=FreeFalcon sortie\r\n");
    fprintf(stderr, "[acmitext] writing %s (origin %.4f/%.4f)\n", g_txt_path, (double)FALCON_ORIGIN_LAT,
            (double)FALCON_ORIGIN_LONG);
}

void ff_acmitext_stop(void)
{
    if (not g_txt)
        return;
    fclose(g_txt);
    g_txt = NULL;
    fprintf(stderr, "[acmitext] closed %s (%ld object samples)\n", g_txt_path, g_samples);
}

static const char* team_colour(long color)
{
    /* TeamInfo colours are indices into the sim's TeamSimColorList (addobj.cpp); these are its own names */
    static const char* names[8] = {"White", "Green", "Blue", "Brown", "Yellow", "Orange", "Red", "Black"};
    return (color >= 0 and color < 8) ? names[color] : "Grey";
}

/* the text file samples aircraft more often than the .flt (every 16th sim frame, ~2 Hz): every Nth frame,
   FF_ACMI_TEXT_DIV (default 3, ~10 Hz at the sim's ~29 Hz) -- touchdown sink rate needs it */
int ff_acmitext_div(void)
{
    static int div = -1;
    if (div < 0) {
        const char* e = getenv("FF_ACMI_TEXT_DIV");
        div = (e and atoi(e) > 0) ? atoi(e) : 3;
    }
    return div;
}

/* kind: 'A' aircraft, 'M' missile. time: the record's header time, seconds. */
void ff_acmitext_pos(char kind, double time, long uid, int type, float x, float y, float z, float yaw, float pitch,
                     float roll, const char* label, long color, int is_player)
{
    if (not g_txt)
        return;
    if (time != g_lastT) {
        fprintf(g_txt, "#%.3f\r\n", time);   /* ms: 10 ms rounding was a few %% of speed at 10 Hz */
        g_lastT = time;
    }
    float lat = 0, lon = 0;
    ApproxLatLong(x, y, &lat, &lon);                          /* radians, the sim's own conversion */
    double hdg = fmod((double)yaw * R2D + 360.0, 360.0);
    fprintf(g_txt, "%lx,T=%.7f|%.7f|%.2f|%.2f|%.2f|%.2f|%.2f|%.2f|%.2f", uid + 1,
            (double)lon * R2D - FALCON_ORIGIN_LONG, (double)lat * R2D - FALCON_ORIGIN_LAT, -(double)z * FT2M,
            (double)roll * R2D, (double)pitch * R2D, hdg, (double)y * FT2M, (double)x * FT2M, hdg);
    if (not g_seen.count(uid)) {                              /* static properties once per object */
        g_seen[uid] = 1;
        char nm[128] = "", cs[64] = "";
        int ci = type - 100;   /* VU_LAST_ENTITY_TYPE: class-table index = entity type - 100 */
        if (ci >= 0 and ci < NumEntities) {
            const char* n = GetClassName(ci);
            if (n) esc(nm, sizeof nm, n);
        }
        if (label and *label) esc(cs, sizeof cs, label);
        fprintf(g_txt, ",Type=%s", kind == 'M' ? "Weapon+Missile" : "Air+FixedWing");
        if (*nm) fprintf(g_txt, ",Name=%s", nm);
        if (*cs) fprintf(g_txt, ",CallSign=%s", cs);
        if (kind == 'A') fprintf(g_txt, ",Color=%s", team_colour(color));
        if (is_player) fprintf(g_txt, ",Pilot=Player");
    }
    fprintf(g_txt, "\r\n");
    g_samples++;
}

/* the import made TAPEnnnn.vhs from acmiNNNN.flt: rename the text file to match */
void ff_acmitext_rename_for_tape(const char* fltname, const char* vhsname)
{
    char from[512], to[512];
    snprintf(from, sizeof from, "%s", fltname);
    snprintf(to, sizeof to, "%s", vhsname);
    for (char* p = from; *p; p++) if (*p == '\\') *p = '/';
    for (char* p = to; *p; p++) if (*p == '\\') *p = '/';
    char* d = strrchr(from, '.'); if (d) *d = 0;
    d = strrchr(to, '.'); if (d) *d = 0;
    strncat(from, ".txt.acmi", sizeof from - strlen(from) - 1);
    strncat(to, ".txt.acmi", sizeof to - strlen(to) - 1);
    if (rename(from, to) == 0)
        fprintf(stderr, "[acmitext] %s -> %s\n", from, to);
}
