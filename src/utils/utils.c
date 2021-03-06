#include "utils.h"

#include <string.h>
#include <stdio.h>
#include <time.h>

struct ErrorReport errorReport = { {0}, {0}, {0}, {0}, 0, {0}, {0}, {{0}}, 0 };

void addErrorReportCommand(const char* command)
{
    int i;
    if (errorReport.historyCommandCounter >= 100) {
        for (i = 0; i < 99; ++i) {
            strncpy(errorReport.history[i], errorReport.history[i + 1], 100);
        }
        errorReport.historyCommandCounter--; /* we delete the oldest command */
    }
    strncpy(errorReport.history[errorReport.historyCommandCounter++], command, 100);
}

void dumpErrorReport(FILE *f)
{
    int i;
    time_t t = -1;
    char *timeStr = NULL;
    t = time(NULL);
    timeStr = ctime(&t);
    fprintf(f, "Error Report. Generated by %s version %s at %s\n", errorReport.applicationName, errorReport.applicationVersion, timeStr);
    fprintf(f, "Error code:        %d\n" \
               "Error name:        %s\n" \
               "Error description: %s\n\n" \
               "Please open an issue and attach this report in %s\n" \
               "Or send this report to %s with topic \"REPORT\"\n\n" \
               "History:\n", errorReport.errorCode, errorReport.errorName, errorReport.errorDescription, errorReport.issueUrl, errorReport.mailUrl);

    for (i = 0; i < errorReport.historyCommandCounter; ++i) {
        fprintf(f, "%d: %s\n", i + 1, errorReport.history[i]);
    }
}

/* Levenshtein distance: The minimum number of single-character edits required to change one word into the other. Strings do not have to be the same length. This is a modified version of the one published in https://en.wikibooks.org/wiki/Algorithm_Implementation/Strings/Levenshtein_distance#C at the date Friday, December 20th, 2019 */
#define MIN3(a, b, c) ((a) < (b) ? ((a) < (c) ? (a) : (c)) : ((b) < (c) ? (b) : (c)))

int levenshtein(const char *s1, const char *s2)
{
    unsigned int s1len, s2len, x, y, lastdiag, olddiag;
    unsigned int column[101]; /* 100 characters maximum (this should be enough for this application) */
    s1len = strlen(s1);
    s2len = strlen(s2);
    for (y = 1; y <= s1len; y++)
        column[y] = y;
    for (x = 1; x <= s2len; x++) {
        column[0] = x;
        for (y = 1, lastdiag = x-1; y <= s1len; y++) {
            olddiag = column[y];
            column[y] = MIN3(column[y] + 1, column[y-1] + 1, lastdiag + (s1[y-1] == s2[x-1] ? 0 : 1));
            lastdiag = olddiag;
        }
    }
    return(column[s1len]);
}

/* Makes usage of the Levenshtein algorithm to find the index of the most similar string in an array. Return -1 if no result can be retrieved (because, for example, the array's lenght is zero or less) */
int findMostSimilarStringInArray(const char *str, const char **strArray, int arrayLenght)
{

    /* assume that the most similar string is the first one */
    int index = 0;
    int currentDistance;
    int minDistance;
    int i;

    if (arrayLenght <= 0) {
        return -1;
    }
    minDistance = currentDistance = levenshtein(str, strArray[0]);
    for (i = 1; i < arrayLenght; ++i) {
        currentDistance = levenshtein(str, strArray[i]);
        if (currentDistance < minDistance) {
            index = i;
            minDistance = currentDistance;
        }
    }

    return index;
}