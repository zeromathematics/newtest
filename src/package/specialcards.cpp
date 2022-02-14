#include "revolution.h"
#include "newtest.h"
#include "specialcards.h"
#include "standard-basics.h"
#include "standard-tricks.h"
#include "strategic-advantage.h"
#include "client.h"
#include "engine.h"
#include "structs.h"
#include "gamerule.h"
#include "settings.h"
#include "roomthread.h"
#include "json.h"
#include "qmath.h"

//real
class Mengxian : public TriggerSkill
{
public:
    Mengxian() : TriggerSkill("mengxian")
    {
        frequency = NotFrequent;
        events << EventPhaseStart;
    }
    virtual QStringList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &) const
    {
        if (TriggerSkill::triggerable(player) && player->getPhase() == Player::Draw)
            return QStringList(objectName());
        return QStringList();
    }

    virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        if (player->askForSkillInvoke(this, data)) {
            return true;
        }
        return false;
    }

     virtual bool effect(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        QString choice = room->askForChoice(player, objectName(), "basic+trick+equip");
        while (true){
            //QString equip = (player->isKongcheng() ? "" : "+equip");
            //QString choice = room->askForChoice(player, objectName(), "basic+trick+equip");
            JudgeStruct judge;
            judge.who = player;
            judge.negative = false;
            judge.play_animation = false;
            judge.time_consuming = true;
            judge.reason = objectName();
            judge.pattern = choice == "equip" ? "EquipCard" : (choice == "trick" ? "TrickCard" : "BasicCard");
            room->judge(judge);
            if ((judge.card->isKindOf("BasicCard") && choice == "basic") || (judge.card->isKindOf("TrickCard") && choice == "trick") || (judge.card->isKindOf("EquipCard") && choice == "equip")){
                if (judge.card->isKindOf("BasicCard"))
                    room->broadcastSkillInvoke(objectName(), 1);
                else if (judge.card->isKindOf("TrickCard"))
                    room->broadcastSkillInvoke(objectName(), 2);
                else
                    room->broadcastSkillInvoke(objectName(), 3);
                room->doLightbox(objectName() + "$", 500);
                room->obtainCard(player, judge.card);
                break;
            }
        }


        return false;
    }
};

class Yuanwang : public TriggerSkill
{
public:
    Yuanwang() : TriggerSkill("yuanwang")
    {
        frequency = Club;
        club_name = "sos",
        events << EventPhaseStart << EventPhaseEnd;
    }

    virtual void record(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
    {

    }

    virtual TriggerList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
    {
        TriggerList skill_list;
        if (event == EventPhaseStart){
            if (TriggerSkill::triggerable(player) && player->getPhase()==Player::Play){
                QList<ServerPlayer *> targets = room->getOtherPlayers(player);
                QList<ServerPlayer *> targets_copy = targets;
                QList<ServerPlayer *> list;
                foreach(ServerPlayer *s, targets){
                    if (s->hasClub("sos")){
                        list.append(s);
                    }
                }
                foreach(ServerPlayer *s, targets_copy){
                    foreach(ServerPlayer *t, list){
                        if (s->isFriendWith(t)){
                            targets.removeOne(s);
                            break;
                        }
                    }
                    if (!s->hasShownOneGeneral() && targets.contains(s)){
                        targets.removeOne(s);
                    }
                }

                if (targets.count() > 0){
                    skill_list.insert(player, QStringList(objectName()));
                }
            }
        }
        else{
            if (player->hasClub("sos") && player->getPhase()==Player::Play){
                ThreatenEmperor *t = new ThreatenEmperor(Card::NoSuit,-1);
                if (t->isAvailable(player) && !player->isKongcheng()){
                    skill_list.insert(player, QStringList(objectName()));
                }
            }
        }
        return skill_list;
    }

    virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who) const
    {
        if(event == EventPhaseStart && ask_who->askForSkillInvoke(this, data)){
            QList<ServerPlayer *> targets = room->getOtherPlayers(player);
            QList<ServerPlayer *> targets_copy = targets;
            QList<ServerPlayer *> list;
            foreach(ServerPlayer *s, targets){
                if (s->hasClub("sos")){
                    list.append(s);
                }
            }
            foreach(ServerPlayer *s, targets_copy){
                foreach(ServerPlayer *t, list){
                    if (s->isFriendWith(t)){
                        targets.removeOne(s);
                        break;
                    }
                }
                if (!s->hasShownOneGeneral() && targets.contains(s)){
                    targets.removeOne(s);
                }
            }
            ServerPlayer *target = room->askForPlayerChosen(ask_who, targets, objectName(), "@yuanwang", true);
            if (target){
               ask_who->tag["yuanwang_target"] = QVariant::fromValue(target);
               return true;
            }
        }
        else if (event == EventPhaseEnd && ask_who->askForSkillInvoke("yuanwangsos", data)){
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *haruhi) const
    {
        if (event == EventPhaseStart){
            ServerPlayer *target = haruhi->tag["yuanwang_target"].value<ServerPlayer *>();
            if (target){
                room->broadcastSkillInvoke(objectName(), haruhi);

                if (room->askForChoice(target, objectName(), objectName() + "_accept+cancel", QVariant::fromValue(haruhi)) == objectName() + "_accept"){
                    target->addClub("sos");
                    if (!target->faceUp()){
                        target->turnOver();
                    }
                }
                else{
                    LogMessage log;
                    log.type = "$refuse_club";
                    log.from = target;
                    log.arg = "sos";
                    room->sendLog(log);
                }
            }
        }
        else{
            QList<int> dislist;
            foreach(auto id, haruhi->handCards()){
                if (!Sanguosha->getCard(id)->isBlack()){
                    dislist << id;
                }
            }
            room->fillAG(haruhi->handCards(), haruhi, dislist);
            int id = room->askForAG(haruhi, haruhi->handCards(), true, objectName());
            room->clearAG(haruhi);
            if (id > -1){
                Card *card = Sanguosha->getCard(id);
                ThreatenEmperor *t = new ThreatenEmperor(card->getSuit(),card->getNumber());
                t->addSubcard(card);
                CardUseStruct use;
                use.from = haruhi;
                use.to << haruhi;
                use.card = t;
                room->useCard(use);
            }

        }
        return false;
    }
};

PengtiaoCard::PengtiaoCard()
{
    target_fixed = true;
    will_throw = false;
}

void PengtiaoCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const
{
     return;
}

class Pengtiaovs : public OneCardViewAsSkill
{
public:
    Pengtiaovs() : OneCardViewAsSkill("pengtiao"){
       response_or_use = true;
    }

    bool viewFilter(const Card *card) const
    {
        return card->isKindOf("Peach") || card->isKindOf("Analeptic") || card->getNumber()==13 || card->getSubtype() == "food_card";
    }

    const Card *viewAs(const Card *originalCard) const
    {
        PengtiaoCard *vs = new PengtiaoCard();
        vs->addSubcard(originalCard->getId());
        vs->setSkillName(objectName());
        return vs;
    }

    bool isEnabledAtPlay(const Player *player) const
    {
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const
    {
        return pattern == "@@pengtiao";
    }
};

class Pengtiao : public TriggerSkill
{
public:
    Pengtiao() : TriggerSkill("pengtiao")
    {
        frequency = NotFrequent;
        events << EventPhaseStart << CardUsed << EventPhaseEnd;
        view_as_skill = new Pengtiaovs;
    }

    virtual bool canPreshow() const
    {
        return true;
    }


    virtual void record(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
    {
         if (event == EventPhaseEnd && player->getPhase() == Player::Play){
             foreach(auto p, room->getAlivePlayers()){
                if (p->hasFlag(player->objectName()+"pengtiao_target")){
                    room->setPlayerFlag(p, "-"+player->objectName()+"pengtiao_target");
                }
             }
         }
         if (event == CardUsed){
             CardUseStruct use = data.value<CardUseStruct>();
             if (use.card && !use.card->isKindOf("TrickCard") && use.card->getTypeId() != Card::TypeSkill && player->getPhase()==Player::Play){
                 foreach(auto p, room->getOtherPlayers(player)){
                     if (p->hasFlag(player->objectName()+"pengtiao_target") && p->isWounded()){
                         if (player->askForSkillInvoke("pengtiao_recover", QVariant::fromValue(p))){
                             RecoverStruct recover;
                             recover.recover = 1;
                             recover.who = player;
                             room->recover(p, recover, true);
                         }
                     }
                 }
             }
         }
    }

    virtual QStringList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &) const
    {
        if (event == EventPhaseStart && player->getPhase() == Player::Play && TriggerSkill::triggerable(player)){
            return QStringList(objectName());
        }
        return QStringList();
    }
    virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        if (player->askForSkillInvoke(this, data)){
            room->broadcastSkillInvoke(objectName(), player);
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        foreach(auto p, room->getOtherPlayers(player)){
            const Card *card = room->askForUseCard(p, "@@pengtiao", "@pengtiao");
            if (card) {
                room->obtainCard(player, card);
                room->setPlayerFlag(p, player->objectName()+"pengtiao_target");
                break;
            }
            else{
                room->setPlayerFlag(p, "pengtiao_cancel");
            }
            foreach(auto p, room->getAlivePlayers()){
               room->setPlayerFlag(p, "-pengtiao_cancel");
            }
        }

        return false;
    }
};

class Shiji : public TriggerSkill
{
public:
    Shiji() : TriggerSkill("shiji")
    {
        frequency = NotFrequent;
        events << CardUsed << Pindian;
    }

    virtual void record(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (event == Pindian){
            PindianStruct *pindian = data.value<PindianStruct *>();
            if (pindian->reason == objectName()){
                ServerPlayer *winner = pindian->isSuccess() ? pindian->from : pindian->to;
                ServerPlayer *loser = pindian->isSuccess() ? pindian->to : pindian->from;
                if (winner->getHp() < loser->getHp() && winner->isWounded() && winner->askForSkillInvoke("shiji_recover", data)){
                    RecoverStruct recover;
                    recover.recover = 1;
                    room->recover(winner, recover, true);
                }
            }
        }
    }

     virtual TriggerList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
    {
        TriggerList skill_list;
        if (event == CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            QList<ServerPlayer *> yukihiras = room->findPlayersBySkillName(objectName());
            foreach(ServerPlayer *yukihira, yukihiras)
                if ((use.card->isKindOf("Peach") || use.card->isKindOf("Analeptic") || use.card->getSubtype() == "food_card") && player->getPhase()==Player::Play && !yukihira->isKongcheng() && yukihira != player && !player->isKongcheng())
                    skill_list.insert(yukihira, QStringList(objectName()));
        }
        return skill_list;
    }

    virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *yukihira) const
    {
        if (yukihira->askForSkillInvoke(this, data)){
            room->broadcastSkillInvoke(objectName(), yukihira);
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *yukihira) const
    {
        if (yukihira->isKongcheng()||player->isKongcheng())
            return false;
        PindianStruct *pd = yukihira->pindianSelect(player, objectName());
        if (yukihira->pindian(pd)) {
           return true;
        }
        return false;
    }
};

//science
class Takamakuri : public TriggerSkill
{
public:
    Takamakuri() : TriggerSkill("Takamakuri")
    {
        frequency = NotFrequent;
        events << Damage;
    }
    virtual QStringList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &) const
    {
        DamageStruct damage = data.value<DamageStruct>();
        if (TriggerSkill::triggerable(player))
            return QStringList(objectName());
        return QStringList();
    }

    virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        if (player->askForSkillInvoke(this, data)) {
            return true;
        }
        return false;
    }

     virtual bool effect(TriggerEvent event, Room *room, ServerPlayer *akari, QVariant &data, ServerPlayer *) const
    {
        if (event == Damage){
            DamageStruct damage = data.value<DamageStruct>();
            akari->setFlags("TakamakuriUsed");
            int id = room->getDrawPile().at(0);
            QList<int> ids;
            ids.append(id);
            room->fillAG(ids);
            room->getThread()->delay(800);

            room->clearAG();
            if (Sanguosha->getCard(id)->isKindOf("BasicCard")){
                room->broadcastSkillInvoke(objectName(), akari);
                room->obtainCard(akari, id);
                if (damage.to->getEquips().length() > 0)
                    room->throwCard(room->askForCardChosen(akari, damage.to, "e", objectName()), damage.to, akari);
            }
        }
        return false;
    }
};

class Tobiugachi : public TriggerSkill
{
public:
    Tobiugachi() : TriggerSkill("Tobiugachi")
    {
        frequency = NotFrequent;
        events << CardAsked;
    }
    virtual QStringList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &) const
    {
        QString pattern = Sanguosha->currentRoomState()->getCurrentCardUsePattern();
        if (pattern == "jink" && TriggerSkill::triggerable(player) && player->getHandcardNum() > player->getHp())
            return QStringList(objectName());
        return QStringList();
    }

    virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        if (player->askForSkillInvoke(this, data)) {
            return true;
        }
        return false;
    }

     virtual bool effect(TriggerEvent event, Room *room, ServerPlayer *akari, QVariant &data, ServerPlayer *) const
    {
        if (event == CardAsked){
            if (room->askForDiscard(akari, objectName(), akari->getHandcardNum() - akari->getHp() + 1, akari->getHandcardNum() - akari->getHp() + 1)){
                akari->setFlags("TobiugachiUsed");
                Card* jink = Sanguosha->cloneCard("jink", Card::NoSuit, 0);
                jink->setSkillName(objectName());
                room->provide(jink);
                ServerPlayer *target = room->askForPlayerChosen(akari, room->getAlivePlayers(), objectName());
                QStringList list = target->getPileNames();
                bool hasPile = false;
                foreach (QString pile, list){
                    if (target->getPile(pile).length() > 0){
                        hasPile = true;
                    }
                }
                QString choice = "ToBiGetRegion";
                if (hasPile)
                    choice = room->askForChoice(akari, objectName(), "ToBiGetRegion+TobiGetPile");
                if (choice == "TobiGetPile"){
                    QString choice2 = room->askForChoice(akari, objectName() + "1", list.join("+"));
                    QList<int> pile = target->getPile(choice2);
                    room->fillAG(pile, akari);
                    int id = room->askForAG(akari, pile, false, objectName());
                    if (id == -1)
                        return false;
                    room->obtainCard(akari, id);
                    room->clearAG(akari);
                }
                else{
                    int id =  room->askForCardChosen(akari, target, "hej", objectName());
                    if (id == -1)
                        return false;
                    room->obtainCard(akari, id);
                }
            }
        }
        return false;
    }
};

class Fukurouza : public TriggerSkill
{
public:
    Fukurouza() : TriggerSkill("Fukurouza")
    {
        events << EventPhaseEnd;
    }

    virtual TriggerList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        TriggerList skill_list;
        if (player == NULL) return skill_list;
        if (triggerEvent == EventPhaseEnd) {

            QList<ServerPlayer *> akaris = room->findPlayersBySkillName(objectName());
            foreach(ServerPlayer *akari, akaris)
                if (player->getPhase() == Player::Finish && (akari->hasFlag("TobiugachiUsed")||akari->hasFlag("TakamakuriUsed")))
                    skill_list.insert(akari, QStringList(objectName()));
            return skill_list;
        }
        return skill_list;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who) const
    {
        ServerPlayer *akari = ask_who;

        if (akari->askForSkillInvoke(this, data)) {
           return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who) const
    {
        ServerPlayer *akari = ask_who;
        /*bool broad = true;
        if (akari && akari->isAlive() && akari->hasFlag("TobiugachiUsed") && room->askForSkillInvoke(akari, objectName() + "Tobi", data)){
            room->broadcastSkillInvoke(objectName());
            broad = false;
            DamageStruct damage;
            damage.from = akari;
            damage.to = player;
            damage.reason = objectName();
            room->damage(damage);
        }

        if (akari && akari->isAlive() && akari->hasFlag("TakamakuriUsed") && room->askForSkillInvoke(akari, objectName() + "Taka", data)){
            if (broad)
                room->broadcastSkillInvoke(objectName());
            akari->drawCards(1);
            akari->setFlags("-TakamakuriUsed");
        }*/
        room->broadcastSkillInvoke(objectName(), akari);
        akari->drawCards(1);
        if (akari && akari->isAlive() && akari->hasFlag("TobiugachiUsed"))
            akari->setFlags("-TobiugachiUsed");
        if (akari && akari->isAlive() && akari->hasFlag("TakamakuriUsed"))
            akari->setFlags("-TakamakuriUsed");
        return false;
    }
};

class Kioku : public TriggerSkill
{
public:
    Kioku() : TriggerSkill("kioku")
    {
        frequency = Compulsory;
        events << EventPhaseStart << Death;
    }

    virtual void record(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (event == Death){
            DeathStruct death = data.value<DeathStruct>();
            if (player == death.who && player->getPile("memory").length()>0){
                ServerPlayer *dest = room->askForPlayerChosen(player, room->getAlivePlayers(), objectName());
                room->broadcastSkillInvoke(objectName(), 8, player);
                room->doLightbox("kioku$", 2500);
                DummyCard *dummy = new DummyCard;
                dummy->deleteLater();
                foreach(int id, player->getPile("memory")){
                    dummy->addSubcard(id);
                }
                dest->obtainCard(dummy, true);
                room->setPlayerMark(dest, "kioku_dest", 1);
                if (player->isAlive()){
                    player->removeGeneral(player->inHeadSkills(this));
                }
            }
        }
    }

    virtual QStringList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &) const
    {
        if (event == EventPhaseStart && player->getPhase()==Player::Play && TriggerSkill::triggerable(player)){
            return QStringList(objectName());
        }
        return QStringList();
    }
    virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        if (player->hasShownSkill(objectName()) || player->askForSkillInvoke(this, data)){
            room->broadcastSkillInvoke(objectName(), rand()%7+1, player);
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        player->drawCards(1);
        if (!player->isKongcheng()){
            int id = room->askForCardChosen(player, player, "h", objectName());
            player->addToPile("memory", id);
        }
        if (player->getPile("memory").length() >= 9){
            ServerPlayer *dest = room->askForPlayerChosen(player, room->getAlivePlayers(), objectName());
            room->broadcastSkillInvoke(objectName(), 8, player);
            room->doLightbox("kioku$");
            DummyCard *dummy = new DummyCard;
            dummy->deleteLater();
            foreach(int id, player->getPile("memory")){
                dummy->addSubcard(id);
            }
            dest->obtainCard(dummy, true);
            room->setPlayerMark(dest, "kioku_dest", 1);
            if (player->isAlive()){
                player->removeGeneral(player->inHeadSkills(this));
            }
        }
        return false;
    }
};

class KiokuMax : public MaxCardsSkill
{
public:
    KiokuMax() : MaxCardsSkill("kiokumax")
    {
    }

    virtual int getExtra(const ServerPlayer *target, MaxCardsType::MaxCardsCount) const
    {
        if (target->getMark("kioku_dest")>0){
            return 2;
        }
        if (target->hasFlag("xiangsui_dest")){
            return target->getMaxHp()-target->getHp();
        }
        return 0;
    }
};

XiangsuiCard::XiangsuiCard()
{
    will_throw = false;
}

bool XiangsuiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    return targets.isEmpty();
}

void XiangsuiCard::use(Room *room, ServerPlayer *player, QList<ServerPlayer *> &targets) const
{
    room->obtainCard(targets.at(0), this);
    room->setPlayerFlag(targets.at(0), "xiangsui_dest");
    player->drawCards(1);
    if (!player->isKongcheng()){
        int id = room->askForCardChosen(player, player, "h", objectName());
        player->addToPile("memory", id);
    }
}

class Xiangsui : public OneCardViewAsSkill
{
public:
    Xiangsui() : OneCardViewAsSkill("xiangsui")
    {
        filter_pattern = ".|.|.|memory";
        expand_pile = "memory";
    }

    virtual bool isEnabledAtPlay(const Player *player) const
    {
        return !player->hasUsed("XiangsuiCard");
    }

    virtual const Card *viewAs(const Card *originalCard) const
    {
        XiangsuiCard *vs = new XiangsuiCard();
        vs->addSubcard(originalCard);
        vs->setSkillName(objectName());
        vs->setShowSkill(objectName());
        return vs;
    }
};

//magic
class Zhouxue : public TriggerSkill
{
public:
    Zhouxue() : TriggerSkill("zhouxue")
    {
        frequency = NotFrequent;
        events << Damage << Damaged;
    }
    virtual QStringList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &) const
    {
        if (TriggerSkill::triggerable(player) && (!room->getCurrent() || !room->getCurrent()->hasFlag(player->objectName() +  "zhouxue_invalid")))
            return QStringList(objectName());
        return QStringList();
    }

    virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        if (player->askForSkillInvoke(this, data)) {
            return true;
        }
        return false;
    }

     virtual bool effect(TriggerEvent event, Room *room, ServerPlayer *kuriyama, QVariant &data, ServerPlayer *) const
    {
        JudgeStruct judge;
        judge.pattern = ".|red";
        judge.good = true;
        judge.reason = "zhouxue";
        judge.who = kuriyama;

        room->judge(judge);
        if (judge.card->isRed()){
            QString choice = room->askForChoice(kuriyama, objectName(), "zhouxue_yes+cancel", data);
            if (choice == "zhouxue_yes")
                kuriyama->addToPile("zhouxue_blood", judge.card, true);
        }
        else{
            kuriyama->drawCards(1);
            if (room->getCurrent()){
               room->setPlayerFlag(room->getCurrent(), kuriyama->objectName() + "zhouxue_invalid");
            }
        }
        return false;
    }
};

QString XuerenPattern = "pattern";
class Xueren : public OneCardViewAsSkill
{
public:
    Xueren() : OneCardViewAsSkill("xueren")
    {
        filter_pattern = ".|.|.|zhouxue_blood";
        expand_pile = "zhouxue_blood";
    }

    virtual bool isEnabledAtPlay(const Player *player) const
    {
        XuerenPattern = "slash";
        Slash *slash = new Slash(Card::NoSuit,-1);
        return slash->isAvailable(player)&&!player->getPile("zhouxue_blood").isEmpty() && !player->hasFlag("xueren_used");
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const
    {
        if (pattern=="slash" || pattern=="jink"){
            XuerenPattern = pattern;
            return player->hasSkill("xueren") && !player->hasFlag("xueren_used");
        }
        return false;
    }

    virtual const Card *viewAs(const Card *originalCard) const
    {
        QString pattern = XuerenPattern;
        Card *card=Sanguosha->cloneCard(pattern,originalCard->getSuit(),originalCard->getNumber());
        card->addSubcard(originalCard);
        card->setSkillName(objectName());
        card->setShowSkill(objectName());
        return card;
    }
};

class XuerenTrigger : public TriggerSkill
{
public:
    XuerenTrigger() : TriggerSkill("#xueren")
    {
        frequency = NotFrequent;
        events << CardResponded << CardUsed << EventPhaseEnd;
        global=true;
    }

    virtual void record(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (triggerEvent == CardResponded){
             const Card *card = data.value<CardResponseStruct>().m_card;
             if (card->getSkillName()=="xueren"){
                 if (room->getCurrent())
                     room->setPlayerFlag(player, "xueren_used");
             }
        }
        else if (triggerEvent == CardUsed){
             const Card *card = data.value<CardUseStruct>().card;
             if (card->getSkillName()=="xueren"){
                 if (room->getCurrent())
                     room->setPlayerFlag(player, "xueren_used");
                 if (card->isKindOf("Slash"))
                     room->addPlayerHistory(player, card->getClassName(), -1);
             }
        }
        else{
            if (player->getPhase()==Player::Finish){
                foreach(auto p, room->getAlivePlayers()){
                    room->setPlayerFlag(p, "-xueren_used");
                }
            }
        }
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &) const
    {

        return QStringList();
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        return false;
    }
};

class XuerenTargetMod : public TargetModSkill
{
public:
    XuerenTargetMod() : TargetModSkill("#xueren-target")
    {
        pattern = "Slash";
    }

    virtual int getDistanceLimit(const Player *from, const Card *card) const
    {

        if (card->getSkillName()=="xueren")
            return 1000;
        else
            return 0;
    }
};

class Caoxue : public TriggerSkill
{
public:
    Caoxue() : TriggerSkill("caoxue")
    {
        frequency = NotFrequent;
        events << DamageCaused << EventPhaseStart;
        relate_to_place = "deputy";
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &) const
    {
            if (triggerEvent == DamageCaused){
                 DamageStruct damage = data.value<DamageStruct>();
                 if (player->hasFlag("caoxue_yes") && (damage.to->getJudgingArea().length()>0||!damage.to->isNude()))
                     return QStringList(objectName());
            }
            else{
                if (player->getPhase()==Player::Start){
                    if (TriggerSkill::triggerable(player) && player->getPile("zhouxue_blood").length()>0)
                        room->setPlayerFlag(player, "caoxue_yes");
                }
            }
        return QStringList();
    }

    virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        if (player->askForSkillInvoke(this, data)) {
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        DamageStruct damage = data.value<DamageStruct>();
        int id = room->askForCardChosen(player, damage.to, "hej", objectName());
        room->obtainCard(player, id, false);
        return false;
    }
};

class Huanzhuang : public TriggerSkill
{
public:
    Huanzhuang() : TriggerSkill("huanzhuang")
    {
        frequency = NotFrequent;
        events << CardUsed << CardFinished;
    }

    virtual void record(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (triggerEvent == CardUsed){
             CardUseStruct use = data.value<CardUseStruct>();
             if (use.card->isKindOf("Slash") && player->hasFlag("huanzhuang_get")){
                 room->addPlayerHistory(player, use.card->getClassName(), -1);
                 room->setPlayerFlag(player, "-huanzhuang_get");
             }
        }
        return;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &) const
    {
            if (triggerEvent == CardUsed){
                 CardUseStruct use = data.value<CardUseStruct>();
                 if (TriggerSkill::triggerable(player) && ((use.card->isKindOf("Weapon")&& player->getWeapon()!= NULL) || (use.card->isKindOf("Armor")&& player->getArmor()!= NULL)) && !player->hasFlag("huanzhuang1used"))
                     return QStringList(objectName());
            }
            else{
                CardUseStruct use = data.value<CardUseStruct>();
                if (TriggerSkill::triggerable(player) && (use.card->isKindOf("Slash") || use.card->isKindOf("Weapon") || use.card->isKindOf("Armor")) && !player->hasFlag("huanzhuang2used")){
                    return QStringList(objectName());
                }
            }
        return QStringList();
    }

    virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        if (player->askForSkillInvoke(this, data)) {
            if (event == CardUsed)
                room->setPlayerFlag(player, "huanzhuang1used");
            if (event == CardFinished)
                room->setPlayerFlag(player, "huanzhuang2used");
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        if (triggerEvent == CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.card->isKindOf("Weapon")&& player->getWeapon()!= NULL){
                room->obtainCard(player, player->getWeapon());
            }
            if (use.card->isKindOf("Armor")&& player->getArmor() != NULL){
                room->obtainCard(player, player->getArmor());
            }
        }
        else{
          CardUseStruct use = data.value<CardUseStruct>();
          QList<int> list;
          if (use.card->isKindOf("Slash")){
              foreach(auto id, room->getDrawPile()){
                  if (Sanguosha->getCard(id)->isKindOf("Weapon")|| Sanguosha->getCard(id)->isKindOf("Armor")){
                      list << id;
                  }
              }
              foreach(auto id, room->getDiscardPile()){
                  if (Sanguosha->getCard(id)->isKindOf("Weapon")|| Sanguosha->getCard(id)->isKindOf("Armor")){
                      list << id;
                  }
              }
          }
          else{
              foreach(auto id, room->getDrawPile()){
                  if (Sanguosha->getCard(id)->isKindOf("Slash")){
                      list << id;
                  }
              }
              foreach(auto id, room->getDiscardPile()){
                  if (Sanguosha->getCard(id)->isKindOf("Slash")){
                      list << id;
                  }
              }
          }
          if (!list.isEmpty()){
              int id = list.at(rand()%list.length());
              room->obtainCard(player, id);
              const Card *card = Sanguosha->getCard(id);
              if (card->isKindOf("Slash") && player->getPhase()!=Player::NotActive){
                  room->setPlayerFlag(player, "huanzhuang_get");
              }
          }
        }
        return false;
    }
};

class HuanzhuangTargetMod : public TargetModSkill
{
public:
    HuanzhuangTargetMod() : TargetModSkill("#huanzhuang-target")
    {
        pattern = "Slash";
    }

    virtual int getDistanceLimit(const Player *from, const Card *card) const
    {
        if (from->hasFlag("huanzhuang_get"))
            return 1000;
        else
            return 0;
    }
};

//game
PenglaiCard::PenglaiCard()
{
}

bool PenglaiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    return targets.isEmpty() && !to_select->isKongcheng() && to_select!=Self;
}

void PenglaiCard::use(Room *room, ServerPlayer *player, QList<ServerPlayer *> &targets) const
{
    ServerPlayer *target = targets.at(0);
    if (!target->isKongcheng()){
        room->askForDiscard(target, "penglai", 1, 1);
        if (target->isWounded()){
            RecoverStruct recover;
            recover.recover = 1;
            recover.who = player;
            room->recover(target, recover, true);
        }
    }
}

class Penglai : public ZeroCardViewAsSkill
{
public:
    Penglai() : ZeroCardViewAsSkill("penglai"){

    }

    bool isEnabledAtPlay(const Player *player) const
    {
        return !player->hasUsed("PenglaiCard");
    }

    const Card *viewAs() const
    {
        PenglaiCard *vs = new PenglaiCard();
        vs->setSkillName(objectName());
        vs->setShowSkill(objectName());
        return vs;
    }
};

class Jiansivs : public ZeroCardViewAsSkill
{
public:
    Jiansivs() : ZeroCardViewAsSkill("jiansi"){

    }

    bool isEnabledAtPlay(const Player *player) const
    {
        return false;
    }

    bool isEnabledAtResponse(const Player *player, const QString &pattern) const
    {
        return pattern == "@@jiansi";
    }

    const Card *viewAs() const
    {
        QString pattern = Self->property("jiansi_card").toString();
        int id = Self->property("jiansi_number").toInt();
        if (pattern == "")
            return NULL;
        Card *vs = Sanguosha->cloneCard(pattern);
        vs->setSkillName(objectName());
        vs->setShowSkill(objectName());
        vs->addSubcard(id);
        return vs;
    }
};

class Jiansi : public TriggerSkill
{
public:
    Jiansi() : TriggerSkill("jiansi")
    {
        view_as_skill = new Jiansivs;
        events << CardUsed << CardResponded << EventPhaseEnd << EventPhaseChanging;
    }

    virtual bool canPreshow() const
    {
        return true;
    }

    virtual void record(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
    {
         if (event == CardUsed){
             CardUseStruct use = data.value<CardUseStruct>();
             if (use.card->getTypeId() != Card::TypeSkill && room->getCurrent()){
                 room->setPlayerMark(use.from, "jiansi_times", 1);
             }
         }
         if (event == CardResponded){
             CardResponseStruct r = data.value<CardResponseStruct>();
             if (r.m_card && r.m_card->getTypeId() != Card::TypeSkill && room->getCurrent()){
                 room->setPlayerMark(player, "jiansi_times", 1);
             }
         }
         if (event == EventPhaseChanging){
             PhaseChangeStruct change = data.value<PhaseChangeStruct>();
             if (change.to == Player::NotActive){
                 foreach(auto p , room->getAlivePlayers()){
                     room->setPlayerMark(p, "jiansi_times", 0);
                 }
             }
         }
    }

    virtual TriggerList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
    {
        TriggerList skill_list;
        if (event == EventPhaseEnd){
            if (player->getPhase()==Player::Finish){
                QList<ServerPlayer *> eirins = room->findPlayersBySkillName(objectName());
                foreach(ServerPlayer *eirin, eirins)
                    if (eirin->getMark("jiansi_times")>0)
                        skill_list.insert(eirin, QStringList(objectName()));
            }
        }

        return skill_list;
    }

    virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who) const
    {
        if(ask_who->askForSkillInvoke(this, data)){
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *eirin) const
    {
        QList<int> list;
        QList<int> disabled_ids;
        int x=2;
        int l=room->getDrawPile().length();
        for (int i=1; i<= x; i++) {
          if (l-i>=0) {
            list << room->getDrawPile().at(l-i);
            const Card *card = Sanguosha->getCard(room->getDrawPile().at(l-i));
            if (!card->isAvailable(eirin)) {
                disabled_ids << room->getDrawPile().at(l-i);
            }
          }
        }
        if (!list.isEmpty()){
             room->fillAG(list, eirin ,disabled_ids);
             int id = room->askForAG(eirin,list,true,"jiansi");
             room->clearAG(eirin);
             if (id > -1){
                 const Card *card = Sanguosha->getCard(id);
                 if (card->isKindOf("EquipCard")) {
                     CardUseStruct use;
                     use.from = eirin;
                     use.to << eirin;
                     use.card =card;
                     room->useCard(use);
                     CardsMoveStruct move = CardsMoveStruct();
                     list.removeOne(id);
                     move.card_ids = list;
                     move.to = eirin;
                     move.to_place = Player::PlaceTable;
                     move.reason = CardMoveReason(CardMoveReason::S_REASON_TURNOVER, eirin->objectName(), objectName(), NULL);
                     room->moveCardsAtomic(move, true);
                     CardMoveReason reason = CardMoveReason();
                     reason.m_reason = CardMoveReason::S_REASON_THROW;
                     reason.m_playerId = eirin->objectName();
                     room->moveCardTo(Sanguosha->getCard(list.at(0)), NULL, Player::DiscardPile, reason, true);
                 }
                 else{
                     room->setPlayerProperty(eirin, "jiansi_card", QVariant(card->objectName()));
                     room->setPlayerProperty(eirin, "jiansi_number", QVariant(id));
                     if (room->askForUseCard(eirin, "@@jiansi", "@jiansi")){
                         CardsMoveStruct move = CardsMoveStruct();
                         list.removeOne(id);
                         move.card_ids = list;
                         move.to = eirin;
                         move.to_place = Player::PlaceTable;
                         move.reason = CardMoveReason(CardMoveReason::S_REASON_TURNOVER, eirin->objectName(), objectName(), NULL);
                         room->moveCardsAtomic(move, true);
                         CardMoveReason reason = CardMoveReason();
                         reason.m_reason = CardMoveReason::S_REASON_THROW;
                         reason.m_playerId = eirin->objectName();
                         room->moveCardTo(Sanguosha->getCard(list.at(0)), NULL, Player::DiscardPile, reason, true);
                     }
                 }
             }
        }
        return false;
    }
};


//Fading Cards
MapoTofu::MapoTofu(Card::Suit suit, int number)
    : BasicCard(suit, number)
{
    setObjectName("mapo_tofu");
}

QString MapoTofu::getSubtype() const
{
    return "food_card";
}

bool MapoTofu::IsAvailable(const Player *player, const Card *tofu)
{
    MapoTofu *newanaleptic = new MapoTofu(Card::NoSuit, 0);
    newanaleptic->deleteLater();
#define THIS_TOFU (tofu == NULL ? newanaleptic : tofu)
    if (player->isCardLimited(THIS_TOFU, Card::MethodUse) || player->isProhibited(player, THIS_TOFU))
        return false;

    return player->usedTimes("MapoTofu") <= Sanguosha->correctCardTarget(TargetModSkill::Residue, player, THIS_TOFU);
#undef THIS_ANALEPTIC
}

bool MapoTofu::isAvailable(const Player *player) const
{

    return IsAvailable(player, this) && BasicCard::isAvailable(player);
}

bool MapoTofu::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    return targets.length() == 0 && Self->distanceTo(to_select) <= 1 && to_select->getMark("mtUsed") == 0;
}

void MapoTofu::onUse(Room *room, const CardUseStruct &card_use) const
{
    CardUseStruct use = card_use;
    if (use.to.isEmpty())
        use.to << use.from;
    BasicCard::onUse(room, use);
}

void MapoTofu::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const
{
    if (targets.isEmpty())
        targets << source;
    BasicCard::use(room, source, targets);
}

void MapoTofu::onEffect(const CardEffectStruct &effect) const
{
    Room *room = effect.to->getRoom();
    //room->setEmotion(effect.to, "mapo_tofu");//TODO

    DamageStruct damage;
    damage.to = effect.to;
    damage.damage = effect.to->getHp() > 0 ? effect.to->getHp() - 1: 0;
    int toDamge = damage.damage;
    // damage.chain = false;
    damage.chain = true;
    damage.nature = DamageStruct::Fire;
    effect.to->getRoom()->damage(damage);
    LogMessage log;
    log.type = "#MapoTofuUse";
    log.from = effect.from;
    log.to << effect.to;
    log.arg = objectName();
    room->sendLog(log);
    effect.to->setMark("mtUsed", toDamge + 1);
}

Tacos::Tacos(Card::Suit suit, int number)
    : BasicCard(suit, number)
{
    setObjectName("tacos");
    target_fixed = true;
}

QString Tacos::getSubtype() const
{
    return "food_card";
}

bool Tacos::isAvailable(const Player *player) const
{

    return BasicCard::isAvailable(player);
}

void Tacos::onUse(Room *room, const CardUseStruct &card_use) const
{
    CardUseStruct use = card_use;
    if (use.to.isEmpty())
        use.to << use.from;
    BasicCard::onUse(room, use);
}

void Tacos::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const
{
    if (targets.isEmpty())
        targets << source;
    BasicCard::use(room, source, targets);
}

void Tacos::onEffect(const CardEffectStruct &effect) const
{
    Room *room = effect.to->getRoom();
    int n=room->getDiscardPile().length();
    if (n==0)
        return;
    int j = rand()%n;
    effect.to->obtainCard(Sanguosha->getCard(room->getDiscardPile().at(j)));

    n = qFloor((room->getDiscardPile().length())*3/4);
    if (n==0)
        return;
    CardsMoveStruct move;
    for(int i = 0; i<n; i++){
        move.card_ids << room->getDiscardPile().at(i);
    }
    move.to_place = Player::DrawPile;
    move.reason.m_reason=CardMoveReason::S_REASON_PUT;
    room->moveCardsAtomic(move,true);
}

SpecialCardPackage::SpecialCardPackage() : Package("specialcard", CardPack)
{
    QList<SkillCard *> cards;
    //cards << new ShifengCard;

    foreach(SkillCard *card, cards)
        card->setParent(this);


}

FadingPackage::FadingPackage()
    : Package("fading")
{
    skills << new XuerenTargetMod << new XuerenTrigger << new KiokuMax << new HuanzhuangTargetMod;

    General *haruhi = new General(this, "haruhi", "real", 3, false);
    haruhi->addSkill(new Mengxian);
    haruhi->addSkill(new Yuanwang);
    General *yukihira = new General(this, "Yukihira", "real", 4);
    yukihira->addSkill(new Pengtiao);
    yukihira->addSkill(new Shiji);

    General *akari = new General(this, "Akari", "science", 3, false);
    akari->addSkill(new Takamakuri);
    akari->addSkill(new Tobiugachi);
    akari->addSkill(new Fukurouza);
    akari->addCompanion("Aria");
    General *isla = new General(this, "Isla", "science", 3, false);
    isla->addSkill(new Kioku);
    isla->addSkill(new Xiangsui);

    General *kuriyama = new General(this, "Kuriyama", "magic", 4, false);
    kuriyama->addSkill(new Zhouxue);
    kuriyama->addSkill(new Xueren);
    kuriyama->addSkill(new Caoxue);
    kuriyama->setDeputyMaxHpAdjustedValue();
    General *eruza = new General(this, "Eruza", "magic", 4, false);
    eruza->addSkill(new Huanzhuang);

    General *eirin  = new General(this, "Eirin", "game", 3, false);
    eirin->addSkill(new Penglai);
    eirin->addSkill(new Jiansi);
    eirin->addCompanion("Reisen");

    addMetaObject<MapoTofu>();
    addMetaObject<Tacos>();
    addMetaObject<PengtiaoCard>();
    addMetaObject<XiangsuiCard>();
    addMetaObject<PenglaiCard>();
}

FadingCardPackage::FadingCardPackage() : Package("fadingcard", CardPack)
{
    QList<Card *> cards;
    cards << new MapoTofu(Card::Spade, 1);
    cards << new Tacos(Card::Heart, 13);
    cards << new Tacos(Card::Club, 3);

    foreach(Card *card, cards)
        card->setParent(this);


}

//ADD_PACKAGE(SpecialCard)
ADD_PACKAGE(Fading)
ADD_PACKAGE(FadingCard)
