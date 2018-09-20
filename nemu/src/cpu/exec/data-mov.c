#include "cpu/exec.h"

make_EHelper(mov) {

    operand_write(id_dest, &id_src->val);
    print_asm_template2(mov);
}

make_EHelper(push) {
    if (decoding.is_operand_size_16) {
        rtl_push_16((uint16_t *)&id_dest->val);
        print_asm_template1(push);
    } else {
        rtl_push(&id_dest->val);
        print_asm_template1(push);
    }

}


make_EHelper(pop) {
//  TODO();
   rtl_pop(&t0);
   operand_write(id_dest,&t0);


}

make_EHelper(pusha) {
    t1 = reg_l(R_ESP);
    rtl_push(&reg_l(R_EAX));
    rtl_push(&reg_l(R_ECX));
    rtl_push(&reg_l(R_EDX));
    rtl_push(&reg_l(R_EBX));
    rtl_push(&t1);
    rtl_push(&reg_l(R_EBP));
    rtl_push(&reg_l(R_ESI));
    rtl_push(&reg_l(R_EDI));
    print_asm("pusha");
}

make_EHelper(popa) {
    rtl_push(&reg_l(R_EDI));
    rtl_push(&reg_l(R_ESI));
    rtl_push(&reg_l(R_EBP));
    //ignore R_ESP
    rtl_push(&reg_l(R_EBX));

    rtl_push(&reg_l(R_EBX));
    rtl_push(&reg_l(R_EDX));
    rtl_push(&reg_l(R_ECX));
    rtl_push(&reg_l(R_EAX));

    print_asm("popa");
}

make_EHelper(leave) {
//  TODO();
    rtl_lr_l(&reg_l(R_ESP), R_EBP);
    if (decoding.is_operand_size_16) {
        rtl_pop_16((uint16_t*)&reg_w(R_BP));
    } else {
        rtl_pop(&reg_l(R_EBP));
    }
    print_asm("leave");
}

make_EHelper(cltd) {
    if (decoding.is_operand_size_16) {
        rtl_msb(&t0, (rtlreg_t*)&reg_w(R_AX), 2);
        rtl_sr_w(R_DX,&tzero);
        if (t0) {
            rtl_xor(&t0,&t0,&t0);
            rtl_not(&t0);
            rtl_sr_w(R_DX,&t0);
        }

    } else {
        rtl_msb(&t0, &reg_l(R_EAX), 4);
        rtl_xor(&reg_l(R_EDX), &reg_l(R_EDX), &reg_l(R_EDX));
        if (t0) { rtl_not(&reg_l(R_EDX)); }

    }

    print_asm(decoding.is_operand_size_16 ? "cwtl" : "cltd");
}

make_EHelper(cwtl) {
    if (decoding.is_operand_size_16) {
        TODO();
    } else {
        TODO();
    }

    print_asm(decoding.is_operand_size_16 ? "cbtw" : "cwtl");
}

make_EHelper(movsx) {
    id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
    rtl_xor(&t2,&t2,&t2);
    rtl_sext(&t2, &id_src->val, id_src->width);
    operand_write(id_dest, &t2);
    print_asm_template2(movsx);
}

make_EHelper(movzx) {

    id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
    operand_write(id_dest, &id_src->val);
    print_asm_template2(movzx);
}

make_EHelper(lea) {
    rtl_li(&t2, id_src->addr);
    operand_write(id_dest, &t2);
    print_asm_template2(lea);
}
