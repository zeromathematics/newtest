#include "maria-battle.h"
#include "skill.h"
#include "engine.h"
#include "standard-tricks.h"
#include "standard-basics.h"
#include "roomthread.h"

class MariaBattleRule : public TriggerSkill
{
public:
    MariaBattleRule() : TriggerSkill("mariabattlerule")
    {
        frequency = NotFrequent;
        events << HpChanged;
        global = true;
    }

    virtual bool canPreshow() const
    {
        return true;
    }

     virtual QStringList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const
    {
        if (room->getMode()!="maria_battle"){
            return QStringList();
        }
        if (event == HpChanged){
            foreach(auto p, room->getAlivePlayers()){
                if (p->getGeneralName() == "Armored_Titan" && p->getHp()<5){
                    if (room->getTag("Colossal_Titan").toBool()){
                        return QStringList();
                    }
                    foreach(auto q, room->getPlayers()){
                        if (q->getGeneralName()!="Pure_Titan" || q ->isAlive()){
                            continue;
                        }
                        room->setTag("Colossal_Titan", QVariant(true));
                        room->doDragonPhoenix(q, "Colossal_Titan", QString(), false, q->getKingdom(), true, "h");
                        room->setPlayerProperty(q, "maxhp", 20);
                        room->setPlayerProperty(q, "hp", 20);
                        room->transformDeputyGeneralTo(q, "white10");
                        room->changeBGM("maria-battle2");
                        room->setTag("Colossal_Titan", QVariant(true));
                        break;
                    }
                    if (!room->getTag("Colossal_Titan").toBool()){
                        foreach(auto q, room->getAlivePlayers()){
                            if (q->getGeneralName()!="Pure_Titan"){
                                continue;
                            }
                            room->setTag("Colossal_Titan", QVariant(true));
                            room->transformHeadGeneralTo(q, "Colossal_Titan");
                            room->setPlayerProperty(q, "maxhp", QVariant(20));
                            room->setPlayerProperty(q, "hp", QVariant(20));
                            room->changeBGM("maria-battle2");
                            break;
                        }
                    }
                }
            }
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *skill_target, QVariant &data, ServerPlayer *player) const
    {
        return false;
    }
};


MariaBattlePackage::MariaBattlePackage()
    : Package("maria-battle")
{
    skills << new MariaBattleRule;
    General *erwin = new General(this, "Erwin", "WOF", 5, true, true, true);
    General *eren = new General(this, "Eren", "WOF", 4, true, true, true);
    General *levi = new General(this, "Levi", "WOF", 4, true, true, true);
    General *mikasa = new General(this, "Mikasa", "WOF", 4, false, true, true);
    General *armin= new General(this, "Armin", "WOF", 4, true, true, true);
    General *hange= new General(this, "Hange", "WOF", 4, false, true, true);
    General *soldier= new General(this, "WOF_Soldier", "WOF", 4, false, true, true);

    General *armored_titan= new General(this, "Armored_Titan", "Titan", 15, true, true, true);
    General *colossal_titan= new General(this, "Colossal_Titan", "Titan", 20, true, true, true);
    General *beast_titan= new General(this, "Beast_Titan", "Titan", 20, true, true, true);
    General *cart_titan= new General(this, "Cart_Titan", "Titan", 12, false, true, true);
    General *pure_titan= new General(this, "Pure_Titan", "Titan", 10, true, true, true);

    General *white10= new General(this, "white10", "Titan", 10, true, true, true);
    General *white15= new General(this, "white15", "Titan", 15, true, true, true);
    General *white5= new General(this, "white5", "WOF", 5, true, true, true);
    General *white4= new General(this, "white4", "WOF", 4, true, true, true);
}
ADD_PACKAGE(MariaBattle)
