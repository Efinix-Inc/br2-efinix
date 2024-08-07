/**
 * Useful links:
 *    http://www.asciitable.com
 *    https://en.wikipedia.org/wiki/ANSI_escape_code
 */

#include "evsoc_def.h"
#include "evsoc_cli.h"

#define CTRL_R 0x12

#define CLEAR_EOL "\x1b[0K"
#define MOVE_BOL "\x1b[1G"

using namespace std;

std::vector<st_cli_entry *> g_cmd_vect;
std::map<std::string, int> g_enum_map;
static struct embedded_cli cli;

void reg_cmd(const char *pName, func_call * pCall, const char * pPara, const char * pDesp)
{
    st_cli_entry * lPtr = new st_cli_entry;
    lPtr->mCmd = pName;
    lPtr->mFunPtr = pCall;
    lPtr->mPara = pPara;
    lPtr->mDesp = pDesp;
    g_cmd_vect.push_back(lPtr);
}

void reg_enum(const char *pName, int pVal)
{
    g_enum_map[pName] = pVal;
}

bool get_enum(const char *pName, int & pInt)
{
    auto lLtr = g_enum_map.find(pName);
    if (lLtr == g_enum_map.end()){
        return false;
    }
    pInt = lLtr->second;
    return true;
}

bool get_int(const char *pName, int & pInt)
{
    if (pName == NULL)
    {
        return false;
    }

    int lLen = strlen(pName);
    if (lLen <= 0)
    {
        return false;
    }

    for (int lTmpI = 0; lTmpI < lLen; lTmpI++){
        if (('0' <= pName[lTmpI]) && ('9' >= pName[lTmpI])){
            continue;
        }

        if (('A' <= pName[lTmpI]) && ('F' >= pName[lTmpI])){
            continue;
        }

        if (('a' <= pName[lTmpI]) && ('f' >= pName[lTmpI])){
            continue;
        }        

        return false;
    }

    if (sscanf(pName, "%d", &pInt) <= 0)
    {
        return true;
    }

    return true;
}


void clearAll()
{
    for (auto lPtr = g_cmd_vect.begin(); lPtr != g_cmd_vect.end(); ++lPtr)
    {
        delete *lPtr;
    }
    g_cmd_vect.clear();
    g_enum_map.clear();
}


static char getch(void)
{
    char buf = 0;
    bool lError = false;
    struct termios old = {0};
    if (tcgetattr(0, &old) < 0){
        perror("tcsetattr()");
    }    

    struct termios raw = old;

    // Do what cfmakeraw does (Using --std=c99 means that cfmakeraw isn't
    // available)
    raw.c_iflag &=
        ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    raw.c_oflag &= ~OPOST;
    raw.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    raw.c_cflag &= ~(CSIZE | PARENB);
    raw.c_cflag |= CS8;

    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    
    if (tcsetattr(0, TCSANOW, &raw) < 0){
        // return 0;
        lError = true;
    }
    //    perror("tcsetattr ICANON");

    if (read(0, &buf, 1) < 0){
        lError = true;
    }
    //    perror("read()");

    if (tcsetattr(0, TCSADRAIN, &old) < 0)
    {
        lError = true;
    }
    //    perror("tcsetattr ~ICANON");
    return (buf);
}

static void intHandler(int dummy)
{
    (void)dummy;
    embedded_cli_insert_char(&cli, '\x03');
}

/**
 * This function outputs a single character to stdout, to be used as the
 * callback from embedded cli
 */
static void posix_putch(void *data, char ch, bool is_last)
{
    FILE *fp = (FILE *)data;
    fputc(ch, fp);
    if (is_last)
        fflush(fp);
}

bool cmd_proc(int cli_argc, char **cli_argv)
{
    bool done = false;

    if (cli_argc > 0)
    {
        int  lCmdType = 0;
        char * lCmd = cli_argv[0];

        if  (strcmp(cli_argv[0], "quit") == 0)
        {
            done = true;
        }
        else if (strcmp(cli_argv[0], "help") == 0)
        {
            int lTmpNum = 1;
            for (auto lLtr = g_cmd_vect.begin(); lLtr != g_cmd_vect.end(); ++lLtr)
            {
                printf("%d. %s %s -- %s\n", lTmpNum++, (*lLtr)->mCmd, (*lLtr)->mPara, (*lLtr)->mDesp);
            }
            printf("%d. %s %s -- %s\n", lTmpNum++, "quit", "", "close this app");
        }
        else 
        {
            bool lFind = false;
            for (auto lLtr = g_cmd_vect.begin(); lLtr != g_cmd_vect.end(); ++lLtr){
                if (strcmp( (*lLtr)->mCmd, lCmd) == 0)
                {
                    if (CMD_RET_OK == ( (*lLtr)->mFunPtr)(cli_argc, cli_argv))
                    {
                        printf("  Sucess!\n");
                    }
                    else
                    {
                        printf("  Fail!\n");
                    }
                    lFind = true;
                    break;
                }
            }

            if (!lFind)
            {
                printf("  Error Cmd, 'help' can be used. \n");
            }
        }
    }
    return done;
}

int cli_main(void)
{
    bool done = false;

    printf("-------------\n");
    printf("- EVSOC CLI -\n");    
    printf("-------------\n");    
    
    /**
     * Start up the Embedded CLI instance with the appropriate
     * callbacks/userdata
     */
    embedded_cli_init(&cli, "EVSOC> ", posix_putch, stdout);
    embedded_cli_prompt(&cli);

    /* Capture Ctrl-C */
    signal(SIGINT, intHandler);

    while (!done) {
        char ch = getch();
        if (ch == 0){
            continue;
        }

        if (embedded_cli_insert_char(&cli, ch)) {
            int cli_argc;
            char **cli_argv;
            cli_argc = embedded_cli_argc(&cli, &cli_argv);
            
            if (cli_argc > 0)
            {
                int  lCmdType = 0;
                char * lCmd = cli_argv[0];

                if  (strcmp(cli_argv[0], "quit") == 0)
                {
                    done = true;
                }
                else if (strcmp(cli_argv[0], "help") == 0)
                {
                    int lTmpNum = 1;
                    for (auto lLtr = g_cmd_vect.begin(); lLtr != g_cmd_vect.end(); ++lLtr)
                    {
                        printf("%d. %s %s -- %s\n", lTmpNum++, (*lLtr)->mCmd, (*lLtr)->mPara, (*lLtr)->mDesp);
                    }
                    printf("%d. %s %s -- %s\n", lTmpNum++, "quit", "", "close this app");
                }
                else 
                {
                    bool lFind = false;
                    for (auto lLtr = g_cmd_vect.begin(); lLtr != g_cmd_vect.end(); ++lLtr){
                        if (strcmp( (*lLtr)->mCmd, lCmd) == 0)
                        {
                            if (CMD_RET_OK == ( (*lLtr)->mFunPtr)(cli_argc, cli_argv))
                            {
                                printf("  Sucess!\n");
                            }
                            else
                            {
                                printf("  Fail!\n");
                            }
                            lFind = true;
                            break;
                        }
                    }

                    if (!lFind)
                    {
                        printf("  Error Cmd, 'help' can be used. \n");
                    }
                }
            }

            if (!done) {
                embedded_cli_prompt(&cli);
            }
        }
    }
    clearAll();
    return 0;
}

//////////
static void cli_putchar(struct embedded_cli *cli, char ch, bool is_last)
{
    if (cli->put_char) {
#if EMBEDDED_CLI_SERIAL_XLATE
        if (ch == '\n')
            cli->put_char(cli->cb_data, '\r', false);
#endif
        cli->put_char(cli->cb_data, ch, is_last);
    }
}

static void cli_puts(struct embedded_cli *cli, const char *s)
{
    for (; *s; s++)
        cli_putchar(cli, *s, s[1] == '\0');
}

static void embedded_cli_reset_line(struct embedded_cli *cli)
{
    cli->len = 0;
    cli->cursor = 0;
    cli->counter = 0;
    cli->have_csi = cli->have_escape = false;
#if EMBEDDED_CLI_HISTORY_LEN
    cli->history_pos = -1;
    cli->searching = false;
#endif
}

void embedded_cli_init(struct embedded_cli *cli, const char *prompt,
                       void (*put_char)(void *data, char ch, bool is_last),
                       void *cb_data)
{
    memset(cli, 0, sizeof(*cli));
    cli->put_char = put_char;
    cli->cb_data = cb_data;
    if (prompt) {
        strncpy(cli->prompt, prompt, sizeof(cli->prompt));
        cli->prompt[sizeof(cli->prompt) - 1] = '\0';
    }

    embedded_cli_reset_line(cli);
}

static void cli_ansi(struct embedded_cli *cli, int n, char code)
{
    char buffer[5] = {'\x1b', '[', (char)('0' + (n % 10)), code, '\0'};
    cli_puts(cli, buffer);
}

static void term_cursor_back(struct embedded_cli *cli, int n)
{
    while (n > 0) {
        int count = n > 9 ? 9 : n;
        cli_ansi(cli, count, 'D');
        n -= count;
    }
}

static void term_cursor_fwd(struct embedded_cli *cli, int n)
{
    while (n > 0) {
        int count = n > 9 ? 9 : n;
        cli_ansi(cli, count, 'C');
        n -= count;
    }
}

#if EMBEDDED_CLI_HISTORY_LEN
static void term_backspace(struct embedded_cli *cli, int n)
{
    // printf("backspace %d ('%s': %d)\n", n, cli->buffer, cli->done);
    while (n--) {
        cli_putchar(cli, '\b', n == 0);
    }

}

static const char *embedded_cli_get_history_search(struct embedded_cli *cli)
{
    for (int i = 0;; i++) {
        const char *h = embedded_cli_get_history(cli, i);
        if (!h)
            return NULL;
        if (strstr(h, cli->buffer))
            return h;
    }
    return NULL;
}
#endif

static void embedded_cli_insert_default_char(struct embedded_cli *cli,
                                             char ch)
{
    // If the buffer is full, there's nothing we can do
    if (cli->len >= (int)sizeof(cli->buffer) - 1)
        return;
    // Insert a gap in the buffer for the new character
    memmove(&cli->buffer[cli->cursor + 1], &cli->buffer[cli->cursor],
            cli->len - cli->cursor);
    cli->buffer[cli->cursor] = ch;
    cli->len++;
    cli->buffer[cli->len] = '\0';
    cli->cursor++;

#if EMBEDDED_CLI_HISTORY_LEN
    if (cli->searching) {
        cli_puts(cli, MOVE_BOL CLEAR_EOL "search:");
        const char *h = embedded_cli_get_history_search(cli);
        if (h)
            cli_puts(cli, h);

    } else
#endif
    {
        cli_puts(cli, &cli->buffer[cli->cursor - 1]);
        term_cursor_back(cli, cli->len - cli->cursor);
    }
}

const char *embedded_cli_get_history(struct embedded_cli *cli,
                                     int history_pos)
{
#if EMBEDDED_CLI_HISTORY_LEN
    int pos = 0;

    if (history_pos < 0)
        return NULL;

    // Search back through the history buffer for `history_pos` entry
    for (int i = 0; i < history_pos; i++) {
        int len = strlen(&cli->history[pos]);
        if (len == 0)
            return NULL;
        pos += len + 1;
        if (pos == sizeof(cli->history))
            return NULL;
    }

    return &cli->history[pos];
#else
    (void)cli;
    (void)history_pos;
    return NULL;
#endif
}

#if EMBEDDED_CLI_HISTORY_LEN
static void embedded_cli_extend_history(struct embedded_cli *cli)
{
    int len = strlen(cli->buffer);
    if (len > 0) {
        // If the new entry is the same as the most recent history entry,
        // then don't insert it
        if (strcmp(cli->buffer, cli->history) == 0)
            return;
        memmove(&cli->history[len + 1], &cli->history[0],
                sizeof(cli->history) - len + 1);
        memcpy(cli->history, cli->buffer, len + 1);
        // Make sure it's always nul terminated
        cli->history[sizeof(cli->history) - 1] = '\0';
    }
}

static void embedded_cli_stop_search(struct embedded_cli *cli, bool print)
{
    const char *h = embedded_cli_get_history_search(cli);
    if (h) {
        strncpy(cli->buffer, h, sizeof(cli->buffer));
        cli->buffer[sizeof(cli->buffer) - 1] = '\0';
    } else
        cli->buffer[0] = '\0';
    cli->len = cli->cursor = strlen(cli->buffer);
    cli->searching = false;
    if (print) {
        cli_puts(cli, MOVE_BOL CLEAR_EOL);
        cli_puts(cli, cli->prompt);
        cli_puts(cli, cli->buffer);
    }
}
#endif


static int embedded_cli_tab(struct embedded_cli *cli)
{
    if ((cli->len == 0) || (cli->buffer[cli->cursor] == ' '))
    {
        return 1;
    }

    if (cli->cursor != cli->len)
    {
        return 0;
    }

    #define MAT_NUM 63
    static char lMatBuf[MAT_NUM + 1];
    int lMatIdx = 0;
    int lTmpI = 0; 
    bool lHasSp = false;
    for (; lTmpI < cli->len; lTmpI++)
    {
        if (' ' != cli->buffer[lTmpI])
        {
            if (lMatIdx < MAT_NUM)
            {
                lMatBuf[lMatIdx++] = cli->buffer[lTmpI];
            }
        }
        else
        {
            lMatIdx = 0;
            lMatBuf[lMatIdx] = 0;
            lHasSp = true;
        }
    }
    lMatBuf[lMatIdx] = 0;
    int lMatLen = strlen(lMatBuf);
    if (lMatLen == 0)
    {
        return 1;
    }  

    // printf("embedded_cli_tab:%d, %s\n", cli->len, lMatBuf);
    const char * lLMatStr = "";
    int lMatNum = 0;
    #define MAX_MAT_MUN 10
    if (!lHasSp)
    {
        for (auto lLtr = g_cmd_vect.begin(); lLtr != g_cmd_vect.end(); ++lLtr)
        {
            int lCmdLen = strlen((*lLtr)->mCmd);
            if (lMatLen > lCmdLen)
            {
                continue;
            }
            if (strncmp(lMatBuf, (*lLtr)->mCmd, lMatLen) == 0)
            {
                if (cli->mDupComp && (lMatNum == 0)){
                    lLMatStr = (*lLtr)->mCmd;
                } else {
                    if (cli->mDupComp && (lMatNum == 1)){
                        printf("\n%s", lLMatStr);
                    }
                    printf("\n%s", (*lLtr)->mCmd);
                }
                if (++lMatNum > MAX_MAT_MUN)
                {
                    break;
                }
            }
        }
    }
    else
    {
        for (auto lLtr = g_enum_map.begin(); lLtr != g_enum_map.end(); ++lLtr)
        {
            int lCmdLen = strlen(lLtr->first.c_str());
            if (lMatLen > lCmdLen)
            {
                continue;
            }
            if (strncmp(lMatBuf, lLtr->first.c_str(), lMatLen) == 0)
            {
                if (cli->mDupComp && (lMatNum == 0)){
                    lLMatStr = lLtr->first.c_str();
                }
                else {
                    if (cli->mDupComp && (lMatNum == 1)){
                        printf("\n%s", lLMatStr);
                    }
                    printf("\n%s", lLtr->first.c_str());
                }

                if (++lMatNum > MAX_MAT_MUN)
                {
                    break;
                }
            }
        }
    }

    if (lMatNum > 0)
    {
        if ((lMatNum == 1) && cli->mDupComp){
            int lLen = strlen(lLMatStr);
            for (lTmpI = lMatLen; lTmpI < lLen; lTmpI++){
                embedded_cli_insert_default_char(cli, lLMatStr[lTmpI]);
            }
            embedded_cli_insert_default_char(cli, ' ');
            cli->mDupComp = false;
        }
        else{
            printf("\n");
            cli_puts(cli, MOVE_BOL CLEAR_EOL);
            cli_puts(cli, cli->prompt);
            cli_puts(cli, cli->buffer);

            if (lMatNum == 1) {
                cli->mDupComp = true;
            }
            else{
                cli->mDupComp = false;
            }

        }
    }


    return 1;
}

bool embedded_cli_insert_char(struct embedded_cli *cli, char ch)
{
    // If we're inserting a character just after a finished line, clear things
    // up
    if (cli->done) {
        cli->buffer[0] = '\0';
        cli->done = false;
    }
    if (ch != '\t'){
        cli->mDupComp = false;
    }
    // printf("Inserting char %d 0x%x '%c'\n", ch, ch, ch);
    if (cli->have_csi) {
        if (ch >= '0' && ch <= '9' && cli->counter < 100) {
            cli->counter = cli->counter * 10 + ch - '0';
            // printf("cli->counter -> %d\n", cli->counter);
        } else {
            if (cli->counter == 0)
                cli->counter = 1;
            switch (ch) {
            case 'A': { // up arrow
#if EMBEDDED_CLI_HISTORY_LEN
                // Backspace over our current line
                term_backspace(cli, cli->done ? 0 : cli->cursor);
                const char *line =
                    embedded_cli_get_history(cli, cli->history_pos + 1);
                if (line) {
                    int len = strlen(line);
                    cli->history_pos++;
                    // printf("history up %d = '%s'\n", cli->history_pos,
                    // line);
                    strncpy(cli->buffer, line, sizeof(cli->buffer));
                    cli->buffer[sizeof(cli->buffer) - 1] = '\0';
                    cli->len = len;
                    cli->cursor = len;
                    cli_puts(cli, cli->buffer);
                    cli_puts(cli, CLEAR_EOL);
                } else {
                    int tmp = cli->history_pos; // We don't want to wrap this
                                                // history, so retain it
                    cli->buffer[0] = '\0';
                    embedded_cli_reset_line(cli);
                    cli->history_pos = tmp;
                    cli_puts(cli, CLEAR_EOL);
                }
#endif
                break;
            }

            case 'B': { // down arrow
#if EMBEDDED_CLI_HISTORY_LEN
                term_backspace(cli, cli->done ? 0 : cli->cursor);
                const char *line =
                    embedded_cli_get_history(cli, cli->history_pos - 1);
                if (line) {
                    int len = strlen(line);
                    cli->history_pos--;
                    // printf("history down %d = '%s'\n",
                    // cli->history_pos, line);
                    strncpy(cli->buffer, line, sizeof(cli->buffer));
                    cli->buffer[sizeof(cli->buffer) - 1] = '\0';
                    cli->len = len;
                    cli->cursor = len;
                    cli_puts(cli, cli->buffer);
                    cli_puts(cli, CLEAR_EOL);
                } else {
                    cli->buffer[0] = '\0';
                    embedded_cli_reset_line(cli);
                    cli_puts(cli, CLEAR_EOL);
                }
#endif
                break;
            }

            case 'C':
                if (cli->cursor <= cli->len - cli->counter) {
                    cli->cursor += cli->counter;
                    term_cursor_fwd(cli, cli->counter);
                }
                break;
            case 'D':
                // printf("back %d vs %d\n", cli->cursor, cli->counter);
                if (cli->cursor >= cli->counter) {
                    cli->cursor -= cli->counter;
                    term_cursor_back(cli, cli->counter);
                }
                break;
            case 'F':
                term_cursor_fwd(cli, cli->len - cli->cursor);
                cli->cursor = cli->len;
                break;
            case 'H':
                term_cursor_back(cli, cli->cursor);
                cli->cursor = 0;
                break;
            case '~':
                if (cli->counter == 3) { // delete key
                    if (cli->cursor < cli->len) {
                        memmove(&cli->buffer[cli->cursor],
                                &cli->buffer[cli->cursor + 1],
                                cli->len - cli->cursor);
                        cli->len--;
                        cli_puts(cli, &cli->buffer[cli->cursor]);
                        cli_puts(cli, " ");
                        term_cursor_back(cli, cli->len - cli->cursor + 1);
                    }
                }
                break;
            default:
                // TODO: Handle more escape sequences
                break;
            }
            cli->have_csi = cli->have_escape = false;
            cli->counter = 0;
        }
    } else {
        switch (ch) {
        case '\0':
            break;
        case '\x01':
            // Go to the beginning of the line
            term_cursor_back(cli, cli->cursor);
            cli->cursor = 0;
            break;
        case '\x03':
            cli_puts(cli, "^C\n");
            cli_puts(cli, cli->prompt);
            embedded_cli_reset_line(cli);
            cli->buffer[0] = '\0';
            break;
        case '\x05': // Ctrl-E
            term_cursor_fwd(cli, cli->len - cli->cursor);
            cli->cursor = cli->len;
            break;
        case '\x0b': // Ctrl-K
            cli_puts(cli, CLEAR_EOL);
            cli->buffer[cli->cursor] = '\0';
            cli->len = cli->cursor;
            break;
        case '\x0c': // Ctrl-L
            cli_puts(cli, MOVE_BOL CLEAR_EOL);
            cli_puts(cli, cli->prompt);
            cli_puts(cli, cli->buffer);
            term_cursor_back(cli, cli->len - cli->cursor);
            break;
        case '\b': // Backspace
        case 0x7f: // backspace?
                   // printf("backspace %d\n", cli->cursor);
#if EMBEDDED_CLI_HISTORY_LEN
            if (cli->searching)
                embedded_cli_stop_search(cli, true);
#endif
            if (cli->cursor > 0) {
                memmove(&cli->buffer[cli->cursor - 1],
                        &cli->buffer[cli->cursor],
                        cli->len - cli->cursor + 1);
                cli->cursor--;
                cli->len--;
                term_cursor_back(cli, 1);
                cli_puts(cli, &cli->buffer[cli->cursor]);
                cli_puts(cli, " ");
                term_cursor_back(cli, cli->len - cli->cursor + 1);
            }
            break;
        case CTRL_R:
#if EMBEDDED_CLI_HISTORY_LEN
            if (!cli->searching) {
                cli_puts(cli, "\nsearch:");
                cli->searching = true;
            }
#endif
            break;
        case '\x1b':
#if EMBEDDED_CLI_HISTORY_LEN
            if (cli->searching)
                embedded_cli_stop_search(cli, true);
#endif
            cli->have_csi = false;
            cli->have_escape = true;
            cli->counter = 0;
            break;
        case '[':
            if (cli->have_escape)
                cli->have_csi = true;
            else
                embedded_cli_insert_default_char(cli, ch);
            break;
#if EMBEDDED_CLI_SERIAL_XLATE
        case '\r':
            ch = '\n'; // So cli->done will exit
#endif
            // fallthrough
        case '\n':
            cli_putchar(cli, '\n', true);
            break;
        default:
            if (ch > 0) {
                embedded_cli_insert_default_char(cli, ch);
            }
        }
    }
    cli->done = (ch == '\n');

    if (cli->done) {
#if EMBEDDED_CLI_HISTORY_LEN
        if (cli->searching)
            embedded_cli_stop_search(cli, false);
        embedded_cli_extend_history(cli);
#endif
        embedded_cli_reset_line(cli);
    }
    // printf("Done with char 0x%x (done=%d)\n", ch, cli->done);
    return cli->done;
}

const char *embedded_cli_get_line(const struct embedded_cli *cli)
{
    if (!cli->done)
        return NULL;
    return cli->buffer;
}

static bool is_whitespace(char ch)
{
    return (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r');
}

int embedded_cli_argc(struct embedded_cli *cli, char ***argv)
{
    int pos = 0;
    bool in_arg = false;
    bool in_escape = false;
    char in_string = '\0';
    if (!cli->done)
        return 0;
    for (size_t i = 0; i < sizeof(cli->buffer) && cli->buffer[i] != '\0';
         i++) {

        // If we're escaping this character, just absorb it regardless
        if (in_escape) {
            in_escape = false;
            continue;
        }

        if (in_string) {
            // If we're finishing a string, blank it out
            if (cli->buffer[i] == in_string) {
                memmove(&cli->buffer[i], &cli->buffer[i + 1],
                        sizeof(cli->buffer) - i - 1);
                in_string = '\0';
                i--;
            }
            continue;
        }

        // Skip over whitespace, and replace it with nul terminators so
        // each argv is nul terminated
        if (is_whitespace(cli->buffer[i])) {
            if (in_arg)
                cli->buffer[i] = '\0';
            in_arg = false;
            continue;
        }

        if (!in_arg) {
            if (pos >= EMBEDDED_CLI_MAX_ARGC) {
                break;
            }
            cli->argv[pos] = &cli->buffer[i];
            pos++;
            in_arg = true;
        }

        if (cli->buffer[i] == '\\') {
            // Absorb the escape character
            memmove(&cli->buffer[i], &cli->buffer[i + 1],
                    sizeof(cli->buffer) - i - 1);
            i--;
            in_escape = true;
        }

        // If we're starting a new string, absorb the character and shuffle
        // things back
        if (cli->buffer[i] == '\'' || cli->buffer[i] == '"') {
            in_string = cli->buffer[i];
            memmove(&cli->buffer[i], &cli->buffer[i + 1],
                    sizeof(cli->buffer) - i - 1);
            i--;
        }
    }
    // Traditionally, there is a NULL entry at argv[argc].
    if (pos >= EMBEDDED_CLI_MAX_ARGC) {
        pos--;
    }
    cli->argv[pos] = NULL;

    *argv = cli->argv;
    return pos;
}

void embedded_cli_prompt(struct embedded_cli *cli)
{
    cli_puts(cli, cli->prompt);
}