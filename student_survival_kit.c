

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>

#ifdef _WIN32
  #include <windows.h>
  #define CLEAR_CMD "cls"
  #ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
    #define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
  #endif
#else
  #include <unistd.h>
  #define CLEAR_CMD "clear"
#endif

/* ANSI Terminal Color Escape Codes */
#define RST     "\033[0m"
#define BOLD    "\033[1m"
#define DIM     "\033[2m"
#define RED     "\033[1;31m"
#define GREEN   "\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE    "\033[1;34m"
#define MAGENTA "\033[1;35m"
#define CYAN    "\033[1;36m"
#define WHITE   "\033[1;37m"

#define CREDENTIALS_FILE "credentials.txt"

/* ============================================================================
   DATA STRUCTURES (Matching Website Modules 1:1)
   ============================================================================ */

#define MAX_SUBJECTS 8
#define MAX_EXPENSES 30
#define MAX_EXAMS    8
#define MAX_TASKS    15
#define MAX_FRIENDS  8
#define MAX_QUESTS   5

/* Module 4: Attendance Tracker Structure */
typedef struct {
    char name[48];
    int  attended;
    int  total;
    int  credits;
    int  threshold; /* Default: 75% */
} Subject;

/* Module 2: Finance Sentinel Structure */
typedef struct {
    int   id;
    char  desc[64];
    char  category[24]; /* Food, Academics, Bills, Groceries, Other */
    float amount;
    char  date[16];
} Expense;

/* Module 2: Bill Splitter Friend Structure */
typedef struct {
    char  name[32];
    float balance; /* Positive = Owed to you, Negative = You owe them */
} Friend;

/* Module 3: Study Hub - Exam Structure */
typedef struct {
    char subject[48];
    int  days_left;
    int  confidence; /* 1 (Low) to 5 (Mastered) */
} Exam;

/* Module 3: Study Hub - Assignment / Lab Task Structure */
typedef struct {
    char title[64];
    int  days_left;
    char priority[16]; /* "urgent", "normal", "low" */
    int  is_completed;  /* 0 = Pending, 1 = Completed */
} Task;

/* Module 5: Daily Quest Structure */
typedef struct {
    int  id;
    char title[64];
    char reward[32];
    int  is_completed;
} Quest;

/* Master Student Survival Container */
typedef struct {
    char  student_name[64];
    char  sap_id[32];
    char  university[64];
    char  program[64];
    float monthly_budget;
    int   days_in_month;
    int   current_day;

    int      num_subjects;
    Subject  subjects[MAX_SUBJECTS];

    int      num_expenses;
    Expense  expenses[MAX_EXPENSES];

    int      num_friends;
    Friend   friends[MAX_FRIENDS];

    int      num_exams;
    Exam     exams[MAX_EXAMS];

    int      num_tasks;
    Task     tasks[MAX_TASKS];

    int      num_quests;
    Quest    quests[MAX_QUESTS];

    int      is_setup_completed;
} StudentData;

/* Global Active State */
static StudentData g_student;

/* Function Prototypes */
void init_console(void);
void clear_screen(void);
void wait_for_enter(void);
void flush_input(void);

/* Authentication using C File Handling */
int  show_auth_menu(void);
int  authenticate_student(const char *sap, const char *pass, char *out_name);
int  register_student(const char *name, const char *sap, const char *pass);

/* Persistence */
void seed_upes_fresher_defaults(void);
void save_current_student(void);
void save_to_binary(const char *filename);
int  load_from_binary(const char *filename);
void export_to_json(const char *filename);

/* Module Views */
void show_command_center(void);
void show_finance_sentinel(void);
void show_study_hub(void);
void show_attendance_tracker(void);
void show_daily_quests(void);
void show_panic_mode(void);
void show_profile_setup(void);

/* Helper Calculations */
float calc_total_spent(void);
float calc_safe_daily_limit(void);
float calc_overall_attendance(void);
int   calc_consecutive_needed(int attended, int total, int threshold);
int   calc_safe_bunks(int attended, int total, int threshold);

/* ============================================================================
   MAIN ENTRY POINT
   ============================================================================ */
int main(void) {
    init_console();

    while (1) {
        /* Step 1: Authentication Gateway (File Handling) */
        if (!show_auth_menu()) {
            printf("\n  " CYAN "Exiting STUDENT.EXE. Good luck with your UPES semester!\n\n" RST);
            break;
        }

        /* Step 2: Logged In -> Main Survival Protocol Menu */
        int choice = -1;
        while (choice != 0) {
            clear_screen();
            printf("\n");
            printf("  " CYAN BOLD "================================================================" RST "\n");
            printf("  " CYAN BOLD "   STUDENT.EXE : STUDENT SURVIVAL SYSTEM (C KERNEL v5.0)" RST "\n");
            printf("  " CYAN BOLD "   University: UPES Dehradun | B.Tech CSE (Fresher - 1st Year)" RST "\n");
            printf("  " CYAN BOLD "================================================================" RST "\n\n");

            printf("  " BOLD "Logged in as:" RST " %s (SAP: %s)\n", g_student.student_name, g_student.sap_id);
            printf("  " BOLD "Active Subjects:" RST " C Programming, Engineering Mathematics, Problem Solving\n\n");

            printf("  " YELLOW BOLD "[1]" RST " Command Center (Dashboard Overview)\n");
            printf("  " GREEN BOLD "[2]" RST " Finance Sentinel (Budget, Expenses & Bill Splitter)\n");
            printf("  " BLUE BOLD "[3]" RST " Study Hub (Exams, Assignments & Deadlines)\n");
            printf("  " CYAN BOLD "[4]" RST " Attendance Tracker (75%% Cutoff & Bunk Calculator)\n");
            printf("  " MAGENTA BOLD "[5]" RST " Daily Quests (Gamified Daily Missions)\n");
            printf("  " RED BOLD "[6]" RST " 🚨 PANIC MODE (Emergency Crisis Recovery Protocol)\n");
            printf("  " WHITE BOLD "[7]" RST " Configure UPES Fresher Setup / Profile\n");
            printf("  " WHITE BOLD "[8]" RST " File Handling: Save & Export data_%s.json\n", g_student.sap_id);
            printf("  " DIM "[0] Logout / Switch Student" RST "\n\n");

            printf("  " BOLD "Select Module (0-8) -> " RST);
            if (scanf("%d", &choice) != 1) {
                flush_input();
                continue;
            }
            flush_input();

            switch (choice) {
                case 1: show_command_center(); break;
                case 2: show_finance_sentinel(); break;
                case 3: show_study_hub(); break;
                case 4: show_attendance_tracker(); break;
                case 5: show_daily_quests(); break;
                case 6: show_panic_mode(); break;
                case 7: show_profile_setup(); break;
                case 8:
                    save_current_student();
                    printf("\n  " GREEN BOLD "[SUCCESS]" RST " Data saved to 'survival_data_%s.dat' and exported to 'data_%s.json' via C File Handling.\n",
                           g_student.sap_id, g_student.sap_id);
                    wait_for_enter();
                    break;
                case 0:
                    printf("\n  " CYAN "Saving session and logging out...\n" RST);
                    save_current_student();
                    wait_for_enter();
                    break;
                default:
                    printf("\n  " RED "Invalid selection. Please enter 0 through 8.\n" RST);
                    wait_for_enter();
                    break;
            }
        }
    }

    return 0;
}

/* ============================================================================
   AUTHENTICATION SYSTEM (C File Handling: fopen, fscanf, fprintf, fclose)
   ============================================================================ */
int show_auth_menu(void) {
    int auth_choice = -1;
    while (1) {
        clear_screen();
        printf("\n");
        printf("  " CYAN BOLD "================================================================" RST "\n");
        printf("  " CYAN BOLD "   STUDENT.EXE : AUTHENTICATION GATEWAY (C FILE ENGINE)" RST "\n");
        printf("  " CYAN BOLD "   UPES Dehradun | B.Tech CSE (Fresher - 1st Year)" RST "\n");
        printf("  " CYAN BOLD "================================================================" RST "\n\n");

        printf("  " YELLOW BOLD "[1]" RST " Student Login (Enter SAP ID & Access Key)\n");
        printf("  " GREEN BOLD "[2]" RST " Register New Fresher Account\n");
        printf("  " BLUE BOLD "[3]" RST " Quick Demo Login (SAP: 68743 / Arjun Sharma)\n");
        printf("  " DIM "[0] Exit Application" RST "\n\n");

        printf("  " BOLD "Select Option (0-3) -> " RST);
        if (scanf("%d", &auth_choice) != 1) {
            flush_input();
            continue;
        }
        flush_input();

        if (auth_choice == 0) {
            return 0; /* Exit application */
        }

        if (auth_choice == 1) {
            char sap[32], pass[32], name[64];
            printf("\n  " BOLD "Enter Student SAP ID / Roll No : " RST);
            fgets(sap, sizeof(sap), stdin);
            sap[strcspn(sap, "\r\n")] = '\0';

            printf("  " BOLD "Enter Access Key (Password)    : " RST);
            fgets(pass, sizeof(pass), stdin);
            pass[strcspn(pass, "\r\n")] = '\0';

            if (authenticate_student(sap, pass, name)) {
                printf("\n  " GREEN BOLD "[ACCESS GRANTED]" RST " Welcome, %s (SAP: %s)!\n", name, sap);
                char dat_file[64];
                snprintf(dat_file, sizeof(dat_file), "survival_data_%s.dat", sap);
                if (!load_from_binary(dat_file)) {
                    seed_upes_fresher_defaults();
                    strcpy(g_student.student_name, name);
                    strcpy(g_student.sap_id, sap);
                    save_current_student();
                }
                wait_for_enter();
                return 1; /* Logged in */
            } else {
                printf("\n  " RED BOLD "[ACCESS DENIED]" RST " Invalid SAP ID or Access Key. Please check 'credentials.txt' or register.\n");
                wait_for_enter();
            }
        } else if (auth_choice == 2) {
            char name[64], sap[32], pass[32];
            printf("\n  " GREEN BOLD "--- NEW FRESHER REGISTRATION ---" RST "\n");
            printf("  Enter Student Full Name : ");
            fgets(name, sizeof(name), stdin);
            name[strcspn(name, "\r\n")] = '\0';

            printf("  Enter Student SAP ID    : ");
            fgets(sap, sizeof(sap), stdin);
            sap[strcspn(sap, "\r\n")] = '\0';

            printf("  Create Access Key (Password) : ");
            fgets(pass, sizeof(pass), stdin);
            pass[strcspn(pass, "\r\n")] = '\0';

            if (strlen(name) == 0 || strlen(sap) == 0 || strlen(pass) == 0) {
                printf("  " RED "All registration fields are required.\n" RST);
                wait_for_enter();
                continue;
            }

            int res = register_student(name, sap, pass);
            if (res > 0) {
                printf("\n  " GREEN BOLD "[REGISTERED]" RST " Credentials saved to 'credentials.txt' via C File Handling!\n");
                seed_upes_fresher_defaults();
                strcpy(g_student.student_name, name);
                strcpy(g_student.sap_id, sap);
                g_student.is_setup_completed = 0; /* Trigger setup wizard */
                save_current_student();
                printf("  " GREEN "[OK]" RST " Initialized UPES B.Tech CSE Fresher subjects: C, Maths, Problem Solving.\n");
                wait_for_enter();
                return 1;
            } else {
                printf("\n  " RED "Registration error: SAP ID already registered.\n" RST);
                wait_for_enter();
            }
        } else if (auth_choice == 3) {
            char name[64];
            authenticate_student("68743", "password", name);
            char dat_file[64];
            snprintf(dat_file, sizeof(dat_file), "survival_data_%s.dat", "68743");
            if (!load_from_binary(dat_file)) {
                seed_upes_fresher_defaults();
                save_current_student();
            }
            printf("\n  " GREEN BOLD "[DEMO LOGIN]" RST " Authenticated as %s (SAP: %s)!\n", g_student.student_name, g_student.sap_id);
            wait_for_enter();
            return 1;
        }
    }
}

int authenticate_student(const char *sap, const char *pass, char *out_name) {
    FILE *fp = fopen(CREDENTIALS_FILE, "r");
    if (!fp) {
        if (strcmp(sap, "68743") == 0 && strcmp(pass, "password") == 0) {
            strcpy(out_name, "Arjun Sharma");
            return 1;
        }
        return 0;
    }
    char f_sap[32], f_pass[32], f_name[64];
    while (fscanf(fp, "%31s %31s %63[^\n]", f_sap, f_pass, f_name) == 3) {
        if (strcmp(f_sap, sap) == 0 && strcmp(f_pass, pass) == 0) {
            char *p = f_name;
            while (*p == ' ') p++;
            strcpy(out_name, p);
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);

    /* Built-in fallback accounts */
    if (strcmp(sap, "68743") == 0 && strcmp(pass, "password") == 0) {
        strcpy(out_name, "Arjun Sharma");
        return 1;
    }
    if (strcmp(sap, "70012345") == 0 && strcmp(pass, "password123") == 0) {
        strcpy(out_name, "Alex Morgan");
        return 1;
    }
    return 0;
}

int register_student(const char *name, const char *sap, const char *pass) {
    char existing_name[64];
    if (authenticate_student(sap, pass, existing_name)) {
        return -1; /* Already exists */
    }
    FILE *fp = fopen(CREDENTIALS_FILE, "a+");
    if (!fp) return 0;
    fprintf(fp, "%s %s %s\n", sap, pass, name);
    fclose(fp);
    return 1;
}

/* ============================================================================
   MODULE 1: COMMAND CENTER (DASHBOARD)
   ============================================================================ */
void show_command_center(void) {
    clear_screen();
    float total_spent = calc_total_spent();
    float remaining = g_student.monthly_budget - total_spent;
    float safe_daily = calc_safe_daily_limit();
    float overall_att = calc_overall_attendance();

    printf("\n  " CYAN BOLD "=== [1] COMMAND CENTER : SURVIVAL OVERVIEW ===" RST "\n\n");
    printf("  " BOLD "STUDENT:" RST " %s | %s | %s\n\n",
           g_student.student_name, g_student.program, g_student.university);

    printf("  " BOLD "------------------------------------------------------------" RST "\n");
    printf("  " BOLD "QUICK METRICS:" RST "\n");
    printf("    Safe Daily Spend Limit : " GREEN BOLD "Rs. %.2f / day" RST "\n", safe_daily);
    printf("    Monthly Budget Balance : Rs. %.2f remaining (of Rs. %.2f)\n", remaining, g_student.monthly_budget);
    printf("    Overall Attendance     : ");
    if (overall_att >= 75.0f) {
        printf(GREEN BOLD "%.1f%% [SAFE]\n" RST, overall_att);
    } else {
        printf(RED BOLD "%.1f%% [DANGER - BELOW 75%% CUTOFF]\n" RST, overall_att);
    }

    /* Danger radar check */
    int danger_count = 0;
    for (int i = 0; i < g_student.num_subjects; i++) {
        float pct = (g_student.subjects[i].attended * 100.0f) / g_student.subjects[i].total;
        if (pct < g_student.subjects[i].threshold) danger_count++;
    }

    printf("\n  " BOLD "ACADEMIC STATUS:" RST "\n");
    if (danger_count == 0) {
        printf("    " GREEN "[CLEAR]" RST " All subjects are currently above the 75%% UPES threshold!\n");
    } else {
        printf("    " RED "[ALERT]" RST " %d subject(s) are below the 75%% cutoff! Check Attendance Tracker.\n", danger_count);
    }

    /* Exam Countdown */
    if (g_student.num_exams > 0) {
        printf("\n  " BOLD "NEAREST EXAM:" RST " %s in " YELLOW BOLD "%d days" RST " (Confidence: %d/5)\n",
               g_student.exams[0].subject, g_student.exams[0].days_left, g_student.exams[0].confidence);
    }

    printf("  " BOLD "------------------------------------------------------------" RST "\n");
    wait_for_enter();
}

/* ============================================================================
   MODULE 2: FINANCE SENTINEL
   ============================================================================ */
void show_finance_sentinel(void) {
    int subchoice = -1;
    while (subchoice != 0) {
        clear_screen();
        float total_spent = calc_total_spent();
        float remaining = g_student.monthly_budget - total_spent;
        float safe_daily = calc_safe_daily_limit();

        printf("\n  " GREEN BOLD "=== [2] FINANCE SENTINEL : EXPENSES & BILL SPLITTER ===" RST "\n\n");
        printf("  Monthly Budget : Rs. %.2f | Spent: Rs. %.2f | Remaining: Rs. %.2f\n",
               g_student.monthly_budget, total_spent, remaining);
        printf("  Safe Daily Allowance : " GREEN BOLD "Rs. %.2f / day" RST "\n\n", safe_daily);

        printf("  " BOLD "RECENT EXPENSES:" RST "\n");
        if (g_student.num_expenses == 0) {
            printf("    (No expenses logged yet)\n");
        } else {
            for (int i = 0; i < g_student.num_expenses; i++) {
                printf("    %2d. %-28s [%-9s] Rs. %7.2f (%s)\n",
                       i + 1, g_student.expenses[i].desc, g_student.expenses[i].category,
                       g_student.expenses[i].amount, g_student.expenses[i].date);
            }
        }

        printf("\n  " BOLD "CAMPUS FRIENDS / BILL SPLITTER BALANCES:" RST "\n");
        for (int i = 0; i < g_student.num_friends; i++) {
            float b = g_student.friends[i].balance;
            if (b > 0) {
                printf("    %-22s : " GREEN "owes you Rs. %.2f" RST "\n", g_student.friends[i].name, b);
            } else if (b < 0) {
                printf("    %-22s : " RED "you owe Rs. %.2f" RST "\n", g_student.friends[i].name, -b);
            } else {
                printf("    %-22s : All settled (Rs. 0.00)\n", g_student.friends[i].name);
            }
        }

        printf("\n  [1] Record New Expense\n");
        printf("  [2] Split a Shared Bill with Friends\n");
        printf("  [3] Adjust Monthly Budget Limit\n");
        printf("  [0] Return to Main Menu\n\n");
        printf("  Select Option -> ");

        if (scanf("%d", &subchoice) != 1) {
            flush_input();
            continue;
        }
        flush_input();

        if (subchoice == 1) {
            if (g_student.num_expenses >= MAX_EXPENSES) {
                printf("\n  " RED "Expense log is full.\n" RST);
                wait_for_enter();
                continue;
            }
            Expense *e = &g_student.expenses[g_student.num_expenses];
            e->id = g_student.num_expenses + 1;

            printf("\n  Enter Expense Description : ");
            fgets(e->desc, sizeof(e->desc), stdin);
            e->desc[strcspn(e->desc, "\r\n")] = '\0';

            printf("  Category (Food/Academics/Bills/Groceries/Other) : ");
            fgets(e->category, sizeof(e->category), stdin);
            e->category[strcspn(e->category, "\r\n")] = '\0';

            printf("  Amount (Rs.) : ");
            scanf("%f", &e->amount);
            flush_input();

            strcpy(e->date, "Today");
            g_student.num_expenses++;
            save_current_student();
            printf("  " GREEN BOLD "[OK] Expense logged successfully!\n" RST);
            wait_for_enter();
        } else if (subchoice == 2) {
            char desc[64];
            float bill_total = 0;
            printf("\n  Enter Bill Description (e.g. Canteen Treat / Uber) : ");
            fgets(desc, sizeof(desc), stdin);
            desc[strcspn(desc, "\r\n")] = '\0';

            printf("  Enter Total Bill Amount (Rs.) : ");
            scanf("%f", &bill_total);
            flush_input();

            if (bill_total > 0 && g_student.num_friends > 0) {
                int total_people = g_student.num_friends + 1; /* Including self */
                float per_person = bill_total / total_people;
                for (int i = 0; i < g_student.num_friends; i++) {
                    g_student.friends[i].balance += per_person;
                }
                save_current_student();
                printf("  " GREEN BOLD "[OK] Split Rs. %.2f among %d people. Added Rs. %.2f to each friend's balance!\n" RST,
                       bill_total, total_people, per_person);
            }
            wait_for_enter();
        } else if (subchoice == 3) {
            printf("\n  Current Monthly Budget: Rs. %.2f\n", g_student.monthly_budget);
            printf("  Enter New Monthly Budget: Rs. ");
            scanf("%f", &g_student.monthly_budget);
            flush_input();
            save_current_student();
            printf("  " GREEN BOLD "[OK] Monthly budget updated to Rs. %.2f!\n" RST, g_student.monthly_budget);
            wait_for_enter();
        }
    }
}

/* ============================================================================
   MODULE 3: STUDY HUB
   ============================================================================ */
void show_study_hub(void) {
    int subchoice = -1;
    while (subchoice != 0) {
        clear_screen();
        printf("\n  " BLUE BOLD "=== [3] STUDY HUB : EXAMS & ASSIGNMENTS ===" RST "\n\n");

        printf("  " BOLD "UPCOMING EXAMS & CONFIDENCE RATINGS:" RST "\n");
        for (int i = 0; i < g_student.num_exams; i++) {
            printf("    %d. %-24s : " YELLOW "%2d days left" RST " | Confidence: %d/5 ",
                   i + 1, g_student.exams[i].subject, g_student.exams[i].days_left, g_student.exams[i].confidence);
            if (g_student.exams[i].confidence <= 2) {
                printf(RED BOLD "[HIGH PRIORITY - REVISION NEEDED]\n" RST);
            } else {
                printf(GREEN "[STABLE]\n" RST);
            }
        }

        printf("\n  " BOLD "LAB ASSIGNMENTS & TASKS:" RST "\n");
        for (int i = 0; i < g_student.num_tasks; i++) {
            printf("    [%c] %-42s (%d days) [%s]\n",
                   g_student.tasks[i].is_completed ? 'X' : ' ',
                   g_student.tasks[i].title,
                   g_student.tasks[i].days_left,
                   g_student.tasks[i].priority);
        }

        printf("\n  [1] Mark Assignment Complete / Pending\n");
        printf("  [2] Add New Assignment / Lab File\n");
        printf("  [0] Return to Main Menu\n\n");
        printf("  Select Option -> ");

        if (scanf("%d", &subchoice) != 1) {
            flush_input();
            continue;
        }
        flush_input();

        if (subchoice == 1) {
            printf("\n  Enter Task Number (1-%d) to toggle : ", g_student.num_tasks);
            int tnum;
            if (scanf("%d", &tnum) == 1 && tnum >= 1 && tnum <= g_student.num_tasks) {
                g_student.tasks[tnum - 1].is_completed = !g_student.tasks[tnum - 1].is_completed;
                save_current_student();
                printf("  " GREEN BOLD "[OK] Task status updated!\n" RST);
            }
            flush_input();
            wait_for_enter();
        } else if (subchoice == 2) {
            if (g_student.num_tasks >= MAX_TASKS) {
                printf("  Task list full.\n");
                wait_for_enter();
                continue;
            }
            Task *t = &g_student.tasks[g_student.num_tasks];
            printf("\n  Enter Task Title : ");
            fgets(t->title, sizeof(t->title), stdin);
            t->title[strcspn(t->title, "\r\n")] = '\0';

            printf("  Days until due : ");
            scanf("%d", &t->days_left);
            flush_input();

            strcpy(t->priority, t->days_left <= 2 ? "urgent" : "normal");
            t->is_completed = 0;
            g_student.num_tasks++;

            save_current_student();
            printf("  " GREEN BOLD "[OK] New task added successfully!\n" RST);
            wait_for_enter();
        }
    }
}

/* ============================================================================
   MODULE 4: ATTENDANCE TRACKER
   ============================================================================ */
void show_attendance_tracker(void) {
    int subchoice = -1;
    while (subchoice != 0) {
        clear_screen();
        float overall = calc_overall_attendance();

        printf("\n  " CYAN BOLD "=== [4] ATTENDANCE TRACKER (75%% CUTOFF SYSTEM) ===" RST "\n\n");
        printf("  Overall Attendance : ");
        if (overall >= 75.0f) printf(GREEN BOLD "%.1f%% [SAFE STANDING]\n\n" RST, overall);
        else printf(RED BOLD "%.1f%% [DANGER - MEDICAL/DEBAR RISK]\n\n" RST, overall);

        printf("  %-25s | Attended / Total | Current %% | Status & Action\n", "Subject");
        printf("  --------------------------+------------------+-----------+---------------------------------\n");

        for (int i = 0; i < g_student.num_subjects; i++) {
            Subject *s = &g_student.subjects[i];
            float pct = (s->total > 0) ? ((s->attended * 100.0f) / s->total) : 0.0f;
            printf("  %-25s |     %2d / %2d      |   %5.1f%%  | ",
                   s->name, s->attended, s->total, pct);

            if (pct < s->threshold) {
                int needed = calc_consecutive_needed(s->attended, s->total, s->threshold);
                printf(RED BOLD "DANGER: Attend %d consecutive\n" RST, needed);
            } else {
                int bunks = calc_safe_bunks(s->attended, s->total, s->threshold);
                printf(GREEN "SAFE: %d safe bunk(s) available\n" RST, bunks);
            }
        }
        printf("\n");

        printf("  [1] Mark Class Attended (Subject attended + 1, total + 1)\n");
        printf("  [2] Mark Class Missed (Subject total + 1)\n");
        printf("  [0] Return to Main Menu\n\n");
        printf("  Select Option -> ");

        if (scanf("%d", &subchoice) != 1) {
            flush_input();
            continue;
        }
        flush_input();

        if (subchoice == 1 || subchoice == 2) {
            printf("\n  Select Subject Number (1-%d):\n", g_student.num_subjects);
            for (int i = 0; i < g_student.num_subjects; i++) {
                printf("    %d. %s\n", i + 1, g_student.subjects[i].name);
            }
            printf("  Choice -> ");
            int snum;
            if (scanf("%d", &snum) == 1 && snum >= 1 && snum <= g_student.num_subjects) {
                Subject *s = &g_student.subjects[snum - 1];
                s->total++;
                if (subchoice == 1) s->attended++;
                save_current_student();
                printf("  " GREEN BOLD "[OK] Attendance updated for %s! Current: %d/%d (%.1f%%)\n" RST,
                       s->name, s->attended, s->total, (s->attended * 100.0f) / s->total);
            }
            flush_input();
            wait_for_enter();
        }
    }
}

/* ============================================================================
   MODULE 5: DAILY QUESTS
   ============================================================================ */
void show_daily_quests(void) {
    clear_screen();
    printf("\n  " MAGENTA BOLD "=== [5] DAILY QUESTS (STUDENT SURVIVAL CHECKLIST) ===" RST "\n\n");
    printf("  Complete these micro-goals daily to maintain academic and financial survival!\n\n");

    for (int i = 0; i < g_student.num_quests; i++) {
        printf("  [%c] Quest %d: %-44s " YELLOW "[Reward: %s]" RST "\n",
               g_student.quests[i].is_completed ? 'X' : ' ',
               i + 1, g_student.quests[i].title, g_student.quests[i].reward);
    }

    printf("\n  Enter Quest Number (1-%d) to toggle completion (or 0 to go back): ", g_student.num_quests);
    int qnum;
    if (scanf("%d", &qnum) == 1 && qnum >= 1 && qnum <= g_student.num_quests) {
        g_student.quests[qnum - 1].is_completed = !g_student.quests[qnum - 1].is_completed;
        save_current_student();
        printf("\n  " GREEN BOLD "[OK] Quest status updated!\n" RST);
    }
    flush_input();
    wait_for_enter();
}

/* ============================================================================
   MODULE 6: PANIC MODE (EMERGENCY RECOVERY PROTOCOL)
   ============================================================================ */
void show_panic_mode(void) {
    clear_screen();
    printf("\n  " RED BOLD "================================================================" RST "\n");
    printf("  " RED BOLD "   🚨 PANIC MODE : EMERGENCY CRISIS RECOVERY PROTOCOL 🚨" RST "\n");
    printf("  " RED BOLD "================================================================" RST "\n\n");

    int crisis_count = 0;

    /* 1. Attendance Crisis Check */
    for (int i = 0; i < g_student.num_subjects; i++) {
        Subject *s = &g_student.subjects[i];
        float pct = (s->total > 0) ? ((s->attended * 100.0f) / s->total) : 0.0f;
        if (pct < s->threshold) {
            crisis_count++;
            int needed = calc_consecutive_needed(s->attended, s->total, s->threshold);
            printf("  " RED BOLD "[CRISIS PRIORITY %d] RECOVER ATTENDANCE: %s" RST "\n", crisis_count, s->name);
            printf("    Current Standing: " RED "%.1f%%" RST " (Below %d%% cutoff).\n", pct, s->threshold);
            printf("    " BOLD "DIRECTIVE:" RST " You must attend the next " YELLOW BOLD "%d consecutive classes" RST " with zero bunks.\n\n", needed);
        }
    }

    /* 2. Urgent Exam Crisis Check */
    for (int i = 0; i < g_student.num_exams; i++) {
        if (g_student.exams[i].days_left <= 3) {
            crisis_count++;
            printf("  " YELLOW BOLD "[CRISIS PRIORITY %d] IMMINENT EXAM: %s" RST "\n", crisis_count, g_student.exams[i].subject);
            printf("    Time Left : " RED BOLD "%d DAYS" RST " | Confidence Rating: %d/5\n",
                   g_student.exams[i].days_left, g_student.exams[i].confidence);
            printf("    " BOLD "DIRECTIVE:" RST " Stop passive reading. Focus exclusively on previous year question papers and lab files.\n\n");
        }
    }

    /* 3. Financial Crisis Check */
    float safe_daily = calc_safe_daily_limit();
    if (safe_daily <= 100.0f) {
        crisis_count++;
        printf("  " RED BOLD "[CRISIS PRIORITY %d] EMERGENCY FINANCIAL DEFICIT" RST "\n", crisis_count);
        printf("    Safe Daily Allowance is down to " RED "Rs. %.2f/day" RST "!\n", safe_daily);
        printf("    " BOLD "DIRECTIVE:" RST " Enforce zero non-essential spending. Settle debts with campus friends immediately.\n\n");
    }

    /* 4. Urgent Assignments Check */
    for (int i = 0; i < g_student.num_tasks; i++) {
        if (!g_student.tasks[i].is_completed && g_student.tasks[i].days_left <= 1) {
            crisis_count++;
            printf("  " RED BOLD "[CRISIS PRIORITY %d] OVERDUE / URGENT SUBMISSION: %s" RST "\n",
                   crisis_count, g_student.tasks[i].title);
            printf("    Due in %d day(s). Complete and submit to portal before midnight cutoff!\n\n",
                   g_student.tasks[i].days_left);
        }
    }

    if (crisis_count == 0) {
        printf("  " GREEN BOLD "[ALL CLEAR] No critical emergencies active at this time." RST "\n");
        printf("  All subjects safe, no imminent exams (<3d), and spending is inside budget parameters.\n\n");
    } else {
        printf("  " BOLD "TOTAL CRITICAL DIRECTIVES ACTIVE: " RED "%d" RST "\n", crisis_count);
        printf("  Execute these actions in order of priority to stabilize your academic semester.\n\n");
    }

    wait_for_enter();
}

/* ============================================================================
   MODULE 7: UPES FRESHER PROFILE SETUP WIZARD
   ============================================================================ */
void show_profile_setup(void) {
    clear_screen();
    printf("\n  " WHITE BOLD "=== [7] UPES B.TECH CSE FRESHER SETUP WIZARD ===" RST "\n\n");
    printf("  Current Settings:\n");
    printf("    Student Name : %s\n", g_student.student_name);
    printf("    SAP ID       : %s\n", g_student.sap_id);
    printf("    University   : %s\n", g_student.university);
    printf("    Program      : %s\n", g_student.program);
    printf("    Budget (Rs.) : %.2f\n\n", g_student.monthly_budget);

    printf("  [1] Reset to UPES Fresher Standard Presets (C, Maths, Problem Solving)\n");
    printf("  [2] Edit Student Name and SAP ID\n");
    printf("  [3] Edit Monthly Budget\n");
    printf("  [0] Return to Main Menu\n\n");
    printf("  Select Option -> ");

    int opt;
    if (scanf("%d", &opt) != 1) {
        flush_input();
        return;
    }
    flush_input();

    if (opt == 1) {
        seed_upes_fresher_defaults();
        save_current_student();
        printf("\n  " GREEN BOLD "[OK] Restored UPES B.Tech CSE Fresher presets!\n" RST);
        wait_for_enter();
    } else if (opt == 2) {
        printf("\n  Enter Student Name : ");
        fgets(g_student.student_name, sizeof(g_student.student_name), stdin);
        g_student.student_name[strcspn(g_student.student_name, "\r\n")] = '\0';

        printf("  Enter SAP ID : ");
        fgets(g_student.sap_id, sizeof(g_student.sap_id), stdin);
        g_student.sap_id[strcspn(g_student.sap_id, "\r\n")] = '\0';

        save_current_student();
        printf("  " GREEN BOLD "[OK] Profile updated!\n" RST);
        wait_for_enter();
    } else if (opt == 3) {
        printf("\n  Enter Monthly Budget (Rs.) : ");
        scanf("%f", &g_student.monthly_budget);
        flush_input();
        save_current_student();
        printf("  " GREEN BOLD "[OK] Budget updated!\n" RST);
        wait_for_enter();
    }
}

/* ============================================================================
   FILE HANDLING (1st-Year C Syllabus: fopen, fwrite, fread, fprintf, fclose)
   ============================================================================ */

void save_current_student(void) {
    char dat_file[64];
    char json_file[64];
    snprintf(dat_file, sizeof(dat_file), "survival_data_%s.dat", g_student.sap_id);
    snprintf(json_file, sizeof(json_file), "data_%s.json", g_student.sap_id);
    save_to_binary(dat_file);
    export_to_json(json_file);
}

void save_to_binary(const char *filename) {
    FILE *fp = fopen(filename, "wb");
    if (fp != NULL) {
        fwrite(&g_student, sizeof(StudentData), 1, fp);
        fclose(fp);
    }
}

int load_from_binary(const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) return 0;
    size_t read_bytes = fread(&g_student, sizeof(StudentData), 1, fp);
    fclose(fp);
    return (read_bytes == 1);
}

void export_to_json(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (fp == NULL) return;

    fprintf(fp, "{\n");
    fprintf(fp, "  \"student_name\": \"%s\",\n", g_student.student_name);
    fprintf(fp, "  \"sap_id\": \"%s\",\n", g_student.sap_id);
    fprintf(fp, "  \"university\": \"%s\",\n", g_student.university);
    fprintf(fp, "  \"program\": \"%s\",\n", g_student.program);
    fprintf(fp, "  \"budget\": %.2f,\n", (double)g_student.monthly_budget);

    /* Subjects (Strictly UPES Fresher 3 Subjects) */
    fprintf(fp, "  \"subjects\": [\n");
    for (int i = 0; i < g_student.num_subjects; i++) {
        fprintf(fp, "    {\"id\": %d, \"name\": \"%s\", \"attended\": %d, \"total\": %d, \"credits\": %d, \"threshold\": %d}%s\n",
                i + 1,
                g_student.subjects[i].name,
                g_student.subjects[i].attended,
                g_student.subjects[i].total,
                g_student.subjects[i].credits,
                g_student.subjects[i].threshold,
                (i + 1 < g_student.num_subjects ? "," : ""));
    }
    fprintf(fp, "  ],\n");

    /* Expenses */
    fprintf(fp, "  \"expenses\": [\n");
    for (int i = 0; i < g_student.num_expenses; i++) {
        fprintf(fp, "    {\"id\": %d, \"desc\": \"%s\", \"category\": \"%s\", \"amount\": %.2f, \"date\": \"%s\"}%s\n",
                g_student.expenses[i].id,
                g_student.expenses[i].desc,
                g_student.expenses[i].category,
                (double)g_student.expenses[i].amount,
                g_student.expenses[i].date,
                (i + 1 < g_student.num_expenses ? "," : ""));
    }
    fprintf(fp, "  ],\n");

    /* Exams */
    fprintf(fp, "  \"exams\": [\n");
    for (int i = 0; i < g_student.num_exams; i++) {
        fprintf(fp, "    {\"id\": %d, \"subject\": \"%s\", \"days_left\": %d, \"confidence\": %d}%s\n",
                i + 1,
                g_student.exams[i].subject,
                g_student.exams[i].days_left,
                g_student.exams[i].confidence,
                (i + 1 < g_student.num_exams ? "," : ""));
    }
    fprintf(fp, "  ],\n");

    /* Tasks */
    fprintf(fp, "  \"tasks\": [\n");
    for (int i = 0; i < g_student.num_tasks; i++) {
        fprintf(fp, "    {\"id\": %d, \"title\": \"%s\", \"days_left\": %d, \"priority\": \"%s\", \"is_completed\": %s}%s\n",
                i + 1,
                g_student.tasks[i].title,
                g_student.tasks[i].days_left,
                g_student.tasks[i].priority,
                g_student.tasks[i].is_completed ? "true" : "false",
                (i + 1 < g_student.num_tasks ? "," : ""));
    }
    fprintf(fp, "  ],\n");

    /* Friends / Bill Splitter */
    fprintf(fp, "  \"friends\": [\n");
    for (int i = 0; i < g_student.num_friends; i++) {
        fprintf(fp, "    {\"id\": %d, \"name\": \"%s\", \"balance\": %.2f}%s\n",
                i + 1,
                g_student.friends[i].name,
                (double)g_student.friends[i].balance,
                (i + 1 < g_student.num_friends ? "," : ""));
    }
    fprintf(fp, "  ],\n");

    fprintf(fp, "  \"completed_quests\": [],\n");
    fprintf(fp, "  \"is_setup_completed\": true\n");
    fprintf(fp, "}\n");

    fclose(fp);
}

/* ============================================================================
   DEFAULT DATA SEEDING (UPES B.Tech CSE Fresher Preset)
   ============================================================================ */
void seed_upes_fresher_defaults(void) {
    memset(&g_student, 0, sizeof(StudentData));

    strcpy(g_student.student_name, "Arjun Sharma");
    strcpy(g_student.sap_id, "68743");
    strcpy(g_student.university, "UPES Dehradun");
    strcpy(g_student.program, "B.Tech CSE (Fresher - 1st Year)");
    g_student.monthly_budget = 7500.0f;
    g_student.days_in_month = 30;
    g_student.current_day = 24;
    g_student.is_setup_completed = 1;

    /* 3 Core UPES Fresher Subjects */
    g_student.num_subjects = 3;
    strcpy(g_student.subjects[0].name, "C Programming");
    g_student.subjects[0].attended = 18; g_student.subjects[0].total = 22; g_student.subjects[0].credits = 4; g_student.subjects[0].threshold = 75;

    strcpy(g_student.subjects[1].name, "Engineering Mathematics");
    g_student.subjects[1].attended = 14; g_student.subjects[1].total = 20; g_student.subjects[1].credits = 4; g_student.subjects[1].threshold = 75;

    strcpy(g_student.subjects[2].name, "Problem Solving");
    g_student.subjects[2].attended = 19; g_student.subjects[2].total = 22; g_student.subjects[2].credits = 3; g_student.subjects[2].threshold = 75;

    /* Sample Expenses */
    g_student.num_expenses = 3;
    g_student.expenses[0].id = 1; strcpy(g_student.expenses[0].desc, "C Programming Reference Book"); strcpy(g_student.expenses[0].category, "Academics"); g_student.expenses[0].amount = 350.0f; strcpy(g_student.expenses[0].date, "2026-11-20");
    g_student.expenses[1].id = 2; strcpy(g_student.expenses[1].desc, "Campus Canteen Lunch"); strcpy(g_student.expenses[1].category, "Food"); g_student.expenses[1].amount = 120.0f; strcpy(g_student.expenses[1].date, "2026-11-23");
    g_student.expenses[2].id = 3; strcpy(g_student.expenses[2].desc, "Hostel Wi-Fi & Printing"); strcpy(g_student.expenses[2].category, "Bills"); g_student.expenses[2].amount = 450.0f; strcpy(g_student.expenses[2].date, "2026-11-24");

    /* Exams */
    g_student.num_exams = 3;
    strcpy(g_student.exams[0].subject, "C Programming"); g_student.exams[0].days_left = 3; g_student.exams[0].confidence = 2;
    strcpy(g_student.exams[1].subject, "Engineering Mathematics"); g_student.exams[1].days_left = 7; g_student.exams[1].confidence = 3;
    strcpy(g_student.exams[2].subject, "Problem Solving"); g_student.exams[2].days_left = 11; g_student.exams[2].confidence = 4;

    /* Lab Tasks */
    g_student.num_tasks = 4;
    strcpy(g_student.tasks[0].title, "C Pointers & Dynamic Allocation Lab File"); g_student.tasks[0].days_left = 1; strcpy(g_student.tasks[0].priority, "urgent"); g_student.tasks[0].is_completed = 0;
    strcpy(g_student.tasks[1].title, "Engineering Mathematics Calculus Sheet 2"); g_student.tasks[1].days_left = 3; strcpy(g_student.tasks[1].priority, "normal"); g_student.tasks[1].is_completed = 0;
    strcpy(g_student.tasks[2].title, "Problem Solving Flowcharts Record"); g_student.tasks[2].days_left = 4; strcpy(g_student.tasks[2].priority, "normal"); g_student.tasks[2].is_completed = 0;
    strcpy(g_student.tasks[3].title, "Submit UPES Bidholi Hostel Gate Pass"); g_student.tasks[3].days_left = 2; strcpy(g_student.tasks[3].priority, "low"); g_student.tasks[3].is_completed = 1;

    /* Campus Friends */
    g_student.num_friends = 3;
    strcpy(g_student.friends[0].name, "Rohan (Bidholi)"); g_student.friends[0].balance = -250.0f;
    strcpy(g_student.friends[1].name, "Aarav (Room 204)"); g_student.friends[1].balance = 180.0f;
    strcpy(g_student.friends[2].name, "Ananya (CSE Batch A)"); g_student.friends[2].balance = -120.0f;

    /* Quests */
    g_student.num_quests = 4;
    g_student.quests[0].id = 1; strcpy(g_student.quests[0].title, "Attend C Programming Lab Session"); strcpy(g_student.quests[0].reward, "+50 XP"); g_student.quests[0].is_completed = 0;
    g_student.quests[1].id = 2; strcpy(g_student.quests[1].title, "Solve 5 Engineering Maths Integration Problems"); strcpy(g_student.quests[1].reward, "+40 XP"); g_student.quests[1].is_completed = 0;
    g_student.quests[2].id = 3; strcpy(g_student.quests[2].title, "Keep daily food spend under safe limit (Rs. 800)"); strcpy(g_student.quests[2].reward, "+30 XP"); g_student.quests[2].is_completed = 1;
    g_student.quests[3].id = 4; strcpy(g_student.quests[3].title, "Settle pending balance with hostel roommate"); strcpy(g_student.quests[3].reward, "+25 XP"); g_student.quests[3].is_completed = 0;
}

/* ============================================================================
   MATHEMATICAL HELPER CALCULATIONS
   ============================================================================ */
float calc_total_spent(void) {
    float total = 0.0f;
    for (int i = 0; i < g_student.num_expenses; i++) {
        total += g_student.expenses[i].amount;
    }
    return total;
}

float calc_safe_daily_limit(void) {
    float remaining = g_student.monthly_budget - calc_total_spent();
    int days_left = g_student.days_in_month - g_student.current_day + 1;
    if (days_left <= 0) days_left = 1;
    float daily = remaining / (float)days_left;
    return (daily > 0.0f) ? daily : 0.0f;
}

float calc_overall_attendance(void) {
    int total_att = 0, total_cls = 0;
    for (int i = 0; i < g_student.num_subjects; i++) {
        total_att += g_student.subjects[i].attended;
        total_cls += g_student.subjects[i].total;
    }
    return (total_cls > 0) ? ((total_att * 100.0f) / total_cls) : 75.0f;
}

/* Formula: ceil((threshold * total - 100 * attended) / (100 - threshold)) */
int calc_consecutive_needed(int attended, int total, int threshold) {
    float num = (float)(threshold * total - 100 * attended);
    float den = (float)(100 - threshold);
    if (num <= 0) return 0;
    return (int)ceilf(num / den);
}

/* Formula: floor((100 * attended - threshold * total) / threshold) */
int calc_safe_bunks(int attended, int total, int threshold) {
    float val = (float)(100 * attended - threshold * total) / (float)threshold;
    if (val < 0) return 0;
    return (int)floorf(val);
}

/* ============================================================================
   TERMINAL UTILITIES
   ============================================================================ */
void init_console(void) {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode);
        }
    }
#endif
}

void clear_screen(void) {
    system(CLEAR_CMD);
}

void wait_for_enter(void) {
    printf("\n  " DIM "Press [Enter] to continue..." RST);
    getchar();
}

void flush_input(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}
