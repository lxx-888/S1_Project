
int htoi(char *);
int decode(char *);
int hex_string_dec(char *str, unsigned int *result);
int set_complete(char *title, char *url);
int streamer_status(int alive);
int upgrade_headerfail(char *title, char *url);
int upgrade_crcfail(char *title, char *url);
int gotopage(char *title, char *url);
int ok_page(char *title, char *url, char *str);
int goto_parent_page(char *title, char *url);

//#define dbg(format, arg...) fprintf(stderr,"%s: " format "\n", __FILE__, ##arg)

typedef struct entry
{
    char          pkey[50];
    char          pval[50];
    struct entry *pnext;
} ENTRY, *PENTRY;
