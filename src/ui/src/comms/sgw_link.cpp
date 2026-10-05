/* src/ui/src/comms/sgw_link.cpp -- the game's side of the Serious Games Week matchmaker (EPIC-MATCHMAKER, 2026-10-03).
 *
 * The matchmaker is an iGOR-style website the PLAYER chooses ($SGW_URL or ~/.config/sgw/url). The game talks to
 * it through the `sgw` client (serious-games-week/sgw.py), never directly:
 *   hosting  -> `sgw announce --game <id> --port <udp> --title <session>` runs for as long as the session; its stdin
 *               is a pipe the game holds, so the listing is withdrawn when the session closes or the game exits.
 *               The matchmaker refuses a game outside the player's day-of-week category (sgw exits 3); the game
 *               still hosts on the LAN, and the refusal is logged.
 *   joining  -> `sgw list --game <id>` gives "host port players title" lines; the session search probes each host.
 * The list is refreshed in a background thread every 10 s, so the menus never wait on the network.
 * Shared verbatim with MA and BoB (their SRC/compat/sgw_link.cpp); FreeFalcon feeds listed hosts into the Comms
 * phonebook (phonebk.cpp) and announces a listening player (uicomms.cpp). The game id is a parameter.
 * Finding sgw: $SGW_BIN, else $APPDIR/usr/bin/sgw (AppImage), else ~/serious-games-week/sgw.py, else `sgw` on PATH.
 * SGW_OFF=1 disables everything here. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>

static const char* sgw_bin(void)
{
	static char path[512] = "";
	if (path[0]) return path;
	/* access(), not stat(): these ports build with -fpack-struct=1, so a `struct stat` here has a different
	   layout from libc's and stat() overruns it (measured: "stack smashing detected" in sgw_bin) */
	const char* e = getenv("SGW_BIN");
	if (e && *e) snprintf(path, sizeof path, "%s", e);
	else if (getenv("APPDIR") && (snprintf(path, sizeof path, "%s/usr/bin/sgw", getenv("APPDIR")), access(path, X_OK) == 0)) {}
	else if (getenv("HOME") && (snprintf(path, sizeof path, "%s/serious-games-week/sgw.py", getenv("HOME")), access(path, R_OK) == 0)) {}
	else snprintf(path, sizeof path, "sgw");
	return path;
}

extern "C" int sgw_configured(void)
{
	if (getenv("SGW_OFF")) return 0;
	if (getenv("SGW_URL") && *getenv("SGW_URL")) return 1;
	char p[512];
	snprintf(p, sizeof p, "%s/.config/sgw/url", getenv("HOME") ? getenv("HOME") : "");
	FILE* f = fopen(p, "r");
	if (!f) {	/* read-only fallback for setups made before 2026-10-05 */
		snprintf(p, sizeof p, "%s/.config/sgweek/url", getenv("HOME") ? getenv("HOME") : "");
		f = fopen(p, "r");
	}
	if (!f) return 0;
	int c = fgetc(f); fclose(f);
	return c != EOF && c != '\n';
}

static void shq(char* out, size_t n, const char* s)	/* single-quote for the shell */
{
	size_t k = 0; if (k < n - 1) out[k++] = '\'';
	for (; *s && k < n - 6; s++) { if (*s == '\'') { memcpy(out + k, "'\\''", 4); k += 4; } else out[k++] = *s; }
	if (k < n - 1) out[k++] = '\''; out[k] = 0;
}

static FILE* g_announce = NULL;

extern "C" void sgw_announce_start(const char* game, int port, const char* title)
{
	if (g_announce || !sgw_configured())
	{
		if (getenv("SGW_TRACE")) fprintf(stderr, "[sgw] not announcing: %s\n", g_announce ? "already listed" : "no matchmaker configured");
		return;
	}
	char t[256], b[600], cmd[1200];
	shq(t, sizeof t, title && *title ? title : game);
	shq(b, sizeof b, sgw_bin());
	snprintf(cmd, sizeof cmd, "%s announce --game %s --port %d --title %s 2>&1 | sed 's/^/[sgw] /' >&2", b, game, port, t);
	g_announce = popen(cmd, "w");
	fprintf(stderr, "[sgw] announcing %s session \"%s\" on UDP %d via %s (%s)\n", game, title ? title : "", port, sgw_bin(), g_announce ? "started" : "failed");
}

extern "C" void sgw_announce_stop(void)
{
	if (!g_announce) return;
	pclose(g_announce);	/* closes sgw's stdin: it withdraws the listing and exits */
	g_announce = NULL;
	fprintf(stderr, "[sgw] session withdrawn from the matchmaker\n");
}

struct SgwHost { struct sockaddr_in addr; char title[96]; };
static SgwHost g_hosts[32]; static int g_nhosts = 0;
static pthread_mutex_t g_mx = PTHREAD_MUTEX_INITIALIZER;
static char g_game[16] = "";
static int g_thread = 0;

static void* sgw_refresher(void*)
{
	for (;;) {
		char b[600], cmd[800];
		shq(b, sizeof b, sgw_bin());
		snprintf(cmd, sizeof cmd, "%s list --game %s 2>/dev/null", b, g_game);
		FILE* f = popen(cmd, "r");
		SgwHost tmp[32]; int n = 0;
		if (f) {
			char line[256];
			while (n < 32 && fgets(line, sizeof line, f)) {
				char host[128]; int port = 0, players = 0, off = 0;
				if (sscanf(line, "%127s %d %d %n", host, &port, &players, &off) < 3 || port <= 0) continue;
				struct addrinfo hints, *ai = NULL; memset(&hints, 0, sizeof hints); hints.ai_family = AF_INET;
				if (getaddrinfo(host, NULL, &hints, &ai) != 0 || !ai) continue;
				memset(&tmp[n], 0, sizeof tmp[n]);
				tmp[n].addr = *(struct sockaddr_in*)ai->ai_addr; tmp[n].addr.sin_port = htons((unsigned short)port);
				freeaddrinfo(ai);
				snprintf(tmp[n].title, sizeof tmp[n].title, "%s", line + off);
				char* nl = strchr(tmp[n].title, '\n'); if (nl) *nl = 0;
				n++;
			}
			pclose(f);
		}
		pthread_mutex_lock(&g_mx);
		memcpy(g_hosts, tmp, sizeof(SgwHost) * n); g_nhosts = n;
		pthread_mutex_unlock(&g_mx);
		if (getenv("SGW_TRACE")) fprintf(stderr, "[sgw] %d %s session(s) listed by the matchmaker\n", n, g_game);
		sleep(10);
	}
	return NULL;
}

/* the hosts the matchmaker lists for this game right now (cached; refresh runs in the background) */
extern "C" int sgw_hosts(const char* game, struct sockaddr_in* out, int max)
{
	if (!sgw_configured()) return 0;
	if (!g_thread) {
		snprintf(g_game, sizeof g_game, "%s", game);
		pthread_t t; if (pthread_create(&t, NULL, sgw_refresher, NULL) == 0) { pthread_detach(t); g_thread = 1; }
	}
	pthread_mutex_lock(&g_mx);
	int n = g_nhosts < max ? g_nhosts : max;
	for (int i = 0; i < n; i++) out[i] = g_hosts[i].addr;
	pthread_mutex_unlock(&g_mx);
	return n;
}
