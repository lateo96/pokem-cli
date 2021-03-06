#include "../../../../test/CuTest.h"

#include "UtilEncode.h"

#include <stdlib.h>


CuSuite* UtilEncodeGetTestSuite(void);
void bitUnpackingEncoding_test(CuTest *tc);


CuSuite* UtilEncodeGetTestSuite()
{
    CuSuite *suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, bitUnpackingEncoding_test);
    return suite;
}

void bitUnpackingEncoding_test(CuTest *tc)
{
#define ARRAY_SIZE 5
    /* The ZERO value after the END HERE comment is the terminal character of every string, and it cannot be taken in account when packing */
    const char input1[ARRAY_SIZE][56] = {
        { 0x86, 0x05, 0xB8, 0x7B, 0x37, 0xC1, 0x6C, 0x00, 0x87, 0xE4, 0x7F, 0x97, 0x00, 0x00, 0x00 /* END HERE */, 0x00 }, /* 4?8**HX?48648Q?Q6R?6*N?W (WM - VALID) */
        { 0x78, 0x15, 0xC8, 0xA0, 0x2F, 0xC1, 0xF4, 0x80, 0x86, 0x8E, 0x7F, 0x90, 0x0B, 0x00, 0x00 /* END HERE */, 0x00 }, /* F?8NS5.?4RK!14?/6P6RQN?W (WM - VALID) */
        { 0xA4, 0x25, 0x08, 0x40, 0x39, 0xC1, 0x00, 0x80, 0x86, 0xB4, 0x7F, 0x90, 0x0B, 0x00, 0x00 /* END HERE */, 0x00 }, /* F?8N?-T?49KMP??R6P?0?N?W (WM - VALID) */
        { 0x6F, 0xA1, 0x93, 0xF5, 0x68, 0x27, 0xC6, 0x91, 0x26, 0x28, 0x9F, 0x73, 0xAA, 0x93, 0x1B, 0xCB, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x28, 0xD4, 0xDA, 0x82, 0x01, 0x00, 0x00, 0x00, 0x50, 0x00 /* END HERE */, 0x00 }, /* NW??2-R.9R/??Y09?!?5????NFC??!??33H???X?-RN??51??TH0R. (SOS - VALID) */
        { 0x03, 0x08, 0x18, 0x14, 0x00, 0x06, 0x05, 0x02, 0x00, 0x13, 0x17, 0x14, 0x1F, 0x1A, 0x07, 0x15, 0x01, 0x17, 0x02, 0x04, 0x11, 0x15, 0x0C, 0x0C, 0x03, 0x08, 0x04, 0x15, 0x17, 0x09, 0x0C, 0x05, 0x03, 0x1B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x1D, 0x0B, 0x12, 0x01, 0x00, 0x15, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00 /* END HERE */, 0x00 }  /* S6???.RF?6F??NWH*5KC???RH1!9?8?JK7P0??SNMJRPSKJ??7QJ4N (SOS - VALID) */
    };
    char input2[ARRAY_SIZE][56] = { {0} };
    const int input3[ARRAY_SIZE] = { 15, 15, 15, 34, 34 };

    const char expected[ARRAY_SIZE][35] = {
        { 0x06, 0x0C, 0x01, 0x10, 0x1B, 0x1D, 0x1D, 0x06, 0x01, 0x06, 0x1B, 0x00, 0x10, 0x03, 0x12 /* END HERE */, 0x00 },
        { 0x18, 0x0B, 0x05, 0x10, 0x0C, 0x10, 0x1E, 0x05, 0x01, 0x06, 0x1D, 0x01, 0x08, 0x03, 0x1A /* END HERE */, 0x00 },
        { 0x04, 0x0D, 0x09, 0x10, 0x00, 0x00, 0x05, 0x07, 0x01, 0x06, 0x00, 0x00, 0x08, 0x03, 0x12 /* END HERE */, 0x00 },
        { 0x0F, 0x0B, 0x08, 0x07, 0x19, 0x1A, 0x03, 0x0D, 0x07, 0x11, 0x11, 0x03, 0x09, 0x13, 0x00, 0x05, 0x1F, 0x1C, 0x1C, 0x14, 0x1A, 0x09, 0x0E, 0x03, 0x0B, 0x1E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* END HERE */, 0x00 },
        { 0x03, 0x00, 0x02, 0x10, 0x01, 0x0A, 0x00, 0x00, 0x06, 0x08, 0x01, 0x04, 0x00, 0x00, 0x0C, 0x02, 0x17, 0x00, 0x05, 0x1E, 0x01, 0x0D, 0x1C, 0x00, 0x15, 0x08, 0x00, 0x0E, 0x01, 0x01, 0x10, 0x00, 0x11, 0x08 /* END HERE */, 0x00 }
    };

    int i, j;
    for (i = 0; i < ARRAY_SIZE; ++i) {
        bitUnpackingEncoding(input1[i], input2[i], input3[i]);
        for (j = 0; j < input3[i]; ++j) {
            CuAssertIntEquals(tc, expected[i][j], input2[i][j]);
        }
    }
#undef ARRAY_SIZE
}
