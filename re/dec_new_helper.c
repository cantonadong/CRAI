// ===== 0x12d90ec =====
// entry 12d90ec

undefined8 FUN_013d90ec(long *param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4)

{
  bool bVar1;
  uint uVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  long lVar5;
  long lVar6;
  undefined8 uVar7;
  undefined8 uVar8;
  ulong uVar9;
  void *pvVar10;
  int iVar11;
  uint uVar12;
  undefined8 *puVar13;
  int iVar14;
  long *plVar15;
  undefined8 local_58;
  undefined8 local_38;
  
  lVar5 = FUN_0149c958(param_2,DAT_01b9b9a0,1);
  iVar11 = *(int *)(lVar5 + 0x10);
  if (*(char *)(lVar5 + 8) != '\0') {
    iVar11 = (int)*(float *)(lVar5 + 0xc);
  }
  FUN_012d22c0(*param_1,iVar11);
  lVar5 = *param_1;
  uVar4 = *(undefined4 *)PTR_DAT_01b9aa08;
  uVar3 = FUN_013f7b30(param_2,DAT_01b9b9a8,uVar4,0);
  FUN_012d2c80(lVar5,uVar3);
  uVar4 = FUN_013f7b30(param_2,DAT_01b9b978,uVar4,0);
  *(undefined4 *)((long)param_1 + 0x24) = uVar4;
  lVar5 = *(long *)(*param_1 + 0xa8);
  if (lVar5 == 0) {
    if (*(long *)(*param_1 + 0x220) == 0) {
      FUN_014998d8("replay load without battle or plugin",1);
      uVar9 = FUN_013d9474(param_1,param_2);
    }
    else {
      uVar9 = FUN_013d9474(param_1,param_2);
    }
    if ((uVar9 & 1) == 0) {
      return 0;
    }
  }
  else {
    lVar6 = FUN_0149ca3c(param_2,DAT_01b9b128,1);
    if (lVar6 == 0) {
      FUN_014998d8("cannot find battle object from replay json",1);
    }
    FUN_00f18fe4(lVar5,3);
    thunk_FUN_013d7d10(lVar5,lVar6,param_3,param_4);
  }
  lVar5 = FUN_0149c9f0(param_2,DAT_01b9b980,1);
  if (lVar5 == 0) {
    FUN_014998d8("cannot find cmd array from replay json",1);
  }
  iVar11 = *(int *)(lVar5 + 0x14);
  if (0 < iVar11) {
    iVar14 = 0;
    do {
      uVar7 = FUN_01499d3c(lVar5,iVar14,1);
      uVar8 = FUN_012d48a4(*param_1);
      FUN_00f553e4(&local_38,uVar7,uVar8);
      local_58 = local_38;
      FUN_00f5443c(*(undefined8 *)(*param_1 + 0x38),&local_58);
      iVar14 = iVar14 + 1;
    } while (iVar11 != iVar14);
  }
  uVar9 = FUN_00ef6a4c();
  if (((uVar9 & 1) != 0) || (uVar9 = FUN_00ef6a70(), (uVar9 & 1) != 0)) {
    FUN_00f556b4(*(undefined8 *)(*param_1 + 0x38));
  }
  FUN_013d9740(param_1,param_2);
  lVar5 = FUN_0149c9f0(param_2,DAT_01b9b988,1);
  if (lVar5 == 0) {
    return 1;
  }
  puVar13 = (undefined8 *)param_1[1];
  if (puVar13 != (undefined8 *)0x0) {
    uVar9 = (ulong)*(uint *)((long)puVar13 + 0xc);
    if (0 < (int)*(uint *)((long)puVar13 + 0xc)) {
      do {
        lVar6 = *(long *)(*(long *)param_1[1] + (uVar9 - 1) * 8);
        if (lVar6 != 0) {
          FUN_00f2cff4(lVar6);
                    /* WARNING: Subroutine does not return */
          FUN_0163dd30(lVar6 + 0x10);
        }
        bVar1 = 1 < uVar9;
        uVar9 = uVar9 - 1;
      } while (bVar1);
      puVar13 = (undefined8 *)param_1[1];
      if (puVar13 == (undefined8 *)0x0) goto LAB_013d9360;
    }
    if ((void *)*puVar13 != (void *)0x0) {
      operator_delete__((void *)*puVar13);
    }
    operator_delete(puVar13);
  }
LAB_013d9360:
  uVar2 = *(uint *)(lVar5 + 0x14);
  puVar13 = operator_new(0x10);
  uVar9 = (ulong)uVar2 << 3;
  puVar13[1] = 0;
  *(uint *)(puVar13 + 1) = uVar2;
  if ((int)uVar2 < 0) {
    uVar9 = 0xffffffffffffffff;
  }
  pvVar10 = operator_new__(uVar9);
  *puVar13 = pvVar10;
  param_1[1] = (long)puVar13;
  if (0 < (int)uVar2) {
    uVar12 = 0;
    do {
      lVar6 = FUN_01499d3c(lVar5,uVar12,1);
      if (lVar6 != 0) {
        pvVar10 = operator_new(0x38);
        FUN_00f2cf40();
        FUN_00f2d22c(pvVar10,lVar6);
        plVar15 = (long *)param_1[1];
        iVar11 = *(int *)((long)plVar15 + 0xc);
        if (iVar11 == (int)plVar15[1]) {
          iVar14 = 5;
          if (iVar11 * 2 != 0) {
            iVar14 = iVar11 * 2;
          }
          if (iVar11 < iVar14) {
            FUN_01333384(plVar15);
            iVar11 = *(int *)((long)plVar15 + 0xc);
          }
        }
        *(int *)((long)plVar15 + 0xc) = iVar11 + 1;
        *(void **)(*plVar15 + (long)iVar11 * 8) = pvVar10;
      }
      uVar12 = uVar12 + 1;
    } while (uVar2 != uVar12);
  }
  return 1;
}


// ===== 0x11d3174 =====
// entry 11d3174

undefined8 FUN_012d3174(long param_1)

{
  int iVar1;
  undefined1 uVar2;
  char cVar3;
  long lVar4;
  long *plVar5;
  ulong uVar6;
  int iVar7;
  
  iVar7 = *(int *)(param_1 + 0x34);
  uVar2 = *(undefined1 *)(param_1 + 0x1e8);
  if (iVar7 == *(int *)(param_1 + 0x30)) {
    iVar1 = 5;
    if (iVar7 * 2 != 0) {
      iVar1 = iVar7 * 2;
    }
    if (iVar7 < iVar1) {
      FUN_00e923c0(param_1 + 0x28);
      iVar7 = *(int *)(param_1 + 0x34);
    }
  }
  lVar4 = *(long *)(param_1 + 0xa8);
  *(int *)(param_1 + 0x34) = iVar7 + 1;
  *(undefined1 *)(*(long *)(param_1 + 0x28) + (long)iVar7) = uVar2;
  *(undefined1 *)(param_1 + 0x1e8) = 1;
  if (lVar4 != 0) {
    lVar4 = FUN_00f1142c();
    *(undefined1 *)(lVar4 + 0x79) = 1;
  }
  plVar5 = *(long **)(param_1 + 0x220);
  if (plVar5 != (long *)0x0) {
    (**(code **)(*plVar5 + 0x50))(plVar5,1);
  }
  if (*(long *)(param_1 + 0xa8) == 0) {
LAB_012d3214:
    if (((*(long **)(param_1 + 0x220) != (long *)0x0) && (*(int *)(param_1 + 0x20) == 4)) &&
       ((uVar6 = (**(code **)(**(long **)(param_1 + 0x220) + 0x30))(), (uVar6 & 1) != 0 ||
        (((*(long *)(param_1 + 0x18) != 0 &&
          (iVar7 = *(int *)(*(long *)(param_1 + 0x18) + 0x24), 0 < iVar7)) &&
         (iVar7 <= *(int *)(param_1 + 0x60))))))) goto LAB_012d327c;
  }
  else {
    uVar6 = FUN_00f12820();
    if ((uVar6 & 1) != 0) goto LAB_012d327c;
    if (*(long *)(param_1 + 0xa8) == 0) goto LAB_012d3214;
  }
  FUN_012d32f8(param_1);
  FUN_013e3358(param_1 + 0x60);
  if (*(int *)(param_1 + 0x20) == 4) {
    FUN_013d9a70(*(undefined8 *)(param_1 + 0x18));
  }
LAB_012d327c:
  iVar7 = *(int *)(param_1 + 0x34);
  if (iVar7 == 0) {
    FUN_014998d8("stack empty",1);
    iVar7 = *(int *)(param_1 + 0x34);
  }
  cVar3 = *(char *)(*(long *)(param_1 + 0x28) + (long)iVar7 + -1);
  *(int *)(param_1 + 0x34) = (int)((long)iVar7 + -1);
  *(char *)(param_1 + 0x1e8) = cVar3;
  if (*(long *)(param_1 + 0xa8) != 0) {
    lVar4 = FUN_00f1142c();
    *(char *)(lVar4 + 0x79) = cVar3;
  }
  plVar5 = *(long **)(param_1 + 0x220);
  if (plVar5 != (long *)0x0) {
    (**(code **)(*plVar5 + 0x50))(plVar5,cVar3 != '\0');
  }
  return 0;
}


// ===== 0x13d90ec =====
// entry 13d9044

void FUN_014d9044(long *param_1)

{
  long lVar1;
  size_t sVar2;
  void *pvVar3;
  void *pvVar4;
  undefined8 *puVar5;
  long lVar6;
  ulong uVar7;
  undefined8 *puVar8;
  undefined8 *puVar9;
  ulong uVar10;
  ulong uVar11;
  undefined8 *puVar12;
  undefined8 *puVar13;
  undefined8 *puVar14;
  undefined8 *puVar15;
  undefined8 uVar16;
  undefined8 *puVar17;
  ulong uVar18;
  undefined8 uVar19;
  undefined8 uVar20;
  undefined8 uVar21;
  void *local_58;
  
  if (0x54 < (ulong)param_1[4]) {
    puVar12 = (undefined8 *)param_1[1];
    puVar17 = (undefined8 *)param_1[2];
    param_1[4] = param_1[4] - 0x55;
    puVar13 = puVar12 + 1;
    uVar16 = *puVar12;
    param_1[1] = (long)puVar13;
    if (puVar17 != (undefined8 *)param_1[3]) goto LAB_014d94e4;
    puVar14 = (undefined8 *)*param_1;
    lVar6 = (long)puVar13 - (long)puVar14;
    if (puVar14 <= puVar13 && lVar6 != 0) {
LAB_014d909c:
      lVar6 = lVar6 >> 3;
      lVar1 = lVar6 + 2;
      if (-2 < lVar6) {
        lVar1 = lVar6 + 1;
      }
      sVar2 = (long)puVar17 - (long)puVar13;
      puVar17 = puVar13 + -(lVar1 >> 1);
      if (sVar2 != 0) {
        memmove(puVar17,puVar13,sVar2);
        puVar13 = (undefined8 *)param_1[1];
      }
      puVar17 = (undefined8 *)((long)puVar17 + sVar2);
      param_1[1] = (long)(puVar13 + -(lVar1 >> 1));
      param_1[2] = (long)puVar17;
      goto LAB_014d94e4;
    }
    uVar7 = (long)puVar17 - (long)puVar14 >> 2;
    if ((long)puVar17 - (long)puVar14 == 0) {
      uVar7 = 1;
    }
    if (uVar7 >> 0x3d != 0) goto LAB_014d9584;
    uVar18 = uVar7 >> 2;
    pvVar4 = operator_new(uVar7 * 8);
    puVar15 = (undefined8 *)((long)pvVar4 + uVar18 * 8);
    pvVar3 = (void *)((long)pvVar4 + uVar7 * 8);
    puVar9 = puVar15;
    if ((long)puVar17 - (long)puVar13 != 0) {
      uVar7 = (long)puVar17 + (-0x10 - (long)puVar12);
      puVar9 = (undefined8 *)((long)puVar15 + ((long)puVar17 - (long)puVar13));
      if (uVar7 < 0x48) {
        puVar17 = puVar15;
      }
      else {
        puVar17 = puVar15;
        if (0x1f < (long)pvVar4 + (uVar18 * 8 - (long)puVar12) + -8) {
          puVar5 = (undefined8 *)((long)pvVar4 + uVar18 * 8 + 0x10);
          uVar7 = (uVar7 >> 3) + 1;
          uVar11 = uVar7 & 0x3ffffffffffffffc;
          puVar17 = puVar15 + uVar11;
          puVar13 = puVar13 + uVar11;
          puVar12 = puVar12 + 3;
          uVar18 = uVar11;
          do {
            puVar8 = puVar12 + -1;
            uVar19 = puVar12[-2];
            uVar21 = puVar12[1];
            uVar20 = *puVar12;
            uVar18 = uVar18 - 4;
            puVar12 = puVar12 + 4;
            puVar5[-1] = *puVar8;
            puVar5[-2] = uVar19;
            puVar5[1] = uVar21;
            *puVar5 = uVar20;
            puVar5 = puVar5 + 4;
          } while (uVar18 != 0);
          if (uVar7 == uVar11) goto LAB_014d94d0;
        }
      }
      do {
        puVar12 = puVar17 + 1;
        *puVar17 = *puVar13;
        puVar17 = puVar12;
        puVar13 = puVar13 + 1;
      } while (puVar12 != puVar9);
    }
LAB_014d94d0:
    *param_1 = (long)pvVar4;
    param_1[1] = (long)puVar15;
    param_1[2] = (long)puVar9;
    param_1[3] = (long)pvVar3;
    operator_delete(puVar14);
    puVar17 = (undefined8 *)param_1[2];
LAB_014d94e4:
    *puVar17 = uVar16;
    param_1[2] = param_1[2] + 8;
    return;
  }
  puVar12 = (undefined8 *)param_1[2];
  puVar17 = (undefined8 *)param_1[3];
  puVar13 = (undefined8 *)param_1[1];
  uVar18 = (long)puVar12 - (long)puVar13;
  uVar7 = (long)puVar17 - *param_1;
  if (uVar18 < uVar7) {
    if (puVar17 != puVar12) {
      local_58 = operator_new(0xff0);
      FUN_014d9588(param_1,&local_58);
      return;
    }
    local_58 = operator_new(0xff0);
    FUN_014d9710(param_1,&local_58);
    puVar12 = (undefined8 *)param_1[1];
    puVar17 = (undefined8 *)param_1[2];
    puVar13 = puVar12 + 1;
    uVar16 = *puVar12;
    param_1[1] = (long)puVar13;
    if (puVar17 != (undefined8 *)param_1[3]) goto LAB_014d94e4;
    puVar14 = (undefined8 *)*param_1;
    lVar6 = (long)puVar13 - (long)puVar14;
    if (puVar14 <= puVar13 && lVar6 != 0) goto LAB_014d909c;
    uVar7 = (long)puVar17 - (long)puVar14 >> 2;
    if ((long)puVar17 - (long)puVar14 == 0) {
      uVar7 = 1;
    }
    if (uVar7 >> 0x3d != 0) goto LAB_014d9584;
    uVar18 = uVar7 >> 2;
    pvVar4 = operator_new(uVar7 * 8);
    puVar15 = (undefined8 *)((long)pvVar4 + uVar18 * 8);
    pvVar3 = (void *)((long)pvVar4 + uVar7 * 8);
    puVar9 = puVar15;
    if ((long)puVar17 - (long)puVar13 != 0) {
      uVar7 = (long)puVar17 + (-0x10 - (long)puVar12);
      puVar9 = (undefined8 *)((long)puVar15 + ((long)puVar17 - (long)puVar13));
      if (uVar7 < 0x48) {
        puVar17 = puVar15;
      }
      else {
        puVar17 = puVar15;
        if (0x1f < (long)pvVar4 + (uVar18 * 8 - (long)puVar12) + -8) {
          puVar5 = (undefined8 *)((long)pvVar4 + uVar18 * 8 + 0x10);
          uVar7 = (uVar7 >> 3) + 1;
          uVar11 = uVar7 & 0x3ffffffffffffffc;
          puVar17 = puVar15 + uVar11;
          puVar13 = puVar13 + uVar11;
          puVar12 = puVar12 + 3;
          uVar18 = uVar11;
          do {
            puVar8 = puVar12 + -1;
            uVar19 = puVar12[-2];
            uVar21 = puVar12[1];
            uVar20 = *puVar12;
            uVar18 = uVar18 - 4;
            puVar12 = puVar12 + 4;
            puVar5[-1] = *puVar8;
            puVar5[-2] = uVar19;
            puVar5[1] = uVar21;
            *puVar5 = uVar20;
            puVar5 = puVar5 + 4;
          } while (uVar18 != 0);
          if (uVar7 == uVar11) goto LAB_014d94d0;
        }
      }
      do {
        puVar12 = puVar17 + 1;
        *puVar17 = *puVar13;
        puVar17 = puVar12;
        puVar13 = puVar13 + 1;
      } while (puVar12 != puVar9);
    }
    goto LAB_014d94d0;
  }
  uVar7 = (long)uVar7 >> 2;
  if (puVar17 == (undefined8 *)*param_1) {
    uVar7 = 1;
  }
  if (uVar7 >> 0x3d != 0) {
LAB_014d9584:
                    /* WARNING: Subroutine does not return */
    FUN_008cabc0();
  }
  puVar14 = operator_new(uVar7 * 8);
  puVar17 = (undefined8 *)((long)puVar14 + uVar18);
  pvVar3 = operator_new(0xff0);
  if (uVar18 == uVar7 * 8) {
    if (0 < (long)uVar18) {
      puVar15 = (undefined8 *)((long)puVar17 - ((uVar18 >> 1) + 4 & 0xfffffffffffffff8));
      *puVar15 = pvVar3;
      puVar9 = puVar17;
      puVar17 = puVar15;
      goto joined_r0x014d9280;
    }
    uVar18 = (long)uVar18 >> 2;
    if (puVar12 == puVar13) {
      uVar18 = 1;
    }
    if (uVar18 >> 0x3d != 0) goto LAB_014d9584;
    puVar5 = operator_new(uVar18 * 8);
    puVar9 = puVar5 + uVar18;
    puVar17 = puVar5 + (uVar18 >> 2);
    operator_delete(puVar14);
    puVar13 = (undefined8 *)param_1[1];
    puVar12 = (undefined8 *)param_1[2];
    puVar15 = puVar17 + 1;
    *puVar17 = pvVar3;
    puVar14 = puVar5;
    if (puVar12 != puVar13) goto LAB_014d932c;
  }
  else {
    puVar9 = puVar14 + uVar7;
    *puVar17 = pvVar3;
joined_r0x014d9280:
    puVar15 = puVar17 + 1;
    puVar5 = puVar14;
    if (puVar12 != puVar13) {
LAB_014d932c:
      do {
        if (puVar17 == puVar5) {
          if (puVar15 < puVar9) {
            lVar6 = (long)puVar9 - (long)puVar15 >> 3;
            uVar7 = lVar6 + 2;
            if (-2 < lVar6) {
              uVar7 = lVar6 + 1;
            }
            lVar6 = (long)puVar15 - (long)puVar17;
            sVar2 = (long)puVar15 - (long)puVar17;
            puVar15 = puVar15 + (uVar7 >> 1);
            puVar13 = (undefined8 *)((long)puVar15 - lVar6);
            puVar5 = puVar17;
            if (sVar2 == 0) {
              puVar17 = puVar13;
            }
            else {
              memmove(puVar13,puVar17,sVar2);
              puVar17 = puVar13;
            }
          }
          else {
            uVar7 = (long)puVar9 - (long)puVar17 >> 2;
            if ((long)puVar9 - (long)puVar17 == 0) {
              uVar7 = 1;
            }
            if (uVar7 >> 0x3d != 0) goto LAB_014d9584;
            uVar18 = uVar7 + 3 >> 2;
            puVar5 = operator_new(uVar7 * 8);
            puVar13 = puVar5 + uVar18;
            lVar6 = (long)puVar15 - (long)puVar17;
            puVar15 = puVar13;
            if (lVar6 != 0) {
              puVar15 = (undefined8 *)((long)puVar13 + lVar6);
              puVar14 = puVar13;
              puVar9 = puVar17;
              if ((0x37 < lVar6 - 8U) && (0x1f < (long)puVar5 + (uVar18 * 8 - (long)puVar17))) {
                puVar14 = puVar17 + 2;
                puVar9 = puVar5 + uVar18 + 2;
                uVar18 = (lVar6 - 8U >> 3) + 1;
                uVar10 = uVar18 & 0x3ffffffffffffffc;
                uVar11 = uVar10;
                do {
                  puVar8 = puVar14 + -1;
                  uVar16 = puVar14[-2];
                  uVar20 = puVar14[1];
                  uVar19 = *puVar14;
                  uVar11 = uVar11 - 4;
                  puVar14 = puVar14 + 4;
                  puVar9[-1] = *puVar8;
                  puVar9[-2] = uVar16;
                  puVar9[1] = uVar20;
                  *puVar9 = uVar19;
                  puVar9 = puVar9 + 4;
                } while (uVar11 != 0);
                puVar14 = puVar13 + uVar10;
                puVar9 = puVar17 + uVar10;
                if (uVar18 == uVar10) goto LAB_014d9448;
              }
              do {
                puVar8 = puVar14 + 1;
                *puVar14 = *puVar9;
                puVar14 = puVar8;
                puVar9 = puVar9 + 1;
              } while (puVar8 != puVar15);
            }
LAB_014d9448:
            puVar9 = puVar5 + uVar7;
            operator_delete(puVar17);
            puVar17 = puVar13;
          }
        }
        puVar12 = puVar12 + -1;
        puVar17 = puVar17 + -1;
        *puVar17 = *puVar12;
      } while (puVar12 != (undefined8 *)param_1[1]);
      goto LAB_014d92e4;
    }
  }
  puVar5 = puVar14;
LAB_014d92e4:
  pvVar3 = (void *)*param_1;
  *param_1 = (long)puVar5;
  param_1[1] = (long)puVar17;
  param_1[2] = (long)puVar15;
  param_1[3] = (long)puVar9;
  if (pvVar3 == (void *)0x0) {
    return;
  }
  operator_delete(pvVar3);
  return;
}


