; threads.asm
global thread_switch_asm

thread_switch_asm:
    ; Guardar contexto actual
    push ebp
    push ebx
    push esi
    push edi
    
    ; Guardar stack pointer actual
    mov eax, [esp+20]    ; old_sp
    mov [eax], esp
    
    ; Cargar nuevo stack pointer
    mov esp, [esp+24]    ; new_sp
    
    ; Restaurar contexto
    pop edi
    pop esi
    pop ebx
    pop ebp
    
    ret