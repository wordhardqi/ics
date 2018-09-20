#include "cpu/rtl.h"

/* Condition Code */

void rtl_setcc(rtlreg_t *dest, uint8_t subcode) {
    bool invert = subcode & 0x1;
    enum {
        /*0 */ CC_O, CC_NO, CC_B, CC_NB,
        /*4 */ CC_E, CC_NE, CC_BE, CC_NBE,
        /*8 */ CC_S, CC_NS, CC_P, CC_NP,
        /*c */ CC_L, CC_NL, CC_LE, CC_NLE
    };

    // TODO: Query EFLAGS to determine whether the condition code is satisfied.
    // dest <- ( cc is satisfied ? 1 : 0)
    rtlreg_t tmp_SF;
    rtlreg_t tmp_OF;
    rtlreg_t tmp_ZF;
    rtlreg_t tmp_CF;
    switch (subcode & 0xe) {
        case CC_O:
            rtl_get_OF(dest);
            break;
        case CC_B:
            rtl_get_CF(dest);
            break;
        case CC_E:
            rtl_get_ZF(dest);
            break;
        case CC_BE:
            rtl_get_CF(&tmp_CF);
            rtl_get_ZF(&tmp_ZF);
            *dest = tmp_CF || tmp_ZF;
            break;
        case CC_S:
            rtl_get_SF(dest);
            break;
        case CC_L:
            rtl_get_SF(&tmp_SF);
            rtl_get_OF(&tmp_OF);
            *dest = (tmp_OF !=tmp_SF) ? 1 : 0;
            break;
        case CC_LE:
            rtl_get_SF(&tmp_SF);
            rtl_get_OF(&tmp_OF);
            rtl_get_ZF(&tmp_ZF);
            *dest =(rtlreg_t)( (tmp_ZF == 1) || (tmp_SF !=tmp_OF));
            break;
        default:
            panic("should not reach here");
        case CC_P:
            panic("n86 does not have PF");
    }

    if (invert) {
        rtl_xori(dest, dest, 0x1);
    }
}
