#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <time.h>

#define MAX_INPUT_SIZE (4096)

size_t thresh_precedes_whitespace = 64;
size_t thresh_max_value = 75;
size_t thresh_min_value = 1;
bool   showsyllablegaps = true;

typedef enum
{
        CONSONANT_MP,
        CONSONANT_NT,
        CONSONANT_NK,
        CONSONANT_NQ,
        CONSONANT_TH,
        CONSONANT_S,
        CONSONANT_RH,
        CONSONANT_H,
        CONSONANT_M,
        CONSONANT_N,
        CONSONANT_NG,
        CONSONANT_P,
        CONSONANT_T,
        CONSONANT_K,
        CONSONANT_Q,
        CONSONANT_L,
        CONSONANT_R,
        CONSONANT_COUNT,
        VOWEL_A,
        VOWEL_E,
        VOWEL_I,
        VOWEL_O,
        VOWEL_U,
        VOWEL_COUNT = VOWEL_U - CONSONANT_COUNT,

        SYM_BREAK = VOWEL_U + 1,
        INVALID_PHONEME,
        WHITESPACE,
} TOKEN;

typedef enum
{
        TYPE_INVALID,
        TYPE_V,
        TYPE_CV,
        TYPE_VC,
        TYPE_CVC,
} SYLLABLETYPE;

typedef struct
{
        SYLLABLETYPE Type;
        TOKEN Onset, Nucleus, Coda;
        bool precedes_whitespace;
} SYLLABLE;

/* Put longest first, as we break on first find */
const char AsciiPhonologyConsonants[][3] = {
    "mp",
    "nt",
    "nk",
    "nq",
    "th",
    "s",
    "rh",
    "h",
    "m",
    "n",
    "g",
    "p",
    "t",
    "k",
    "q",
    "l",
    "r",
};

const char AsciiPhonologyVowels[] = "aeiou";

int ncsstrncmp(const char *restrict const a, const char *restrict const b, size_t max)
{
        for (size_t i = 0; i < max; ++i)
        {
                if (tolower((unsigned char)a[i]) == tolower((unsigned char)b[i]))
                        continue;
                return b[i] - a[i];
        }

        return 0;
}

TOKEN FindToken(const char **const Base)
{
        if (*(*Base) == '.')
        {
                ++(*Base);
                return SYM_BREAK;
        }
        else if (*(*Base) == '\0')
        {
                return INVALID_PHONEME;
        }
        else if (isspace(*(*Base)))
        {
                while (isspace(*(*Base)))
                        ++(*Base);
                return WHITESPACE;
        }

        for (size_t i = 0; i < sizeof(AsciiPhonologyConsonants) / sizeof(AsciiPhonologyConsonants[0]); ++i)
        {
                if (!ncsstrncmp(AsciiPhonologyConsonants[i], (*Base), strlen(AsciiPhonologyConsonants[i])))
                {
                        fprintf(stderr, " [TOKEN INFO] Found %s as %ld\n", AsciiPhonologyConsonants[i], i);
                        (*Base) += strlen(AsciiPhonologyConsonants[i]);
                        return i;
                }
        }

        for (size_t i = 0; i < sizeof(AsciiPhonologyVowels) / sizeof(AsciiPhonologyVowels[0]); ++i)
        {
                if (*(*Base) == AsciiPhonologyVowels[i])
                {
                        fprintf(stderr, " [TOKEN INFO] Found %c as %ld\n", *(*Base), i);
                        ++(*Base);
                        return i + VOWEL_I;
                }
        }

        return INVALID_PHONEME;
}

SYLLABLE ParseSyllable(const char **const string)
{
        SYLLABLE Syllable = {
            .Type = TYPE_INVALID,
            .Onset = INVALID_PHONEME,
            .Nucleus = INVALID_PHONEME,
            .Coda = INVALID_PHONEME,
            .precedes_whitespace = false};

        const char *original = *string;
        TOKEN Initial = FindToken(string), preceding;

        /* C Initial */
        if (Initial < CONSONANT_COUNT)
        {
                Syllable.Type = TYPE_CV;
                Syllable.Onset = Initial;
                TOKEN Nucleus = FindToken(string);
                if (Nucleus <= CONSONANT_COUNT)
                {
                        fprintf(stderr, " [PARSER ERROR] Syntax Error: %d\n", Nucleus);
                        exit(1);
                }

                Syllable.Nucleus = Nucleus;
                const char *temp = *string;
                TOKEN Coda = FindToken(&temp);

                if (Coda < CONSONANT_COUNT)
                {
                        Syllable.Coda = FindToken(string);
                        Syllable.Type = TYPE_CVC;
                }

                if (Coda == SYM_BREAK)
                        fprintf(stderr, " [PARSER INFO] Manual Break\n");
        }

        /* V Initial */
        else if (Initial > CONSONANT_COUNT && Initial <= VOWEL_U)
        {
                Syllable.Type = TYPE_V;
                Syllable.Onset = INVALID_PHONEME;
                Syllable.Nucleus = Initial;
                const char *temp = *string;
                TOKEN Coda = FindToken(&temp);

                if (Coda < CONSONANT_COUNT)
                {
                        Syllable.Coda = FindToken(string);
                        Syllable.Type = TYPE_VC;
                }
        }

        /* Break */
        else if (Initial == SYM_BREAK)
        {
                return ParseSyllable(string);
        }

        original = *string;
        preceding = FindToken(&original);
        if (preceding == WHITESPACE)
        {
                FindToken(string);
                Syllable.precedes_whitespace = true;
        }
        return Syllable;
}

size_t randomn(size_t min, size_t max)
{
        return min + (rand() % (max - min + 1));
}

/**
 * Generate Gibberish that follows our phonotactics
 */
SYLLABLE GenerateSyllable(void)
{
        static bool whiteSpacePresent = false;

        SYLLABLE Syllable = {
            .Type = TYPE_INVALID,
            .Onset = INVALID_PHONEME,
            .Nucleus = INVALID_PHONEME,
            .Coda = INVALID_PHONEME,
            .precedes_whitespace = false};

        if (!whiteSpacePresent && randomn(thresh_min_value, thresh_max_value) <= thresh_precedes_whitespace)
        {
                fprintf(stderr, " [GENERATE INFO] Precedes whitespace\n");
                Syllable.precedes_whitespace = true;
                whiteSpacePresent = true;
        }
        else
                whiteSpacePresent = false;

        Syllable.Onset = randomn(CONSONANT_MP, CONSONANT_R + 1);
        if (Syllable.Onset == CONSONANT_COUNT)
                Syllable.Onset = INVALID_PHONEME;
        Syllable.Coda = randomn(CONSONANT_MP, CONSONANT_R + 1);
        if (Syllable.Coda == CONSONANT_COUNT)
                Syllable.Coda = SYM_BREAK;
        Syllable.Nucleus = randomn(VOWEL_A - VOWEL_A, VOWEL_U - VOWEL_A) + VOWEL_A;
        Syllable.Type = (Syllable.Onset == INVALID_PHONEME) && (Syllable.Coda == SYM_BREAK) ? TYPE_V : (Syllable.Onset == INVALID_PHONEME) ? TYPE_VC
                                                                                                   : (Syllable.Coda == SYM_BREAK)          ? TYPE_CV
                                                                                                                                           : TYPE_CVC;
        return Syllable;
}

void PrintSyllable(const SYLLABLE *const s, FILE *fp)
{
        if (s->Onset != INVALID_PHONEME)
                fprintf(fp, "%s", AsciiPhonologyConsonants[s->Onset]);
        fprintf(fp, "%c", AsciiPhonologyVowels[s->Nucleus - VOWEL_A]);
        if (s->Coda != INVALID_PHONEME && s->Coda != SYM_BREAK)
                fprintf(fp, "%s", AsciiPhonologyConsonants[s->Coda]);
        if (s->precedes_whitespace)
                fprintf(fp, " ");
        else if (showsyllablegaps)
                fprintf(fp, ".");
}

int main(const int argc, const char *const *const argv)
{
        SYLLABLE Syllable = {0};
        const size_t count = 24;
        char *const input = malloc(MAX_INPUT_SIZE);
        const char *string = input;
        FILE *fp;

        srand(time(NULL));
        if (!input)
                return 1;

        /**
         * TODO - Factor out using function pointers
         */
        for (size_t i = 1; i < (size_t)argc; ++i)
        {
                const size_t remaining = argc - i;
                if (remaining >= 1 && !ncsstrncmp(argv[i], "/max", 5))
                {
                        if (remaining < 2)
                        {
                                fprintf(stderr, " [ERROR] please provide a threshold maximum\n");
                                free(input);
                                return 1;
                        }

                        thresh_max_value = strtoul(argv[i+1], NULL, 10);
                        i+=1;
                }
                else if (remaining >= 1 && !ncsstrncmp(argv[i], "/min", 5))
                {
                        if (remaining < 2)
                        {
                                fprintf(stderr, " [ERROR] please provide a threshold minimum\n");
                                free(input);
                                return 1;
                        }

                        thresh_min_value = strtoul(argv[i+1], NULL, 10);
                        i+=1;
                }
                else if (remaining >= 1 && !ncsstrncmp(argv[i], "/s", 2))
                        showsyllablegaps = true;
                else if (remaining >= 1 && !ncsstrncmp(argv[i], "/h", 2))
                        showsyllablegaps = false;
                else if (remaining >= 1 && !ncsstrncmp(argv[i], "/thr", 5))
                {
                        if (remaining < 2)
                        {
                                fprintf(stderr, " [ERROR] please provide a threshold for the whitespace\n");
                                free(input);
                                return 1;
                        }

                        thresh_precedes_whitespace = strtoul(argv[i+1], NULL, 10);
                        i+=1;
                }
                else if (remaining >= 1 && !ncsstrncmp(argv[i], "/p", 3))
                {
                        if (remaining < 2)
                        {
                                fprintf(stderr, " [ERROR] please provide a file to parse\n");
                                free(input);
                                return 1;
                        }
                        fp = fopen(argv[i+1], "rb");
                        if (!fp)
                        {
                                fprintf(stderr, " [ERROR] could not open file '%s' for reading\n", argv[2]);
                                free(input);
                                return 1;
                        }

                        fread(input, 1, MAX_INPUT_SIZE, fp);

                        do
                        {
                                Syllable = ParseSyllable(&string);
                                if (Syllable.Type != TYPE_INVALID)
                                        fprintf(stderr, " [PARSER INFO] syllable found as: %d(%d,%d,%d)\n", Syllable.Type, Syllable.Onset, Syllable.Nucleus, Syllable.Coda);
                        } while (Syllable.Type != TYPE_INVALID);
                        i+=1;
                        fclose(fp);
                        fp = NULL;
                }
                else if (remaining >= 1 && !ncsstrncmp(argv[i], "/g", 3))
                {
                        if (remaining < 2)
                        {
                                fprintf(stderr, " [ERROR] please provide a file to generate\n");
                                free(input);
                                return 1;
                        }

                        fp = fopen(argv[i+1], "wb");
                        if (!fp)
                        {
                                fprintf(stderr, " [ERROR] could not open file '%s' for writing\n", argv[2]);
                                free(input);
                                return 1;
                        }

                        for (size_t i = 0; i < count; ++i)
                        {
                                Syllable = GenerateSyllable();
                                if (i == count - 1)
                                        Syllable.precedes_whitespace = true;
                                PrintSyllable(&Syllable, fp);
                                fprintf(stderr, " [GENERATE INFO] syllable found as: %d(%d,%d,%d)\n", Syllable.Type, Syllable.Onset, Syllable.Nucleus, Syllable.Coda);
                        }
                        i+=1;
                        fclose(fp);
                        fp = NULL;
                }
                else
                {
                        fprintf(stderr, " [ERROR] Invalid option '%s'\n", argv[i]);
                        free(input);
                        return 1;
                }
        }

        free(input);
}
