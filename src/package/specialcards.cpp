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
        events << EventPhaseStart;
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
        return skill_list;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who) const
    {
        if(ask_who->askForSkillInvoke(this, data)){
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
    General *haruhi = new General(this, "haruhi", "real", 3, false);
    haruhi->addSkill(new Mengxian);
    haruhi->addSkill(new Yuanwang);

    General *akari = new General(this, "Akari", "science", 3, false);
    akari->addSkill(new Takamakuri);
    akari->addSkill(new Tobiugachi);
    akari->addSkill(new Fukurouza);
    akari->addCompanion("Aria");

    addMetaObject<MapoTofu>();
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

ADD_PACKAGE(SpecialCard)
ADD_PACKAGE(Fading)
ADD_PACKAGE(FadingCard)
