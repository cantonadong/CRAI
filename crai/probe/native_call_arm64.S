// AArch64 调用桥：libg 里有些函数用 x8 传回结果指针（隐式返回值），C 里没法直接这样调用。
// 写法参考 FirstLight_CR native_runner/probe/native_call_arm64.S（Apache-2.0）。

    .text
    .align 2
    .global cr_call_logic_command_parse
    .type cr_call_logic_command_parse, %function

// void cr_call_logic_command_parse(void* json_object /*x0*/, void* command_context /*x1*/,
//                                  void** result /*x2 -> x8*/, void* target /*x3*/);
cr_call_logic_command_parse:
    stp x29, x30, [sp, #-16]!
    mov x29, sp
    mov x8, x2
    blr x3
    ldp x29, x30, [sp], #16
    ret

    .size cr_call_logic_command_parse, .-cr_call_logic_command_parse

    .align 2
    .global cr_call_card_selection_build
    .type cr_call_card_selection_build, %function

// void cr_call_card_selection_build(void* battle_deck_slot /*x0*/, void* player /*x1*/, int alternate /*w2*/,
//                                   void* result /*x3 -> x8*/, void* target /*x4*/);
// result 布局：{ void* card_data; void* secondary_data; uint32 packed(pd); uint32 pad; }（24 字节）
cr_call_card_selection_build:
    stp x29, x30, [sp, #-16]!
    mov x29, sp
    mov x8, x3
    blr x4
    ldp x29, x30, [sp], #16
    ret

    .size cr_call_card_selection_build, .-cr_call_card_selection_build
