#if !defined(FPS_DIGITS_H)
#define FPS_DIGITS_H

namespace GM {

    class FpsDigits {
        public:
            static char * getDigit(int nr);
            static char zero[18][15];
            static char one[18][15];
            static char two[18][15];
            static char three[18][15];
            static char four[18][15];
            static char five[18][15];
            static char six[18][15];
            static char seven[18][15];
            static char eight[18][15];
            static char nine[18][15];
            static char blank[18][15];
    };
}

#endif /* !defined(FPS_DIGITS_H) */
