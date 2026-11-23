// ===== 0xd9a2cc =====
// entry d9a2cc

void FUN_00e9a2cc(float param_1,long param_2)

{
  int iVar1;
  uint uVar2;
  char cVar3;
  void *pvVar4;
  undefined8 *puVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  undefined4 uVar8;
  long lVar9;
  ulong uVar10;
  undefined8 uVar11;
  undefined8 uVar12;
  long lVar13;
  long lVar14;
  long *plVar15;
  undefined8 uVar16;
  void *pvVar17;
  long *plVar18;
  undefined1 auStack_108 [16];
  future<void> afStack_f8 [8];
  future<void> afStack_f0 [8];
  future<void> afStack_e8 [8];
  undefined8 local_e0;
  long *plStack_d8;
  void *local_d0;
  undefined8 uStack_c8;
  undefined8 *local_c0;
  undefined8 uStack_b8;
  undefined8 local_b0;
  undefined8 uStack_a8;
  long local_a0;
  undefined8 uStack_98;
  undefined8 uStack_90;
  undefined8 uStack_88;
  long *local_80;
  undefined8 uStack_78;
  undefined8 local_70;
  undefined8 uStack_68;
  future<void> afStack_58 [8];
  
  iVar1 = *(int *)(param_2 + 0x30);
  if (4 < iVar1) {
    if (iVar1 < 7) {
      if (iVar1 == 5) {
        uVar10 = FUN_00e9b2cc(param_2);
        if ((uVar10 & 1) == 0) {
          return;
        }
        uVar16 = 6;
      }
      else {
        if (iVar1 != 6) {
          return;
        }
        lVar9 = FUN_00ea0658();
        lVar13 = *(long *)(param_2 + 0x90);
        if (((lVar13 == 0) || ((*(long *)(lVar13 + 0xa8) == 0 && (*(long *)(lVar13 + 0x220) == 0))))
           && ((uVar10 = FUN_00ea20c8(lVar9), (uVar10 & 1) == 0 || (*(int *)(lVar9 + 0x14) != 3))))
        {
          uVar10 = FUN_00ea22c4(lVar9);
          if ((uVar10 & 1) == 0) {
            return;
          }
          uVar16 = FUN_00ea22d4(lVar9);
          FUN_00ea22ec(lVar9);
          uVar11 = FUN_00ea0664(lVar9);
          uVar7 = FUN_00ea22e4(lVar9);
          uVar8 = FUN_00ea2378(lVar9);
          lVar13 = *(long *)(param_2 + 0x90);
          uVar6 = *(undefined4 *)(lVar9 + 0x60);
          *(undefined8 *)(lVar13 + 0x1c8) = *(undefined8 *)(lVar9 + 0x70);
          FUN_012d2c80(lVar13,uVar6);
          FUN_012d29cc(*(undefined8 *)(param_2 + 0x90),uVar16,uVar11,uVar7,0xffffffff,uVar8);
          FUN_012d316c(*(undefined8 *)(param_2 + 0x90));
          uVar11 = FUN_011fae98();
          FUN_012d316c(*(undefined8 *)(param_2 + 0x90));
          uVar12 = FUN_011f5f6c();
          FUN_00ea2b58(uVar11,uVar12);
          lVar9 = FUN_012d316c(*(undefined8 *)(param_2 + 0x90));
          FUN_0121b8e4(*(undefined8 *)(lVar9 + 0x460));
          FUN_012d316c(*(undefined8 *)(param_2 + 0x90));
          FUN_00eaf12c();
          FUN_00e9954c(param_2,7);
          uVar10 = FUN_00ef6a78();
          if ((uVar10 & 1) == 0) {
            return;
          }
          FUN_011fbdf8(uVar16);
          return;
        }
        uVar16 = 7;
      }
    }
    else {
      if (iVar1 == 7) {
        if (*(long **)(param_2 + 0x98) != (long *)0x0) {
          (**(code **)(**(long **)(param_2 + 0x98) + 8))();
        }
        *(undefined8 *)(param_2 + 0x98) = 0;
        if (*(int *)(param_2 + 0x58) == 2) {
          plVar18 = (long *)FUN_00db5fe8(param_2,*(undefined4 *)(param_2 + 0x244));
        }
        else if (*(int *)(param_2 + 0x58) == 0) {
          FUN_00ea05dc(&DAT_004149c1);
          plVar18 = operator_new(0x510);
          FUN_00db8fb8(plVar18,param_2);
        }
        else {
          FUN_00e9b420(param_2);
          lVar9 = FUN_014bbf84();
          if (lVar9 != 0) {
            FUN_00978c00();
            lVar13 = FUN_00ff9c28();
            if (*(char *)(lVar13 + 0x5d8) != '\0') {
              FUN_014bcbc4(lVar9);
            }
          }
          *DAT_01b9b078 = 0;
          plVar18 = operator_new(0x4e0);
          FUN_00da9eb4(plVar18,param_2);
        }
        *(long **)(param_2 + 0x50) = plVar18;
        uVar16 = (**(code **)(*plVar18 + 0xa40))();
        *(undefined8 *)(param_2 + 0x38) = uVar16;
        lVar9 = FUN_008c2808();
        if (*(char *)(lVar9 + 0x25d) != '\0') {
          *(undefined1 *)(*(long *)(param_2 + 0x38) + 8) = 0;
        }
        lVar9 = FUN_008c2808();
        FUN_0145e318(*(undefined8 *)(lVar9 + 0x170),*(undefined8 *)(param_2 + 0x38));
        FUN_00e9b7a4(param_2,*(undefined8 *)(param_2 + 0x50));
        cVar3 = *DAT_01b99af0;
        FUN_01640938(&local_e0,*(undefined8 *)PTR_PTR_01b9aa68,*(undefined8 *)PTR_PTR_01b9ab98);
        FUN_008c2808();
        uVar16 = FUN_008c2cec();
        if (cVar3 != '\0') {
          FUN_0163de50(auStack_108,*(undefined8 *)PTR_PTR_01b9ac20);
          FUN_00cc347c(uVar16,&local_e0,auStack_108);
                    /* WARNING: Subroutine does not return */
          FUN_0163dd30(auStack_108);
        }
        (**(code **)(**(long **)(param_2 + 0x50) + 0x9b8))();
        *(undefined1 *)(param_2 + 0x17d) = 1;
        if (*(int *)(param_2 + 0x58) == 0) {
          uVar16 = 8;
          *(undefined4 *)(param_2 + 0x34) = 0;
        }
        else {
          uVar16 = 9;
        }
        FUN_00e9954c(param_2,uVar16);
                    /* WARNING: Subroutine does not return */
        FUN_0163dd30(&local_e0);
      }
      if (iVar1 != 8) {
        return;
      }
      lVar9 = FUN_00db8d28();
      if (((lVar9 != 0) && (uVar10 = FUN_00dbe0ec(), (uVar10 & 1) != 0)) &&
         (*(float *)(param_2 + 0x34) < 2.0)) {
        *(float *)(param_2 + 0x34) = *(float *)(param_2 + 0x34) + param_1;
        return;
      }
      uVar16 = 9;
    }
    FUN_00e9954c(param_2,uVar16);
    return;
  }
  if (iVar1 < 3) {
    if (1 < iVar1 - 1U) {
      if (iVar1 != 0) {
        return;
      }
      if (*(int *)(param_2 + 0x58) == 2) {
        lVar9 = FUN_00ea0658();
        if (*(int *)(lVar9 + 0x14) == 3) {
LAB_00e9a80c:
          uVar16 = 3;
          goto LAB_00e9add8;
        }
      }
      else {
        if (*(int *)(param_2 + 0x58) != 1) {
          plVar18 = &local_a0;
          uStack_78 = 0;
          local_80 = (long *)0x0;
          uStack_68 = 0;
          plStack_d8 = (long *)0x0;
          local_e0 = 0;
          uStack_c8 = 0;
          local_d0 = (void *)0x0;
          uStack_b8 = 0;
          local_c0 = (undefined8 *)0x0;
          uStack_a8 = 0;
          local_b0 = 0x3f800000;
          local_70 = 1;
          uStack_98 = 0;
          local_a0 = 0;
          uStack_88 = 0;
          uStack_90 = 0;
          FUN_01428d44(afStack_58,*(undefined8 *)(param_2 + 0x248),&local_e0);
          std::__ndk1::future<void>::~future(afStack_58);
          if (local_80 == plVar18) {
            lVar9 = 4;
LAB_00e9abc4:
            (**(code **)(*plVar18 + lVar9 * 8))();
            pvVar4 = local_d0;
            puVar5 = local_c0;
          }
          else {
            pvVar4 = local_d0;
            puVar5 = local_c0;
            if (local_80 != (long *)0x0) {
              lVar9 = 5;
              plVar18 = local_80;
              goto LAB_00e9abc4;
            }
          }
          while (puVar5 != (void *)0x0) {
            plVar15 = (long *)puVar5[8];
            plVar18 = puVar5 + 4;
            pvVar17 = (void *)*puVar5;
            local_d0 = pvVar4;
            if (plVar15 == plVar18) {
              lVar9 = 4;
LAB_00e9ac2c:
              (**(code **)(*plVar18 + lVar9 * 8))();
            }
            else if (plVar15 != (long *)0x0) {
              lVar9 = 5;
              plVar18 = plVar15;
              goto LAB_00e9ac2c;
            }
            operator_delete(puVar5);
            pvVar4 = local_d0;
            puVar5 = pvVar17;
          }
          local_d0 = (void *)0x0;
          if (pvVar4 != (void *)0x0) {
            operator_delete(pvVar4);
          }
          plVar18 = plStack_d8;
          if ((plStack_d8 != (long *)0x0) &&
             (lVar9 = FUN_01a89e40(0xffffffffffffffff,plStack_d8 + 1), lVar9 == 0)) {
            (**(code **)(*plVar18 + 0x10))(plVar18);
            std::__ndk1::__shared_weak_count::__release_weak();
          }
          uVar16 = 5;
          goto LAB_00e9add8;
        }
        lVar9 = FUN_00ea0658();
        iVar1 = *(int *)(lVar9 + 0x14);
        if (iVar1 == 4) {
          uVar16 = 4;
          goto LAB_00e9add8;
        }
        if (iVar1 == 3) goto LAB_00e9a80c;
        if (iVar1 == 1) {
          uVar16 = 2;
          goto LAB_00e9add8;
        }
      }
      uVar16 = 1;
LAB_00e9add8:
      FUN_00e9954c(param_2,uVar16);
      *(undefined4 *)(param_2 + 0x74) = 0x3f800000;
      return;
    }
  }
  else {
    if (iVar1 == 3) {
      FUN_00ea0658();
      uVar10 = FUN_00ea20c8();
      if ((uVar10 & 1) == 0) {
        return;
      }
      FUN_00ea0658();
      lVar9 = FUN_00ea20c0();
      if (lVar9 == 0) {
        FUN_014998d8("replay json object is null",1);
      }
      FUN_00cb40bc();
      if (*(int *)(param_2 + 0x58) == 2) {
        uVar10 = FUN_012d2dc4(*(undefined8 *)(param_2 + 0x90),lVar9,0,param_2 + 0x10);
        if ((uVar10 & 1) == 0) {
          FUN_00e9910c(param_2);
          uVar16 = FUN_00ea0658();
          FUN_00ea20b8(uVar16,3,0);
          return;
        }
        if (*(int *)(param_2 + 0x244) == 1) {
          FUN_00d390e8(*(undefined8 *)(*(long *)(param_2 + 0x90) + 0x220),
                       *(undefined8 *)(param_2 + 0x248));
        }
        uStack_78 = 0;
        local_80 = (long *)0x0;
        uStack_68 = 0;
        plStack_d8 = (long *)0x0;
        local_e0 = 0;
        uStack_c8 = 0;
        local_d0 = (void *)0x0;
        uStack_b8 = 0;
        local_c0 = (undefined8 *)0x0;
        uStack_a8 = 0;
        local_b0 = 0x3f800000;
        local_70 = 1;
        uStack_98 = 0;
        local_a0 = 0;
        uStack_88 = 0;
        uStack_90 = 0;
        FUN_01428d44(afStack_f0,*(undefined8 *)(param_2 + 0x248),&local_e0);
        std::__ndk1::future<void>::~future(afStack_f0);
        FUN_00d6ff74(&local_e0);
        uVar6 = FUN_01419e08();
        *(undefined4 *)(param_2 + 0xa0) = uVar6;
        *(undefined4 *)(param_2 + 0xbc) = 0;
        *(undefined4 *)(param_2 + 0xa4) = 0;
        *(undefined4 *)(param_2 + 0xa8) = 0;
        FUN_00e9954c(param_2,5);
        FUN_00e9910c(param_2);
        return;
      }
      FUN_00ea0658();
      lVar13 = FUN_00ea20d8();
      uVar10 = FUN_00ef6a4c();
      if (((uVar10 & 1) == 0) && (lVar13 == 0)) {
        FUN_014998d8("replay is missing battle log json",1);
      }
      lVar14 = FUN_0149ca3c(lVar9,DAT_01b9b128,1);
      if (lVar14 == 0) {
        FUN_014998d8("cannot find battle object from replay json",1);
      }
      lVar14 = FUN_0149cadc(lVar14,"location",8,1);
      if (lVar14 != 0) {
        uVar16 = FUN_00978c00();
        iVar1 = *(int *)(lVar14 + 0x10);
        if (*(char *)(lVar14 + 8) != '\0') {
          iVar1 = (int)*(float *)(lVar14 + 0xc);
        }
        FUN_00ff9f60(uVar16,iVar1);
        FUN_009ae750();
      }
      lVar14 = *(long *)(param_2 + 0x90);
      uVar16 = *(undefined8 *)(lVar14 + 0xa8);
      if ((*DAT_01b9afd0 != -1) && (DAT_01b9afd0[1] != -1)) {
        FUN_00f20d30(uVar16,DAT_01b9afd0);
        lVar14 = *(long *)(param_2 + 0x90);
      }
      FUN_012d2dc4(lVar14,lVar9,lVar13,param_2 + 0x10);
      FUN_00db4274(uVar16,*(undefined8 *)(param_2 + 0x248),0);
      plVar18 = &local_a0;
      uStack_78 = 0;
      local_80 = (long *)0x0;
      uStack_68 = 0;
      plStack_d8 = (long *)0x0;
      local_e0 = 0;
      uStack_c8 = 0;
      local_d0 = (void *)0x0;
      uStack_b8 = 0;
      local_c0 = (undefined8 *)0x0;
      uStack_a8 = 0;
      local_b0 = 0x3f800000;
      local_70 = 1;
      uStack_98 = 0;
      local_a0 = 0;
      uStack_88 = 0;
      uStack_90 = 0;
      FUN_01428d44(afStack_f8,*(undefined8 *)(param_2 + 0x248),&local_e0);
      std::__ndk1::future<void>::~future(afStack_f8);
      if (local_80 == plVar18) {
        lVar14 = 4;
      }
      else {
        pvVar4 = local_d0;
        puVar5 = local_c0;
        if (local_80 == (long *)0x0) goto joined_r0x00e9ac80;
        lVar14 = 5;
        plVar18 = local_80;
      }
      (**(code **)(*plVar18 + lVar14 * 8))();
      pvVar4 = local_d0;
      puVar5 = local_c0;
joined_r0x00e9ac80:
      do {
        if (puVar5 == (void *)0x0) {
          local_d0 = (void *)0x0;
          if (pvVar4 != (void *)0x0) {
            operator_delete(pvVar4);
          }
          plVar18 = plStack_d8;
          if ((plStack_d8 != (long *)0x0) &&
             (lVar14 = FUN_01a89e40(0xffffffffffffffff,plStack_d8 + 1), lVar14 == 0)) {
            (**(code **)(*plVar18 + 0x10))(plVar18);
            std::__ndk1::__shared_weak_count::__release_weak();
          }
          uVar6 = FUN_01419e08();
          *(undefined4 *)(param_2 + 0xa0) = uVar6;
          *(undefined4 *)(param_2 + 0xbc) = 0;
          *(undefined4 *)(param_2 + 0xa4) = 0;
          *(undefined4 *)(param_2 + 0xa8) = 0;
          FUN_00e9954c(param_2,5);
          uVar2 = *(uint *)(param_2 + 0x16c);
          if (0 < (int)uVar2) {
            uVar10 = (ulong)uVar2 + 1;
            lVar14 = (ulong)uVar2 * 0x20 + -0x18;
            do {
              plVar18 = *(long **)(*(long *)(param_2 + 0x160) + lVar14);
              if (plVar18 != (long *)0x0) {
                (**(code **)(*plVar18 + 0x38))(plVar18);
                (**(code **)(*plVar18 + 8))(plVar18);
              }
              uVar10 = uVar10 - 1;
              lVar14 = lVar14 + -0x20;
            } while (1 < uVar10);
          }
          *(undefined4 *)(param_2 + 0x16c) = 0;
          FUN_00978c00();
          lVar14 = FUN_00ff9c28();
          if (*(char *)(lVar14 + 0x1e4) == '\0') {
            return;
          }
          FUN_00e9ae04(param_2,lVar9,lVar13);
          lVar9 = FUN_00ea0658();
          if (*(int *)(lVar9 + 0x98) < 1) {
            return;
          }
          lVar9 = FUN_00ea0658();
          FUN_00e9b294(param_2,*(undefined4 *)(lVar9 + 0x98));
          lVar9 = FUN_00ea0658();
          *(undefined4 *)(lVar9 + 0x98) = 0;
          return;
        }
        plVar15 = (long *)puVar5[8];
        plVar18 = puVar5 + 4;
        pvVar17 = (void *)*puVar5;
        local_d0 = pvVar4;
        if (plVar15 == plVar18) {
          lVar14 = 4;
LAB_00e9ad44:
          (**(code **)(*plVar18 + lVar14 * 8))();
        }
        else if (plVar15 != (long *)0x0) {
          lVar14 = 5;
          plVar18 = plVar15;
          goto LAB_00e9ad44;
        }
        operator_delete(puVar5);
        pvVar4 = local_d0;
        puVar5 = pvVar17;
      } while( true );
    }
    if (iVar1 != 4) {
      return;
    }
  }
  if (*(int *)(param_2 + 0x58) == 2) {
    if (*(long *)(param_2 + 0x90) == 0) {
      return;
    }
    uVar10 = FUN_012d6c44();
    if ((uVar10 & 1) == 0) {
      return;
    }
    if (*(int *)(param_2 + 0x244) == 1) {
      FUN_00d390e8(*(undefined8 *)(*(long *)(param_2 + 0x90) + 0x220),
                   *(undefined8 *)(param_2 + 0x248));
    }
  }
  else if (*(int *)(param_2 + 0x58) == 1) {
    if (*(long *)(param_2 + 0x90) == 0) {
      return;
    }
    if (*(long *)(*(long *)(param_2 + 0x90) + 0xa8) == 0) {
      return;
    }
    uVar10 = FUN_012d6c44();
    if ((uVar10 & 1) == 0) {
      return;
    }
    FUN_00db4274(*(undefined8 *)(*(long *)(param_2 + 0x90) + 0xa8),*(undefined8 *)(param_2 + 0x248),
                 0);
    uVar6 = FUN_01419e08();
    *(undefined4 *)(param_2 + 0xbc) = 0;
    *(undefined4 *)(param_2 + 0xa4) = 0;
    *(undefined4 *)(param_2 + 0xa8) = 0;
    *(undefined4 *)(param_2 + 0xa0) = uVar6;
  }
  plVar18 = &local_a0;
  uStack_78 = 0;
  local_80 = (long *)0x0;
  uStack_68 = 0;
  plStack_d8 = (long *)0x0;
  local_e0 = 0;
  uStack_c8 = 0;
  local_d0 = (void *)0x0;
  uStack_b8 = 0;
  local_c0 = (undefined8 *)0x0;
  uStack_a8 = 0;
  local_b0 = 0x3f800000;
  local_70 = 1;
  uStack_98 = 0;
  local_a0 = 0;
  uStack_88 = 0;
  uStack_90 = 0;
  FUN_01428d44(afStack_e8,*(undefined8 *)(param_2 + 0x248),&local_e0);
  std::__ndk1::future<void>::~future(afStack_e8);
  if (local_80 == plVar18) {
    lVar9 = 4;
  }
  else {
    pvVar4 = local_d0;
    puVar5 = local_c0;
    if (local_80 == (long *)0x0) goto joined_r0x00e9a660;
    lVar9 = 5;
    plVar18 = local_80;
  }
  (**(code **)(*plVar18 + lVar9 * 8))();
  pvVar4 = local_d0;
  puVar5 = local_c0;
joined_r0x00e9a660:
  do {
    if (puVar5 == (void *)0x0) {
      local_d0 = (void *)0x0;
      if (pvVar4 != (void *)0x0) {
        operator_delete(pvVar4);
      }
      plVar18 = plStack_d8;
      if ((plStack_d8 != (long *)0x0) &&
         (lVar9 = FUN_01a89e40(0xffffffffffffffff,plStack_d8 + 1), lVar9 == 0)) {
        (**(code **)(*plVar18 + 0x10))(plVar18);
        std::__ndk1::__shared_weak_count::__release_weak();
      }
      FUN_00e9954c(param_2,5);
      return;
    }
    plVar15 = (long *)puVar5[8];
    plVar18 = puVar5 + 4;
    pvVar17 = (void *)*puVar5;
    local_d0 = pvVar4;
    if (plVar15 == plVar18) {
      lVar9 = 4;
LAB_00e9a6bc:
      (**(code **)(*plVar18 + lVar9 * 8))();
    }
    else if (plVar15 != (long *)0x0) {
      lVar9 = 5;
      plVar18 = plVar15;
      goto LAB_00e9a6bc;
    }
    operator_delete(puVar5);
    pvVar4 = local_d0;
    puVar5 = pvVar17;
  } while( true );
}


// ===== 0xd9ae04 =====
// entry d9ae04

/* WARNING: Removing unreachable block (ram,0x00e9b1f0) */

void FUN_00e9ae04(long param_1,undefined8 param_2,undefined8 param_3)

{
  uint uVar1;
  long lVar2;
  int iVar3;
  bool bVar4;
  undefined *puVar5;
  int iVar6;
  int iVar7;
  undefined4 uVar8;
  int iVar9;
  long lVar10;
  undefined8 uVar11;
  undefined8 uVar12;
  void *pvVar13;
  long *plVar14;
  ulong uVar15;
  long lVar16;
  undefined8 *puVar17;
  int *piVar18;
  long lVar19;
  long *plVar20;
  long lVar21;
  undefined8 uVar22;
  undefined8 uVar23;
  undefined8 uVar24;
  int local_c8;
  int iStack_c4;
  undefined1 auStack_c0 [8];
  void *local_b8;
  undefined8 uStack_b0;
  int *local_a8;
  uint local_a0;
  uint local_9c;
  long *local_98;
  undefined8 local_90;
  undefined8 uStack_88;
  undefined8 uStack_80;
  undefined8 uStack_78;
  undefined8 local_70;
  undefined8 uStack_68;
  
  lVar10 = FUN_008c2808();
  if (*(int *)(lVar10 + 0x198) == 0) {
    FUN_00978c00();
    lVar10 = FUN_00ff9c28();
    if (*(char *)(lVar10 + 0x1e5) == '\0') {
      return;
    }
  }
  FUN_0167655c();
  uVar11 = FUN_00978c00();
  uVar12 = FUN_010f253c();
  pvVar13 = operator_new(0x240);
  FUN_012d1e68(pvVar13,1,uVar11,uVar12,0);
  FUN_012d225c(pvVar13,1);
  plVar14 = operator_new(0x20);
  plVar14[1] = 0;
  *plVar14 = 0;
  plVar14[3] = 0;
  plVar14[2] = 0;
  FUN_011cab40();
  puVar5 = PTR_DAT_01b9a0d0;
  uVar11 = *(undefined8 *)((long)pvVar13 + 0x40);
  plVar20 = plVar14 + 1;
  *plVar20 = 0;
  plVar14[2] = 0;
  plVar14[3] = 0x4000000000;
  *plVar14 = (long)(puVar5 + 0x10);
  local_98 = plVar14;
  FUN_0118d23c(uVar11,&local_98);
  FUN_012d2dc4(pvVar13,param_2,param_3,0);
  uVar15 = FUN_00ef6a4c();
  if (((uVar15 & 1) != 0) || (uVar15 = FUN_00ef6a70(), (uVar15 & 1) != 0)) {
    FUN_00f556b4(*(undefined8 *)((long)pvVar13 + 0x38));
  }
  lVar10 = *(long *)((long)pvVar13 + 0xa8);
  FUN_00f18fe4(lVar10,3);
  if (((*(long *)(lVar10 + 0x1b0) == 0) ||
      (lVar16 = *(long *)(*(long *)(lVar10 + 0x1b0) + 0x18), lVar16 == 0)) ||
     (*(char *)(lVar16 + 0x181) == '\0')) {
    FUN_00978c00();
    lVar16 = FUN_00ff9c28();
    local_a8 = (int *)0x0;
    local_a0 = 0;
    local_9c = 0;
    iVar9 = *(int *)(lVar16 + 0x1dc) * 0x14;
LAB_00e9af6c:
    do {
      uVar15 = FUN_00f1318c(lVar10);
      if (((uVar15 & 1) == 0) && (*(int *)(*(long *)(lVar10 + 8) + 500) < 1)) {
        bVar4 = false;
        iVar6 = FUN_012d32f0(pvVar13);
        iVar7 = 0;
        if (iVar9 != 0) {
          iVar7 = iVar6 / iVar9;
        }
        if (iVar6 == iVar7 * iVar9) goto LAB_00e9afac;
      }
      else {
        bVar4 = true;
        iVar6 = FUN_012d32f0(pvVar13);
LAB_00e9afac:
        local_c8 = iVar6;
        iStack_c4 = iVar6;
        FUN_00cb3ac4(auStack_c0,pvVar13);
        local_b8 = (void *)0x0;
        uStack_b0 = 0;
        FUN_00e9e5a4(param_1 + 0x160,&local_c8);
        lVar16 = (long)*(int *)(param_1 + 0x16c) + -1;
        if (0 < *(int *)((long)plVar14 + 0x14)) {
          lVar19 = 0;
          lVar21 = 0;
          lVar2 = *(long *)(param_1 + 0x160) + lVar16 * 0x20;
          do {
            piVar18 = (int *)(*plVar20 + lVar19);
            if (((piVar18[7] < 0) || (iVar6 < piVar18[7])) &&
               ((iVar7 = (**(code **)(**(long **)(piVar18 + 4) + 0x110))(), iVar7 < 0 ||
                (iVar6 < *piVar18 + iVar7)))) {
              iVar7 = *(int *)(lVar2 + 0x1c);
              if (iVar7 == *(int *)(lVar2 + 0x18)) {
                uStack_88 = *(undefined8 *)(piVar18 + 2);
                local_90 = *(undefined8 *)piVar18;
                uStack_78 = *(undefined8 *)(piVar18 + 6);
                uStack_80 = *(undefined8 *)(piVar18 + 4);
                uStack_68 = *(undefined8 *)(piVar18 + 10);
                local_70 = *(undefined8 *)(piVar18 + 8);
                iVar3 = 5;
                if (iVar7 * 2 != 0) {
                  iVar3 = iVar7 * 2;
                }
                if (iVar7 < iVar3) {
                  FUN_009a2044(lVar2 + 0x10);
                  iVar7 = *(int *)(lVar2 + 0x1c);
                }
                puVar17 = (undefined8 *)(*(long *)(lVar2 + 0x10) + (long)iVar7 * 0x30);
                *(int *)(lVar2 + 0x1c) = iVar7 + 1;
                puVar17[1] = uStack_88;
                *puVar17 = local_90;
                puVar17[3] = uStack_78;
                puVar17[2] = uStack_80;
                puVar17[5] = uStack_68;
                puVar17[4] = local_70;
              }
              else {
                *(int *)(lVar2 + 0x1c) = iVar7 + 1;
                puVar17 = (undefined8 *)(*(long *)(lVar2 + 0x10) + (long)iVar7 * 0x30);
                uVar22 = *(undefined8 *)(piVar18 + 4);
                uVar12 = *(undefined8 *)(piVar18 + 10);
                uVar11 = *(undefined8 *)(piVar18 + 8);
                uVar24 = *(undefined8 *)(piVar18 + 2);
                uVar23 = *(undefined8 *)piVar18;
                puVar17[3] = *(undefined8 *)(piVar18 + 6);
                puVar17[2] = uVar22;
                puVar17[5] = uVar12;
                puVar17[4] = uVar11;
                puVar17[1] = uVar24;
                *puVar17 = uVar23;
              }
            }
            lVar21 = lVar21 + 1;
            lVar19 = lVar19 + 0x30;
          } while (lVar21 < *(int *)((long)plVar14 + 0x14));
        }
        if (local_9c == local_a0) {
          iVar6 = 5;
          if (local_9c * 2 != 0) {
            iVar6 = local_9c * 2;
          }
          if ((int)local_9c < iVar6) {
            FUN_008e1410(&local_a8);
          }
        }
        uVar1 = local_9c + 1;
        local_a8[(int)local_9c] = (int)lVar16;
        local_9c = uVar1;
        if (local_b8 != (void *)0x0) {
          operator_delete__(local_b8);
        }
      }
      uVar11 = FUN_012d3174(pvVar13);
      FUN_00e9e868(uVar11,pvVar13,plVar20);
      uVar8 = FUN_012d32f0(pvVar13);
      FUN_009a1ed8(plVar20,uVar8);
      uVar15 = FUN_00f19d4c(lVar10);
      if (((uVar15 & 1) == 0) && (0 < (int)local_9c)) {
        iVar6 = FUN_012d32f0(pvVar13);
        uVar15 = (ulong)local_9c;
        if (0 < (int)local_9c) {
          lVar16 = *(long *)(param_1 + 0x160);
          piVar18 = local_a8;
          do {
            if (*(int *)(lVar16 + (long)*piVar18 * 0x20) + 1 < iVar6) {
              *(int *)(lVar16 + (long)*piVar18 * 0x20 + 4) = iVar6;
            }
            uVar15 = uVar15 - 1;
            piVar18 = piVar18 + 1;
          } while (uVar15 != 0);
        }
        local_9c = 0;
        if (bVar4) goto LAB_00e9b250;
        goto LAB_00e9af6c;
      }
    } while (!bVar4);
    if (0 < (int)local_9c) {
      iVar9 = FUN_012d32f0(pvVar13);
      uVar15 = (ulong)local_9c;
      if (0 < (int)local_9c) {
        lVar10 = *(long *)(param_1 + 0x160);
        piVar18 = local_a8;
        do {
          if (*(int *)(lVar10 + (long)*piVar18 * 0x20) + 1 < iVar9) {
            *(int *)(lVar10 + (long)*piVar18 * 0x20 + 4) = iVar9;
          }
          uVar15 = uVar15 - 1;
          piVar18 = piVar18 + 1;
        } while (uVar15 != 0);
      }
      local_9c = 0;
    }
LAB_00e9b250:
    FUN_012d2388(pvVar13);
    FUN_00afe0b0(pvVar13);
    operator_delete(pvVar13);
    if (local_a8 != (int *)0x0) {
      operator_delete__(local_a8);
    }
  }
  else {
    FUN_012d2388(pvVar13);
    FUN_00afe0b0(pvVar13);
    operator_delete(pvVar13);
  }
  return;
}


// ===== 0x11d2dc4 =====
// entry 11d2dc4

uint FUN_012d2dc4(long param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4)

{
  int iVar1;
  undefined1 uVar2;
  char cVar3;
  uint uVar4;
  long lVar5;
  long *plVar6;
  void *pvVar7;
  int iVar8;
  
  iVar8 = *(int *)(param_1 + 0x34);
  uVar2 = *(undefined1 *)(param_1 + 0x1e8);
  if (iVar8 == *(int *)(param_1 + 0x30)) {
    iVar1 = 5;
    if (iVar8 * 2 != 0) {
      iVar1 = iVar8 * 2;
    }
    if (iVar8 < iVar1) {
      FUN_00e923c0(param_1 + 0x28);
      iVar8 = *(int *)(param_1 + 0x34);
    }
  }
  lVar5 = *(long *)(param_1 + 0xa8);
  *(int *)(param_1 + 0x34) = iVar8 + 1;
  *(undefined1 *)(*(long *)(param_1 + 0x28) + (long)iVar8) = uVar2;
  *(undefined1 *)(param_1 + 0x1e8) = 1;
  if (lVar5 != 0) {
    lVar5 = FUN_00f1142c();
    *(undefined1 *)(lVar5 + 0x79) = 1;
  }
  plVar6 = *(long **)(param_1 + 0x220);
  if (plVar6 != (long *)0x0) {
    (**(code **)(*plVar6 + 0x50))(plVar6,1);
  }
  if (*(int *)(param_1 + 0x20) == 0) {
    *(undefined4 *)(param_1 + 0x20) = 4;
    if (*(long *)(param_1 + 0x18) != 0) {
      FUN_013d8fd4();
      if (*(void **)(param_1 + 0x18) != (void *)0x0) {
        operator_delete(*(void **)(param_1 + 0x18));
      }
      *(undefined8 *)(param_1 + 0x18) = 0;
    }
    pvVar7 = operator_new(0x28);
    FUN_013d8fc4(pvVar7,param_1);
    *(void **)(param_1 + 0x18) = pvVar7;
    uVar4 = FUN_013d90ec(pvVar7,param_2,param_3,param_4);
    iVar8 = *(int *)(param_1 + 0x34);
    if (iVar8 == 0) {
      FUN_014998d8("stack empty",1);
      iVar8 = *(int *)(param_1 + 0x34);
    }
    cVar3 = *(char *)(*(long *)(param_1 + 0x28) + (long)iVar8 + -1);
    *(int *)(param_1 + 0x34) = (int)((long)iVar8 + -1);
    *(char *)(param_1 + 0x1e8) = cVar3;
    if (*(long *)(param_1 + 0xa8) != 0) {
      lVar5 = FUN_00f1142c();
      *(char *)(lVar5 + 0x79) = cVar3;
    }
    plVar6 = *(long **)(param_1 + 0x220);
    if (plVar6 != (long *)0x0) {
      (**(code **)(*plVar6 + 0x50))(plVar6,cVar3 != '\0');
      return uVar4 & 1;
    }
  }
  else {
    iVar8 = *(int *)(param_1 + 0x34);
    if (iVar8 == 0) {
      FUN_014998d8("stack empty",1);
      iVar8 = *(int *)(param_1 + 0x34);
    }
    cVar3 = *(char *)(*(long *)(param_1 + 0x28) + (long)iVar8 + -1);
    *(int *)(param_1 + 0x34) = (int)((long)iVar8 + -1);
    *(char *)(param_1 + 0x1e8) = cVar3;
    if (*(long *)(param_1 + 0xa8) != 0) {
      lVar5 = FUN_00f1142c();
      *(char *)(lVar5 + 0x79) = cVar3;
    }
    plVar6 = *(long **)(param_1 + 0x220);
    if (plVar6 == (long *)0x0) {
      return 0;
    }
    (**(code **)(*plVar6 + 0x50))(plVar6,cVar3 != '\0');
    uVar4 = 0;
  }
  return uVar4 & 1;
}


// ===== 0xbae160 =====
// entry bae060

/* WARNING: Type propagation algorithm not settling */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined8 FUN_00cae060(undefined1 param_1 [16],undefined4 param_2,long param_3,long *param_4)

{
  long *plVar1;
  long *plVar2;
  undefined1 *puVar3;
  bool bVar4;
  bool bVar5;
  char cVar6;
  byte bVar7;
  byte bVar8;
  int iVar9;
  undefined4 uVar10;
  undefined4 uVar11;
  uint uVar12;
  int iVar13;
  long lVar14;
  void *pvVar15;
  undefined8 uVar16;
  undefined8 uVar17;
  long lVar18;
  long lVar19;
  void *pvVar20;
  void *pvVar21;
  undefined **ppuVar22;
  undefined8 *puVar23;
  long *plVar24;
  ulong uVar25;
  long lVar26;
  char *pcVar27;
  undefined1 uVar28;
  undefined8 *puVar29;
  undefined8 *puVar30;
  undefined ***pppuVar31;
  byte bVar32;
  long lVar33;
  undefined **local_2c0 [4];
  undefined1 auStack_2a0 [8];
  undefined1 auStack_298 [8];
  undefined1 auStack_290 [8];
  undefined1 local_288 [8];
  undefined **appuStack_280 [40];
  undefined1 auStack_140 [8];
  undefined1 auStack_138 [8];
  undefined **local_130;
  undefined **local_128;
  undefined **local_120;
  void *local_118;
  undefined **local_110 [4];
  undefined ***local_f0;
  undefined8 local_e0;
  long lStack_d8;
  long *local_c0;
  undefined **local_b0;
  undefined **local_a8;
  undefined1 auStack_a0 [8];
  long local_98 [2];
  undefined8 auStack_88 [2];
  undefined8 local_78;
  
  iVar9 = (**(code **)(*param_4 + 0x28))(param_4);
  if (iVar9 == 0x60a6) {
    *(undefined1 *)(param_3 + 0x54) = 1;
    *(undefined4 *)(param_3 + 0x58) = 0xffffffff;
  }
  plVar24 = DAT_01b99950;
  if (((*(int *)(*DAT_01b99950 + 8) != 7 && iVar9 != 0x4e87) && iVar9 != 0x60a6) && iVar9 != 0x4e8c)
  {
    return 1;
  }
  FUN_00cb7060(*DAT_01b99950,iVar9);
  plVar2 = DAT_01b99b08;
  plVar1 = DAT_01b99a98;
  if (iVar9 < 0x61b2) {
    if (iVar9 < 0x5713) {
      if (iVar9 < 0x517f) {
        if (iVar9 < 0x4f70) {
          if (iVar9 < 0x4ee2) {
            if (iVar9 < 0x4e8c) {
              if (iVar9 == 0x467e) {
                lVar14 = FUN_00db8d28();
                if (lVar14 != 0) {
                  FUN_00db8d28();
                  FUN_00db8270();
                }
                goto LAB_00cb2eb0;
              }
              if (iVar9 == 0x4e87) {
                FUN_0163de50(&local_e0,*(undefined8 *)PTR_PTR_01b9aa50);
                FUN_00cc35f4(&local_e0);
                    /* WARNING: Subroutine does not return */
                FUN_0163dd30(&local_e0);
              }
            }
            else {
              if (iVar9 == 0x4e8c) {
                FUN_00cb3144(param_3);
                goto LAB_00cb2eb0;
              }
              if (iVar9 == 0x4ec8) {
                iVar9 = (int)param_4[0x12];
                if (iVar9 < 3) {
                  if (iVar9 == 1) {
                    lVar14 = FUN_009e30c0();
                    pcVar27 = "TID_PLAYER_REPORTED_SUCCESSFULLY";
                  }
                  else {
                    if (iVar9 != 2) goto LAB_00cb2eb0;
                    lVar14 = FUN_009e30c0();
                    pcVar27 = "TID_TOO_MANY_REPORTS_SENT";
                  }
                }
                else {
                  if (iVar9 != 3) {
                    if (iVar9 == 6) {
                      lVar14 = FUN_009e30c0();
                      pcVar27 = "TID_TOO_MANY_ALLIANCE_CHAT_REPORTS_SENT";
                      goto LAB_00cb16a8;
                    }
                    if (iVar9 != 7) goto LAB_00cb2eb0;
                  }
                  lVar14 = FUN_009e30c0();
                  pcVar27 = "TID_PLAYER_ALREADY_REPORTED";
                }
LAB_00cb16a8:
                FUN_0163de50(&local_e0,pcVar27);
                puVar23 = (undefined8 *)FUN_00eae7d8(&local_e0);
                goto LAB_00cb16c4;
              }
            }
          }
          else if (iVar9 < 0x4f42) {
            if (iVar9 == 0x4ee2) goto LAB_00caff30;
            if (iVar9 == 0x4ee4) {
              lVar14 = FUN_00db8d28();
              if (lVar14 != 0) {
                lVar14 = param_4[0x13];
                FUN_00db8d28();
                uVar16 = FUN_00db8270();
                FUN_00ddb20c(uVar16,lVar14);
              }
              goto LAB_00cb2eb0;
            }
          }
          else {
            if (iVar9 == 0x4f42) {
              lVar14 = FUN_009e30c0();
              pcVar27 = "TID_SPECTATE_FAILED_DUE_TO_PRIVACY";
              goto LAB_00cb16a8;
            }
            if (iVar9 == 0x4f45) {
              if (*(long **)(param_3 + 0xe8) != (long *)0x0) {
                (**(code **)(**(long **)(param_3 + 0xe8) + 8))();
              }
              *(undefined8 *)(param_3 + 0xe8) = 0;
              plVar24 = (long *)FUN_00e9833c();
              if (plVar24 != (long *)0x0) {
                lVar14 = FUN_00db3094();
                if (lVar14 == 0) {
                  plVar24 = (long *)FUN_00e9cd50(plVar24);
                  if (plVar24 == (long *)0x0) {
                    *(long **)(param_3 + 0xd8) = param_4;
                    return 0;
                  }
                  (**(code **)(*plVar24 + 0xa70))(plVar24,param_4);
                }
                else {
                  lVar14 = (**(code **)(*plVar24 + 0x88))(plVar24);
                  if (lVar14 != 0) {
                    uVar10 = *(undefined4 *)((long)param_4 + 0xac);
                    uVar17 = *(undefined8 *)(lVar14 + 0xa8);
                    uVar16 = FUN_00e9ddb0(plVar24,0);
                    FUN_00ea07d0(uVar10,uVar17,uVar16);
                  }
                  FUN_00db13a0();
                  uVar16 = FUN_00db3094();
                  FUN_00dac998(uVar16,param_4);
                }
              }
              goto LAB_00cb2eb0;
            }
          }
        }
        else if (iVar9 < 0x50be) {
          if (iVar9 < 0x500f) {
            if (iVar9 == 0x4f70) {
              if (*DAT_01b99a90 != 0) {
                FUN_00eb22a4(*DAT_01b99a90,param_4);
              }
              goto LAB_00cb2eb0;
            }
            if (iVar9 == 0x4ffc) {
              if (*DAT_01b9a928 != 0) {
                FUN_00d6e308(*DAT_01b9a928,param_4);
              }
              goto LAB_00cb2eb0;
            }
          }
          else {
            if (iVar9 == 0x500f) {
              lVar14 = FUN_00e9833c();
              if ((((lVar14 != 0) && (lVar14 = FUN_00e9d260(), lVar14 != 0)) &&
                  (lVar14 = FUN_009e30c0(), lVar14 != 0)) &&
                 (lVar14 = FUN_009e45e0(lVar14,1,1), lVar14 != 0)) {
                FUN_00b99774();
              }
              goto LAB_00cb2eb0;
            }
            if (iVar9 == 0x506f) {
              lVar14 = param_4[0x13];
              plVar24 = (long *)param_4[0x14];
              if ((plVar24 != (long *)0x0) && (0 < *(int *)((long)plVar24 + 0xc))) {
                lVar26 = 0;
                do {
                  uVar16 = FUN_00a049f4();
                  FUN_00a06a8c(uVar16,lVar14,*(undefined8 *)(*plVar24 + lVar26 * 8));
                  lVar26 = lVar26 + 1;
                } while (lVar26 < *(int *)((long)plVar24 + 0xc));
              }
              FUN_00f4c118(&local_e0,param_4);
              if (local_e0 != (undefined **)0x0) {
                uVar16 = FUN_00a049f4();
                local_a8 = local_e0;
                FUN_00a06b04(uVar16,lVar14,&local_a8);
              }
              goto LAB_00cb2eb0;
            }
          }
        }
        else if (iVar9 < 0x511d) {
          if (iVar9 == 0x50be) {
            lVar14 = FUN_00a049f4();
            if ((lVar14 == 0) || (FUN_012a0ef4(&local_e0,param_4), local_e0 == (undefined **)0x0))
            goto LAB_00cb2eb0;
            uVar16 = FUN_00a049f4();
            FUN_00a059ac(uVar16,local_e0,(int)param_4[0x13]);
joined_r0x00cb06a8:
            if (local_e0 != (undefined **)0x0) {
              operator_delete(local_e0);
            }
            goto LAB_00cb2eb0;
          }
          if (iVar9 == 0x50e7) {
            lVar14 = FUN_009e30c0();
            if ((lVar14 != 0) && (lVar14 = FUN_00db8d28(), lVar14 != 0)) {
              lVar26 = FUN_00db8270();
              if (lVar26 != 0) {
                if (*(char *)(lVar26 + 0x3fc) != '\0') {
                  uVar16 = FUN_009e30c0();
                  FUN_0163de50(&local_e0,"TID_2V2_MATCHMAKE_CANCELLED_CO_MATE");
                  uVar17 = FUN_00eae7d8(&local_e0);
                  FUN_009e408c(0,uVar16,uVar17,0xffffffff);
                    /* WARNING: Subroutine does not return */
                  FUN_0163dd30(&local_e0);
                }
                if ((*(long *)(lVar26 + 0x1e8) != 0) &&
                   (uVar25 = FUN_00c5ccb8(*(long *)(lVar26 + 0x1e8)), (uVar25 & 1) != 0)) {
                  uVar16 = FUN_009e30c0();
                  FUN_0163de50(&local_e0,"TID_BATTLE_CANCELLED");
                  uVar17 = FUN_00eae7d8(&local_e0);
                  FUN_009e408c(0,uVar16,uVar17,0xffffffff);
                    /* WARNING: Subroutine does not return */
                  FUN_0163dd30(&local_e0);
                }
                FUN_00dd96b8(lVar26);
                uVar16 = FUN_00ea0658();
                FUN_00ea2354(uVar16,0);
              }
              lVar26 = FUN_00db825c(lVar14);
              if ((lVar26 != 0) && (lVar26 = FUN_00db825c(lVar14), *(long *)(lVar26 + 0x1c8) != 0))
              {
                lVar14 = FUN_00db825c(lVar14);
                *(undefined1 *)(*(long *)(lVar14 + 0x1c8) + 0x158) = 1;
              }
            }
            goto LAB_00cb2eb0;
          }
        }
        else {
          if (iVar9 == 0x511d) {
            lVar14 = FUN_009e30c0();
            if (lVar14 == 0) goto LAB_00cb2eb0;
            iVar9 = (int)param_4[0x12];
            if (iVar9 < 3) {
              if (iVar9 == 1) {
                pcVar27 = "TID_CLAN_JOIN_FAIL_CLOSED";
              }
              else {
                if (iVar9 != 2) goto LAB_00cb1870;
                pcVar27 = "TID_CLAN_JOIN_FAIL_ALREADY_SENT";
              }
            }
            else if (iVar9 == 3) {
              pcVar27 = "TID_CLAN_JOIN_FAIL_NO_SCORE";
            }
            else if (iVar9 == 4) {
              pcVar27 = "TID_CLAN_JOIN_FAILED_BANNED";
            }
            else {
LAB_00cb1870:
              pcVar27 = "TID_CLAN_JOIN_FAIL_GENERIC";
            }
LAB_00cb0780:
            FUN_0163de50(&local_e0,pcVar27);
LAB_00cb0f48:
            uVar16 = FUN_00eae7d8(&local_e0);
            FUN_009e408c(0,lVar14,uVar16,0xffffffff);
            goto LAB_00cb2ea8;
          }
          if (iVar9 == 0x5130) {
            lVar14 = FUN_00a049f4();
            if (lVar14 == 0) goto LAB_00cb2eb0;
            uVar16 = FUN_00a049f4();
            FUN_012a13ac(auStack_140,param_4);
            FUN_00a05250(uVar16,auStack_140);
            FUN_012a13bc(&local_e0,param_4);
            uVar16 = FUN_00a049f4();
            FUN_00a07658(uVar16,local_e0,(int)param_4[0x14]);
            goto joined_r0x00cb06a8;
          }
        }
      }
      else {
        if (0x52ab < iVar9) {
          if (0x5589 < iVar9) {
            if (iVar9 < 0x5647) {
              if (iVar9 == 0x558a) {
LAB_00cafa94:
                if (*DAT_01b99cf8 != 0) {
                  FUN_00905c34(*DAT_01b99cf8,param_4);
                }
                goto LAB_00cb2eb0;
              }
              iVar13 = 0x55aa;
LAB_00caf4f4:
              if (iVar9 == iVar13) goto LAB_00cb2eb0;
              goto LAB_00cb0b58;
            }
            if (iVar9 == 0x5647) goto LAB_00caff30;
            if (iVar9 != 0x5690) goto LAB_00cb0b58;
            if (*(long **)(param_3 + 0xe8) != (long *)0x0) {
              (**(code **)(**(long **)(param_3 + 0xe8) + 8))();
            }
            *(undefined8 *)(param_3 + 0xe8) = 0;
            FUN_012d1ad0(&local_e0,param_4);
            if (local_e0 == (undefined **)0x0) goto LAB_00cb2eb0;
            lVar14 = param_4[0x12];
            pvVar15 = (void *)FUN_012d192c(param_4);
            pvVar21 = operator_new(0x6b0);
            uVar16 = FUN_014bbe6c();
            FUN_015e88d0(pvVar21,uVar16);
            *(void **)(param_3 + 0xe8) = pvVar21;
            pvVar21 = operator_new(0x20);
            uVar16 = FUN_00978b78();
            FUN_01292e44(pvVar21,uVar16,1);
            FUN_015e9620(*(undefined8 *)(param_3 + 0xe8),pvVar21);
            uVar16 = FUN_00978bbc();
            FUN_00979420(uVar16,(long)pvVar21 + 8);
            uVar25 = FUN_015e8bd8(*(undefined8 *)(param_3 + 0xe8),local_e0,(int)lVar14,pvVar15,
                                  DAT_01b9aa80,param_4 + 0x16);
            plVar24 = *(long **)(param_3 + 0xe8);
            if ((uVar25 & 1) == 0) {
              if (plVar24 != (long *)0x0) {
                lVar14 = *plVar24;
LAB_00cb16dc:
                (**(code **)(lVar14 + 8))(plVar24);
              }
LAB_00cb16e4:
              *(undefined8 *)(param_3 + 0xe8) = 0;
            }
            else {
              pvVar21 = operator_new(0x90);
              FUN_012d186c();
              FUN_008c2808();
              FUN_008c2814();
              uVar25 = FUN_015e9688(plVar24,pvVar21);
              if ((uVar25 & 1) == 0) {
                plVar24 = *(long **)(param_3 + 0xe8);
                if (plVar24 != (long *)0x0) {
                  lVar14 = *plVar24;
                  goto LAB_00cb16dc;
                }
                goto LAB_00cb16e4;
              }
              FUN_00978c00();
              lVar14 = FUN_00ff9c28();
              if (lVar14 != 0) {
                FUN_015e9698((float)*(int *)(lVar14 + 0x194),*(undefined8 *)(param_3 + 0xe8));
                FUN_015e96a0((float)*(int *)(lVar14 + 400) * 0.001,*(undefined8 *)(param_3 + 0xe8));
              }
            }
            if (local_e0 != (undefined **)0x0) {
                    /* WARNING: Subroutine does not return */
              FUN_0163dd30(local_e0);
            }
            local_e0 = (undefined **)0x0;
            if (pvVar15 != (void *)0x0) {
              operator_delete__(pvVar15);
            }
            goto LAB_00cb2eb0;
          }
          if (iVar9 < 0x53a3) {
            if (iVar9 != 0x52ac) {
              iVar13 = 0x5355;
              goto LAB_00caf4f4;
            }
          }
          else if (iVar9 != 0x53a3) {
            if (iVar9 == 0x53db) goto LAB_00cafa94;
            goto LAB_00cb0b58;
          }
          goto LAB_00caff30;
        }
        if (iVar9 < 0x51f6) {
          if (iVar9 < 0x51d4) {
            if (iVar9 == 0x517f) {
              lVar14 = FUN_009e30c0();
              if (lVar14 == 0) goto LAB_00cb2eb0;
              iVar9 = (int)param_4[0x12];
              if (iVar9 < 9) {
                if (iVar9 == 3) {
                  uVar16 = FUN_009e30c0();
                  pcVar27 = "TID_TOURNAMENT_MATCHMAKE_FAILED_PROCESSING";
                  goto LAB_00cb1bb0;
                }
                if (iVar9 == 4) {
                  uVar16 = FUN_009e30c0();
                  pcVar27 = "TID_TOURNAMENT_MATCHMAKE_FAILED";
                  goto LAB_00cb1bb0;
                }
                if (iVar9 == 7) {
                  uVar16 = FUN_009e30c0();
                  pcVar27 = "TID_MATCHMAKE_FAILED_WAR_ANOTHER_PLAYER_BATTLING";
                  goto LAB_00cb1bb0;
                }
              }
              else {
                if (iVar9 == 9) {
                  uVar16 = FUN_009e30c0();
                  pcVar27 = "TID_MATCHMAKE_FAILED_WAR_SECTION_ENDED";
                  goto LAB_00cb1bb0;
                }
                if (iVar9 == 10) {
                  plVar24 = (long *)FUN_00e9833c();
                  (**(code **)(*plVar24 + 0x88))();
                  lVar14 = FUN_012d316c();
                  FUN_012621f8(*(undefined8 *)(lVar14 + 0x490));
                  uVar25 = FUN_0125c084();
                  uVar16 = FUN_009e30c0();
                  if ((uVar25 & 1) == 0) {
                    pcVar27 = "TID_MATCHMAKE_FAILED_DUELS_DISABLED";
                  }
                  else {
                    pcVar27 = "TID_SHUTDOWN_BATTLE_DISABLED";
                  }
                  goto LAB_00cb1bb0;
                }
                if (iVar9 == 0xb) {
                  uVar16 = FUN_009e30c0();
                  pcVar27 = "TID_MATCHMAKE_FAILED_MAX_WAR_PARTICIPANTS";
                  goto LAB_00cb1bb0;
                }
              }
              uVar16 = FUN_009e30c0();
              pcVar27 = "TID_MATCHMAKE_FAILED";
LAB_00cb1bb0:
              FUN_0163de50(&local_e0,pcVar27);
              uVar17 = FUN_00eae7d8(&local_e0);
              FUN_009e408c(0,uVar16,uVar17,0xffffffff);
                    /* WARNING: Subroutine does not return */
              FUN_0163dd30(&local_e0);
            }
            if (iVar9 == 0x51d1) {
              lVar14 = FUN_009e30c0();
              if (lVar14 != 0) {
                FUN_0163de50(&local_78,"TID_SPELL_FACTORY_CLAIM_FAILED_TITLE");
                uVar16 = FUN_00eae7d8(&local_78);
                FUN_0163ded0(&local_e0,uVar16);
                    /* WARNING: Subroutine does not return */
                FUN_0163dd30(&local_78);
              }
              goto LAB_00cb2eb0;
            }
          }
          else {
            if (iVar9 == 0x51d4) goto LAB_00caff4c;
            if (iVar9 == 0x51f1) goto LAB_00caf9dc;
          }
        }
        else {
          if (iVar9 < 0x520a) {
            if (iVar9 == 0x51f6) {
              lVar14 = param_4[0x12];
              *DAT_01b999d8 = 0;
              lVar26 = FUN_009e30c0();
              if (lVar26 != 0) {
                FUN_0163da5c(&local_e0);
                if ((int)lVar14 == 1) {
                  pcVar27 = "TID_SHARED_FRIENDLY_CHALLENGE_ERROR_ALREADY_STARTED";
                }
                else if ((int)lVar14 == 2) {
                  pcVar27 = "TID_SHARED_FRIENDLY_CHALLENGE_ERROR_EXPIRED";
                }
                else {
                  pcVar27 = "TID_SHARED_FRIENDLY_CHALLENGE_ERROR_FAILED";
                }
                FUN_0163e214(&local_e0,pcVar27);
                FUN_0163de50(&local_78,"TID_CHALLENGE");
                FUN_00eae7d8(&local_78);
                    /* WARNING: Subroutine does not return */
                FUN_0163dd30(&local_78);
              }
              goto LAB_00cb2eb0;
            }
            iVar13 = 0x51fd;
            goto LAB_00caff28;
          }
          if (iVar9 == 0x520a) {
            lVar14 = FUN_00d69808();
            if (lVar14 != 0) {
              FUN_00d69808();
              uVar25 = FUN_00d6bb70();
              if ((uVar25 & 1) != 0) {
                uVar16 = FUN_00d69808();
                FUN_00d6c784(uVar16,param_4 + 0x12);
              }
            }
            lVar14 = FUN_015d0788();
            if (lVar14 != 0) {
              FUN_015d0b10(lVar14,param_4 + 0x12);
            }
            goto LAB_00cb2eb0;
          }
          if (iVar9 == 0x5237) {
            lVar14 = FUN_009e30c0();
            if (lVar14 == 0) goto LAB_00cb2eb0;
            if ((int)param_4[0x12] == 2) {
              pcVar27 = "TID_FILTERING_SERVICE_OUTAGE";
            }
            else {
              if ((int)param_4[0x12] != 1) goto LAB_00cb2eb0;
              pcVar27 = "TID_YOUNG_PLAYER_CANNOT_SEND_CHAT_MESSAGE";
            }
            goto LAB_00cb16a8;
          }
        }
      }
    }
    else if (iVar9 < 0x5bd3) {
      if (iVar9 < 0x589c) {
        if (iVar9 < 0x57a5) {
          if (iVar9 < 0x573b) {
            if (iVar9 == 0x5713) goto LAB_00caff30;
            iVar13 = 0x5738;
            goto LAB_00caff28;
          }
          if (iVar9 == 0x573b) {
            lVar14 = FUN_00a049f4();
            if (lVar14 != 0) {
              uVar16 = FUN_00a049f4();
              FUN_00a0529c(uVar16,param_4 + 0x12);
            }
            goto LAB_00cb2eb0;
          }
          if (iVar9 == 0x5791) {
            FUN_00d3e6c4();
            goto LAB_00cb2eb0;
          }
        }
        else if (iVar9 < 0x5852) {
          if (iVar9 == 0x57a5) {
            lVar14 = FUN_00a049f4();
            if (lVar14 != 0) {
              FUN_00aa0414(*(undefined8 *)(lVar14 + 0x50),param_4[0x12]);
            }
            goto LAB_00cb2eb0;
          }
          if (iVar9 == 0x57d2) {
            lVar14 = FUN_009e30c0();
            if (lVar14 != 0) {
              lVar14 = FUN_00db8d28();
              if ((lVar14 != 0) && (lVar26 = FUN_00db8270(), lVar26 != 0)) {
                FUN_00db8270(lVar14);
                FUN_00dd96b8();
              }
              lVar14 = FUN_00a049f4();
              if (lVar14 != 0) {
                uVar16 = FUN_00a049f4();
                FUN_00a06378(uVar16,0,0,0);
              }
            }
            goto LAB_00cb2eb0;
          }
        }
        else {
          if (iVar9 == 0x5852) {
            FUN_00d3e44c(param_4);
            goto LAB_00cb2eb0;
          }
          if (iVar9 == 0x5856) {
            lVar14 = FUN_00a049f4();
            if ((lVar14 != 0) && (FUN_012a3f90(&local_e0,param_4), local_e0 != (undefined **)0x0)) {
              lVar14 = FUN_00a049f4();
              thunk_FUN_00a9fe0c(*(undefined8 *)(lVar14 + 0x50),local_e0);
            }
            goto LAB_00cb2eb0;
          }
        }
      }
      else {
        if (0x59de < iVar9) {
          if (iVar9 < 0x5a4c) {
            if (iVar9 == 0x59df) {
              lVar14 = FUN_009e30c0();
              if ((lVar14 == 0) || (lVar14 = FUN_00db3094(), lVar14 == 0)) goto LAB_00cb2eb0;
              FUN_012c30c0(&local_e0,param_4);
              uVar16 = FUN_00db3094();
              FUN_00db2040(uVar16,local_e0);
              goto joined_r0x00cb06a8;
            }
            if (iVar9 != 0x5a43) goto LAB_00cb0b58;
            local_110[0] = &PTR_LAB_01af9f68;
            local_f0 = local_110;
            uVar25 = FUN_00e94bc0(param_4,local_110);
            if (local_f0 == local_110) {
              lVar14 = 4;
              pppuVar31 = local_110;
LAB_00cb0e10:
              (*(code *)(*pppuVar31)[lVar14])(pppuVar31);
            }
            else if (local_f0 != (undefined ***)0x0) {
              lVar14 = 5;
              pppuVar31 = local_f0;
              goto LAB_00cb0e10;
            }
            if ((uVar25 & 1) == 0) {
              pvVar15 = operator_new(0x90);
              FUN_012b22dc();
              local_118 = pvVar15;
              FUN_00cadd44(param_3,&local_118);
            }
            goto LAB_00cb2eb0;
          }
          if (iVar9 == 0x5a4c) {
            if ((int)param_4[0x12] == 3) {
              (**(code **)(*(long *)*DAT_01b99a38 + 0xe8))((long *)*DAT_01b99a38,DAT_01b9aaa0);
            }
            else if ((int)param_4[0x12] == 1) {
              uVar16 = FUN_008c2808();
              FUN_0163da5c(&local_e0);
              FUN_0163da5c(&local_78);
              FUN_0163da5c(auStack_88);
              uVar17 = 0xd;
              goto LAB_00cb0e98;
            }
            goto LAB_00cb2eb0;
          }
          if (iVar9 != 0x5bb2) goto LAB_00cb0b58;
          lVar14 = param_4[0x13];
          lVar26 = param_4[0x14];
          iVar9 = (int)param_4[0x12];
          lVar18 = FUN_00db8d28();
          FUN_01640938(&local_e0,"FriendlyChallengeFailedMessage RECEIVED, reason %d",iVar9);
          if (lVar14 != 0) {
            FUN_0149fd14(auStack_88,lVar14);
            FUN_0163e6c0(&local_78,", senderId ",auStack_88);
            FUN_0163e6d0(&local_e0,&local_78);
                    /* WARNING: Subroutine does not return */
            FUN_0163dd30(&local_78);
          }
          if (lVar26 != 0) {
            FUN_0149fd14(auStack_88,lVar26);
            FUN_0163e6c0(&local_78,", entryId ",auStack_88);
            FUN_0163e6d0(&local_e0,&local_78);
                    /* WARNING: Subroutine does not return */
            FUN_0163dd30(&local_78);
          }
          lVar33 = *DAT_01b99b08;
          lVar14 = FUN_00e87bf0(lVar33);
          lVar26 = FUN_00e87bf8(lVar33);
          if (lVar14 == 0) {
            bVar7 = 0;
LAB_00cb12b4:
            bVar4 = false;
          }
          else {
            lVar19 = FUN_012b0abc(lVar14);
            if (lVar19 == 0) {
              bVar7 = 0;
            }
            else {
              FUN_012b0abc(lVar14);
              bVar7 = FUN_01013144();
            }
            lVar19 = FUN_012b0abc(lVar14);
            if ((lVar19 == 0) || (lVar19 = FUN_012b0abc(lVar14), *(int *)(lVar19 + 0x78) != 1))
            goto LAB_00cb12b4;
            lVar19 = FUN_012b0ac4(lVar14);
            bVar4 = lVar19 != 0;
          }
          if ((lVar26 != 0) && (bVar4 == false)) {
            lVar19 = FUN_012b0ac4(lVar26);
            bVar4 = lVar19 != 0;
          }
          if ((lVar26 != 0) && (((bVar7 ^ 1) & 1) != 0)) {
            lVar19 = FUN_012b0abc(lVar26);
            if (lVar19 == 0) {
              bVar7 = 0;
            }
            else {
              FUN_012b0abc(lVar26);
              bVar7 = FUN_01013144();
            }
          }
          if ((lVar18 != 0) && ((bVar7 & 1) == 0)) {
            lVar26 = FUN_00db8270(lVar18);
            if ((lVar26 == 0) || (uVar25 = FUN_00dbd5e0(lVar18), (uVar25 & 1) == 0)) {
              bVar7 = 0;
            }
            else {
              FUN_00db8270(lVar18);
              lVar26 = FUN_00db8270(lVar18);
              bVar7 = FUN_0149fbe0(lVar26 + 0x2e8,0);
            }
          }
          if ((lVar14 == 0) || (*(char *)(lVar33 + 0xa6) == '\0')) {
LAB_00cb1570:
            bVar32 = 0;
          }
          else {
            uVar16 = FUN_012af3a8(lVar14);
            uVar25 = FUN_0149fbe0(uVar16,0);
            if ((uVar25 & 1) == 0) goto LAB_00cb1570;
            bVar32 = (bVar4 | bVar7) ^ 1;
          }
          if (iVar9 == 3) {
            bVar8 = FUN_00e87d10(lVar33,0,0);
            bVar8 = bVar7 | bVar8;
          }
          else {
            if (iVar9 == 4) {
              FUN_00e87d10(lVar33,0,0);
            }
            bVar8 = 1;
          }
          lVar14 = FUN_009e30c0();
          if (((bVar8 & 1) != 0) && (lVar14 != 0)) {
            if (lVar18 == 0) {
              lVar14 = 0;
            }
            else {
              lVar14 = FUN_00db8270(lVar18);
              FUN_00dbe220(lVar18);
            }
            if (iVar9 == 1) {
              if (lVar14 != 0 && ((bVar7 ^ 0xff) & 1) == 0) {
                FUN_00ddaf84(lVar14);
              }
            }
            else if (iVar9 == 3) {
              if (((lVar14 != 0 && ((bVar7 ^ 0xff) & 1) == 0) &&
                  (uVar25 = FUN_00dda4dc(lVar14), (uVar25 & 1) != 0)) &&
                 (uVar25 = FUN_00dda4e4(lVar14,0), (uVar25 & 1) != 0)) {
                FUN_00ddaf84(lVar14);
              }
              if ((bVar32 & 1) != 0) {
                uVar16 = FUN_009e30c0();
                FUN_0163de50(&local_78,"TID_FRIENDLY_CHALLENGE_CANCELED");
                uVar17 = FUN_00eae7d8(&local_78);
                FUN_009e408c(0,uVar16,uVar17,0xffffffff);
                    /* WARNING: Subroutine does not return */
                FUN_0163dd30(&local_78);
              }
            }
            else {
              if (iVar9 == 6) {
                uVar16 = FUN_009e30c0();
                FUN_0163de50(&local_78,"TID_BATTLE_FRIEND_IN_BATTLE");
                uVar17 = FUN_00eae7d8(&local_78);
                FUN_009e408c(0,uVar16,uVar17,0xffffffff);
                    /* WARNING: Subroutine does not return */
                FUN_0163dd30(&local_78);
              }
              if (iVar9 == 2) {
                if (lVar14 != 0 && ((bVar7 ^ 0xff) & 1) == 0) {
                  FUN_00ddb168(lVar14);
                }
              }
              else if (lVar14 != 0 && ((bVar7 ^ 0xff) & 1) == 0) {
                FUN_00ddb178(lVar14,0,0);
              }
            }
            lVar14 = FUN_00db8d28();
            if (lVar14 != 0) {
              FUN_00db8d28();
              lVar14 = FUN_00db825c();
              if (lVar14 != 0) {
                FUN_00db8d28();
                lVar14 = FUN_00db825c();
                if (*(long *)(lVar14 + 0x1c8) != 0) {
                  uVar16 = FUN_009e30c0();
                  FUN_0163de50(&local_78,"TID_CLAN_WAR_MATCHMAKING_FAILED_GENERIC");
                  uVar17 = FUN_00eae7d8(&local_78);
                  FUN_009e408c(0,uVar16,uVar17,0xffffffff);
                    /* WARNING: Subroutine does not return */
                  FUN_0163dd30(&local_78);
                }
              }
            }
          }
LAB_00cb19e0:
                    /* WARNING: Subroutine does not return */
          FUN_0163dd30(&local_e0);
        }
        if (iVar9 < 0x595f) {
          if (iVar9 == 0x589c) {
            lVar14 = FUN_00e9833c();
            if ((lVar14 != 0) && (lVar14 = FUN_009e30c0(), lVar14 != 0)) {
              FUN_00e9833c();
              lVar14 = FUN_00e9cd3c();
              if (lVar14 == 0) {
                FUN_00e9833c();
                lVar14 = FUN_00e9cd28();
                if (lVar14 != 0) {
                  FUN_00e9833c();
                  FUN_00e9cd28();
                  lVar14 = FUN_00db8270();
                  if (lVar14 != 0) {
                    FUN_00e9833c();
                    FUN_00e9cd28();
                    FUN_00db8270();
                    uVar25 = FUN_00dd9834();
                    if ((uVar25 & 1) != 0) goto LAB_00cb2eb0;
                  }
                }
                FUN_0163de50(&local_e0,"TID_PUSH_MATCHMAKING_TOP");
                FUN_00eae7d8(&local_e0);
                    /* WARNING: Subroutine does not return */
                FUN_0163dd30(&local_e0);
              }
            }
            goto LAB_00cb2eb0;
          }
          iVar13 = 0x58e7;
          goto LAB_00caff28;
        }
        if (iVar9 == 0x595f) {
          iVar9 = FUN_0129ceb8(param_4);
          uVar16 = _DAT_01c4b578;
          uVar17 = FUN_0129ceb0(param_4);
          FUN_00da98c8(uVar16,uVar17,iVar9 == 0,iVar9 == 2);
          goto LAB_00cb2eb0;
        }
        if (iVar9 == 0x59a5) {
          uVar16 = FUN_009e30c0();
          uVar10 = FUN_009e401c();
          local_e0 = (undefined **)CONCAT44(param_2,uVar10);
          FUN_009e411c(0,uVar16,&local_e0,param_4 + 0x13,0xffffffff,0);
          goto LAB_00cb2eb0;
        }
      }
    }
    else {
      if (iVar9 < 0x5fda) {
        if (iVar9 < 0x5d8c) {
          if (iVar9 < 0x5c87) {
            if (iVar9 == 0x5bd3) {
              FUN_012b2298(&local_78,param_4);
              if (CONCAT44(local_78._4_4_,(undefined4)local_78) == 0) goto LAB_00cb2eb0;
              cVar6 = FUN_00f2d2e8();
              if (cVar6 == '\x04') {
                iVar9 = FUN_00f2d524(CONCAT44(local_78._4_4_,(undefined4)local_78));
                if (1 < iVar9) {
                  uVar10 = FUN_00f2d530(CONCAT44(local_78._4_4_,(undefined4)local_78),0);
                  uVar11 = FUN_00f2d530(CONCAT44(local_78._4_4_,(undefined4)local_78),1);
                  FUN_00db2fa0(uVar10,uVar11);
                }
              }
              else {
                lVar14 = FUN_00db3094();
                if (lVar14 == 0) {
                  lVar14 = FUN_00e9833c();
                  if ((lVar14 == 0) || (plVar24 = (long *)FUN_00e9cd50(), plVar24 == (long *)0x0)) {
                    lVar14 = FUN_00ea0658();
                    if (lVar14 == 0) {
                      lVar14 = -0x100000000;
                    }
                    else {
                      lVar14 = FUN_00ea2134();
                      lVar14 = lVar14 << 0x20;
                    }
                    lVar26 = FUN_00e9833c();
                    if (lVar26 == 0) {
                      local_e0 = &PTR_LAB_01af9f20;
                      lStack_d8 = lVar14;
                      local_c0 = &local_e0;
                      FUN_0163bd84(1,100,&local_e0);
                      if (local_c0 == &local_e0) {
                        lVar14 = 4;
                        plVar24 = &local_e0;
                      }
                      else {
                        if (local_c0 == (long *)0x0) goto LAB_00cb2f7c;
                        lVar14 = 5;
                        plVar24 = local_c0;
                      }
                      (**(code **)(*plVar24 + lVar14 * 8))();
                    }
                  }
                  else {
                    (**(code **)(*plVar24 + 0xa80))
                              (plVar24,CONCAT44(local_78._4_4_,(undefined4)local_78),0);
                  }
                }
                else {
                  uVar25 = FUN_00dac1fc();
                  if ((uVar25 & 1) == 0) {
                    FUN_00db3adc(lVar14,CONCAT44(local_78._4_4_,(undefined4)local_78),0);
                  }
                }
              }
LAB_00cb2f7c:
              if (CONCAT44(local_78._4_4_,(undefined4)local_78) != 0) {
                FUN_00f2cff4();
                if (CONCAT44(local_78._4_4_,(undefined4)local_78) == 0) goto LAB_00cb2eb0;
                ppuVar22 = (undefined **)(CONCAT44(local_78._4_4_,(undefined4)local_78) + 0x10);
                goto LAB_00cb0c10;
              }
              goto LAB_00cb2eb0;
            }
            if (iVar9 == 0x5c44) {
              lVar14 = FUN_009e30c0();
              if (lVar14 == 0) goto LAB_00cb2eb0;
              FUN_0163da5c(&local_e0);
              uVar12 = (int)param_4[0x12] - 1;
              if (uVar12 < 9) {
                pcVar27 = &DAT_0046065c + *(int *)(&DAT_0046065c + (ulong)uVar12 * 4);
              }
              else {
                pcVar27 = "TID_CLAN_JOIN_FAILED";
              }
              FUN_0163e214(&local_e0,pcVar27);
              goto LAB_00cb0f48;
            }
          }
          else {
            if (iVar9 == 0x5c87) {
              *DAT_01b999d8 = 0;
              goto LAB_00cb2eb0;
            }
            if (iVar9 == 0x5d36) {
              lVar14 = FUN_00db8d28();
              if (lVar14 != 0) {
                lVar18 = FUN_00db8270();
                lVar26 = _DAT_01c4b558;
                if ((((_DAT_01c4b558 != 0) &&
                     (uVar25 = FUN_00e8cf38(_DAT_01c4b558), (uVar25 & 1) != 0)) &&
                    (uVar25 = FUN_00e8cab8(lVar26), (uVar25 & 1) == 0)) &&
                   (uVar25 = FUN_00dd9834(lVar18), (uVar25 & 1) == 0)) {
                  FUN_00dda550(lVar18);
                }
                if ((lVar18 != 0) && (uVar25 = FUN_00dd9834(lVar18), (uVar25 & 1) != 0)) {
                  FUN_00dda538(lVar18,(int)param_4[0x12]);
                }
                lVar26 = FUN_00db825c(lVar14);
                if ((lVar26 != 0) && (lVar26 = FUN_00db825c(lVar14), *(long *)(lVar26 + 0x1c8) != 0)
                   ) {
                  lVar14 = FUN_00db825c(lVar14);
                  FUN_00abca5c(*(undefined8 *)(lVar14 + 0x1c8),(int)param_4[0x12]);
                }
              }
              goto LAB_00cb2eb0;
            }
          }
          goto LAB_00cb0b58;
        }
        if (iVar9 < 0x5ef1) {
          if (iVar9 == 0x5d8c) goto LAB_00cafa94;
          iVar13 = 0x5dd9;
          goto LAB_00caf4f4;
        }
        if (iVar9 == 0x5ef1) {
          lVar14 = *DAT_01b99a98;
          if (lVar14 != 0) {
            FUN_00e89a44(lVar14);
            FUN_00e89ae4(lVar14);
          }
          goto LAB_00cb2eb0;
        }
        iVar13 = 0x5fc7;
      }
      else {
        if (iVar9 < 0x60a6) {
          if (0x6049 < iVar9) {
            if (iVar9 == 0x604a) goto LAB_00cafefc;
            if (iVar9 != 0x606c) goto LAB_00cb0b58;
            FUN_0163ded0(&local_e0,param_4 + 0x12);
            uVar25 = FUN_016404cc(&local_e0,&DAT_0043fe1e);
            if ((uVar25 & 1) != 0) {
              uVar16 = FUN_00eae7d8(&local_e0);
              FUN_0163e334(&local_e0,uVar16);
            }
            if (local_e0._4_4_ == 0) goto LAB_00cb19e0;
            lVar14 = FUN_009e30c0();
            puVar23 = &local_e0;
LAB_00cb16c4:
            FUN_009e408c(0,lVar14,puVar23,0xffffffff);
            goto LAB_00cb19e0;
          }
          if (iVar9 != 0x5fda) {
            if (iVar9 == 0x5ff4) {
              FUN_00ea0658();
              FUN_00ea2380();
              lVar14 = FUN_009e30c0();
              if (lVar14 == 0) goto LAB_00cb2eb0;
              uVar16 = FUN_009e30c0();
              local_78._0_4_ = FUN_009e401c();
              local_78._4_4_ = param_2;
              FUN_0163de50(&local_e0,"TID_SHUTDOWN_STARTED");
              uVar17 = FUN_00eae7d8(&local_e0);
              FUN_009e411c(0,uVar16,&local_78,uVar17,0xffffffff,0);
              goto LAB_00cb19e0;
            }
            goto LAB_00cb0b58;
          }
          FUN_0129f1d4(&local_e0,param_4);
          FUN_0129f1c4(&local_78,param_4);
          lVar14 = FUN_00e9833c();
          if (lVar14 == 0) {
LAB_00cb0bd8:
            if ((local_e0 != (undefined **)0x0) && (FUN_00cb31e0(), local_e0 != (undefined **)0x0))
            {
              if (*local_e0 != (undefined *)0x0) {
                operator_delete__(*local_e0);
              }
              operator_delete(local_e0);
            }
            local_e0 = (undefined **)0x0;
          }
          else {
            FUN_00e9833c();
            lVar14 = FUN_00e9cd28();
            if (lVar14 == 0) goto LAB_00cb0bd8;
            FUN_00e9833c();
            FUN_00e9cd28();
            uVar16 = FUN_00db825c();
            local_128 = local_e0;
            FUN_00e40118(uVar16,&local_128,CONCAT44(local_78._4_4_,(undefined4)local_78));
          }
          ppuVar22 = (undefined **)CONCAT44(local_78._4_4_,(undefined4)local_78);
          goto joined_r0x00cb174c;
        }
        if (0x613c < iVar9) {
          if (iVar9 == 0x613d) {
            FUN_012c05f0(local_98,param_4);
            lVar14 = local_98[0];
            uVar16 = FUN_008c2808();
            if (lVar14 != 0) {
              plVar24 = (long *)(local_98[0] + 8);
              if (7 < *(int *)(local_98[0] + 4)) {
                plVar24 = (long *)*(long *)(local_98[0] + 8);
              }
              FUN_0163de50(&local_e0,plVar24);
              FUN_0163da5c(&local_78);
              FUN_0163da5c(auStack_88);
              FUN_008c79d8(uVar16,10,0,&local_e0,&local_78,auStack_88);
                    /* WARNING: Subroutine does not return */
              FUN_0163dd30(auStack_88);
            }
            FUN_0163de50(&local_e0,"ServerError");
            FUN_0163da5c(&local_78);
            FUN_0163da5c(auStack_88);
            uVar17 = 10;
LAB_00cb0e98:
            FUN_008c79d8(uVar16,uVar17,0,&local_e0,&local_78,auStack_88);
                    /* WARNING: Subroutine does not return */
            FUN_0163dd30(auStack_88);
          }
          if (iVar9 == 0x6162) {
            FUN_00bc58e4();
            uVar16 = FUN_008c2808();
            FUN_0163da5c(&local_e0);
            FUN_0163da5c(&local_78);
            FUN_0163da5c(auStack_88);
            uVar10 = 1;
            goto LAB_00cb0b44;
          }
          if (iVar9 == 24999) {
            FUN_00e87ff4(*DAT_01b99b08,param_4);
            lVar14 = FUN_00a049f4();
            if (lVar14 != 0) {
              lVar26 = param_4[0x13];
              lVar14 = param_4[0x12];
              uVar16 = FUN_00a049f4();
              FUN_00a053f4(uVar16,lVar26,(int)lVar14,1);
            }
            goto LAB_00cb2eb0;
          }
          goto LAB_00cb0b58;
        }
        if (iVar9 == 0x60a6) {
          FUN_0163de50(&local_e0,*(undefined8 *)PTR_PTR_01b9aa50);
          FUN_00cc35f4(&local_e0);
                    /* WARNING: Subroutine does not return */
          FUN_0163dd30(&local_e0);
        }
        iVar13 = 0x612e;
      }
LAB_00caff28:
      if (iVar9 == iVar13) {
LAB_00caff30:
        if (*DAT_01b99a90 != 0) {
          FUN_00eb0d24(*DAT_01b99a90,param_4);
        }
        goto LAB_00cb2eb0;
      }
    }
  }
  else {
    if (iVar9 < 0x6b30) {
      if (iVar9 < 0x6530) {
        if (iVar9 < 0x6415) {
          if (iVar9 < 0x630b) {
            if (0x62c9 < iVar9) {
              if (iVar9 == 0x62ca) {
                iVar9 = (int)param_4[0x14];
                if (iVar9 < 4) {
                  if (iVar9 == 0) {
                    lVar14 = *DAT_01b99b08;
                    if (lVar14 != 0) {
                      FUN_012b5714(auStack_298,param_4);
                      FUN_00e85d14(lVar14,auStack_298);
                    }
                  }
                  else {
                    if (iVar9 != 2) goto LAB_00cb2eb0;
                    lVar14 = *DAT_01b99a98;
                    if (lVar14 != 0) {
                      FUN_012b5714(auStack_290,param_4);
                      FUN_00e89d64(lVar14,auStack_290);
                    }
                  }
                }
                else if (iVar9 == 4) {
                  FUN_01641eac("FriendListMessage with clan list type received!\n");
                }
                else {
                  if (iVar9 != 5) goto LAB_00cb2eb0;
                  if (*DAT_01b99a38 != 0) {
                    FUN_00da0b60(*DAT_01b99a38,param_4);
                  }
                }
                FUN_00e87a88(*DAT_01b99b08);
                goto LAB_00cb2eb0;
              }
              iVar13 = 0x62db;
              goto LAB_00caf4f4;
            }
            if (iVar9 == 0x61b2) {
LAB_00caf590:
              FUN_00d3e154();
              goto LAB_00cb2eb0;
            }
            if (iVar9 == 0x628b) {
              if (*DAT_01b99aa0 != 0) {
                lVar14 = FUN_009e30c0();
                plVar24 = DAT_01b99a38;
                if ((lVar14 != 0) &&
                   ((lVar14 = param_4[0x17], *(char *)(*DAT_01b99a38 + 0x40) == '\0' ||
                    ((uVar25 = FUN_00da0084(), (uVar25 & 1) == 0 &&
                     (uVar25 = FUN_016bf8f0(*plVar24), (uVar25 & 1) == 0)))))) {
                  if (((char)param_4[0x16] != '\0') && (*(char *)(*plVar24 + 0x40) != '\0'))
                  goto LAB_00caf2b8;
                  if (param_4[0x14] != 0) {
                    iVar9 = FUN_00f03fd8(lVar14);
                    FUN_00978c00();
                    lVar14 = FUN_00ff9c28();
                    if (*(int *)(lVar14 + 0x2a4) <= iVar9) {
                      FUN_0163de50(&local_e0,"TID_GOOGLE_SERVICE_BOUND_TO_OTHER_ACCOUNT_TITLE");
                      FUN_00eae7d8(&local_e0);
                    /* WARNING: Subroutine does not return */
                      FUN_0163dd30(&local_e0);
                    }
                  }
                }
              }
              goto LAB_00cb2eb0;
            }
          }
          else if (iVar9 < 0x634e) {
            if (iVar9 == 0x630b) {
              if ((int)param_4[0x13] == 1) {
                uVar16 = FUN_00a049f4();
                FUN_012a18d8(auStack_138,param_4);
                FUN_00a062dc(uVar16,auStack_138);
                goto LAB_00cb2eb0;
              }
              uVar16 = FUN_0163de50(&local_e0,"TID_INVITE_CLAN_TOKEN_ERROR");
              FUN_00cb3250(uVar16,&local_e0);
              goto LAB_00cb2ea8;
            }
            if (iVar9 == 0x6330) {
              FUN_012bc518(&local_e0,param_4);
              if (local_e0 != (undefined **)0x0) {
                local_2c0[0] = local_e0;
                FUN_00cb35a0(param_3,local_2c0);
              }
              goto LAB_00cb2eb0;
            }
          }
          else {
            if (iVar9 == 0x634e) {
              lVar14 = FUN_00e9833c();
              if (lVar14 != 0) {
                uVar16 = FUN_00e9cd64(lVar14,param_4);
                return uVar16;
              }
              goto LAB_00cb2eb0;
            }
            if (iVar9 == 0x6403) goto LAB_00caf590;
          }
        }
        else if (iVar9 < 0x64ed) {
          if (iVar9 < 0x6485) {
            if (iVar9 == 0x6415) {
              lVar14 = FUN_00a049f4();
              if ((lVar14 == 0) || (FUN_0129fa24(&local_e0,param_4), local_e0 == (undefined **)0x0))
              goto LAB_00cb2eb0;
              uVar16 = FUN_00a049f4();
              FUN_00a057f8(uVar16,local_e0);
              if (local_e0 == (undefined **)0x0) goto LAB_00cb2eb0;
              FUN_0129f6cc();
joined_r0x00cb00d0:
              if (local_e0 != (undefined **)0x0) {
                (**(code **)(*local_e0 + 8))();
              }
              goto LAB_00cb2eb0;
            }
            if (iVar9 == 0x647f) {
              if (*DAT_01b9aa60 != '\0') {
                FUN_008c2808();
                uVar16 = FUN_008c2cec();
                FUN_01640938(&local_e0,*(undefined8 *)PTR_PTR_01b9aa68,
                             *(undefined8 *)PTR_PTR_01b9aa70);
                FUN_0163de50(&local_78,*(undefined8 *)PTR_PTR_01b9aa78);
                FUN_00cc347c(uVar16,&local_e0,&local_78);
                    /* WARNING: Subroutine does not return */
                FUN_0163dd30(&local_78);
              }
              FUN_012bffdc(&local_e0,param_4);
              FUN_012bffec(&local_78,param_4);
              if ((local_e0 != (undefined **)0x0) && (lVar14 = FUN_011fae98(), lVar14 != 0)) {
                lVar14 = FUN_011fae98(local_e0);
                *(undefined1 *)(lVar14 + 9) = *(undefined1 *)(DAT_01b99a70 + 0xe);
              }
              if (CONCAT44(local_78._4_4_,(undefined4)local_78) != 0) {
                lVar14 = FUN_008c2808();
                uVar17 = *(undefined8 *)(lVar14 + 0x610);
                uVar16 = FUN_00effe24(CONCAT44(local_78._4_4_,(undefined4)local_78));
                FUN_00ef427c(uVar17,uVar16);
              }
              FUN_00ddabc0((int)param_4[0x16]);
              lVar14 = FUN_00ea0658();
              *(undefined4 *)(lVar14 + 8) = 3;
              uVar16 = FUN_00ea0658();
              lVar14 = param_4[0x15];
              iVar9 = *(int *)((long)param_4 + 0xac);
              uVar10 = *(undefined4 *)((long)param_4 + 0xb4);
              lVar26 = param_4[0x17];
              FUN_00ddab98((int)lVar26 - iVar9);
              appuStack_280[0] = local_e0;
              FUN_00ea21ec(uVar16,appuStack_280,local_288,0,1,(int)lVar14,iVar9,uVar10,(int)lVar26);
              goto LAB_00cb2eb0;
            }
          }
          else {
            if (iVar9 == 0x6485) {
LAB_00caf9dc:
              if (*DAT_01b9a290 != 0) {
                FUN_00cba684(*DAT_01b9a290,param_4);
              }
              goto LAB_00cb2eb0;
            }
            if (iVar9 == 0x64bb) {
              FUN_00d3e988(param_4);
              goto LAB_00cb2eb0;
            }
          }
        }
        else if (iVar9 < 0x6517) {
          if (iVar9 == 0x64ed) {
            FUN_00f4c2e4(&local_e0,param_4);
            uVar16 = FUN_00a049f4();
            local_b0 = local_e0;
            FUN_00a075a0(uVar16,&local_b0);
            goto LAB_00cb2eb0;
          }
          if (iVar9 == 0x64f3) {
            iVar9 = (int)param_4[0x12];
            lVar26 = param_4[0x13];
            lVar14 = FUN_00db8d28();
            FUN_01640938(&local_e0,"HostedMatchChallengeFailedMessage RECEIVED, reason %d",iVar9);
            if (lVar26 != 0) {
              FUN_0149fd14(auStack_88,lVar26);
              FUN_0163e6c0(&local_78,", matchId ",auStack_88);
              FUN_0163e6d0(&local_e0,&local_78);
                    /* WARNING: Subroutine does not return */
              FUN_0163dd30(&local_78);
            }
            pcVar27 = (char *)*DAT_01b99b10;
            lVar26 = FUN_00b0b388(pcVar27);
            if (lVar26 == 0) {
              bVar4 = false;
            }
            else {
              bVar4 = *pcVar27 != '\0';
            }
            uVar25 = FUN_00b0b3f8(pcVar27,0);
            lVar26 = FUN_009e30c0();
            if ((((lVar14 != 0) && (lVar26 != 0)) && (bVar4)) &&
               (((uVar25 & 1) != 0 && (lVar26 = FUN_00db8270(lVar14), lVar26 != 0)))) {
              FUN_00db8270(lVar14);
              if (iVar9 == 4) {
                FUN_00ddb0ac();
              }
              else if (iVar9 == 3) {
                FUN_00ddb050();
              }
              else if (iVar9 == 1) {
                FUN_00ddaff4();
              }
              else {
                FUN_00ddb108();
              }
            }
            goto LAB_00cb19e0;
          }
        }
        else {
          if (iVar9 == 0x6517) {
            lVar14 = FUN_009e30c0();
            if (lVar14 == 0) goto LAB_00cb2eb0;
            lVar14 = FUN_009e30c0();
            pcVar27 = "TID_CHALLENGE_FAIL_SERVER";
            goto LAB_00cb16a8;
          }
          if (iVar9 == 0x6524) {
            iVar9 = FUN_014a5314(param_4);
            if (iVar9 == 2) {
              FUN_00cb6748(*plVar24,0xffffffff,1);
            }
            goto LAB_00cb2eb0;
          }
        }
      }
      else if (iVar9 < 0x6816) {
        if (iVar9 < 0x6668) {
          if (iVar9 < 0x660d) {
            if (iVar9 == 0x6530) goto LAB_00cafa94;
            if (iVar9 == 0x6585) {
              uVar16 = FUN_009e30c0();
              lVar14 = FUN_009e45e0(uVar16,0x5c,1);
              if (lVar14 != 0) {
                FUN_00c057b0(lVar14,param_4);
              }
              goto LAB_00cb2eb0;
            }
          }
          else {
            if (iVar9 == 0x660d) {
LAB_00caff4c:
              FUN_009072f0(*DAT_01b99ac0,param_4 + 0x12,param_4 + 0x14);
              goto LAB_00cb2eb0;
            }
            if (iVar9 == 0x660e) {
              iVar9 = (int)param_4[0x12];
              if (iVar9 == 8) {
                pcVar27 = "TID_ADD_FRIEND_FAILED_THEIR_LIST_FULL";
              }
              else if (iVar9 == 7) {
                pcVar27 = "TID_ADD_FRIEND_FAILED_YOUR_LIST_FULL";
              }
              else if (iVar9 == 3) {
                pcVar27 = "TID_ADD_FRIEND_FAILED_BANNED";
              }
              else {
                pcVar27 = "TID_ADD_FRIEND_FAILED";
              }
              uVar16 = FUN_0163de50(&local_e0,pcVar27);
              FUN_00cb3250(uVar16,&local_e0);
                    /* WARNING: Subroutine does not return */
              FUN_0163dd30(&local_e0);
            }
          }
        }
        else if (iVar9 < 0x671d) {
          if (iVar9 == 0x6668) {
            lVar14 = FUN_009e30c0();
            if (lVar14 == 0) goto LAB_00cb2eb0;
            iVar9 = (int)param_4[0x12];
            if (iVar9 < 5) {
              if (iVar9 == 2) {
                pcVar27 = "TID_ALLIANCE_INVITATION_FAILED_NO_RIGHTS";
              }
              else {
                if (iVar9 != 4) goto LAB_00cb2eb0;
                pcVar27 = "TID_ALLIANCE_INVITATION_FAILED_ALREADY_IN_ALLIANCE";
              }
            }
            else if (iVar9 == 5) {
              pcVar27 = "TID_ALLIANCE_INVITATION_FAILED_AVATAR_ALREADY_HAS_AN_INVITE";
            }
            else {
              if (iVar9 != 6) goto LAB_00cb2eb0;
              pcVar27 = "TID_ALLIANCE_INVITATION_FAILED_AVATAR_HAS_TOO_MANY_INVITES";
            }
            goto LAB_00cb0780;
          }
          if (iVar9 == 0x6690) {
            lVar14 = FUN_009e30c0();
            if ((lVar14 != 0) && (lVar14 = FUN_009e45e0(lVar14,0xb,1), lVar14 != 0)) {
              FUN_00bee174(lVar14,param_4 + 0x12,(int)param_4[0x14]);
            }
            goto LAB_00cb2eb0;
          }
        }
        else {
          if (iVar9 == 0x671d) {
            lVar14 = FUN_00a049f4();
            if ((lVar14 == 0) || (FUN_0129fba0(&local_e0,param_4), local_e0 == (undefined **)0x0))
            goto LAB_00cb2eb0;
            uVar16 = FUN_00a049f4();
            FUN_00a05630(uVar16,local_e0);
            goto joined_r0x00cb06a8;
          }
          if (iVar9 == 0x67e4) {
            lVar14 = FUN_00ea0658();
            if (lVar14 != 0) {
              lVar14 = param_4[0x12];
              uVar16 = FUN_00ea0658();
              if ((int)lVar14 < 1) {
                FUN_0163de50(&local_e0,&DAT_003ef238);
                FUN_00ea2570(uVar16,0,0,&local_e0);
                    /* WARNING: Subroutine does not return */
                FUN_0163dd30(&local_e0);
              }
              FUN_00ea2570(uVar16,(int)param_4[0x12] + 5,*(undefined4 *)((long)param_4 + 0x94),
                           param_4 + 0x13);
              uVar16 = FUN_00ea0658();
              FUN_00ea25a0(uVar16,1,1);
              FUN_00ea0658();
              uVar25 = FUN_00ea274c();
              if (((uVar25 & 1) != 0) && (lVar14 = FUN_00a049f4(), lVar14 != 0)) {
                lVar14 = FUN_00a049f4();
                FUN_00aa0600(*(undefined8 *)(lVar14 + 0x50));
              }
              lVar14 = FUN_008c2808();
              uVar16 = *(undefined8 *)(lVar14 + 0x610);
              FUN_00ea0658();
              uVar12 = FUN_00ea2730();
              FUN_00ef4274(uVar16,uVar12 & 1);
            }
            goto LAB_00cb2eb0;
          }
        }
      }
      else if (iVar9 < 0x68d7) {
        if (iVar9 < 0x6885) {
          if (iVar9 == 0x6816) {
            if ((int)param_4[0x16] == 4) {
              lVar14 = FUN_00a049f4();
              if (lVar14 != 0) {
                uVar16 = FUN_00a049f4();
                FUN_00a05348(uVar16,param_4 + 0x14,param_4 + 0x12);
              }
            }
            else {
              FUN_00e8829c(*DAT_01b99b08,param_4);
            }
            goto LAB_00cb2eb0;
          }
          if (iVar9 == 0x6876) {
            lVar14 = FUN_00db3094();
            if (lVar14 != 0) {
              FUN_00db3094();
              lVar14 = FUN_00db1318();
              if (lVar14 != 0) {
                FUN_00db3094();
                uVar16 = FUN_00db1318();
                FUN_00a1d074(uVar16,param_4 + 0x12,(char)param_4[0x13]);
              }
            }
            goto LAB_00cb2eb0;
          }
        }
        else {
          if (iVar9 == 0x6885) goto LAB_00cb2eb0;
          if (iVar9 == 0x689c) {
            FUN_012a1fec(&local_e0,param_4);
            lVar14 = FUN_00e9833c();
            if (lVar14 != 0) {
              FUN_00e9833c();
              lVar14 = FUN_00e9cd28();
              if (lVar14 != 0) {
                FUN_00e9833c();
                FUN_00e9cd28();
                uVar16 = FUN_00db825c();
                local_120 = local_e0;
                FUN_00e3fff0(uVar16,&local_120);
                goto LAB_00cb2eb0;
              }
            }
            if (local_e0 != (undefined **)0x0) {
              FUN_00cb31e0();
              ppuVar22 = local_e0;
              if (local_e0 != (undefined **)0x0) {
                if (*local_e0 != (undefined *)0x0) {
                  operator_delete__(*local_e0);
                }
                operator_delete(ppuVar22);
              }
            }
            goto LAB_00cb2eb0;
          }
        }
      }
      else if (iVar9 < 0x6a8b) {
        if (iVar9 == 0x68d7) {
          uVar16 = FUN_00ea0658();
          FUN_00ea2354(uVar16,1);
          goto LAB_00cb2eb0;
        }
        if (iVar9 == 0x696f) {
          if ((long *)*DAT_01b99a38 != (long *)0x0) {
            (**(code **)(*(long *)*DAT_01b99a38 + 0xd0))();
          }
          goto LAB_00cb2eb0;
        }
      }
      else {
        if (iVar9 == 0x6a8b) {
          lVar14 = FUN_009e30c0();
          if (lVar14 == 0) goto LAB_00cb2eb0;
          lVar14 = FUN_009e30c0();
          pcVar27 = "TID_ALLIANCE_INVITATION_OK";
          goto LAB_00cb0780;
        }
        if (iVar9 == 0x6b1d) {
          FUN_0163ded0(&local_e0,param_4 + 0x12);
          uVar25 = FUN_016404cc(&local_e0,&DAT_0043fe1e);
          if ((uVar25 & 1) != 0) {
            uVar16 = FUN_00eae7d8(&local_e0);
            FUN_0163e334(&local_e0,uVar16);
          }
          FUN_0163ded0(&local_78,param_4 + 0x14);
          uVar25 = FUN_016404cc(&local_78,&DAT_0043fe1e);
          if ((uVar25 & 1) != 0) {
            uVar16 = FUN_00eae7d8(&local_78);
            FUN_0163e334(&local_78,uVar16);
          }
          uVar16 = FUN_008c2808();
          uVar28 = (undefined1)param_4[0x16];
          FUN_0163de50(auStack_88,&DAT_003ef238);
          puVar23 = &local_78;
          puVar29 = auStack_88;
          puVar30 = &local_e0;
          uVar10 = 0x15;
          goto LAB_00cb0b48;
        }
        if (iVar9 == 0x6b2a) {
          FUN_009077e8(*DAT_01b99ac0,param_4 + 0x13,(int)param_4[0x12],
                       *(uint *)((long)param_4 + 0x94) & 1);
          if (((*(byte *)((long)param_4 + 0x94) >> 1 & 1) != 0) &&
             (lVar14 = FUN_009e30c0(), lVar14 != 0)) {
            pvVar15 = operator_new(0x330);
            FUN_0163de50(&local_e0,"TID_BILLING_WRONG_PURCHASE_ACCOUNT_TITLE");
            uVar16 = FUN_00eae7d8(&local_e0);
            FUN_0163de50(&local_78,"TID_BILLING_WRONG_PURCHASE_ACCOUNT_INFO");
            uVar17 = FUN_00eae7d8(&local_78);
            FUN_00b5ee60(pvVar15,0x3e,uVar16,uVar17);
                    /* WARNING: Subroutine does not return */
            FUN_0163dd30(&local_78);
          }
          goto LAB_00cb2eb0;
        }
      }
      goto LAB_00cb0b58;
    }
    if (iVar9 < 0x709f) {
      if (iVar9 < 0x6ded) {
        if (0x6bb5 < iVar9) {
          if (iVar9 < 0x6dd2) {
            if (iVar9 == 0x6bb6) {
              iVar9 = (int)param_4[0x12];
              if (iVar9 < 2) {
                if (iVar9 == 0) {
                  if (*DAT_01b99aa0 != 0) {
                    FUN_00e8b1bc();
                  }
                  goto LAB_00cb2eb0;
                }
                if (iVar9 != 1) goto LAB_00cb2eb0;
              }
              else if ((iVar9 != 2) && (iVar9 != 3)) goto LAB_00cb2eb0;
              if (*DAT_01b99aa0 != 0) {
                FUN_00e8b870();
              }
              goto LAB_00cb2eb0;
            }
            iVar13 = 0x6c31;
            goto LAB_00caff28;
          }
          if (iVar9 == 0x6dd2) {
            iVar9 = (int)param_4[0x13];
            if (iVar9 == 4) {
              uVar16 = FUN_0163de50(&local_e0,"TID_INVITE_FRIEND_TOKEN_ERROR_TOO_MANY");
              FUN_00cb3250(uVar16,&local_e0);
                    /* WARNING: Subroutine does not return */
              FUN_0163dd30(&local_e0);
            }
            if (iVar9 == 3) {
              uVar16 = FUN_0163de50(&local_e0,"TID_INVITE_FRIEND_TOKEN_ERROR_FRIENDS_FULL");
              FUN_00cb3250(uVar16,&local_e0);
                    /* WARNING: Subroutine does not return */
              FUN_0163dd30(&local_e0);
            }
            if (iVar9 != 1) {
              uVar16 = FUN_0163de50(&local_e0,"TID_INVITE_FRIEND_TOKEN_ERROR");
              FUN_00cb3250(uVar16,&local_e0);
                    /* WARNING: Subroutine does not return */
              FUN_0163dd30(&local_e0);
            }
            lVar14 = *DAT_01b99b08;
            if (lVar14 != 0) {
              FUN_012b5bb4(auStack_2a0,param_4);
              FUN_00e88580(lVar14,auStack_2a0);
            }
            goto LAB_00cb2eb0;
          }
          if (iVar9 != 0x6dd8) goto LAB_00cb0b58;
          lVar14 = FUN_009e30c0();
          if ((lVar14 == 0) || (plVar24 = (long *)FUN_009f1348(), plVar24 == (long *)0x0))
          goto LAB_00cb2eb0;
          if ((char)param_4[0x12] != '\0') {
            iVar9 = *(int *)((long)param_4 + 0x94);
            if (iVar9 < 4) {
              if (iVar9 == 1) goto LAB_00cb1b9c;
              if (iVar9 == 2) {
                pcVar27 = "TID_NAME_TOO_SHORT";
              }
              else {
                if (iVar9 != 3) goto LAB_00cb2eb0;
                pcVar27 = "TID_NAME_ALREADY_CHANGED";
              }
            }
            else {
              if (iVar9 < 6) {
                if (iVar9 != 4) {
                  if (iVar9 == 5) {
                    FUN_00978c00();
                    lVar14 = FUN_00ff9c28();
                    FUN_0164089c(&local_e0,&DAT_0042e23f,*(undefined4 *)(lVar14 + 0x1c4));
                    FUN_0163de50(auStack_88,"TID_NAME_CHANGE_TH_LEVEL_TOO_LOW");
                    uVar16 = FUN_00eae7d8(auStack_88);
                    FUN_0163de50(local_98,"<level>");
                    FUN_0163f924(&local_78,uVar16,local_98,&local_e0);
                    (**(code **)*plVar24)(plVar24,&local_78);
                    /* WARNING: Subroutine does not return */
                    FUN_0163dd30(&local_78);
                  }
                  goto LAB_00cb2eb0;
                }
              }
              else {
                if (iVar9 == 6) {
                  pcVar27 = "TID_SERVER_INTERNAL_ERROR";
                  goto LAB_00cb1d84;
                }
                if (iVar9 != 0xb) goto LAB_00cb2eb0;
              }
LAB_00cb1b9c:
              pcVar27 = "TID_INVALID_NAME";
            }
LAB_00cb1d84:
            FUN_0163de50(&local_e0,pcVar27);
            uVar16 = FUN_00eae7d8(&local_e0);
            (**(code **)*plVar24)(plVar24,uVar16);
LAB_00cb2ea8:
                    /* WARNING: Subroutine does not return */
            FUN_0163dd30(&local_e0);
          }
          FUN_012aab8c(&local_e0,param_4);
          (**(code **)(*plVar24 + 8))(plVar24,local_e0);
          ppuVar22 = local_e0;
joined_r0x00cb174c:
          if (ppuVar22 != (undefined **)0x0) {
LAB_00cb0c10:
                    /* WARNING: Subroutine does not return */
            FUN_0163dd30(ppuVar22);
          }
          goto LAB_00cb2eb0;
        }
        if (0x6b6b < iVar9) {
          if (iVar9 == 0x6b6c) {
            lVar14 = param_4[0x13];
            if ((0 < (int)lVar14) &&
               (pvVar15 = (void *)FUN_012be484(param_4), pvVar15 != (void *)0x0)) {
              local_e0 = (void *)0x0;
              iVar9 = FUN_0161e5ac(pvVar15,(int)lVar14,&local_e0,0);
              pvVar21 = local_e0;
              if ((0 < iVar9) && (local_e0 != (void *)0x0)) {
                pvVar20 = operator_new(0x10);
                FUN_0163de30(pvVar20,pvVar21,iVar9);
                uVar16 = FUN_00ea0658();
                FUN_00ea1f90(uVar16,pvVar20);
              }
              if (local_e0 != (void *)0x0) {
                operator_delete__(local_e0);
              }
              local_e0 = (undefined **)0x0;
              operator_delete__(pvVar15);
            }
            goto LAB_00cb2eb0;
          }
          iVar13 = 0x6bad;
          goto LAB_00caff28;
        }
        if (iVar9 == 0x6b30) {
          lVar14 = FUN_009e30c0();
          if ((lVar14 == 0) || (lVar14 = FUN_00db3094(), lVar14 == 0)) goto LAB_00cb2eb0;
          FUN_012c2f44(&local_e0,param_4);
          FUN_00db3094();
          lVar14 = FUN_00db1318();
          uVar16 = FUN_00db3094();
          if (lVar14 == 0) {
            FUN_00db1f24(uVar16,local_e0);
          }
          else {
            uVar16 = FUN_00db1318();
            FUN_00a1d384(uVar16,local_e0);
          }
          goto joined_r0x00cb06a8;
        }
        if (iVar9 == 0x6b34) {
          uVar10 = 0x17;
          if ((int)param_4[0x12] != 1) {
            uVar10 = 7;
          }
          uVar16 = FUN_008c2808(7);
          FUN_0163da5c(&local_e0);
          FUN_0163da5c(&local_78);
          FUN_0163da5c(auStack_88);
LAB_00cb0b44:
          puVar30 = auStack_88;
          puVar29 = &local_78;
          puVar23 = &local_e0;
          uVar28 = 0;
LAB_00cb0b48:
          FUN_008c79d8(uVar16,uVar10,uVar28,puVar23,puVar29,puVar30);
LAB_00cb0b4c:
                    /* WARNING: Subroutine does not return */
          FUN_0163dd30(auStack_88);
        }
      }
      else if (iVar9 < 0x6f58) {
        if (iVar9 < 0x6f03) {
          if (iVar9 == 0x6ded) {
            lVar26 = param_4[0x13];
            lVar14 = FUN_009e30c0();
            if (lVar14 != 0) {
              uVar16 = FUN_009e30c0();
              lVar14 = FUN_009e45e0(uVar16,0x33,1);
              if (lVar14 != 0) {
                FUN_00b5ca24(lVar14,lVar26);
              }
            }
            lVar14 = FUN_00e9833c();
            if (((lVar14 != 0) && (lVar14 = FUN_00e9cd28(), lVar14 != 0)) &&
               (lVar14 = FUN_00a049f4(), lVar14 != 0)) {
              uVar16 = FUN_00a049f4();
              FUN_00a05140(uVar16,lVar26);
            }
            goto LAB_00cb2eb0;
          }
          if (iVar9 == 0x6e89) {
            lVar14 = FUN_00db3094();
            if (lVar14 != 0) {
              FUN_00db3094();
              lVar14 = FUN_00db1318();
              if (lVar14 != 0) {
                FUN_00db3094();
                uVar16 = FUN_00db1318();
                FUN_00a1cf6c(uVar16,param_4 + 0x12,(char)param_4[0x13],
                             *(undefined1 *)((long)param_4 + 0x99));
              }
            }
            goto LAB_00cb2eb0;
          }
        }
        else {
          if (iVar9 == 0x6f03) goto LAB_00cafefc;
          if (iVar9 == 0x6f45) {
            lVar14 = FUN_00a049f4();
            if (lVar14 != 0) {
              uVar16 = FUN_00a049f4();
              FUN_00a05340(uVar16,(int)param_4[0x12]);
              uVar16 = FUN_00a049f4();
              FUN_00a05348(uVar16,param_4 + 0x15,param_4 + 0x13);
            }
            goto LAB_00cb2eb0;
          }
        }
      }
      else if (iVar9 < 0x6fec) {
        if (iVar9 == 0x6f58) goto LAB_00cb2eb0;
        if (iVar9 == 0x6fd4) {
          uVar16 = FUN_00a049f4();
          FUN_00f4b57c(auStack_a0,param_4);
          FUN_00a068c8(uVar16,auStack_a0);
          goto LAB_00cb2eb0;
        }
      }
      else {
        if (iVar9 == 0x6fec) {
          *DAT_01b999d8 = 0;
          lVar14 = FUN_009e30c0();
          if (((lVar14 != 0) && (lVar14 = FUN_00db8d28(), lVar14 != 0)) &&
             (lVar26 = FUN_00db8270(), lVar26 != 0)) {
            FUN_00db8270(lVar14);
            thunk_FUN_00dd96b8();
          }
          goto LAB_00cb2eb0;
        }
        if (iVar9 == 0x708a) {
          iVar9 = (int)param_4[0x13];
          if (iVar9 < 3) {
            if (iVar9 == 1) {
              pcVar27 = "TID_ADD_FRIEND_FAILED_BANNED";
            }
            else {
              if (iVar9 != 2) goto LAB_00cb1404;
              pcVar27 = "TID_ADD_FRIEND_FAILED_THEIR_LIST_FULL";
            }
          }
          else if (iVar9 == 3) {
            pcVar27 = "TID_ADD_FRIEND_FAILED_YOUR_LIST_FULL";
          }
          else if (iVar9 == 4) {
            pcVar27 = "TID_ADD_FRIEND_FAILED_TOKEN_EXPIRED";
          }
          else {
LAB_00cb1404:
            pcVar27 = "TID_ADD_FRIEND_FAILED";
          }
          uVar16 = FUN_0163de50(&local_e0,pcVar27);
          FUN_00cb3250(uVar16,&local_e0);
          goto LAB_00cb19e0;
        }
      }
    }
    else if (iVar9 < 0x738f) {
      if (iVar9 < 0x7224) {
        if (iVar9 < 0x71ec) {
          if (iVar9 == 0x709f) {
            lVar14 = FUN_00db8d28();
            if (lVar14 == 0) {
LAB_00cb0b6c:
              bVar4 = false;
            }
            else {
              FUN_00db8d28();
              lVar14 = FUN_00db8270();
              if (lVar14 == 0) goto LAB_00cb0b6c;
              FUN_00db8d28();
              lVar14 = FUN_00db8270();
              bVar4 = *(char *)(lVar14 + 0x2c8) != '\0';
            }
            lVar14 = FUN_00db8d28();
            if (lVar14 != 0) {
              FUN_00db8d28();
              lVar14 = FUN_00db825c();
              if (lVar14 != 0) {
                FUN_00db8d28();
                lVar14 = FUN_00db825c();
                if (*(long *)(lVar14 + 0x1c8) != 0) {
                  bVar4 = true;
                }
              }
            }
            plVar24 = (long *)FUN_01484ca8();
            if (plVar24 == (long *)0x0) {
              bVar7 = 1;
            }
            else {
              bVar7 = (**(code **)(*plVar24 + 0x28))();
            }
            lVar14 = FUN_00e9833c();
            if (lVar14 == 0) {
              bVar5 = false;
            }
            else {
              bVar5 = *(char *)(lVar14 + 0x1c8) != '\0';
            }
            if ((bVar4 || ((bVar7 ^ 0xff) & 1) != 0) || (bVar5 != false)) {
              if (!bVar4 && (bVar7 & bVar5) == 1) {
                if (*(long **)(param_3 + 0xf8) != (long *)0x0) {
                  (**(code **)(**(long **)(param_3 + 0xf8) + 0x38))();
                  if (*(long **)(param_3 + 0xf8) != (long *)0x0) {
                    (**(code **)(**(long **)(param_3 + 0xf8) + 8))();
                  }
                }
                *(undefined4 *)(param_3 + 0x100) = 0;
                *(undefined1 *)(param_3 + 0x104) = 1;
                *(long **)(param_3 + 0xf8) = param_4;
                return 0;
              }
              lVar14 = FUN_00db8d28();
              if (lVar14 != 0) {
                FUN_00db8d28();
                lVar14 = FUN_00db825c();
                if (lVar14 != 0) {
                  FUN_00db8d28();
                  lVar14 = FUN_00db825c();
                  if (*(long *)(lVar14 + 0x1c8) != 0) {
                    FUN_00db8d28();
                    lVar14 = FUN_00db825c();
                    (**(code **)(**(long **)(lVar14 + 0x1c8) + 0x1c0))();
                    goto LAB_00cb14bc;
                  }
                }
              }
              lVar14 = FUN_00db8d28();
              if (lVar14 != 0) {
                FUN_00db8d28();
                lVar14 = FUN_00db8270();
                if (lVar14 != 0) {
                  FUN_00db8d28();
                  FUN_00db8270();
                  FUN_00ddb4ec();
                }
              }
LAB_00cb14bc:
              if (*(long **)(param_3 + 0xf8) != (long *)0x0) {
                (**(code **)(**(long **)(param_3 + 0xf8) + 0x38))();
                if (*(long **)(param_3 + 0xf8) != (long *)0x0) {
                  (**(code **)(**(long **)(param_3 + 0xf8) + 8))();
                }
              }
              *(undefined4 *)(param_3 + 0x100) = 0;
              *(undefined1 *)(param_3 + 0x104) = 0;
              *(long **)(param_3 + 0xf8) = param_4;
              return 0;
            }
            FUN_00cb2f98(param_3,param_4);
            goto LAB_00cb2eb0;
          }
          if (iVar9 == 0x7139) {
            FUN_00978c00();
            uVar25 = FUN_00ffd548();
            if ((uVar25 & 1) != 0) {
              FUN_00978c00();
              lVar14 = FUN_00ff9c28();
              if (*(char *)(lVar14 + 0x395) != '\0') {
                FUN_00ca26c8((int)param_4[0x12]);
                FUN_00ef428c((int)param_4[0x12]);
              }
            }
            goto LAB_00cb2eb0;
          }
        }
        else {
          if (iVar9 == 0x71ec) {
            if ((long *)*DAT_01b99a38 != (long *)0x0) {
              lVar14 = *(long *)*DAT_01b99a38;
              if ((int)param_4[0x12] == 0) {
                (**(code **)(lVar14 + 0xe0))();
              }
              else {
                (**(code **)(lVar14 + 0xd8))();
              }
            }
            goto LAB_00cb2eb0;
          }
          if (iVar9 == 0x71f8) {
            if (((*DAT_01b99b08 == 0) ||
                (FUN_012b59b4(&local_e0,param_4), local_e0 == (undefined **)0x0)) ||
               (FUN_00e85f80(*plVar2), local_e0 == (undefined **)0x0)) goto LAB_00cb2eb0;
            FUN_012b4c00();
            goto joined_r0x00cb00d0;
          }
        }
      }
      else {
        if (0x72b5 < iVar9) {
          if (iVar9 == 0x72b6) {
            thunk_FUN_016a1e50((char)param_4[0x12],*(undefined4 *)((long)param_4 + 0x94),0);
            puVar3 = DAT_01b9aa88;
            lVar14 = param_4[0x14];
            *DAT_01b9a630 = (char)param_4[0x13];
            *puVar3 = (char)lVar14;
            goto LAB_00cb2eb0;
          }
          iVar13 = 0x72ee;
          goto LAB_00caf4f4;
        }
        if (iVar9 == 0x7224) goto LAB_00caff30;
        if (iVar9 == 0x7261) {
          plVar24 = (long *)*DAT_01b99a38;
          if (plVar24 != (long *)0x0) {
            if ((int)param_4[0x12] == 0) {
              lVar14 = FUN_012c6368(param_4);
              if (lVar14 == 0) {
                puVar23 = (undefined8 *)&DAT_003ef238;
              }
              else {
                lVar14 = FUN_012c6368(param_4);
                puVar23 = (undefined8 *)(lVar14 + 8);
                if (7 < *(int *)(lVar14 + 4)) {
                  puVar23 = *(undefined8 **)(lVar14 + 8);
                }
              }
              FUN_0163de50(&local_e0,puVar23);
              (**(code **)(*plVar24 + 0xc0))(plVar24,&local_e0);
              goto LAB_00cb19e0;
            }
            (**(code **)(*plVar24 + 200))(plVar24);
          }
          goto LAB_00cb2eb0;
        }
      }
    }
    else if (iVar9 < 0x74b3) {
      if (iVar9 < 0x73ea) {
        if (iVar9 == 0x738f) {
          lVar14 = FUN_00a049f4();
          if (lVar14 != 0) {
            lVar14 = FUN_00a049f4();
            uVar16 = *(undefined8 *)(lVar14 + 0x50);
            FUN_012a4458(&local_e0,param_4);
            if (local_e0 != (undefined **)0x0) {
              local_130 = local_e0;
              FUN_00a9fd88(uVar16,&local_130);
            }
          }
          goto LAB_00cb2eb0;
        }
        if (iVar9 == 0x73a3) {
          if (*DAT_01b99a98 == 0) goto LAB_00cb2eb0;
          lVar14 = FUN_009e30c0();
          plVar24 = DAT_01b99a38;
          if ((lVar14 == 0) ||
             ((lVar14 = param_4[0x18], *(char *)(*DAT_01b99a38 + 0x40) != '\0' &&
              ((uVar25 = FUN_00da0084(), (uVar25 & 1) != 0 ||
               (uVar25 = FUN_016bf8f0(*plVar24), (uVar25 & 1) != 0)))))) goto LAB_00cb2eb0;
          if (((char)param_4[0x17] != '\0') && (*(char *)(*plVar24 + 0x40) != '\0')) {
LAB_00caf2b8:
            FUN_0163de50(&local_e0,"TID_JUPITER_BOUND_TO_OTHER_ACCOUNT_TITLE");
            FUN_00eae7d8(&local_e0);
                    /* WARNING: Subroutine does not return */
            FUN_0163dd30(&local_e0);
          }
          if (param_4[0x14] == 0) goto LAB_00cb2eb0;
          FUN_0163da5c(&local_e0);
          FUN_0163da5c(&local_78);
          FUN_00978c00();
          lVar26 = FUN_00ff9c28();
          iVar9 = *(int *)(lVar26 + 0x2a4);
          FUN_0163da5c(auStack_88);
          if (*plVar1 != 0) {
            uVar16 = FUN_00e8ad50();
            FUN_0163e334(auStack_88,uVar16);
          }
          iVar13 = FUN_00f03fd8(lVar14);
          if (iVar9 <= iVar13) {
            FUN_0163de50(local_98,"TID_GAMECENTER_BOUND_TO_OTHER_ACCOUNT_TITLE");
            FUN_00eae7d8(local_98);
                    /* WARNING: Subroutine does not return */
            FUN_0163dd30(local_98);
          }
          goto LAB_00cb0b4c;
        }
      }
      else {
        if (iVar9 == 0x73ea) goto LAB_00cb2eb0;
        if (iVar9 == 0x749f) {
          if (*(char *)((long)param_4 + 0xa4) != '\0') {
            pvVar15 = operator_new(0x88);
            uVar16 = FUN_00978c00();
            FUN_012a7b84(pvVar15,uVar16);
            uVar16 = FUN_0149fc4c(param_4[0x12]);
            FUN_012a7df8(pvVar15,uVar16);
            FUN_012a7e48(pvVar15,param_4[0x13]);
            FUN_012a7de8(pvVar15,(int)param_4[0x14]);
            uVar25 = FUN_00dd67e4(0x11,pvVar15,0,0,0,0);
            if ((uVar25 & 1) != 0) {
              FUN_00db8d28();
              FUN_00db8270();
              FUN_00dd6be4();
            }
            goto LAB_00cb2eb0;
          }
          lVar14 = FUN_009e30c0();
          pcVar27 = "TID_REPLAY_NOT_AVAILABLE";
          goto LAB_00cb16a8;
        }
      }
    }
    else {
      if (0x74cc < iVar9) {
        if (iVar9 == 0x74cd) {
          pvVar15 = (void *)param_4[0x13];
          param_4[0x13] = 0;
          if (pvVar15 != (void *)0x0) {
            lVar14 = FUN_014b45d4(pvVar15);
            if (((lVar14 != 0) && (lVar26 = *DAT_01b99a38, lVar26 != 0)) &&
               (*(char *)(lVar26 + 0x40) != '\0')) {
              FUN_0163e334(lVar26 + 0x208,lVar14);
            }
            FUN_014b4470(pvVar15);
            pvVar21 = *(void **)((long)pvVar15 + 8);
            *(undefined8 *)((long)pvVar15 + 8) = 0;
            if (pvVar21 != (void *)0x0) {
              operator_delete__(pvVar21);
            }
            operator_delete(pvVar15);
            if (lVar14 != 0) {
                    /* WARNING: Subroutine does not return */
              FUN_0163dd30(lVar14);
            }
          }
          if (param_4[0x12] != 0) {
            FUN_01561b54();
            lVar14 = FUN_00e9833c();
            if (lVar14 != 0) {
              plVar24 = (long *)FUN_00e9833c();
              uVar25 = (**(code **)(*plVar24 + 0x50))();
              if ((uVar25 & 1) != 0) {
                plVar24 = (long *)FUN_00e9833c();
                lVar14 = (**(code **)(*plVar24 + 0x88))();
                if (((lVar14 != 0) && (lVar14 = FUN_012d316c(), lVar14 != 0)) &&
                   (lVar26 = FUN_011fae98(), lVar26 != 0)) {
                  lVar14 = FUN_011fae98(lVar14);
                  *(undefined1 *)(lVar14 + 9) = *(undefined1 *)(DAT_01b99a70 + 0xe);
                }
              }
            }
            if (*DAT_01b99ab0 != 0) {
              FUN_00cbf048();
            }
          }
          plVar24 = (long *)FUN_01484c9c();
          if (plVar24 != (long *)0x0) {
            (**(code **)(*plVar24 + 0x90))();
          }
          goto LAB_00cb2eb0;
        }
        if (iVar9 == 0x752d) {
          FUN_00cb6748(*plVar24,(int)param_4[0x12],0);
          goto LAB_00cb2eb0;
        }
        iVar13 = 40000;
        goto LAB_00caf4f4;
      }
      if (iVar9 == 0x74b3) {
LAB_00cafefc:
        if (*DAT_01b99d08 != 0) {
          FUN_009098ec(*DAT_01b99d08,param_4);
        }
        goto LAB_00cb2eb0;
      }
      if (iVar9 == 0x74cc) {
        lVar14 = *DAT_01b99a38;
        if (lVar14 != 0) {
          uVar16 = FUN_014a4dac(param_4);
          FUN_00da069c(lVar14,uVar16);
        }
        goto LAB_00cb2eb0;
      }
    }
  }
LAB_00cb0b58:
  FUN_00e8bc8c(_DAT_01c4b558,param_4);
LAB_00cb2eb0:
  lVar14 = FUN_01297cdc(param_4);
  if (((lVar14 != 0) && (plVar24 = (long *)FUN_00e9833c(), plVar24 != (long *)0x0)) &&
     (uVar25 = FUN_00e9cab0(), (uVar25 & 1) == 0)) {
    FUN_00ea0658();
    uVar25 = FUN_00ea214c();
    if ((((uVar25 & 1) != 0) && (lVar26 = (**(code **)(*plVar24 + 0x88))(plVar24), lVar26 != 0)) &&
       ((*(long *)(lVar26 + 0x220) != 0 && (iVar9 = FUN_012d3c98(lVar26), iVar9 != 4)))) {
      FUN_01297d10(lVar14,lVar26);
    }
  }
  puVar29 = *(undefined8 **)(param_3 + 0x128);
  for (puVar23 = *(undefined8 **)(param_3 + 0x120); puVar23 != puVar29; puVar23 = puVar23 + 1) {
    (**(code **)(*(long *)*puVar23 + 0x10))((long *)*puVar23,param_4);
  }
  return 1;
}


// ===== 0x7c2808 =====
// entry 7c2808

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined8 FUN_008c2808(void)

{
  return _DAT_01c4b720;
}


