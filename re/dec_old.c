// ===== 0xc87620 =====
// entry c87620

void FUN_00d87620(long param_1)

{
  int iVar1;
  uint uVar2;
  void *pvVar3;
  undefined8 *puVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  ulong uVar8;
  long lVar9;
  long lVar10;
  long *plVar11;
  undefined8 uVar12;
  undefined8 uVar13;
  long *plVar14;
  void *pvVar15;
  future<void> afStack_d8 [8];
  undefined8 local_d0;
  long *plStack_c8;
  void *local_c0;
  undefined8 uStack_b8;
  undefined8 *local_b0;
  undefined8 uStack_a8;
  undefined8 local_a0;
  undefined8 uStack_98;
  long local_90;
  undefined8 uStack_88;
  undefined8 uStack_80;
  undefined8 uStack_78;
  long *local_70;
  undefined8 uStack_68;
  undefined8 local_60;
  undefined8 uStack_58;
  future<void> afStack_48 [8];
  
  iVar1 = *(int *)(param_1 + 0x30);
  if (iVar1 < 4) {
    if (1 < iVar1 - 1U) {
      if (iVar1 != 0) {
        if (iVar1 != 3) {
          return;
        }
        FUN_00d8c5b8();
        uVar8 = FUN_00d8df64();
        if ((uVar8 & 1) == 0) {
          return;
        }
        FUN_00d8c5b8();
        lVar9 = FUN_00d8df5c();
        if (lVar9 == 0) {
          FUN_013145f8("replay json object is null",1);
        }
        FUN_00d8c5b8();
        lVar10 = FUN_00d8df74();
        uVar8 = FUN_00dd6df8();
        if (((uVar8 & 1) == 0) && (lVar10 == 0)) {
          FUN_013145f8("replay is missing battle log json",1);
        }
        lVar9 = FUN_01317a18(lVar9,DAT_019e6f88,1);
        if (lVar9 == 0) {
          FUN_013145f8("cannot find battle object from replay json",1);
        }
        FUN_0148a810(&local_d0,"location");
        FUN_01317934(lVar9,&local_d0,1);
                    /* WARNING: Subroutine does not return */
        FUN_0148a700(&local_d0);
      }
      if (*(int *)(param_1 + 0x58) != 2) {
        if (*(int *)(param_1 + 0x58) != 1) {
          plVar11 = &local_90;
          plStack_c8 = (long *)0x0;
          local_d0 = 0;
          uStack_b8 = 0;
          local_c0 = (void *)0x0;
          uStack_a8 = 0;
          local_b0 = (undefined8 *)0x0;
          uStack_98 = 0;
          local_a0 = 0x3f800000;
          uStack_88 = 0;
          local_90 = 0;
          uStack_78 = 0;
          uStack_80 = 0;
          uStack_68 = 0;
          local_70 = (long *)0x0;
          uStack_58 = 0;
          local_60 = 1;
          FUN_012a2188(afStack_48,*(undefined8 *)(param_1 + 0x1e8),&local_d0);
          std::__ndk1::future<void>::~future(afStack_48);
          if (local_70 == plVar11) {
            lVar9 = 4;
LAB_00d87e0c:
            (**(code **)(*plVar11 + lVar9 * 8))();
            pvVar3 = local_c0;
            puVar4 = local_b0;
          }
          else {
            pvVar3 = local_c0;
            puVar4 = local_b0;
            if (local_70 != (long *)0x0) {
              lVar9 = 5;
              plVar11 = local_70;
              goto LAB_00d87e0c;
            }
          }
          while (puVar4 != (void *)0x0) {
            plVar14 = (long *)puVar4[8];
            plVar11 = puVar4 + 4;
            pvVar15 = (void *)*puVar4;
            local_c0 = pvVar3;
            if (plVar14 == plVar11) {
              lVar9 = 4;
LAB_00d87e90:
              (**(code **)(*plVar11 + lVar9 * 8))();
            }
            else if (plVar14 != (long *)0x0) {
              lVar9 = 5;
              plVar11 = plVar14;
              goto LAB_00d87e90;
            }
            operator_delete(puVar4);
            pvVar3 = local_c0;
            puVar4 = pvVar15;
          }
          local_c0 = (void *)0x0;
          if (pvVar3 != (void *)0x0) {
            operator_delete(pvVar3);
          }
          plVar11 = plStack_c8;
          if ((plStack_c8 != (long *)0x0) &&
             (lVar9 = FUN_018e0ef0(0xffffffffffffffff,plStack_c8 + 1), lVar9 == 0)) {
            (**(code **)(*plVar11 + 0x10))(plVar11);
            std::__ndk1::__shared_weak_count::__release_weak();
          }
          uVar5 = 5;
          goto LAB_00d87e64;
        }
        lVar9 = FUN_00d8c5b8();
        uVar2 = *(int *)(lVar9 + 0x10) - 1;
        if (uVar2 < 4) {
          uVar5 = *(undefined4 *)(&DAT_003b4eb0 + (ulong)uVar2 * 4);
          goto LAB_00d87e64;
        }
      }
      uVar5 = 1;
LAB_00d87e64:
      *(undefined4 *)(param_1 + 0x30) = uVar5;
      *(undefined4 *)(param_1 + 0x74) = 0x3f800000;
      return;
    }
  }
  else {
    if (5 < iVar1) {
      if (iVar1 != 6) {
        if (iVar1 != 7) {
          return;
        }
        if (*(long **)(param_1 + 0x98) != (long *)0x0) {
          (**(code **)(**(long **)(param_1 + 0x98) + 8))();
        }
        *(undefined8 *)(param_1 + 0x98) = 0;
        if (*(int *)(param_1 + 0x58) == 2) {
          plVar11 = (long *)FUN_00cb8f04(param_1,*(undefined4 *)(param_1 + 0x1e0));
        }
        else if (*(int *)(param_1 + 0x58) == 0) {
          FUN_00d8c53c(&DAT_003f812b);
          plVar11 = operator_new(0x4d0);
          FUN_00cbbd78(plVar11,param_1);
        }
        else {
          FUN_00d8845c(param_1);
          lVar9 = FUN_01337e24();
          if (lVar9 != 0) {
            FUN_0090031c();
            lVar10 = FUN_00eccc70();
            if (*(char *)(lVar10 + 0x5d0) != '\0') {
              FUN_01338a58(lVar9);
            }
          }
          *DAT_019e6ee8 = 0;
          plVar11 = operator_new(0x470);
          FUN_00caebe8(plVar11,param_1);
        }
        *(long **)(param_1 + 0x50) = plVar11;
        uVar13 = (**(code **)(*plVar11 + 0xa58))();
        *(undefined8 *)(param_1 + 0x38) = uVar13;
        lVar9 = FUN_00855818();
        if (*(char *)(lVar9 + 0x251) != '\0') {
          *(undefined1 *)(*(long *)(param_1 + 0x38) + 8) = 0;
        }
        lVar9 = FUN_00855818();
        FUN_012d8d58(*(undefined8 *)(lVar9 + 0x168),*(undefined8 *)(param_1 + 0x38));
        FUN_00d887d4(param_1,*(undefined8 *)(param_1 + 0x50));
        (**(code **)(**(long **)(param_1 + 0x50) + 0x9d0))();
        *(undefined1 *)(param_1 + 0x18d) = 1;
        *(undefined4 *)(param_1 + 0x30) = 8;
        return;
      }
      lVar9 = FUN_00d8c5b8();
      lVar10 = *(long *)(param_1 + 0x90);
      if (((lVar10 == 0) || ((*(long *)(lVar10 + 0xa8) == 0 && (*(long *)(lVar10 + 0x218) == 0))))
         && ((uVar8 = FUN_00d8df64(lVar9), (uVar8 & 1) == 0 || (*(int *)(lVar9 + 0x10) != 3)))) {
        uVar8 = FUN_00d8e12c(lVar9);
        if ((uVar8 & 1) == 0) {
          return;
        }
        uVar13 = FUN_00d8e13c(lVar9);
        FUN_00d8e154(lVar9);
        uVar12 = FUN_00d8c5c4(lVar9);
        uVar6 = FUN_00d8e14c(lVar9);
        uVar7 = FUN_00d8e1dc(lVar9);
        lVar10 = *(long *)(param_1 + 0x90);
        uVar5 = *(undefined4 *)(lVar9 + 0x58);
        *(undefined8 *)(lVar10 + 0x1c8) = *(undefined8 *)(lVar9 + 0x68);
        FUN_011619d8(lVar10,uVar5);
        FUN_01161724(*(undefined8 *)(param_1 + 0x90),uVar13,uVar12,uVar6,0xffffffff,uVar7);
        lVar9 = FUN_0116209c(*(undefined8 *)(param_1 + 0x90));
        FUN_010bb008(*(undefined8 *)(lVar9 + 0x468));
        *(undefined4 *)(param_1 + 0x30) = 7;
        uVar8 = FUN_00dd6e24();
        if ((uVar8 & 1) == 0) {
          return;
        }
        FUN_0109e420(uVar13);
        return;
      }
      uVar5 = 7;
      goto LAB_00d87a24;
    }
    if (iVar1 != 4) {
      if (iVar1 != 5) {
        return;
      }
      uVar8 = FUN_00d88308(param_1);
      if ((uVar8 & 1) == 0) {
        return;
      }
      *(undefined4 *)(param_1 + 0x30) = 6;
      return;
    }
  }
  if (*(int *)(param_1 + 0x58) == 2) {
    if ((*(long *)(param_1 + 0x90) == 0) || (uVar8 = FUN_011659c8(), (uVar8 & 1) == 0)) {
      return;
    }
    if (*(int *)(param_1 + 0x1e0) == 1) {
      FUN_00c5529c(*(undefined8 *)(*(long *)(param_1 + 0x90) + 0x218),
                   *(undefined8 *)(param_1 + 0x1e8));
    }
  }
  else if (*(int *)(param_1 + 0x58) == 1) {
    if (*(long *)(param_1 + 0x90) == 0) {
      return;
    }
    if (*(long *)(*(long *)(param_1 + 0x90) + 0xa8) == 0) {
      return;
    }
    uVar8 = FUN_011659c8();
    if ((uVar8 & 1) == 0) {
      return;
    }
    FUN_00cb7338(*(undefined8 *)(*(long *)(param_1 + 0x90) + 0xa8),*(undefined8 *)(param_1 + 0x1e8))
    ;
    uVar5 = FUN_01292fa4();
    *(undefined4 *)(param_1 + 0xbc) = 0;
    *(undefined4 *)(param_1 + 0xa4) = 0;
    *(undefined4 *)(param_1 + 0xa8) = 0;
    *(undefined4 *)(param_1 + 0xa0) = uVar5;
  }
  plVar11 = &local_90;
  plStack_c8 = (long *)0x0;
  local_d0 = 0;
  uStack_b8 = 0;
  local_c0 = (void *)0x0;
  uStack_a8 = 0;
  local_b0 = (undefined8 *)0x0;
  uStack_98 = 0;
  local_a0 = 0x3f800000;
  uStack_88 = 0;
  local_90 = 0;
  uStack_78 = 0;
  uStack_80 = 0;
  uStack_68 = 0;
  local_70 = (long *)0x0;
  uStack_58 = 0;
  local_60 = 1;
  FUN_012a2188(afStack_d8,*(undefined8 *)(param_1 + 0x1e8),&local_d0);
  std::__ndk1::future<void>::~future(afStack_d8);
  if (local_70 == plVar11) {
    lVar9 = 4;
LAB_00d879cc:
    (**(code **)(*plVar11 + lVar9 * 8))();
    pvVar3 = local_c0;
    puVar4 = local_b0;
  }
  else {
    pvVar3 = local_c0;
    puVar4 = local_b0;
    if (local_70 != (long *)0x0) {
      lVar9 = 5;
      plVar11 = local_70;
      goto LAB_00d879cc;
    }
  }
  while (puVar4 != (void *)0x0) {
    plVar14 = (long *)puVar4[8];
    plVar11 = puVar4 + 4;
    pvVar15 = (void *)*puVar4;
    local_c0 = pvVar3;
    if (plVar14 == plVar11) {
      lVar9 = 4;
LAB_00d87a48:
      (**(code **)(*plVar11 + lVar9 * 8))();
    }
    else if (plVar14 != (long *)0x0) {
      lVar9 = 5;
      plVar11 = plVar14;
      goto LAB_00d87a48;
    }
    operator_delete(puVar4);
    pvVar3 = local_c0;
    puVar4 = pvVar15;
  }
  local_c0 = (void *)0x0;
  if (pvVar3 != (void *)0x0) {
    operator_delete(pvVar3);
  }
  plVar11 = plStack_c8;
  if ((plStack_c8 != (long *)0x0) &&
     (lVar9 = FUN_018e0ef0(0xffffffffffffffff,plStack_c8 + 1), lVar9 == 0)) {
    (**(code **)(*plVar11 + 0x10))(plVar11);
    std::__ndk1::__shared_weak_count::__release_weak();
  }
  uVar5 = 5;
LAB_00d87a24:
  *(undefined4 *)(param_1 + 0x30) = uVar5;
  return;
}


// ===== 0xc87ecc =====
// entry c87ecc

/* WARNING: Removing unreachable block (ram,0x00d88198) */

void FUN_00d87ecc(long param_1,undefined8 param_2,undefined8 param_3)

{
  undefined8 *puVar1;
  int iVar2;
  bool bVar3;
  int iVar4;
  int iVar5;
  long lVar6;
  undefined8 uVar7;
  undefined8 uVar8;
  void *pvVar9;
  ulong uVar10;
  int iVar11;
  long lVar12;
  int *piVar13;
  undefined8 uStack_78;
  int *piStack_70;
  uint uStack_68;
  uint uStack_64;
  
  lVar6 = FUN_00855818();
  if (*(int *)(lVar6 + 400) == 0) {
    FUN_0090031c();
    lVar6 = FUN_00eccc70();
    if (*(char *)(lVar6 + 0x1e5) == '\0') {
      return;
    }
  }
  FUN_014d6880();
  uVar7 = FUN_0090031c();
  uVar8 = FUN_00faf1c8();
  pvVar9 = operator_new(0x230);
  FUN_01160ba8(pvVar9,1,uVar7,uVar8,0);
  FUN_01160f84(pvVar9,1);
  FUN_01161b1c(pvVar9,param_2,param_3,0);
  uVar10 = FUN_00dd6df8();
  if (((uVar10 & 1) != 0) || (uVar10 = FUN_00dd6e1c(), (uVar10 & 1) != 0)) {
    FUN_00e30180(*(undefined8 *)((long)pvVar9 + 0x38));
  }
  lVar6 = *(long *)((long)pvVar9 + 0xa8);
  FUN_00df6e18(lVar6,3);
  if (((*(long *)(lVar6 + 0x1b0) != 0) &&
      (lVar12 = *(long *)(*(long *)(lVar6 + 0x1b0) + 0x18), lVar12 != 0)) &&
     (*(char *)(lVar12 + 0x191) != '\0')) {
    FUN_011610b0(pvVar9);
    FUN_00a88844(pvVar9);
    (*(code *)PTR_operator_delete_019e9368)(pvVar9);
    return;
  }
  FUN_0090031c();
  lVar12 = FUN_00eccc70();
  piStack_70 = (int *)0x0;
  uStack_68 = 0;
  uStack_64 = 0;
  iVar5 = *(int *)(lVar12 + 0x1dc) * 0x14;
LAB_00d88004:
  do {
    uVar10 = FUN_00df13d0(lVar6);
    if (((uVar10 & 1) == 0) && (*(int *)(*(long *)(lVar6 + 8) + 0x1f0) < 1)) {
      bVar3 = false;
      iVar4 = FUN_01162bc0(pvVar9);
      iVar11 = 0;
      if (iVar5 != 0) {
        iVar11 = iVar4 / iVar5;
      }
      if (iVar4 == iVar11 * iVar5) goto LAB_00d8803c;
    }
    else {
      bVar3 = true;
      iVar4 = FUN_01162bc0(pvVar9);
LAB_00d8803c:
      FUN_00bd9a50(&uStack_78,pvVar9);
      uVar7 = uStack_78;
      iVar11 = *(int *)(param_1 + 0x17c);
      if (iVar11 == *(int *)(param_1 + 0x178)) {
        iVar2 = 5;
        if (iVar11 * 2 != 0) {
          iVar2 = iVar11 * 2;
        }
        if (iVar11 < iVar2) {
          FUN_00d8b8e0(param_1 + 0x170);
          iVar11 = *(int *)(param_1 + 0x17c);
        }
        *(int *)(param_1 + 0x17c) = iVar11 + 1;
        puVar1 = (undefined8 *)(*(long *)(param_1 + 0x170) + (long)iVar11 * 0x10);
        *puVar1 = CONCAT44(iVar4,iVar4);
        puVar1[1] = uVar7;
        iVar4 = *(int *)(param_1 + 0x17c);
        if (uStack_64 == uStack_68) {
LAB_00d880dc:
          iVar11 = 5;
          if (uStack_64 * 2 != 0) {
            iVar11 = uStack_64 * 2;
          }
          if ((int)uStack_64 < iVar11) {
            FUN_008734e8(&piStack_70);
          }
        }
      }
      else {
        *(int *)(param_1 + 0x17c) = iVar11 + 1;
        puVar1 = (undefined8 *)(*(long *)(param_1 + 0x170) + (long)iVar11 * 0x10);
        *puVar1 = CONCAT44(iVar4,iVar4);
        puVar1[1] = uStack_78;
        iVar4 = *(int *)(param_1 + 0x17c);
        if (uStack_64 == uStack_68) goto LAB_00d880dc;
      }
      piStack_70[(int)uStack_64] = iVar4 + -1;
      uStack_64 = uStack_64 + 1;
    }
    FUN_011620a4(pvVar9);
    uVar10 = FUN_00df7bac(lVar6);
    if (((uVar10 & 1) != 0) || ((int)uStack_64 < 1)) {
      if (bVar3) {
        if (0 < (int)uStack_64) {
          iVar5 = FUN_01162bc0(pvVar9);
          uVar10 = (ulong)uStack_64;
          if (0 < (int)uStack_64) {
            lVar6 = *(long *)(param_1 + 0x170);
            piVar13 = piStack_70;
            do {
              if (*(int *)(lVar6 + (long)*piVar13 * 0x10) + 1 < iVar5) {
                *(int *)(lVar6 + (long)*piVar13 * 0x10 + 4) = iVar5;
              }
              uVar10 = uVar10 - 1;
              piVar13 = piVar13 + 1;
            } while (uVar10 != 0);
          }
          uStack_64 = 0;
        }
        goto LAB_00d881f4;
      }
      goto LAB_00d88004;
    }
    iVar4 = FUN_01162bc0(pvVar9);
    uVar10 = (ulong)uStack_64;
    if (0 < (int)uStack_64) {
      lVar12 = *(long *)(param_1 + 0x170);
      piVar13 = piStack_70;
      do {
        if (*(int *)(lVar12 + (long)*piVar13 * 0x10) + 1 < iVar4) {
          *(int *)(lVar12 + (long)*piVar13 * 0x10 + 4) = iVar4;
        }
        uVar10 = uVar10 - 1;
        piVar13 = piVar13 + 1;
      } while (uVar10 != 0);
    }
    uStack_64 = 0;
    if (bVar3) {
LAB_00d881f4:
      FUN_011610b0(pvVar9);
      FUN_00a88844(pvVar9);
      operator_delete(pvVar9);
      if (piStack_70 != (int *)0x0) {
        operator_delete__(piStack_70);
      }
      return;
    }
  } while( true );
}


// ===== 0x1061b1c =====
// entry 1061b1c

void FUN_01161b1c(long param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4)

{
  bool bVar1;
  uint uVar2;
  undefined1 uVar3;
  char cVar4;
  undefined4 uVar5;
  long lVar6;
  long *plVar7;
  undefined8 uVar8;
  ulong uVar9;
  void *pvVar10;
  int iVar11;
  uint uVar12;
  undefined8 *puVar13;
  long lVar14;
  int iVar15;
  undefined8 local_58;
  undefined8 local_38;
  
  iVar11 = *(int *)(param_1 + 0x34);
  uVar3 = *(undefined1 *)(param_1 + 0x1e8);
  if (iVar11 == *(int *)(param_1 + 0x30)) {
    iVar15 = 5;
    if (iVar11 * 2 != 0) {
      iVar15 = iVar11 * 2;
    }
    if (iVar11 < iVar15) {
      FUN_00dedb50(param_1 + 0x28);
      iVar11 = *(int *)(param_1 + 0x34);
    }
  }
  lVar6 = *(long *)(param_1 + 0xa8);
  *(int *)(param_1 + 0x34) = iVar11 + 1;
  *(undefined1 *)(*(long *)(param_1 + 0x28) + (long)iVar11) = uVar3;
  *(undefined1 *)(param_1 + 0x1e8) = 1;
  if (lVar6 != 0) {
    lVar6 = FUN_00df0584();
    *(undefined1 *)(lVar6 + 0x79) = 1;
  }
  plVar7 = *(long **)(param_1 + 0x218);
  if (plVar7 != (long *)0x0) {
    (**(code **)(*plVar7 + 0x50))(plVar7,1);
  }
  if (*(int *)(param_1 + 0x24) != 0) goto LAB_01161bc4;
  *(undefined4 *)(param_1 + 0x24) = 4;
  lVar6 = FUN_01317934(param_2,DAT_019e7708,1);
  iVar11 = *(int *)(lVar6 + 0x10);
  if (*(char *)(lVar6 + 8) != '\0') {
    iVar11 = (int)*(float *)(lVar6 + 0xc);
  }
  *(int *)(param_1 + 0x9c) = iVar11;
  FUN_0131b2e4(param_1 + 0xa0,iVar11);
  if (*(long *)(param_1 + 0x218) != 0) {
    FUN_0117c8b0(*(long *)(param_1 + 0x218),iVar11);
  }
  uVar5 = FUN_0127184c(param_2,DAT_019e7710,*(undefined4 *)PTR_DAT_019e6968,0);
  *(undefined4 *)(param_1 + 0x1c4) = uVar5;
  lVar6 = FUN_01317a18(param_2,DAT_019e6f88,1);
  if (lVar6 == 0) {
    FUN_013145f8("cannot find battle object from replay json",1);
  }
  FUN_00df6e18(*(undefined8 *)(param_1 + 0xa8),3);
  thunk_FUN_01253bd4(*(undefined8 *)(param_1 + 0xa8),lVar6,param_3,param_4);
  lVar6 = FUN_013179cc(param_2,DAT_019e7718,1);
  if (lVar6 == 0) {
    FUN_013145f8("cannot find cmd array from replay json",1);
  }
  iVar11 = *(int *)(lVar6 + 0x14);
  if (0 < iVar11) {
    iVar15 = 0;
    do {
      uVar8 = FUN_01314ee0(lVar6,iVar15,1);
      FUN_00e2fe78(&local_38,uVar8,*(undefined8 *)(param_1 + 0x208));
      local_58 = local_38;
      FUN_00e2eef0(*(undefined8 *)(param_1 + 0x38),&local_58);
      iVar15 = iVar15 + 1;
    } while (iVar11 != iVar15);
  }
  uVar9 = FUN_00dd6df8();
  if (((uVar9 & 1) != 0) || (uVar9 = FUN_00dd6e1c(), (uVar9 & 1) != 0)) {
    FUN_00e30180(*(undefined8 *)(param_1 + 0x38));
  }
  lVar6 = FUN_013179cc(param_2,DAT_019e7720,1);
  if (lVar6 == 0) goto LAB_01161bc4;
  puVar13 = *(undefined8 **)(param_1 + 0x18);
  if (puVar13 != (undefined8 *)0x0) {
    uVar9 = (ulong)*(uint *)((long)puVar13 + 0xc);
    if (0 < (int)*(uint *)((long)puVar13 + 0xc)) {
      do {
        lVar14 = *(long *)(**(long **)(param_1 + 0x18) + (uVar9 - 1) * 8);
        if (lVar14 != 0) {
          FUN_00e098f0(lVar14);
                    /* WARNING: Subroutine does not return */
          FUN_0148a700(lVar14 + 0x10);
        }
        bVar1 = 1 < uVar9;
        uVar9 = uVar9 - 1;
      } while (bVar1);
      puVar13 = *(undefined8 **)(param_1 + 0x18);
      if (puVar13 == (undefined8 *)0x0) goto LAB_01161e24;
    }
    if ((void *)*puVar13 != (void *)0x0) {
      operator_delete__((void *)*puVar13);
    }
    operator_delete(puVar13);
  }
LAB_01161e24:
  uVar2 = *(uint *)(lVar6 + 0x14);
  puVar13 = operator_new(0x10);
  uVar9 = (ulong)uVar2 << 3;
  puVar13[1] = 0;
  *(uint *)(puVar13 + 1) = uVar2;
  if ((int)uVar2 < 0) {
    uVar9 = 0xffffffffffffffff;
  }
  pvVar10 = operator_new__(uVar9);
  *puVar13 = pvVar10;
  *(undefined8 **)(param_1 + 0x18) = puVar13;
  if (0 < (int)uVar2) {
    uVar12 = 0;
    do {
      lVar14 = FUN_01314ee0(lVar6,uVar12,1);
      if (lVar14 != 0) {
        pvVar10 = operator_new(0x38);
        FUN_00e0983c();
        FUN_00e09b28(pvVar10,lVar14);
        plVar7 = *(long **)(param_1 + 0x18);
        iVar11 = *(int *)((long)plVar7 + 0xc);
        if (iVar11 == (int)plVar7[1]) {
          iVar15 = 5;
          if (iVar11 * 2 != 0) {
            iVar15 = iVar11 * 2;
          }
          if (iVar11 < iVar15) {
            FUN_01165cfc(plVar7);
            iVar11 = *(int *)((long)plVar7 + 0xc);
          }
        }
        *(int *)((long)plVar7 + 0xc) = iVar11 + 1;
        *(void **)(*plVar7 + (long)iVar11 * 8) = pvVar10;
      }
      uVar12 = uVar12 + 1;
    } while (uVar2 != uVar12);
  }
LAB_01161bc4:
  iVar11 = *(int *)(param_1 + 0x34);
  if (iVar11 == 0) {
    FUN_013145f8("stack empty",1);
    iVar11 = *(int *)(param_1 + 0x34);
  }
  cVar4 = *(char *)(*(long *)(param_1 + 0x28) + (long)iVar11 + -1);
  *(int *)(param_1 + 0x34) = (int)((long)iVar11 + -1);
  *(char *)(param_1 + 0x1e8) = cVar4;
  if (*(long *)(param_1 + 0xa8) != 0) {
    lVar6 = FUN_00df0584();
    *(char *)(lVar6 + 0x79) = cVar4;
  }
  plVar7 = *(long **)(param_1 + 0x218);
  if (plVar7 != (long *)0x0) {
    (**(code **)(*plVar7 + 0x50))(plVar7,cVar4 != '\0');
  }
  return;
}


// ===== 0xd2fe78 =====
// entry d2fe78

void FUN_00e2fe78(undefined8 param_1)

{
  undefined1 auStack_40 [16];
  
  FUN_0148a810(auStack_40,&DAT_003cd8cb);
  FUN_01317934(param_1,auStack_40,1);
                    /* WARNING: Subroutine does not return */
  FUN_0148a700(auStack_40);
}


// ===== 0xc8de8c =====
// entry c8de8c

void FUN_00d8de8c(long param_1,long param_2)

{
  undefined8 uVar1;
  long lVar2;
  
  if (param_2 == 0) {
    FUN_013145f8("replay data passed as NULL to game state manager",1);
  }
  if (*(long **)(param_1 + 0x78) != (long *)0x0) {
    (**(code **)(**(long **)(param_1 + 0x78) + 8))();
    *(undefined8 *)(param_1 + 0x78) = 0;
  }
  uVar1 = FUN_0131a648(param_2,0,100);
  *(undefined8 *)(param_1 + 0x78) = uVar1;
  if (param_2 != 0) {
                    /* WARNING: Subroutine does not return */
    FUN_0148a700(param_2);
  }
  FUN_013145f8("failed parsing replay json string",1);
  if ((((*(long *)(param_1 + 0x20) != 0) && (*(long *)(*(long *)(param_1 + 0x20) + 0x90) != 0)) &&
      (lVar2 = FUN_0116209c(), lVar2 != 0)) && (*(long *)(lVar2 + 0x2f8) != 0)) {
    FUN_01255c00(*(long *)(lVar2 + 0x2f8),*(undefined8 *)(param_1 + 0x78));
  }
  *(undefined4 *)(param_1 + 0x54) = 0xffffffff;
  *(undefined4 *)(param_1 + 0x34) = 4;
  *(undefined4 *)(param_1 + 0x10) = 3;
  return;
}


// ===== 0xad3da8 =====
// entry ad3da8

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined8 FUN_00bd3da8(undefined1 param_1 [16],undefined4 param_2,long param_3,long *param_4)

{
  long *plVar1;
  long *plVar2;
  bool bVar3;
  char cVar4;
  byte bVar5;
  byte bVar6;
  int iVar7;
  int iVar8;
  uint uVar9;
  undefined4 uVar10;
  undefined4 uVar11;
  void *pvVar12;
  long lVar13;
  void *pvVar14;
  void *pvVar15;
  long lVar16;
  undefined8 uVar17;
  long lVar18;
  undefined4 *puVar19;
  undefined8 uVar20;
  undefined8 *puVar21;
  long *plVar22;
  long lVar23;
  ulong uVar24;
  char *pcVar25;
  undefined1 uVar26;
  undefined8 *puVar27;
  undefined8 *puVar28;
  code *pcVar29;
  byte bVar30;
  long lVar31;
  undefined4 uVar32;
  undefined8 uVar33;
  undefined8 uVar34;
  undefined8 uVar35;
  undefined8 uVar36;
  void *local_270;
  long *local_268 [4];
  undefined1 auStack_248 [8];
  undefined1 auStack_240 [8];
  undefined1 auStack_238 [8];
  long *local_230;
  long *aplStack_228 [25];
  undefined8 local_160;
  undefined1 auStack_d8 [8];
  undefined1 auStack_d0 [8];
  long *local_c8;
  long *local_c0;
  long *local_b8;
  long *local_b0;
  long *local_a8;
  undefined1 auStack_a0 [8];
  long local_98 [2];
  undefined8 auStack_88 [2];
  undefined8 local_78;
  
  iVar7 = (**(code **)(*param_4 + 0x28))(param_4);
  plVar22 = DAT_019e5800;
  if (*(int *)(*DAT_019e5800 + 8) != 7) {
    if (iVar7 < 0x4f53) {
      if ((iVar7 != 0x4e87) && (iVar7 != 0x4e8c)) {
        return 1;
      }
    }
    else if (((iVar7 != 0x4f53) && (iVar7 != 0x74d5)) && (iVar7 != 0x51e3)) {
      return 1;
    }
  }
  FUN_00bdbeac(*DAT_019e5800,iVar7);
  plVar2 = DAT_019e59b8;
  plVar1 = DAT_019e5938;
  if (iVar7 < 0x62ee) {
    if (iVar7 < 0x569e) {
      if (iVar7 < 0x5295) {
        if (iVar7 < 0x5045) {
          if (iVar7 < 0x4ea1) {
            if (iVar7 < 0x4e87) {
              if (iVar7 == 0x2b5a) {
                lVar23 = FUN_00cbbc78();
                if (lVar23 != 0) {
                  FUN_00cbbc78();
                  FUN_00cbaec0();
                }
                goto LAB_00bd7aa0;
              }
              iVar8 = 0x4e22;
              goto LAB_00bd5118;
            }
            if (iVar7 == 0x4e87) {
              FUN_0148a810(&local_160,*(undefined8 *)PTR_PTR_019e6980);
              FUN_00be22bc(&local_160);
                    /* WARNING: Subroutine does not return */
              FUN_0148a700(&local_160);
            }
            if (iVar7 == 0x4e8c) {
              FUN_00bd9204(param_3);
              goto LAB_00bd7aa0;
            }
          }
          else if (iVar7 < 0x4f53) {
            if (iVar7 == 0x4ea1) {
              lVar23 = FUN_00984c30();
              if ((lVar23 != 0) && (lVar23 = FUN_00cbbc78(), lVar23 != 0)) {
                lVar13 = FUN_00cbaec0();
                if (lVar13 != 0) {
                  if (*(char *)(lVar13 + 0x3bc) != '\0') {
                    uVar20 = FUN_00984c30();
                    FUN_0148a810(&local_160,"TID_2V2_MATCHMAKE_CANCELLED_CO_MATE");
                    uVar17 = FUN_00d994d8(&local_160);
                    FUN_00985bb0(0,uVar20,uVar17,0xffffffff);
                    /* WARNING: Subroutine does not return */
                    FUN_0148a700(&local_160);
                  }
                  if ((*(long *)(lVar13 + 0x1b8) != 0) &&
                     (uVar24 = FUN_00b94790(*(long *)(lVar13 + 0x1b8)), (uVar24 & 1) != 0)) {
                    uVar20 = FUN_00984c30();
                    FUN_0148a810(&local_160,"TID_BATTLE_CANCELLED");
                    uVar17 = FUN_00d994d8(&local_160);
                    FUN_00985bb0(0,uVar20,uVar17,0xffffffff);
                    /* WARNING: Subroutine does not return */
                    FUN_0148a700(&local_160);
                  }
                  FUN_00cd978c(lVar13);
                  uVar20 = FUN_00d8c5b8();
                  FUN_00d8e1bc(uVar20,0);
                }
                lVar13 = FUN_00cbaeac(lVar23);
                if ((lVar13 != 0) && (lVar13 = FUN_00cbaeac(lVar23), *(long *)(lVar13 + 0x1a8) != 0)
                   ) {
                  lVar23 = FUN_00cbaeac(lVar23);
                  *(undefined1 *)(*(long *)(lVar23 + 0x1a8) + 0x158) = 1;
                }
              }
              goto LAB_00bd7aa0;
            }
            if (iVar7 == 0x4ee0) {
              lVar23 = FUN_00984c30();
              if (lVar23 == 0) goto LAB_00bd7aa0;
              lVar13 = FUN_009919f4();
              if (lVar13 == 0) {
                pvVar15 = operator_new(0x250);
                FUN_0092d374(pvVar15,0);
                lVar13 = FUN_009855cc(lVar23,pvVar15,1,0,0,0,0);
                if (lVar13 == 0) goto LAB_00bd7aa0;
              }
              pvVar15 = operator_new(0x110);
              FUN_0092ceac(pvVar15,param_4 + 0x12);
              goto LAB_00bd4f28;
            }
          }
          else {
            if (iVar7 == 0x4f53) {
              FUN_00d8c5b8();
              lVar23 = FUN_00d8de84();
              iVar7 = (int)param_4[0x12];
              if (iVar7 == 10) {
                uVar20 = FUN_00855818();
                FUN_0148a42c(&local_160);
                FUN_0148a42c(&local_78);
                FUN_0148a42c(auStack_88);
                uVar32 = 6;
              }
              else if (iVar7 == 5) {
                uVar20 = FUN_00855818();
                FUN_0148a42c(&local_160);
                FUN_0148a42c(&local_78);
                FUN_0148a42c(auStack_88);
                uVar32 = 0xc;
              }
              else {
                if (iVar7 == 4) {
                  if (lVar23 != 0) {
                    FUN_00d91458(lVar23,4);
                  }
                  goto LAB_00bd7aa0;
                }
                uVar20 = FUN_00855818();
                FUN_0148a42c(&local_160);
                FUN_0148a42c(&local_78);
                FUN_0148a42c(auStack_88);
                uVar32 = 8;
              }
LAB_00bd7178:
              puVar28 = auStack_88;
              puVar27 = &local_78;
              puVar21 = &local_160;
              uVar26 = 0;
              goto LAB_00bd717c;
            }
            if (iVar7 == 0x4fe2) goto LAB_00bd5760;
          }
        }
        else if (iVar7 < 0x51cd) {
          if (iVar7 < 0x50d4) {
            if (iVar7 == 0x5045) {
              lVar23 = FUN_009a51a0();
              if (lVar23 != 0) {
                lVar23 = FUN_009a51a0();
                uVar20 = *(undefined8 *)(lVar23 + 0x50);
                FUN_01137edc(&local_160,param_4);
                if (local_160 != (long *)0x0) {
                  local_c8 = local_160;
                  FUN_00a35e58(uVar20,&local_c8);
                }
              }
              goto LAB_00bd7aa0;
            }
            if (iVar7 == 0x505c) {
              FUN_01135a68(&local_160,param_4);
              lVar23 = FUN_00d85c28();
              if (lVar23 != 0) {
                FUN_00d85c28();
                lVar23 = FUN_00d89ae4();
                if (lVar23 != 0) {
                  FUN_00d85c28();
                  FUN_00d89ae4();
                  uVar20 = FUN_00cbaeac();
                  local_b8 = local_160;
                  FUN_00d315ac(uVar20,&local_b8);
                  goto LAB_00bd7aa0;
                }
              }
              if (local_160 != (long *)0x0) {
                FUN_00bd9270();
                plVar22 = local_160;
                if (local_160 != (long *)0x0) {
                  if ((void *)*local_160 != (void *)0x0) {
                    operator_delete__((void *)*local_160);
                  }
                  operator_delete(plVar22);
                }
              }
              goto LAB_00bd7aa0;
            }
          }
          else {
            if (iVar7 == 0x50d4) {
              FUN_0148a890(&local_160,param_4 + 0x12);
              uVar24 = FUN_0148ce14(&local_160,&DAT_00421ecf);
              if ((uVar24 & 1) != 0) {
                uVar20 = FUN_00d994d8(&local_160);
                FUN_0148acf4(&local_160,uVar20);
              }
              FUN_0148a890(&local_78,param_4 + 0x14);
              uVar24 = FUN_0148ce14(&local_78,&DAT_00421ecf);
              if ((uVar24 & 1) != 0) {
                uVar20 = FUN_00d994d8(&local_78);
                FUN_0148acf4(&local_78,uVar20);
              }
              uVar20 = FUN_00855818();
              uVar26 = (undefined1)param_4[0x16];
              FUN_0148a810(auStack_88,&DAT_003d4b92);
              puVar21 = &local_78;
              puVar27 = auStack_88;
              puVar28 = &local_160;
              uVar32 = 0x15;
LAB_00bd717c:
              FUN_0085a6dc(uVar20,uVar32,uVar26,puVar21,puVar27,puVar28);
LAB_00bd7180:
                    /* WARNING: Subroutine does not return */
              FUN_0148a700(auStack_88);
            }
            if (iVar7 == 0x516a) {
              uVar20 = FUN_00984c30();
              uVar32 = FUN_00985b40();
              local_160 = (long *)CONCAT44(param_2,uVar32);
              FUN_00985c3c(0,uVar20,&local_160,param_4 + 0x13,0xffffffff);
              goto LAB_00bd7aa0;
            }
          }
        }
        else if (iVar7 < 0x527d) {
          if (iVar7 == 0x51cd) {
            lVar23 = FUN_00d85c28();
            if (lVar23 != 0) {
              uVar20 = FUN_00d89b20(lVar23,param_4);
              return uVar20;
            }
            goto LAB_00bd7aa0;
          }
          if (iVar7 == 0x51e3) {
            FUN_0148a810(&local_160,*(undefined8 *)PTR_PTR_019e6980);
            FUN_00be22bc(&local_160);
                    /* WARNING: Subroutine does not return */
            FUN_0148a700(&local_160);
          }
        }
        else {
          if (iVar7 == 0x527d) {
            FUN_0090031c();
            uVar24 = FUN_00ed04dc();
            if ((uVar24 & 1) != 0) {
              FUN_0090031c();
              lVar23 = FUN_00eccc70();
              if (*(char *)(lVar23 + 0x389) != '\0') {
                FUN_00bc90b0((int)param_4[0x12]);
                FUN_00dd4b28((int)param_4[0x12]);
              }
            }
            goto LAB_00bd7aa0;
          }
          if (iVar7 == 0x5286) {
            lVar23 = param_4[0x13];
            plVar22 = (long *)param_4[0x14];
            if ((plVar22 != (long *)0x0) && (0 < *(int *)((long)plVar22 + 0xc))) {
              lVar13 = 0;
              do {
                uVar20 = FUN_009a51a0();
                FUN_009a7208(uVar20,lVar23,*(undefined8 *)(*plVar22 + lVar13 * 8));
                lVar13 = lVar13 + 1;
              } while (lVar13 < *(int *)((long)plVar22 + 0xc));
            }
            FUN_00e26c5c(&local_160,param_4);
            if (local_160 != (long *)0x0) {
              uVar20 = FUN_009a51a0();
              local_a8 = local_160;
              FUN_009a7280(uVar20,lVar23,&local_a8);
            }
            goto LAB_00bd7aa0;
          }
        }
      }
      else if (iVar7 < 0x54b5) {
        if (iVar7 < 0x5391) {
          if (iVar7 < 0x52c2) {
            if (iVar7 == 0x5295) {
              FUN_00d8c5b8();
              FUN_00d8e1e4();
              lVar23 = FUN_00984c30();
              if (lVar23 != 0) {
                uVar20 = FUN_00984c30();
                uVar32 = FUN_00985b40();
                local_78 = (long *)CONCAT44(param_2,uVar32);
                FUN_0148a810(&local_160,"TID_SHUTDOWN_STARTED");
                uVar17 = FUN_00d994d8(&local_160);
                FUN_00985c3c(0,uVar20,&local_78,uVar17,0xffffffff);
                goto LAB_00bd7a9c;
              }
              goto LAB_00bd7aa0;
            }
            iVar8 = 0x52aa;
            goto LAB_00bd560c;
          }
          if (iVar7 == 0x52c2) {
            lVar23 = FUN_00984c30();
            if (lVar23 == 0) goto LAB_00bd7aa0;
            FUN_0148a42c(&local_160);
            uVar9 = (int)param_4[0x12] - 1;
            if (uVar9 < 9) {
              pcVar25 = &DAT_00440d40 + *(int *)(&DAT_00440d40 + (ulong)uVar9 * 4);
            }
            else {
              pcVar25 = "TID_CLAN_JOIN_FAILED";
            }
            FUN_0148abd4(&local_160,pcVar25);
LAB_00bd6f7c:
            uVar20 = FUN_00d994d8(&local_160);
            FUN_00985bb0(0,lVar23,uVar20,0xffffffff);
            goto LAB_00bd7a9c;
          }
          if (iVar7 == 0x52f8) {
            lVar23 = FUN_00d85c28();
            if ((lVar23 != 0) && (lVar23 = FUN_00984c30(), lVar23 != 0)) {
              FUN_00d85c28();
              lVar23 = FUN_00d89af8();
              if (lVar23 == 0) {
                FUN_00d85c28();
                lVar23 = FUN_00d89ae4();
                if (lVar23 != 0) {
                  FUN_00d85c28();
                  FUN_00d89ae4();
                  lVar23 = FUN_00cbaec0();
                  if (lVar23 != 0) {
                    FUN_00d85c28();
                    FUN_00d89ae4();
                    FUN_00cbaec0();
                    uVar24 = FUN_00cd99ac();
                    if ((uVar24 & 1) != 0) goto LAB_00bd7aa0;
                  }
                }
                FUN_0148a810(&local_160,"TID_PUSH_MATCHMAKING_TOP");
                FUN_00d994d8(&local_160);
                    /* WARNING: Subroutine does not return */
                FUN_0148a700(&local_160);
              }
            }
            goto LAB_00bd7aa0;
          }
        }
        else {
          if (0x5444 < iVar7) {
            if (iVar7 == 0x5445) {
              lVar23 = FUN_00d8c5b8();
              if (lVar23 != 0) {
                lVar23 = param_4[0x12];
                uVar20 = FUN_00d8c5b8();
                if ((int)lVar23 < 1) {
                  FUN_0148a810(&local_160,&DAT_003d4b92);
                  FUN_00d8e3d4(uVar20,0,0,&local_160);
                    /* WARNING: Subroutine does not return */
                  FUN_0148a700(&local_160);
                }
                FUN_00d8e3d4(uVar20,(int)param_4[0x12] + 5,*(undefined4 *)((long)param_4 + 0x94),
                             param_4 + 0x13);
                uVar20 = FUN_00d8c5b8();
                FUN_00d8e404(uVar20,1,1);
                FUN_00d8c5b8();
                uVar24 = FUN_00d8e5b0();
                if (((uVar24 & 1) != 0) && (lVar23 = FUN_009a51a0(), lVar23 != 0)) {
                  lVar23 = FUN_009a51a0();
                  FUN_00a366d0(*(undefined8 *)(lVar23 + 0x50));
                }
                lVar23 = FUN_00855818();
                uVar20 = *(undefined8 *)(lVar23 + 0x510);
                FUN_00d8c5b8();
                uVar9 = FUN_00d8e594();
                FUN_00dd4b10(uVar20,uVar9 & 1);
              }
              goto LAB_00bd7aa0;
            }
            if (iVar7 != 0x547f) goto LAB_00bd6b1c;
            if (*(long **)(param_3 + 0x98) != (long *)0x0) {
              (**(code **)(**(long **)(param_3 + 0x98) + 8))();
            }
            *(undefined8 *)(param_3 + 0x98) = 0;
            FUN_01160810(&local_160,param_4);
            if (local_160 == (long *)0x0) goto LAB_00bd7aa0;
            lVar23 = param_4[0x12];
            pvVar15 = (void *)FUN_0116066c(param_4);
            pvVar14 = operator_new(0x6b0);
            uVar20 = FUN_01337d0c();
            FUN_01431c24(pvVar14,uVar20);
            *(void **)(param_3 + 0x98) = pvVar14;
            pvVar14 = operator_new(0x20);
            uVar20 = FUN_00900294();
            FUN_01126818(pvVar14,uVar20,1);
            FUN_01432974(*(undefined8 *)(param_3 + 0x98),pvVar14);
            uVar20 = FUN_009002d8();
            FUN_009006dc(uVar20,(long)pvVar14 + 8);
            uVar24 = FUN_01431f2c(*(undefined8 *)(param_3 + 0x98),local_160,(int)lVar23,pvVar15,
                                  DAT_019e6990,param_4 + 0x16);
            plVar22 = *(long **)(param_3 + 0x98);
            if ((uVar24 & 1) == 0) {
              if (plVar22 != (long *)0x0) {
                lVar23 = *plVar22;
LAB_00bd77c4:
                (**(code **)(lVar23 + 8))(plVar22);
              }
LAB_00bd77cc:
              *(undefined8 *)(param_3 + 0x98) = 0;
            }
            else {
              pvVar14 = operator_new(0x90);
              FUN_011605ac();
              FUN_00855818();
              FUN_00855860();
              uVar24 = FUN_014329dc(plVar22,pvVar14);
              if ((uVar24 & 1) == 0) {
                plVar22 = *(long **)(param_3 + 0x98);
                if (plVar22 != (long *)0x0) {
                  lVar23 = *plVar22;
                  goto LAB_00bd77c4;
                }
                goto LAB_00bd77cc;
              }
              FUN_0090031c();
              lVar23 = FUN_00eccc70();
              if (lVar23 != 0) {
                FUN_014329ec((float)*(int *)(lVar23 + 0x194),*(undefined8 *)(param_3 + 0x98));
                FUN_014329f4((float)*(int *)(lVar23 + 400) * 0.001,*(undefined8 *)(param_3 + 0x98));
              }
            }
            if (local_160 != (long *)0x0) {
                    /* WARNING: Subroutine does not return */
              FUN_0148a700(local_160);
            }
            local_160 = (long *)0x0;
            if (pvVar15 != (void *)0x0) {
              operator_delete__(pvVar15);
            }
            goto LAB_00bd7aa0;
          }
          if (iVar7 == 0x5391) goto LAB_00bd5614;
          if (iVar7 == 0x5417) {
            lVar23 = FUN_00984c30();
            if (lVar23 != 0) {
              FUN_0148a810(&local_78,"TID_SPELL_FACTORY_CLAIM_FAILED_TITLE");
              uVar20 = FUN_00d994d8(&local_78);
              FUN_0148a890(&local_160,uVar20);
                    /* WARNING: Subroutine does not return */
              FUN_0148a700(&local_78);
            }
            goto LAB_00bd7aa0;
          }
        }
      }
      else if (iVar7 < 0x5564) {
        if (iVar7 < 0x553a) {
          if (iVar7 == 0x54b5) {
            lVar23 = FUN_00984c30();
            if ((lVar23 == 0) || (plVar22 = (long *)FUN_00991a18(), plVar22 == (long *)0x0))
            goto LAB_00bd7aa0;
            if ((char)param_4[0x12] == '\0') {
              FUN_0113f2ac(&local_160,param_4);
              (**(code **)(*plVar22 + 8))(plVar22,local_160);
              if (local_160 != (long *)0x0) {
                    /* WARNING: Subroutine does not return */
                FUN_0148a700(local_160);
              }
              goto LAB_00bd7aa0;
            }
            iVar7 = *(int *)((long)param_4 + 0x94);
            if (iVar7 < 4) {
              if (iVar7 == 1) goto LAB_00bd7d1c;
              if (iVar7 == 2) {
                pcVar25 = "TID_NAME_TOO_SHORT";
              }
              else {
                if (iVar7 != 3) goto LAB_00bd7aa0;
                pcVar25 = "TID_NAME_ALREADY_CHANGED";
              }
            }
            else {
              if (iVar7 < 6) {
                if (iVar7 != 4) {
                  if (iVar7 == 5) {
                    FUN_0090031c();
                    lVar23 = FUN_00eccc70();
                    FUN_0148d1e4(&local_160,&DAT_00410d61,*(undefined4 *)(lVar23 + 0x1c4));
                    FUN_0148a810(auStack_88,"TID_NAME_CHANGE_TH_LEVEL_TOO_LOW");
                    uVar20 = FUN_00d994d8(auStack_88);
                    FUN_0148a810(local_98,"<level>");
                    FUN_0148c2e4(&local_78,uVar20,local_98,&local_160);
                    (**(code **)*plVar22)(plVar22,&local_78);
                    /* WARNING: Subroutine does not return */
                    FUN_0148a700(&local_78);
                  }
                  goto LAB_00bd7aa0;
                }
              }
              else {
                if (iVar7 == 6) {
                  pcVar25 = "TID_SERVER_INTERNAL_ERROR";
                  goto LAB_00bd7f00;
                }
                if (iVar7 != 0xb) goto LAB_00bd7aa0;
              }
LAB_00bd7d1c:
              pcVar25 = "TID_INVALID_NAME";
            }
LAB_00bd7f00:
            FUN_0148a810(&local_160,pcVar25);
            uVar20 = FUN_00d994d8(&local_160);
            (**(code **)*plVar22)(plVar22,uVar20);
            goto LAB_00bd7a9c;
          }
          if (iVar7 == 0x552b) {
            lVar23 = FUN_00984c30();
            if (lVar23 == 0) goto LAB_00bd7aa0;
            if ((int)param_4[0x12] == 2) {
              pcVar25 = "TID_FILTERING_SERVICE_OUTAGE";
            }
            else {
              if ((int)param_4[0x12] != 1) goto LAB_00bd7aa0;
              pcVar25 = "TID_YOUNG_PLAYER_CANNOT_SEND_CHAT_MESSAGE";
            }
            goto LAB_00bd7554;
          }
        }
        else {
          if (iVar7 == 0x553a) {
            lVar23 = FUN_00984c30();
            pcVar25 = "TID_SPECTATE_FAILED_DUE_TO_PRIVACY";
            goto LAB_00bd7554;
          }
          if (iVar7 == 0x554b) {
            thunk_FUN_01500938((char)param_4[0x12],*(undefined4 *)((long)param_4 + 0x94),0);
            *DAT_019e6608 = (char)param_4[0x13];
            goto LAB_00bd7aa0;
          }
        }
      }
      else if (iVar7 < 0x55f4) {
        if (iVar7 == 0x5564) {
          lVar23 = FUN_009a51a0();
          if ((lVar23 != 0) && (FUN_01137a14(&local_160,param_4), local_160 != (long *)0x0)) {
            lVar23 = FUN_009a51a0();
            thunk_FUN_00a35edc(*(undefined8 *)(lVar23 + 0x50),local_160);
          }
          goto LAB_00bd7aa0;
        }
        if (iVar7 == 0x558f) {
          lVar23 = FUN_00984c30();
          if ((lVar23 == 0) || (lVar23 = FUN_009919f4(), lVar23 == 0)) goto LAB_00bd7aa0;
          iVar7 = *(int *)((long)param_4 + 0x94);
          lVar13 = param_4[0x12];
          if (iVar7 < 2) {
            if (iVar7 == 0) {
              pvVar15 = operator_new(0x120);
              uVar20 = 100;
            }
            else {
              if (iVar7 != 1) goto LAB_00bd7aa0;
              pvVar15 = operator_new(0x120);
              uVar20 = 0x65;
            }
          }
          else if (iVar7 == 2) {
            pvVar15 = operator_new(0x120);
            uVar20 = 0x66;
          }
          else {
            if (iVar7 != 3) goto LAB_00bd7aa0;
            pvVar15 = operator_new(0x120);
            uVar20 = 0x67;
          }
          FUN_0092bf14(pvVar15,uVar20,(char)lVar13 != '\0');
LAB_00bd7cfc:
          FUN_0092d998(lVar23,pvVar15);
          goto LAB_00bd7aa0;
        }
      }
      else {
        if (iVar7 == 0x55f4) {
          iVar7 = (int)param_4[0x13];
          if (iVar7 == 4) {
            uVar20 = FUN_0148a810(&local_160,"TID_INVITE_FRIEND_TOKEN_ERROR_TOO_MANY");
            FUN_00bd92e0(uVar20,&local_160);
                    /* WARNING: Subroutine does not return */
            FUN_0148a700(&local_160);
          }
          if (iVar7 == 3) {
            uVar20 = FUN_0148a810(&local_160,"TID_INVITE_FRIEND_TOKEN_ERROR_FRIENDS_FULL");
            FUN_00bd92e0(uVar20,&local_160);
                    /* WARNING: Subroutine does not return */
            FUN_0148a700(&local_160);
          }
          if (iVar7 != 1) {
            uVar20 = FUN_0148a810(&local_160,"TID_INVITE_FRIEND_TOKEN_ERROR");
            FUN_00bd92e0(uVar20,&local_160);
                    /* WARNING: Subroutine does not return */
            FUN_0148a700(&local_160);
          }
          lVar23 = *DAT_019e59b8;
          if (lVar23 != 0) {
            FUN_0114a6f0(auStack_248,param_4);
            FUN_00d78cdc(lVar23,auStack_248);
          }
          goto LAB_00bd7aa0;
        }
        if (iVar7 == 0x561e) goto LAB_00bd5614;
        if (iVar7 == 0x5668) {
          if ((long *)*DAT_019e58e8 != (long *)0x0) {
            (**(code **)(*(long *)*DAT_019e58e8 + 200))();
          }
          goto LAB_00bd7aa0;
        }
      }
    }
    else if (iVar7 < 0x5c78) {
      if (iVar7 < 0x5a69) {
        if (iVar7 < 0x5884) {
          if (0x5754 < iVar7) {
            if (iVar7 == 0x5755) goto LAB_00bd5460;
            iVar8 = 0x57fa;
            goto LAB_00bd560c;
          }
          if (iVar7 == 0x569e) {
            lVar23 = FUN_009a51a0();
            if (lVar23 != 0) {
              uVar20 = FUN_009a51a0();
              FUN_009a5a40(uVar20,param_4 + 0x12);
            }
            goto LAB_00bd7aa0;
          }
          if (iVar7 == 0x5723) {
            iVar7 = (int)param_4[0x12];
            if (iVar7 == 8) {
              pcVar25 = "TID_ADD_FRIEND_FAILED_THEIR_LIST_FULL";
            }
            else if (iVar7 == 7) {
              pcVar25 = "TID_ADD_FRIEND_FAILED_YOUR_LIST_FULL";
            }
            else if (iVar7 == 3) {
              pcVar25 = "TID_ADD_FRIEND_FAILED_BANNED";
            }
            else {
              pcVar25 = "TID_ADD_FRIEND_FAILED";
            }
            uVar20 = FUN_0148a810(&local_160,pcVar25);
            FUN_00bd92e0(uVar20,&local_160);
                    /* WARNING: Subroutine does not return */
            FUN_0148a700(&local_160);
          }
        }
        else if (iVar7 < 0x599b) {
          if (iVar7 == 0x5884) {
            if ((int)param_4[0x12] == 3) {
              (**(code **)(*(long *)*DAT_019e58e8 + 0xe0))((long *)*DAT_019e58e8,DAT_019e69a0);
            }
            else if ((int)param_4[0x12] == 1) {
              uVar20 = FUN_00855818();
              FUN_0148a42c(&local_160);
              FUN_0148a42c(&local_78);
              FUN_0148a42c(auStack_88);
              uVar17 = 0xd;
              goto LAB_00bd6f04;
            }
            goto LAB_00bd7aa0;
          }
          if (iVar7 == 0x595f) {
            iVar7 = FUN_01130280(param_4);
            uVar20 = _DAT_01a97d48;
            uVar17 = FUN_01130278(param_4);
            FUN_00cae814(uVar20,uVar17,iVar7 == 0,iVar7 == 2);
            goto LAB_00bd7aa0;
          }
        }
        else {
          if (iVar7 == 0x599b) {
LAB_00bd5744:
            if (*DAT_019e62e0 != 0) {
              FUN_00bdefa8(*DAT_019e62e0,param_4);
            }
            goto LAB_00bd7aa0;
          }
          if (iVar7 == 0x59fa) {
            lVar23 = FUN_00984c30();
            if ((lVar23 == 0) || (lVar23 = FUN_00cb6508(), lVar23 == 0)) goto LAB_00bd7aa0;
            FUN_0115582c(&local_160,param_4);
            FUN_00cb6508();
            lVar23 = FUN_00cb5180();
            uVar20 = FUN_00cb6508();
            if (lVar23 == 0) {
              FUN_00cb54ac(uVar20,local_160);
            }
            else {
              uVar20 = FUN_00cb5180();
              FUN_009bcd18(uVar20,local_160);
            }
            goto joined_r0x00bd58a0;
          }
        }
      }
      else if (iVar7 < 0x5aed) {
        if (iVar7 < 0x5acd) {
          if (iVar7 == 0x5a69) {
            lVar23 = FUN_00984c30();
            if (lVar23 != 0) {
              lVar23 = FUN_00cbbc78();
              if ((lVar23 != 0) && (lVar13 = FUN_00cbaec0(), lVar13 != 0)) {
                FUN_00cbaec0(lVar23);
                FUN_00cd978c();
              }
              lVar23 = FUN_009a51a0();
              if (lVar23 != 0) {
                uVar20 = FUN_009a51a0();
                FUN_009a6af4(uVar20,0,0,0);
              }
            }
            goto LAB_00bd7aa0;
          }
          if (iVar7 == 0x5ac9) {
            FUN_00d78758(*DAT_019e59b8,param_4);
            lVar23 = FUN_009a51a0();
            if (lVar23 != 0) {
              lVar13 = param_4[0x13];
              lVar23 = param_4[0x12];
              uVar20 = FUN_009a51a0();
              FUN_009a5b98(uVar20,lVar13,(int)lVar23,1);
            }
            goto LAB_00bd7aa0;
          }
        }
        else {
          if (iVar7 == 0x5acd) {
            FUN_01152b68(&local_160,param_4);
            FUN_01152b78(&local_78,param_4);
            if (local_78 != (long *)0x0) {
              lVar23 = FUN_00855818();
              uVar17 = *(undefined8 *)(lVar23 + 0x510);
              uVar20 = FUN_00ddf840(local_78);
              FUN_00dd4b18(uVar17,uVar20);
            }
            FUN_00cdb09c((int)param_4[0x16]);
            lVar23 = FUN_00d8c5b8();
            *(undefined4 *)(lVar23 + 8) = 3;
            uVar20 = FUN_00d8c5b8();
            lVar23 = param_4[0x15];
            iVar7 = *(int *)((long)param_4 + 0xac);
            uVar32 = *(undefined4 *)((long)param_4 + 0xb4);
            lVar13 = param_4[0x17];
            FUN_00cdb074((int)lVar13 - iVar7);
            local_230 = local_78;
            aplStack_228[0] = local_160;
            FUN_00d8e054(uVar20,aplStack_228,&local_230,0,1,(int)lVar23,iVar7,uVar32,(int)lVar13);
            goto LAB_00bd7aa0;
          }
          if (iVar7 == 0x5ae2) {
            lVar23 = FUN_009a51a0();
            if (lVar23 == 0) goto LAB_00bd7aa0;
            uVar20 = FUN_009a51a0();
            FUN_01134e10(auStack_d8,param_4);
            FUN_009a59f4(uVar20,auStack_d8);
            FUN_01134e20(&local_160,param_4);
            uVar20 = FUN_009a51a0();
            FUN_009a7dd4(uVar20,local_160,(int)param_4[0x14]);
joined_r0x00bd58a0:
            if (local_160 != (long *)0x0) {
              operator_delete(local_160);
            }
            goto LAB_00bd7aa0;
          }
        }
      }
      else if (iVar7 < 0x5c28) {
        if (iVar7 == 0x5aed) {
          lVar23 = FUN_00984c30();
          if ((lVar23 == 0) || (lVar23 = FUN_009919f4(), lVar23 == 0)) goto LAB_00bd7aa0;
          plVar22 = (long *)FUN_0092d7f0();
          iVar7 = (**(code **)(*plVar22 + 0x1a8))();
          if (iVar7 != 5) goto LAB_00bd7aa0;
          iVar7 = *(int *)((long)param_4 + 0xac);
          if (iVar7 == 2) {
LAB_00bd8fb4:
            pvVar15 = operator_new(0x120);
            uVar20 = 1;
            goto LAB_00bd5eac;
          }
          if (iVar7 == 1) {
LAB_00bd8fc8:
            pvVar15 = operator_new(0x120);
            uVar20 = 2;
            goto LAB_00bd5eac;
          }
          if (iVar7 != 0) goto LAB_00bd7aa0;
          pvVar15 = operator_new(0x128);
          FUN_0092b164(pvVar15,param_4 + 0x13,(int)param_4[0x15]);
          goto LAB_00bd5eb4;
        }
        if (iVar7 == 0x5b8b) {
          lVar23 = FUN_00984c30();
          if ((lVar23 != 0) && (lVar23 = FUN_009860d0(lVar23,0xb,1), lVar23 != 0)) {
            FUN_00b3cfd8(lVar23,param_4 + 0x12,(int)param_4[0x14]);
          }
          goto LAB_00bd7aa0;
        }
      }
      else {
        if (iVar7 == 0x5c28) {
          lVar23 = FUN_00cbbc78();
          if (lVar23 != 0) {
            lVar16 = FUN_00cbaec0();
            lVar13 = _DAT_01a97d28;
            if ((((_DAT_01a97d28 != 0) && (uVar24 = FUN_00d7d6e4(_DAT_01a97d28), (uVar24 & 1) != 0))
                && (uVar24 = FUN_00d7d274(lVar13), (uVar24 & 1) == 0)) &&
               (uVar24 = FUN_00cd99ac(lVar16), (uVar24 & 1) == 0)) {
              FUN_00cda93c(lVar16);
            }
            if ((lVar16 != 0) && (uVar24 = FUN_00cd99ac(lVar16), (uVar24 & 1) != 0)) {
              FUN_00cda918(lVar16,(int)param_4[0x12]);
              FUN_00cda930(lVar16,(int)param_4[0x13],*(undefined4 *)((long)param_4 + 0x94));
            }
            lVar13 = FUN_00cbaeac(lVar23);
            if ((lVar13 != 0) && (lVar13 = FUN_00cbaeac(lVar23), *(long *)(lVar13 + 0x1a8) != 0)) {
              lVar23 = FUN_00cbaeac(lVar23);
              FUN_00a52b88(*(undefined8 *)(lVar23 + 0x1a8),(int)param_4[0x12]);
            }
          }
          goto LAB_00bd7aa0;
        }
        if (iVar7 == 0x5c3d) {
          iVar7 = (int)param_4[0x14];
          if (iVar7 < 4) {
            if (iVar7 == 0) {
              lVar23 = *DAT_019e59b8;
              if (lVar23 != 0) {
                FUN_0114a250(auStack_240,param_4);
                FUN_00d76454(lVar23,auStack_240);
              }
            }
            else {
              if (iVar7 != 2) goto LAB_00bd7aa0;
              lVar23 = *DAT_019e5938;
              if (lVar23 != 0) {
                FUN_0114a250(auStack_238,param_4);
                FUN_00d7a4f8(lVar23,auStack_238);
              }
            }
          }
          else if (iVar7 == 4) {
            FUN_0148e7f4("FriendListMessage with clan list type received!\n");
          }
          else {
            if (iVar7 != 5) goto LAB_00bd7aa0;
            if (*DAT_019e58e8 != 0) {
              FUN_00ca7178(*DAT_019e58e8,param_4);
            }
          }
          FUN_00d781ec(*DAT_019e59b8);
          goto LAB_00bd7aa0;
        }
      }
    }
    else if (iVar7 < 0x5f19) {
      if (iVar7 < 0x5d45) {
        if (0x5c9c < iVar7) {
          if (iVar7 == 0x5c9d) {
            lVar23 = FUN_009a51a0();
            if (lVar23 != 0) {
              FUN_00a364e4(*(undefined8 *)(lVar23 + 0x50),param_4[0x12]);
            }
            goto LAB_00bd7aa0;
          }
          iVar8 = 0x5d0c;
          goto LAB_00bd5118;
        }
        if (iVar7 == 0x5c78) goto LAB_00bd7aa0;
        if (iVar7 == 0x5c82) {
          lVar13 = FUN_00984c30();
          if ((((lVar13 == 0) || (lVar23 = FUN_009919f4(), lVar23 == 0)) ||
              (plVar22 = (long *)FUN_00985a10(lVar13), plVar22 == (long *)0x0)) ||
             ((iVar7 = (**(code **)(*plVar22 + 0x268))(), iVar7 != 0x1b ||
              (lVar13 = FUN_0092d7f0(lVar23), lVar13 == 0)))) goto LAB_00bd7aa0;
          plVar22 = (long *)FUN_0092d7f0(lVar23);
          iVar7 = (**(code **)(*plVar22 + 0x1a8))();
          if (iVar7 != 1) {
            plVar22 = (long *)FUN_0092d7f0(lVar23);
            iVar7 = (**(code **)(*plVar22 + 0x1a8))();
            if (iVar7 != 2) goto LAB_00bd7aa0;
          }
          plVar22 = (long *)FUN_0092d7f0(lVar23);
          iVar7 = (**(code **)(*plVar22 + 0x1a8))();
          FUN_0092d7f0(lVar23);
          if (iVar7 == 2) {
            uVar20 = FUN_0092b974();
          }
          else {
            uVar20 = FUN_0092bec4();
          }
          uVar24 = FUN_0148b21c(uVar20,param_4 + 0x13);
          if ((uVar24 & 1) != 0) {
            iVar7 = (int)param_4[0x15];
            if (iVar7 < 4) {
              if (iVar7 == 0) {
                uVar32 = *(undefined4 *)((long)param_4 + 0xac);
                uVar24 = FUN_0131a9dc(param_4 + 0x18);
                if (((uVar24 & 1) != 0) || (*(int *)((long)param_4 + 0xcc) == 0)) goto LAB_00bd7aa0;
                pvVar15 = operator_new(0x150);
                uVar10 = FUN_0131aa88(param_4 + 0x18);
                uVar11 = FUN_0131aa90(param_4 + 0x18);
                FUN_0092b594(pvVar15,param_4 + 0x13,uVar32,param_4 + 0x16,param_4 + 0x19,uVar10,
                             uVar11);
                goto LAB_00bd7cfc;
              }
              if (iVar7 == 1) goto LAB_00bd8fc8;
              if (iVar7 != 2) goto LAB_00bd7aa0;
              goto LAB_00bd8fb4;
            }
            if (5 < iVar7) {
              if (iVar7 == 6) {
                pvVar15 = operator_new(0x120);
                uVar20 = 9;
              }
              else {
                if (iVar7 != 7) goto LAB_00bd7aa0;
                pvVar15 = operator_new(0x120);
                uVar20 = 8;
              }
              goto LAB_00bd5eac;
            }
            if (iVar7 != 4) {
              if (iVar7 != 5) goto LAB_00bd7aa0;
              pvVar15 = operator_new(0x120);
              uVar20 = 5;
              uVar17 = 1;
              goto LAB_00bd5eb0;
            }
          }
          pvVar15 = operator_new(0x120);
          uVar20 = 4;
          uVar17 = 1;
          goto LAB_00bd5eb0;
        }
      }
      else if (iVar7 < 0x5e01) {
        if (iVar7 == 0x5d45) goto LAB_00bd542c;
        if (iVar7 == 0x5d69) {
          lVar23 = FUN_00984c30();
          if (lVar23 == 0) goto LAB_00bd7aa0;
          iVar7 = (int)param_4[0x12];
          if (iVar7 < 9) {
            if (iVar7 == 3) {
              uVar20 = FUN_00984c30();
              pcVar25 = "TID_TOURNAMENT_MATCHMAKE_FAILED_PROCESSING";
              goto LAB_00bd7d30;
            }
            if (iVar7 == 4) {
              uVar20 = FUN_00984c30();
              pcVar25 = "TID_TOURNAMENT_MATCHMAKE_FAILED";
              goto LAB_00bd7d30;
            }
            if (iVar7 == 7) {
              uVar20 = FUN_00984c30();
              pcVar25 = "TID_MATCHMAKE_FAILED_WAR_ANOTHER_PLAYER_BATTLING";
              goto LAB_00bd7d30;
            }
          }
          else {
            if (iVar7 == 9) {
              uVar20 = FUN_00984c30();
              pcVar25 = "TID_MATCHMAKE_FAILED_WAR_SECTION_ENDED";
              goto LAB_00bd7d30;
            }
            if (iVar7 == 10) {
              lVar23 = FUN_00d85c28();
              lVar23 = FUN_0116209c(*(undefined8 *)(lVar23 + 0x90));
              FUN_010faad8(*(undefined8 *)(lVar23 + 0x4a0));
              uVar24 = FUN_010f51f8();
              uVar20 = FUN_00984c30();
              if ((uVar24 & 1) == 0) {
                pcVar25 = "TID_MATCHMAKE_FAILED_DUELS_DISABLED";
              }
              else {
                pcVar25 = "TID_SHUTDOWN_BATTLE_DISABLED";
              }
              goto LAB_00bd7d30;
            }
            if (iVar7 == 0xb) {
              uVar20 = FUN_00984c30();
              pcVar25 = "TID_MATCHMAKE_FAILED_MAX_WAR_PARTICIPANTS";
              goto LAB_00bd7d30;
            }
          }
          uVar20 = FUN_00984c30();
          pcVar25 = "TID_MATCHMAKE_FAILED";
LAB_00bd7d30:
          FUN_0148a810(&local_160,pcVar25);
          uVar17 = FUN_00d994d8(&local_160);
          FUN_00985bb0(0,uVar20,uVar17,0xffffffff);
                    /* WARNING: Subroutine does not return */
          FUN_0148a700(&local_160);
        }
      }
      else {
        if (iVar7 == 0x5e01) {
LAB_00bd542c:
          if (*DAT_019e5bb8 != 0) {
            FUN_008958e0(*DAT_019e5bb8,param_4);
          }
          goto LAB_00bd7aa0;
        }
        if (iVar7 == 0x5e9e) {
          uVar20 = FUN_009a51a0();
          FUN_00e260c0(auStack_a0,param_4);
          FUN_009a7044(uVar20,auStack_a0);
          goto LAB_00bd7aa0;
        }
      }
    }
    else if (iVar7 < 0x6099) {
      if (iVar7 < 0x5f90) {
        if (iVar7 == 0x5f19) {
          lVar23 = FUN_00984c30();
          if ((lVar23 == 0) || (lVar23 = FUN_00cb6508(), lVar23 == 0)) goto LAB_00bd7aa0;
          FUN_011559a8(&local_160,param_4);
          uVar20 = FUN_00cb6508();
          FUN_00cb55cc(uVar20,local_160);
          goto joined_r0x00bd58a0;
        }
        if (iVar7 == 0x5f4b) {
          FUN_01153154(local_98,param_4);
          lVar23 = local_98[0];
          uVar20 = FUN_00855818();
          if (lVar23 != 0) {
            plVar22 = (long *)(local_98[0] + 8);
            if (7 < *(int *)(local_98[0] + 4)) {
              plVar22 = (long *)*(long *)(local_98[0] + 8);
            }
            FUN_0148a810(&local_160,plVar22);
            FUN_0148a42c(&local_78);
            FUN_0148a42c(auStack_88);
            FUN_0085a6dc(uVar20,10,0,&local_160,&local_78,auStack_88);
                    /* WARNING: Subroutine does not return */
            FUN_0148a700(auStack_88);
          }
          FUN_0148a810(&local_160,"ServerError");
          FUN_0148a42c(&local_78);
          FUN_0148a42c(auStack_88);
          uVar17 = 10;
          goto LAB_00bd6f04;
        }
      }
      else {
        if (iVar7 == 0x5f90) {
          uVar20 = FUN_00855818();
          FUN_0148a42c(&local_160);
          FUN_0148a42c(&local_78);
          FUN_0148a42c(auStack_88);
          uVar17 = 1;
LAB_00bd6f04:
          FUN_0085a6dc(uVar20,uVar17,0,&local_160,&local_78,auStack_88);
                    /* WARNING: Subroutine does not return */
          FUN_0148a700(auStack_88);
        }
        if (iVar7 == 0x5fc3) {
          lVar23 = FUN_009a51a0();
          if ((lVar23 == 0) || (FUN_01133488(&local_160,param_4), local_160 == (long *)0x0))
          goto LAB_00bd7aa0;
          uVar20 = FUN_009a51a0();
          FUN_009a5f9c(uVar20,local_160);
          if (local_160 == (long *)0x0) goto LAB_00bd7aa0;
          FUN_01133130();
          goto LAB_00bd6a58;
        }
      }
    }
    else if (iVar7 < 0x61f5) {
      if (iVar7 == 0x6099) {
        if (*DAT_019e5940 != 0) {
          lVar23 = FUN_00984c30();
          plVar22 = DAT_019e58e8;
          if ((lVar23 != 0) &&
             ((lVar23 = param_4[0x17], *(char *)(*DAT_019e58e8 + 0x40) == '\0' ||
              ((uVar24 = FUN_00ca6710(), (uVar24 & 1) == 0 &&
               (uVar24 = FUN_0151eb74(*plVar22), (uVar24 & 1) == 0)))))) {
            if (((char)param_4[0x16] != '\0') && (*(char *)(*plVar22 + 0x40) != '\0')) {
LAB_00bd6808:
              FUN_0148a810(&local_160,"TID_JUPITER_BOUND_TO_OTHER_ACCOUNT_TITLE");
              FUN_00d994d8(&local_160);
                    /* WARNING: Subroutine does not return */
              FUN_0148a700(&local_160);
            }
            if (param_4[0x14] != 0) {
              iVar7 = FUN_00de3864(lVar23);
              FUN_0090031c();
              lVar23 = FUN_00eccc70();
              if (*(int *)(lVar23 + 0x2a0) <= iVar7) {
                FUN_0148a810(&local_160,"TID_GOOGLE_SERVICE_BOUND_TO_OTHER_ACCOUNT_TITLE");
                FUN_00d994d8(&local_160);
                    /* WARNING: Subroutine does not return */
                FUN_0148a700(&local_160);
              }
            }
          }
        }
        goto LAB_00bd7aa0;
      }
      if (iVar7 == 0x61b2) {
        lVar23 = FUN_00984c30();
        if (lVar23 == 0) goto LAB_00bd7aa0;
        iVar7 = (int)param_4[0x12];
        if (iVar7 < 3) {
          if (iVar7 == 1) {
            pcVar25 = "TID_CLAN_JOIN_FAIL_CLOSED";
          }
          else {
            if (iVar7 != 2) goto LAB_00bd7940;
            pcVar25 = "TID_CLAN_JOIN_FAIL_ALREADY_SENT";
          }
        }
        else if (iVar7 == 3) {
          pcVar25 = "TID_CLAN_JOIN_FAIL_NO_SCORE";
        }
        else if (iVar7 == 4) {
          pcVar25 = "TID_CLAN_JOIN_FAILED_BANNED";
        }
        else {
LAB_00bd7940:
          pcVar25 = "TID_CLAN_JOIN_FAIL_GENERIC";
        }
LAB_00bd65a8:
        FUN_0148a810(&local_160,pcVar25);
        goto LAB_00bd6f7c;
      }
    }
    else {
      if (iVar7 == 0x61f5) {
        if (((*DAT_019e59b8 == 0) || (FUN_0114a4f0(&local_160,param_4), local_160 == (long *)0x0))
           || (FUN_00d766c0(*plVar2), local_160 == (long *)0x0)) goto LAB_00bd7aa0;
        FUN_0114973c();
LAB_00bd6a58:
        if (local_160 != (long *)0x0) {
          (**(code **)(*local_160 + 8))();
        }
        goto LAB_00bd7aa0;
      }
      if (iVar7 == 0x62c1) {
        lVar23 = FUN_00d85c28();
        if (((lVar23 != 0) && (lVar23 = FUN_00d89c10(), lVar23 != 0)) &&
           ((lVar23 = FUN_00984c30(), lVar23 != 0 &&
            (lVar23 = FUN_009860d0(lVar23,1,1), lVar23 != 0)))) {
          FUN_00af3f0c();
        }
        goto LAB_00bd7aa0;
      }
      if (iVar7 == 0x62ca) {
        if ((int)param_4[0x13] != 1) {
          uVar20 = FUN_0148a810(&local_160,"TID_INVITE_CLAN_TOKEN_ERROR");
          FUN_00bd92e0(uVar20,&local_160);
          goto LAB_00bd7a9c;
        }
        uVar20 = FUN_009a51a0();
        FUN_0113533c(auStack_d0,param_4);
        FUN_009a6a60(uVar20,auStack_d0);
        goto LAB_00bd7aa0;
      }
    }
    goto LAB_00bd6b1c;
  }
  if (iVar7 < 0x6c6e) {
    if (iVar7 < 0x669f) {
      if (iVar7 < 0x64c5) {
        if (iVar7 < 0x6378) {
          if (iVar7 < 0x6338) {
            if (iVar7 == 0x62ee) {
              if ((int)param_4[0x16] == 4) {
                lVar23 = FUN_009a51a0();
                if (lVar23 != 0) {
                  uVar20 = FUN_009a51a0();
                  FUN_009a5aec(uVar20,param_4 + 0x14,param_4 + 0x12);
                }
              }
              else {
                FUN_00d78a00(*DAT_019e59b8,param_4);
              }
              goto LAB_00bd7aa0;
            }
            iVar8 = 0x6307;
            goto LAB_00bd560c;
          }
          if (iVar7 == 0x6338) {
            lVar23 = FUN_00984c30();
            if ((lVar23 == 0) || (lVar23 = FUN_009919f4(), lVar23 == 0)) goto LAB_00bd7aa0;
            pvVar15 = operator_new(0x120);
            uVar20 = 5;
LAB_00bd5eac:
            uVar17 = 0;
LAB_00bd5eb0:
            FUN_0092bf14(pvVar15,uVar20,uVar17);
LAB_00bd5eb4:
            FUN_0092d998(lVar23,pvVar15);
            goto LAB_00bd7aa0;
          }
          if (iVar7 == 0x633e) {
            uVar20 = FUN_00984c30();
            lVar23 = FUN_009860d0(uVar20,0x5c,1);
            if (lVar23 != 0) {
              FUN_00b55c2c(lVar23,param_4);
            }
            goto LAB_00bd7aa0;
          }
        }
        else {
          if (iVar7 < 0x6420) {
            if (iVar7 == 0x6378) {
              if ((long *)*DAT_019e58e8 != (long *)0x0) {
                lVar23 = *(long *)*DAT_019e58e8;
                if ((int)param_4[0x12] == 0) {
                  (**(code **)(lVar23 + 0xd8))();
                }
                else {
                  (**(code **)(lVar23 + 0xd0))();
                }
              }
              goto LAB_00bd7aa0;
            }
            iVar8 = 0x640b;
            goto LAB_00bd5118;
          }
          if (iVar7 == 0x6420) {
            iVar7 = (int)param_4[0x12];
            lVar13 = param_4[0x13];
            lVar23 = FUN_00cbbc78();
            FUN_0148d280(&local_160,"HostedMatchChallengeFailedMessage RECEIVED, reason %d",iVar7);
            if (lVar13 != 0) {
              FUN_0131ab24(auStack_88,lVar13);
              FUN_0148b080(&local_78,", matchId ",auStack_88);
              FUN_0148b090(&local_160,&local_78);
                    /* WARNING: Subroutine does not return */
              FUN_0148a700(&local_78);
            }
            pcVar25 = (char *)*DAT_019e59c0;
            lVar13 = FUN_00a94ac8(pcVar25);
            if (lVar13 == 0) {
              bVar3 = false;
            }
            else {
              bVar3 = *pcVar25 != '\0';
            }
            uVar24 = FUN_00a94b38(pcVar25,0);
            lVar13 = FUN_00984c30();
            if ((((lVar23 != 0) && (lVar13 != 0)) && (bVar3)) &&
               (((uVar24 & 1) != 0 && (lVar13 = FUN_00cbaec0(lVar23), lVar13 != 0)))) {
              FUN_00cbaec0(lVar23);
              if (iVar7 == 4) {
                FUN_00cdb908();
              }
              else if (iVar7 == 3) {
                FUN_00cdb8ac();
              }
              else if (iVar7 == 1) {
                FUN_00cdb850();
              }
              else {
                FUN_00cdb964();
              }
            }
            goto LAB_00bd7a9c;
          }
          if (iVar7 == 0x643b) {
            lVar13 = param_4[0x13];
            lVar23 = FUN_00984c30();
            if (lVar23 != 0) {
              uVar20 = FUN_00984c30();
              lVar23 = FUN_009860d0(uVar20,0x33,1);
              if (lVar23 != 0) {
                FUN_00ad8d74(lVar23,lVar13);
              }
            }
            lVar23 = FUN_00d85c28();
            if (((lVar23 != 0) && (lVar23 = FUN_00d89ae4(), lVar23 != 0)) &&
               (lVar23 = FUN_009a51a0(), lVar23 != 0)) {
              uVar20 = FUN_009a51a0();
              FUN_009a58e4(uVar20,lVar13);
            }
            goto LAB_00bd7aa0;
          }
        }
      }
      else if (iVar7 < 0x6623) {
        if (iVar7 < 0x65a6) {
          if (iVar7 == 0x64c5) goto LAB_00bd5614;
          if (iVar7 == 0x6524) {
            iVar7 = FUN_01321248(param_4);
            if (iVar7 == 2) {
              FUN_00bdbda0(*plVar22,0xffffffff,1);
            }
            goto LAB_00bd7aa0;
          }
        }
        else {
          if (iVar7 == 0x65a6) {
            lVar23 = FUN_009a51a0();
            if (lVar23 != 0) {
              uVar20 = FUN_009a51a0();
              FUN_009a5ae4(uVar20,(int)param_4[0x12]);
              uVar20 = FUN_009a51a0();
              FUN_009a5aec(uVar20,param_4 + 0x15,param_4 + 0x13);
            }
            goto LAB_00bd7aa0;
          }
          iVar8 = 0x6620;
LAB_00bd5118:
          if (iVar7 == iVar8) goto LAB_00bd7aa0;
        }
      }
      else if (iVar7 < 0x6696) {
        if (iVar7 == 0x6623) {
          iVar7 = (int)param_4[0x12];
          if (iVar7 < 2) {
            if (iVar7 == 0) {
              if (*DAT_019e5940 != 0) {
                FUN_00d7b950();
              }
              goto LAB_00bd7aa0;
            }
            if (iVar7 != 1) goto LAB_00bd7aa0;
          }
          else if ((iVar7 != 2) && (iVar7 != 3)) goto LAB_00bd7aa0;
          if (*DAT_019e5940 != 0) {
            FUN_00d7c014();
          }
          goto LAB_00bd7aa0;
        }
        if (iVar7 == 0x6648) goto LAB_00bd542c;
      }
      else {
        if (iVar7 == 0x6696) {
          iVar7 = (int)param_4[0x12];
          if (iVar7 < 3) {
            if (iVar7 == 1) {
              lVar23 = FUN_00984c30();
              pcVar25 = "TID_PLAYER_REPORTED_SUCCESSFULLY";
            }
            else {
              if (iVar7 != 2) goto LAB_00bd7aa0;
              lVar23 = FUN_00984c30();
              pcVar25 = "TID_TOO_MANY_REPORTS_SENT";
            }
          }
          else {
            if (iVar7 != 3) {
              if (iVar7 == 6) {
                lVar23 = FUN_00984c30();
                pcVar25 = "TID_TOO_MANY_ALLIANCE_CHAT_REPORTS_SENT";
                goto LAB_00bd7554;
              }
              if (iVar7 != 7) goto LAB_00bd7aa0;
            }
            lVar23 = FUN_00984c30();
            pcVar25 = "TID_PLAYER_ALREADY_REPORTED";
          }
LAB_00bd7554:
          FUN_0148a810(&local_160,pcVar25);
          puVar21 = (undefined8 *)FUN_00d994d8(&local_160);
          goto LAB_00bd7570;
        }
        if (iVar7 == 0x669c) {
          lVar23 = *plVar22;
          if (((lVar23 != 0) && (uVar24 = FUN_0131a9dc(param_4 + 0x12), (uVar24 & 1) == 0)) &&
             (*(int *)((long)param_4 + 0x9c) != 0)) {
            uVar32 = *(undefined4 *)(lVar23 + 0x58);
            uVar10 = FUN_0131aa88(param_4 + 0x12);
            uVar11 = FUN_0131aa90(param_4 + 0x12);
            FUN_00bddf2c(lVar23,uVar32,param_4 + 0x13,uVar10,uVar11);
          }
          lVar23 = FUN_00984c30();
          if (lVar23 == 0) goto LAB_00bd7aa0;
          lVar13 = FUN_009919f4();
          if (lVar13 == 0) {
            pvVar15 = operator_new(0x2d0);
            FUN_00b3b0e8(pvVar15,0);
            FUN_009855cc(lVar23,pvVar15,1,0,0,0,0);
            lVar13 = FUN_009919f4(lVar23);
            if (lVar13 == 0) goto LAB_00bd7aa0;
          }
          pvVar15 = operator_new(0x118);
          FUN_0092cc44();
LAB_00bd4f28:
          FUN_0092d998(lVar13,pvVar15);
          goto LAB_00bd7aa0;
        }
      }
    }
    else if (iVar7 < 0x69fb) {
      if (iVar7 < 0x6810) {
        if (iVar7 < 0x66ed) {
          if (iVar7 == 0x669f) {
            FUN_00e26e28(&local_160,param_4);
            uVar20 = FUN_009a51a0();
            local_b0 = local_160;
            FUN_009a7d1c(uVar20,&local_b0);
            goto LAB_00bd7aa0;
          }
          if (iVar7 == 0x66b7) {
            lVar23 = FUN_00984c30();
            if (lVar23 == 0) goto LAB_00bd7aa0;
            lVar23 = FUN_00984c30();
            pcVar25 = "TID_CHALLENGE_FAIL_SERVER";
            goto LAB_00bd7554;
          }
        }
        else {
          if (iVar7 == 0x66ed) {
            lVar23 = FUN_00cbbc78();
            if (lVar23 == 0) {
LAB_00bd6b54:
              bVar3 = false;
            }
            else {
              FUN_00cbbc78();
              lVar23 = FUN_00cbaec0();
              if (lVar23 == 0) goto LAB_00bd6b54;
              FUN_00cbbc78();
              lVar23 = FUN_00cbaec0();
              bVar3 = *(char *)(lVar23 + 0x298) != '\0';
            }
            lVar23 = FUN_00cbbc78();
            if (lVar23 != 0) {
              FUN_00cbbc78();
              lVar23 = FUN_00cbaeac();
              if (lVar23 != 0) {
                FUN_00cbbc78();
                lVar23 = FUN_00cbaeac();
                if (*(long *)(lVar23 + 0x1a8) != 0) {
                  bVar3 = true;
                }
              }
            }
            plVar22 = (long *)FUN_012ff54c();
            if (plVar22 == (long *)0x0) {
              uVar9 = 0;
            }
            else {
              uVar9 = (**(code **)(*plVar22 + 0x28))();
              uVar9 = uVar9 ^ 1;
            }
            if (!bVar3 && (uVar9 & 1) == 0) {
              FUN_00bd9070(param_3,param_4);
              goto LAB_00bd7aa0;
            }
            lVar23 = FUN_00cbbc78();
            if (lVar23 == 0) {
LAB_00bd6eac:
              lVar23 = FUN_00cbbc78();
              if (lVar23 != 0) {
                FUN_00cbbc78();
                lVar23 = FUN_00cbaec0();
                if (lVar23 != 0) {
                  FUN_00cbbc78();
                  FUN_00cbaec0();
                  FUN_00cdbb2c();
                  goto LAB_00bd7374;
                }
              }
              if (*(long **)(param_3 + 0xa8) == (long *)0x0) goto LAB_00bd7374;
              (**(code **)(**(long **)(param_3 + 0xa8) + 0x38))();
              if (*(long **)(param_3 + 0xa8) == (long *)0x0) goto LAB_00bd7374;
              pcVar29 = *(code **)(**(long **)(param_3 + 0xa8) + 8);
            }
            else {
              FUN_00cbbc78();
              lVar23 = FUN_00cbaeac();
              if (lVar23 == 0) goto LAB_00bd6eac;
              FUN_00cbbc78();
              lVar23 = FUN_00cbaeac();
              if (*(long *)(lVar23 + 0x1a8) == 0) goto LAB_00bd6eac;
              FUN_00cbbc78();
              lVar23 = FUN_00cbaeac();
              pcVar29 = *(code **)(**(long **)(lVar23 + 0x1a8) + 0x1c0);
            }
            (*pcVar29)();
LAB_00bd7374:
            *(undefined4 *)(param_3 + 0xb0) = 0;
            *(long **)(param_3 + 0xa8) = param_4;
            return 0;
          }
          if (iVar7 == 0x67b8) {
            lVar23 = FUN_009a51a0();
            if ((lVar23 == 0) || (FUN_01134958(&local_160,param_4), local_160 == (long *)0x0))
            goto LAB_00bd7aa0;
            uVar20 = FUN_009a51a0();
            FUN_009a6150(uVar20,local_160,(int)param_4[0x13]);
            goto joined_r0x00bd58a0;
          }
        }
      }
      else {
        if (iVar7 < 0x6923) {
          if (iVar7 == 0x6810) {
LAB_00bd5460:
            if (*DAT_019e5bc8 != 0) {
              FUN_008985b4(*DAT_019e5bc8,param_4);
            }
            goto LAB_00bd7aa0;
          }
          iVar8 = 0x68e3;
          goto LAB_00bd560c;
        }
        if (iVar7 == 0x6923) {
          if (*(long **)(param_3 + 0x98) != (long *)0x0) {
            (**(code **)(**(long **)(param_3 + 0x98) + 8))();
          }
          *(undefined8 *)(param_3 + 0x98) = 0;
          lVar23 = FUN_00d85c28();
          if (lVar23 != 0) {
            lVar13 = FUN_00cb6508();
            if (lVar13 == 0) {
              FUN_00d85c28();
              lVar23 = FUN_00d89b0c();
              if (lVar23 == 0) {
                *(long **)(param_3 + 0x88) = param_4;
                return 0;
              }
              FUN_00d85c28();
              plVar22 = (long *)FUN_00d89b0c();
              if (plVar22 != (long *)0x0) {
                (**(code **)(*plVar22 + 0xa88))(plVar22,param_4);
              }
            }
            else {
              if (*(long *)(lVar23 + 0x90) != 0) {
                uVar32 = *(undefined4 *)((long)param_4 + 0xac);
                uVar17 = *(undefined8 *)(*(long *)(lVar23 + 0x90) + 0xa8);
                uVar20 = FUN_00d8a954(lVar23,0,0);
                FUN_00d8c730(uVar32,uVar17,uVar20);
              }
              FUN_00cb5208();
              uVar20 = FUN_00cb6508();
              FUN_00cb145c(uVar20,param_4);
            }
          }
          goto LAB_00bd7aa0;
        }
        if (iVar7 == 0x69a6) {
          if (*DAT_019e5938 == 0) goto LAB_00bd7aa0;
          lVar23 = FUN_00984c30();
          plVar22 = DAT_019e58e8;
          if ((lVar23 == 0) ||
             ((lVar23 = param_4[0x18], *(char *)(*DAT_019e58e8 + 0x40) != '\0' &&
              ((uVar24 = FUN_00ca6710(), (uVar24 & 1) != 0 ||
               (uVar24 = FUN_0151eb74(*plVar22), (uVar24 & 1) != 0)))))) goto LAB_00bd7aa0;
          if (((char)param_4[0x17] != '\0') && (*(char *)(*plVar22 + 0x40) != '\0'))
          goto LAB_00bd6808;
          if (param_4[0x14] == 0) goto LAB_00bd7aa0;
          FUN_0148a42c(&local_160);
          FUN_0148a42c(&local_78);
          FUN_0090031c();
          lVar13 = FUN_00eccc70();
          iVar7 = *(int *)(lVar13 + 0x2a0);
          FUN_0148a42c(auStack_88);
          if (*plVar1 != 0) {
            uVar20 = FUN_00d7b4e4();
            FUN_0148acf4(auStack_88,uVar20);
          }
          iVar8 = FUN_00de3864(lVar23);
          if (iVar7 <= iVar8) {
            FUN_0148a810(local_98,"TID_GAMECENTER_BOUND_TO_OTHER_ACCOUNT_TITLE");
            FUN_00d994d8(local_98);
                    /* WARNING: Subroutine does not return */
            FUN_0148a700(local_98);
          }
          goto LAB_00bd7180;
        }
      }
    }
    else if (iVar7 < 0x6b71) {
      if (iVar7 < 0x6abb) {
        if (iVar7 == 0x69fb) goto LAB_00bd5614;
        if (iVar7 == 0x6a6f) {
          uVar32 = 0x17;
          if ((int)param_4[0x12] != 1) {
            uVar32 = 7;
          }
          uVar20 = FUN_00855818(7);
          FUN_0148a42c(&local_160);
          FUN_0148a42c(&local_78);
          FUN_0148a42c(auStack_88);
          goto LAB_00bd7178;
        }
      }
      else {
        if (iVar7 == 0x6abb) goto LAB_00bd7aa0;
        if (iVar7 == 0x6b1b) {
          *DAT_019e5888 = 0;
          lVar23 = FUN_00984c30();
          if (((lVar23 != 0) && (lVar23 = FUN_00cbbc78(), lVar23 != 0)) &&
             (lVar13 = FUN_00cbaec0(), lVar13 != 0)) {
            FUN_00cbaec0(lVar23);
            thunk_FUN_00cd978c();
          }
          goto LAB_00bd7aa0;
        }
      }
    }
    else if (iVar7 < 0x6c0f) {
      if (iVar7 == 0x6b71) {
        lVar23 = FUN_00984c30();
        if (lVar23 == 0) goto LAB_00bd7aa0;
        iVar7 = (int)param_4[0x12];
        if (iVar7 < 5) {
          if (iVar7 == 2) {
            pcVar25 = "TID_ALLIANCE_INVITATION_FAILED_NO_RIGHTS";
          }
          else {
            if (iVar7 != 4) goto LAB_00bd7aa0;
            pcVar25 = "TID_ALLIANCE_INVITATION_FAILED_ALREADY_IN_ALLIANCE";
          }
        }
        else if (iVar7 == 5) {
          pcVar25 = "TID_ALLIANCE_INVITATION_FAILED_AVATAR_ALREADY_HAS_AN_INVITE";
        }
        else {
          if (iVar7 != 6) goto LAB_00bd7aa0;
          pcVar25 = "TID_ALLIANCE_INVITATION_FAILED_AVATAR_HAS_TOO_MANY_INVITES";
        }
        goto LAB_00bd65a8;
      }
      iVar8 = 0x6b83;
LAB_00bd560c:
      if (iVar7 == iVar8) {
LAB_00bd5614:
        if (*DAT_019e5930 != 0) {
          FUN_00d9a260(*DAT_019e5930,param_4);
        }
        goto LAB_00bd7aa0;
      }
    }
    else {
      if (iVar7 == 0x6c0f) {
        if (*DAT_019e5930 != 0) {
          FUN_00d9b7d8(*DAT_019e5930,param_4);
        }
        goto LAB_00bd7aa0;
      }
      if (iVar7 == 0x6c1f) {
        FUN_01150ac0(&local_160,param_4);
        if (local_160 != (long *)0x0) {
          local_268[0] = local_160;
          FUN_00bd9658(param_3,local_268);
        }
        goto LAB_00bd7aa0;
      }
      if (iVar7 == 0x6c3a) {
        lVar23 = param_4[0x13];
        if ((0 < (int)lVar23) && (pvVar15 = (void *)FUN_01151798(param_4), pvVar15 != (void *)0x0))
        {
          local_160 = (void *)0x0;
          iVar7 = FUN_0146afbc(pvVar15,(int)lVar23,&local_160,0);
          pvVar14 = local_160;
          if ((0 < iVar7) && (local_160 != (void *)0x0)) {
            pvVar12 = operator_new(0x10);
            FUN_0148a800(pvVar12,pvVar14,iVar7);
            uVar20 = FUN_00d8c5b8();
            FUN_00d8de8c(uVar20,pvVar12);
          }
          if (local_160 != (void *)0x0) {
            operator_delete__(local_160);
          }
          local_160 = (long *)0x0;
          operator_delete__(pvVar15);
        }
        goto LAB_00bd7aa0;
      }
    }
    goto LAB_00bd6b1c;
  }
  if (iVar7 < 0x7278) {
    if (iVar7 < 0x6efd) {
      if (iVar7 < 0x6da0) {
        if (iVar7 < 0x6cd8) {
          if (iVar7 == 0x6c6e) {
            lVar23 = *DAT_019e5938;
            if (lVar23 != 0) {
              FUN_00d7a1b8(lVar23);
              FUN_00d7a258(lVar23);
            }
            goto LAB_00bd7aa0;
          }
          if (iVar7 == 0x6c8a) {
            if (*(char *)((long)param_4 + 0xa4) != '\0') {
              pvVar15 = operator_new(0x88);
              uVar20 = FUN_0090031c();
              FUN_0113b7e4(pvVar15,uVar20);
              uVar20 = FUN_0131aa5c(param_4[0x12]);
              FUN_0113ba58(pvVar15,uVar20);
              FUN_0113baa8(pvVar15,param_4[0x13]);
              FUN_0113ba48(pvVar15,(int)param_4[0x14]);
              uVar24 = FUN_00cd6674(0x11,pvVar15,0,0,0,0);
              if ((uVar24 & 1) != 0) {
                FUN_00cbbc78();
                FUN_00cbaec0();
                FUN_00cd6a68();
              }
              goto LAB_00bd7aa0;
            }
            lVar23 = FUN_00984c30();
            pcVar25 = "TID_REPLAY_NOT_AVAILABLE";
            goto LAB_00bd7554;
          }
        }
        else {
          if (iVar7 == 0x6cd8) {
            lVar23 = param_4[0x12];
            *DAT_019e5888 = 0;
            lVar13 = FUN_00984c30();
            if (lVar13 != 0) {
              FUN_0148a42c(&local_160);
              if ((int)lVar23 == 1) {
                pcVar25 = "TID_SHARED_FRIENDLY_CHALLENGE_ERROR_ALREADY_STARTED";
              }
              else if ((int)lVar23 == 2) {
                pcVar25 = "TID_SHARED_FRIENDLY_CHALLENGE_ERROR_EXPIRED";
              }
              else {
                pcVar25 = "TID_SHARED_FRIENDLY_CHALLENGE_ERROR_FAILED";
              }
              FUN_0148abd4(&local_160,pcVar25);
              FUN_0148a810(&local_78,"TID_CHALLENGE");
              FUN_00d994d8(&local_78);
                    /* WARNING: Subroutine does not return */
              FUN_0148a700(&local_78);
            }
            goto LAB_00bd7aa0;
          }
          if (iVar7 == 0x6d67) {
            if (*DAT_019e68a0 != 0) {
              FUN_00c80acc(*DAT_019e68a0,param_4);
            }
            goto LAB_00bd7aa0;
          }
        }
      }
      else {
        if (0x6ed8 < iVar7) {
          if (iVar7 == 0x6ed9) {
            FUN_01146998(&local_160,param_4);
            if (local_160 == (long *)0x0) goto LAB_00bd7aa0;
            cVar4 = FUN_00e09be4();
            if (cVar4 == '\x04') {
              iVar7 = FUN_00e09e20(local_160);
              if (1 < iVar7) {
                uVar32 = FUN_00e09e2c(local_160,0);
                uVar10 = FUN_00e09e2c(local_160,1);
                FUN_00cb640c(uVar32,uVar10);
              }
            }
            else {
              lVar23 = FUN_00cb6508();
              if (lVar23 == 0) {
                FUN_00d85c28();
                plVar22 = (long *)FUN_00d89b0c();
                if (plVar22 != (long *)0x0) {
                  (**(code **)(*plVar22 + 0xa98))(plVar22,local_160,0);
                }
              }
              else {
                uVar24 = FUN_00cb0dbc();
                if ((uVar24 & 1) == 0) {
                  FUN_00cb6f50(lVar23,local_160,0);
                }
              }
            }
            if ((local_160 == (long *)0x0) || (FUN_00e098f0(), local_160 == (long *)0x0))
            goto LAB_00bd7aa0;
            local_78 = local_160 + 2;
            goto LAB_00bd6bd4;
          }
          iVar8 = 0x6ee5;
          goto LAB_00bd560c;
        }
        if (iVar7 == 0x6da0) {
          lVar23 = FUN_00cb6508();
          if (lVar23 != 0) {
            FUN_00cb6508();
            lVar23 = FUN_00cb5180();
            if (lVar23 != 0) {
              FUN_00cb6508();
              uVar20 = FUN_00cb5180();
              FUN_009bca08(uVar20,param_4 + 0x12,(char)param_4[0x13]);
            }
          }
          goto LAB_00bd7aa0;
        }
        if (iVar7 == 0x6e87) {
          plVar22 = (long *)*DAT_019e58e8;
          if (plVar22 != (long *)0x0) {
            if ((int)param_4[0x12] == 0) {
              lVar23 = FUN_01157eb8(param_4);
              if (lVar23 == 0) {
                puVar21 = (undefined8 *)&DAT_003d4b92;
              }
              else {
                lVar23 = FUN_01157eb8(param_4);
                puVar21 = (undefined8 *)(lVar23 + 8);
                if (7 < *(int *)(lVar23 + 4)) {
                  puVar21 = *(undefined8 **)(lVar23 + 8);
                }
              }
              FUN_0148a810(&local_160,puVar21);
              (**(code **)(*plVar22 + 0xb8))(plVar22,&local_160);
              goto LAB_00bd7a9c;
            }
            (**(code **)(*plVar22 + 0xc0))(plVar22);
          }
          goto LAB_00bd7aa0;
        }
      }
    }
    else if (iVar7 < 0x7167) {
      if (iVar7 < 0x6f9c) {
        if (iVar7 == 0x6efd) {
          lVar23 = FUN_009a51a0();
          if ((lVar23 == 0) || (FUN_01133604(&local_160,param_4), local_160 == (long *)0x0))
          goto LAB_00bd7aa0;
          uVar20 = FUN_009a51a0();
          FUN_009a5dd4(uVar20,local_160);
          goto joined_r0x00bd58a0;
        }
        if (iVar7 == 0x6f62) {
          lVar23 = FUN_00cb6508();
          if (lVar23 != 0) {
            FUN_00cb6508();
            lVar23 = FUN_00cb5180();
            if (lVar23 != 0) {
              FUN_00cb6508();
              uVar20 = FUN_00cb5180();
              FUN_009bc900(uVar20,param_4 + 0x12,(char)param_4[0x13],
                           *(undefined1 *)((long)param_4 + 0x99));
            }
          }
          goto LAB_00bd7aa0;
        }
      }
      else {
        if (iVar7 == 0x6f9c) {
          *DAT_019e5888 = 0;
          goto LAB_00bd7aa0;
        }
        if (iVar7 == 0x7028) {
          FUN_00896da8(*DAT_019e5968,param_4 + 0x13,(int)param_4[0x12],
                       *(uint *)((long)param_4 + 0x94) & 1);
          goto LAB_00bd7aa0;
        }
      }
    }
    else if (iVar7 < 0x71f6) {
      if (iVar7 == 0x7167) goto LAB_00bd5614;
      if (iVar7 == 0x71e7) goto LAB_00bd5744;
    }
    else {
      if (iVar7 == 0x71f6) {
LAB_00bd5760:
        FUN_00896b20(*DAT_019e5968,param_4 + 0x12,param_4 + 0x14);
        goto LAB_00bd7aa0;
      }
      if (iVar7 == 0x7277) {
        uVar20 = FUN_00d8c5b8();
        FUN_00d8e1bc(uVar20,1);
        goto LAB_00bd7aa0;
      }
    }
    goto LAB_00bd6b1c;
  }
  if (iVar7 < 0x745d) {
    if (iVar7 < 0x7384) {
      if (0x7338 < iVar7) {
        if (iVar7 == 0x7339) goto LAB_00bd5614;
        iVar8 = 0x735b;
        goto LAB_00bd560c;
      }
      if (iVar7 == 0x7278) {
        iVar7 = (int)param_4[0x13];
        if (iVar7 < 3) {
          if (iVar7 == 1) {
            pcVar25 = "TID_ADD_FRIEND_FAILED_BANNED";
          }
          else {
            if (iVar7 != 2) goto LAB_00bd746c;
            pcVar25 = "TID_ADD_FRIEND_FAILED_THEIR_LIST_FULL";
          }
        }
        else if (iVar7 == 3) {
          pcVar25 = "TID_ADD_FRIEND_FAILED_YOUR_LIST_FULL";
        }
        else if (iVar7 == 4) {
          pcVar25 = "TID_ADD_FRIEND_FAILED_TOKEN_EXPIRED";
        }
        else {
LAB_00bd746c:
          pcVar25 = "TID_ADD_FRIEND_FAILED";
        }
        uVar20 = FUN_0148a810(&local_160,pcVar25);
        FUN_00bd92e0(uVar20,&local_160);
        goto LAB_00bd7a9c;
      }
      if (iVar7 != 0x72de) goto LAB_00bd6b1c;
      FUN_0148a890(&local_160,param_4 + 0x12);
      uVar24 = FUN_0148ce14(&local_160,&DAT_00421ecf);
      if ((uVar24 & 1) != 0) {
        uVar20 = FUN_00d994d8(&local_160);
        FUN_0148acf4(&local_160,uVar20);
      }
      if (local_160._4_4_ == 0) goto LAB_00bd7a9c;
      lVar23 = FUN_00984c30();
      puVar21 = &local_160;
LAB_00bd7570:
      FUN_00985bb0(0,lVar23,puVar21,0xffffffff);
      goto LAB_00bd7a9c;
    }
    if (iVar7 < 0x7430) {
      if (iVar7 == 0x7384) {
        lVar23 = param_4[0x13];
        lVar13 = param_4[0x14];
        iVar7 = (int)param_4[0x12];
        lVar16 = FUN_00cbbc78();
        FUN_0148d280(&local_160,"FriendlyChallengeFailedMessage RECEIVED, reason %d",iVar7);
        if (lVar23 != 0) {
          FUN_0131ab24(auStack_88,lVar23);
          FUN_0148b080(&local_78,", senderId ",auStack_88);
          FUN_0148b090(&local_160,&local_78);
                    /* WARNING: Subroutine does not return */
          FUN_0148a700(&local_78);
        }
        if (lVar13 != 0) {
          FUN_0131ab24(auStack_88,lVar13);
          FUN_0148b080(&local_78,", entryId ",auStack_88);
          FUN_0148b090(&local_160,&local_78);
                    /* WARNING: Subroutine does not return */
          FUN_0148a700(&local_78);
        }
        lVar31 = *DAT_019e59b8;
        lVar23 = FUN_00d78354(lVar31);
        lVar13 = FUN_00d7835c(lVar31);
        if (lVar23 == 0) {
          bVar5 = false;
LAB_00bd75b4:
          bVar3 = false;
        }
        else {
          lVar18 = FUN_011451bc(lVar23);
          if (lVar18 == 0) {
LAB_00bd6d1c:
            lVar18 = FUN_011451c4(lVar23);
            if (lVar18 == 0) {
              bVar5 = false;
            }
            else {
              FUN_011451c4(lVar23);
              iVar8 = FUN_00fc0c24();
              bVar5 = iVar8 == 0x14;
            }
          }
          else {
            FUN_011451bc(lVar23);
            uVar24 = FUN_00ede568();
            if ((uVar24 & 1) == 0) goto LAB_00bd6d1c;
            bVar5 = true;
          }
          lVar18 = FUN_011451bc(lVar23);
          if ((lVar18 == 0) || (lVar18 = FUN_011451bc(lVar23), *(int *)(lVar18 + 0x78) != 1))
          goto LAB_00bd75b4;
          lVar18 = FUN_011451c4(lVar23);
          bVar3 = lVar18 != 0;
        }
        if ((lVar13 != 0) && (bVar3 == false)) {
          lVar18 = FUN_011451c4(lVar13);
          bVar3 = lVar18 != 0;
        }
        if ((lVar13 != 0) && ((bool)bVar5 != true)) {
          lVar18 = FUN_011451bc(lVar13);
          if (lVar18 == 0) {
            bVar5 = 0;
          }
          else {
            FUN_011451bc(lVar13);
            bVar5 = FUN_00ede568();
          }
        }
        if ((lVar13 != 0) && ((bVar5 & 1) == 0)) {
          lVar18 = FUN_011451c4(lVar13);
          if (lVar18 == 0) {
            bVar5 = 0;
          }
          else {
            FUN_011451c4(lVar13);
            iVar8 = FUN_00fc0c24();
            bVar5 = iVar8 == 0x14;
          }
        }
        if ((lVar16 != 0) && ((bVar5 & 1) == 0)) {
          lVar13 = FUN_00cbaec0(lVar16);
          if ((lVar13 == 0) || (uVar24 = FUN_00cc00a0(lVar16), (uVar24 & 1) == 0)) {
            bVar5 = 0;
          }
          else {
            FUN_00cbaec0(lVar16);
            lVar13 = FUN_00cbaec0(lVar16);
            bVar5 = FUN_0131a9f0(lVar13 + 0x2a8,0);
          }
        }
        if ((lVar23 == 0) || (*(char *)(lVar31 + 0xa6) == '\0')) {
LAB_00bd76a4:
          bVar30 = 0;
        }
        else {
          uVar20 = FUN_01143aa8(lVar23);
          uVar24 = FUN_0131a9f0(uVar20,0);
          if ((uVar24 & 1) == 0) goto LAB_00bd76a4;
          bVar30 = (bVar3 | bVar5) ^ 1;
        }
        if (iVar7 == 3) {
          bVar6 = FUN_00d78474(lVar31,0,0);
          bVar6 = bVar5 | bVar6;
        }
        else {
          if (iVar7 == 4) {
            FUN_00d78474(lVar31,0,0);
          }
          bVar6 = 1;
        }
        lVar23 = FUN_00984c30();
        if (((bVar6 & 1) != 0) && (lVar23 != 0)) {
          if (lVar16 == 0) {
            lVar23 = 0;
          }
          else {
            lVar23 = FUN_00cbaec0(lVar16);
            FUN_00cc0a40(lVar16);
          }
          if (iVar7 == 1) {
            if (lVar23 != 0 && ((bVar5 ^ 0xff) & 1) == 0) {
              FUN_00cdb7e0(lVar23);
            }
          }
          else if (iVar7 == 3) {
            if (((lVar23 != 0 && ((bVar5 ^ 0xff) & 1) == 0) &&
                (uVar24 = FUN_00cda8bc(lVar23), (uVar24 & 1) != 0)) &&
               (uVar24 = FUN_00cda8c4(lVar23,0), (uVar24 & 1) != 0)) {
              FUN_00cdb7e0(lVar23);
            }
            if ((bVar30 & 1) != 0) {
              uVar20 = FUN_00984c30();
              FUN_0148a810(&local_78,"TID_FRIENDLY_CHALLENGE_CANCELED");
              uVar17 = FUN_00d994d8(&local_78);
              FUN_00985bb0(0,uVar20,uVar17,0xffffffff);
                    /* WARNING: Subroutine does not return */
              FUN_0148a700(&local_78);
            }
          }
          else {
            if (iVar7 == 6) {
              uVar20 = FUN_00984c30();
              FUN_0148a810(&local_78,"TID_BATTLE_FRIEND_IN_BATTLE");
              uVar17 = FUN_00d994d8(&local_78);
              FUN_00985bb0(0,uVar20,uVar17,0xffffffff);
                    /* WARNING: Subroutine does not return */
              FUN_0148a700(&local_78);
            }
            if (iVar7 == 2) {
              if (lVar23 != 0 && ((bVar5 ^ 0xff) & 1) == 0) {
                FUN_00cdb9c4(lVar23);
              }
            }
            else if (lVar23 != 0 && ((bVar5 ^ 0xff) & 1) == 0) {
              FUN_00cdba98(lVar23,0,0);
            }
          }
          lVar23 = FUN_00cbbc78();
          if (lVar23 != 0) {
            FUN_00cbbc78();
            lVar23 = FUN_00cbaeac();
            if (lVar23 != 0) {
              FUN_00cbbc78();
              lVar23 = FUN_00cbaeac();
              if (*(long *)(lVar23 + 0x1a8) != 0) {
                uVar20 = FUN_00984c30();
                FUN_0148a810(&local_78,"TID_CLAN_WAR_MATCHMAKING_FAILED_GENERIC");
                uVar17 = FUN_00d994d8(&local_78);
                FUN_00985bb0(0,uVar20,uVar17,0xffffffff);
                    /* WARNING: Subroutine does not return */
                FUN_0148a700(&local_78);
              }
            }
          }
        }
LAB_00bd7a9c:
                    /* WARNING: Subroutine does not return */
        FUN_0148a700(&local_160);
      }
      if (iVar7 == 0x73ad) {
        lVar23 = FUN_00cbbc78();
        if (lVar23 != 0) {
          lVar23 = param_4[0x13];
          FUN_00cbbc78();
          uVar20 = FUN_00cbaec0();
          FUN_00cdb444(uVar20,lVar23);
        }
        goto LAB_00bd7aa0;
      }
    }
    else {
      if (iVar7 == 0x7430) {
        lVar23 = FUN_00984c30();
        if (lVar23 == 0) goto LAB_00bd7aa0;
        lVar23 = FUN_00984c30();
        pcVar25 = "TID_ALLIANCE_INVITATION_OK";
        goto LAB_00bd65a8;
      }
      if (iVar7 == 0x745b) goto LAB_00bd5460;
    }
    goto LAB_00bd6b1c;
  }
  if (0x74cc < iVar7) {
    if (0x74f1 < iVar7) {
      if (iVar7 == 0x74f2) {
        if ((int)param_4[0x12] == 1) {
          if (0 < *(int *)((long)param_4 + 0xa4)) {
            FUN_00cbbc78();
            lVar23 = FUN_00cbaed4();
            puVar19 = (undefined4 *)FUN_010faad8(*(undefined8 *)(lVar23 + 0x4a0));
            if (puVar19 != (undefined4 *)0x0) {
              uVar20 = FUN_0090031c();
              pvVar15 = operator_new(0x40);
              FUN_00e3f6b4(pvVar15,uVar20);
              uVar32 = FUN_00ec2570(*(undefined8 *)(puVar19 + 6));
              FUN_00e3f6ec(pvVar15,uVar32,*puVar19);
              if (0 < *(int *)((long)param_4 + 0xa4)) {
                lVar23 = 0;
                do {
                  FUN_00e3f6f4(pvVar15,*(undefined4 *)(param_4[0x13] + lVar23 * 4));
                  lVar23 = lVar23 + 1;
                } while (lVar23 < *(int *)((long)param_4 + 0xa4));
              }
              uVar20 = FUN_00d85c28();
              local_270 = pvVar15;
              FUN_00d893d0(uVar20,&local_270,1);
            }
          }
          goto LAB_00bd7aa0;
        }
        uVar9 = rand();
        pcVar25 = "TID_DECK_EXPLORER_ERROR_GETTING_RESULTS";
        if ((uVar9 & 1) != 0) {
          pcVar25 = "TID_DECK_EXPLORER_ERROR_GETTING_AVAILABLE_CARDS";
        }
        uVar20 = FUN_00984c30();
        uVar32 = FUN_00985b40();
        local_78 = (long *)CONCAT44(param_2,uVar32);
        FUN_0148a810(&local_160,pcVar25);
        uVar17 = FUN_00d994d8(&local_160);
        FUN_00985c3c(0,uVar20,&local_78,uVar17,0xffffffff);
        goto LAB_00bd7a9c;
      }
      if (iVar7 == 0x752d) {
        FUN_00bdbda0(*plVar22,(int)param_4[0x12],0);
        goto LAB_00bd7aa0;
      }
      iVar8 = 40000;
      goto LAB_00bd5118;
    }
    if (iVar7 == 0x74cd) {
      pvVar15 = (void *)param_4[0x13];
      param_4[0x13] = 0;
      if (pvVar15 != (void *)0x0) {
        lVar23 = FUN_01330840(pvVar15);
        if (((lVar23 != 0) && (lVar13 = *DAT_019e58e8, lVar13 != 0)) &&
           (*(char *)(lVar13 + 0x40) != '\0')) {
          FUN_0148acf4(lVar13 + 0x218,lVar23);
        }
        FUN_013306dc(pvVar15);
        pvVar14 = *(void **)((long)pvVar15 + 8);
        *(undefined8 *)((long)pvVar15 + 8) = 0;
        if (pvVar14 != (void *)0x0) {
          operator_delete__(pvVar14);
        }
        operator_delete(pvVar15);
        if (lVar23 != 0) {
                    /* WARNING: Subroutine does not return */
          FUN_0148a700(lVar23);
        }
      }
      lVar23 = DAT_019e5910;
      lVar13 = param_4[0x12];
      if (lVar13 != 0) {
        uVar17 = *(undefined8 *)(lVar13 + 0x30);
        uVar20 = *(undefined8 *)(lVar13 + 0x28);
        uVar34 = *(undefined8 *)(lVar13 + 0x10);
        uVar33 = *(undefined8 *)(lVar13 + 8);
        uVar36 = *(undefined8 *)(lVar13 + 0x20);
        uVar35 = *(undefined8 *)(lVar13 + 0x18);
        *(undefined8 *)(DAT_019e5910 + 0x35) = *(undefined8 *)(lVar13 + 0x35);
        *(undefined8 *)(lVar23 + 0x30) = uVar17;
        *(undefined8 *)(lVar23 + 0x28) = uVar20;
        *(undefined8 *)(lVar23 + 0x20) = uVar36;
        *(undefined8 *)(lVar23 + 0x18) = uVar35;
        *(undefined8 *)(lVar23 + 0x10) = uVar34;
        *(undefined8 *)(lVar23 + 8) = uVar33;
      }
      plVar22 = (long *)FUN_012ff540();
      if (plVar22 != (long *)0x0) {
        (**(code **)(*plVar22 + 0x90))();
      }
      goto LAB_00bd7aa0;
    }
    if (iVar7 != 0x74d5) goto LAB_00bd6b1c;
    FUN_01131308(&local_160,param_4);
    FUN_011312f8(&local_78,param_4);
    lVar23 = *plVar22;
    if (lVar23 == 0) {
LAB_00bd5704:
      if (local_78 != (long *)0x0) {
        operator_delete(local_78);
      }
    }
    else if (local_78 != (long *)0x0) {
      uVar24 = FUN_0131a9dc();
      plVar22 = local_160;
      if (((uVar24 & 1) == 0) && (*(int *)((long)local_160 + 4) != 0)) {
        uVar32 = *(undefined4 *)(lVar23 + 0x58);
        uVar10 = FUN_0131aa88(local_78);
        uVar11 = FUN_0131aa90(local_78);
        FUN_00bddf2c(lVar23,uVar32,plVar22,uVar10,uVar11);
      }
      goto LAB_00bd5704;
    }
    local_78 = (long *)0x0;
    if (local_160 != (long *)0x0) {
                    /* WARNING: Subroutine does not return */
      FUN_0148a700(local_160);
    }
    local_160 = (long *)0x0;
    plVar22 = (long *)FUN_00855818();
    (**(code **)(*plVar22 + 0x20))();
    goto LAB_00bd7aa0;
  }
  if (iVar7 < 0x74cb) {
    if (iVar7 == 0x745d) goto LAB_00bd542c;
    if (iVar7 == 0x7484) {
      lVar23 = FUN_00c7bde0();
      if (lVar23 != 0) {
        FUN_00c7bde0();
        uVar24 = FUN_00c7e35c();
        if ((uVar24 & 1) != 0) {
          uVar20 = FUN_00c7bde0();
          FUN_00c7ef58(uVar20,param_4 + 0x12);
        }
      }
      lVar23 = FUN_01419624();
      if (lVar23 != 0) {
        FUN_01419988(lVar23,param_4 + 0x12);
      }
      goto LAB_00bd7aa0;
    }
  }
  else {
    if (iVar7 == 0x74cb) {
      FUN_01132c38(&local_160,param_4);
      FUN_01132c28(&local_78,param_4);
      lVar23 = FUN_00d85c28();
      if (lVar23 == 0) {
LAB_00bd6b9c:
        if ((local_160 != (long *)0x0) && (FUN_00bd9270(), local_160 != (long *)0x0)) {
          if ((void *)*local_160 != (void *)0x0) {
            operator_delete__((void *)*local_160);
          }
          operator_delete(local_160);
        }
        local_160 = (long *)0x0;
      }
      else {
        FUN_00d85c28();
        lVar23 = FUN_00d89ae4();
        if (lVar23 == 0) goto LAB_00bd6b9c;
        FUN_00d85c28();
        FUN_00d89ae4();
        uVar20 = FUN_00cbaeac();
        local_c0 = local_160;
        FUN_00d316d4(uVar20,&local_c0,local_78);
      }
      if (local_78 != (long *)0x0) {
LAB_00bd6bd4:
                    /* WARNING: Subroutine does not return */
        FUN_0148a700(local_78);
      }
      goto LAB_00bd7aa0;
    }
    if (iVar7 == 0x74cc) {
      lVar23 = *DAT_019e58e8;
      if (lVar23 != 0) {
        uVar20 = FUN_01320ce0(param_4);
        FUN_00ca6cb4(lVar23,uVar20);
      }
      goto LAB_00bd7aa0;
    }
  }
LAB_00bd6b1c:
  FUN_00d7c480(_DAT_01a97d28,param_4);
LAB_00bd7aa0:
  plVar22 = (long *)FUN_0113e7d8(param_4);
  if (((plVar22 != (long *)0x0) && (lVar23 = FUN_00d85c28(), lVar23 != 0)) &&
     (uVar24 = FUN_00d8986c(), (uVar24 & 1) == 0)) {
    FUN_00d8c5b8();
    uVar24 = FUN_00d8dfe8();
    if ((((uVar24 & 1) != 0) && (*(long *)(lVar23 + 0x90) != 0)) &&
       (*(long *)(*(long *)(lVar23 + 0x90) + 0x218) != 0)) {
      (**(code **)(*plVar22 + 0x40))(plVar22);
    }
  }
  puVar27 = *(undefined8 **)(param_3 + 0xd8);
  for (puVar21 = *(undefined8 **)(param_3 + 0xd0); puVar21 != puVar27; puVar21 = puVar21 + 1) {
    (**(code **)(*(long *)*puVar21 + 0x10))((long *)*puVar21,param_4);
  }
  return 1;
}


// ===== 0x755818 =====
// entry 755818

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void * FUN_00855818(void)

{
  void *pvVar1;
  
  if (_DAT_01a97cf0 == (void *)0x0) {
    pvVar1 = operator_new(0x530);
    FUN_00855a98();
    _DAT_01a97cf0 = pvVar1;
    return pvVar1;
  }
  return _DAT_01a97cf0;
}


