#include <stdio.h>
#include "src/floatConst.h"     // To make values based on float.h more readable
#include <float.h>
//# include <xmmintrin.h>     // SSE instructions
# include <time.h>          // Performance messuring
# include <math.h>          // For comparasion to approximation


// Math approximation libary functions
# include "src/fastLog2.h"
# include "src/fastLog.h"
# include "src/fastExp.h"
# include "src/fastExp2.h"
# include "src/fastSqrt.h"
# include "src/fastReciprocal.h"

# include "src/fastPow.h"
# include "src/fastArithmetic.h"
# include "src/fastPaulMineiro.h"   // Exp2 & log2




// NOTE: Values very close to zero produce massive errors (technically correct but misleading?)
float percentError(const float exact, const float estimate) {
    return 100 * (estimate - exact) / ( (exact > estimate)? exact : estimate );
}

#define CMP <
#define FUNCTIONS exp2f(i), fastExp2_accurate(i)

int main() {

    const int MIN_RANGE = -20;
    const int MAX_RANGE = 20;
    const double STEP_SIZE = 0.1;

    float maxError = 0;
    float maxErr_i = 0;
    for (float i = MIN_RANGE; i < MAX_RANGE; i += STEP_SIZE) {
        float error = percentError(FUNCTIONS);
        if (error CMP maxError) {
            maxError = error;
            maxErr_i = i;
        }
    }
    printf("Max error at %.2f: %+.4f%%\n", maxErr_i, maxError);



    for (float i = maxErr_i - STEP_SIZE; i < maxErr_i + STEP_SIZE; i += STEP_SIZE / 100) {
        float error = percentError(FUNCTIONS);

        //printf("%3.3f:\t%.4f vs %.4f    (%+.4f%%)\n", i, FUNCTIONS, error);

        if (error CMP maxError) {
            maxError = error;
            maxErr_i = i;
        }
    }
    printf("\n\n");
    printf("Max error at %.4f: %+.4f%%\t(%.4e)\n", maxErr_i, maxError, maxError);


    float totalError = 0;
    size_t count = 0;
    for (float i = MIN_RANGE; i < MAX_RANGE; i += STEP_SIZE / 100) {
        totalError += fabs(percentError(FUNCTIONS));
        count++;
    }
    printf("Average error: %+.4f%%\t(%.4e)\n", totalError / count, totalError / count);
}