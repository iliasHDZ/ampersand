bits 32
section .text

extern int_isr_handler
extern int_isr_regs

extern isr_src_stack_esp
extern isr_src_stack_ebp

global isr_return

extern syscall_handler_func

isr_common_stub:
    pusha
    mov ax, ds
    push eax
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov [int_isr_regs], esp

    mov [isr_src_stack_esp], esp
    mov [isr_src_stack_ebp], ebp

    mov esp, isr_handler_stack_top
    mov ebp, esp

    call int_isr_handler

isr_return:
    mov esp, [isr_src_stack_esp]
    mov ebp, [isr_src_stack_ebp]

    mov eax, [syscall_handler_func]
    cmp eax, 0
    je .no_syscall
    sti
    call eax
    cli


.no_syscall:
    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    popa
    add esp, 8
    iret

section .data

isr_handler_stack_bottom:
    resb 4096 * 32
isr_handler_stack_top:
    dd 0x00000000

section .text

%macro DEFINE_ISR 1
global isr_ %+ %1

isr_ %+ %1 :
    cli
    push dword 0
    push dword %1
    jmp isr_common_stub
%endmacro

%macro DEFINE_ISR_W_ERR_CODE 1
global isr_ %+ %1

isr_ %+ %1 :
    cli
    nop
    nop
    push dword %1
    jmp isr_common_stub
%endmacro

; CPU Exceptions (ID 0 - 31)
DEFINE_ISR 0
DEFINE_ISR 1
DEFINE_ISR 2
DEFINE_ISR 3
DEFINE_ISR 4
DEFINE_ISR 5
DEFINE_ISR 6
DEFINE_ISR 7
DEFINE_ISR_W_ERR_CODE 8
DEFINE_ISR 9
DEFINE_ISR_W_ERR_CODE 10
DEFINE_ISR_W_ERR_CODE 11
DEFINE_ISR_W_ERR_CODE 12
DEFINE_ISR_W_ERR_CODE 13
DEFINE_ISR_W_ERR_CODE 14
DEFINE_ISR 15
DEFINE_ISR 16
DEFINE_ISR 17
DEFINE_ISR 18
DEFINE_ISR 19
DEFINE_ISR 20
DEFINE_ISR 21
DEFINE_ISR 22
DEFINE_ISR 23
DEFINE_ISR 24
DEFINE_ISR 25
DEFINE_ISR 26
DEFINE_ISR 27
DEFINE_ISR 28
DEFINE_ISR 29
DEFINE_ISR 30
DEFINE_ISR 31

; External Interrupts (ID 32 - 255)
DEFINE_ISR 32
DEFINE_ISR 33
DEFINE_ISR 34
DEFINE_ISR 35
DEFINE_ISR 36
DEFINE_ISR 37
DEFINE_ISR 38
DEFINE_ISR 39
DEFINE_ISR 40
DEFINE_ISR 41
DEFINE_ISR 42
DEFINE_ISR 43
DEFINE_ISR 44
DEFINE_ISR 45
DEFINE_ISR 46
DEFINE_ISR 47
DEFINE_ISR 48
DEFINE_ISR 49
DEFINE_ISR 50
DEFINE_ISR 51
DEFINE_ISR 52
DEFINE_ISR 53
DEFINE_ISR 54
DEFINE_ISR 55
DEFINE_ISR 56
DEFINE_ISR 57
DEFINE_ISR 58
DEFINE_ISR 59
DEFINE_ISR 60
DEFINE_ISR 61
DEFINE_ISR 62
DEFINE_ISR 63
DEFINE_ISR 64
DEFINE_ISR 65
DEFINE_ISR 66
DEFINE_ISR 67
DEFINE_ISR 68
DEFINE_ISR 69
DEFINE_ISR 70
DEFINE_ISR 71
DEFINE_ISR 72
DEFINE_ISR 73
DEFINE_ISR 74
DEFINE_ISR 75
DEFINE_ISR 76
DEFINE_ISR 77
DEFINE_ISR 78
DEFINE_ISR 79
DEFINE_ISR 80
DEFINE_ISR 81
DEFINE_ISR 82
DEFINE_ISR 83
DEFINE_ISR 84
DEFINE_ISR 85
DEFINE_ISR 86
DEFINE_ISR 87
DEFINE_ISR 88
DEFINE_ISR 89
DEFINE_ISR 90
DEFINE_ISR 91
DEFINE_ISR 92
DEFINE_ISR 93
DEFINE_ISR 94
DEFINE_ISR 95
DEFINE_ISR 96
DEFINE_ISR 97
DEFINE_ISR 98
DEFINE_ISR 99
DEFINE_ISR 100
DEFINE_ISR 101
DEFINE_ISR 102
DEFINE_ISR 103
DEFINE_ISR 104
DEFINE_ISR 105
DEFINE_ISR 106
DEFINE_ISR 107
DEFINE_ISR 108
DEFINE_ISR 109
DEFINE_ISR 110
DEFINE_ISR 111
DEFINE_ISR 112
DEFINE_ISR 113
DEFINE_ISR 114
DEFINE_ISR 115
DEFINE_ISR 116
DEFINE_ISR 117
DEFINE_ISR 118
DEFINE_ISR 119
DEFINE_ISR 120
DEFINE_ISR 121
DEFINE_ISR 122
DEFINE_ISR 123
DEFINE_ISR 124
DEFINE_ISR 125
DEFINE_ISR 126
DEFINE_ISR 127
DEFINE_ISR 128
DEFINE_ISR 129
DEFINE_ISR 130
DEFINE_ISR 131
DEFINE_ISR 132
DEFINE_ISR 133
DEFINE_ISR 134
DEFINE_ISR 135
DEFINE_ISR 136
DEFINE_ISR 137
DEFINE_ISR 138
DEFINE_ISR 139
DEFINE_ISR 140
DEFINE_ISR 141
DEFINE_ISR 142
DEFINE_ISR 143
DEFINE_ISR 144
DEFINE_ISR 145
DEFINE_ISR 146
DEFINE_ISR 147
DEFINE_ISR 148
DEFINE_ISR 149
DEFINE_ISR 150
DEFINE_ISR 151
DEFINE_ISR 152
DEFINE_ISR 153
DEFINE_ISR 154
DEFINE_ISR 155
DEFINE_ISR 156
DEFINE_ISR 157
DEFINE_ISR 158
DEFINE_ISR 159
DEFINE_ISR 160
DEFINE_ISR 161
DEFINE_ISR 162
DEFINE_ISR 163
DEFINE_ISR 164
DEFINE_ISR 165
DEFINE_ISR 166
DEFINE_ISR 167
DEFINE_ISR 168
DEFINE_ISR 169
DEFINE_ISR 170
DEFINE_ISR 171
DEFINE_ISR 172
DEFINE_ISR 173
DEFINE_ISR 174
DEFINE_ISR 175
DEFINE_ISR 176
DEFINE_ISR 177
DEFINE_ISR 178
DEFINE_ISR 179
DEFINE_ISR 180
DEFINE_ISR 181
DEFINE_ISR 182
DEFINE_ISR 183
DEFINE_ISR 184
DEFINE_ISR 185
DEFINE_ISR 186
DEFINE_ISR 187
DEFINE_ISR 188
DEFINE_ISR 189
DEFINE_ISR 190
DEFINE_ISR 191
DEFINE_ISR 192
DEFINE_ISR 193
DEFINE_ISR 194
DEFINE_ISR 195
DEFINE_ISR 196
DEFINE_ISR 197
DEFINE_ISR 198
DEFINE_ISR 199
DEFINE_ISR 200
DEFINE_ISR 201
DEFINE_ISR 202
DEFINE_ISR 203
DEFINE_ISR 204
DEFINE_ISR 205
DEFINE_ISR 206
DEFINE_ISR 207
DEFINE_ISR 208
DEFINE_ISR 209
DEFINE_ISR 210
DEFINE_ISR 211
DEFINE_ISR 212
DEFINE_ISR 213
DEFINE_ISR 214
DEFINE_ISR 215
DEFINE_ISR 216
DEFINE_ISR 217
DEFINE_ISR 218
DEFINE_ISR 219
DEFINE_ISR 220
DEFINE_ISR 221
DEFINE_ISR 222
DEFINE_ISR 223
DEFINE_ISR 224
DEFINE_ISR 225
DEFINE_ISR 226
DEFINE_ISR 227
DEFINE_ISR 228
DEFINE_ISR 229
DEFINE_ISR 230
DEFINE_ISR 231
DEFINE_ISR 232
DEFINE_ISR 233
DEFINE_ISR 234
DEFINE_ISR 235
DEFINE_ISR 236
DEFINE_ISR 237
DEFINE_ISR 238
DEFINE_ISR 239
DEFINE_ISR 240
DEFINE_ISR 241
DEFINE_ISR 242
DEFINE_ISR 243
DEFINE_ISR 244
DEFINE_ISR 245
DEFINE_ISR 246
DEFINE_ISR 247
DEFINE_ISR 248
DEFINE_ISR 249
DEFINE_ISR 250
DEFINE_ISR 251
DEFINE_ISR 252
DEFINE_ISR 253
DEFINE_ISR 254
DEFINE_ISR 255