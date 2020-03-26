#ifndef VIEW_H
#define VIEW_H

#include <stdio.h>
#include "../../lib/pokem.h"

/* Colors */
#define COLOR_BORDER COLOR(96, 128, 248)
#define COLOR_BACKGROUND COLOR_BG(32, 72, 104)
#define COLOR_GREEN COLOR(88, 248, 88)
#define COLOR_YELLOW COLOR(248, 248, 0)
#define COLOR_CYAN COLOR(0, 248, 248)
#define COLOR_RED COLOR(248, 128, 88)

enum DatabaseType { PokemonDB, ItemsDB, DungeonDB, FriendAreaDB, MissionDB, RewardTypeDB, SosMailTypeDB };

int showSelectionScreen(void);

void showHelp(const char *programName);
void showDatabase(enum DatabaseType type);

int requestWonderMailPassword(char *password);
int requestAndParseWonderMailData(struct WonderMail *wm);
int requestSOSMailPassword(char *password);
int requestAndParseSosMailData(struct SosMail *sos);
int requestAndParseSOSMailConvertion(char *password, int *item);

void printWonderMailData(const struct WonderMailInfo *mailInfo, const struct WonderMail *mail);
void printSOSData(const struct SosMailInfo *mailInfo, const struct SosMail *mail);

void printWonderMailDataToFile(const struct WonderMailInfo *mailInfo, FILE *f);
void printSOSDataToFile(const struct SosMailInfo *mailInfo, enum MailType mailType, FILE *f);

int requestAndValidateIntegerInput(unsigned int *n, int allowEmptyValue, int valueIfEmpty, const char* message);
int requestAndValidateStringInput(char* str, unsigned int maxLength, int allowEmptyValue, const char* valueIfEmpty, const char* message);

#endif /* VIEW_H */
