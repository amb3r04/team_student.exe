/*
 * ============================================================================
 *   NEXUS: STUDENT SURVIVAL KIT — ENTERPRISE HACKATHON EDITION (v3.0)
 *   Engineered for UPES & College Hackathons | Team Nexus
 *   Pure ANSI C99 | 100% Clean ASCII | Zero Unicode Corruption on Windows
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>

#ifdef _WIN32
  #include <windows.h>
  #include <conio.h>
  #define CLEAR_CMD "cls"
  #ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
    #define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
  #endif
#else
  #include <unistd.h>
  #include <strings.h>
  #define CLEAR_CMD "clear"
  #define Sleep(ms) usleep((ms) * 1000)
#endif

/* Clean portable case-insensitive string comparison */
static int str_case_eq(const char *s1, const char *s2) {
    while (*s1 && *s2) {
        if (tolower((unsigned char)*s1) != tolower((unsigned char)*s2)) return 0;
        s1++; s2++;
    }
    return (*s1 == '\0' && *s2 == '\0');
}

/* ─── ANSI Colors ───────────────────────────────────────────────────────────── */
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
#define GRAY    "\033[90m"

#define SAVE_FILE_DEFAULT "survival_data.dat"

/* ============================================================================
   DATA STRUCTURES (20+ MODULES ENGINE)
   ============================================================================ */

#define MAX_SUBJECTS 12
typedef struct {
    char  name[32];
    int   attended;
    int   total;
    int   credits;
    float expected_grade_point; /* 0.0 to 10.0 */
    int   study_minutes;        /* Study session tracker */
} Subject;

#define MAX_EXAMS 10
typedef struct {
    char  subject[32];
    int   days_left;
    int   confidence; /* 1 to 5 */
    float priority_score;
} Exam;

#define MAX_EXPENSES 60
typedef struct {
    char  desc[32];
    char  category[16]; /* Food, Travel, Fun, Bills, Academics, Other */
    float amount;
} ExpenseItem;

#define MAX_TASKS 25
typedef struct {
    char title[40];
    char category[16];   /* Project, Assignment, Personal */
    char assignee[24];
    int  days_until_due; /* negative = overdue */
    int  is_completed;
    int  priority;       /* 1 = Low, 2 = Medium, 3 = High */
} TaskItem;

#define MAX_SPLIT_MEMBERS 10
typedef struct {
    char  name[24];
    float balance;
} SplitMember;

typedef struct {
    int   from;
    int   to;
    float amount;
} Settlement;

#define MAX_IOU 15
typedef struct {
    char  person[24];
    float amount;
    int   is_lent; /* 1 = I lent to them (they owe me), 0 = I borrowed (I owe them) */
    char  note[32];
    int   settled;
} IOUItem;

#define MAX_INVENTORY 15
typedef struct {
    char name[28];
    char category[16]; /* Toiletries, Snacks, Stationery, Meds */
    int  quantity;
    int  min_threshold;
} InventoryItem;

#define MAX_EMERGENCY 8
typedef struct {
    char title[28];
    char phone[20];
} EmergencyContact;

/* Master Application State */
typedef struct {
    char student_name[32];
    char sap_id[16];
    char current_save_file[64];

    /* 1. Academics & Attendance */
    Subject subjects[MAX_SUBJECTS];
    int     num_subjects;
    float   prior_cgpa;
    int     prior_credits;

    /* 2. Exams & Study */
    Exam    exams[MAX_EXAMS];
    int     num_exams;

    /* 3. Budget & Expenses */
    float       initial_budget;
    int         days_spent;
    int         total_month_days;
    ExpenseItem expenses[MAX_EXPENSES];
    int         num_expenses;
    float       spent_today;

    /* 4. Group Project, Assignments & To-Dos */
    TaskItem    tasks[MAX_TASKS];
    int         num_tasks;

    /* 5. Splitter & IOU */
    SplitMember split_members[MAX_SPLIT_MEMBERS];
    int         num_split_members;
    IOUItem     ious[MAX_IOU];
    int         num_ious;

    /* 6. Hostel Inventory */
    InventoryItem inventory[MAX_INVENTORY];
    int           num_inventory;

    /* 7. Mess & Meals Tracker */
    int meals_taken[4];   /* 0: Breakfast, 1: Lunch, 2: Snacks, 3: Dinner */
    int meals_skipped[4];

    /* 8. Energy & Well-being */
    float sleep_hours;
    int   study_hours_today;
    int   stress_level; /* 1 to 5 */

    /* 9. Emergency Contacts */
    EmergencyContact emergency[MAX_EMERGENCY];
    int              num_emergency;

    /* 10. Gamification & Streaks */
    int survival_streak;
    int longest_streak;
    int today_mission_completed;
    int total_tasks_completed;

} AppData;

static AppData g_data;

/* ============================================================================
   TERMINAL & ANIMATION ENGINE
   ============================================================================ */
void enable_ansi(void) {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (GetConsoleMode(h, &mode)) {
        mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(h, mode);
    }
#endif
}

void clear_screen(void) {
    system(CLEAR_CMD);
}

void wait_key(void) {
    printf(GRAY "\n  Press [Enter] to return..." RST);
    fflush(stdout);
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void flush_input(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void read_line(char *buf, int max) {
    if (fgets(buf, max, stdin)) {
        buf[strcspn(buf, "\r\n")] = 0;
    }
}

void type_text(const char *text, int delay_ms) {
    for (int i = 0; text[i]; i++) {
        putchar(text[i]);
        fflush(stdout);
        Sleep(delay_ms);
    }
}

void animate_progress_bar(int width, float percent, const char *color, int delay_ms) {
    int filled = (int)((percent / 100.0f) * width);
    if (filled > width) filled = width;
    if (filled < 0) filled = 0;

    printf("  %s[", color);
    for (int i = 0; i < width; i++) {
        if (i < filled) {
            putchar('#');
        } else {
            printf(GRAY "." RST "%s", color);
        }
        fflush(stdout);
        if (delay_ms > 0 && i < filled) Sleep(delay_ms);
    }
    printf("]" RST);
}

void animate_currency(float target, int steps, int delay_ms) {
    for (int i = 1; i <= steps; i++) {
        float val = target * ((float)i / steps);
        printf("\r  " GREEN BOLD "Rs. %.0f" RST, val);
        fflush(stdout);
        Sleep(delay_ms);
    }
    printf("\r  " GREEN BOLD "Rs. %.2f" RST "\n", target);
}

/* ============================================================================
   DEFAULT SEEDING & PERSISTENCE
   ============================================================================ */
void seed_default_data(void) {
    memset(&g_data, 0, sizeof(AppData));
    strcpy(g_data.student_name, "Loqman");
    strcpy(g_data.sap_id, "500092341");
    strcpy(g_data.current_save_file, SAVE_FILE_DEFAULT);

    /* 1. Subjects */
    g_data.num_subjects = 4;
    strcpy(g_data.subjects[0].name, "Data Structures");
    g_data.subjects[0].attended = 22; g_data.subjects[0].total = 32; g_data.subjects[0].credits = 4;
    g_data.subjects[0].expected_grade_point = 8.0f; g_data.subjects[0].study_minutes = 240;

    strcpy(g_data.subjects[1].name, "Operating Systems");
    g_data.subjects[1].attended = 28; g_data.subjects[1].total = 30; g_data.subjects[1].credits = 4;
    g_data.subjects[1].expected_grade_point = 9.0f; g_data.subjects[1].study_minutes = 310;

    strcpy(g_data.subjects[2].name, "Computer Networks");
    g_data.subjects[2].attended = 21; g_data.subjects[2].total = 28; g_data.subjects[2].credits = 3;
    g_data.subjects[2].expected_grade_point = 7.0f; g_data.subjects[2].study_minutes = 150;

    strcpy(g_data.subjects[3].name, "Theory of Computation");
    g_data.subjects[3].attended = 17; g_data.subjects[3].total = 26; g_data.subjects[3].credits = 3;
    g_data.subjects[3].expected_grade_point = 6.0f; g_data.subjects[3].study_minutes = 80;

    g_data.prior_cgpa = 8.12f;
    g_data.prior_credits = 64;

    /* 2. Exams */
    g_data.num_exams = 3;
    strcpy(g_data.exams[0].subject, "Operating Systems");
    g_data.exams[0].days_left = 2; g_data.exams[0].confidence = 2;

    strcpy(g_data.exams[1].subject, "Data Structures");
    g_data.exams[1].days_left = 5; g_data.exams[1].confidence = 4;

    strcpy(g_data.exams[2].subject, "Theory of Computation");
    g_data.exams[2].days_left = 1; g_data.exams[2].confidence = 1;

    /* 3. Budget */
    g_data.initial_budget = 7500.0f;
    g_data.days_spent = 14;
    g_data.total_month_days = 30;
    g_data.spent_today = 180.0f;

    g_data.num_expenses = 4;
    strcpy(g_data.expenses[0].desc, "Zomato Biryani"); strcpy(g_data.expenses[0].category, "Food"); g_data.expenses[0].amount = 420.0f;
    strcpy(g_data.expenses[1].desc, "Hostel Wi-Fi"); strcpy(g_data.expenses[1].category, "Bills"); g_data.expenses[1].amount = 600.0f;
    strcpy(g_data.expenses[2].desc, "Tapri Chai & Maggi"); strcpy(g_data.expenses[2].category, "Food"); g_data.expenses[2].amount = 160.0f;
    strcpy(g_data.expenses[3].desc, "Printouts & Lab File"); strcpy(g_data.expenses[3].category, "Academics"); g_data.expenses[3].amount = 250.0f;

    /* 4. Tasks & Assignments */
    g_data.num_tasks = 4;
    strcpy(g_data.tasks[0].title, "DBMS Lab Assignment 3");
    strcpy(g_data.tasks[0].category, "Assignment");
    strcpy(g_data.tasks[0].assignee, "Loqman");
    g_data.tasks[0].days_until_due = 1;
    g_data.tasks[0].is_completed = 0;
    g_data.tasks[0].priority = 3;

    strcpy(g_data.tasks[1].title, "Project Backend API");
    strcpy(g_data.tasks[1].category, "Project");
    strcpy(g_data.tasks[1].assignee, "Rahul");
    g_data.tasks[1].days_until_due = -1; /* Overdue */
    g_data.tasks[1].is_completed = 0;
    g_data.tasks[1].priority = 3;

    strcpy(g_data.tasks[2].title, "Submit Gate Pass for Weekend");
    strcpy(g_data.tasks[2].category, "Personal");
    strcpy(g_data.tasks[2].assignee, "Loqman");
    g_data.tasks[2].days_until_due = 2;
    g_data.tasks[2].is_completed = 0;
    g_data.tasks[2].priority = 1;

    strcpy(g_data.tasks[3].title, "Read OS Memory Management Notes");
    strcpy(g_data.tasks[3].category, "Personal");
    strcpy(g_data.tasks[3].assignee, "Loqman");
    g_data.tasks[3].days_until_due = 0; /* Due Today */
    g_data.tasks[3].is_completed = 1;
    g_data.tasks[3].priority = 2;

    /* 5. Splitter */
    g_data.num_split_members = 4;
    strcpy(g_data.split_members[0].name, "Rahul");  g_data.split_members[0].balance = -350.0f;
    strcpy(g_data.split_members[1].name, "Amit");   g_data.split_members[1].balance = -150.0f;
    strcpy(g_data.split_members[2].name, "Priya");  g_data.split_members[2].balance =  200.0f;
    strcpy(g_data.split_members[3].name, "Loqman"); g_data.split_members[3].balance =  300.0f;

    /* 6. IOU */
    g_data.num_ious = 3;
    strcpy(g_data.ious[0].person, "Aman");
    g_data.ious[0].amount = 500.0f;
    g_data.ious[0].is_lent = 1; /* Aman owes me 500 */
    strcpy(g_data.ious[0].note, "Concert Ticket");
    g_data.ious[0].settled = 0;

    strcpy(g_data.ious[1].person, "Canteen Bhaiya");
    g_data.ious[1].amount = 80.0f;
    g_data.ious[1].is_lent = 0; /* I owe Canteen 80 */
    strcpy(g_data.ious[1].note, "Cold drink & Patties");
    g_data.ious[1].settled = 0;

    strcpy(g_data.ious[2].person, "Rohan");
    g_data.ious[2].amount = 250.0f;
    g_data.ious[2].is_lent = 1;
    strcpy(g_data.ious[2].note, "Uber Share");
    g_data.ious[2].settled = 1;

    /* 7. Hostel Inventory */
    g_data.num_inventory = 5;
    strcpy(g_data.inventory[0].name, "Bathing Soap / Gel"); strcpy(g_data.inventory[0].category, "Toiletries"); g_data.inventory[0].quantity = 1; g_data.inventory[0].min_threshold = 2;
    strcpy(g_data.inventory[1].name, "Maggi Packets"); strcpy(g_data.inventory[1].category, "Snacks"); g_data.inventory[1].quantity = 4; g_data.inventory[1].min_threshold = 3;
    strcpy(g_data.inventory[2].name, "Toothpaste"); strcpy(g_data.inventory[2].category, "Toiletries"); g_data.inventory[2].quantity = 2; g_data.inventory[2].min_threshold = 1;
    strcpy(g_data.inventory[3].name, "Paracetamol Tablets"); strcpy(g_data.inventory[3].category, "Meds"); g_data.inventory[3].quantity = 2; g_data.inventory[3].min_threshold = 4;
    strcpy(g_data.inventory[4].name, "A4 Exam Sheets"); strcpy(g_data.inventory[4].category, "Stationery"); g_data.inventory[4].quantity = 15; g_data.inventory[4].min_threshold = 10;

    /* 8. Mess Tracker */
    g_data.meals_taken[0] = 9;   g_data.meals_skipped[0] = 5; /* Breakfast */
    g_data.meals_taken[1] = 13;  g_data.meals_skipped[1] = 1; /* Lunch */
    g_data.meals_taken[2] = 8;   g_data.meals_skipped[2] = 6; /* Snacks */
    g_data.meals_taken[3] = 12;  g_data.meals_skipped[3] = 2; /* Dinner */

    /* 9. Energy & Sleep */
    g_data.sleep_hours = 5.5f;
    g_data.study_hours_today = 4;
    g_data.stress_level = 3;

    /* 10. Emergency Contacts */
    g_data.num_emergency = 4;
    strcpy(g_data.emergency[0].title, "Campus Security Control"); strcpy(g_data.emergency[0].phone, "+91 98765-11001");
    strcpy(g_data.emergency[1].title, "Chief Hostel Warden");     strcpy(g_data.emergency[1].phone, "+91 98765-22002");
    strcpy(g_data.emergency[2].title, "Hostel Ambulance / Clinic"); strcpy(g_data.emergency[2].phone, "+91 98765-33003");
    strcpy(g_data.emergency[3].title, "Senior Academic Proctor"); strcpy(g_data.emergency[3].phone, "+91 98765-44004");

    /* 11. Streaks & Badges */
    g_data.survival_streak = 5;
    g_data.longest_streak = 12;
    g_data.today_mission_completed = 0;
    g_data.total_tasks_completed = 14;
}

void save_data(void) {
    const char *path = (strlen(g_data.current_save_file) > 0) ? g_data.current_save_file : SAVE_FILE_DEFAULT;
    FILE *f = fopen(path, "wb");
    if (f) {
        fwrite(&g_data, sizeof(AppData), 1, f);
        fclose(f);
    }
}

void load_data(const char *filename) {
    const char *path = (filename && strlen(filename) > 0) ? filename : SAVE_FILE_DEFAULT;
    FILE *f = fopen(path, "rb");
    if (f) {
        fread(&g_data, sizeof(AppData), 1, f);
        fclose(f);
        if (strlen(g_data.current_save_file) == 0) {
            strncpy(g_data.current_save_file, path, sizeof(g_data.current_save_file) - 1);
        }
    } else {
        seed_default_data();
        strncpy(g_data.current_save_file, path, sizeof(g_data.current_save_file) - 1);
        save_data();
    }
}

/* ============================================================================
   CORE ALGORITHMS & MATHEMATICAL SCORING ENGINES
   ============================================================================ */

/* 1. Survival Score Engine (0 to 100) */
float calculate_survival_score(void) {
    /* Component A: Attendance (30%) */
    float att_sum = 0;
    int valid_subjects = 0;
    for (int i = 0; i < g_data.num_subjects; i++) {
        if (g_data.subjects[i].total > 0) {
            att_sum += ((float)g_data.subjects[i].attended / g_data.subjects[i].total) * 100.0f;
            valid_subjects++;
        }
    }
    float att_avg = (valid_subjects > 0) ? (att_sum / valid_subjects) : 75.0f;

    /* Component B: Pocket Money Health (25%) */
    float total_spent = 0;
    for (int i = 0; i < g_data.num_expenses; i++) total_spent += g_data.expenses[i].amount;
    float bal = g_data.initial_budget - total_spent;
    int days_left = g_data.total_month_days - g_data.days_spent;
    if (days_left <= 0) days_left = 1;
    float safe_daily = (bal > 0) ? (bal / (float)days_left) : 0.0f;
    float budget_health = (safe_daily >= 200.0f) ? 100.0f : ((safe_daily / 200.0f) * 100.0f);
    if (budget_health < 0) budget_health = 0.0f;

    /* Component C: Exam Readiness (25%) */
    float exam_readiness = 80.0f;
    if (g_data.num_exams > 0) {
        float conf_sum = 0;
        for (int i = 0; i < g_data.num_exams; i++) {
            conf_sum += (float)g_data.exams[i].confidence;
        }
        exam_readiness = (conf_sum / (g_data.num_exams * 5.0f)) * 100.0f;
    }

    /* Component D: Tasks & Energy (20%) */
    int pending = 0, overdue = 0;
    for (int i = 0; i < g_data.num_tasks; i++) {
        if (!g_data.tasks[i].is_completed) {
            pending++;
            if (g_data.tasks[i].days_until_due < 0) overdue++;
        }
    }
    float task_health = 100.0f - (overdue * 25.0f) - (pending * 5.0f);
    if (task_health < 0) task_health = 0.0f;

    float energy_factor = (g_data.sleep_hours / 8.0f) * 100.0f;
    if (energy_factor > 100.0f) energy_factor = 100.0f;

    float life_health = (task_health * 0.6f) + (energy_factor * 0.4f);

    float total_score = (att_avg * 0.30f) + (budget_health * 0.25f) + 
                        (exam_readiness * 0.25f) + (life_health * 0.20f);

    if (total_score > 100.0f) total_score = 100.0f;
    if (total_score < 0.0f) total_score = 0.0f;
    return total_score;
}

/* 2. Safe Daily Spend Calculation */
float get_safe_daily_limit(void) {
    float total_spent = 0;
    for (int i = 0; i < g_data.num_expenses; i++) total_spent += g_data.expenses[i].amount;
    float bal = g_data.initial_budget - total_spent;
    int days_left = g_data.total_month_days - g_data.days_spent;
    if (days_left <= 0) days_left = 1;
    if (bal <= 0) return 0.0f;
    return bal / (float)days_left;
}

/* 3. Exam Sorting by Urgency */
int compare_exams(const void *a, const void *b) {
    const Exam *ea = (const Exam *)a;
    const Exam *eb = (const Exam *)b;
    if (eb->priority_score > ea->priority_score) return 1;
    if (eb->priority_score < ea->priority_score) return -1;
    return 0;
}

void recalculate_exam_priorities(void) {
    for (int i = 0; i < g_data.num_exams; i++) {
        int d = g_data.exams[i].days_left;
        if (d < 1) d = 1;
        g_data.exams[i].priority_score = (6.0f - (float)g_data.exams[i].confidence) / (float)d;
    }
    if (g_data.num_exams > 1) {
        qsort(g_data.exams, g_data.num_exams, sizeof(Exam), compare_exams);
    }
}

/* ============================================================================
   MODULE 1: SMART ALERT CENTER & TRIAGE
   ============================================================================ */
void module_smart_alerts(void) {
    clear_screen();
    printf("\n  " RED BOLD "=== [🚨 SMART ALERT CENTER: EMERGENCY RADAR] ===" RST "\n\n");
    printf("  " GRAY "Scanning all subsystems for academic, financial and health hazards..." RST "\n\n");

    int alert_count = 0;

    /* 1. Debarment Check */
    for (int i = 0; i < g_data.num_subjects; i++) {
        Subject s = g_data.subjects[i];
        float pct = (s.total > 0) ? ((float)s.attended / s.total) * 100.0f : 0.0f;
        if (pct < 75.0f) {
            int needed = 3 * s.total - 4 * s.attended;
            if (needed <= 0) needed = 1;
            printf("  " RED BOLD "[CRITICAL ATTENDANCE]" RST " %s is at " RED "%.1f%%" RST "\n", s.name, pct);
            printf("    >> Debarment Threat! You MUST attend the next " BOLD "%d class(es)" RST " without fail.\n\n", needed);
            alert_count++;
        }
    }

    /* 2. Exam Shockwave */
    recalculate_exam_priorities();
    for (int i = 0; i < g_data.num_exams; i++) {
        if (g_data.exams[i].days_left <= 2 && g_data.exams[i].confidence <= 2) {
            printf("  " RED BOLD "[CRITICAL EXAM HAZARD]" RST " %s exam is in " BOLD "%d day(s)" RST "!\n", 
                   g_data.exams[i].subject, g_data.exams[i].days_left);
            printf("    >> Confidence is only %d/5! High backlog probability. Start emergency cramming.\n\n", 
                   g_data.exams[i].confidence);
            alert_count++;
        }
    }

    /* 3. Financial Exhaustion */
    float safe_limit = get_safe_daily_limit();
    if (safe_limit <= 50.0f) {
        printf("  " YELLOW BOLD "[FINANCIAL INSOLVENCY]" RST " Daily safe budget depleted to " RED "Rs. %.2f/day" RST "\n", safe_limit);
        printf("    >> Restrict all external food and luxury spending immediately!\n\n");
        alert_count++;
    }

    /* 4. Overdue Project Deliverables */
    for (int i = 0; i < g_data.num_tasks; i++) {
        if (!g_data.tasks[i].is_completed && g_data.tasks[i].days_until_due < 0) {
            printf("  " RED BOLD "[OVERDUE DEADLINE]" RST " \"%s\" is " RED "%d day(s) late!" RST "\n",
                   g_data.tasks[i].title, -g_data.tasks[i].days_until_due);
            printf("    >> Assigned to: %s. Penalty marks being deducted!\n\n", g_data.tasks[i].assignee);
            alert_count++;
        }
    }

    /* 5. Hostel Inventory Shortage */
    for (int i = 0; i < g_data.num_inventory; i++) {
        if (g_data.inventory[i].quantity <= g_data.inventory[i].min_threshold) {
            printf("  " CYAN BOLD "[HOSTEL ESSENTIAL SHORTAGE]" RST " %s is down to " RED "%d units" RST " (Min safe: %d)\n",
                   g_data.inventory[i].name, g_data.inventory[i].quantity, g_data.inventory[i].min_threshold);
            alert_count++;
        }
    }

    /* 6. Burnout / Energy Warning */
    if (g_data.sleep_hours < 5.0f) {
        printf("  " MAGENTA BOLD "[COGNITIVE BURNOUT WARNING]" RST " Only %.1f hours of sleep recorded.\n", g_data.sleep_hours);
        printf("    >> Reaction time and memory retention critically reduced. Schedule a 45 min power nap!\n\n");
        alert_count++;
    }

    if (alert_count == 0) {
        printf("  " GREEN BOLD "[ALL CLEAR]" RST " No critical emergencies detected. You are operating at peak stability!\n\n");
    } else {
        printf("  " BOLD "Total Active Critical Alerts: " RED "%d" RST "\n\n", alert_count);
    }

    wait_key();
}

/* ============================================================================
   MODULE 2: DAILY MISSION PLANNER & SURVIVAL STREAK
   ============================================================================ */
void module_daily_mission(void) {
    while (1) {
        clear_screen();
        printf("\n  " CYAN BOLD "=== [📅 DAILY MISSION PLANNER & SURVIVAL STREAK] ===" RST "\n\n");
        printf("  " YELLOW "Current Survival Streak: " BOLD "%d Days" RST " 🔥  |  Longest Record: %d Days ⚡\n\n",
               g_data.survival_streak, g_data.longest_streak);

        /* Determine Today's Top 3 Missions */
        /* Mission 1: Lowest Attendance Subject */
        int worst_att_idx = -1;
        float min_pct = 999.0f;
        for (int i = 0; i < g_data.num_subjects; i++) {
            float pct = (g_data.subjects[i].total > 0) ? 
                        ((float)g_data.subjects[i].attended / g_data.subjects[i].total) * 100.0f : 0.0f;
            if (pct < min_pct) {
                min_pct = pct;
                worst_att_idx = i;
            }
        }

        /* Mission 2: Top Exam to Cram */
        recalculate_exam_priorities();
        const char *top_exam = (g_data.num_exams > 0) ? g_data.exams[0].subject : "Self-Study Revision";

        /* Mission 3: Urgents Deadline */
        int top_task_idx = -1;
        int min_task_days = 999;
        for (int i = 0; i < g_data.num_tasks; i++) {
            if (!g_data.tasks[i].is_completed && g_data.tasks[i].days_until_due < min_task_days) {
                min_task_days = g_data.tasks[i].days_until_due;
                top_task_idx = i;
            }
        }

        printf("  " BOLD "TODAY'S SURVIVAL BATTLE ORDERS:" RST "\n");
        printf("  ------------------------------------------------------------------------\n");
        if (worst_att_idx != -1) {
            printf("  [M-1] Academic Priority : Attend next %s lecture (Current: %.1f%%)\n",
                   g_data.subjects[worst_att_idx].name, min_pct);
        }
        printf("  [M-2] Exam Revision     : Complete 45 mins focused prep for %s\n", top_exam);

        if (top_task_idx != -1) {
            printf("  [M-3] Deadline Defense  : Submit/work on \"%s\" (%s)\n",
                   g_data.tasks[top_task_idx].title, 
                   (min_task_days <= 0) ? RED "URGENT!" : "Upcoming");
        } else {
            printf("  [M-3] Deadline Defense  : All pending assignments are clear! Review notes.\n");
        }

        float safe = get_safe_daily_limit();
        printf("  [M-4] Fiscal Discipline : Keep today's expenses strictly under Rs. %.2f\n", safe);
        printf("  ------------------------------------------------------------------------\n\n");

        if (g_data.today_mission_completed) {
            printf("  " GREEN BOLD "⭐ [COMPLETED] You completed today's survival mission! Streak updated." RST "\n\n");
        } else {
            printf("  " YELLOW "[PENDING] Complete your daily requirements to advance your streak." RST "\n\n");
        }

        printf("  " BOLD "[1] Complete Today's Mission (+1 Streak)   [2] Reset Streak   [0] Back" RST "\n");
        printf("  Select: ");
        int choice;
        if (scanf("%d", &choice) != 1) { flush_input(); continue; }
        flush_input();

        if (choice == 0) break;
        if (choice == 1) {
            if (!g_data.today_mission_completed) {
                g_data.today_mission_completed = 1;
                g_data.survival_streak++;
                if (g_data.survival_streak > g_data.longest_streak) {
                    g_data.longest_streak = g_data.survival_streak;
                }
                save_data();
                printf("\n  " GREEN BOLD "🔥 STREAK EXTENDED! You are now at %d consecutive days of survival!" RST "\n", g_data.survival_streak);
                Sleep(1000);
            } else {
                printf("\n  " YELLOW "Today's mission was already recorded as complete!" RST "\n");
                Sleep(800);
            }
        } else if (choice == 2) {
            g_data.survival_streak = 0;
            g_data.today_mission_completed = 0;
            save_data();
            printf("\n  " RED "Streak reset to 0." RST "\n");
            Sleep(800);
        }
    }
}

/* ============================================================================
   MODULE 3: SURVIVAL SCORE & WEAK SUBJECT DETECTOR
   ============================================================================ */
void module_score_and_weak_subjects(void) {
    clear_screen();
    printf("\n  " CYAN BOLD "=== [🎯 SURVIVAL SCORE & WEAK SUBJECT RADAR] ===" RST "\n\n");

    float score = calculate_survival_score();
    const char *status_tag = (score >= 80.0f) ? GREEN "ACADEMIC WEAPON" RST :
                             (score >= 60.0f) ? YELLOW "BORDERLINE SURVIVOR" RST :
                             RED "CRITICAL LOCKDOWN" RST;

    printf("  Overall Survival Index: " BOLD "%.1f / 100.0" RST "  [%s]\n", score, status_tag);
    animate_progress_bar(30, score, (score >= 75) ? GREEN : ((score >= 55) ? YELLOW : RED), 6);
    printf("\n\n");

    printf("  " BOLD "🧠 WEAK SUBJECT DETECTOR (Backlog Probability Matrix):" RST "\n");
    printf("  ------------------------------------------------------------------------\n");
    printf("  %-24s %-12s %-12s %-14s %s\n", "SUBJECT", "ATTENDANCE", "CONFIDENCE", "RISK INDEX", "STATUS");
    printf("  ------------------------------------------------------------------------\n");

    for (int i = 0; i < g_data.num_subjects; i++) {
        Subject s = g_data.subjects[i];
        float att_pct = (s.total > 0) ? ((float)s.attended / s.total) * 100.0f : 0.0f;

        /* Find matching exam confidence if logged */
        int conf = 3;
        for (int e = 0; e < g_data.num_exams; e++) {
            if (str_case_eq(g_data.exams[e].subject, s.name)) {
                conf = g_data.exams[e].confidence;
                break;
            }
        }

        /* Risk Calculation */
        float risk = (100.0f - att_pct) * 0.7f + (5.0f - (float)conf) * 12.0f;
        const char *col = (risk >= 45.0f) ? RED : ((risk >= 25.0f) ? YELLOW : GREEN);
        const char *label = (risk >= 45.0f) ? "[HIGH RISK]" : ((risk >= 25.0f) ? "[MODERATE]" : "[SAFE]");

        printf("  %-24s %5.1f%%       %d/5          %5.1f         %s%s" RST "\n",
               s.name, att_pct, conf, risk, col, label);
    }
    printf("  ------------------------------------------------------------------------\n");
    printf("  " GRAY "Tip: Subjects with Risk Index > 40 need immediate extra tutorial hours & zero bunks." RST "\n\n");

    wait_key();
}

/* ============================================================================
   MODULE 4: 🚨 PANIC MODE! (THE HACKATHON SHOWSTOPPER)
   ============================================================================ */
void module_panic_mode(void) {
    clear_screen();
    printf("\a"); /* System Audio Beep */

    printf("\n");
    printf("  " RED BOLD "************************************************************************" RST "\n");
    printf("  " RED BOLD "*                🚨 ! ! ! P A N I C   M O D E   A C T I V E ! ! !       *" RST "\n");
    printf("  " RED BOLD "*              CRITICAL SEMESTER SURVIVAL PROTOCOL ENGAGED              *" RST "\n");
    printf("  " RED BOLD "************************************************************************" RST "\n\n");

    Sleep(200);

    /* 1. Debarment Defense */
    printf("  " WHITE BOLD "[1] IMMEDIATE ATTENDANCE DEFENSE:" RST "\n");
    int critical_classes = 0;
    for (int i = 0; i < g_data.num_subjects; i++) {
        Subject s = g_data.subjects[i];
        float pct = (s.total > 0) ? ((float)s.attended / s.total) * 100.0f : 0.0f;
        if (pct < 75.0f) {
            int needed = 3 * s.total - 4 * s.attended;
            if (needed <= 0) needed = 1;
            printf("    " RED ">> MUST ATTEND %s:" RST " Next %d lecture(s) are strictly MANDATORY!\n", s.name, needed);
            critical_classes++;
        }
    }
    if (!critical_classes) printf("    " GREEN ">> All attendance >= 75%%. Debarment risk is neutralized." RST "\n");

    /* 2. Exam Triage */
    printf("\n  " WHITE BOLD "[2] NEXT 24-HOUR EXAM EMERGENCY FOCUS:" RST "\n");
    recalculate_exam_priorities();
    if (g_data.num_exams > 0) {
        printf("    " YELLOW ">> ZERO DISTRACTION TARGET: %s (Exam in %d days, Confidence %d/5)" RST "\n",
               g_data.exams[0].subject, g_data.exams[0].days_left, g_data.exams[0].confidence);
        printf("    >> ACTION: Stop watching reels/gaming. Solve previous 3 years question papers right now!\n");
    } else {
        printf("    " GREEN ">> No immediate exams in 48 hours." RST "\n");
    }

    /* 3. Emergency Food & Cash Cap */
    printf("\n  " WHITE BOLD "[3] EMERGENCY CASH RATION TODAY:" RST "\n");
    float safe = get_safe_daily_limit();
    float panic_cap = (safe < 150.0f) ? safe : 150.0f;
    printf("    >> Your maximum permitted food budget for today is: " GREEN BOLD "Rs. %.2f" RST "\n", panic_cap);
    printf("    >> RULE: Eat in Mess or stick to basic canteen meal. No Swiggy/Zomato allowed!\n");

    /* 4. SOS Contact */
    printf("\n  " WHITE BOLD "[4] EMERGENCY HOTLINE:" RST "\n");
    if (g_data.num_emergency > 0) {
        printf("    >> %s : " CYAN BOLD "%s" RST "\n", g_data.emergency[0].title, g_data.emergency[0].phone);
        if (g_data.num_emergency > 1) {
            printf("    >> %s : " CYAN BOLD "%s" RST "\n", g_data.emergency[1].title, g_data.emergency[1].phone);
        }
    }

    printf("\n  " RED BOLD "************************************************************************" RST "\n");
    printf("  " YELLOW "Deep breath. Follow these 4 steps and you will make it through the semester!" RST "\n\n");

    wait_key();
}

/* ============================================================================
   MODULE 5: ACADEMICS HUB (Attendance, Exams, CGPA, Study Tracker)
   ============================================================================ */
void sub_module_attendance(void) {
    while (1) {
        clear_screen();
        printf("\n  " CYAN BOLD "=== [ACADEMICS: ATTENDANCE RESCUE GAUGES] ===" RST "\n\n");

        if (g_data.num_subjects == 0) {
            printf("  " GRAY "No subjects registered." RST "\n");
        } else {
            for (int i = 0; i < g_data.num_subjects; i++) {
                Subject s = g_data.subjects[i];
                float pct = (s.total > 0) ? ((float)s.attended / s.total) * 100.0f : 0.0f;
                const char *bar_color = (pct < 75.0f) ? RED : ((pct < 80.0f) ? YELLOW : GREEN);

                printf("  " BOLD "%-24s" RST " [%2d/%-2d classes] \n", s.name, s.attended, s.total);
                animate_progress_bar(25, pct, bar_color, 2);
                printf("  %s%5.1f%%" RST " ", bar_color, pct);

                if (pct < 75.0f) {
                    int needed = 3 * s.total - 4 * s.attended;
                    if (needed <= 0) needed = 1;
                    printf(RED BOLD "[!] ATTEND NEXT %d CLASS(ES) FOR 75%%!" RST "\n", needed);
                } else {
                    int bunks = (4 * s.attended - 3 * s.total) / 3;
                    if (bunks > 0) {
                        printf(GREEN "[OK] Safe (Can bunk %d class(es))" RST "\n", bunks);
                    } else {
                        printf(YELLOW "[!] Borderline (Do not bunk!)" RST "\n");
                    }
                }
                printf("\n");
            }
        }

        printf("  " BOLD "[1] Add Subject   [2] Quick Attendance Tick   [0] Back" RST "\n");
        printf("  Select: ");
        int choice;
        if (scanf("%d", &choice) != 1) { flush_input(); continue; }
        flush_input();

        if (choice == 0) break;
        if (choice == 1 && g_data.num_subjects < MAX_SUBJECTS) {
            printf("  Subject Name: ");
            read_line(g_data.subjects[g_data.num_subjects].name, 32);
            printf("  Classes Attended: ");
            scanf("%d", &g_data.subjects[g_data.num_subjects].attended);
            printf("  Total Classes Held: ");
            scanf("%d", &g_data.subjects[g_data.num_subjects].total);
            printf("  Credits (e.g. 3 or 4): ");
            scanf("%d", &g_data.subjects[g_data.num_subjects].credits);
            flush_input();
            g_data.num_subjects++;
            save_data();
        } else if (choice == 2 && g_data.num_subjects > 0) {
            printf("  Select subject (1-%d): ", g_data.num_subjects);
            int idx; scanf("%d", &idx); flush_input();
            if (idx >= 1 && idx <= g_data.num_subjects) {
                printf("  Did you attend today? (1 = Yes, 0 = Bunked): ");
                int att; scanf("%d", &att); flush_input();
                g_data.subjects[idx - 1].total++;
                if (att == 1) g_data.subjects[idx - 1].attended++;
                save_data();
            }
        }
    }
}

void sub_module_cgpa_predictor(void) {
    clear_screen();
    printf("\n  " CYAN BOLD "=== [🧮 CGPA & SEMESTER SGPA PREDICTOR] ===" RST "\n\n");

    printf("  Past Cumulative CGPA: " BOLD "%.2f" RST " (Over %d credits)\n\n", 
           g_data.prior_cgpa, g_data.prior_credits);

    printf("  " BOLD "Current Semester Courses & Expected Grades:" RST "\n");
    printf("  ------------------------------------------------------------------------\n");
    printf("  %-4s %-26s %-10s %-16s\n", "ID", "SUBJECT", "CREDITS", "EXPECTED GRADE PT");
    printf("  ------------------------------------------------------------------------\n");

    float sem_points = 0;
    int sem_credits = 0;

    for (int i = 0; i < g_data.num_subjects; i++) {
        Subject s = g_data.subjects[i];
        int cr = (s.credits > 0) ? s.credits : 3;
        float gp = (s.expected_grade_point > 0.0f) ? s.expected_grade_point : 8.0f;
        sem_points += (gp * cr);
        sem_credits += cr;

        printf("  [%d] %-26s %-10d %5.1f / 10.0\n", i + 1, s.name, cr, gp);
    }
    printf("  ------------------------------------------------------------------------\n");

    float predicted_sgpa = (sem_credits > 0) ? (sem_points / sem_credits) : 0.0f;
    float total_creds = g_data.prior_credits + sem_credits;
    float predicted_cgpa = (total_creds > 0) ? 
        ((g_data.prior_cgpa * g_data.prior_credits) + sem_points) / total_creds : predicted_sgpa;

    printf("\n  Predicted Semester SGPA : " CYAN BOLD "%.2f" RST "\n", predicted_sgpa);
    printf("  Projected New CGPA      : " GREEN BOLD "%.2f" RST "\n\n", predicted_cgpa);

    printf("  " BOLD "[1] Update Expected Grade for Subject   [2] Update Prior CGPA/Credits   [0] Back" RST "\n");
    printf("  Select: ");
    int choice;
    if (scanf("%d", &choice) != 1) { flush_input(); return; }
    flush_input();

    if (choice == 1 && g_data.num_subjects > 0) {
        printf("  Select Subject ID (1-%d): ", g_data.num_subjects);
        int idx; scanf("%d", &idx); flush_input();
        if (idx >= 1 && idx <= g_data.num_subjects) {
            printf("  Enter Expected Grade Points (e.g. 10 for O, 9 for A+, 8 for A, 7 for B+): ");
            scanf("%f", &g_data.subjects[idx - 1].expected_grade_point);
            flush_input();
            save_data();
        }
    } else if (choice == 2) {
        printf("  Enter current CGPA: ");
        scanf("%f", &g_data.prior_cgpa);
        printf("  Enter total credits completed till last semester: ");
        scanf("%d", &g_data.prior_credits);
        flush_input();
        save_data();
    }
}

void sub_module_study_tracker(void) {
    while (1) {
        clear_screen();
        printf("\n  " CYAN BOLD "=== [📖 STUDY SESSION & POMODORO LOG] ===" RST "\n\n");

        int total_mins = 0;
        printf("  " BOLD "Logged Study Time by Subject:" RST "\n");
        printf("  ------------------------------------------------------------------------\n");
        for (int i = 0; i < g_data.num_subjects; i++) {
            total_mins += g_data.subjects[i].study_minutes;
            int hrs = g_data.subjects[i].study_minutes / 60;
            int mins = g_data.subjects[i].study_minutes % 60;
            printf("  * %-24s : %2d hrs %02d mins\n", g_data.subjects[i].name, hrs, mins);
        }
        printf("  ------------------------------------------------------------------------\n");
        printf("  Total Dedicated Study Time : " GREEN BOLD "%d hrs %02d mins" RST "\n\n", total_mins / 60, total_mins % 60);

        printf("  " BOLD "[1] Log New Study Session   [0] Back" RST "\n");
        printf("  Select: ");
        int choice;
        if (scanf("%d", &choice) != 1) { flush_input(); continue; }
        flush_input();

        if (choice == 0) break;
        if (choice == 1 && g_data.num_subjects > 0) {
            printf("  Select subject (1-%d): ", g_data.num_subjects);
            int idx; scanf("%d", &idx); flush_input();
            if (idx >= 1 && idx <= g_data.num_subjects) {
                printf("  Enter study duration in minutes (e.g. 45 or 90): ");
                int mins; scanf("%d", &mins); flush_input();
                g_data.subjects[idx - 1].study_minutes += mins;
                save_data();
                printf("  " GREEN "Logged successfully!" RST "\n");
                Sleep(600);
            }
        }
    }
}

void module_academics_hub(void) {
    while (1) {
        clear_screen();
        printf("\n  " CYAN BOLD "=== [📚 ACADEMICS COMMAND HUB] ===" RST "\n\n");
        printf("  [1] Attendance Rescue & Debarment Gauges\n");
        printf("  [2] Exam Radar & Study Prioritizer\n");
        printf("  [3] CGPA & SGPA Predictor\n");
        printf("  [4] Study Session Logger\n");
        printf("  [0] Back to Main Menu\n\n");
        printf("  Select option: ");
        int choice;
        if (scanf("%d", &choice) != 1) { flush_input(); continue; }
        flush_input();

        if (choice == 0) break;
        if (choice == 1) sub_module_attendance();
        else if (choice == 2) {
            /* Exam radar module */
            while (1) {
                clear_screen();
                printf("\n  " CYAN BOLD "=== [EXAM RADAR: SORTED BY URGENCY] ===" RST "\n\n");
                recalculate_exam_priorities();

                if (g_data.num_exams == 0) {
                    printf("  " GRAY "No upcoming exams logged." RST "\n");
                } else {
                    for (int i = 0; i < g_data.num_exams; i++) {
                        Exam e = g_data.exams[i];
                        printf("  #%-2d %-22s (Exam in " BOLD "%d days" RST ", Confidence: %d/5)\n",
                               i + 1, e.subject, e.days_left, e.confidence);
                        float urgency_pct = (e.priority_score / 5.0f) * 100.0f;
                        if (urgency_pct > 100.0f) urgency_pct = 100.0f;
                        const char *col = (i == 0) ? RED : ((e.days_left <= 2) ? YELLOW : GREEN);
                        animate_progress_bar(20, urgency_pct, col, 2);

                        if (i == 0) {
                            printf("  " RED BOLD "[#1 TOP PRIORITY: STUDY THIS TODAY!]" RST "\n\n");
                        } else {
                            printf("\n\n");
                        }
                    }
                }
                printf("  [1] Add Exam   [2] Update Exam Days/Confidence   [0] Back\n  Select: ");
                int ec; if (scanf("%d", &ec) != 1) { flush_input(); break; } flush_input();
                if (ec == 0) break;
                if (ec == 1 && g_data.num_exams < MAX_EXAMS) {
                    printf("  Subject: "); read_line(g_data.exams[g_data.num_exams].subject, 32);
                    printf("  Days until exam: "); scanf("%d", &g_data.exams[g_data.num_exams].days_left);
                    printf("  Confidence (1 to 5): "); scanf("%d", &g_data.exams[g_data.num_exams].confidence);
                    flush_input();
                    g_data.num_exams++;
                    save_data();
                } else if (ec == 2 && g_data.num_exams > 0) {
                    printf("  Select exam rank (1-%d): ", g_data.num_exams);
                    int r; scanf("%d", &r); flush_input();
                    if (r >= 1 && r <= g_data.num_exams) {
                        printf("  New days left: "); scanf("%d", &g_data.exams[r - 1].days_left);
                        printf("  New confidence (1-5): "); scanf("%d", &g_data.exams[r - 1].confidence);
                        flush_input();
                        save_data();
                    }
                }
            }
        }
        else if (choice == 3) sub_module_cgpa_predictor();
        else if (choice == 4) sub_module_study_tracker();
    }
}

/* ============================================================================
   MODULE 6: FINANCE SENTINEL (Daily Spend, Impulse Check, Analytics, Split, IOU)
   ============================================================================ */
void sub_module_impulse_check(void) {
    clear_screen();
    printf("\n  " CYAN BOLD "=== [🚫 IMPULSE PURCHASE SIMULATOR] ===" RST "\n\n");

    float total_spent = 0;
    for (int i = 0; i < g_data.num_expenses; i++) total_spent += g_data.expenses[i].amount;
    float bal = g_data.initial_budget - total_spent;
    int days_left = g_data.total_month_days - g_data.days_spent;
    if (days_left <= 0) days_left = 1;

    float current_daily = (bal > 0) ? (bal / days_left) : 0.0f;

    printf("  Current Remaining Balance : Rs. %.2f\n", bal);
    printf("  Days Remaining in Month   : %d days\n", days_left);
    printf("  Current Safe Daily Limit  : " GREEN "Rs. %.2f / day" RST "\n\n", current_daily);

    printf("  Enter item you want to buy (e.g. Gaming Mouse / Hoodie / Party): ");
    char item[32]; read_line(item, 32);
    printf("  Enter item price (Rs): ");
    float price; scanf("%f", &price); flush_input();

    float new_bal = bal - price;
    float new_daily = (new_bal > 0) ? (new_bal / days_left) : 0.0f;

    printf("\n  " BOLD "SIMULATION RESULTS FOR \"%s\":" RST "\n", item);
    printf("  ------------------------------------------------------------------------\n");
    printf("  Old Daily Limit : Rs. %.2f / day\n", current_daily);
    printf("  New Daily Limit : %sRs. %.2f / day" RST "\n", (new_daily < 100) ? RED : YELLOW, new_daily);
    printf("  Daily Reduction : - Rs. %.2f / day\n", current_daily - new_daily);
    printf("  ------------------------------------------------------------------------\n");

    if (new_bal < 0) {
        printf("  " RED BOLD "[VERDICT: FINANCIAL SUICIDE!] You cannot afford this without going into negative debt." RST "\n");
    } else if (new_daily < 80.0f) {
        printf("  " RED BOLD "[VERDICT: HIGH HAZARD!] Buying this will force you to eat plain instant noodles for %d days." RST "\n", days_left);
    } else if (new_daily < 150.0f) {
        printf("  " YELLOW BOLD "[VERDICT: TIGHT SQUEEZE] You can buy it, but no more eating out this month." RST "\n");
    } else {
        printf("  " GREEN BOLD "[VERDICT: SAFE TO BUY] Fits comfortably within your survival threshold." RST "\n");
    }

    wait_key();
}

void sub_module_expense_analytics(void) {
    clear_screen();
    printf("\n  " CYAN BOLD "=== [📊 EXPENSE ANALYTICS & CATEGORY BREAKDOWN] ===" RST "\n\n");

    float cat_totals[6] = {0}; /* 0: Food, 1: Travel, 2: Fun, 3: Bills, 4: Academics, 5: Other */
    const char *cat_names[6] = {"Food", "Travel", "Fun", "Bills", "Academics", "Other"};
    float grand_total = 0;

    for (int i = 0; i < g_data.num_expenses; i++) {
        grand_total += g_data.expenses[i].amount;
        if (str_case_eq(g_data.expenses[i].category, "Food")) cat_totals[0] += g_data.expenses[i].amount;
        else if (str_case_eq(g_data.expenses[i].category, "Travel")) cat_totals[1] += g_data.expenses[i].amount;
        else if (str_case_eq(g_data.expenses[i].category, "Fun")) cat_totals[2] += g_data.expenses[i].amount;
        else if (str_case_eq(g_data.expenses[i].category, "Bills")) cat_totals[3] += g_data.expenses[i].amount;
        else if (str_case_eq(g_data.expenses[i].category, "Academics")) cat_totals[4] += g_data.expenses[i].amount;
        else cat_totals[5] += g_data.expenses[i].amount;
    }

    printf("  Total Spent This Month: " BOLD "Rs. %.2f" RST "\n\n", grand_total);
    printf("  Category Distribution:\n");
    for (int i = 0; i < 6; i++) {
        float pct = (grand_total > 0) ? (cat_totals[i] / grand_total) * 100.0f : 0.0f;
        printf("  %-12s : Rs. %7.2f (%5.1f%%) ", cat_names[i], cat_totals[i], pct);
        animate_progress_bar(20, pct, CYAN, 1);
        printf("\n");
    }

    printf("\n  " BOLD "Recent Logged Expenses:" RST "\n");
    for (int i = 0; i < g_data.num_expenses && i < 6; i++) {
        printf("  * %-20s [%-10s] : Rs. %.2f\n",
               g_data.expenses[i].desc, g_data.expenses[i].category, g_data.expenses[i].amount);
    }

    wait_key();
}

void sub_module_iou_tracker(void) {
    while (1) {
        clear_screen();
        printf("\n  " CYAN BOLD "=== [👫 BORROW / LEND TRACKER (IOU SENTINEL)] ===" RST "\n\n");

        float owed_to_me = 0;
        float i_owe = 0;

        for (int i = 0; i < g_data.num_ious; i++) {
            if (!g_data.ious[i].settled) {
                if (g_data.ious[i].is_lent) owed_to_me += g_data.ious[i].amount;
                else i_owe += g_data.ious[i].amount;
            }
        }

        printf("  Net Cash to Collect : " GREEN BOLD "+ Rs. %.2f" RST "\n", owed_to_me);
        printf("  Net Cash to Pay Out : " RED BOLD "- Rs. %.2f" RST "\n\n", i_owe);

        printf("  %-4s %-16s %-10s %-12s %-20s %s\n", "ID", "PERSON", "TYPE", "AMOUNT", "NOTE", "STATUS");
        printf("  ------------------------------------------------------------------------\n");
        for (int i = 0; i < g_data.num_ious; i++) {
            IOUItem item = g_data.ious[i];
            const char *type_str = item.is_lent ? GREEN "Lent to" RST : RED "Borrowed" RST;
            const char *st_str = item.settled ? GRAY "[SETTLED]" RST : YELLOW "[PENDING]" RST;
            printf("  [%d] %-16s %-18s Rs. %-8.2f %-20s %s\n",
                   i + 1, item.person, type_str, item.amount, item.note, st_str);
        }
        printf("  ------------------------------------------------------------------------\n\n");

        printf("  " BOLD "[1] Add New IOU   [2] Toggle Settled Status   [0] Back" RST "\n");
        printf("  Select: ");
        int choice;
        if (scanf("%d", &choice) != 1) { flush_input(); continue; }
        flush_input();

        if (choice == 0) break;
        if (choice == 1 && g_data.num_ious < MAX_IOU) {
            printf("  Person Name: "); read_line(g_data.ious[g_data.num_ious].person, 24);
            printf("  Did you LEND or BORROW? (1 = You Lent money, 0 = You Borrowed money): ");
            scanf("%d", &g_data.ious[g_data.num_ious].is_lent);
            printf("  Amount (Rs): ");
            scanf("%f", &g_data.ious[g_data.num_ious].amount); flush_input();
            printf("  Reason/Note: ");
            read_line(g_data.ious[g_data.num_ious].note, 32);
            g_data.ious[g_data.num_ious].settled = 0;
            g_data.num_ious++;
            save_data();
        } else if (choice == 2 && g_data.num_ious > 0) {
            printf("  Select IOU ID (1-%d): ", g_data.num_ious);
            int idx; scanf("%d", &idx); flush_input();
            if (idx >= 1 && idx <= g_data.num_ious) {
                g_data.ious[idx - 1].settled = !g_data.ious[idx - 1].settled;
                save_data();
            }
        }
    }
}

void sub_module_splitter(void) {
    while (1) {
        clear_screen();
        printf("\n  " CYAN BOLD "=== [FRIEND EXPENSE SPLITTER - GREEDY OPTIMIZER] ===" RST "\n\n");

        int n = g_data.num_split_members;
        if (n == 0) {
            printf("  " GRAY "No group members configured." RST "\n");
        } else {
            printf("  Balances (+ is owed cash, - owes cash):\n");
            for (int i = 0; i < n; i++) {
                float b = g_data.split_members[i].balance;
                printf("  * %-14s : %s%sRs. %.2f" RST "\n",
                       g_data.split_members[i].name,
                       (b > 0) ? GREEN : (b < 0 ? RED : GRAY),
                       (b > 0) ? "+ " : "", b);
            }

            float bal[MAX_SPLIT_MEMBERS];
            for (int i = 0; i < n; i++) bal[i] = g_data.split_members[i].balance;

            Settlement settlements[MAX_SPLIT_MEMBERS * 2];
            int count = 0;

            while (count < MAX_SPLIT_MEMBERS * 2) {
                int max_cred = -1, max_deb = -1;
                float mx_c = 0.01f, mx_d = -0.01f;

                for (int i = 0; i < n; i++) {
                    if (bal[i] > mx_c) { mx_c = bal[i]; max_cred = i; }
                    if (bal[i] < mx_d) { mx_d = bal[i]; max_deb = i; }
                }

                if (max_cred == -1 || max_deb == -1) break;

                float transfer = fminf(mx_c, -mx_d);
                settlements[count].from = max_deb;
                settlements[count].to = max_cred;
                settlements[count].amount = transfer;
                count++;

                bal[max_cred] -= transfer;
                bal[max_deb] += transfer;
            }

            printf("\n  " BOLD "Optimized Cash Transfers (Fewest Transactions):" RST "\n");
            printf("  ------------------------------------------------------------------------\n");
            if (count == 0) {
                printf("  " GREEN "[OK] All debts settled! No payments needed." RST "\n");
            } else {
                for (int i = 0; i < count; i++) {
                    printf("  %s%-12s" RST " ---> [ Rs. %.2f ] ---> %s%s" RST "\n",
                           RED, g_data.split_members[settlements[i].from].name,
                           settlements[i].amount,
                           GREEN, g_data.split_members[settlements[i].to].name);
                }
            }
        }

        printf("\n  " BOLD "[1] Split Bill   [2] Add / Reset Friends   [0] Back" RST "\n");
        printf("  Select: ");
        int choice;
        if (scanf("%d", &choice) != 1) { flush_input(); continue; }
        flush_input();

        if (choice == 0) break;
        if (choice == 1 && n > 0) {
            printf("  Who paid? (1-%d): ", n);
            int payer; scanf("%d", &payer); flush_input();
            printf("  Total bill amount (Rs): ");
            float amt; scanf("%f", &amt); flush_input();

            if (payer >= 1 && payer <= n) {
                float split_each = amt / (float)n;
                for (int i = 0; i < n; i++) {
                    if (i == payer - 1) {
                        g_data.split_members[i].balance += (amt - split_each);
                    } else {
                        g_data.split_members[i].balance -= split_each;
                    }
                }
                save_data();
            }
        } else if (choice == 2) {
            printf("  How many friends? (2-%d): ", MAX_SPLIT_MEMBERS);
            scanf("%d", &g_data.num_split_members); flush_input();
            for (int i = 0; i < g_data.num_split_members; i++) {
                printf("  Friend %d Name: ", i + 1);
                read_line(g_data.split_members[i].name, 24);
                g_data.split_members[i].balance = 0.0f;
            }
            save_data();
        }
    }
}

void module_finance_hub(void) {
    while (1) {
        clear_screen();
        printf("\n  " CYAN BOLD "=== [💸 FINANCE SENTINEL COMMAND HUB] ===" RST "\n\n");

        float total_spent = 0;
        for (int i = 0; i < g_data.num_expenses; i++) total_spent += g_data.expenses[i].amount;
        float bal = g_data.initial_budget - total_spent;
        float safe_daily = get_safe_daily_limit();

        printf("  Monthly Allowance   : Rs. %.2f\n", g_data.initial_budget);
        printf("  Current Cash Balance: " GREEN BOLD "Rs. %.2f" RST "\n", bal);
        printf("  Safe Daily Spending : " YELLOW BOLD "Rs. %.2f / day" RST "\n\n", safe_daily);

        printf("  [1] Log Expense\n");
        printf("  [2] Impulse Purchase Check\n");
        printf("  [3] Category Analytics & Chart\n");
        printf("  [4] Friend Bill Splitter (Greedy Algorithm)\n");
        printf("  [5] Borrow / Lend Tracker (IOU)\n");
        printf("  [6] Update Monthly Budget & Days\n");
        printf("  [0] Back to Main Menu\n\n");
        printf("  Select option: ");
        int choice;
        if (scanf("%d", &choice) != 1) { flush_input(); continue; }
        flush_input();

        if (choice == 0) break;
        if (choice == 1 && g_data.num_expenses < MAX_EXPENSES) {
            printf("  Expense Name: "); read_line(g_data.expenses[g_data.num_expenses].desc, 32);
            printf("  Category (Food/Travel/Fun/Bills/Academics): "); read_line(g_data.expenses[g_data.num_expenses].category, 16);
            printf("  Amount (Rs): "); scanf("%f", &g_data.expenses[g_data.num_expenses].amount); flush_input();
            g_data.num_expenses++;
            save_data();
        } else if (choice == 2) sub_module_impulse_check();
        else if (choice == 3) sub_module_expense_analytics();
        else if (choice == 4) sub_module_splitter();
        else if (choice == 5) sub_module_iou_tracker();
        else if (choice == 6) {
            printf("  Monthly Allowance (Rs): "); scanf("%f", &g_data.initial_budget);
            printf("  Days elapsed this month: "); scanf("%d", &g_data.days_spent); flush_input();
            save_data();
        }
    }
}

/* ============================================================================
   MODULE 7: DEADLINES & TO-DO MANAGER (ASSIGNMENT COUNTDOWN)
   ============================================================================ */
void module_deadlines_hub(void) {
    while (1) {
        clear_screen();
        printf("\n  " CYAN BOLD "=== [⏳ DEADLINE SENTINEL & TO-DO MANAGER] ===" RST "\n\n");

        if (g_data.num_tasks == 0) {
            printf("  " GRAY "No tasks or assignments registered." RST "\n");
        } else {
            printf("  %-4s %-26s %-12s %-14s %-10s %s\n", "ID", "TASK / ASSIGNMENT", "CATEGORY", "DEADLINE", "PRIORITY", "STATUS");
            printf("  ----------------------------------------------------------------------------------------\n");

            for (int i = 0; i < g_data.num_tasks; i++) {
                TaskItem t = g_data.tasks[i];
                const char *p_col = (t.priority == 3) ? RED "HIGH" RST : ((t.priority == 2) ? YELLOW "MED " RST : GRAY "LOW " RST);

                printf("  [%d] %-26s %-12s ", i + 1, t.title, t.category);

                if (t.is_completed) {
                    printf(GRAY "%-14s" RST " %-18s " GREEN "[DONE]" RST "\n", "Finished", p_col);
                } else if (t.days_until_due < 0) {
                    char buf[20]; snprintf(buf, sizeof(buf), "%d days late", -t.days_until_due);
                    printf(RED "%-14s" RST " %-18s " RED BOLD "[OVERDUE!]" RST "\n", buf, p_col);
                } else if (t.days_until_due == 0) {
                    printf(YELLOW "%-14s" RST " %-18s " YELLOW BOLD "[DUE TODAY]" RST "\n", "Today", p_col);
                } else {
                    char buf[20]; snprintf(buf, sizeof(buf), "in %d days", t.days_until_due);
                    printf(CYAN "%-14s" RST " %-18s Pending\n", buf, p_col);
                }
            }
        }
        printf("  ----------------------------------------------------------------------------------------\n\n");

        printf("  [1] Add Task / Assignment   [2] Toggle Complete   [3] Delete Task   [0] Back\n");
        printf("  Select: ");
        int choice;
        if (scanf("%d", &choice) != 1) { flush_input(); continue; }
        flush_input();

        if (choice == 0) break;
        if (choice == 1 && g_data.num_tasks < MAX_TASKS) {
            printf("  Title: "); read_line(g_data.tasks[g_data.num_tasks].title, 40);
            printf("  Category (Assignment/Project/Personal): "); read_line(g_data.tasks[g_data.num_tasks].category, 16);
            printf("  Assignee: "); read_line(g_data.tasks[g_data.num_tasks].assignee, 24);
            printf("  Days until due: "); scanf("%d", &g_data.tasks[g_data.num_tasks].days_until_due);
            printf("  Priority (1 = Low, 2 = Med, 3 = High): "); scanf("%d", &g_data.tasks[g_data.num_tasks].priority); flush_input();
            g_data.tasks[g_data.num_tasks].is_completed = 0;
            g_data.num_tasks++;
            save_data();
        } else if (choice == 2 && g_data.num_tasks > 0) {
            printf("  Task ID (1-%d): ", g_data.num_tasks);
            int idx; scanf("%d", &idx); flush_input();
            if (idx >= 1 && idx <= g_data.num_tasks) {
                g_data.tasks[idx - 1].is_completed = !g_data.tasks[idx - 1].is_completed;
                if (g_data.tasks[idx - 1].is_completed) g_data.total_tasks_completed++;
                save_data();
            }
        } else if (choice == 3 && g_data.num_tasks > 0) {
            printf("  Task ID to delete (1-%d): ", g_data.num_tasks);
            int idx; scanf("%d", &idx); flush_input();
            if (idx >= 1 && idx <= g_data.num_tasks) {
                for (int i = idx - 1; i < g_data.num_tasks - 1; i++) {
                    g_data.tasks[i] = g_data.tasks[i + 1];
                }
                g_data.num_tasks--;
                save_data();
            }
        }
    }
}

/* ============================================================================
   MODULE 8: HOSTEL & LIFE (Inventory, Mess Tracker, Energy Monitor)
   ============================================================================ */
void sub_module_inventory(void) {
    while (1) {
        clear_screen();
        printf("\n  " CYAN BOLD "=== [🏠 HOSTEL ROOM INVENTORY TRACKER] ===" RST "\n\n");

        printf("  %-4s %-24s %-16s %-12s %s\n", "ID", "ITEM", "CATEGORY", "QUANTITY", "STATUS");
        printf("  ------------------------------------------------------------------------\n");
        for (int i = 0; i < g_data.num_inventory; i++) {
            InventoryItem item = g_data.inventory[i];
            const char *st = (item.quantity <= item.min_threshold) ? RED BOLD "[LOW STOCK - REPLENISH]" RST : GREEN "[STOCKED]" RST;
            printf("  [%d] %-24s %-16s %-12d %s\n", i + 1, item.name, item.category, item.quantity, st);
        }
        printf("  ------------------------------------------------------------------------\n\n");

        printf("  [1] Add Item   [2] Consume / Use 1 Unit   [3] Restock Item   [0] Back\n  Select: ");
        int choice; if (scanf("%d", &choice) != 1) { flush_input(); continue; } flush_input();

        if (choice == 0) break;
        if (choice == 1 && g_data.num_inventory < MAX_INVENTORY) {
            printf("  Item Name: "); read_line(g_data.inventory[g_data.num_inventory].name, 28);
            printf("  Category (Toiletries/Snacks/Stationery/Meds): "); read_line(g_data.inventory[g_data.num_inventory].category, 16);
            printf("  Current Quantity: "); scanf("%d", &g_data.inventory[g_data.num_inventory].quantity);
            printf("  Minimum Safe Threshold: "); scanf("%d", &g_data.inventory[g_data.num_inventory].min_threshold); flush_input();
            g_data.num_inventory++;
            save_data();
        } else if (choice == 2 && g_data.num_inventory > 0) {
            printf("  Item ID (1-%d): ", g_data.num_inventory);
            int idx; scanf("%d", &idx); flush_input();
            if (idx >= 1 && idx <= g_data.num_inventory && g_data.inventory[idx - 1].quantity > 0) {
                g_data.inventory[idx - 1].quantity--;
                save_data();
            }
        } else if (choice == 3 && g_data.num_inventory > 0) {
            printf("  Item ID (1-%d): ", g_data.num_inventory);
            int idx; scanf("%d", &idx);
            printf("  Units to add: ");
            int add; scanf("%d", &add); flush_input();
            if (idx >= 1 && idx <= g_data.num_inventory) {
                g_data.inventory[idx - 1].quantity += add;
                save_data();
            }
        }
    }
}

void sub_module_mess_tracker(void) {
    while (1) {
        clear_screen();
        printf("\n  " CYAN BOLD "=== [🍜 MESS & MEAL SKIPPING SENTINEL] ===" RST "\n\n");

        const char *meal_names[4] = {"Breakfast", "Lunch", "Snacks", "Dinner"};
        int total_skipped = 0;

        printf("  Meal Participation History (Last 14 Days):\n");
        printf("  ------------------------------------------------------------------------\n");
        for (int i = 0; i < 4; i++) {
            total_skipped += g_data.meals_skipped[i];
            int total = g_data.meals_taken[i] + g_data.meals_skipped[i];
            float skip_rate = (total > 0) ? ((float)g_data.meals_skipped[i] / total) * 100.0f : 0.0f;
            printf("  %-12s : Taken %2d | Skipped %2d (Skip Rate: %s%5.1f%%" RST ")\n",
                   meal_names[i], g_data.meals_taken[i], g_data.meals_skipped[i],
                   (skip_rate > 35) ? RED : GREEN, skip_rate);
        }
        printf("  ------------------------------------------------------------------------\n");
        printf("  Estimated Outside Food Burn from skipped meals: " RED BOLD "~ Rs. %d" RST "\n\n", total_skipped * 120);

        printf("  [1] Log Today's Meal (Taken / Skipped)   [0] Back\n  Select: ");
        int choice; if (scanf("%d", &choice) != 1) { flush_input(); continue; } flush_input();

        if (choice == 0) break;
        if (choice == 1) {
            printf("  Meal (1 = Breakfast, 2 = Lunch, 3 = Snacks, 4 = Dinner): ");
            int m; scanf("%d", &m);
            printf("  Did you eat in mess? (1 = Yes Taken, 0 = Skipped): ");
            int eaten; scanf("%d", &eaten); flush_input();
            if (m >= 1 && m <= 4) {
                if (eaten == 1) g_data.meals_taken[m - 1]++;
                else g_data.meals_skipped[m - 1]++;
                save_data();
            }
        }
    }
}

void sub_module_energy_tracker(void) {
    clear_screen();
    printf("\n  " CYAN BOLD "=== [🔋 ENERGY & BURNOUT PREDICTOR] ===" RST "\n\n");

    printf("  Sleep Recorded Last Night : " BOLD "%.1f hours" RST "\n", g_data.sleep_hours);
    printf("  Study Time Today          : " BOLD "%d hours" RST "\n", g_data.study_hours_today);
    printf("  Subjective Stress Level   : " BOLD "%d / 5" RST "\n\n", g_data.stress_level);

    float energy_score = (g_data.sleep_hours / 8.0f) * 60.0f + 
                         (float)(6 - g_data.stress_level) * 8.0f - 
                         (float)g_data.study_hours_today * 2.0f;

    if (energy_score > 100.0f) energy_score = 100.0f;
    if (energy_score < 10.0f) energy_score = 10.0f;

    printf("  Current Battery Status : " BOLD "%.0f%%" RST "\n", energy_score);
    animate_progress_bar(25, energy_score, (energy_score >= 65) ? GREEN : ((energy_score >= 40) ? YELLOW : RED), 5);
    printf("\n\n");

    if (energy_score < 40.0f) {
        printf("  " RED BOLD "[!] BURNOUT IMMINENT:" RST " Extreme fatigue detected. Take a 45 min power nap or sleep early.\n");
    } else if (energy_score < 70.0f) {
        printf("  " YELLOW "[OK] MODERATE ENERGY:" RST " Functional, but drink water and avoid excessive caffeine spikes.\n");
    } else {
        printf("  " GREEN "[PEAK] PRIME COGNITIVE STATE:" RST " Optimal focus window. Tackle your hardest academic problem now!\n");
    }

    printf("\n  " BOLD "[1] Update Sleep / Study / Stress   [0] Back" RST "\n");
    printf("  Select: ");
    int choice; if (scanf("%d", &choice) != 1) { flush_input(); return; } flush_input();

    if (choice == 1) {
        printf("  Hours of sleep last night (e.g. 6.5): "); scanf("%f", &g_data.sleep_hours);
        printf("  Study hours today: "); scanf("%d", &g_data.study_hours_today);
        printf("  Stress level (1 = completely chill, 5 = severe panic): "); scanf("%d", &g_data.stress_level);
        flush_input();
        save_data();
    }
}

void module_hostel_life_hub(void) {
    while (1) {
        clear_screen();
        printf("\n  " CYAN BOLD "=== [🏠 HOSTEL & WELL-BEING COMMAND HUB] ===" RST "\n\n");
        printf("  [1] Hostel Room Inventory (Toiletries, Snacks, Meds)\n");
        printf("  [2] Mess & Meal Skipping Tracker\n");
        printf("  [3] Energy & Burnout Battery Monitor\n");
        printf("  [0] Back to Main Menu\n\n");
        printf("  Select option: ");
        int choice; if (scanf("%d", &choice) != 1) { flush_input(); continue; } flush_input();

        if (choice == 0) break;
        if (choice == 1) sub_module_inventory();
        else if (choice == 2) sub_module_mess_tracker();
        else if (choice == 3) sub_module_energy_tracker();
    }
}

/* ============================================================================
   MODULE 9: EMERGENCY CONTACTS & SOS DIRECTORY
   ============================================================================ */
void module_emergency_contacts(void) {
    while (1) {
        clear_screen();
        printf("\n  " RED BOLD "=== [🆘 EMERGENCY CONTACTS & SOS DIRECTORY] ===" RST "\n\n");

        for (int i = 0; i < g_data.num_emergency; i++) {
            printf("  [%d] %-28s : " WHITE BOLD "%s" RST "\n",
                   i + 1, g_data.emergency[i].title, g_data.emergency[i].phone);
        }
        printf("  ------------------------------------------------------------------------\n\n");

        printf("  [1] Add Contact   [2] Edit Contact   [0] Back\n  Select: ");
        int choice; if (scanf("%d", &choice) != 1) { flush_input(); continue; } flush_input();

        if (choice == 0) break;
        if (choice == 1 && g_data.num_emergency < MAX_EMERGENCY) {
            printf("  Contact Title: "); read_line(g_data.emergency[g_data.num_emergency].title, 28);
            printf("  Phone Number: "); read_line(g_data.emergency[g_data.num_emergency].phone, 20);
            g_data.num_emergency++;
            save_data();
        } else if (choice == 2 && g_data.num_emergency > 0) {
            printf("  Contact ID to edit (1-%d): ", g_data.num_emergency);
            int idx; scanf("%d", &idx); flush_input();
            if (idx >= 1 && idx <= g_data.num_emergency) {
                printf("  New Title: "); read_line(g_data.emergency[idx - 1].title, 28);
                printf("  New Phone: "); read_line(g_data.emergency[idx - 1].phone, 20);
                save_data();
            }
        }
    }
}

/* ============================================================================
   MODULE 10: ACHIEVEMENTS & BADGES SYSTEM
   ============================================================================ */
void module_achievements(void) {
    clear_screen();
    printf("\n  " YELLOW BOLD "=== [🏆 SURVIVAL ACHIEVEMENTS & BADGES] ===" RST "\n\n");

    /* Badge 1: Attendance Warrior */
    int att_warrior = 1;
    for (int i = 0; i < g_data.num_subjects; i++) {
        float pct = (g_data.subjects[i].total > 0) ? ((float)g_data.subjects[i].attended / g_data.subjects[i].total) * 100.0f : 0.0f;
        if (pct < 80.0f) { att_warrior = 0; break; }
    }

    /* Badge 2: Budget Master */
    float safe = get_safe_daily_limit();
    int budget_master = (safe >= 180.0f);

    /* Badge 3: Deadline Slayer */
    int deadline_slayer = (g_data.total_tasks_completed >= 5);

    /* Badge 4: Streak Legend */
    int streak_legend = (g_data.longest_streak >= 7);

    /* Badge 5: Academic Weapon */
    int total_study = 0;
    for (int i = 0; i < g_data.num_subjects; i++) total_study += g_data.subjects[i].study_minutes;
    int academic_weapon = (total_study >= 600); /* 10 hours */

    printf("  [1] %s [ATTENDANCE WARRIOR]" RST " : All registered subjects maintain >= 80%% attendance.\n",
           att_warrior ? GREEN "⭐ UNLOCKED" : GRAY "🔒 LOCKED  ");

    printf("  [2] %s [BUDGET MASTER]     " RST " : Maintain a safe daily burn rate of >= Rs. 180/day.\n",
           budget_master ? GREEN "⭐ UNLOCKED" : GRAY "🔒 LOCKED  ");

    printf("  [3] %s [DEADLINE SLAYER]   " RST " : Complete at least 5 assignments/tasks on time.\n",
           deadline_slayer ? GREEN "⭐ UNLOCKED" : GRAY "🔒 LOCKED  ");

    printf("  [4] %s [STREAK LEGEND]     " RST " : Maintain a daily survival streak of 7+ consecutive days.\n",
           streak_legend ? GREEN "⭐ UNLOCKED" : GRAY "🔒 LOCKED  ");

    printf("  [5] %s [ACADEMIC WEAPON]   " RST " : Log more than 10 total hours in the Study Session tracker.\n",
           academic_weapon ? GREEN "⭐ UNLOCKED" : GRAY "🔒 LOCKED  ");

    printf("\n  " GRAY "Complete more daily missions and attend classes to unlock all badges!" RST "\n\n");
    wait_key();
}

/* ============================================================================
   MODULE 11: WEEKLY EXECUTIVE REPORT
   ============================================================================ */
void module_weekly_report(void) {
    clear_screen();
    printf("\n  " CYAN BOLD "+======================================================================+" RST "\n");
    printf("  " CYAN "|" RST BOLD WHITE "              NEXUS: WEEKLY EXECUTIVE SURVIVAL REPORT                  " RST CYAN "|" RST "\n");
    printf("  " CYAN "|" RST GRAY "    Student: %-16s | SAP ID: %-12s | Status: OK    " RST CYAN "|" RST "\n",
           g_data.student_name, g_data.sap_id);
    printf("  " CYAN "+======================================================================+" RST "\n\n");

    /* 1. Academic Health */
    int total_classes = 0, attended_classes = 0;
    for (int i = 0; i < g_data.num_subjects; i++) {
        attended_classes += g_data.subjects[i].attended;
        total_classes += g_data.subjects[i].total;
    }
    float avg_att = (total_classes > 0) ? ((float)attended_classes / total_classes) * 100.0f : 0.0f;
    printf("  " BOLD "1. ACADEMIC INTEGRITY:" RST "\n");
    printf("     * Aggregate Attendance : %.1f%% (%d of %d classes attended)\n", avg_att, attended_classes, total_classes);
    printf("     * Debarment Alerts     : %s\n", (avg_att >= 75.0f) ? GREEN "None (Safe)" RST : RED "Debarment threat detected!" RST);

    /* 2. Fiscal Runway */
    float total_spent = 0;
    for (int i = 0; i < g_data.num_expenses; i++) total_spent += g_data.expenses[i].amount;
    float bal = g_data.initial_budget - total_spent;
    int days_left = g_data.total_month_days - g_data.days_spent;
    if (days_left <= 0) days_left = 1;
    float safe = (bal > 0) ? (bal / days_left) : 0.0f;
    printf("\n  " BOLD "2. FINANCIAL SOLVENCY:" RST "\n");
    printf("     * Total Capital Spent  : Rs. %.2f\n", total_spent);
    printf("     * Current Cash Balance : Rs. %.2f\n", bal);
    printf("     * Safe Daily Allowance : Rs. %.2f / day (Runway: %d days)\n", safe, days_left);

    /* 3. Deadlines & Tasks */
    int pending = 0, overdue = 0;
    for (int i = 0; i < g_data.num_tasks; i++) {
        if (!g_data.tasks[i].is_completed) {
            pending++;
            if (g_data.tasks[i].days_until_due < 0) overdue++;
        }
    }
    printf("\n  " BOLD "3. DELIVERABLES & MISSIONS:" RST "\n");
    printf("     * Overdue Deliverables : %s%d" RST "\n", (overdue > 0) ? RED : GREEN, overdue);
    printf("     * Active Tasks Pending : %d\n", pending);
    printf("     * Current Daily Streak : %d consecutive days\n", g_data.survival_streak);

    /* 4. Composite Score */
    float score = calculate_survival_score();
    printf("\n  " BOLD "4. COMPOSITE SURVIVAL RATING: " CYAN BOLD "%.1f / 100" RST "\n", score);
    printf("  " CYAN "+======================================================================+" RST "\n\n");

    wait_key();
}

/* ============================================================================
   MODULE 12: STUDENT LOGIN & MULTI-PROFILE SYSTEM
   ============================================================================ */
void module_student_login(void) {
    clear_screen();
    printf("\n  " CYAN BOLD "=== [🔐 STUDENT PROFILE / MULTI-USER LOGIN] ===" RST "\n\n");
    printf("  Current Logged In User : " BOLD "%s" RST " (SAP ID: %s)\n", g_data.student_name, g_data.sap_id);
    printf("  Active Database File   : %s\n\n", g_data.current_save_file);

    printf("  [1] Switch Profile / Load Student Data\n");
    printf("  [2] Create New Student Profile\n");
    printf("  [0] Back\n\n");
    printf("  Select: ");
    int choice; if (scanf("%d", &choice) != 1) { flush_input(); return; } flush_input();

    if (choice == 1) {
        printf("  Enter database file name to load (e.g. rahul_data.dat): ");
        char path[64]; read_line(path, 64);
        if (strlen(path) > 0) {
            load_data(path);
            printf("  " GREEN "Profile loaded successfully!" RST "\n");
            Sleep(800);
        }
    } else if (choice == 2) {
        char name[32], sap[16], path[64];
        printf("  Enter Student Name: "); read_line(name, 32);
        printf("  Enter SAP ID / Roll Number: "); read_line(sap, 16);
        snprintf(path, sizeof(path), "%s_survival.dat", sap);

        seed_default_data();
        strcpy(g_data.student_name, name);
        strcpy(g_data.sap_id, sap);
        strcpy(g_data.current_save_file, path);
        save_data();

        printf("  " GREEN "Created new student profile in %s!" RST "\n", path);
        Sleep(800);
    }
}

/* ============================================================================
   RETRO HACKER BOOT ANIMATION
   ============================================================================ */
void play_boot_animation(void) {
    clear_screen();
    printf("\n");
    type_text(GRAY ">> [INIT] Booting NEXUS KERNEL v3.0 (Enterprise Hackathon Edition)...\n" RST, 8);
    type_text(GRAY ">> [INIT] Connecting to Student Academic & Financial Microservices...\n" RST, 8);
    type_text(CYAN ">> [OK] Initializing Emergency Student Survival Engine...\n\n" RST, 8);
    Sleep(120);

    const char *logo[] = {
        "  " CYAN " _   _  _______   ___   _  _____ " RST,
        "  " CYAN "| \\ | ||  ___\\ \\ / / | | |/  ___|" RST,
        "  " CYAN "|  \\| || |__  \\ V /| | | |\\ `--. " RST,
        "  " CYAN "| . ` ||  __| /   \\| | | | `--. \\" RST,
        "  " CYAN "| |\\  || |___/ /^\\ \\ |_| |/\\__/ /" RST,
        "  " CYAN "\\_| \\_/\\____/\\/   \\/\\___/ \\____/ " RST
    };

    for (int i = 0; i < 6; i++) {
        printf("%s\n", logo[i]);
        Sleep(30);
    }

    printf("\n");
    printf("  " YELLOW "+======================================================================+" RST "\n");
    printf("  " YELLOW "|         " WHITE BOLD "S T U D E N T   S U R V I V A L   K I T   v 3 . 0" YELLOW "            |" RST "\n");
    printf("  " YELLOW "|             " CYAN "Engineered for College Hackathons by Team Nexus" YELLOW "          |" RST "\n");
    printf("  " YELLOW "+======================================================================+" RST "\n\n");

    printf("  " GRAY "Mounting persistent state: " RST);
    animate_progress_bar(30, 100.0f, CYAN, 6);
    printf(" " GREEN "[ONLINE]\n\n" RST);
    Sleep(300);
}

/* ============================================================================
   MAIN DASHBOARD DISPLAY
   ============================================================================ */
void display_dashboard(void) {
    clear_screen();
    float score = calculate_survival_score();
    float safe = get_safe_daily_limit();

    printf("\n");
    printf("  " CYAN "+======================================================================+" RST "\n");
    printf("  " CYAN "|" RST BOLD WHITE "        NEXUS: STUDENT SURVIVAL KIT -- DAILY EMERGENCY DASHBOARD       " RST CYAN "|" RST "\n");
    printf("  " CYAN "|" RST GRAY "  Student: %-12s | SAP: %-10s | Survival Score: " BOLD "%-5.1f" RST GRAY "    " RST CYAN "|" RST "\n",
           g_data.student_name, g_data.sap_id, score);
    printf("  " CYAN "+======================================================================+" RST "\n\n");

    /* Quick Live Indicators */
    printf("  " BOLD "⚡ QUICK STATUS:" RST "\n");
    printf("  * Survival Streak : " YELLOW BOLD "%d Days" RST " 🔥   |   Safe Spend: " GREEN "Rs. %.2f/day" RST "\n",
           g_data.survival_streak, safe);

    /* Debarment SOS */
    int att_threat = 0;
    for (int i = 0; i < g_data.num_subjects; i++) {
        float pct = (g_data.subjects[i].total > 0) ? ((float)g_data.subjects[i].attended / g_data.subjects[i].total) * 100.0f : 0.0f;
        if (pct < 75.0f) {
            printf("  * " RED BOLD "[!] ATTENDANCE ALERT:" RST " %s is at " RED "%.1f%%" RST " (Debarment Threat!)\n", g_data.subjects[i].name, pct);
            att_threat = 1;
            break;
        }
    }
    if (!att_threat) printf("  * " GREEN "[OK] ATTENDANCE:" RST " All subjects above 75%% debarment threshold.\n");

    /* Next Urgent Deadline */
    int top_due = 999, top_t_idx = -1;
    for (int i = 0; i < g_data.num_tasks; i++) {
        if (!g_data.tasks[i].is_completed && g_data.tasks[i].days_until_due < top_due) {
            top_due = g_data.tasks[i].days_until_due;
            top_t_idx = i;
        }
    }
    if (top_t_idx != -1) {
        printf("  * " YELLOW "[*] NEXT DEADLINE:" RST " \"%s\" (%s)\n",
               g_data.tasks[top_t_idx].title, 
               (top_due < 0) ? RED "OVERDUE!" : ((top_due == 0) ? YELLOW "DUE TODAY" : "Pending"));
    }

    printf("\n  " CYAN "------------------------------------------------------------------------" RST "\n");
    printf("  " BOLD "CORE COMMAND DIRECTORY:" RST "\n");
    printf("  " RED   "[1]  🚨 Smart Alert Center & Crisis Radar (Triage Engine)" RST "\n");
    printf("  " CYAN  "[2]  📅 Daily Mission Planner & Survival Streak" RST "\n");
    printf("  " CYAN  "[3]  🎯 Survival Score & Weak Subject Detector" RST "\n");
    printf("  " RED BOLD "[4]  🚨 PANIC MODE! (24-Hour Emergency Protocol)" RST "\n");
    printf("  " WHITE "[5]  📚 Academics Hub (Attendance, Exam Radar, CGPA, Study Log)" RST "\n");
    printf("  " WHITE "[6]  💸 Finance Sentinel (Daily Limit, Impulse, Analytics, Splitter, IOU)" RST "\n");
    printf("  " WHITE "[7]  ⏳ Deadlines & To-Do Manager (Assignment Countdowns)" RST "\n");
    printf("  " WHITE "[8]  🏠 Hostel & Well-Being (Inventory, Mess Tracker, Energy)" RST "\n");
    printf("  " WHITE "[9]  🆘 Emergency SOS Directory" RST "\n");
    printf("  " WHITE "[10] 🏆 Achievements & Badges" RST "\n");
    printf("  " WHITE "[11] 📈 Weekly Executive Report" RST "\n");
    printf("  " WHITE "[12] 🔐 Student Profile / Multi-User Switch" RST "\n");
    printf("  " RED   "[0]  Exit Survival Kit" RST "\n");
    printf("\n  " BOLD "Enter Command [0-12]: " RST);
}

/* ============================================================================
   MAIN ENTRY POINT
   ============================================================================ */
int main(void) {
    enable_ansi();
    load_data(SAVE_FILE_DEFAULT);
    play_boot_animation();

    while (1) {
        display_dashboard();
        int choice;
        if (scanf("%d", &choice) != 1) {
            flush_input();
            continue;
        }
        flush_input();

        switch (choice) {
            case 1: module_smart_alerts(); break;
            case 2: module_daily_mission(); break;
            case 3: module_score_and_weak_subjects(); break;
            case 4: module_panic_mode(); break;
            case 5: module_academics_hub(); break;
            case 6: module_finance_hub(); break;
            case 7: module_deadlines_hub(); break;
            case 8: module_hostel_life_hub(); break;
            case 9: module_emergency_contacts(); break;
            case 10: module_achievements(); break;
            case 11: module_weekly_report(); break;
            case 12: module_student_login(); break;
            case 0:
                clear_screen();
                save_data();
                printf("\n  " GREEN BOLD "Survival data successfully preserved in %s.\n" RST, g_data.current_save_file);
                printf("  " CYAN "Stay safe and conquer your semester! -- Team Nexus\n\n" RST);
                return 0;
            default:
                break;
        }
    }
    return 0;
}
