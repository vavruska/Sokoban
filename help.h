/* help pages */
typedef struct helpline {
    int xpos, ypos, page;
    char *textline;
} h_line;

h_line help_pages[] = {
    { 0, 1, 0, "Objective: Push all the objects from" },
    { 11, 2, 0, "their starting positions" },
    { 11, 3, 0, "into their goal positions." },
    { 11, 4, 0, "Be warned that you can push" },
    { 11, 5, 0, "only one object at a time;" },
    { 11, 6, 0, "watch out for corners!" },
    { 0, 8, 0, "Movement:" },
    { 5, 9, 0, "Use the arrow keys or hljk:" },
    { 5, 10, 0, "Move/Push     h    l    k   j" },
    { 5, 11, 0, "Run/Push      H    L    K   J" },
    { 5, 12, 0, "Run Only     ^H   ^L   ^K  ^J" },
    { 0, 14, 0, "Other Commands:" },
    { 5, 15, 0, "?: this help message" },
    { 5, 16, 0, "u: undo last action" },
    { 5, 17, 0, "U: restart this level" },
    { 5, 18, 0, "s: save game and quit" },
    { 5, 19, 0, "q: quit without save" },
    { 5, 20, 0, "c: snapshot position" },
    { 5, 21, 0, "O: restore snapshot" },
    { 5, 22, 0, "S: view score file" },
    { 7, 24, 0, "Press <Return> to exit." },
    { 0, 0, 0, NULL }
};

#define HELP_PAGES 1
