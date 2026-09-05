# Порт Gamesense-Legacy → CS:GO сборка 13.12.2016

## 1. Идентификация сборки
- client.dll / engine.dll: PE32, TimeDateStamp **13.12.2016**
- Остальные DLL: 07–13.12.2016
- Проверка: **34/36 сигнатур hazedumper** из коммита `ab62680` («Game Update 13.12.2016») матчатся → сборка подтверждена
- Интерфейсы менять НЕ нужно: VClient018, VEngineClient014, VGUI_Surface031, VGUI_Panel009, VEngineCvar007, GAMEEVENTSMANAGER002, VEngineModel016, VClientEntityList003, VClientPrediction001, GameMovement001 — все присутствуют

## 2. Статистика сигнатур (156 сканов)
| Модуль | OK | FAIL | Из FAIL исправлено | Занулено (guarded) |
|---|---|---|---|---|
| client.dll | 67 | 53 | 12 | остальные (неиспользуемые/отсутствующие в 2016) |
| engine.dll | 15 | 9 | 2 (sendmove, +CL_FireEvents → non-fatal) | остальные |
| vguimatsurface / shaderapidx9 | 5 | 0 | — | — |
| server.dll | — | — | — | бинарь не предоставлен (7 сканов не проверяемы) |

## 3. Заменённые сигнатуры (проверены по бинарям)
| Что | Новый паттерн | Адрес (RVA) |
|---|---|---|
| m_nCachedBonesPosition | `8D 87 ? ? ? ? 50 E8 ? ? ? ? 8B 44 24 1C` (+2, без +4) | 0x1CE093 → 0xA5C |
| m_nCachedBonesRotation | `8D 87 ? ? ? ? 50 E8 ? ? ? ? 83 C4 0C` (+2, без +4) | 0x1CE0BA → 0x165C |
| m_CachedBoneData | прежний якорь, **без +4** | 0x1CE103 → 0x28FC |
| m_pStudioHdr | `8B 86 ? ? ? ? 89 44 24 0C 85 C0 74 05` (+2, без +4) | 0x19EABE → 0x293C |
| m_uInput | `B9 ? ? ? ? FF 75 08 E8 ? ? ? ? 8B 06` (+1) | 0x253871 |
| m_uGlowObjectManager | `A1 ? ? ? ? A8 01 75 4E 0F 57 C0` (+1) | 0x2AD000 |
| m_uUpdateAnimState | `55 8B EC 83 EC 1C 56 57 8B F9 F3 0F 11 55 F8 F3 0F 11 4D F4` | 0x3E6120 |
| m_nUpdateCache (BoneMerge) | `FF 75 08 8B 8E ? ? ? ? E8 ? ? ? ? 5E 8B E5 5D C2 04 00` (+9→rel) | 0x1CA4CB → 0x1BB400 |
| m_SetCollisionBounds | `53 8B DC 83 EC 08 83 E4 F8 83 C4 04 55 8B 6B 04 89 6C 24 04 8B EC 83 EC` | 0x66CC10 |
| WeaponSystem | `8B 0D ? ? ? ? 8B 01 8B 77 14 FF 10 0F B7 C0` (+2) | 0x495B60 |
| ViewMatrix | `F3 0F 6F 05 ? ? ? ? 8D 85` (+4, без +176) | 0x217049 |
| sendmove (WUCMD hook) | `84 C0 74 04 B0 01 EB 02 32 C0 8B FE 46 3B 75 FC 7E C8 84 C0` | engine 0xE40E6 |

## 4. Хардкоды, исправленные под 2016
- Бон-снапшоты в `C_CSPlayer::SetupBones`: 0x39F0/0x6E40 → **0x3A00/0x6E50**
- `m_pCurrentCommand` 0x3338 → 0 (не резолвится, запись отключена гардом)

## 5. Отключено гардами (функций нет в сборке 13.12.2016)
- DoProceduralFootPlant (появился позже)
- ShouldSkipAnimationFrame (2019+)
- SimulatePlayerSimulatedEntities (EnginePrediction)
- ModifyEyePos (анти-аим концепт 2019+)
- PostProcessParameters (_removepostprocess_)
- IK-эмуляция (CIKContext-цепочка), m_bShouldDraw, FindHudElement (skin changer HUD-иконка)
- CL_FireEvents — не фатально (события идут через ProcessTempEntities)
- sv_cheats bypass / server impacts / auto-accept (безопасно возвращают 0)

## 6. Этап 2 (продолжение порта)

### 6.1 Лейаут CCSGOPlayerAnimState портирован на 2016 ✅
Полностью дизассемблированы `Reset` (0x3E5CB0) и `Update` (0x3E6120). Установлено:
- Поля **0x00..0x14F идентичны** реплике из лейка (проверено по якорям: m_pPlayer@0x60,
  переходы aimmatrix@0x14/0x28/0x3C, LandAnimMultiplier=1.0@0x118, вектор ActivityModifiers@0x13C)
- 2016-сборка содержит **2 лишних dword** в блоке ускорений (после m_flTimeOfLastKnownInjury)
  → всё после сдвинуто на **+8** (вставлен m_flUnknown2016[2])
- m_flLastTimeVelocityOverTen и m_nAnimstateModelVersion в 2016 **отсутствуют** → удалены
- Итог: pose-кэш 20×12 @0x1B0, StaticApproachSpeed=80 @0x2A4, aim поля @0x32C..0x338 ✓
- Конвенция Update(pitch xmm2, yaw xmm1, time stack) совпадает с GL-вызовом ✓
- m_PlayerAnimState = 0x3894 (резолвится原有 паттерном) ✓

### 6.2 Исправлены скрытые краши CallableFromRelative(0) ✅
- Data.m_ModifyEyePos, ShouldSkipAnimationFrame, LookupBone — теперь с null-проверкой до разыменования

### 6.3 Прочее
- server-оффсеты (m_uServerGlobals и др.) объявлены, но нигде не используются — гарды не нужны
- Player.cpp::138 (EyeAngles base) — паттерн матчится ✓

## 7. ⚠️ Оставшиеся ограничения
1. m_pCurrentCommand не найден (0x2EC4-кандидаты оказались viewmodel-фабрикой) —
   предикшн-реплика пишет мимо, гард отключает запись; игровой предикшн не страдает
2. server.dll отсутствует → 4 серверных скана не проверены (не используются)
3. SkinChanger: иконки китов отключены (econ-схема 2016 отличается), базовая замена скинов работает
4. Разрешение CL_FireEvents/SendDatagram/ReadSubChannelData не критично (события идут через ProcessTempEntities)
5. Требуется сборка в VS (m_dev|Win32) и живое тестирование в игре

## 8. Инструменты (tools/port/)
- `extract_patterns.py` — выгрузка всех сигнатур из кода → patterns.json
- `pe_scan.py` — сканер PE с вайлдкардами + дамп интерфейсных строк
- `disx.py` — дизассемблер-помощник (capstone): dis/findstr/xrefs
- `matches.json` — все совпадения по 156 паттернам

## 9. Этап 3: server.dll (13.12.2016) проверен ✅
Пользователь предоставил server.dll (timedate 13.12.2016 18:39, родная сборка).
- m_uServerGlobals: `8B 15 ? ? ? ? 33 C9 83 7A 18 01` → OK (оба хита ведут на один глобал 0x1092F8C4)
- m_uServerPoseParameters: OK (0x1AD8B7)
- m_uServerAnimState: OK (0x42BAAC)
- ServerAnimations.cpp:115 (CalcPoseOperator host) → OK (0x19F050)
- m_uTicksAllowed: FAIL → занулён (нигде не используется)
- ActivityModifiersWrapper::AddActivityModifier: FAIL → гард (нигде не вызывается)

«Серверные анимации» в этом чите — клиентская эмуляция серверного CCSGOPlayerAnimState
(LBY-предсказание), server.dll в рантайме не нужен. Индексы анимационных слоёв
(enum animstate_layer_t в sdk.hpp) уже в до-2019 формате из 13 слоёв — совпадает с 2016.
