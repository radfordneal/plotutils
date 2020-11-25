/* This header file specifies HP's Roman-8 encoding (used by HP's Stick
   fonts, including variable-width arc fonts).  In particular, it gives a
   mapping from the upper half of the ISO-Latin-1 character set to
   Roman-8. */

#define HPGL2_STICK 48		/* HP-GL/2 typeface #48 is Stick */

/* ISO-Latin-1 characters not included in Roman-8; we map each of them to
   040, i.e., to the space character. */
#define COPYRIGHT 040
#define NEGATION 040
#define REGISTERED 040
#define RAISEDONE 040
#define RAISEDTWO 040
#define RAISEDTHREE 040
#define CEDILLA 040
#define MULTIPLY 040
#define DIVIDES 040

static const unsigned char _iso_to_roman8 [128] =
{
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    /* printable iso-latin-1 characters */
    040, 0270, 0277, 0257, 0272, 0274, (unsigned char)'|', 0275,
    0253, COPYRIGHT, 0371, 0373, NEGATION, 0366, REGISTERED, 0260,
    0263, 0376, RAISEDTWO, RAISEDTHREE, 0250, 0363, 0364, 0362, 
    CEDILLA, RAISEDONE, 0263, 0375, 0367, 0370, 0365, 0271,
    0241, 0340, 0242, 0341, 0330, 0320, 0323, 0264,
    0243, 0334, 0244, 0245, 0346, 0345, 0246, 0247,
    0343, 0266, 0350, 0347, 0337, 0351, 0332, MULTIPLY,
    0322, 0255, 0355, 0256, 0333, 0261, 0360, 0336,
    0310, 0304, 0300, 0342, 0314, 0324, 0327, 0265,
    0311, 0305, 0301, 0315, 0331, 0325, 0321, 0335,
    0344, 0267, 0312, 0306, 0302, 0352, 0316, DIVIDES,
    0326, 0313, 0307, 0303, 0317, 0262, 0361, 0357,
};
