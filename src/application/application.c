#include "application.h"
#include "../view/view.h"
#include "../utils/utils.h"
#include "../utils/colors.h"

#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>

int decodeWM(int argc, const char *argv[]) /* The passwords are received here: in argv */
{
    char psw[25] = {0};

    if (argc <= 1 || argv == NULL) {
        requestWonderMailPassword(psw);
    }

    struct WonderMail mail = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    struct WonderMailInfo mailInfo  = { {0}, {0}, {0}, {0}, {0}, {0}, {0}, 0, {0}, {0} }; /* The 8th element is a char */
    int i;
    int errorCode;

    FILE *f = NULL;
    time_t t = -1;
    char *timeStr = NULL;

    for (i = 1; i < argc || i == 1; ++i) {
        if (argc > 1) {
            fprintf(stdout, LIGHT "%d.\n" RESET, i);
            strncpy(psw, argv[i], 24);
        }
        errorCode = decodeWonderMail(psw, &mail);
        if (errorCode) {
            continue;
        }

        /* Bulking the mail's data... */
        setWonderMailInfo(&mail, &mailInfo);
        strncpy(mailInfo.password, psw, 24);
        printWonderMailData(&mailInfo, &mail);
        fputc('\n', stdout);
        f = fopen(LOG_WM_FILENAME, "a");
        if (f) {
            // header
            t = time(NULL);
            timeStr = ctime(&t);
            if (timeStr) {
                timeStr[strlen(timeStr) - 1] = '\0'; /* remove the trailing newline */
            }
            fprintf(f, "%s ~~~~~~~~~~~~~~~~~~~~~~~\n", timeStr);
            printWonderMailDataToFile(&mailInfo, f);
            fputs("\n\n\n", f);
            fflush(f);
            fclose(f);
        }
    }
    fflush(stdout);

    return NoError;
}


int encodeWM(int argc, const char *argv[])
{
    struct WonderMail wm;
    wm.mailType = WonderMailType;

    if (argc != 10 || argv == NULL) {
        requestAndParseWonderMailData(&wm);
    } else if (parseWMData(argv, &wm) != NoError) {
        fputs("Aborting...\n", stderr);
        return InputError;
    }

    char finalPassword[25] = {0};
    int errorCode = encodeWonderMail(&wm, finalPassword, 1); /* "1": Try special missions */
    if (errorCode != NoError) {
        return errorCode;
    }

    /* Get the full Wonder Mail info */
    struct WonderMailInfo wmInfo = { {0}, {0}, {0}, {0}, {0}, {0}, {0}, 0, {0}, {0} };
    setWonderMailInfo(&wm, &wmInfo);
    strncpy(wmInfo.password, finalPassword, 24);
    printWonderMailData(&wmInfo, &wm);
    FILE *f = fopen(LOG_WM_FILENAME, "a");
    time_t t = -1;
    char *timeStr = NULL;
    if (f) {
        // header
        t = time(NULL);
        timeStr = ctime(&t);
        if (timeStr) {
            timeStr[strlen(timeStr) - 1] = '\0'; /* remove the trailing newline */
        }
        fprintf(f, "%s ~~~~~~~~~~~~~~~~~~~~~~~\n", timeStr);
        printWonderMailDataToFile(&wmInfo, f);
    }
    if (wm.dungeon == 10 || wm.dungeon == 12 || wm.dungeon == 14 || wm.dungeon == 16 || wm.dungeon == 18 || wm.dungeon == 22 || wm.dungeon == 47 || wm.dungeon == 48 || wm.dungeon == 52) {
        printMessage(stderr, WarningMessage, "Due to the choosen dungeon, you will not be able to accept the above mission.\n\n");
        fputs("WARNING: Due to the choosen dungeon, you will not be able to accept the above mission.\n", f);
        fflush(stderr);
    }
    fputs("\n\n\n", f);
    fflush(f);
    fclose(f);
    fflush(stdout);

    return NoError;
}



int parseWMData(const char *argv[], struct WonderMail *wm)
{
    unsigned int i;
    int mostSimilarIndex = 0;
    char* stringEnd;

    wm->mailType         = WonderMailType; /* Wonder Mail */
    wm->missionType      = (unsigned int)atoi(argv[1]);

    /* user can input the pkmns, items, dungeons and friend zones by using its name or its index */
    wm->pkmnClient = (unsigned int)strtol(argv[2], &stringEnd, 10);
    if (*stringEnd) {
        wm->pkmnClient   = pkmnSpeciesCount; /* invalid name, invalid index */
        for (i = 0; i < pkmnSpeciesCount; ++i) {
            if (strcmp(pkmnSpeciesStr[i], argv[2]) == 0) {
                wm->pkmnClient = i;
                break;
            }
        }
        if (wm->pkmnClient == pkmnSpeciesCount) {
            mostSimilarIndex = findMostSimilarStringInArray(argv[2], pkmnSpeciesStr, pkmnSpeciesCount);
            if (mostSimilarIndex == -1) {
                printMessage(stderr, ErrorMessage, "Cannot find pokemon " LRED "\"%s\"" RESET " in the database.\n", argv[2]);
                return InputError;
            } else {
                wm->pkmnClient = mostSimilarIndex;
                printMessage(stderr, InfoMessage, "Pokemon " LGREEN "%s" RESET " has been assumed.\n", pkmnSpeciesStr[mostSimilarIndex]);
            }
        }
    }

    if (wm->missionType == Find || wm->missionType == Escort) {
        wm->pkmnTarget = (unsigned int)strtol(argv[3], &stringEnd, 10);
        if (*stringEnd) {
            wm->pkmnTarget   = (unsigned int)atoi(argv[3]);
        } else {
            wm->pkmnTarget   = pkmnSpeciesCount; /* invalid name, invalid index */
            for (i = 0; i < pkmnSpeciesCount; ++i) {
                if (strcmp(pkmnSpeciesStr[i], argv[3]) == 0) {
                    wm->pkmnTarget = i;
                    break;
                }
            }
        }
        if (wm->pkmnTarget == pkmnSpeciesCount) {
            mostSimilarIndex = findMostSimilarStringInArray(argv[3], pkmnSpeciesStr, pkmnSpeciesCount);
            if (mostSimilarIndex == -1) {
                printMessage(stderr, ErrorMessage, "Cannot find pokemon " LRED "\"%s\"" RESET " in the database.\n", argv[3]);
                return InputError;
            } else {
                wm->pkmnClient = mostSimilarIndex;
                printMessage(stderr, InfoMessage, "Pokemon " LGREEN "%s" RESET " has been assumed.\n", pkmnSpeciesStr[mostSimilarIndex]);
            }
        }
    } else {
        wm->pkmnTarget   = wm->pkmnClient;
    }

    if (wm->missionType == FindItem || wm->missionType == DeliverItem) {
        wm->itemDeliverFind = (unsigned int)strtol(argv[4], &stringEnd, 10);
        if (*stringEnd) {
            wm->itemDeliverFind   = itemsCount; /* invalid name, invalid index */
            for (i = 0; i < itemsCount; ++i) {
                if (strcmp(itemsStr[i], argv[4]) == 0) {
                    wm->itemDeliverFind = i;
                    break;
                }
            }
            if (wm->itemDeliverFind == itemsCount) {
                mostSimilarIndex = findMostSimilarStringInArray(argv[4], itemsStr, itemsCount);
                if (mostSimilarIndex == -1) {
                    printMessage(stderr, ErrorMessage, "Cannot find item " LRED "\"%s\"" RESET LIGHT " in the database.\n", argv[4]);
                    return InputError;
                } else {
                    wm->itemDeliverFind = mostSimilarIndex;
                    printMessage(stderr, InfoMessage, "Item " LGREEN "%s" RESET " has been assumed.\n", dungeonsStr[mostSimilarIndex]);
                }
            }
        }
    } else {
        wm->itemDeliverFind = 9;
    }

    wm->dungeon = (unsigned int)strtol(argv[5], &stringEnd, 10);
    if (*stringEnd) {
        wm->dungeon   = dungeonsCount; /* invalid name, invalid index */
        for (i = 0; i < dungeonsCount; ++i) {
            if (strcmp(dungeonsStr[i], argv[5]) == 0) {
                wm->dungeon = i;
                break;
            }
        }
        if (wm->dungeon == dungeonsCount) {
            mostSimilarIndex = findMostSimilarStringInArray(argv[5], dungeonsStr, dungeonsCount);
            if (mostSimilarIndex == -1) {
                printMessage(stderr, ErrorMessage, "Cannot find dungeon " LRED "\"%s\"" RESET LIGHT " in the database.\n", argv[5]);
                return InputError;
            } else {
                wm->dungeon = mostSimilarIndex;
                printMessage(stderr, InfoMessage, "Dungeon " LGREEN "%s" RESET " has been assumed.\n", dungeonsStr[mostSimilarIndex]);
            }
        }
    }

    wm->floor            = (unsigned int)atoi(argv[6]);
    wm->rewardType       = (unsigned int)atoi(argv[7]);

    wm->itemReward = (unsigned int)strtol(argv[8], &stringEnd, 10);
    if (*stringEnd) {
        wm->itemReward   = itemsCount; /* invalid name, invalid index */
        for (i = 0; i < itemsCount; ++i) {
            if (strcmp(itemsStr[i], argv[8]) == 0) {
                wm->itemReward = i;
                break;
            }
        }
        if (wm->itemReward == itemsCount) {
            mostSimilarIndex = findMostSimilarStringInArray(argv[8], itemsStr, itemsCount);
            if (mostSimilarIndex == -1) {
                printMessage(stderr, ErrorMessage, "Cannot find item " LRED "\"%s\"" RESET " in the database.\n", argv[8]);
                return InputError;
            } else {
                wm->itemReward = mostSimilarIndex;
                printMessage(stderr, InfoMessage, "Item " LGREEN "%s" RESET " has been assumed.\n", itemsStr[mostSimilarIndex]);
            }
        }
    }

    wm->itemReward = (unsigned int)strtol(argv[9], &stringEnd, 10);
    if (wm->rewardType == FriendArea) {
        if (*stringEnd) {
            wm->friendAreaReward   = friendAreasCount; /* invalid name, invalid index */
            for (i = 0; i < friendAreasCount; ++i) {
                if (strcmp(friendAreasStr[i], argv[9]) == 0) {
                    wm->friendAreaReward = i;
                    break;
                }
            }
            if (wm->friendAreaReward == friendAreasCount) {
                mostSimilarIndex = findMostSimilarStringInArray(argv[9], friendAreasStr, friendAreasCount);
                if (mostSimilarIndex == -1) {
                    printMessage(stderr, ErrorMessage, "Cannot find friend area " LRED "\"%s\"" RESET " in the database.\n", argv[9]);
                    return InputError;
                } else {
                    wm->friendAreaReward = mostSimilarIndex;
                    printMessage(stderr, InfoMessage, "Friend area " LGREEN "%s" RESET " has been assumed.\n", friendAreasStr[mostSimilarIndex]);
                }
            }
        }
    } else {
        wm->friendAreaReward = 0;
    }

    return NoError;
}



int decodeSOSM(int argc, const char *argv[])
{
    char psw[55] = {0};

    if (argc <= 1) {
        requestSOSMailPassword(psw);
    }

    struct SosMail mail = { 0, 0, 0, 0, 0, 0, 0, {0}, 0, 0, 0, 0, 0, 0, 0 };
    struct SosMailInfo mailInfo  = { {0}, {0}, {0}, {0}, {0}, {0}, {0}, 0, {0}, {0}, {0}, {0} }; /* The 8th element is a char */
    int i;
    int errorCode;

    FILE *f = NULL;
    time_t t = -1;
    char *timeStr = NULL;

    for (i = 1; i < argc || i == 1; ++i) {
        if (argc > 1) {
            fprintf(stdout, LIGHT "%d.\n" RESET, i);
            strncpy(psw, argv[i], 54);
        }
        errorCode = decodeSosMail(psw, &mail);
        if (errorCode != NoError) {
            continue;
        }

        /* Bulking the mail's data... */
        setSosInfo(&mail, &mailInfo);
        strncpy(mailInfo.password, psw, 54);
        printSOSData(&mailInfo, &mail);
        fputc('\n', stdout);
        f = fopen(LOG_SOS_FILENAME, "a");
        if (f) {
            // header
            t = time(NULL);
            timeStr = ctime(&t);
            if (timeStr) {
                timeStr[strlen(timeStr) - 1] = '\0'; /* remove the trailing newline */
            }
            fprintf(f, "%s ~~~~~~~~~~~~~~~~~~~~~~~~~\n", timeStr);
            printSOSDataToFile(&mailInfo, mail.mailType, f);
            fputs("\n\n\n", f);
            fflush(f);
            fclose(f);
        }
    }
    fflush(stdout);

    return NoError;
}


int encodeSOSM(int argc, const char *argv[])
{
    struct SosMail sos;

    if (argc != 8 || argv == NULL) {
        requestAndParseSosMailData(&sos);
    } else if (parseSOSData(argv, &sos) != NoError) {
        fputs("Aborting...\n", stderr);
        return InputError;
    }

    char finalPassword[55] = {0};
    int errorCode = encodeSosMail(&sos, finalPassword);
    if (errorCode) {
        return errorCode;
    }

    /* Get the full SOS Mail info */
    struct SosMailInfo sosInfo  = { {0}, {0}, {0}, {0}, {0}, {0}, {0}, 0, {0}, {0}, {0}, {0} };
    setSosInfo(&sos, &sosInfo);
    strncpy(sosInfo.password, finalPassword, 54);
    printSOSData(&sosInfo, &sos);
    FILE *f = fopen(LOG_SOS_FILENAME, "a");
    time_t t = -1;
    char *timeStr = NULL;
    if (f) {
        // header
        t = time(NULL);
        timeStr = ctime(&t);
        if (timeStr) {
            timeStr[strlen(timeStr) - 1] = '\0'; /* remove the trailing newline */
        }
        fprintf(f, "%s ~~~~~~~~~~~~~~~~~~~~~~~~~\n", timeStr);
        printSOSDataToFile(&sosInfo, sos.mailType, f);
        fputs("\n\n\n", f);
        fflush(f);
        fclose(f);
    }
    fflush(stdout);

    return NoError;
}

int parseSOSData(const char *argv[], struct SosMail *sos)
{
    unsigned int i;
    int mostSimilarIndex = 0;
    char* stringEnd;

    int hold = (unsigned int)atoi(argv[1]);
    sos->mailType = hold == 0 ? SosMailType : (hold == 1) ? AOkMailType : (hold == 2) ? ThankYouMailType : hold;
    
    /* user can input the pkmns, items, dungeons and friend zones by using its name or its index */
    sos->pkmnToRescue = (unsigned int)strtol(argv[2], &stringEnd, 10);
    if (*stringEnd) {
        sos->pkmnToRescue   = pkmnSpeciesCount; /* invalid name, invalid index */
        for (i = 0; i < pkmnSpeciesCount; ++i) {
            if (strcmp(pkmnSpeciesStr[i], argv[2]) == 0) {
                sos->pkmnToRescue = i;
                break;
            }
        }
        if (sos->pkmnToRescue == pkmnSpeciesCount) {
            mostSimilarIndex = findMostSimilarStringInArray(argv[2], pkmnSpeciesStr, pkmnSpeciesCount);
            if (mostSimilarIndex == -1) {
                printMessage(stderr, ErrorMessage, "Cannot find pokemon " LRED "\"%s\"" RESET " in the database.\n", argv[2]);
                return InputError;
            } else {
                sos->pkmnToRescue = mostSimilarIndex;
                printMessage(stderr, InfoMessage, LGREEN "%s" RESET " has been assumed.\n", pkmnSpeciesStr[mostSimilarIndex]);
            }
        }
    }
    
    sos->dungeon = (unsigned int)strtol(argv[4], &stringEnd, 10);
    if (*stringEnd) {
        sos->dungeon   = dungeonsCount; /* invalid name, invalid index */
        for (i = 0; i < dungeonsCount; ++i) {
            if (strcmp(dungeonsStr[i], argv[4]) == 0) {
                sos->dungeon = i;
                break;
            }
        }
        if (sos->dungeon == dungeonsCount) {
            mostSimilarIndex = findMostSimilarStringInArray(argv[4], dungeonsStr, dungeonsCount);
            if (mostSimilarIndex == -1) {
                printMessage(stderr, ErrorMessage, "Cannot find dungeon " LRED "\"%s\"" RESET LIGHT " in the database.\n", argv[4]);
                return InputError;
            } else {
                sos->dungeon = mostSimilarIndex;
                printMessage(stderr, InfoMessage, "Dungeon " LGREEN "%s" RESET " has been assumed.\n", dungeonsStr[mostSimilarIndex]);
            }
        }
    }
    
    
    sos->floor = (unsigned int)atoi(argv[5]);
    sos->mailID = (unsigned int)atoi(argv[6]);
    sos->chancesLeft = (unsigned int)atoi(argv[7]);
    if (strlen(argv[3]) > 0) {
        strncpy(sos->pkmnNick, argv[3], 10);
    } else {
        strncpy(sos->pkmnNick, sos->pkmnToRescue >= pkmnSpeciesCount ? "" : pkmnSpeciesStr[sos->pkmnToRescue], 10);
    }

    return NoError;
}

int convertSOS(int argc, const char *argv[])
{
    char SOSPassword[55];
    char AOKPassword[55];
    char ThankYouPassword[55];
    int itemReward;
    char *stringEnd; /* strtol() */

    struct SosMail SOSMail = { 0, 0, 0, 0, 0, 0, 0, {0}, 0, 0, 0, 0, 0, 0, 0 };
    struct SosMail AOKMail = { 0, 0, 0, 0, 0, 0, 0, {0}, 0, 0, 0, 0, 0, 0, 0 };
    struct SosMail ThxMail = { 0, 0, 0, 0, 0, 0, 0, {0}, 0, 0, 0, 0, 0, 0, 0 };
    struct SosMailInfo SOSInfo = { {0}, {0}, {0}, {0}, {0}, {0}, {0}, 0, {0}, {0}, {0}, {0} };
    struct SosMailInfo AOKInfo = { {0}, {0}, {0}, {0}, {0}, {0}, {0}, 0, {0}, {0}, {0}, {0} };
    struct SosMailInfo ThxInfo = { {0}, {0}, {0}, {0}, {0}, {0}, {0}, 0, {0}, {0}, {0}, {0} };

    int i, j, count;
    int mostSimilarIndex = 0;
    int errorCode;
    int mailType = SosMailType; /* default type */

    FILE *f = NULL;
    time_t t = -1;
    char *timeStr = NULL;

    if (argc <= 1) {
        requestAndParseSOSMailConvertion(SOSPassword, &itemReward);
        mailType = getMailType(SOSPassword);
        if (mailType == AOkMailType) {
            strncpy(AOKPassword, SOSPassword, 54);
        } else if (mailType == ThankYouMailType) {
            strncpy(ThankYouPassword, SOSPassword, 54);
        }
    }

    for (i = 1, count = 1; i < argc || i == 1; i += 2, ++count) {
        if (argc > 1) {
            fprintf(stdout, LIGHT "%d.\n" RESET, count);
            mailType = getMailType(argv[i]);
            strncpy(SOSPassword, argv[i], 54);
            if (mailType == AOkMailType) {
                strncpy(AOKPassword, argv[i], 54);
            } else if (mailType == ThankYouMailType) {
                strncpy(ThankYouPassword, argv[i], 54);
            } else if (mailType == InvalidMailType) {
                continue;
            }
        }

        if (argc > 1 && i + 1 >= argc) {
            itemReward = 0;
            printMessage(stderr, WarningMessage, "Reward item not specified. Default to " LGREEN "\"%s\"" RESET LIGHT ".\n", itemsStr[itemReward]);
        } else if (argc > 1) {
            itemReward = strtol(argv[i + 1], &stringEnd, 10);
            if (*stringEnd) { /* non-digit found */
                itemReward = itemsCount; /* invalid name, invalid index */
                for (j = 0; j < (int)itemsCount; ++j) {
                    if (strcmp(itemsStr[j], argv[i + 1]) == 0) {
                        itemReward = j;
                        break; /* item found */
                    }
                }

                if ((unsigned int)itemReward == itemsCount) {
                    printMessage(stderr, ErrorMessage, "Cannot find item " LRED "\"%s\"" RESET " in the database.\n", argv[i + 1]);
                    mostSimilarIndex = findMostSimilarStringInArray(argv[i + 1], itemsStr, itemsCount);
                    itemReward = mostSimilarIndex == -1 ? 0 : mostSimilarIndex;
                    printMessage(stderr, InfoMessage, LGREEN "%s" RESET " has been assumed.\n", itemsStr[itemReward]);
                }
            } else {
                printMessage(stdout, InfoMessage, "(%s" "%d" RESET ") %s\n", (unsigned int)itemReward < itemsCount ? LGREEN : LRED, (unsigned int)itemReward, (unsigned int)itemReward < itemsCount ? itemsStr[(unsigned int)itemReward] : LRED "[INVALID]" RESET);
            }

            errorCode = checkItem(itemReward);
            if (errorCode == ItemCannotBeObtainedError) {
                printMessage(stderr, ErrorMessage, "Item " LRED "%d" RESET " [" LRED "%s" RESET "] cannot be obtained as reward.\n\n", itemReward, itemsStr[itemReward]);
                continue;
            } else if (errorCode == ItemOutOfRangeError) {
                printMessage(stderr, ErrorMessage, "Items to find or deliver must be numbers between " LGREEN "1" RESET " [" LGREEN "%s" RESET "] and " LGREEN "%d" RESET " [" LGREEN "%s" RESET "]. Current value: %u [INVALID]\n\n", itemsStr[1], itemsCount - 5, itemsStr[itemsCount - 5], itemReward);
                continue;
            }
        }

        errorCode = convertSosMail(SOSPassword, 0, itemReward, AOKPassword, ThankYouPassword);
        if (errorCode) {
            printMessage(stderr, ErrorMessage, "Conversion error.\n");
            continue;
        }

        /* Bulking the mail's data... */
        if (mailType != SosMailType) {
            SOSPassword[0] = '\0';
        }
        f = fopen(LOG_SOS_FILENAME, "a");
        t = time(NULL);
        timeStr = ctime(&t);
        if (timeStr) {
            timeStr[strlen(timeStr) - 1] = '\0'; /* remove the trailing newline */
        }
        // header
        fprintf(f, "%s ~~~~~~~~~~~~~~~~~~~~~~~~~\n", timeStr);

        if (strlen(SOSPassword) == 54) {
            fputs(RESET "=================== SOS Mail =====================\n", stdout);
            if (decodeSosMail(SOSPassword, &SOSMail) != NoError) {
                fprintf(stderr, "Cannot show SOS Mail. Errors ocurr.\n");
                continue;
            }
            setSosInfo(&SOSMail, &SOSInfo);
            strncpy(SOSInfo.password, SOSPassword, 54);
            printSOSData(&SOSInfo, &SOSMail);
            fputc('\n', stdout);
            if (f) {
                printSOSDataToFile(&SOSInfo, SOSMail.mailType, f);
                fputc('\n', f);
            }
        }
        if (strlen(AOKPassword) == 54) {
            if (decodeSosMail(AOKPassword, &AOKMail) != NoError) {
                continue;
            }
            fputs(RESET "=================== A-OK Mail ====================\n", stdout);
            setSosInfo(&AOKMail, &AOKInfo);
            strncpy(AOKInfo.password, AOKPassword, 54);
            printSOSData(&AOKInfo, &AOKMail);
            fputc('\n', stdout);
            if (f) {
                printSOSDataToFile(&AOKInfo, AOKMail.mailType, f);
                fputc('\n', f);
            }
        }
        if (strlen(ThankYouPassword) == 54) {
            if (decodeSosMail(ThankYouPassword, &ThxMail) != NoError) {
                continue;
            }
            fputs(RESET "================= Thank-You Mail =================\n", stdout);
            setSosInfo(&ThxMail, &ThxInfo);
            strncpy(ThxInfo.password, ThankYouPassword, 54);
            printSOSData(&ThxInfo, &ThxMail);
            if (f) {
                printSOSDataToFile(&ThxInfo, ThxMail.mailType, f);
                fputc('\n', f);
            }
        }
        if (f) {
            fputs("\n\n\n", f);
            fflush(f);
            fclose(f);
        }
        fflush(stdout);
    } /* for loop */
    fflush(stdout);

    return NoError;
}

int generateMassiveItemMissions(int dungeon, int item, int amount)
{
    int errorCode = checkDungeon(dungeon, SosMailType);
    if (errorCode == DungeonOutOfRangeError) {
        printMessage(stderr, ErrorMessage, "The dungeon must be between " LGREEN "0" RESET " [" LGREEN "%s" RESET "] and " LGREEN "%d" RESET " [" LGREEN "%s" RESET "]. Current value: " LRED "%u" RESET " [" LGREEN "INVALID" RESET "]\n\n", dungeonsStr[0], dungeonsCount - 1, dungeonsStr[dungeonsCount - 1], dungeon);
        return errorCode;
    } else if (errorCode == DungeonIsInvalidError) {
        printMessage(stderr, ErrorMessage, "The dungeon " LRED "%u" RESET " [" LRED "INVALID" RESET "] is not a valid dungeon.\n\n", dungeon);
        return errorCode;
    } else if (errorCode == MissionCannotBeAcceptedInDungeonError) {
        printMessage(stderr, WarningMessage, "A mission in dungeon " LYELLOW "%u" RESET " [" LYELLOW "%s" RESET "] can be generated, but cannot be done.\n\n", dungeon, (unsigned int)dungeon < dungeonsCount ? dungeonsStr[dungeon] : "INVALID");
    } 
    
    if (amount > (difficulties[dungeon][0] - forbiddenFloorsInDungeons[dungeon][0])) {
        printMessage(stderr, WarningMessage, "No enough floors. Amount truncated to " LGREEN "%d" RESET ".\n", difficulties[dungeon][0] - forbiddenFloorsInDungeons[dungeon][0]);
        amount = difficulties[dungeon][0] - forbiddenFloorsInDungeons[dungeon][0];
    }

    struct WonderMail wm = { WonderMailType, HelpMe, 0, 0, 0, 0, MoneyMoneyItem, item, 0, 0, 0, 0xFF, dungeon, 0 };
    struct WonderMailInfo wmInfo = { {0}, {0}, {0}, {0}, {0}, {0}, {0}, '\0', {0}, {0} };
    char password[25] = {0};
    int i;
    FILE *f = NULL;
    time_t t = -1;
    char *timeStr = NULL;
    for (i = 0; i < amount; ++i) {
        wm.floor = i + 1;
        while (checkFloor(wm.floor, wm.dungeon) != NoError) {
            wm.floor++;
        }
        while (checkPokemon(wm.pkmnClient, WonderMailType) != NoError) {
            wm.pkmnClient = 1 + rand() % (pkmnSpeciesCount - 11);
        }
        wm.pkmnTarget = wm.pkmnClient;
        encodeWonderMail(&wm, password, 1);
        setWonderMailInfo(&wm, &wmInfo);
        strncpy(wmInfo.password, password, 24);
        printWonderMailData(&wmInfo, &wm);
        if (i < amount - 1) {
            fputc('\n', stdout);
        }
        f = fopen(LOG_WM_FILENAME, "a");
        if (f) {
            // header
            t = time(NULL);
            timeStr = ctime(&t);
            if (timeStr) {
                timeStr[strlen(timeStr) - 1] = '\0'; /* remove the trailing newline */
            }
            fprintf(f, "%s ~~~~~~~~~~~~~~~~~~~~~~~\n", timeStr);
            printWonderMailDataToFile(&wmInfo, f);
            fputs("\n\n\n", f);
            fflush(f);
            fclose(f);
        }
    }
    return NoError;
}

int generateMassiveHighRankMissions(int dungeon, int item, int amount)
{
    int errorCode = checkDungeon(dungeon, SosMailType);
    if (errorCode == DungeonOutOfRangeError) {
        printMessage(stderr, ErrorMessage, "The dungeon must be between " LGREEN "0" RESET " [" LGREEN "%s" RESET "] and " LGREEN "%d" RESET " [" LGREEN "%s" RESET "]. Current value: " LRED "%u" RESET " [" LGREEN "INVALID" RESET "]\n\n", dungeonsStr[0], dungeonsCount - 1, dungeonsStr[dungeonsCount - 1], dungeon);
        return errorCode;
    } else if (errorCode == DungeonIsInvalidError) {
        printMessage(stderr, ErrorMessage, "The dungeon " LRED "%u" RESET " [" LRED "INVALID" RESET "] is not a valid dungeon.\n\n", dungeon);
        return errorCode;
    } else if (errorCode == MissionCannotBeAcceptedInDungeonError) {
        printMessage(stderr, WarningMessage, "A mission in dungeon " LYELLOW "%u" RESET " [" LYELLOW "%s" RESET "] can be generated, but cannot be done.\n\n", dungeon, (unsigned int)dungeon < dungeonsCount ? dungeonsStr[dungeon] : "INVALID");
    }
    
    if (amount > (difficulties[dungeon][0] - forbiddenFloorsInDungeons[dungeon][0])) {
        printMessage(stderr, WarningMessage, "No enough floors. Truncated to " LGREEN "%d" RESET ".\n", difficulties[dungeon][0] - forbiddenFloorsInDungeons[dungeon][0]);
        amount = difficulties[dungeon][0] - forbiddenFloorsInDungeons[dungeon][0];
    }

    struct WonderMail wm = { WonderMailType, HelpMe, 0, 0, 0, 0, MoneyMoneyItem, item, 0, 0, 0, 0xFF, dungeon, 0 };
    struct WonderMailInfo wmInfo = { {0}, {0}, {0}, {0}, {0}, {0}, {0}, '\0', {0}, {0} };
    char password[25] = {0};

    FILE *f = NULL;
    time_t t = -1;
    char *timeStr = NULL;

    char calculatedDiffChar = 'E';
    char diffColor[50] = {0};
    int targetRank = 12;
    int i;

    /* locate the first appareance of a high rank floor */
    for (i = 1; i <= difficulties[dungeon][0]; ++i) {
        if (difficulties[dungeon][i] >= targetRank) {
            break;
        } else if (i == difficulties[dungeon][0] && targetRank > 1) {
            calculatedDiffChar = difficultiesChars[(targetRank >> 1) > 6 ? 6 : (targetRank >> 1)];
            strcpy(diffColor, calculatedDiffChar == 'E' ? RESET : calculatedDiffChar == 'D' || calculatedDiffChar == 'C' ? COLOR_GREEN : calculatedDiffChar == 'B' || calculatedDiffChar == 'A' ? COLOR_CYAN : calculatedDiffChar == 'S' ? COLOR_RED : LYELLOW);
            printMessage(stderr, WarningMessage, "The dungeon " LGREEN "%u" RESET LIGHT " [" LGREEN "%s" RESET LIGHT "] cannot provide %s%c" RESET LIGHT " rank missions.\n", dungeon, ((unsigned int)dungeon >= dungeonsCount) ? "INVALID" : dungeonsStr[dungeon], diffColor, calculatedDiffChar);
            while (calculatedDiffChar == difficultiesChars[(targetRank >> 1) > 6 ? 6 : (targetRank >> 1)]) {
                targetRank--; /* try again with a lower rank */
            }
            i = 0; /* it is increased to 1 for the next iterations */
        }
    }
    /* if `i` is too close to the end of the dungeon, assign an appropriate value to `i` */
    if ((i + amount + forbiddenFloorsInDungeons[dungeon][0] - 1) > (difficulties[dungeon][0])) {
        i = difficulties[dungeon][0] - forbiddenFloorsInDungeons[dungeon][0] - amount + 1;
    }
    /* now generate the mails */
    int top = i + amount;
    for (; i < top; ++i) {
        wm.floor = i;
        while (checkFloor(wm.floor, wm.dungeon) != NoError) {
            wm.floor++;
        }
        while (checkPokemon(wm.pkmnClient, WonderMailType)) {
            wm.pkmnClient = 1 + rand() % (pkmnSpeciesCount - 11);
        }
        wm.pkmnTarget = wm.pkmnClient;
        encodeWonderMail(&wm, password, 1);
        setWonderMailInfo(&wm, &wmInfo);
        strncpy(wmInfo.password, password, 24);
        printWonderMailData(&wmInfo, &wm);
        if (i < top - 1) {
            fputc('\n', stdout);
        }
        f = fopen(LOG_WM_FILENAME, "a");
        if (f) {
            // header
            t = time(NULL);
            timeStr = ctime(&t);
            if (timeStr) {
                timeStr[strlen(timeStr) - 1] = '\0'; /* remove the trailing newline */
            }
            fprintf(f, "%s ~~~~~~~~~~~~~~~~~~~~~~~\n", timeStr);
            printWonderMailDataToFile(&wmInfo, f);
            fputs("\n\n\n", f);
            fflush(f);
            fclose(f);
        }
    }
    return NoError;
}

int unlockExclusivePokemon(enum GameType gameType)
{
    if (gameType != RedRescueTeam && gameType != BlueRescueTeam) {
        printMessage(stderr, ErrorMessage, "Unrecognized game type.\n");
        return InputError;
    }
    int pokemonRedRescueTeam[]  = { 137, 251, 336, 340, 374 }; /* Porygon, Mantine, Plusle, Roselia and Feebas */
    int pokemonBlueRescueTeam[] = { 129, 131, 190, 337 }; /* Magikarp, Lapras, Aipom and Minum */

    struct WonderMail wm = { WonderMailType, HelpMe, 0, 0, 0, 0, MoneyMoney, 0x09, 0, 0, 0, 0xFF, 0, 1 };
    struct WonderMailInfo wmInfo = { {0}, {0}, {0}, {0}, {0}, {0}, {0}, '\0', {0}, {0} };
    char password[25] = {0};

    FILE *f = NULL;
    time_t t = -1;
    char *timeStr = NULL;

    int i;
    int top = gameType == RedRescueTeam ? 5 : 4;
    for (i = 0; i < top; ++i) {
        wm.pkmnClient = gameType == RedRescueTeam ? pokemonRedRescueTeam[i] : pokemonBlueRescueTeam[i];
        wm.pkmnTarget = wm.pkmnClient;
        encodeWonderMail(&wm, password, 1);
        setWonderMailInfo(&wm, &wmInfo);
        strncpy(wmInfo.password, password, 24);
        printWonderMailData(&wmInfo, &wm);
        if (i < top - 1) {
            fputc('\n', stdout);
        }
        f = fopen(LOG_WM_FILENAME, "a");
        if (f) {
            // header
            t = time(NULL);
            timeStr = ctime(&t);
            if (timeStr) {
                timeStr[strlen(timeStr) - 1] = '\0'; /* remove the trailing newline */
            }
            fprintf(f, "%s ~~~~~~~~~~~~~~~~~~~~~~~\n", timeStr);
            printWonderMailDataToFile(&wmInfo, f);
            fputs("\n\n\n", f);
            fflush(f);
            fclose(f);
        }
    }
    return NoError;
}

int unlockDungeons()
{
    int dungeonsToUnlock[] = { 44, 45, 46 }; /* Remains Island, Marvelous Sea and Fantasy Strait */
    struct WonderMail wm = { WonderMailType, HelpMe, 0, 0, 0, 0, MoneyMoney, 0x09, 0, 0, 0, 0xFF, 0, 1 };
    struct WonderMailInfo wmInfo = { {0}, {0}, {0}, {0}, {0}, {0}, {0}, '\0', {0}, {0} };
    char password[25] = {0};

    FILE *f = NULL;
    time_t t = -1;
    char *timeStr = NULL;

    int i;
    for (i = 0; i < 3; ++i) {
        wm.dungeon = dungeonsToUnlock[i];
        while (checkPokemon(wm.pkmnClient, WonderMailType)) {
            wm.pkmnClient = 1 + rand() % (pkmnSpeciesCount - 11);
        }
        wm.pkmnTarget = wm.pkmnClient;
        encodeWonderMail(&wm, password, 1);
        setWonderMailInfo(&wm, &wmInfo);
        strncpy(wmInfo.password, password, 24);
        printWonderMailData(&wmInfo, &wm);
        if (i < 2) {
            fputc('\n', stdout);
        }
        f = fopen(LOG_WM_FILENAME, "a");
        if (f) {
            // header
            t = time(NULL);
            timeStr = ctime(&t);
            if (timeStr) {
                timeStr[strlen(timeStr) - 1] = '\0'; /* remove the trailing newline */
            }
            fprintf(f, "%s ~~~~~~~~~~~~~~~~~~~~~~~\n", timeStr);
            printWonderMailDataToFile(&wmInfo, f);
            fputs("\n\n\n", f);
            fflush(f);
            fclose(f);
        }
    }
    return NoError;
}