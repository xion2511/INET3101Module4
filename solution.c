#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define SEATS   24
#define NAMELEN 20

static const char *DATAFILE = "colossus_data.txt";

typedef struct {
    int  id;                 /* 1..24 */
    int  assigned;           /* 0 empty, 1 taken */
    char first[NAMELEN];
    char last[NAMELEN];
} Seat;

typedef struct {
    Seat seat[SEATS];
} Flight;

/* prototypes */
void init_flight(Flight *f);
void clear_line(void);
int  count_empty(const Flight *f);
void show_num_empty(const Flight *f);
void show_empty_list(const Flight *f);
int  name_cmp(const Seat *a, const Seat *b);
void show_alpha_list(const Flight *f);

int  assign_seat(Flight *f);              /* returns 1 if changed */
int  delete_seat(Flight *f);              /* returns 1 if changed */
int  second_menu(Flight *f, const char *title);  /* returns 1 if changed */

int  save_data(const char *path, const Flight *outb, const Flight *inb);
int  load_data(const char *path, Flight *outb, Flight *inb);

/* -------- core helpers -------- */

void init_flight(Flight *f) {
    for (int i = 0; i < SEATS; i++) {
        f->seat[i].id = i + 1;
        f->seat[i].assigned = 0;
        f->seat[i].first[0] = '\0';
        f->seat[i].last[0]  = '\0';
    }
}

void clear_line(void) {
    int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
}

int count_empty(const Flight *f) {
    int c = 0;
    for (int i = 0; i < SEATS; i++) if (!f->seat[i].assigned) c++;
    return c;
}

void show_num_empty(const Flight *f) {
    printf("Empty seats: %d of %d\n\n", count_empty(f), SEATS);
}

void show_empty_list(const Flight *f) {
    int any = 0;
    printf("Empty seat numbers: ");
    for (int i = 0; i < SEATS; i++) {
        if (!f->seat[i].assigned) { printf("%d ", f->seat[i].id); any = 1; }
    }
    if (!any) printf("(none)");
    printf("\n\n");
}

/* compare by last, then first */
int name_cmp(const Seat *a, const Seat *b) {
    int c = strcmp(a->last, b->last);
    if (c == 0) c = strcmp(a->first, b->first);
    return c;
}

/* selection sort over indices so seat IDs don't move */
void show_alpha_list(const Flight *f) {
    int idx[SEATS], n = 0;
    for (int i = 0; i < SEATS; i++) if (f->seat[i].assigned) idx[n++] = i;

    printf("Alphabetical list of assigned seats:\n");
    if (n == 0) { puts("(none)\n"); return; }

    for (int i = 0; i < n - 1; i++) {
        int m = i;
        for (int j = i + 1; j < n; j++)
            if (name_cmp(&f->seat[idx[j]], &f->seat[idx[m]]) < 0) m = j;
        if (m != i) { int t = idx[i]; idx[i] = idx[m]; idx[m] = t; }
    }
    for (int i = 0; i < n; i++) {
        int k = idx[i];
        printf("Seat %2d: %s %s\n", f->seat[k].id, f->seat[k].first, f->seat[k].last);
    }
    printf("\n");
}

/* 0 cancels; returns 1 when a seat changes */
int assign_seat(Flight *f) {
    int seatno;
    char first[NAMELEN], last[NAMELEN];

    printf("\nAssign a customer to a seat (0 = cancel)\n");
    show_empty_list(f);
    if (count_empty(f) == 0) { puts("No empty seats.\n"); return 0; }

    printf("Seat number (1-%d, 0=cancel): ", SEATS);
    if (scanf("%d", &seatno) != 1) { clear_line(); puts("Invalid input.\n"); return 0; }
    clear_line();
    if (seatno == 0) { puts("Assignment canceled.\n"); return 0; }
    if (seatno < 1 || seatno > SEATS) { puts("Seat out of range.\n"); return 0; }
    if (f->seat[seatno - 1].assigned) { printf("Seat %d is already taken.\n\n", seatno); return 0; }

    printf("First name (no spaces): ");
    if (scanf("%19s", first) != 1) { clear_line(); puts("Invalid input.\n"); return 0; }
    clear_line();
    printf("Last name (no spaces): ");
    if (scanf("%19s", last) != 1) { clear_line(); puts("Invalid input.\n"); return 0; }
    clear_line();

    strcpy(f->seat[seatno - 1].first, first);
    strcpy(f->seat[seatno - 1].last,  last);
    f->seat[seatno - 1].assigned = 1;

    printf("Assigned seat %d to %s %s.\n\n", seatno, first, last);
    return 1;
}

/* 0 cancels; returns 1 when a seat changes */
int delete_seat(Flight *f) {
    int seatno; char ans;

    printf("\nDelete a seat assignment (0 = cancel)\n");
    printf("Seat number to clear (1-%d, 0=cancel): ", SEATS);
    if (scanf("%d", &seatno) != 1) { clear_line(); puts("Invalid input.\n"); return 0; }
    clear_line();
    if (seatno == 0) { puts("Delete canceled.\n"); return 0; }
    if (seatno < 1 || seatno > SEATS) { puts("Seat out of range.\n"); return 0; }
    if (!f->seat[seatno - 1].assigned) { printf("Seat %d is already empty.\n\n", seatno); return 0; }

    printf("Confirm delete for seat %d (%s %s)? (y/n): ",
           seatno, f->seat[seatno - 1].first, f->seat[seatno - 1].last);
    if (scanf(" %c", &ans) != 1) { clear_line(); puts("Invalid input.\n"); return 0; }
    clear_line();
    if (tolower((unsigned char)ans) != 'y') { puts("No changes made.\n"); return 0; }

    f->seat[seatno - 1].assigned = 0;
    f->seat[seatno - 1].first[0] = '\0';
    f->seat[seatno - 1].last[0]  = '\0';
    puts("Seat cleared.\n");
    return 1;
}

/* loop until user returns; track if anything changed */
int second_menu(Flight *f, const char *title) {
    char choice;
    int changed = 0;

    for (;;) {
        printf("\nSecond Level Menu – %s\n", title);
        puts("a) Show number of empty seats");
        puts("b) Show list of empty seats");
        puts("c) Show alphabetical list of seats");
        puts("d) Assign a customer to a seat assignment");
        puts("e) Delete a seat assignment");
        puts("f) Return to Main menu");
        printf("Enter choice: ");

        if (scanf(" %c", &choice) != 1) { clear_line(); puts("Invalid input.\n"); continue; }
        clear_line();
        choice = (char)tolower((unsigned char)choice);

        if      (choice == 'a') show_num_empty(f);
        else if (choice == 'b') show_empty_list(f);
        else if (choice == 'c') show_alpha_list(f);
        else if (choice == 'd') changed |= assign_seat(f);
        else if (choice == 'e') changed |= delete_seat(f);
        else if (choice == 'f') return changed;
        else puts("Invalid choice.\n");
    }
}

/* -------- file I/O (simple readable text) -------- */
static int save_one(FILE *fp, const char *tag, const Flight *fl) {
    fprintf(fp, "%s\n", tag);
    for (int i = 0; i < SEATS; i++) {
        const Seat *s = &fl->seat[i];
        const char *fn = s->assigned ? s->first : "-";
        const char *ln = s->assigned ? s->last  : "-";
        if (fprintf(fp, "%d %d %s %s\n", s->id, s->assigned, fn, ln) < 0) return 0;
    }
    return 1;
}

static int load_one(FILE *fp, const char *expect_tag, Flight *fl) {
    char tag[32];
    if (fscanf(fp, " %31s", tag) != 1) return 0;
    if (strcmp(tag, expect_tag) != 0)  return 0;

    for (int i = 0; i < SEATS; i++) {
        int id, assigned;
        char first[NAMELEN], last[NAMELEN];
        if (fscanf(fp, " %d %d %19s %19s", &id, &assigned, first, last) != 4) return 0;
        fl->seat[i].id = id;
        fl->seat[i].assigned = assigned ? 1 : 0;
        if (fl->seat[i].assigned) {
            strncpy(fl->seat[i].first, first, NAMELEN-1);  fl->seat[i].first[NAMELEN-1] = '\0';
            strncpy(fl->seat[i].last,  last,  NAMELEN-1);  fl->seat[i].last[NAMELEN-1]  = '\0';
        } else {
            fl->seat[i].first[0] = '\0';
            fl->seat[i].last[0]  = '\0';
        }
    }
    return 1;
}

int save_data(const char *path, const Flight *outb, const Flight *inb) {
    FILE *fp = fopen(path, "w");
    if (!fp) { perror("fopen"); return 0; }
    if (fprintf(fp, "COLOSSUS_V1\n") < 0) { fclose(fp); return 0; }
    int ok = save_one(fp, "OUTBOUND", outb) && save_one(fp, "INBOUND", inb);
    if (fclose(fp) != 0) ok = 0;
    return ok;
}

int load_data(const char *path, Flight *outb, Flight *inb) {
    FILE *fp = fopen(path, "r");
    if (!fp) return 0;   /* start fresh if no file */

    char header[32];
    if (fscanf(fp, " %31s", header) != 1 || strcmp(header, "COLOSSUS_V1") != 0) {
        fclose(fp); return 0;
    }
    int ok = load_one(fp, "OUTBOUND", outb) && load_one(fp, "INBOUND", inb);
    fclose(fp);
    return ok;
}

/* -------- main -------- */
int main(void) {
    Flight outbound, inbound;
    char choice;

    init_flight(&outbound);
    init_flight(&inbound);

    if (load_data(DATAFILE, &outbound, &inbound))
        puts("[Loaded existing reservation data.]\n");
    else
        puts("[No data file found or unreadable. Starting fresh.]\n");

    puts("Welcome to Colossus Airlines Seat Reservation");

    for (;;) {
        puts("\nFirst Level Menu");
        puts("a) Outbound Flight");
        puts("b) Inbound Flight");
        puts("c) Quit");
        printf("Enter choice: ");

        if (scanf(" %c", &choice) != 1) { clear_line(); puts("Invalid input.\n"); continue; }
        clear_line();
        choice = (char)tolower((unsigned char)choice);

        if (choice == 'a') {
            if (second_menu(&outbound, "Outbound"))
                save_data(DATAFILE, &outbound, &inbound);
        } else if (choice == 'b') {
            if (second_menu(&inbound, "Inbound"))
                save_data(DATAFILE, &outbound, &inbound);
        } else if (choice == 'c') {
            save_data(DATAFILE, &outbound, &inbound);
            puts("Goodbye!");
            break;
        } else {
            puts("Invalid choice.");
        }
    }
    return 0;
}
