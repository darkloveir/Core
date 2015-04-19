/*
 * Copyright (C) 2008-2015 TrinityCore <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ScriptPCH.h"
#include "ScriptedEscortAI.h"
#include "trial_of_the_champion.h"
#include "Vehicle.h"

enum Talk
{
   SAY_INTRO_1              = 0, 
   SAY_INTRO_2              = 56,
   SAY_INTRO_3              = 1,
   SAY_INTRO_4              = 2,
   SAY_AGGRO                = 3,
   SAY_AGGRO_OUTRO          = 51,
   SAY_KILLED_PLAYER        = 6,
   SAY_PHASE_1              = 4,
   SAY_PHASE_2              = 5,
   SAY_DEATH                = 7,
};


enum Spells
{
    //phase 1
    SPELL_PLAGUE_STRIKE     = 67724,
    SPELL_ICY_TOUCH         = 67718,
    SPELL_ICY_TOUCH_H       = 67881,
    SPELL_DEATH_RESPITE     = 67745,
    SPELL_DEATH_RESPITE_H   = 68306,
    SPELL_OBLITERATE        = 67725,
    SPELL_OBLITERATE_H      = 67883,
    ZOMBIE_JAEREN           = 67715,
    ZOMBIE_ARELAS           = 67705,
    KILL_HERALD             = 66804,
    SPELL_RESPITE_HERALD    = 66798,

    //phase 2
    SPELL_ARMY_DEAD         = 67761,
    SPELL_DESECRATION       = 68766,
    SPELL_GHOUL_EXPLODE     = 67751,

    //phase 3
    SPELL_DEATH_BITE        = 67808,
    SPELL_DEATH_BITE_H      = 67875,
    SPELL_MARKED_DEATH      = 67882,

    SPELL_BLACK_KNIGHT_RES  = 67693,

    SPELL_LEAP              = 67749,
    SPELL_LEAP_H            = 67880,
    SPELL_CLAW              = 67774,
    SPELL_CLAW_H            = 67879,

    GHOUL_EXPLODE_DAMAGE    = 67729,
    H_GHOUL_EXPLODE_DAMAGE  = 67886,

    SPELL_KILL_CREDIT       = 68663,

    SPELL_MOUNT_VEHICLE     = 46598
};

enum Models
{
    MODEL_SKELETON          = 29846,
    MODEL_GHOST             = 21300
};

enum Equip
{
    EQUIP_SWORD            = 40343
};

enum IntroPhase
{
    IDLE,
    INTRO,
    NORMAL,
    FINISHED,
};

enum Phases
{
    PHASE_UNDEAD            = 3,
    PHASE_SKELETON          = 4,
    PHASE_GHOST             = 5,

};

enum Creatures
{
    CREATURE_HIGHLORD       = 34996,
    CREATURE_ANNOUNCER      = 35004,
};

const Position MoveKnightPos = {746.993286f, 622.990784f, 411.417237f, 4.712464f};

class boss_black_knight : public CreatureScript
{
public:
    boss_black_knight() : CreatureScript("boss_black_knight") { }

    struct boss_black_knightAI : public ScriptedAI
    {
        boss_black_knightAI(Creature* creature) : ScriptedAI(creature)
        {
            Initialize();
            instance = creature->GetInstanceScript();
            Phase = IDLE;
            bCredit = false;
        }

        void Initialize()
        {
            uiPhase = PHASE_UNDEAD;
            uiIcyTouchTimer = urand(5000, 9000);
            uiIcyTouch1Timer = urand(15000, 15000);
            uiPlagueStrikeTimer = urand(10000, 13000);
            uiDeathRespiteTimer = 17000;
            uiPlagueStrike1Timer = urand(14000, 14000);
            uiObliterateTimer = urand(17000, 19000);
            uiObliterate1Timer = urand(15000, 15000);
            uiDesecrationTimer = urand(15000, 16000);
            uiDesecrationTimer = 22000;
            uiDeathArmyCheckTimer = 7000;
            uiResurrectTimer = 4000;
            uiGhoulExplodeTimer = 8000;
            uiDeathBiteTimer = urand (2000, 4000);
            uiMarkedDeathTimer = urand (5000, 7000);
            uiIntroTimer = 15000;
            uiIntroPhase = 0;
            Phase = INTRO;

            bEventInProgress = false;
            bEvent = false;
            bSummonArmy = false;
            bDeathArmyDone = false;
            bFight = false;
            iveHadWorse = true;
            announcer = 0;
        }

        InstanceScript* instance;

        GuidList SummonList;

        bool bEventInProgress;
        bool bEvent;
        bool bSummonArmy;
        bool bDeathArmyDone;
        bool bEventInBattle;
        bool bFight;
        bool bCredit;
        bool iveHadWorse;

        uint8 uiPhase;
        uint8 uiIntroPhase;

        Creature* highlord;
        Creature* announcer;

        IntroPhase Phase;

        uint32 uiIntroTimer;
        uint32 uiPlagueStrikeTimer;
        uint32 uiPlagueStrike1Timer;
        uint32 uiIcyTouchTimer;
        uint32 uiIcyTouch1Timer;
        uint32 uiDeathRespiteTimer;
        uint32 uiObliterateTimer;
        uint32 uiObliterate1Timer;
        uint32 uiDesecrationTimer;
        uint32 uiResurrectTimer;
        uint32 uiDeathArmyCheckTimer;
        uint32 uiGhoulExplodeTimer;
        uint32 uiDeathBiteTimer;
        uint32 uiMarkedDeathTimer;

        void Reset() override
        {
            RemoveSummons();
            me->SetDisplayId(me->GetNativeDisplayId());
            me->ClearUnitState(UNIT_STATE_ROOT | UNIT_STATE_STUNNED);

            Initialize();

            if (GameObject* go = ObjectAccessor::GetGameObject(*me, instance->GetGuidData(DATA_MAIN_GATE)))
                instance->HandleGameObject(go->GetGUID(), false);

            if (GameObject* go = ObjectAccessor::GetGameObject(*me, instance->GetGuidData(DATA_MAIN_GATE1)))
                instance->HandleGameObject(go->GetGUID(), true);

            if (bEventInBattle)
            {
                me->GetMotionMaster()->MovePoint(1, 743.396f, 635.411f, 411.575f);
                me->setFaction(14);
                me->SetReactState(REACT_AGGRESSIVE);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            }
        }

        void MoveInLineOfSight(Unit* who) override
        {
            if (!who)
                return;

            if (Phase == IDLE && me->IsValidAttackTarget(who) && me->IsWithinDistInMap(who, 200.0f))
            {
                me->SetReactState(REACT_PASSIVE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                Phase = INTRO;
            }
        }

        void RemoveSummons()
        {
            if (SummonList.empty())
                return;

            for (GuidList::const_iterator itr = SummonList.begin(); itr != SummonList.end(); ++itr)
            {
                if (Creature* temp = ObjectAccessor::GetCreature(*me, *itr))
                {
                    if (temp)
                    {
                        if ((temp->GetEntry() == RISEN_CHAMPION || temp->GetEntry() == 12444) && temp->IsAlive())
                        {
                            me->CastSpell(temp, SPELL_GHOUL_EXPLODE, true);
                        }
                        else
                            temp->DisappearAndDie();
                    }
                }
            }
            SummonList.clear();
        }

        void JustSummoned(Creature* summon) override
        {
            SummonList.push_back(summon->GetGUID());
        }

        void UpdateAI(uint32 uiDiff) override
        {
            if (Phase == IDLE)
                return;

            if (Phase == INTRO)
            {
                if (uiIntroTimer <= uiDiff)
                {
                    switch (uiIntroPhase)
                    {
                        case 0:
                            ++uiIntroPhase;
                            uiIntroTimer = 3000;
                            break;
                        case 1: // MEH
                            ++uiIntroPhase;
                            uiIntroTimer = 2000;
                            break;
                        case 2:
                            if (Creature* announcer = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_ANNOUNCER)))
                                announcer->DisappearAndDie();

                            me->SetUnitMovementFlags(MOVEMENTFLAG_WALKING);
                            me->GetMotionMaster()->MovePoint(0, MoveKnightPos);
                            ++uiIntroPhase;
                            uiIntroTimer = 2000;
                            break;
                        case 3:
                            Talk(SAY_INTRO_3);
                            ++uiIntroPhase;
                            uiIntroTimer = 6000;
                            break;
                        case 4:
                            Talk(SAY_INTRO_4);
                            ++uiIntroPhase;
                            uiIntroTimer = 3000;
                            break;
                        case 5:
                            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NON_ATTACKABLE);
                            me->SetReactState(REACT_AGGRESSIVE);
                            ++uiIntroPhase;
                            uiIntroTimer = 3000;
                            if (Unit* unit = me->SelectNearestTarget())
                                AttackStart(unit);
                            DoZoneInCombat();
                            Phase = NORMAL;
                            break;
                    }
                } 
                else
                    uiIntroTimer -= uiDiff;
                return;
            }

            if (!UpdateVictim() || me->HasUnitMovementFlag(MOVEMENTFLAG_ONTRANSPORT) || me->GetVehicle())
                return;

            if (bEventInProgress)
            {
                if (uiResurrectTimer <= uiDiff)
                {
                    me->SetFullHealth();
                    me->AttackStop();

                    switch (uiPhase)
                    {
                        case PHASE_UNDEAD:
                            Talk(SAY_PHASE_1);
                            break;
                        case PHASE_SKELETON:
                            Talk(SAY_PHASE_2);
                            break;
                    }

                    DoCast(me, SPELL_BLACK_KNIGHT_RES, true);
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                    uiPhase++;
                    uiResurrectTimer = 3000;
                    bEventInProgress = false;
                    me->ClearUnitState(UNIT_STATE_ROOT | UNIT_STATE_STUNNED);
                } else uiResurrectTimer -= uiDiff;
            }

            switch (uiPhase)
            {
                case PHASE_UNDEAD:
                {
                    if (uiPlagueStrikeTimer <= uiDiff)
                    {
                        DoCastVictim(SPELL_PLAGUE_STRIKE);
                        uiPlagueStrikeTimer = urand(12000, 15000);
                    } else uiPlagueStrikeTimer -= uiDiff;
                    
                    if (uiObliterateTimer <= uiDiff)
                    {
                        DoCastVictim(DUNGEON_MODE(SPELL_OBLITERATE, SPELL_OBLITERATE_H));
                        uiObliterateTimer = urand(17000, 19000);
                    } else uiObliterateTimer -= uiDiff;
                    
                    if (uiIcyTouchTimer <= uiDiff)
                    {
                        DoCastVictim(DUNGEON_MODE(SPELL_ICY_TOUCH, SPELL_ICY_TOUCH_H));
                        uiIcyTouchTimer = urand(5000, 7000);
                    } else uiIcyTouchTimer -= uiDiff;
                    break;
                }
                case PHASE_SKELETON:
                {
                    if (!bSummonArmy)
                    {
                        bSummonArmy = true;
                        me->AddUnitState(UNIT_STATE_ROOT | UNIT_STATE_STUNNED);
                        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                        DoCast(me, SPELL_ARMY_DEAD);
                    }

                    if (!bDeathArmyDone)
                    {
                        if (uiDeathArmyCheckTimer <= uiDiff)
                        {
                            me->ClearUnitState(UNIT_STATE_ROOT | UNIT_STATE_STUNNED);
                            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                            uiDeathArmyCheckTimer = 0;
                            bDeathArmyDone = true;
                        } else uiDeathArmyCheckTimer -= uiDiff;
                    }
                    
                    if (uiDesecrationTimer <= uiDiff)
                    {
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                        {
                            if (target->IsAlive())
                                DoCast(target, SPELL_DESECRATION);
                        }
                        uiDesecrationTimer = urand(15000, 16000);
                    } else uiDesecrationTimer -= uiDiff;
                        
                    if (uiGhoulExplodeTimer <= uiDiff)
                    {
                        if (!SummonList.empty())
                        {
                            for (GuidList::const_iterator itr = SummonList.begin(); itr != SummonList.end(); ++itr)
                            {
                                if (Creature* temp = ObjectAccessor::GetCreature(*me, *itr))
                                {
                                    if (temp)
                                    {
                                        if (temp->GetEntry() == RISEN_CHAMPION || temp->GetEntry() == 12444)
                                        {
                                            if (temp->IsAlive())
                                            {
                                                me->CastSpell(temp, SPELL_GHOUL_EXPLODE, true);
                                                break;
                                            }
                                            else
                                                continue;
                                        }
                                    }
                                }
                            }
                        }

                        uiGhoulExplodeTimer = 8000;
                    } else uiGhoulExplodeTimer -= uiDiff;
                    
                    if (uiPlagueStrike1Timer <= uiDiff)
                    {
                        DoCastVictim(SPELL_PLAGUE_STRIKE);
                        uiPlagueStrike1Timer = urand(12000, 15000);
                    } else uiPlagueStrike1Timer -= uiDiff;
                    
                    if (uiObliterate1Timer <= uiDiff)
                    {
                        DoCastVictim(SPELL_OBLITERATE);
                        uiObliterate1Timer = urand(17000, 19000);
                    } else uiObliterate1Timer -= uiDiff;
                    
                    if (uiIcyTouch1Timer <= uiDiff)
                    {
                        DoCastVictim(SPELL_ICY_TOUCH);
                        uiIcyTouch1Timer = urand(5000, 7000);
                    } else uiIcyTouch1Timer -= uiDiff;
                    
                    if (uiDeathRespiteTimer <= uiDiff)
                    {
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                        {
                            if (target && target->IsAlive())
                            DoCast(target, SPELL_DEATH_RESPITE);
                        }
                        uiDeathRespiteTimer = urand(15000, 16000);
                    } else uiDeathRespiteTimer -= uiDiff;
                    break;
                }

                case PHASE_GHOST:
                {
                    if (uiDeathBiteTimer <= uiDiff)
                    {
                        SetEquipmentSlots(false, EQUIP_UNEQUIP, EQUIP_NO_CHANGE, EQUIP_NO_CHANGE);
                        DoCast(me, DUNGEON_MODE(SPELL_DEATH_BITE, SPELL_DEATH_BITE_H));
                        uiDeathBiteTimer = urand (2000, 4000);
                    } else uiDeathBiteTimer -= uiDiff;
                
                    if (uiMarkedDeathTimer <= uiDiff)
                    {
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                        {
                            if (target && target->IsAlive())
                                DoCast(target, SPELL_MARKED_DEATH);
                        }
                        uiMarkedDeathTimer = urand (10000, 12000);
                    } else uiMarkedDeathTimer -= uiDiff;
                    break;
                }
            }

            if (!me->HasUnitState(UNIT_STATE_ROOT) && !me->HealthBelowPct(1))
                DoMeleeAttackIfReady();
        }

        void EnterCombat(Unit* /*who*/) override
        {
            bEventInBattle = true;
            Talk(SAY_AGGRO);

            SetEquipmentSlots(false, EQUIP_SWORD, EQUIP_NO_CHANGE, EQUIP_NO_CHANGE);

            if (instance->GetData(DATA_TEAM_IN_INSTANCE) == HORDE)
            {
                DoCast(me, ZOMBIE_JAEREN);
                if (Creature* garrosh = me->FindNearestCreature(NPC_GARROSH, 200.0f))
                    garrosh->AI()->Talk(SAY_AGGRO_OUTRO);
            }
            else
            {
                DoCast(me, ZOMBIE_ARELAS);
                if (Creature* varian = me->FindNearestCreature(NPC_VARIAN, 200.0f))
                    varian->AI()->Talk(SAY_AGGRO_OUTRO);
            }

            if (GameObject* gate = ObjectAccessor::GetGameObject(*me, instance->GetGuidData(DATA_MAIN_GATE1)))
                instance->HandleGameObject(gate->GetGUID(), false);

            if (GameObject* gate = ObjectAccessor::GetGameObject(*me, instance->GetGuidData(DATA_MAIN_GATE)))
                instance->HandleGameObject(gate->GetGUID(), false);

            instance->SetData(BOSS_BLACK_KNIGHT, IN_PROGRESS);
        }

        void KilledUnit(Unit* /*victim*/) override
        {
            Talk(SAY_KILLED_PLAYER);
        }

        void DamageTaken(Unit* /*who*/, uint32& damage) override
        {
            if (damage >= me->GetHealth() && uiPhase <= PHASE_SKELETON)
            {
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                damage = 0;
                me->SetHealth(0);
                me->AddUnitState(UNIT_STATE_ROOT | UNIT_STATE_STUNNED);
                RemoveSummons();
                switch (uiPhase)
                {
                    case PHASE_UNDEAD:
                        me->SetDisplayId(MODEL_SKELETON);
                        break;
                    case PHASE_SKELETON:
                        me->SetDisplayId(MODEL_GHOST);
                        SetEquipmentSlots(false, EQUIP_UNEQUIP, EQUIP_NO_CHANGE, EQUIP_NO_CHANGE);
                        break;
                }
                bEventInProgress = true;
            }
            else if (damage >= me->GetHealth() && uiPhase == PHASE_GHOST && !bCredit)
            {
                bCredit = true;
                HandleSpellOnPlayersInInstanceToC5(me, 68663);
            }
        }

        uint32 GetData(uint32 type) const override
        {
            if (type == DATA_IVE_HAD_WORSE)
                return iveHadWorse;

            return 0;
        }

        void SetData(uint32 uiType, uint32 uiData) override
        {
            if (uiType == DATA_IVE_HAD_WORSE)
                iveHadWorse = uiData;
        }

        void JustDied(Unit* /*killer*/) override
        {
            DoCast(me, SPELL_KILL_CREDIT);
            Talk(SAY_DEATH);
            
            if (TempSummon* summ = me->ToTempSummon())
                summ->SetTempSummonType(TEMPSUMMON_DEAD_DESPAWN);

            instance->SetData(BOSS_BLACK_KNIGHT, DONE);
            instance->DoCastSpellOnPlayers(SPELL_KILL_CREDIT);

            if (GameObject* gate = ObjectAccessor::GetGameObject(*me, instance->GetGuidData(DATA_MAIN_GATE1)))
                instance->HandleGameObject(gate->GetGUID(), true);
        }
        private:
            EventMap _events;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return GetTrialOfTheChampionAI<boss_black_knightAI>(creature);
    }

};

class npc_risen_ghoul : public CreatureScript
{
public:
    npc_risen_ghoul() : CreatureScript("npc_risen_ghoul") { }

    struct npc_risen_ghoulAI : public ScriptedAI
    {
        npc_risen_ghoulAI(Creature* creature) : ScriptedAI(creature) 
        {
            Initialize();
        }

        void Initialize()
        {
            uiAttackTimer = 3500;
        }

        uint32 uiAttackTimer;

        InstanceScript* instance;

        void Reset() override
        {
            Initialize();   
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            if (uiAttackTimer <= diff)
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 100, true))
                {
                    if (target && target->IsAlive())
                        DoCast(target, DUNGEON_MODE(SPELL_LEAP, SPELL_LEAP_H));
                }
                uiAttackTimer = 3500;
            } else uiAttackTimer -= diff;

            DoMeleeAttackIfReady();
        }

        void SpellHitTarget(Unit* target, const SpellInfo* spell) override
        {
            if (target->GetTypeId() == TYPEID_PLAYER && (spell->Id == GHOUL_EXPLODE_DAMAGE || spell->Id == H_GHOUL_EXPLODE_DAMAGE || spell->Id == SPELL_GHOUL_EXPLODE))
                if (Creature* knight = me->FindNearestCreature(NPC_BLACK_KNIGHT, 200.0f))
                    knight->AI()->SetData(DATA_IVE_HAD_WORSE, false);
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return GetTrialOfTheChampionAI<npc_risen_ghoulAI>(creature);
    }
};

class npc_risen_announcer : public CreatureScript
{
public:
    npc_risen_announcer() : CreatureScript("npc_risen_announcer") { }

    struct npc_risen_announcerAI : public ScriptedAI
    {
        npc_risen_announcerAI(Creature* creature) : ScriptedAI(creature) 
        {
            Initialize();
            me->setFaction(14);
        }

        void Initialize()
        {
            uiLeapTimer = 10000;
            uiClawTimer = 1000;
        }

        uint32 uiLeapTimer;
        uint32 uiClawTimer;

        void Reset() override
        {
            Initialize();
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            if (uiLeapTimer <= diff)
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 100, true))
                {
                    if (target && target->IsAlive())
                        DoCast(target, DUNGEON_MODE(SPELL_LEAP, SPELL_LEAP_H));
                }
                uiLeapTimer = 10000;
            } else uiLeapTimer -= diff;

            if (uiClawTimer <= diff)
            {
                DoCastVictim(DUNGEON_MODE(SPELL_CLAW, SPELL_CLAW_H));
                uiClawTimer = 1000;
            } else uiClawTimer -= diff;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return GetTrialOfTheChampionAI<npc_risen_announcerAI>(creature);
    }
};

class npc_black_knight_skeletal_gryphon : public CreatureScript
{
public:
    npc_black_knight_skeletal_gryphon() : CreatureScript("npc_black_knight_skeletal_gryphon") { }

    struct npc_black_knight_skeletal_gryphonAI : public npc_escortAI
    {
        npc_black_knight_skeletal_gryphonAI(Creature* creature) : npc_escortAI(creature)
        {
            Initialize();
            Start(false, true);
            instance = creature->GetInstanceScript();
        }

        void Initialize()
        {
            highlord = 0;
        }

        Creature* highlord;

        InstanceScript* instance;

        void Reset() override
        {
            Initialize();
        }

        void WaypointReached(uint32 uiPointId) override
        {
            switch (uiPointId)
            {
                case 1:
                    me->SetSpeed(MOVE_FLIGHT, 2.0f);
                    break;
                case 2:
                    me->SetSpeed(MOVE_FLIGHT, 2.0f);
                    if (Creature* blackknight = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_BLACK_KNIGHT)))
                        blackknight->AI()->Talk(SAY_INTRO_1);
                    break;
                case 3:
                case 4:
                case 5:
                    me->SetSpeed(MOVE_FLIGHT, 2.0f);
                    break;
                case 6:
                    me->SetSpeed(MOVE_FLIGHT, 2.0f);
                    if (Creature* tirion = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_HIGHLORD)))
                        tirion->AI()->Talk(SAY_INTRO_2);
                    break;
                case 7:
                case 8:
                case 9:
                    me->SetSpeed(MOVE_FLIGHT, 2.0f);
                    break;
                case 10:
                    me->SetUnitMovementFlags(MOVEMENTFLAG_WALKING);
                    me->SetSpeed(MOVE_RUN, 2.0f);
                    break;
            }
        }

        void UpdateAI(uint32 uiDiff) override
        {
            npc_escortAI::UpdateAI(uiDiff);

            if (!UpdateVictim())
                return;
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return GetTrialOfTheChampionAI<npc_black_knight_skeletal_gryphonAI>(creature);
    }
};

class achievement_ive_had_worse : public AchievementCriteriaScript
{
    public:
        achievement_ive_had_worse() : AchievementCriteriaScript("achievement_ive_had_worse") { }

        bool OnCheck(Player* /*player*/, Unit* target) override
        {
            if (!target)
                return false;

            if (Creature* Knight = target->ToCreature())
                if (Knight->AI()->GetData(DATA_IVE_HAD_WORSE) && Knight->GetMap()->ToInstanceMap()->IsHeroic())
                    return true;

            return false;
        }
};

void AddSC_boss_black_knight()
{
    new boss_black_knight();
    new npc_risen_ghoul();
    new npc_risen_announcer();
    new npc_black_knight_skeletal_gryphon();
    new achievement_ive_had_worse();
}
