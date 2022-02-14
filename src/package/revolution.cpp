
#include "revolution.h"
#include "newtest.h"
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

class CommandSelect : public ViewAsSkill
{
public:
    CommandSelect() : ViewAsSkill("commandefect")
    {
        response_pattern = "@@commandefect!";
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const
    {
        return selected.isEmpty() || (selected.length() == 1 && to_select->isEquipped() != selected.first()->isEquipped());
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const
    {
        bool ok = false;
        if (cards.length() == 1) {
            if (cards.first()->isEquipped())
                ok = Self->isKongcheng();
            else
                ok = !Self->hasEquip();
        } else if (cards.length() == 2) {
            ok = true;
        }

        if (!ok)
            return NULL;

        DummyCard *dummy = new DummyCard;
        dummy->addSubcards(cards);
        return dummy;
    }
};

class CommandEffect : public TriggerSkill
{
public:
    CommandEffect() : TriggerSkill("commandefect")
    {
        events << EventPhaseStart;
        view_as_skill = new CommandSelect;
        global = true;
    }

    virtual void record(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &) const
    {
        if (triggerEvent == EventPhaseStart && player->getPhase() ==  Player::NotActive) {
            QList<ServerPlayer *> alls = room->getAlivePlayers();
            foreach (ServerPlayer *p, alls) {
                room->setPlayerMark(p, "JieyueExtraDraw", 0);
                if (p->getMark("command4_effect") > 0) {
                    room->setPlayerMark(p, "command4_effect", 0);

                    foreach(ServerPlayer *p, room->getAllPlayers())
                        room->filterCards(p, p->getCards("he"), false);

                    JsonArray args;
                    args << QSanProtocol::S_GAME_EVENT_UPDATE_SKILL;
                    room->doBroadcastNotify(QSanProtocol::S_COMMAND_LOG_EVENT, args);

                    room->removePlayerCardLimitation(p, "use,response", ".|.|.|hand$1");
                }
                if (p->getMark("command5_effect") > 0) {
                    room->setPlayerMark(p, "command5_effect", 0);
                    p->tag["CannotRecover"] = false;
                }
            }
        }
    }

    virtual QStringList triggerable(TriggerEvent , Room *, ServerPlayer *, QVariant &, ServerPlayer * &) const
    {
        return QStringList();
    }

};

ShifengCard::ShifengCard()
{
}

bool ShifengCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *) const
{
    if (targets.isEmpty()){
        return to_select->isWounded()||Self->inMyAttackRange(to_select);
    }
    else{
        if (!Self->inMyAttackRange(targets.at(0))){
              return false;
        }
        else{
            return Self->inMyAttackRange(to_select);
        }
    }
}

void ShifengCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const
{
   int n = 0;
   QList<ServerPlayer *> list;
   foreach(auto p, targets){
       if (room->askForUseCard(p, "BasicCard+^Jink,TrickCard+^Nullification,EquipCard|.|.|hand", "@shifeng_use", -1, Card::MethodUse, false)!=NULL){
           n = n+1;
           list << p;
       }
   }
   if (n==0) {
       return;
   }
   QString choice = room->askForChoice(source, "shifeng" , "shifeng_selfdraw+shifeng_otherdraw");
   if (choice=="shifeng_selfdraw"){
       source->drawCards(n);
   }
   else{
       foreach(auto p, list){
           p->drawCards(1);
       }
   }
}

class ShifengVS : public ZeroCardViewAsSkill
{
public:
    ShifengVS() : ZeroCardViewAsSkill("shifeng"){
       response_pattern = "@@shifeng";
    }

    bool isEnabledAtPlay(const Player *player) const
    {
        return false;
    }

    const Card *viewAs() const
    {
        ShifengCard *vs = new ShifengCard();
        vs->setSkillName(objectName());
        vs->setShowSkill(objectName());
        return vs;
    }
};

class Shifeng : public TriggerSkill
{
public:
    Shifeng() : TriggerSkill("shifeng")
    {
        frequency = NotFrequent;
        events << EventPhaseStart;
        view_as_skill = new ShifengVS;
    }

    virtual bool canPreshow() const
    {
        return true;
    }

     virtual QStringList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const
    {
        if (TriggerSkill::triggerable(player) && player->getPhase()==Player::Start){
            return QStringList(objectName());
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *skill_target, QVariant &data, ServerPlayer *player) const
    {
        if (player->askForSkillInvoke(this, data) && room->askForUseCard(player, "@@shifeng", "@shifeng")){
            return false;
        }
        return false;
    }
};

ZhiyanCard::ZhiyanCard()
{
}

bool ZhiyanCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    return targets.isEmpty() && !to_select->isKongcheng() && to_select != Self && !Self->hasFlag(to_select->objectName()+"zhiyan");
}

void ZhiyanCard::extraCost(Room *, const CardUseStruct &card_use) const
{
    ServerPlayer *yukino = card_use.from;
    PindianStruct *pd = yukino->pindianSelect(card_use.to.first(), "zhiyan");
    yukino->tag["zhiyan_pd"] = QVariant::fromValue(pd);
}

void ZhiyanCard::onEffect(const CardEffectStruct &effect) const
{
    Room *room = effect.to->getRoom();
    PindianStruct *pd = effect.from->tag["zhiyan_pd"].value<PindianStruct *>();
    effect.from->tag.remove("zhiyan_pd");
    if (pd != NULL) {
        bool success = effect.from->pindian(pd);
        pd = NULL;
        room->setPlayerFlag(effect.from, effect.to->objectName()+ "zhiyan");
        if (success){
            if (!effect.to->isNude()||effect.to->getJudgingArea().length()>0){
              int id = room->askForCardChosen(effect.from, effect.to, "hej", "zhiyan");
              room->throwCard(id, effect.to, effect.from);
            }
        }
        else{
            room->askForDiscard(effect.from, "zhiyan", 1, 1);
            room->setPlayerFlag(effect.from, "zhiyan_used");
        }
    } else
        Q_ASSERT(false);
}

class Zhiyan : public ZeroCardViewAsSkill
{
public:
    Zhiyan() : ZeroCardViewAsSkill("zhiyan")
    {
    }

    virtual bool isEnabledAtPlay(const Player *player) const
    {
        return !player->isKongcheng() && !player->hasFlag("zhiyan_used");
    }

    virtual const Card *viewAs() const
    {
        ZhiyanCard *card = new ZhiyanCard;
        card->setShowSkill(objectName());
        return card;
    }

    virtual int getEffectIndex(const ServerPlayer *, const Card *) const
    {
        return 2;
    }
};

class Zuozhan : public TriggerSkill
{
public:
    Zuozhan() : TriggerSkill("zuozhan")
    {
        frequency = Frequent;
        events << EventPhaseStart << EventPhaseChanging;
    }

    virtual void record(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (event == EventPhaseChanging){
            QStringList result = player->tag["zuozhan_tag"].toStringList();
            if (result.count() == 0){
                return;
            }
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.to == Player::NotActive){
                player->tag["zuozhan_tag"].clear();
                return;
            }
            QString next = result.first();
            if (next == "1_Zuozhan"){
                change.to = Player::Judge;
            }
            else if (next == "2_Zuozhan"){
                change.to = Player::Draw;
            }
            else if (next == "3_Zuozhan"){
                change.to = Player::Play;
            }
            else if (next == "4_Zuozhan"){
                change.to = Player::Discard;
            }
            else if (next == "0_Zuozhan"){
                change.to = Player::Finish;
            }
            data.setValue(change);
            result.removeAt(0);
            if (result.count() == 0 && next != "0_Zuozhan"){
                result.append("0_Zuozhan");
            }
            player->tag["zuozhan_tag"] = QVariant(result);
        }
    }

    virtual TriggerList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
    {
        TriggerList skill_list;
        if (event==EventPhaseStart && player != NULL && player->isAlive() && player->getPhase() == Player::Start)
        {
            QList<ServerPlayer *> yuris = room->findPlayersBySkillName(objectName());
            foreach(auto c, player->getJudgingArea()){
                if (!c->isKindOf("Key")){
                    return skill_list;
                }
            }

            foreach (ServerPlayer *yuri, yuris)
            {
                if ((yuri->getHp() < player->getHp() ||( player->hasClub("sss") && yuri->hasShownSkill("nishen"))))
                {
                    skill_list.insert(yuri, QStringList(objectName()));
                }
            }
        }
        else if (event == EventPhaseStart && player->getPhase() == Player::Finish){
            player->tag["zuozhan_tag"].clear();
        }

        return skill_list;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who) const
    {
        if(ask_who->askForSkillInvoke(this, data)){
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who) const
    {
        if (event==EventPhaseStart){
            ServerPlayer * yuri = ask_who;
            room->broadcastSkillInvoke(objectName());
            if (player->hasClub("sss"))
                room->doLightbox(objectName() + "$", 800);
            QStringList choices;
            choices << "1_Zuozhan" << "2_Zuozhan" << "3_Zuozhan" << "4_Zuozhan";
            QString choice1 = room->askForChoice(yuri, "zuozhan1%from:" + player->objectName(), choices.join("+"));
            choices.removeAll(choice1);
            QString choice2 = room->askForChoice(yuri, "zuozhan2%from:" + player->objectName(), choices.join("+"));
            choices.removeAll(choice2);
            QString choice3 = room->askForChoice(yuri, "zuozhan3%from:" + player->objectName(), choices.join("+"));
            choices.removeAll(choice3);
            QString choice4 = choices.first();
            QStringList result;
            result << choice1 << choice2 << choice3 << choice4;

            player->tag["zuozhan_tag"] = QVariant(result);
        }

        return false;
    }
};

class Nishen : public TriggerSkill
{
public:
    Nishen() : TriggerSkill("nishen")
    {
        frequency = Club;
        club_name = "sss",
        events << Dying << Death;
    }
    virtual QStringList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const
   {
        if (event == Death){
            DeathStruct death = data.value<DeathStruct>();
            if (death.who->hasClub("sss") && player->hasSkill(this)){
                return QStringList(objectName());
            }
        }
        else if (event == Dying){
            DyingStruct dying = data.value<DyingStruct>();
            if (!dying.who->hasSkill(objectName())&& TriggerSkill::triggerable(player)){
                ServerPlayer *yuri = room->findPlayerBySkillName(objectName());
                if (!yuri || !yuri->isAlive()){
                    return QStringList();
                }
                QStringList used = yuri->tag["sss_targets"].toStringList();
                if (used.contains(dying.who->objectName())){
                    return QStringList();
                }
                return QStringList(objectName());

            }
        }
       return QStringList();
   }

   virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *skill_target, QVariant &data, ServerPlayer *player) const
   {
       if (event == Death && (player->hasShownSkill(this)||player->askForSkillInvoke(this, data))){
           return true;
       }
       else if(event == Dying && player->askForSkillInvoke(this, data)){
           return true;
       }
       return false;
   }

    virtual bool effect(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who) const
    {
        if (event==Death){
            DeathStruct death = data.value<DeathStruct>();
            room->setTag("no_reward_or_punish", QVariant(death.who->objectName()));
            foreach(ServerPlayer *p, room->getPlayersByClub("sss")){
                if (p->getLostHp() > 0){
                    if (room->askForChoice(p, objectName(), "nishen_draw+nishen_recover", data) == "nishen_draw"){
                        p->drawCards(2);
                    }
                    else{
                        RecoverStruct recover;
                        recover.recover = 1;
                        room->recover(p, recover, true);
                    }
                }
                else{
                    p->drawCards(2);
                }
            }
        }
        if (event == Dying){
            DyingStruct dying = data.value<DyingStruct>();
            room->broadcastSkillInvoke(objectName(), player);
            if (room->askForChoice(dying.who, "nishen", "nishen_accept+cancel", QVariant::fromValue(player)) == "nishen_accept"){
                dying.who->addClub("sss");
            }
            else{
                LogMessage log;
                log.type = "$refuse_club";
                log.from = dying.who;
                log.arg = "sss";
                room->sendLog(log);
            }
            QStringList used = player->tag["sss_targets"].toStringList();
            used.append(dying.who->objectName());
            player->tag["sss_targets"] = used;
        }
    }
};

JiuzhuCard::JiuzhuCard()
{
}

bool JiuzhuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    if (targets.length()<2){
        return to_select->getHp() <= Self->getHp() && to_select != Self;
    }
    return false;
}

void JiuzhuCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const
{
    foreach(auto p ,targets){
        RecoverStruct recover;
        recover.recover = 1;
        recover.who = source;
        room->recover(p, recover, true);
    }
    room->loseHp(source);
    if (targets.length()==1){
        targets.at(0)->drawCards(1);
    }
}


class Jiuzhu : public ZeroCardViewAsSkill
{
public:
    Jiuzhu() : ZeroCardViewAsSkill("jiuzhu")
    {
    }

    virtual bool isEnabledAtPlay(const Player *player) const
    {
        return !player->hasUsed("JiuzhuCard");
    }

    virtual const Card *viewAs() const
    {
        JiuzhuCard *card = new JiuzhuCard;
        card->setSkillName(objectName());
        card->setShowSkill(objectName());
        return card;
    }
};

ShexinCard::ShexinCard()
{
    will_throw = false;
}

bool ShexinCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    if (targets.length()<1){
        return true;
    }
    return false;
}

void ShexinCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const
{
    ServerPlayer *target = targets.at(0);
    Key *key = new Key(Card::SuitToBeDecided, -1);
    key->addSubcards(this->getSubcards());
    CardUseStruct use;
    use.from = source;
    use.to << target;
    use.card = key;
    room->useCard(use);
    target->drawCards(2);
    room->acquireSkill(target, "xintiao");
}

class ShexinVS : public OneCardViewAsSkill
{
public:
    ShexinVS() : OneCardViewAsSkill("shexin")
    {
        response_pattern = "@@shexin";
    }

    virtual bool isEnabledAtPlay(const Player *player) const
    {
        return false;
    }

    bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
        return to_select->isRed();
    }

    virtual const Card *viewAs(const Card *card) const
    {
        auto xb = new ShexinCard;
        xb->addSubcard(card);
        xb->setShowSkill("shexin");
        xb->setSkillName("shexin");
        return xb;
    }
};

class Shexin : public TriggerSkill
{
public:
    Shexin() : TriggerSkill("shexin")
    {
        frequency = Limited;
        events << Dying;
        view_as_skill= new ShexinVS;
        relate_to_place = "deputy";
        limit_mark = "@shexin";
    }

    virtual bool canPreshow() const
    {
        return true;
    }

    virtual QStringList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const
   {
       if (TriggerSkill::triggerable(player)){
           DyingStruct dying= data.value<DyingStruct>();
           if ( dying.who==player && player->getMark("@shexin")>0){
               return QStringList(objectName());
           }
       }
       return QStringList();
   }

   virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *skill_target, QVariant &data, ServerPlayer *player) const
   {
       if (player->askForSkillInvoke(this, data)){
           return true;
       }
       return false;
   }
   virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who) const
   {
        if (room->askForUseCard(player, "@@shexin", "@shexin")){
            player->loseMark("@shexin");
        }
        return false;
   }
};

class Xintiao : public TriggerSkill
{
public:
    Xintiao() : TriggerSkill("xintiao")
    {
        frequency = NotFrequent;
        events << EventPhaseStart << CardsMoveOneTime;
    }


    virtual QStringList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const
   {
       if (event == EventPhaseStart && TriggerSkill::triggerable(player) && player->getPhase()==Player::Start){
           foreach(auto p, room->getAlivePlayers()){
               foreach(auto c, p->getJudgingArea()){
                   if ( c->isKindOf("Key")){
                       return QStringList(objectName());
                   }
               }
           }
       }
       if (event == CardsMoveOneTime && TriggerSkill::triggerable(player)){
           CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
           if (!move.from || !move.from->isFriendWith(player)){
               return QStringList();
           }
           if (!move.from_places.contains(Player::PlaceDelayedTrick) || (move.to_place != Player::DiscardPile && move.to_place != Player::PlaceTable)){
               return QStringList();
           }
           /*if (move.reason.m_reason != CardMoveReason::S_REASON_DISCARD && move.reason.m_reason != CardMoveReason::S_REASON_DISMANTLE && move.reason.m_reason != CardMoveReason::S_REASON_THROW && move.reason.m_reason != CardMoveReason::S_REASON_RULEDISCARD){
               return QStringList();
           }*/
           foreach(int id, move.card_ids){
              if (VariantList2IntList(room->getTag("xintiaoList").toList()).contains(id)){
                  return QStringList(objectName());
              }
           }
       }
       return QStringList();
   }

   virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *skill_target, QVariant &data, ServerPlayer *player) const
   {
       if (player->askForSkillInvoke(this, data)){
           return true;
       }
       else if (event == CardsMoveOneTime){
           QList<QVariant> q2 = room->getTag("xintiaoList").toList();
           q2.clear();
           room->setTag("xintiaoList", q2);
       }
       return false;
   }
   virtual bool effect(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who) const
   {
        if (event == EventPhaseStart){
            QList<ServerPlayer*> list;
            QList<ServerPlayer*> list2;
            foreach(auto p, room->getAlivePlayers()){
                foreach(auto c, p->getJudgingArea()){
                    if ( c->isKindOf("Key") && !list.contains(p)){
                        list << p;
                    }
                }
            }
            foreach(auto p, room->getAlivePlayers()){
                if (!list.contains(p)){
                    list2 << p;
                }
            }
            ServerPlayer *from = room->askForPlayerChosen(player, list, objectName(), QString(), true);
            if (from && list2.length()>0){
                QList<int> ids;
                foreach(auto c, from->getJudgingArea()){
                    if ( c->isKindOf("Key")){
                        ids << c->getEffectiveId();
                    }
                }
                room->fillAG(ids, player);
                int id = room->askForAG(player, ids, true, objectName());
                room->clearAG(player);
                if (id > -1){
                    Card *card=Sanguosha->getCard(id);
                    ServerPlayer *to = room->askForPlayerChosen(player, list2, objectName(), QString(), false);
                    CardMoveReason reason=CardMoveReason(CardMoveReason::S_REASON_TRANSFER,player->objectName(),objectName(),"");
                    room->moveCardTo(card, from,to,room->getCardPlace(id),reason);
                }
            }

        }
        else{
            CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
            foreach(int id, move.card_ids){
               if (VariantList2IntList(room->getTag("xintiaoList").toList()).contains(id)){
                   room->obtainCard(player, id);
               }
            }
            QList<QVariant> q2 = room->getTag("xintiaoList").toList();
            q2.clear();
            room->setTag("xintiaoList", q2);
        }
        return false;
   }
};

ShoujiCard::ShoujiCard()
{
    will_throw=true;
}

bool ShoujiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    foreach(const Player *p, targets){
            if (to_select->isFriendWith(p)){
                return false;
            }
    }
    return !to_select->isKongcheng() && to_select != Self && targets.length() < this->getSubcards().length();
}

void ShoujiCard::use(Room *room, ServerPlayer *player, QList<ServerPlayer *> &targets) const
{
    room->doLightbox("shouji$", 1000);
    foreach(auto target, targets){
        if (!target->isKongcheng()){
            QList<int> list = target->handCards();
            room->fillAG(list,player);
            int card_id = room->askForAG(player,list,true,objectName());
            const Card *card=Sanguosha->getCard(card_id);
            if (card){
                room->obtainCard(player,card,false);
            }
            room->clearAG(player);
        }
    }
}

class Shouji : public ViewAsSkill
{
public:
    Shouji() : ViewAsSkill("shouji")
    {
    }

    bool isEnabledAtPlay(const Player *player) const
    {
        return !player->hasUsed("ShoujiCard");
    }

    bool viewFilter(const QList<const Card *> &selected, const Card *) const
    {
        return selected.length()< qMax(Self->getHp(), 1);
    }

    const Card *viewAs(const QList<const Card *> &cards) const
    {
        if (cards.length() != qMax(Self->getHp(), 1))
            return NULL;
        ShoujiCard *vs = new ShoujiCard();
        vs->addSubcards(cards);
        vs->setSkillName(objectName());
        vs->setShowSkill(objectName());
        return vs;
    }
};

class Haoqi : public TriggerSkill
{
public:
    Haoqi() : TriggerSkill("haoqi")
    {
        frequency = NotFrequent;
        events << EventPhaseStart << EventPhaseChanging << Death << CardUsed;
    }

    virtual TriggerList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
   {
        TriggerList skill_list;
        if(event == CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            if (!use.card->isKindOf("BasicCard") && !use.card->isNDTrick()){
                return skill_list;
            }
            if (use.from->getPhase() != Player::Play || use.from->hasShownAllGenerals()){
                return skill_list;
            }
            QList< ServerPlayer*> chitandas = room->findPlayersBySkillName(objectName());
            foreach(auto chitanda, chitandas){
                if (!use.from->hasFlag(chitanda->objectName()+ "haoqi") && use.from != chitanda){
                   skill_list.insert(chitanda, QStringList(objectName()));
                }
            }
        }
        return skill_list;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *chitanda) const
    {
        if(chitanda->askForSkillInvoke(this, data)){
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *chitanda) const
    {
        if (event == CardUsed){
            room->broadcastSkillInvoke(objectName(), chitanda);
            room->setPlayerFlag(room->getCurrent(),chitanda->objectName()+ "haoqi");
            CardUseStruct use = data.value<CardUseStruct>();
            if (!use.from->askForGeneralShow(false, true)){
                return true;
            }
            else if(use.card && use.card->isRed()){
                chitanda->drawCards(1);
            }
        }
        return false;
    }
};

ShashouCard::ShashouCard()
{
}

bool ShashouCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    return targets.length()<1 && to_select->hasShownOneGeneral();
}

void ShashouCard::use(Room *room, ServerPlayer *player, QList<ServerPlayer *> &targets) const
{
    ServerPlayer *p = targets.at(0);
    QStringList choices;
    if (p->hasShownGeneral1()){
        choices << p->getActualGeneral1Name();
    }
    if (p->hasShownGeneral2()){
        choices << p->getActualGeneral2Name();
    }
    QString general="";
    if (!choices.isEmpty()){
        general=room->askForGeneral(player,choices.join("+"),QString(),true,"shashou");
    }
    if (p!=player){
        if (general == p->getActualGeneral1Name()){
          p->hideGeneral(true);
          room->setPlayerDisableShow(p, "h", "shashou");
        }
        else{
          p->hideGeneral(false);
          room->setPlayerDisableShow(p, "d", "shashou");
        }
    }
    else{
        if (general == p->getActualGeneral1Name()){
          p->hideGeneral(true);
        }
        else{
          p->hideGeneral(false);
        }
    }
}


class Shashouvs : public OneCardViewAsSkill
{
public:
    Shashouvs() : OneCardViewAsSkill("shashou")
    {
    }

    bool isEnabledAtPlay(const Player *player) const
    {
        return !player->hasUsed("ShashouCard");
    }

    bool viewFilter(const Card *card) const
    {
        return !card->isEquipped();
    }

    const Card *viewAs(const Card *card) const
    {
        ShashouCard *vs = new ShashouCard();
        vs->addSubcard(card);
        vs->setSkillName(objectName());
        vs->setShowSkill(objectName());
        return vs;
    }
};

class Shashou : public TriggerSkill
{
public:
    Shashou() : TriggerSkill("shashou")
    {

        events << DamageCaused << EventPhaseEnd;
        frequency = NotFrequent;
        view_as_skill = new Shashouvs;
    }

    virtual bool canPreshow() const
    {
        return true;
    }


    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const
    {
        if (triggerEvent == DamageCaused) {
            DamageStruct damage = data.value<DamageStruct>();
            if (TriggerSkill::triggerable(player) && !player->hasShownAllGenerals())
                return QStringList(objectName());
        }
        if (triggerEvent == EventPhaseEnd) {
           if (player->getPhase()==Player::Finish){
              foreach(auto p, room->getAlivePlayers()){
                 room->removePlayerDisableShow(p,objectName());
              }
           }
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        if (triggerEvent == DamageCaused && player->askForSkillInvoke(this, data)) {
            if (player->askForGeneralShow(true, true)){
              return true;
            }
        }
        return false;
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        if (triggerEvent == DamageCaused){
            DamageStruct damage = data.value<DamageStruct>();
            room->broadcastSkillInvoke(objectName(), player);
            damage.damage=damage.damage+1;
            data.setValue(damage);
        }
        return false;
    }
};

class Erdao : public TriggerSkill
{
public:
    Erdao() : TriggerSkill("erdao")
    {
        frequency = Frequent;
        events << EventPhaseStart;
    }

     virtual QStringList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const
    {
        if (TriggerSkill::triggerable(player) && player->getPhase()==Player::Start){
            return QStringList(objectName());
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *skill_target, QVariant &data, ServerPlayer *player) const
    {
        if (player->askForSkillInvoke(this, data)){
            room->broadcastSkillInvoke(objectName(), player);
            return true;
        }
        return false;
    }
    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who) const
    {
        QString choice = room->askForChoice(player, objectName(), "erdao_extraslash+erdao_extratarget");
        if (choice == "erdao_extraslash") {
            room->setPlayerFlag(player, "erdao_extraslash");
            LogMessage log;
            log.from = player;
            log.type = "$ErdaoExtraslash";
            room->sendLog(log);
        }
        else{
            room->setPlayerFlag(player, "erdao_extratarget");
            LogMessage log;
            log.from = player;
            log.type = "$ErdaoExtratarget";
            room->sendLog(log);
        }
        return false;
    }
};

class ErdaoExtra : public TargetModSkill
{
public:
    ErdaoExtra() : TargetModSkill("#erdaoextra")
    {
    }

    virtual int getResidueNum(const Player *from, const Card *card) const
    {
        if (!Sanguosha->matchExpPattern(pattern, from, card))
            return 0;

        if (from->hasFlag("erdao_extraslash"))
            return 1;
        else
            return 0;
    }
    virtual int getExtraTargetNum(const Player *from, const Card *card) const
    {
        if (!Sanguosha->matchExpPattern(pattern, from, card))
            return 0;

        if (from->hasFlag("erdao_extratarget"))
            return 1;
        else
            return 0;
    }
};

class Fengbi : public TriggerSkill
{
public:
    Fengbi() : TriggerSkill("fengbi")
    {
        frequency = Frequent;
        events << GeneralShown << DrawNCards << GeneralHidden << GeneralTransformed;
    }

    virtual bool canPreshow() const
    {
        return false;
    }

    virtual QStringList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const
   {
       if (event==DrawNCards && TriggerSkill::triggerable(player) && player->hasShownOneGeneral() && !player->hasShownAllGenerals() && player->hasShownGeneral1() == player->inHeadSkills(this)){
           if (!player->hasShownOneGeneral())
               return QStringList();
           QStringList big_kingdoms = player->getBigKingdoms(objectName(), MaxCardsType::Max);
           bool invoke = !big_kingdoms.isEmpty();
           if (invoke) {
               if (big_kingdoms.length() == 1 && big_kingdoms.first().startsWith("sgs")) // for JadeSeal
                   invoke = big_kingdoms.contains(player->objectName());
               else if (player->getRole() == "careerist")
                   invoke = false;
               else
                   invoke = big_kingdoms.contains(player->getKingdom());
           }
           if (!invoke){
               return QStringList(objectName());
           }
       }
       if (event == GeneralShown || event == GeneralTransformed){
           if ( TriggerSkill::triggerable(player) && player->hasShownAllGenerals()){
               bool head = player->inHeadSkills(this);
               //room->detachSkillFromPlayer(player, objectName(), false, false, head);
               room->acquireSkill(player, "xingbao", true, head);
           }
           else if(event == GeneralTransformed && !player->hasShownSkill("fengbi")){
               if (player->hasSkill("xingbao")){
                   room->detachSkillFromPlayer(player, "xingbao", false, false, player->inHeadSkills("xingbao"));
               }
           }
       }
       if (event == GeneralHidden){
           if (player->hasSkill("xingbao")){
               room->detachSkillFromPlayer(player, "xingbao", false, false, data.toBool());
           }
       }
       return QStringList();
   }

   virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *skill_target, QVariant &data, ServerPlayer *player) const
   {
       if (player->askForSkillInvoke(this, data)){
           int n=0;
           foreach(auto p, room->getAlivePlayers()){
               if (player->isFriendWith(p)){
                   n = n+1;
               }
           }

           data = data.toInt() + n;
           room->broadcastSkillInvoke(objectName(), player);
           return true;
       }
       return false;
   }
   virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who) const
   {
        return false;
   }
};


XingbaoCard::XingbaoCard()
{
    target_fixed = true;
}

void XingbaoCard::onUse(Room *room, const CardUseStruct &card_use) const
{
    CardMoveReason reason(CardMoveReason::S_REASON_RECAST, card_use.from->objectName());
    reason.m_skillName = getSkillName();
    room->moveCardTo(this, card_use.from, NULL, Player::PlaceTable, reason, true);
    card_use.from->broadcastSkillInvoke("@recast");
    room->broadcastSkillInvoke("xingbao", card_use.from);

    LogMessage log;
    log.type = "#Card_Recast";
    log.from = card_use.from;
    log.card_str = card_use.card->toString();
    room->sendLog(log);

    QString skill_name = card_use.card->showSkill();
    if (!skill_name.isNull() && card_use.from->ownSkill(skill_name) && !card_use.from->hasShownSkill(skill_name))
        card_use.from->showGeneral(card_use.from->inHeadSkills(skill_name));

    QList<int> table_cardids = room->getCardIdsOnTable(this);
    if (!table_cardids.isEmpty())
    {
        DummyCard dummy(table_cardids);
        room->moveCardTo(&dummy, card_use.from, NULL, Player::DiscardPile, reason, true);
    }

    card_use.from->drawCards(1);
}

class XingbaoVS : public OneCardViewAsSkill
{
public:
    XingbaoVS() : OneCardViewAsSkill("xingbao")
    {
        response_pattern = "@@xingbao";
    }

    virtual bool isEnabledAtPlay(const Player *player) const
    {
        return false;
    }

    bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
        if (Self->hasFlag("xingbao_red")){
            return to_select->isRed();
        }
        else if (Self->hasFlag("xingbao_black")){
            return to_select->isBlack();
        }
    }

    virtual const Card *viewAs(const Card *card) const
    {
        auto xb = new XingbaoCard;
        xb->addSubcard(card);
        xb->setShowSkill("xingbao");
        xb->setSkillName("xingbao");
        return xb;
    }
};

class Xingbao : public TriggerSkill
{
public:
    Xingbao() : TriggerSkill("xingbao")
    {
        frequency = NotFrequent;
        events << Damage << CardFinished;
        view_as_skill= new XingbaoVS;
    }

    virtual bool canPreshow() const
    {
        return true;
    }

    virtual QStringList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const
   {
       if (event == Damage && TriggerSkill::triggerable(player)){
           DamageStruct damage = data.value<DamageStruct>();
           if ( damage.card && damage.card->isKindOf("Slash") && (damage.card->isRed() || damage.card->isBlack())){
               return QStringList(objectName());
           }
       }
       if (event == CardFinished){
           CardUseStruct use = data.value<CardUseStruct>();
           if (use.card->hasFlag("xingbao_card")){
               use.card->clearFlags();
           }
       }
       return QStringList();
   }

   virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *skill_target, QVariant &data, ServerPlayer *player) const
   {
       if (player->askForSkillInvoke(this, data)){
           return true;
       }
       return false;
   }
   virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who) const
   {
        DamageStruct damage = data.value<DamageStruct>();
        if (damage.card && damage.card->isRed()){
            room->setPlayerFlag(player, "xingbao_red");
        }
        if (damage.card && damage.card->isBlack()){
            room->setPlayerFlag(player, "xingbao_black");
        }
        if (room->askForUseCard(player, "@@xingbao", "@xingbao-recast") != NULL && !damage.card->hasFlag("xingbao_card"))
        {
            damage.card->setFlags("xingbao_card");
            room->addPlayerHistory(player, damage.card->getClassName(), -1);
        }
        room->setPlayerFlag(player, "-xingbao_red");
        room->setPlayerFlag(player, "-xingbao_black");
        return false;
   }
};

class Rennai : public TriggerSkill
{
public:
    Rennai() : TriggerSkill("rennai")
    {
        frequency = Compulsory;
        events << DamageInflicted << PreHpLost << EventPhaseStart << Death;
    }

    void calcFreeze(Room *room, ServerPlayer *player) const{
            if (!player || player->isDead()){
                return;
            }
            if (room->askForChoice(player, objectName(), "rennai_hp+rennai_handcardnum") == "rennai_hp"){
                QStringList hps;
                foreach(ServerPlayer *p, room->getAlivePlayers()){

                    if (!hps.contains(QString::number(p->getHp()))){
                        hps.append(QString::number(p->getHp()));
                    }
                }
                int targetHp = room->askForChoice(player, objectName(), hps.join("+"), QVariant("hp")).toInt();
                if (room->askForChoice(player, objectName(), "rennai_gain+rennai_lose", QVariant("hp+" + QString::number(targetHp))) == "rennai_gain"){
                    foreach(ServerPlayer *p, room->getAlivePlayers()){
                        if (p->getHp() == targetHp){
                            p->gainMark("@Frozen_Eu");
                        }
                    }
                }
                else{
                    foreach(ServerPlayer *p, room->getAlivePlayers()){
                        if (p->getHp() == targetHp){
                            p->loseMark("@Frozen_Eu");
                        }
                    }
                }
            }
            else{
                QStringList handcardnums;
                foreach(ServerPlayer *p, room->getAlivePlayers()){
                    if (!handcardnums.contains(QString::number(p->getHandcardNum()))){
                        handcardnums.append(QString::number(p->getHandcardNum()));
                    }
                }
                int targetHandcardnum = room->askForChoice(player, objectName(), handcardnums.join("+"), QVariant("handcardnum")).toInt();
                if (room->askForChoice(player, objectName(), "rennai_gain+rennai_lose", QVariant("handcardnum+" + QString::number(targetHandcardnum))) == "rennai_gain"){
                    foreach(ServerPlayer *p, room->getAlivePlayers()){
                        if (p->getHandcardNum() == targetHandcardnum){
                            p->gainMark("@Frozen_Eu");
                        }
                    }
                }
                else{
                    foreach(ServerPlayer *p, room->getAlivePlayers()){
                        if (p->getHandcardNum() == targetHandcardnum){
                            p->loseMark("@Frozen_Eu");
                        }
                    }
                }
            }
        }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const
   {
       if (TriggerSkill::triggerable(player)){
           if (triggerEvent == DamageInflicted){
               DamageStruct damage = data.value<DamageStruct>();
               if (damage.to == player){
                  return QStringList(objectName());
               }
           }
           else if (triggerEvent == PreHpLost ){
               if ( player->getMark("@Patience") > 0){
                  return QStringList(objectName());
               }
           }
           else if (triggerEvent == EventPhaseStart && player->getPhase() == Player::Start){
               player->loseAllMarks("@Patience");
           }
       }
       else if (triggerEvent == Death){
           DeathStruct death = data.value<DeathStruct>();
           if (death.who->hasSkill(objectName())){
               foreach(ServerPlayer *p, room->getAlivePlayers()){
                   p->loseAllMarks("@Frozen_Eu");
               }
           }
       }
       return QStringList();
   }

    virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *skill_target, QVariant &data, ServerPlayer *player) const
    {
        if (player->hasShownSkill(this)||player->askForSkillInvoke(this, data)){
            return true;
        }
        return false;
    }
    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who) const
    {
        if (triggerEvent == DamageInflicted){
                    DamageStruct damage = data.value<DamageStruct>();
                    if (damage.to->hasSkill(objectName()) && damage.to->getMark("@Patience") == 0){
                        room->loseHp(damage.to);
                        room->broadcastSkillInvoke(objectName(), rand()%2+1, player);
                        room->doLightbox(objectName() + "$", 800);
                        damage.to->gainMark("@Patience");
                        calcFreeze(room, damage.to);
                        return true;
                    }
                    else if (damage.to->hasSkill(objectName()) && damage.to->getMark("@Patience") > 0){
                        room->broadcastSkillInvoke(objectName(), rand()%2+3, player);
                        if (damage.from){
                            LogMessage log;
                            log.type = "$rennai_effect";
                            log.arg = damage.from->getGeneralName();
                            room->sendLog(log);
                            player->loseMark("@Patience");
                            calcFreeze(room, damage.to);
                        }
                        else if (damage.card){
                            LogMessage log;
                            log.type = "$rennai_effect";
                            log.arg = damage.card->getClassName();
                            room->sendLog(log);
                            player->loseMark("@Patience");
                            calcFreeze(room, damage.to);
                        }
                        else{
                            player->loseMark("@Patience");
                            calcFreeze(room, damage.to);
                        }
                        return true;
                    }
                }
                else if (triggerEvent == PreHpLost){
                    if (player->hasSkill(objectName()) && player->getMark("@Patience") > 0){
                        room->broadcastSkillInvoke(objectName(), rand()%2+3, player);
                        LogMessage log;
                        log.type = "$rennai_effect2";
                        room->sendLog(log);
                        calcFreeze(room, player);
                        player->loseMark("@Patience");
                        return true;
                    }
                }
        return false;
    }
};

class Zhanfang : public TriggerSkill
{
public:
    Zhanfang() : TriggerSkill("zhanfang")
    {
        frequency = NotFrequent;
        events << PreCardUsed << CardFinished << TrickCardCanceling << SlashProceed;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const
   {
        if (triggerEvent == PreCardUsed){
                   CardUseStruct use = data.value<CardUseStruct>();
                   if (use.from->hasSkill(objectName())) {

                       if (!use.card->isKindOf("BasicCard") && !use.card->isNDTrick()){
                           return QStringList();
                       }

                       if (use.to.count() != 1){
                           return QStringList();
                       }
                       ServerPlayer *target = use.to.first();

                       if (target->getMark("@Frozen_Eu") > 0 ){
                           return QStringList(objectName());
                       }
                   }


               }
        else if (triggerEvent == CardFinished){
                    CardUseStruct use = data.value<CardUseStruct>();
                    if (use.from->isAlive() && use.card->hasFlag("zhanfang_card")){
                        foreach(ServerPlayer *p, use.to){
                            if (p->isAlive() && p->getEquips().count() > 0 && room->askForChoice(p, objectName(), "zhanfang_discard+cancel", data) == "zhanfang_discard"){
                                room->throwCard(room->askForCardChosen(p, p, "e", objectName()), p);
                                p->loseMark("@Frozen_Eu");
                            }
                        }
                    }
                }
        else if (triggerEvent == TrickCardCanceling && TriggerSkill::triggerable(player)){
                    CardEffectStruct effect = data.value<CardEffectStruct>();
                    if (effect.from && effect.from->isAlive() && effect.from->hasSkill(objectName()) && effect.to && effect.to->getMark("@Frozen_Eu") > 0){
                        return QStringList(objectName());
                    }
                }
                else if (triggerEvent == SlashProceed && TriggerSkill::triggerable(player)){
                    SlashEffectStruct effect = data.value<SlashEffectStruct>();
                    if (effect.from && effect.from->isAlive() && effect.from->hasSkill(objectName()) && effect.to && effect.to->getMark("@Frozen_Eu") > 0){
                        return QStringList(objectName());
                    }
                }
       return QStringList();
   }

    virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *skill_target, QVariant &data, ServerPlayer *player) const
    {
        if (event == PreCardUsed && player->askForSkillInvoke(this, data)){
            return true;
        }
        else if((event == TrickCardCanceling ||event == SlashProceed )&&(player->hasShownSkill(this)||player->askForSkillInvoke(this, data))){
            return true;
        }
        return false;
    }
    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who) const
    {
       if (triggerEvent == PreCardUsed){
           CardUseStruct use = data.value<CardUseStruct>();
           ServerPlayer *target = use.to.first();

                           if (target->getMark("@Frozen_Eu") > 0){
                               use.to.clear();
                               QList<ServerPlayer *> list = room->getAlivePlayers();
                               room->sortByActionOrder(list);
                               foreach(ServerPlayer *p, list){
                                   if (p->getMark("@Frozen_Eu") > 0 && !use.to.contains(p)){
                                       use.to.append(p);
                                   }
                               }
                               use.card->setFlags("zhanfang_card");
                               room->broadcastSkillInvoke(objectName(), player);
                               room->doLightbox(objectName() + "$", 800);
                               data = QVariant::fromValue(use);
                           }
       }
       else if (triggerEvent == TrickCardCanceling){
                   CardEffectStruct effect = data.value<CardEffectStruct>();
                   if (effect.from && effect.from->isAlive() && effect.from->hasSkill(objectName()) && effect.to && effect.to->getMark("@Frozen_Eu") > 0){
                       LogMessage log;
                       log.type = "$zhanfang_effect";
                       log.from = effect.to;
                       log.arg = effect.card->objectName();
                       room->sendLog(log);
                       return true;
                   }
               }
               else if (triggerEvent == SlashProceed){
                   SlashEffectStruct effect = data.value<SlashEffectStruct>();
                   if (effect.from && effect.from->isAlive() && effect.from->hasSkill(objectName()) && effect.to && effect.to->getMark("@Frozen_Eu") > 0){
                       LogMessage log;
                       log.type = "$zhanfang_effect";
                       log.from = effect.to;
                       log.arg = effect.slash->objectName();
                       room->sendLog(log);
                       room->slashResult(effect, NULL);
                       return true;
                   }
               }
       return false;
    }
};

YaozhanCard::YaozhanCard()
{
}

bool YaozhanCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    if (targets.isEmpty()){
        return !to_select->hasShownOneGeneral() || !to_select->isFriendWith(Self);
    }
    return false;
}

void YaozhanCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const
{
   ServerPlayer *target = targets.at(0);
   bool slash = room->askForUseSlashTo(target, source, "@yaozhan-slash:" + source->objectName(), false);
   if (slash && source->isAlive() && target->isAlive()){
       room->askForUseSlashTo(source, target, "@yaozhan-slash:" + source->objectName(), false);
   }
   else{
       QStringList choices;
       if (!target->isKongcheng())
           choices << "handcards";
       if (!target->hasShownAllGenerals())
           choices << "hidden_general";

       room->setPlayerFlag(target, "yaozhanTarget");        //for AI
       QString choice = room->askForChoice(source, "yaozhan%to:" + target->objectName(),
           choices.join("+"), QVariant::fromValue(target));
       room->setPlayerFlag(target, "-yaozhanTarget");
       LogMessage log;
       log.type = "#KnownBothView";
       log.from = source;
       log.to << target;
       log.arg = choice;
       foreach (ServerPlayer *p, room->getOtherPlayers(source, true))
           room->doNotify(p, QSanProtocol::S_COMMAND_LOG_SKILL, log.toVariant());

       if (choice.contains("handcards")) {
           room->showAllCards(target, source);
       } else {
           QStringList list, list2;
           if (!target->hasShownGeneral1()) {
               list << "head_general";
               list2 << target->getActualGeneral1Name();
           }
           if (!target->hasShownGeneral2()) {
               list << "deputy_general";
               list2 << target->getActualGeneral2Name();
           }
           foreach (const QString &name, list) {
               LogMessage log;
               log.type = "$KnownBothViewGeneral";
               log.from = source;
               log.to << target;
               log.arg = Sanguosha->translate(name);
               log.arg2 = (name == "head_general" ? target->getActualGeneral1Name() : target->getActualGeneral2Name());
               room->doNotify(source, QSanProtocol::S_COMMAND_LOG_SKILL, log.toVariant());
           }
           JsonArray arg;
           arg << "yaozhan";
           arg << JsonUtils::toJsonArray(list2);
           room->doNotify(source, QSanProtocol::S_COMMAND_VIEW_GENERALS, arg);
       }
   }
}

class Yaozhan : public ZeroCardViewAsSkill
{
public:
    Yaozhan() : ZeroCardViewAsSkill("yaozhan"){
    }

    bool isEnabledAtPlay(const Player *player) const
    {
        return !player->hasUsed("YaozhanCard");
    }

    const Card *viewAs() const
    {
        YaozhanCard *vs = new YaozhanCard();
        vs->setSkillName(objectName());
        vs->setShowSkill(objectName());
        return vs;
    }
};

class Lianji : public TriggerSkill
{
public:
    Lianji() : TriggerSkill("lianji")
    {
        frequency = Frequent;
        events << EventPhaseEnd;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const
   {
        if (TriggerSkill::triggerable(player) && player->getPhase()==Player::Finish && player->getMark("lianji_times")> player->getHp() && player->getMark("lianji_turns")<3) {
            return QStringList(objectName());
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        if (player->askForSkillInvoke(this, data)) {
           room->broadcastSkillInvoke(objectName(), player);
           return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        room->setPlayerMark(player, "lianji_times", 0);
        room->setPlayerMark(player, "lianji_turns", player->getMark("lianji_turns")+1);
        player->gainAnInstantExtraTurn();
        return false;
    }
};

class LianjiTrigger : public TriggerSkill
{
public:
    LianjiTrigger() : TriggerSkill("#lianji")
    {
        frequency = NotFrequent;
        events << CardFinished << Damage << EventPhaseStart;
        global=true;
    }

    virtual void record(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (event == CardFinished) {
             CardUseStruct use = data.value<CardUseStruct>();
             if (use.card!= NULL && use.card->isKindOf("Slash")){
                 room->setPlayerMark(player,"lianji_times",player->getMark("lianji_times")+1);
             }
        }
        if (event == Damage) {
             DamageStruct damage = data.value<DamageStruct>();
             if (damage.damage>0){
                 room->setPlayerMark(player,"lianji_times",player->getMark("lianji_times")+1);
             }
        }
        if (event == EventPhaseStart){
            if (player->getPhase()==Player::Start){
                foreach(auto p, room->getOtherPlayers(player)){
                    room->setPlayerMark(p, "lianji_turns", 0);
                }
            }
        }
        return;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const
   {
        return QStringList();
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        return false;
    }
};

class Lianchui : public TriggerSkill
{
public:
    Lianchui() : TriggerSkill("lianchui")
    {
        frequency = NotFrequent;
        events << CardUsed;
        relate_to_place = "head";
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const
   {
        if (triggerEvent==CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            if (TriggerSkill::triggerable(player) && use.card != NULL && use.card->isKindOf("Slash")&& player == use.from) {
                QList<ServerPlayer*> targets;
                foreach (ServerPlayer *to, room->getOtherPlayers(player)) {
                   if (!use.to.contains(to)&&player->inMyAttackRange(to)){
                       targets << to;
                   }
                }
                if (!targets.isEmpty())
                    return QStringList(objectName());
            }
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        if (player->askForSkillInvoke(this, data)) {
            QList<ServerPlayer*> targets;
            CardUseStruct use = data.value<CardUseStruct>();
            foreach (ServerPlayer *to, room->getOtherPlayers(player)) {
               if (!use.to.contains(to)&&player->inMyAttackRange(to)){
                   targets << to;
               }
            }
            ServerPlayer *target=room->askForPlayerChosen(player,targets,objectName(),QString(),true,true);
            if (target){
                player->tag["lianchui_target"] = QVariant::fromValue(target);
                return true;
            }
        }
        return false;
    }

    virtual bool effect(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        ServerPlayer *target = player->tag["lianchui_target"].value<ServerPlayer *>();
        if (target){
            room->broadcastSkillInvoke(objectName(),player);
            CardUseStruct use = data.value<CardUseStruct>();
            if (!target->hasShownOneGeneral()){
                target->askForGeneralShow(true, true);
            }
            if (!target->hasShownOneGeneral() || !target->isFriendWith(player)){
                use.to.append(target);
                data.setValue(use);
            }
        }
        return false;
    }
};

ZhouliCard::ZhouliCard()
{
    will_throw=true;
}

bool ZhouliCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    return !to_select->isFriendWith(Self) && targets.length() == 0;
}

void ZhouliCard::use(Room *room, ServerPlayer *player, QList<ServerPlayer *> &targets) const
{
    ServerPlayer *target = targets.at(0);
    if (!target)
        return;
    if (!target->isKongcheng()){
        QList<int> list = target->handCards();
        QList<int> disabled_ids;
        foreach(int id,list){
            const Card *card=Sanguosha->getCard(id);
            if (card->getColor()!=this->getColor()) {
                disabled_ids.append(id);
            }
        }

        room->fillAG(list,player);
        int card_id = room->askForAG(player,list,true,objectName());
        const Card *card=Sanguosha->getCard(card_id);
        if (card){
            room->obtainCard(player,card,false);
        }
        room->clearAG(player);
    }

    int min = 0;
    bool con;
    foreach (auto p, room->getOtherPlayers(player))
    {
        min = qMin(min, p->getHandcardNum());
    }

    if (player->getHandcardNum() > min)
    {
        con = true;
    }
    if (!con)
        return;
    QList<ServerPlayer *> list;
    foreach(auto p, room->getAlivePlayers()){
        if (p->isFriendWith(player)){
            list << p;
        }
    }
    if (list.isEmpty())
        return;
    ServerPlayer *dest = room->askForPlayerChosen(player, list, objectName(), QString());
    QStringList choices;
    QStringList skills;
    if (dest->hasShownGeneral1()){
        choices << dest->getActualGeneral1Name();
    }
    if (dest->hasShownGeneral2()){
        choices << dest->getActualGeneral2Name();
    }
    QString general="";
    if (!choices.isEmpty()){
        general=room->askForGeneral(player,choices.join("+"),QString(),true,objectName());
    }
    const General *g=Sanguosha->getGeneral(general);
    foreach(const Skill *skill, g->getVisibleSkillList()){
        if (dest->hasShownSkill(skill->objectName())){
           skills<<skill->objectName();
        }
    }
    QString skill = room->askForChoice(player, "zhouli", skills.join("+"));
    room->detachSkillFromPlayer(dest, skill, false, false, dest->inHeadSkills(skill));
}

/*class Zhouli : public ZeroCardViewAsSkill
{
public:
    Zhouli() : ZeroCardViewAsSkill("zhouli")
    {
    }

    bool isEnabledAtPlay(const Player *player) const
    {
        return !player->hasUsed("ZhouliCard");
    }

    const Card *viewAs() const
    {
        ZhouliCard *vs = new ZhouliCard();
        vs->setSkillName(objectName());
        vs->setShowSkill(objectName());
        return vs;
    }
};*/

class Zhouli : public TriggerSkill
{
public:
    Zhouli() : TriggerSkill("zhouli")
    {
        frequency = NotFrequent;
        events << DamageCaused << EventPhaseStart;
    }

    virtual TriggerList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
    {
        TriggerList skill_list;
        if (event==DamageCaused){
            QList<ServerPlayer *> sakis = room->findPlayersBySkillName(objectName());
            foreach (ServerPlayer *saki, sakis) {
                if (room->getCurrent() && !room->getCurrent()->hasFlag(saki->objectName()+"zhouli")){
                     skill_list.insert(saki, QStringList(objectName()));
                }
            }
        }
        /*if (event == EventPhaseStart && player->getPhase() == Player::Start){
            //room->removePlayerDisableShow(player, objectName());
        }*/
        return skill_list;
    }

    virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *saki) const
   {
        if (saki->askForSkillInvoke(this, data)) {
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *saki) const
    {
        room->setPlayerFlag(room->getCurrent(), saki->objectName()+"zhouli");
        DamageStruct damage = data.value<DamageStruct>();
        if (!player->hasShownOneGeneral()){
            player->askForGeneralShow(true, true);
        }
        if ((!player->hasShownOneGeneral() || !player->isFriendWith(saki))&& !damage.to->hasShownAllGenerals()){
            room->loseHp(player, damage.damage);
        }
        else if (player->isFriendWith(saki)){
            if (player->hasShownAllGenerals()){
               QString choice = room->askForChoice(player, objectName(), "zhouli_head+zhouli_deputy+cancel", data);
               if (choice=="zhouli_head"){
                   player->hideGeneralWithoutChangingRole(true);
                   room->setPlayerDisableShow(player, "h", objectName());
               }
               if (choice=="zhouli_deputy"){
                   player->hideGeneralWithoutChangingRole(false);
                   room->setPlayerDisableShow(player, "d", objectName());
               }
            }
            if (room->askForChoice(player, objectName(), "zhouli_prevent+cancel", data) == "zhouli_prevent"){
                player->drawCards(2);
                return true;
            }
        }
        return false;
    }
};

class ZhouliRecord : public TriggerSkill
{
public:
    ZhouliRecord() : TriggerSkill("#zhouli-record")
    {
        events << EventPhaseStart;
    }

    virtual void record(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (event==EventPhaseStart && player->getPhase() == Player::Start){
            room->removePlayerDisableShow(player, "zhouli");
        }
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *, QVariant &, ServerPlayer* &) const
    {
        return QStringList();
    }

};

class ZhenyanRecord : public TriggerSkill
{
public:
    ZhenyanRecord() : TriggerSkill("#zhenyan-record")
    {
        events << CardsMoveOneTime << EventPhaseEnd;
    }

    virtual void record(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (event==CardsMoveOneTime && TriggerSkill::triggerable(player)){
            CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
            if (move.from  && move.from->getPhase()==Player::NotActive && (move.from_places.contains(Player::PlaceHand)|| move.from_places.contains(Player::PlaceEquip)) && (!move.to || move.to != move.from)){
                QStringList list = room->getTag(move.from->objectName()+"zhenyan").toStringList();
                foreach (int id, move.card_ids) {
                    if (!list.contains(QString::number(Sanguosha->getCard(id)->getTypeId()))){
                      list <<QString::number(Sanguosha->getCard(id)->getTypeId());
                    }
                }
                room->setTag(move.from->objectName()+"zhenyan", QVariant(list));
            }
        }
        else if(event == EventPhaseEnd && player->getPhase()==Player::Finish) {
            foreach(auto p, room->getAlivePlayers()){
               room->setTag(p->objectName()+"zhenyan", QVariant());
            }
        }
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *, QVariant &, ServerPlayer* &) const
    {
        return QStringList();
    }

};

class Zhenyan : public TriggerSkill
{
public:
    Zhenyan() : TriggerSkill("zhenyan")
    {
        frequency = NotFrequent;
        events << EventPhaseStart;
    }

    virtual TriggerList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
    {
        TriggerList skill_list;
        if (event==EventPhaseStart){
            if ( player->getPhase()==Player::Finish){
                QList<ServerPlayer *> sakis = room->findPlayersBySkillName(objectName());
                foreach (ServerPlayer *saki, sakis) {
                   foreach(auto p, room->getAlivePlayers()){
                       if (player==saki && p != player && room->getTag(p->objectName()+"zhenyan").toStringList().length()>0){
                            skill_list.insert(saki, QStringList(objectName()));
                       }
                   }
                }
            }
        }
        return skill_list;
    }

    virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *saki) const
   {
        QList<ServerPlayer *> list;
        foreach(auto p, room->getAlivePlayers()){
            if (p != player && room->getTag(p->objectName()+"zhenyan").toStringList().length()>0){
                 list << p;
            }
        }

        if (event==EventPhaseStart && saki->askForSkillInvoke(this, data) ) {
            ServerPlayer *target=room->askForPlayerChosen(saki,list,objectName(),QString(),true,true);
            if (target){
              saki->tag["zhenyan_target"] = QVariant::fromValue(target);
              return true;
            }
        }
        return false;
    }

    virtual bool effect(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *saki) const
    {
        if (event==EventPhaseStart){
            room->broadcastSkillInvoke(objectName(),saki);
            ServerPlayer *dest = saki->tag["zhenyan_target"].value<ServerPlayer *>();
            QStringList list = room->getTag(dest->objectName()+"zhenyan").toStringList();
            saki->drawCards(1);
            if (!saki->isNude()){
                int id = room->askForCardChosen(saki, saki, "he", objectName());
                room->obtainCard(dest, id);
                /*Card *card = Sanguosha->getCard(id);
                QString type = QString::number(card->getTypeId());
                if (list.contains(type)){
                    QStringList choices;
                    QStringList skills;
                    if (dest->hasShownGeneral1()){
                        choices << dest->getActualGeneral1Name();
                    }
                    if (dest->hasShownGeneral2()){
                        choices << dest->getActualGeneral2Name();
                    }
                    QString general="";
                    if (!choices.isEmpty()){
                        general=room->askForGeneral(saki,choices.join("+"),QString(),true,objectName());
                    }
                    const General *g=Sanguosha->getGeneral(general);
                    foreach(const Skill *skill, g->getVisibleSkillList()){
                        if (!dest->hasShownSkill(skill->objectName())){
                           skills<<skill->objectName();
                        }
                    }
                    QString skill = room->askForChoice(saki, objectName(), skills.join("+"));
                    room->acquireSkill(dest, skill, true, dest->getActualGeneral1()->hasSkill(skill));
                    if (room->askForChoice(dest,"transform","transform+cancel",data)=="transform"){
                        dest->showGeneral(false);
                        room->transformDeputyGeneral(dest);
                    }
                }*/
            }
        }
        return false;
    }
};

class Zhuisha : public TriggerSkill
{
public:
    Zhuisha() : TriggerSkill("zhuisha")
    {
        frequency = NotFrequent;
        events << TargetConfirmed;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &) const
    {
        CardUseStruct use = data.value<CardUseStruct>();
        if (TriggerSkill::triggerable(player)&& use.from && use.from==player && use.to.count() == 1 && use.card->getTypeId() != Card::TypeSkill){
            ServerPlayer *p = use.to.at(0);
            if (!player->isNude() && !player->hasFlag(p->objectName()+"zhuisha")&&(!room->getCurrent()||!room->getCurrent()->hasFlag(p->objectName()+"zhuisha")) && !p->isNude()){
                return QStringList(objectName());
            }
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        if (player->askForSkillInvoke(this, data) && room->askForDiscard(player,objectName(),1,1,true,true,QString(),true)) {
            return true;
        }
        return false;
    }

     virtual bool effect(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        CardUseStruct use = data.value<CardUseStruct>();

        ServerPlayer *target = use.to.at(0);
        if (player->getPhase()!=Player::NotActive)
            room->setPlayerFlag(player, target->objectName()+"zhuisha");
        else if (room->getCurrent())
            room->setPlayerFlag(room->getCurrent(),target->objectName()+"zhuisha");
        room->broadcastSkillInvoke(objectName(),player);
        if (!target->isNude()){
            int id=room->askForCardChosen(player,target,"he",objectName());
            target->addToPile("zhui", id);
        }
        return false;
    }
};

class Songzang : public TriggerSkill
{
public:
    Songzang() : TriggerSkill("songzang")
    {

        events << DamageCaused;
        frequency = NotFrequent;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const
    {
        if (triggerEvent == DamageCaused) {
            DamageStruct damage = data.value<DamageStruct>();
            if (TriggerSkill::triggerable(player) && damage.card && (damage.card->isKindOf("Slash")||damage.card->isKindOf("Duel")) && damage.to->getPile("zhui").length()>0)
                return QStringList(objectName());
        }

        return QStringList();
    }

    virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        if (triggerEvent == DamageCaused && player->askForSkillInvoke(this, data)) {
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        if (triggerEvent == DamageCaused){
            DamageStruct damage = data.value<DamageStruct>();
            room->broadcastSkillInvoke(objectName(), player);
            int n = damage.to->getPile("zhui").length();
            foreach(int id, damage.to->getPile("zhui")){
                room->throwCard(id, damage.to, player);
            }
            damage.damage=damage.damage+n;
            data.setValue(damage);
        }
        return false;
    }
};

class Xianshu : public TriggerSkill
{
public:
    Xianshu () : TriggerSkill("xianshu")
    {
        frequency = NotFrequent;
        events << EventPhaseStart << DrawNCards;
        relate_to_place = "deputy";
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const
   {
        if (triggerEvent == EventPhaseStart){
            if (TriggerSkill::triggerable(player) && player->getPhase()==Player::Start){
                return QStringList(objectName());
            }
            else if(player->hasFlag("xianshu_used") && player->getPhase()==Player::Finish){
                room->setPlayerFlag(player,"-xianshu_used");
            }
        }
        else if(triggerEvent == DrawNCards){
            if (player->hasFlag("xianshu_used")){
                room->setPlayerFlag(player,"-xianshu_used");
                data.setValue(data.toInt()-1);
            }
        }
        return QStringList();
   }

    virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        if (event==EventPhaseStart&&player->askForSkillInvoke(this, data)) {
            QList<ServerPlayer*> list;
            foreach(auto p, room->getAlivePlayers()){
                if (p->isMale()&&p->isWounded()){
                    list<<p;
                }
            }

            ServerPlayer *target=room->askForPlayerChosen(player,list,objectName(),QString(),true,true);
            if (target){
                player->tag["xianshu_target"] = QVariant::fromValue(target);
                return true;
            }
        }
        return false;
    }

    virtual bool effect(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        ServerPlayer *target = player->tag["xianshu_target"].value<ServerPlayer *>();
        if (target){
            room->broadcastSkillInvoke(objectName(),player);
            room->setPlayerFlag(player,"xianshu_used");
            if (room->askForChoice(player,objectName(),"xianshurecover+xianshudraw") == "xianshurecover"){
                RecoverStruct recover;
                recover.recover = 1;
                recover.who = player;
                room->recover(target, recover, true);
            }
            else{
                target->drawCards(target->getLostHp());
            }
        }
        return false;
    }
};

class Huanbing : public TriggerSkill
{
public:
    Huanbing () : TriggerSkill("huanbing")
    {
        frequency = NotFrequent;
        events << DamageCaused << CardFinished;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const
   {
        if (triggerEvent == DamageCaused){
            DamageStruct damage = data.value<DamageStruct>();
            if (TriggerSkill::triggerable(player) && damage.card && damage.card->isBlack()){
                return QStringList(objectName());
            }
        }
        else{
            CardUseStruct use= data.value<CardUseStruct>();
            if (use.card && use.card->isBlack() && use.card->isNDTrick() && TriggerSkill::triggerable(player) && !player->hasFlag("huanbing_used") && player->getPhase()!= Player::NotActive){
                return QStringList(objectName());
            }
        }
        return QStringList();
   }

    virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *skill_target, QVariant &data, ServerPlayer *player) const
    {
        if (player->askForSkillInvoke(this, data)){
            room->broadcastSkillInvoke(objectName(), player);
            return true;
        }
        return false;
    }
    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who) const
    {
        if (triggerEvent == DamageCaused){
            DamageStruct damage = data.value<DamageStruct>();
            if (!damage.to->isNude()){
                int id = room->askForCardChosen(player, damage.to, "he", objectName());
                room->throwCard(id, damage.to, player);
            }
        }
        else{
            ServerPlayer *target = room->askForPlayerChosen(player, room->getAlivePlayers(), objectName(), QString(), true, true);
            Card *c = Sanguosha->cloneCard("slash");
            if (target && !player->isProhibited(target, c)){
                room->setPlayerFlag(player, "huanbing_used");
                CardUseStruct use= data.value<CardUseStruct>();
                Card *card = Sanguosha->cloneCard("slash");
                card->addSubcards(use.card->getSubcards());
                card->setSkillName("huanbing");
                room->useCard(CardUseStruct(card, player, target), false);
            }
        }
        return false;
    }
};

class Guanli : public TriggerSkill
{
public:
    Guanli() : TriggerSkill("guanli")
    {
        frequency = Frequent;
        events <<  EventPhaseChanging;
    }

    virtual TriggerList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
    {
        TriggerList skill_list;
        PhaseChangeStruct change = data.value<PhaseChangeStruct>();
        if (!player->hasFlag("SE_Guanli_on") && player != NULL && player->isAlive() && change.to == Player::Start )
        {
            QList<ServerPlayer *> asagis = room->findPlayersBySkillName(objectName());

            foreach (ServerPlayer *asagi, asagis)
            {
                if (!asagi->isNude())
                {
                    skill_list.insert(asagi, QStringList(objectName()));
                }
            }
        }
        else if(change.to == Player::Finish && player->hasFlag("SE_Guanli_on")){
            player->setFlags("-SE_Guanli_on");
        }
        return skill_list;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who) const
    {
        if(ask_who->askForSkillInvoke(this, QVariant::fromValue(player)) && room->askForCard(ask_who,".|.|.|hand","@guanli:"+ player->objectName(),QVariant::fromValue(player), objectName())){
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who) const
    {
        PhaseChangeStruct change = data.value<PhaseChangeStruct>();
        room->broadcastSkillInvoke(objectName(), ask_who);
        player->setFlags("SE_Guanli_on");
        QString choice = room->askForChoice(ask_who, objectName(), "Gl_draw+Gl_play+Gl_discard");
        if(choice == "Gl_draw"){
           change.to = Player::Draw;
           data.setValue(change);
           player->insertPhase(Player::Draw);
        }
        if(choice == "Gl_play"){
           change.to = Player::Play;
           data.setValue(change);
           player->insertPhase(Player::Play);
        }
        if(choice == "Gl_discard"){
           change.to = Player::Discard;
           data.setValue(change);
           player->insertPhase(Player::Discard);
        }
        return false;
    }
};

PoyiCard::PoyiCard()
{
    mute = true;
}

bool PoyiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    if (targets.length()>=2) return false;
    if (targets.isEmpty()) return to_select->getHp()>= Self->getHp() && !to_select->isFriendWith(Self);
    if (targets.length()==1) return targets.at(0)->inMyAttackRange(to_select) && to_select!=targets.at(0);
}

bool PoyiCard::targetsFeasible(const QList<const Player *> &targets, const Player *) const
{
    return targets.length() == 2;
}

void PoyiCard::onUse(Room *room, const CardUseStruct &card_use) const
{
    ServerPlayer *asagi = card_use.from;

    LogMessage log;
    log.from = asagi;
    log.to << card_use.to;
    log.type = "#UseCard";
    log.card_str = toString();
    room->sendLog(log);

    QVariant data = QVariant::fromValue(card_use);
    RoomThread *thread = room->getThread();

    thread->trigger(PreCardUsed, room, asagi , data);
    room->broadcastSkillInvoke("poyi", asagi );


    if (asagi ->ownSkill("poyi") && !asagi ->hasShownSkill("poyi"))
        asagi ->showGeneral(asagi ->inHeadSkills("poyi"));

    thread->trigger(CardUsed, room, asagi , data);
    thread->trigger(CardFinished, room, asagi , data);
}

void PoyiCard::use(Room *room, ServerPlayer *asagi , QList<ServerPlayer *> &targets) const
{
    ServerPlayer *target = targets.at(0);
    ServerPlayer *slashTarget = targets.at(1);
    int index = asagi->startCommand(objectName());
    ServerPlayer *dest = NULL;
    if (index == 0) {
        dest = room->askForPlayerChosen(asagi, room->getAlivePlayers(), "command_poyi", "@command-damage");
        room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, asagi->objectName(), dest->objectName());
    }
    if (!target->doCommand("poyi",index,asagi,dest)){
        Card *card = Sanguosha->cloneCard("slash");
        room->useCard(CardUseStruct(card, target, slashTarget), false);
    }
}

class Poyi : public ZeroCardViewAsSkill
{
public:
    Poyi() : ZeroCardViewAsSkill("poyi")
    {
    }

    bool isEnabledAtPlay(const Player *player) const
    {
        return !player->hasUsed("PoyiCard");
    }

    const Card *viewAs() const
    {
        PoyiCard *vs = new PoyiCard();
        vs->setSkillName(objectName());
        vs->setShowSkill(objectName());
        return vs;
    }
};

class Trial : public TriggerSkill
{
public:
    Trial() : TriggerSkill("trial")
    {
        frequency = NotFrequent;
        events << TargetConfirmed << EventPhaseStart;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const
   {
        if (triggerEvent==TargetConfirmed){
            CardUseStruct use = data.value<CardUseStruct>();
            if (TriggerSkill::triggerable(player) && use.card != NULL && (use.card->isKindOf("Slash")||(use.card->isBlack()&& use.card->isNDTrick())) && use.to.length()==1 && !use.to.contains(player)) {
                if (player->getMark("trial_used")==0)
                    return QStringList(objectName());
            }
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        if (player->askForSkillInvoke(this, data)) {
            room->broadcastSkillInvoke(objectName(),player);
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        room->setPlayerMark(player, "trial_used", 1);
        CardUseStruct use = data.value<CardUseStruct>();
        use.to.clear();
        use.to.append(player);
        data.setValue(use);
        QList<int> trial = room->getNCards(3, false);

        LogMessage log;
        log.type = "$ViewDrawPile";
        log.from = player;
        log.card_str = IntList2StringList(trial).join("+");
        room->doNotify(player, QSanProtocol::S_COMMAND_LOG_SKILL, log.toVariant());
        room->askForGuanxing(player, trial);
        player->drawCards(1);
        return false;
    }
};

class TrialTrigger : public TriggerSkill
{
public:
    TrialTrigger() : TriggerSkill("#trial")
    {
        frequency = NotFrequent;
        events << EventPhaseStart;
        global=true;
    }

    virtual QStringList triggerable(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &) const
    {
        if (TriggerSkill::triggerable(player) && player->getMark("trial_used")>0 && player->getPhase() == Player::RoundStart) {
             room->setPlayerMark(player,"trial_used",0);
        }
        return QStringList();
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        if (triggerEvent == EventPhaseStart){
            if (player->isAlive() && player->getPhase() == Player::RoundStart){
            }
        }
        return false;
    }
};

PoshiCard::PoshiCard()
{
    target_fixed = true;
    will_throw = false;
}
void PoshiCard::use(Room *room, ServerPlayer *aika, QList<ServerPlayer *> &) const
{
    room->loseHp(aika);
}

class Poshi : public ZeroCardViewAsSkill
{
public:
    Poshi() : ZeroCardViewAsSkill("poshi")
    {
    }

    bool isEnabledAtPlay(const Player *player) const
    {
        return !player->hasUsed("PoshiCard");
    }

    const Card *viewAs() const
    {
        PoshiCard *vs = new PoshiCard();
        vs->setSkillName(objectName());
        vs->setShowSkill(objectName());
        return vs;
    }
};

class PoshiDistance : public DistanceSkill
{
public:
    PoshiDistance() : DistanceSkill("#poshi-dist")
    {
    }

    virtual int getCorrect(const Player *from, const Player *) const
    {
        if (from->hasUsed("PoshiCard"))
            return -1000;
        else
            return 0;
    }
};

class PoshiTargetMod : public TargetModSkill
{
public:
    PoshiTargetMod () : TargetModSkill("#poshitargetmod")
    {
    }

    virtual int getResidueNum(const Player *from, const Card *card) const
    {
        if (!Sanguosha->matchExpPattern(pattern, from, card))
            return 0;

        if (from->hasUsed("PoshiCard"))
            return 1000;
        else
            return 0;
    }
};


class PoshiTrigger : public TriggerSkill
{
public:
    PoshiTrigger() : TriggerSkill("#poshi")
    {
        frequency = NotFrequent;
        events << CardFinished << TargetConfirmed;
        global=true;
    }

    virtual QStringList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &) const
    {
        if (event == CardFinished) {
             CardUseStruct use = data.value<CardUseStruct>();
             foreach(auto p, use.to){
                 if (p->getMark("poshi_null")>0){
                     room->setPlayerMark(p, "Armor_Nullified", p->getMark("Armor_Nullified")-1);
                     room->setPlayerMark(p, "poshi_null", 0);
                 }
             }
        }
        if (event == TargetConfirmed) {
             CardUseStruct use = data.value<CardUseStruct>();
             if (use.from && use.from->hasUsed("PoshiCard") && player==use.from){
                 foreach(auto p, use.to){
                     if (p != use.from){
                         room->setPlayerMark(p, "Armor_Nullified", p->getMark("Armor_Nullified")+1);
                         room->setPlayerMark(p, "poshi_null", 1);
                     }
                 }
             }
        }
        return QStringList();
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        return false;
    }
};

class Liansuo : public TriggerSkill
{
public:
    Liansuo() : TriggerSkill("liansuo")
    {
        frequency = NotFrequent;
        events << CardsMoveOneTime << EventPhaseStart;
    }

     virtual QStringList triggerable(TriggerEvent event, Room *room, ServerPlayer *aika, QVariant &data, ServerPlayer* &) const
    {
        if (TriggerSkill::triggerable(aika) && event == CardsMoveOneTime){
            CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
            if (!move.from_places.contains(Player::PlaceHand) && !move.from_places.contains(Player::PlaceEquip)){
                return QStringList();
            }
            if (!move.from || aika->getPhase() != Player::NotActive || move.from ==aika){
                return QStringList();
            }
            if (move.reason.m_reason != CardMoveReason::S_REASON_DISCARD && move.reason.m_reason != CardMoveReason::S_REASON_DISMANTLE && move.reason.m_reason != CardMoveReason::S_REASON_THROW && move.reason.m_reason != CardMoveReason::S_REASON_RULEDISCARD){
                return QStringList();
            }
            if (move.card_ids.length() == 0){
                return QStringList();
            }
            if (aika->getMark("aikadraw")>=4){
                return QStringList();
            }
            return QStringList(objectName());
        }
        else if (event == EventPhaseStart && aika->getPhase()==Player::RoundStart){
            room->setPlayerMark(aika, "aikadraw", 0);
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *aika, QVariant &data, ServerPlayer *ask_who) const
    {
        if (aika->askForSkillInvoke(this, data)){
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *aika, QVariant &data, ServerPlayer *) const
    {
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        QList<int> list = move.card_ids;
        foreach(int id, list){
            Card *c = Sanguosha->getCard(id);
            if (id == list.at(0) || (id != list.at(0) && aika->getMark("aikadraw")<4 && aika->askForSkillInvoke(this, data))){
                room->setPlayerMark(aika, "aikadraw", aika->getMark("aikadraw")+1);
                room->broadcastSkillInvoke(objectName(), aika);
                QList<int> card_ids = room->getNCards(1);
                Card *c1 = Sanguosha->getCard(card_ids.at(0));
                LogMessage log;
                log.type = "$ViewDrawPile";
                log.from = aika;
                log.card_str = IntList2StringList(card_ids).join("+");
                room->doNotify(aika, QSanProtocol::S_COMMAND_LOG_SKILL, log.toVariant());
                foreach (int id, card_ids) {
                    room->moveCardTo(Sanguosha->getCard(id), aika, Player::PlaceTable, CardMoveReason(CardMoveReason::S_REASON_TURNOVER, aika->objectName(), "liansuo", ""), false);
                }
                if (c->getTypeId()==c1->getTypeId()){
                    room->obtainCard(aika, c1);
                }
                else{
                    room->throwCard(c1, aika);
                    ServerPlayer *target = room->askForPlayerChosen(aika, room->getAlivePlayers(), objectName(), QString(), true);
                    if (target){
                        room->setPlayerProperty(target, "chained", QVariant(true));
                    }
                }
            }
        }

        return false;
    }
};

class Yinguo : public TriggerSkill
{
public:
    Yinguo() : TriggerSkill("yinguo")
    {
        frequency = Limited;
        events << Death;
    }

     virtual QStringList triggerable(TriggerEvent event, Room *room, ServerPlayer *aika, QVariant &data, ServerPlayer* &) const
    {
        if (aika->hasSkill(this) && event == Death){
            DeathStruct death = data.value<DeathStruct>();
            if (death.who != aika || !aika->hasSkill(this) ){
                return QStringList();
            }
            if (room->getTag(aika->objectName()+"yinguo_used").toBool()){
                return QStringList();
            }
            return QStringList(objectName());
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *aika, QVariant &data, ServerPlayer *ask_who) const
    {
        if (aika->askForSkillInvoke(this, data)){
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *aika, QVariant &data, ServerPlayer *) const
    {
        DeathStruct death = data.value<DeathStruct>();
        room->setTag(aika->objectName()+"yinguo_used", QVariant(true));
        QList<ServerPlayer*> list;
        foreach(auto p, room->getAlivePlayers()){
            if(p->hasShownOneGeneral() || p->hasShownAllGenerals()){
                list << p;
            }
        }
        if (list.length()==0){
            return false;
        }
        else{
            ServerPlayer *p1 = room->askForPlayerChosen(aika,list,objectName(),"@yinguo",true,true);
            ServerPlayer *p2 = NULL;
            if (p1) {
              list.removeOne(p1);
            }
            if (list.length()>0){
              p2 = room->askForPlayerChosen(aika,list,objectName(),"@yinguo2",true,true);
            }
            QList<ServerPlayer*> targets;
            if (p1){
                targets << p1;
            }
            if (p2){
                targets << p2;
            }
            QList<int> ids = aika->handCards();
            if (p1 && p2){
                while (!aika->isKongcheng()){
                    room->askForYiji(aika, ids, objectName(), false, false, false, -1, targets);
                }
            }
            else if (!targets.isEmpty()){
                DummyCard *card = aika->wholeHandCards();
                room->obtainCard(targets.at(0),card,false);
            }
            QStringList avaliable_generals;
            foreach(QString s,aika->getSelected()){
                const General *g = Sanguosha->getGeneral(s);
                if (!room->getUsedGeneral().contains(s) && g->getKingdom() != "careerist")
                    avaliable_generals << s;
            }
            if (avaliable_generals.isEmpty()){
                return false;
            }
            if (p1 && room->askForChoice(aika,"yinguo","yinguo1_transform+cancel",data)=="yinguo1_transform") {
                QString to_change = room->askForGeneral(aika, avaliable_generals, QString(), true, "yinguo", p1->getKingdom());
                p1->showGeneral(false);
                room->transformDeputyGeneralTo(p1, to_change);
                avaliable_generals.removeOne(to_change);
            }
            if (avaliable_generals.isEmpty()){
                return false;
            }
            if (p2 && room->askForChoice(aika,"yinguo","yinguo2_transform+cancel",data)=="yinguo2_transform") {
                QString to_change = room->askForGeneral(aika, avaliable_generals, QString(), true, "yinguo", p2->getKingdom());
                p2->showGeneral(false);
                room->transformDeputyGeneralTo(p2, to_change);
                avaliable_generals.removeOne(to_change);
            }
        }
        return false;
    }
};

class Weituo : public TriggerSkill
{
public:
    Weituo() : TriggerSkill("weituo")
    {
        frequency = NotFrequent;
        events << Damaged << EventPhaseStart;
    }

    virtual TriggerList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
    {
        TriggerList skill_list;
        if (event==Damaged){
            DamageStruct damage = data.value<DamageStruct>();
            foreach(auto p, room->getAlivePlayers()){
                if (p->getMark("@liufangzhe")>0){
                    return skill_list;
                }
            }
            if (!damage.from || damage.from->isDead()){
                return skill_list;
            }
            QList<ServerPlayer *> ais = room->findPlayersBySkillName(objectName());
            foreach (ServerPlayer *ai, ais) {
                if (ai != damage.to && damage.to->isAlive() && damage.from != ai){
                     skill_list.insert(ai, QStringList(objectName()));
                }
            }
        }
        return skill_list;
    }

    virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ai) const
   {
        if (ai->askForSkillInvoke(this, data)) {
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ai) const
    {
        DamageStruct damage = data.value<DamageStruct>();
        QList<int> card_ids = room->getNCards(1);
        foreach (int id, card_ids) {
            room->moveCardTo(Sanguosha->getCard(id), ai, Player::PlaceTable, CardMoveReason(CardMoveReason::S_REASON_TURNOVER, ai->objectName(), objectName(), ""), false);
        }
        int id = card_ids.at(0);
        if (!damage.from->hasShownAllGenerals()){
            damage.from->askForGeneralShow(true, true);
        }
        if (damage.from->isFriendWith(ai)){
            room->obtainCard(ai ,id);
        }
        else{
            foreach(auto p, room->getAlivePlayers()){
               room->setPlayerMark(p, "@liufangzhe", 0);
               room->setPlayerMark(p, "@weituozhe", 0);
            }
            damage.from->gainMark("@liufangzhe");
            damage.to->gainMark("@weituozhe");
            room->obtainCard(damage.to ,id);
        }
        return false;
    }
};

class Liufang : public TriggerSkill
{
public:
    Liufang() : TriggerSkill("liufang")
    {
        frequency = NotFrequent;
        events << EventPhaseEnd << EventPhaseStart << EventPhaseChanging;
        global = true;
    }

   virtual QStringList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const
    {
        if (event==EventPhaseStart){
            int n = 0;
            foreach (ServerPlayer *p, room->getAlivePlayers()) {
                if (p->getMark("@weituozhe")>0){
                     n = p->getLostHp();
                }
            }
           if (player->getPhase()==Player::Start && player->getMark("@liufangzhe")>0){
               return QStringList(objectName());
           }
        }
        if (event==EventPhaseEnd){
            if (player->getPhase()==Player::Finish && player->getMark("@liufangzhe")>0 && player->hasFlag("liufang_clear")){
                foreach(auto p, room->getAlivePlayers()){
                   room->setPlayerMark(p, "@liufangzhe", 0);
                   room->setPlayerMark(p, "@weituozhe", 0);
                }
            }
        }
        if (event==EventPhaseChanging){
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (!player->isSkipped(Player::Draw)&& change.to==Player::Draw && player->hasFlag("liufang_skipdraw")) {
                 return QStringList(objectName());
            }
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ai) const
   {
        if (event==EventPhaseStart || event==EventPhaseChanging) {
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ai) const
    {
        if (event==EventPhaseStart) {
            int n = 0;
            foreach (ServerPlayer *p, room->getAlivePlayers()) {
                if (p->getMark("@weituozhe")>0){
                     n = p->getLostHp();
                }
            }
            room->setPlayerFlag(player, "liufang_clear");
            if (n>0){
              room->loseHp(player, n);
            }
            if (player->getHp() == 1){
                room->setPlayerFlag(player, "liufang_skipdraw");
            }
        }
        if (event==EventPhaseChanging){
            player->skip(Player::Draw);
        }
        return false;
    }
};

class Jiaji : public TriggerSkill
{
public:
    Jiaji () : TriggerSkill("jiaji")
    {
        events << EventPhaseStart << DrawNCards;
    }

   virtual QStringList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const
    {
        if (event==EventPhaseStart){
           if (TriggerSkill::triggerable(player) &&player->getPhase()==Player::Start && !player->isNude()){
               return QStringList(objectName());
           }
        }
        if (event==DrawNCards){
            if (TriggerSkill::triggerable(player) && player->getPile("gem").length()>0) {
                 return QStringList(objectName());
            }
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ai) const
   {
        if (event==EventPhaseStart && player->askForSkillInvoke(this, data)) {
            int id = room->askForCardChosen(player,player,"he",objectName());
            player->setProperty("jiaji", QVariant(id));
            return true;
        }
        if (event==DrawNCards){
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ai) const
    {
        if (event==EventPhaseStart) {
           int id = player->property("jiaji").toInt();
           player->addToPile("gem", id);
        }
        if (event==DrawNCards){
            int n = data.toInt();
            QStringList suits;
            foreach(int id, player->getPile("gem")){
                Card *card = Sanguosha->getCard(id);
                if (!suits.contains(card->getSuitString())){
                    suits << card->getSuitString();
                }
            }
            n = n+(suits.length()+suits.length()%2)/2;
            data.setValue(n);
        }
        return false;
    }
};

ModanCard::ModanCard()
{
}

bool ModanCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    return Self->inMyAttackRange(to_select);
}

void ModanCard::use(Room *room, ServerPlayer *player, QList<ServerPlayer *> &targets) const
{
    int id = this->getSubcards().at(0);
    Card *card = Sanguosha->getCard(id);
    if ((card->isKindOf("BasicCard") && !card->isKindOf("Jink")) || (card->isNDTrick() && !card->isKindOf("Nullification") && !card->isKindOf("HegNullification"))){
        CardUseStruct use = CardUseStruct(card, player, targets);
        room->useCard(use, false);
    }
    else if(card->isKindOf("EquipCard")){
        Slash *slash = new Slash(card->getSuit(),card->getNumber());
        slash->addSubcard(card);
        CardUseStruct use = CardUseStruct(slash, player, targets);
        room->useCard(use, false);
    }
    else{
        foreach(auto p, targets){
            p->drawCards(1);
        }
    }
}

class Modan : public OneCardViewAsSkill
{
public:
    Modan() : OneCardViewAsSkill("modan")
    {
        filter_pattern = ".|.|.|gem";
        expand_pile = "gem";
    }

    bool isEnabledAtPlay(const Player *player) const
    {
        return !player->hasUsed("ModanCard");
    }

    const Card *viewAs(const Card *card) const
    {
        ModanCard *vs = new ModanCard();
        vs->addSubcard(card);
        vs->setSkillName(objectName());
        vs->setShowSkill(objectName());
        return vs;
    }
};

//game
ChichengCard::ChichengCard()
{
    target_fixed = true;
    will_throw = false;
}
void ChichengCard::use(Room *room, ServerPlayer *akagi, QList<ServerPlayer *> &) const
{
    akagi->addToPile("akagi_lv", this);
    if (this->getSubcards().length()>1){
        RecoverStruct recover;
        recover.recover = 1;
        room->recover(akagi, recover, true);
    }
}

class Chicheng : public ViewAsSkill
{
public:

    Chicheng() : ViewAsSkill("chicheng")
    {
    }

    bool isEnabledAtPlay(const Player *player) const
    {
        return !player->hasUsed("ChichengCard");
    }

    bool viewFilter(const QList<const Card *> &selected, const Card *card) const
    {
        return true;
    }

    const Card *viewAs(const QList<const Card *> &cards) const
    {
        if (cards.isEmpty())
            return NULL;
        ChichengCard *zrc = new ChichengCard();
        zrc->addSubcards(cards);
        zrc->setSkillName(objectName());
        zrc->setShowSkill(objectName());
        return zrc;
    }
};

class Zhikong : public TriggerSkill
{
public:
    Zhikong() : TriggerSkill("zhikong")
    {
        frequency = Frequent;
        events <<  EventPhaseStart << DamageCaused << EventPhaseChanging;
    }

    virtual void record(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if(event == DamageCaused){
            DamageStruct damage = data.value<DamageStruct>();
            if (damage.to && damage.to->isAlive() && damage.from->hasFlag("se_zhikong_on") && damage.card && damage.card->isKindOf("Slash")){
                if (rand()%100 < 62){
                    damage.damage= damage.damage+1;
                    data.setValue(damage);
                }
            }
        }

        else if(event == EventPhaseChanging){
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.to==Player::Finish && player->hasFlag("se_zhikong_on")){
                player->setFlags("-se_zhikong_on");
                foreach(auto p, room->getOtherPlayers(player)){
                    if(p->getMark("has_been_Armor_Nullified")==0){
                        room->setPlayerMark(p, "Armor_Nullified", 0);
                    }
                    else{
                        room->setPlayerMark(p, "has_been_Armor_Nullified", 0);
                    }
                }
            }
        }
    }

    virtual TriggerList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
    {
        TriggerList skill_list;
        if (event == EventPhaseStart && player != NULL && player->isAlive() && player->getPhase() == Player::Start )
        {
            QList<ServerPlayer *> akagis = room->findPlayersBySkillName(objectName());

            foreach (ServerPlayer *akagi, akagis)
            {
                if (akagi->getPile("akagi_lv").length()>0 && akagi->getHp() > 1)
                {
                    skill_list.insert(akagi, QStringList(objectName()));
                }
            }
        }

        return skill_list;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who) const
    {
        if(ask_who->askForSkillInvoke(this, QVariant::fromValue(player))){
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who) const
    {
        ServerPlayer *akagi = ask_who;
        room->fillAG(akagi->getPile("akagi_lv"), akagi);
        int id = room->askForAG(akagi, akagi->getPile("akagi_lv"), false, objectName());
        room->clearAG(akagi);
        room->throwCard(id, akagi);
        room->broadcastSkillInvoke(objectName(), akagi);
        room->doLightbox("se_zhikong$", 800);
        if (player->isFriendWith(akagi)){
            akagi->drawCards(1);
        }
        player->setFlags("se_zhikong_on");
        foreach(auto p, room->getOtherPlayers(player)){
            if(p->getMark("Armor_Nullified")==0){
                room->setPlayerMark(p, "Armor_Nullified", 1);
            }
            else{
                room->setPlayerMark(p, "has_been_Armor_Nullified", 1);
            }
        }

        return false;
    }
};

FanqianCard::FanqianCard()
{
    target_fixed = true;
}
void FanqianCard::use(Room *room, ServerPlayer *asashio, QList<ServerPlayer *> &) const
{
    QList<ServerPlayer *> all = room->getAlivePlayers();
    QStringList string;
    foreach(ServerPlayer *p, all){
        string.append(QString::number(p->getSeat()));
    }
    QString targetName = room->askForChoice(asashio, "fanqian", string.join("+"));
    ServerPlayer *target;
    foreach(ServerPlayer *p, all){
        if (QString::number(p->getSeat()) == targetName){
            target = p;
            break;
        }
    }
    if (target){
        Card *card = Sanguosha->getCard(this->subcards.at(0));
        card->setSkillName("fanqian");
        room->setTag("fanqian_target", QVariant().fromValue(target));
        CardUseStruct use = CardUseStruct(card, asashio, target);
        room->useCard(use);
    }
}


class FanqianVS : public OneCardViewAsSkill
{
public:
    FanqianVS() : OneCardViewAsSkill("fanqian")
    {
    }

    bool viewFilter(const Card *to_select) const
    {
        return !to_select->isKindOf("Collateral") && !to_select->isKindOf("Jink") && !to_select->isKindOf("Nullification") && !to_select->isKindOf("DelayedTrick") &&!to_select->isEquipped();
    }

    const Card *viewAs(const Card *originalCard) const
    {
        FanqianCard *fqc = new FanqianCard();
        fqc->addSubcard(originalCard);
        fqc->setShowSkill("fanqian");
        fqc->setSkillName("fanqian");
        return fqc;
    }
};

class Fanqian : public TriggerSkill
{
public:
    Fanqian() : TriggerSkill("fanqian")
    {
        view_as_skill = new FanqianVS;
        events << PreCardUsed;
    }

    bool trigger(TriggerEvent, Room *room, ServerPlayer *, QVariant &data) const
    {
        CardUseStruct use = data.value<CardUseStruct>();
        if ((use.card->isKindOf("AOE") || use.card->isKindOf("GlobalEffect")) && use.card->getSkillName() == "fanqian"){
            use.to.clear();
            use.to.append(room->getTag("fanqian_target").value<ServerPlayer *>());
            data = QVariant::fromValue(use);
        }

        return false;
    }

    virtual QStringList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const{
        return QStringList();
    }
};

class Buyu : public TriggerSkill
{
public:
    Buyu() : TriggerSkill("buyu")
    {
        events << EventPhaseStart << EventPhaseEnd << TargetConfirmed;
    }

    virtual QStringList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const
   {
        if (event == EventPhaseStart){
            if (player->getPhase() == Player::Play && TriggerSkill::triggerable(player)){
                return QStringList(objectName());
            }
        }
        else if(event == EventPhaseEnd){
                    foreach(ServerPlayer *p, room->getAlivePlayers()){
                        if (p->getMark("@Buyu") > 0)
                            p->loseAllMarks("@Buyu");
                    }
        }
        else if (event == TargetConfirmed){
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.from && use.from==player && use.from->hasShownSkill(objectName()) && use.card && !(use.card->isKindOf("Slash") && use.card->isBlack())){
                foreach(ServerPlayer *p, use.to){
                    if (p->getMark("@Buyu") > 0){
                        if (!use.from->hasFlag("Buyu_sdraw_played")){
                            room->broadcastSkillInvoke(objectName(), 1);
                            use.from->setFlags("Buyu_sdraw_played");
                        }

                        use.from->drawCards(1);
                        return QStringList();
                    }
                }
                if (!use.from->isNude() && use.from->hasFlag("buyu_used")){
                    if (!use.from->hasFlag("Buyu_sdis_played")){
                        room->broadcastSkillInvoke(objectName(), 2);
                        use.from->setFlags("Buyu_sdis_played");
                    }
                    room->askForDiscard(use.from, objectName(), 1, 1, false, true);
                }
            }
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who) const
    {
        if(player->askForSkillInvoke(this, data)){
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent event, Room *room, ServerPlayer *asashio, QVariant &data, ServerPlayer *ask_who) const
    {
        if (event == EventPhaseStart){
                room->broadcastSkillInvoke(objectName(), 1);
                ServerPlayer *target = room->askForPlayerChosen(asashio, room->getAlivePlayers(), objectName());
                if (target){
                    target->gainMark("@Buyu");
                    asashio->setFlags("buyu_used");
                }

        }
        return false;
    }
};

HuanshiCard::HuanshiCard()
{
}

bool HuanshiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    return targets.isEmpty() && !to_select->isKongcheng() && to_select != Self;
}

void HuanshiCard::extraCost(Room *, const CardUseStruct &card_use) const
{
    ServerPlayer *reisen = card_use.from;
    PindianStruct *pd = reisen->pindianSelect(card_use.to.first(), "huanshi");
    reisen->tag["huanshi_pd"] = QVariant::fromValue(pd);
}

void HuanshiCard::onEffect(const CardEffectStruct &effect) const
{
    Room *room = effect.to->getRoom();
    PindianStruct *pd = effect.from->tag["huanshi_pd"].value<PindianStruct *>();
    effect.from->tag.remove("huanshi_pd");
    if (pd != NULL) {
        bool success = effect.from->pindian(pd);
        if (success){
            CardUseStruct use;
            use.from = effect.from;
            use.to.append(effect.to);
            Card *card1 = Sanguosha->getCard(pd->from_card->getEffectiveId());
            Card *card2 = Sanguosha->getCard(pd->to_card->getEffectiveId());
           // Card *card1 = Sanguosha->cloneCard(pd->from_card->objectName(),Card::SuitToBeDecided,-1);
            //Card *card2 = Sanguosha->cloneCard(pd->to_card->objectName(),Card::SuitToBeDecided,-1);
           // card1->addSubcard(pd->from_card->getEffectiveId());
           // card2->addSubcard(pd->to_card->getEffectiveId());
            use.card = card1;
            room->useCard(use, false);
            CardUseStruct use2;
            use2.from = effect.from;
            use2.to.append(effect.to);
            use2.card = card2;
            room->useCard(use2, false);
        }
        else{
           // Card *card1 = Sanguosha->cloneCard(pd->from_card->objectName(),Card::SuitToBeDecided,-1);
            //Card *card2 = Sanguosha->cloneCard(pd->to_card->objectName(),Card::SuitToBeDecided,-1);
            //card1->addSubcard(pd->from_card->getEffectiveId());
            //card2->addSubcard(pd->to_card->getEffectiveId());
            Card *card1 = Sanguosha->getCard(pd->from_card->getEffectiveId());
            Card *card2 = Sanguosha->getCard(pd->to_card->getEffectiveId());
            CardUseStruct use;
            use.from = effect.to;
            use.to.append(effect.from);
            use.card = card1;
            room->useCard(use, false);
            CardUseStruct use2;
            use2.from = effect.to;
            use2.to.append(effect.from);
            use2.card = card2;
            room->useCard(use2, false);
        }
        pd = NULL;
    } else
        Q_ASSERT(false);
}

class Huanshi : public ZeroCardViewAsSkill
{
public:
    Huanshi() : ZeroCardViewAsSkill("huanshi")
    {
    }

    virtual bool isEnabledAtPlay(const Player *player) const
    {
        return !player->isKongcheng() && !player->hasUsed("HuanshiCard");
    }

    virtual const Card *viewAs() const
    {
        HuanshiCard *card = new HuanshiCard;
        card->setShowSkill(objectName());
        return card;
    }

    virtual int getEffectIndex(const ServerPlayer *, const Card *) const
    {
        return 2;
    }
};

class Kuangzao : public TriggerSkill
{
public:
    Kuangzao() : TriggerSkill("kuangzao")
    {
        events << DamageCaused;
        frequency = NotFrequent;
    }

    virtual QStringList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &) const
    {
        DamageStruct damage = data.value<DamageStruct>();
        if (TriggerSkill::triggerable(player) && damage.card && (damage.card->isKindOf("BasicCard")||damage.card->isKindOf("TrickCard") ) && player->getPhase() != Player::NotActive && !player->hasFlag(damage.to->objectName()+ "kuangzao")){
            return QStringList(objectName());
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

    virtual bool effect(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        DamageStruct damage = data.value<DamageStruct>();
        room->broadcastSkillInvoke(objectName());
        room->setPlayerFlag(player, damage.to->objectName()+"kuangzao");
        room->loseHp(damage.to);
        return false;
    }
};

class Wushi : public TriggerSkill
{
public:
    Wushi() : TriggerSkill("wushi")
    {
        events << TargetConfirmed;
        frequency = NotFrequent;
    }

    virtual QStringList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &) const
    {
        CardUseStruct use = data.value<CardUseStruct>();
        ServerPlayer *current = room->getCurrent();
        if (current){
            if ((use.card->isKindOf("BasicCard") && current->hasFlag(player->objectName()+"wushi_basic"))||(use.card->isKindOf("TrickCard") && current->hasFlag(player->objectName()+"wushi_trick"))){
                return QStringList();
            }
        }
        if (TriggerSkill::triggerable(player) && use.card && (use.card->isKindOf("BasicCard")||use.card->isKindOf("TrickCard") ) && use.from && player == use.from && use.to.length()>0){
            return QStringList(objectName());
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

    virtual bool effect(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        CardUseStruct use = data.value<CardUseStruct>();
        room->broadcastSkillInvoke(objectName(), player);
        ServerPlayer *current = room->getCurrent();
        if (current){
            if (use.card->isKindOf("BasicCard")){
                room->setPlayerFlag(current, player->objectName()+"wushi_basic");
            }
            else if(use.card->isKindOf("TrickCard")){
                room->setPlayerFlag(current, player->objectName()+"wushi_trick");
            }
        }
        QList<ServerPlayer *> players = room->askForPlayersChosen(player, use.to, objectName(), 0, use.to.length(), QString(), true);
        foreach(auto p, players){
            room->loseHp(p);
            room->cancelTarget(use, p);
        }
        data = QVariant::fromValue(use);
        bool has = false;
        foreach(auto p, use.to){
            if (p->isFriendWith(player)){
                has = true;
            }
        }

        if (has){
            player->drawCards(1);
        }

        return false;
    }
};

class Wuyou : public DistanceSkill
{
public:
   Wuyou(): DistanceSkill("wuyou")
   {
   }

   virtual int getCorrect(const Player *, const Player *to) const
   {
        if (to->hasSkill(objectName()) && to->hasShownSkill(this))
            return 1;
        else
            return 0;
   }

};

class Wuyoumax : public MaxCardsSkill
{
public:
    Wuyoumax() : MaxCardsSkill("wuyoumax")
    {
    }

    virtual int getExtra(const ServerPlayer *target, MaxCardsType::MaxCardsCount) const
    {
        bool has = false;
        Room *room=target->getRoom();
        foreach(ServerPlayer *p, room->getAlivePlayers()){
            if (p->isFriendWith(target) && p->objectName()!= target->objectName()){
                has = true;
            }
        }
        if (has == false && target->hasShownSkill("wuyou")){
            return 2;
        }
        else{
            return 0;
        }
    }
};

DaokegiveCard::DaokegiveCard()
{
    will_throw = false;
    m_skillName = "daokegive";
}

bool DaokegiveCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    return to_select->hasSkill("daoke") && targets.length() == 0;
}

void DaokegiveCard::use(Room *room, ServerPlayer *player, QList<ServerPlayer *> &targets) const
{
    ServerPlayer *target = targets.at(0);
    if (!target)
        return;
    room->obtainCard(target, this, false);
    const Card *card = room->askForCard(target, ".|.|.|.", "@daoke", QVariant::fromValue(player));
    if (card){
        int m = card->getNumber();
        if (player->getMark("@kaihua")>0){
            m = m+1;
        }
        int n = (m+m%2)/2;
        QString choice = room->askForChoice(target, "daoke", "daoke_drawpile+daoke_discardpile");
        QList<int> list;
        if (choice == "daoke_drawpile") {
            for(int i = 0; i < n; i++){
                if (room->getDrawPile().length()>i){
                  list << room->getDrawPile().at(i);
                }

            }
        }
        else{
            for(int i = 0; i < n; i++){
                if (room->getDiscardPile().length()>i){
                   list << room->getDiscardPile().at(i);
                }
            }
        }
        if (list.length()>0){
           room->fillAG(list);
           int want=room->askForAG(player,list,false, "daoke");
           room->obtainCard(player, want);
           if(player != target && target->hasShownSkill("kaihua")){
               player->gainMark("@kaihua");
               if (player->getMark("@kaihua") == 5){
                   room->setPlayerProperty(player, "maxhp", QVariant(player->getMaxHp()+1));
                   room->setPlayerProperty(target, "maxhp", QVariant(target->getMaxHp()+1));
               }
           }
           room->clearAG();
           room->clearAG(player);
        }
    }
}

class Daokegive : public ViewAsSkill
{
public:
    Daokegive() : ViewAsSkill("daokegive")
    {
        attached_lord_skill = true;
    }

    bool isEnabledAtPlay(const Player *player) const
    {
        return !player->hasUsed("DaokegiveCard");
    }

    bool viewFilter(const QList<const Card *> &selected, const Card *) const
    {
        return selected.length() < 1;
    }

    const Card *viewAs(const QList<const Card *> &cards) const
    {
        if (cards.isEmpty())
            return NULL;
        DaokegiveCard *zrc = new DaokegiveCard();
        zrc->addSubcards(cards);
        zrc->setSkillName(objectName());
        zrc->setShowSkill(objectName());
        return zrc;
    }
};

class Daoke : public TriggerSkill
{
public:
    Daoke()
        : TriggerSkill("daoke")
    {
        events << GeneralShown << EventAcquireSkill << GeneralTransformed;
        frequency = NotFrequent;
    }

    virtual QStringList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &) const{
        if ((event==GeneralShown||GeneralTransformed)&&TriggerSkill::triggerable(player)&&player->inHeadSkills(this)==data.toBool()){
            foreach(auto p, room->getAlivePlayers()){
                room->attachSkillToPlayer(p, "daokegive");
            }
        }
        else if (event==EventAcquireSkill){
            if (data.toString().split(":").first()==objectName()){
                foreach(auto p, room->getOtherPlayers(player)){
                    room->attachSkillToPlayer(p, "daokegive");
                }
            }
        }
        return QStringList();
    }
};

class Kaihua : public TriggerSkill
{
public:
    Kaihua()
        : TriggerSkill("kaihua")
    {
        events << HpRecover;
        frequency = Compulsory;
    }

    virtual TriggerList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
    {
        TriggerList skill_list;
        RecoverStruct re = data.value<RecoverStruct>();
        if (re.who!=NULL && player->hasSkill("kaihua") && player != re.who){
           skill_list.insert(player, QStringList(objectName()));
        }
        else if( re.who!=NULL && re.who ->hasSkill("kaihua") && player != re.who){
            skill_list.insert(re.who, QStringList(objectName()));
        }
        return skill_list;
    }

    virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *noora) const
    {
        if (noora->hasShownSkill("kaihua")||noora->askForSkillInvoke(this, data)) {
            return true;
        }
        return false;
    }

     virtual bool effect(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *noora) const
    {
        RecoverStruct re = data.value<RecoverStruct>();
        if (player == noora){
            re.who->gainMark("@kaihua");
            if (re.who->getMark("@kaihua") == 5){
                room->setPlayerProperty(re.who, "maxhp", QVariant(re.who->getMaxHp()+1));
                room->setPlayerProperty(player, "maxhp", QVariant(player->getMaxHp()+1));
            }
        }
        else{
            player->gainMark("@kaihua");
            if (player->getMark("@kaihua") == 5){
                room->setPlayerProperty(player, "maxhp", QVariant(player->getMaxHp()+1));
                room->setPlayerProperty(re.who, "maxhp", QVariant(re.who->getMaxHp()+1));
            }
        }
        return false;
    }
};

//lord

class Jinji : public TriggerSkill
{
public:
    Jinji() : TriggerSkill("jinji$")
    {
        events << Damage << EventPhaseStart << GeneralShown;
    }

     /*virtual QStringList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const
    {
        if (event==GeneralShown && TriggerSkill::triggerable(player)&&data.toBool()==player->inHeadSkills(this)){
            room->broadcastSkillInvoke(objectName(), 1, player);
            room->acquireSkill(player, "jiyuunotsubasa");
        }
        if (event==EventPhaseStart && TriggerSkill::triggerable(player) && player->getPhase()==Player::Start){
            return QStringList(objectName());
        }
        return QStringList();
    }*/

   virtual TriggerList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
   {
        TriggerList skill_list;
        if (event==GeneralShown && TriggerSkill::triggerable(player)&&data.toBool()==player->inHeadSkills(this)){
            room->broadcastSkillInvoke(objectName(), 1, player);
            room->acquireSkill(player, "jiyuunotsubasa");
        }
        /*if (event==EventPhaseStart && TriggerSkill::triggerable(player) && player->getPhase()==Player::Start){
            skill_list.insert(player, QStringList(objectName()));
        }*/
        if (event == Damage){
            QList<ServerPlayer *> erens = room->findPlayersBySkillName(objectName());
            DamageStruct damage = data.value<DamageStruct>();
            foreach (ServerPlayer *eren, erens) {
                if (damage.to->isFriendWith(eren) && damage.damage>0 && damage.from->isAlive())
                    skill_list.insert(eren, QStringList(objectName()));
            }
        }
        return skill_list;
   }

    virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *skill_target, QVariant &data, ServerPlayer *player) const
    {
        if (event==EventPhaseStart && player->askForSkillInvoke(this, data)) {
            QList<ServerPlayer*> list;
            foreach(auto p , room->getAlivePlayers()){
                if (!player->isFriendWith(p) && p->getMark("jinji_used")==0){
                    list << p;
                }
            }
            ServerPlayer *target=room->askForPlayerChosen(player,list,objectName(),QString(),true, true);
            if(target){
             player->tag["quzhu_target"] = QVariant::fromValue(target);
             room->setPlayerMark(target, "jinji_used", 1);
             return true;
            }
        }
        if (event == Damage){
            if (player->askForSkillInvoke("jinji", data)){
                return true;
            }
        }
        return false;
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who) const
    {
        if (triggerEvent == EventPhaseStart){
            ServerPlayer *target = ask_who->tag["quzhu_target"].value<ServerPlayer *>();
            if (target){
                target->gainMark("@quzhu");
                room->setFixedDistance(ask_who, target, 1);
                if (ask_who->getMark("jinji_first")==0) {
                    room->setPlayerMark(ask_who, "jinji_first", 1);
                    room->doLightbox("Erenattack1$", 1000);
                }
            }
        }
        if (triggerEvent == Damage){
            DamageStruct damage = data.value<DamageStruct>();
            damage.from->gainMark("@quzhu", damage.damage);
            room->setFixedDistance(ask_who, damage.from, 1);
            if (ask_who->getMark("jinji_first")==0) {
                room->setPlayerMark(ask_who, "jinji_first", 1);
                room->doLightbox("Erenattack1$", 2000);
            }
        }
        return false;
    }
};

int shisoPower(Card *card)
{
    int n = card->getNumber();
    if (n<=10){
        return 1;
    }
    else if(n==11){
        return 2;
    }
    else if(n==12){
        return 3;
    }
    else{
        return 4;
    }
}

class Shizu : public TriggerSkill
{
public:
    Shizu() : TriggerSkill("shizu")
    {
        events << TargetChosen << Damage << GeneralShown;
    }

     virtual QStringList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const
    {
        if (event==TargetChosen){
            CardUseStruct use = data.value<CardUseStruct>();
            if (TriggerSkill::triggerable(player) && player->hasSkill("zahyo")&& use.card != NULL && (use.card->isKindOf("Slash")||use.card->isKindOf("Duel"))) {
                QStringList targets;
                foreach (ServerPlayer *to, use.to) {
                    int n = 0;
                    foreach(int id, player->getPile("roads")){
                        if(Sanguosha->getCard(id)->getNumber()>10){
                          n = n+1;
                        }
                    }

                    if (n >= 1)
                        targets << to->objectName();
                }
                if (!targets.isEmpty())
                    return QStringList(objectName() + "->" + targets.join("+"));
            }
        }
        if (event==GeneralShown && TriggerSkill::triggerable(player)&&data.toBool()==player->inHeadSkills(this)){
            foreach(auto p, room->getAlivePlayers()){
                room->attachSkillToPlayer(p, "shiso");
            }
            room->attachSkillToPlayer(player, "zahyo");
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who) const
    {
        if (event==TargetChosen){
            if (ask_who->askForSkillInvoke("zahyo", QVariant::fromValue(player))) {
                return true;
            }
        }
        return false;
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who) const
    {
        if (triggerEvent==TargetChosen){
           CardUseStruct use = data.value<CardUseStruct>();
           QList<int> list0;
           foreach(int id, ask_who->getPile("roads")){
               if(Sanguosha->getCard(id)->getNumber()>10){
                 list0 << id;
               }
           }
           room->fillAG(list0, ask_who);
           int id = room->askForAG(ask_who,list0, false, objectName());
           room->clearAG(ask_who);
           room->throwCard(id, ask_who, ask_who);
           int index = ask_who->startCommand(objectName());
           ServerPlayer *dest = NULL;
           if (index == 0) {
               dest = room->askForPlayerChosen(ask_who, room->getAlivePlayers(), "command_shizu", "@command-damage");
               room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, ask_who->objectName(), dest->objectName());
           }
           QStringList list;
           foreach(auto p, room->getAlivePlayers()){
               QStringList big_kingdoms = p->getBigKingdoms(objectName(), MaxCardsType::Max);
               bool invoke = !big_kingdoms.isEmpty();
               if (invoke) {
                   if (big_kingdoms.length() == 1 && big_kingdoms.first().startsWith("sgs")) // for JadeSeal
                       invoke = big_kingdoms.contains(p->objectName());
                   else if (p->getRole() == "careerist")
                       invoke = false;
                   else
                       invoke = big_kingdoms.contains(p->getKingdom());
               }
               if (invoke || p->getKingdom()==ask_who->getKingdom()){
                   if (!list.contains(p->getKingdom())){
                       list<< p->getKingdom();
                   }
               }
           }
           QString choice = room->askForChoice(ask_who, objectName(), list.join("+"));
           room->setEmotion(player, "skills/zahyo");
          if (choice == ask_who->getKingdom()){
              QList<ServerPlayer *> alls = room->getOtherPlayers(ask_who);
                  room->sortByActionOrder(alls);
                  foreach(ServerPlayer *anjiang, alls) {
                      if (anjiang->hasShownOneGeneral()) continue;

                      QString kingdom = ask_who->getKingdom();
                      ServerPlayer *lord = NULL;

                      int num = 0;
                      foreach (ServerPlayer *p, room->getAllPlayers(true)) {
                          if (p->getKingdom() != kingdom) continue;
                          QStringList list = room->getTag(p->objectName()).toStringList();
                          if (!list.isEmpty()) {
                              const General *general = Sanguosha->getGeneral(list.first());
                              if (general->isLord())
                                  lord = p;
                          }
                          if (p->hasShownOneGeneral() && p->getRole() != "careerist")
                              num++;
                      }

                      bool full = (ask_who->getRole() == "careerist" || ((lord == NULL || !lord->hasShownGeneral1()) && num >= room->getPlayers().length() / 2));

                      if (anjiang->getKingdom() == kingdom && !full) {
                          anjiang->askForGeneralShow(false, true);
                      }
                      else{
                          room->askForChoice(anjiang,objectName(),"cannot_showgeneral+cancel",data);
                      }
                  }
          }
          foreach (ServerPlayer *p, room->getOtherPlayers(ask_who)) {
              if(!p->hasShownOneGeneral())
                  continue;
              if (p->getKingdom()==choice&&!room->askForUseSlashTo(p, player,"player:"+player->objectName(),false)){
                  if (p->getKingdom()==ask_who->getKingdom()){
                      if (!p->doCommand(objectName(),index,ask_who,dest)){
                          p->drawCards(1);
                      }
                  }
                  else{
                     p->doCommandForcely(objectName(),index,ask_who,dest);
                  }
              }
              if (p->getKingdom()==ask_who->getKingdom() && p->getKingdom()==choice && room->askForChoice(p,"transform","transform+cancel",data)=="transform"){
                  p->showGeneral(false);
                  room->transformDeputyGeneral(p);
              }
          }
        }
        return false;
    }
};

class Zahyo : public TriggerSkill
{
public:
    Zahyo() : TriggerSkill("zahyo")
    {
        events << TargetChosen;
    }

     virtual QStringList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const
    {
        return QStringList();
    }

    virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who) const
    {
        return false;
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who) const
    {
        return false;
    }
};


class Jiyuunotsubasa : public TriggerSkill
{
public:
    Jiyuunotsubasa() : TriggerSkill("jiyuunotsubasa")
    {
        events << Damaged << Damage << EventAcquireSkill << DamageCaused << PreCardUsed << TurnStart;
        attached_lord_skill = true;
    }

     virtual TriggerList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
    {
        TriggerList skill_list;
        /*if (event== EventAcquireSkill && TriggerSkill::triggerable(player)&& data.toString().split(":").first()==objectName()){
            foreach(auto p, room->getAlivePlayers()){
                if (p->hasShownOneGeneral()&&!p->isFriendWith(player)){
                    p->gainMark("@quzhu");
                    room->setFixedDistance(player, p, 1);
                }
            }
        }
        if (event == Damage){
            QList<ServerPlayer *> erens = room->findPlayersBySkillName(objectName());
            DamageStruct damage = data.value<DamageStruct>();
            foreach (ServerPlayer *eren, erens) {
                if (damage.to->isFriendWith(eren) && damage.damage>0)
                    skill_list.insert(eren, QStringList(objectName()));
            }
        }*/
        if (event==TurnStart){
            ServerPlayer *eren = room->getCurrent();
            if (!eren || !TriggerSkill::triggerable(eren))
                return skill_list;
            //if (!eren->faceUp() || eren->isChained() || eren->getJudgingArea().length()>0 || )
            skill_list.insert(eren, QStringList(objectName()));
        }

        if (event == DamageCaused){
            QList<ServerPlayer *> erens = room->findPlayersBySkillName(objectName());
            DamageStruct damage = data.value<DamageStruct>();
            if (!damage.card || (!damage.card->isKindOf("Slash")&&(!damage.card->isKindOf("Duel")))){
                return skill_list;
            }
            foreach (ServerPlayer *eren, erens) {
                if (damage.to->getMark("@quzhu")>0 && damage.from == eren && damage.to->getMark("@quzhu") >= 2)
                    skill_list.insert(eren, QStringList(objectName()));
            }
        }
        if (event == PreCardUsed){
            QList<ServerPlayer *> erens = room->findPlayersBySkillName(objectName());
            CardUseStruct use = data.value<CardUseStruct>();
            if (!use.card || (!use.card->isKindOf("Slash")&&(!use.card->isKindOf("Duel")))){
                return skill_list;
            }
            int n = 0;
            foreach(auto p, room->getAlivePlayers()){
                n=n+p->getMark("@quzhu");
            }
            int m = 0;
            foreach(auto p, use.to){
                m=m+p->getMark("@quzhu");
            }
            foreach (ServerPlayer *eren, erens) {
                if ( m>0 && use.from == eren && n > 2)
                    skill_list.insert(eren, QStringList(objectName()));
            }
        }
        return skill_list;
    }

    virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *skill_target, QVariant &data, ServerPlayer *player) const
    {
        if (event == DamageCaused){
            DamageStruct damage = data.value<DamageStruct>();
            if (player->askForSkillInvoke("quzhudamage", QVariant::fromValue(damage.to))){
                return true;
            }
        }
        if (event == PreCardUsed){
            if (player->askForSkillInvoke("quzhuaddtarget", data)){
                return true;
            }
        }
        if (event == TurnStart){
            if (player->hasShownSkill(this)||player->askForSkillInvoke("erenfate",data)){
                return true;
            }
        }

        return false;
    }

     virtual bool effect(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who) const
    {
        if (event == DamageCaused){
            DamageStruct damage = data.value<DamageStruct>();
            damage.damage =  damage.damage+1;
            data.setValue(damage);
        }
        if (event == PreCardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            use.to.clear();
            QList<ServerPlayer *> list = room->getAlivePlayers();
            room->sortByActionOrder(list);
            foreach(ServerPlayer *p, list){
                if (p->getMark("@quzhu") > 0){
                    use.to.append(p);
                }
            }
            data = QVariant::fromValue(use);
        }
        if(event == TurnStart){
            if ((!ask_who->faceUp() || ask_who->isChained())&&ask_who->askForSkillInvoke("erenfate_normalize")){
                if (!ask_who->faceUp()){
                  ask_who->turnOver();
                }
                room->setPlayerProperty(ask_who, "chained", QVariant(false));
                QStringList choices;
                choices<<"eren_damage";
                if (ask_who->getPile("roads").length()>0)
                    choices << "discard_road";
                QString choice = room->askForChoice(ask_who, "erenfate", choices.join("+"));
                if (choice == "eren_damage"){
                    DamageStruct da;
                    da.from= NULL;
                    da.to=ask_who;
                    room->damage(da);
                }
                else{
                    room->fillAG(ask_who->getPile("roads"), ask_who);
                    int id = room->askForAG(ask_who, ask_who->getPile("roads"), false, objectName());
                    room->clearAG(ask_who);
                    room->throwCard(id, ask_who, ask_who);
                }
            }
            if (ask_who->isAlive() &&ask_who->getJudgingArea().length()>0 && ask_who->askForSkillInvoke("erenfate_discardjudge", data)){
                int id0 = room->askForCardChosen(ask_who, ask_who, "j", objectName());
                room->throwCard(id0, ask_who, ask_who);
                QStringList choices;
                choices<<"eren_damage";
                if (ask_who->getPile("roads").length()>0)
                    choices << "discard_road";
                QString choice = room->askForChoice(ask_who, "erenfate", choices.join("+"));
                if (choice == "eren_damage"){
                    DamageStruct da;
                    da.from= NULL;
                    da.to=ask_who;
                    room->damage(da);
                }
                else{
                    room->fillAG(ask_who->getPile("roads"), ask_who);
                    int id = room->askForAG(ask_who, ask_who->getPile("roads"), false, objectName());
                    room->clearAG(ask_who);
                    room->throwCard(id, ask_who, ask_who);
                }
            }
            if (ask_who->isAlive()&&ask_who->askForSkillInvoke("erenfate_seefuture", data)){
                int n =0;
                foreach(auto p, room->getAlivePlayers()){
                    if (p->getMark("@quzhu")>0){
                        n=n+1;
                    }
                    if (p->isFriendWith(ask_who)){
                        n=n+1;
                    }
                }
                QList<int> shenzhi = room->getNCards(n, false);

                LogMessage log;
                log.type = "$ViewDrawPile";
                log.from = ask_who;
                log.card_str = IntList2StringList(shenzhi).join("+");
                room->doNotify(ask_who, QSanProtocol::S_COMMAND_LOG_SKILL, log.toVariant());
                room->askForGuanxing(ask_who, shenzhi);
                QStringList choices;
                choices<<"eren_damage";
                if (ask_who->getPile("roads").length()>0)
                    choices << "discard_road";
                QString choice = room->askForChoice(ask_who, "erenfate", choices.join("+"));
                if (choice == "eren_damage"){
                    DamageStruct da;
                    da.from= NULL;
                    da.to=ask_who;
                    room->damage(da);
                }
                else{
                    room->fillAG(ask_who->getPile("roads"), ask_who);
                    int id = room->askForAG(ask_who, ask_who->getPile("roads"), false, objectName());
                    room->clearAG(ask_who);
                    room->throwCard(id, ask_who, ask_who);
                }
            }
        }
        return false;
    }
};

ShisoCard::ShisoCard()
{
    will_throw = false;
}

bool ShisoCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *) const
{
    if (targets.isEmpty()){
        return to_select->hasShownSkill("shizu") && to_select->getPile("roads").length()<13;
    }
}

void ShisoCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const
{
    if (!source->hasShownOneGeneral()){
        source->askForGeneralShow(false, false);
    }
    ServerPlayer *target = targets.at(0);
    target->addToPile("roads", this);
}

class Shiso : public ViewAsSkill
{
public:

    Shiso() : ViewAsSkill("shiso")
    {
        attached_lord_skill = true;
    }

    bool isEnabledAtPlay(const Player *player) const
    {
        const Player *eren = player->getLord();
        if (!eren || !eren->hasShownSkill("shizu") || !player->willBeFriendWith(eren))
            return false;
        return !player->hasUsed("ShisoCard") && player->canShowGeneral();
    }

    bool viewFilter(const QList<const Card *> &selected, const Card *card) const
    {
        return selected.length() < 1 && (card->getSuitString()=="heart" || card->getSuitString()=="spade");
    }

    const Card *viewAs(const QList<const Card *> &cards) const
    {
        if (cards.isEmpty())
            return NULL;
        ShisoCard *zrc = new ShisoCard();
        zrc->addSubcards(cards);
        zrc->setSkillName(objectName());
        zrc->setShowSkill(objectName());
        return zrc;
    }
};

//double kingdoms
class Suipian : public TriggerSkill
{
public:
    Suipian() : TriggerSkill("suipian")
    {
        frequency = Frequent;
        events << EventPhaseStart << Damaged;
    }

    virtual TriggerList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
   {
        TriggerList skill_list;
        if (event == EventPhaseStart){
            if (TriggerSkill::triggerable(player) && player->getPhase()==Player::Start && player->getPile("Fragments").isEmpty()){
               skill_list.insert(player, QStringList(objectName()));
            }
        }
        else if(event == Damaged){
            DamageStruct damage = data.value<DamageStruct>();
            QList< ServerPlayer*> rikas = room->findPlayersBySkillName(objectName());
            foreach(auto rika, rikas){
                if (damage.to->isAlive() && damage.to ->isFriendWith(rika) && rika->getPile("Fragments").length()>0){
                    skill_list.insert(rika, QStringList(objectName()));
                }
            }
        }
        return skill_list;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *rika) const
    {
        if(rika->askForSkillInvoke(this, data)){
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *rika) const
    {
        if (event == EventPhaseStart){
           room->broadcastSkillInvoke(objectName(), rand()%2 +1 ,rika);
           int n = 1;
           foreach(auto p, room->getAlivePlayers()){
               if (p ->isFriendWith(player)){
                   n = n+1;
               }
           }

           QList<int> list = room->getNCards(n, false);
           rika->addToPile("Fragments", list);
           if (n>2){
               room->askForDiscard(rika, objectName(), 1, 1 ,false, true, QString());
           }
        }
        else{
           room->broadcastSkillInvoke(objectName(), rand()%4 +3 ,rika);
           room->fillAG(rika->getPile("Fragments"), rika);
           int id = room->askForAG(rika, rika->getPile("Fragments"), false, objectName());
           room->clearAG(rika);
           room->obtainCard(player, id);
           if (room->askForChoice(player,"transform","transform+cancel",data)=="transform"){
               player->showGeneral(false);
               room->transformDeputyGeneral(player);
           }
        }
        return false;
    }
};

class LunhuiRecord : public TriggerSkill
{
public:
    LunhuiRecord() : TriggerSkill("#lunhuirecord")
    {
        frequency = NotFrequent;
        events << CardFinished << EventPhaseEnd << CardUsed;
        global=true;
    }

    virtual QStringList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &) const
    {
        if (event == CardFinished){
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.card!=NULL && (use.card->isKindOf("BasicCard")||use.card->isNDTrick()) && player->getPhase() == Player::Play) {
                 room->setPlayerProperty(player, "lunhui_card", QVariant(use.card->objectName()));
            }
        }
        else if(event == EventPhaseEnd){
            if (player->getPhase() == Player::Play){
                 room->setPlayerProperty(player, "lunhui_card", QVariant(""));
            }
        }
        else{
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.card != NULL && use.card->getSkillName()=="lunhui"){
                room->setPlayerFlag(player, "lunhui_used");
            }
        }
        return QStringList();
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        return false;
    }
};

class Lunhui : public OneCardViewAsSkill
{
public:
    Lunhui() : OneCardViewAsSkill("lunhui"){
        filter_pattern = ".|.|.|Fragments";
        expand_pile = "Fragments";
    }

    const Card *viewAs(const Card *originalCard) const
    {
        QString pattern=Self->property("lunhui_card").toString();
        if (pattern=="")
            return NULL;
        Card *card=Sanguosha->cloneCard(pattern,originalCard->getSuit(),originalCard->getNumber());
        card->addSubcard(originalCard);
        card->setSkillName(objectName());
        card->setShowSkill(objectName());
        return card;
    }

    bool isEnabledAtPlay(const Player *player) const
    {
        return !player->hasFlag("lunhui_used") && player->getKingdom()=="magic";
    }

};

PoxiaoCard::PoxiaoCard()
{
}

bool PoxiaoCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    if (targets.isEmpty()){
        return true;
    }
    return false;
}

void PoxiaoCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const
{
    ServerPlayer *target = targets.at(0);
    QString choice = room->askForChoice(source,"poxiao","Mipa_Basic+Mipa_NotBasic");
    if (choice == "Mipa_Basic") {
        target->gainMark("@mipa_basic");
        room->setPlayerCardLimitation(target, "use,response", "BasicCard+^Slash", false);
    }
    else{
        target->gainMark("@mipa_notbasic");
        room->setPlayerCardLimitation(target, "use,response", "^BasicCard", false);
    }
}

class Poxiao : public OneCardViewAsSkill
{
public:
    Poxiao() : OneCardViewAsSkill("poxiao"){
        filter_pattern = ".|.|.|Fragments";
        expand_pile = "Fragments";
    }

    bool isEnabledAtPlay(const Player *player) const
    {
        return !player->hasUsed("PoxiaoCard") && player->getKingdom()=="real";
    }

    const Card *viewAs(const Card *originalCard) const
    {
        PoxiaoCard *vs = new PoxiaoCard();
        vs->addSubcard(originalCard);
        vs->setSkillName(objectName());
        vs->setShowSkill(objectName());
        return vs;
    }
};

class PoxiaoTrigger : public TriggerSkill
{
public:
    PoxiaoTrigger() : TriggerSkill("#poxiao")
    {
        frequency = NotFrequent;
        events << EventPhaseEnd;
        global=true;
    }

    virtual QStringList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &) const
    {
        if (event == EventPhaseEnd) {
             if (player->getPhase()==Player::Finish){
                 if (player->getMark("@mipa_basic") > 0) {
                     player->loseMark("@mipa_basic");
                     room->removePlayerCardLimitation(player, "use,response", "BasicCard+^Slash");
                  }
                 if (player->getMark("@mipa_notbasic") > 0) {
                     player->loseMark("@mipa_notbasic");
                     room->removePlayerCardLimitation(player, "use,response", "^BasicCard");
                 }
             }
        }
        return QStringList();
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        return false;
    }
};

class Yandan : public TriggerSkill
{
public:
    Yandan() : TriggerSkill("yandan")
    {
        frequency = NotFrequent;
        events << CardsMoveOneTime << Death;
    }

     virtual QStringList triggerable(TriggerEvent event, Room *room, ServerPlayer *makoto, QVariant &data, ServerPlayer* &) const
    {
        if (TriggerSkill::triggerable(makoto) && event == CardsMoveOneTime){
            CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
            if (!move.from_places.contains(Player::PlaceHand) && !move.from_places.contains(Player::PlaceEquip)){
                return QStringList();
            }
            if (!move.from || move.from->getPhase() != Player::NotActive){
                return QStringList();
            }
            if (move.reason.m_reason != CardMoveReason::S_REASON_DISCARD && move.reason.m_reason != CardMoveReason::S_REASON_DISMANTLE && move.reason.m_reason != CardMoveReason::S_REASON_THROW && move.reason.m_reason != CardMoveReason::S_REASON_RULEDISCARD){
                return QStringList();
            }
            if (move.card_ids.length() == 0){
                return QStringList();
            }
            if (makoto->getPile("yandan").length() > makoto->getMaxHp()){
                return QStringList();
            }
            return QStringList(objectName());
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *makoto, QVariant &data, ServerPlayer *ask_who) const
    {
        if (makoto->askForSkillInvoke(this, data)){
            room->broadcastSkillInvoke(objectName(), makoto);
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *makoto, QVariant &data, ServerPlayer *) const
    {
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        QList<int> list = move.card_ids;
        room->fillAG(list, makoto);
        int id = room->askForAG(makoto, list, true, objectName());
        room->clearAG(makoto);
        if (id != -1){
            makoto->addToPile("yandan", id);
        }
        return false;
    }
};

class YandanMaxCards : public MaxCardsSkill
{
public:
    YandanMaxCards() : MaxCardsSkill("#yandan")
    {
    }

    virtual int getExtra(const ServerPlayer *target, MaxCardsType::MaxCardsCount) const
    {
        if (target->hasSkill("yandan")){
            int i = target->getPile("yandan").length() > 0 ? 1 : 0;
            return  i + target->getMark("yandan_death");
        }
        else
            return 0;
    }
};

class Lunpo : public TriggerSkill
{
public:
    Lunpo() : TriggerSkill("lunpo")
    {
        frequency = NotFrequent;
        events << EventPhaseStart << EventPhaseChanging << Death << CardUsed;
        relate_to_place = "head";
    }

    virtual TriggerList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
   {
        TriggerList skill_list;
        if (event == EventPhaseStart){
            if (player->getPhase() == Player::Play && player->hasSkill(objectName())){
                int minHp = 100;
                foreach(ServerPlayer *p, room->getAlivePlayers()){
                    if (p->getHp() < minHp){
                        minHp = p->getHp();
                    }
                }
                if (player->getPile("yandan").length() < minHp){
                    return skill_list;
                }

                skill_list.insert(player, QStringList(objectName()));
            }
        }
        else if(event == CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.card->isKindOf("EquipCard")|| use.card->isKindOf("DelayedTrick") ||use.to.isEmpty()){
                return skill_list;
            }
            QList< ServerPlayer*> makotos = room->findPlayersBySkillName(objectName());
            foreach(auto makoto, makotos){
                foreach(int id, makoto->getPile("yandan")){
                    if (use.card->getSuit()==Sanguosha->getCard(id)->getSuit()){
                      skill_list.insert(makoto, QStringList(objectName()));
                      break;
                    }
                }
            }
        }
        return skill_list;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *makoto) const
    {
        if(makoto->askForSkillInvoke(this, data)){
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *makoto) const
    {
        if (event == CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            QList<int> list;
            foreach(int id, makoto->getPile("yandan")){
                if (Sanguosha->getCard(id)->getSuit() == use.card->getSuit()){
                    list.append(id);
                }
            }
            room->fillAG(list, makoto);
            int id = room->askForAG(makoto, list, true, objectName());
            room->clearAG(makoto);
            if (id != -1){
                room->throwCard(id, makoto, makoto);
                room->broadcastSkillInvoke(objectName(), makoto, 2);
                room->doLightbox("lunpo$", 300);
                return true;
            }
        }
        if (event == EventPhaseStart){
            int minHp = 100;
            foreach(ServerPlayer *p, room->getAlivePlayers()){
                if (p->getHp() < minHp){
                    minHp = p->getHp();
                }
            }

            QList<int> list = player->getPile("yandan");
            for (int i = 0; i < minHp; i++){
                room->fillAG(list, player);
                int id = room->askForAG(player, list, false, objectName());
                room->clearAG(player);
                if (id != -1){
                    list.removeOne(id);
                    room->throwCard(id, player, player);
                }
            }

            room->broadcastSkillInvoke(objectName(), 1);
            room->doLightbox("lunpo$", 500);
            foreach(ServerPlayer *p, room->getOtherPlayers(player)){
                QStringList InvalidSkill = p->property("invalid_skill_has").toString().split("+");
                foreach(const Skill *skill, p->getSkillList()){
                    if (skill->getFrequency()!= Skill::Compulsory && !InvalidSkill.contains(skill->objectName()+":lunpo")){
                      InvalidSkill<<skill->objectName()+":lunpo";
                    }
                }

                p->setProperty("invalid_skill_has",QVariant(InvalidSkill.join("+")));

            }
        }
        return false;
    }
};

class LunpoRecord : public TriggerSkill
{
public:
    LunpoRecord() : TriggerSkill("#lunpo-record")
    {
        events << EventPhaseChanging << Death;
    }

    virtual void record(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
    {
        if (event==EventPhaseChanging){
            QList<ServerPlayer *> players = room->getAllPlayers();
            foreach(ServerPlayer *p, players) {
                QStringList InvalidSkill = p->property("invalid_skill_has").toString().split("+");
                foreach(const Skill *skill, p->getSkillList()){
                    if (InvalidSkill.contains(skill->objectName()+":lunpo")){
                     InvalidSkill.removeOne(skill->objectName()+":lunpo");
                    }
                }

                p->setProperty("invalid_skill_has",QVariant(InvalidSkill.join("+")));
            }
            JsonArray args;
            args << QSanProtocol::S_GAME_EVENT_UPDATE_SKILL;
            room->doBroadcastNotify(QSanProtocol::S_COMMAND_LOG_EVENT, args);
        }
        else if(event == Death) {
            DeathStruct death = data.value<DeathStruct>();
            if (death.who->hasSkill("lunpo")){
                QList<ServerPlayer *> players = room->getAllPlayers();
                foreach(ServerPlayer *p, players) {
                    QStringList InvalidSkill = p->property("invalid_skill_has").toString().split("+");
                    foreach(const Skill *skill, p->getSkillList()){
                        if (InvalidSkill.contains(skill->objectName()+":lunpo")){
                         InvalidSkill.removeOne(skill->objectName()+":lunpo");
                        }
                    }

                    p->setProperty("invalid_skill_has",QVariant(InvalidSkill.join("+")));
                }
                JsonArray args;
                args << QSanProtocol::S_GAME_EVENT_UPDATE_SKILL;
                room->doBroadcastNotify(QSanProtocol::S_COMMAND_LOG_EVENT, args);
            }
        }
    }

    virtual QStringList triggerable(TriggerEvent, Room *, ServerPlayer *, QVariant &, ServerPlayer* &) const
    {
        return QStringList();
    }

};

class Zizheng : public ViewAsSkill
{
public:
    Zizheng() : ViewAsSkill("zizheng")
    {
        relate_to_place = "deputy";
        expand_pile = "yandan";
        response_pattern = "nullification";
    }

    bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const
    {
        if (Self->getHandcards().contains(to_select)||Self->getEquips().contains(to_select))
            return false;
        return selected.length()<2;
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const
    {
        if (cards.length()<2)
            return NULL;
        Card *ncard = new HegNullification(Card::SuitToBeDecided, -1);
        ncard->addSubcards(cards);
        ncard->setSkillName(objectName());
        ncard->setShowSkill(objectName());
        return ncard;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const
    {
        return pattern == "nullification" && player->hasSkill("zizheng");
    }

    virtual bool isEnabledAtPlay(const Player *player) const
    {
        return false;
    }

    virtual bool isEnabledAtNullification(const ServerPlayer *player) const
    {
        QList <int> list = player->getPile("yandan");
        if (list.length()>1){
            return true;
        }
        return false;
    }
};

class ZizhengTrigger : public TriggerSkill
{
public:
    ZizhengTrigger() : TriggerSkill("#zizheng")
    {
        frequency = NotFrequent;
        events << CardFinished;
        global=true;
    }

    virtual QStringList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &) const
    {
        if (event == CardFinished) {
             CardUseStruct use = data.value<CardUseStruct>();
             if (use.card!= NULL && use.card->getSubcards().length()>=2 && use.card->getSkillName()=="zizheng"){
                 Card *c1 = Sanguosha->getCard(use.card->getSubcards().at(0));
                 Card *c2 = Sanguosha->getCard(use.card->getSubcards().at(1));
                 if (c1->getSuit()==c2->getSuit() || c1->getNumber()==c2->getNumber()){
                    if ( room->askForChoice(player,"zizheng","zizheng_transform+cancel",data)=="zizheng_transform"){
                        /*QStringList avaliable_generals;
                        foreach(QString s,player->getSelected()){
                            const General *g = Sanguosha->getGeneral(s);
                            if (!room->getUsedGeneral().contains(s) && g->getKingdom() != "careerist")
                                avaliable_generals << s;
                        }
                        if (avaliable_generals.isEmpty()){
                            return QStringList();
                        }
                        QString to_change = room->askForGeneral(player, avaliable_generals, QString(), true, "zizheng", player->getKingdom());*/
                        player->showGeneral(false);
                        room->transformDeputyGeneral(player);
                    }
                 }
             }
        }
        return QStringList();
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        return false;
    }
};

class Xuexi : public OneCardViewAsSkill
{
public:
    Xuexi() : OneCardViewAsSkill("xuexi"){
        guhuo_type="p";
    }

    virtual bool canPreshow() const
    {
        return true;
    }

    bool viewFilter(const Card *card) const
    {
        QString pattern=Self->tag[objectName()].toString();
        if (pattern=="")
            return NULL;
        Card *c = Sanguosha->cloneCard(pattern);
        return card->getTypeId() == c->getTypeId();
    }

    const Card *viewAs(const Card *originalCard) const
    {
        QString pattern=Self->tag[objectName()].toString();
        if (pattern=="")
            return NULL;
        /*Card *c = Sanguosha->cloneCard(pattern);
        if (originalCard->getTypeId() != c->getTypeId())
            return NULL;*/
        Card *card=Sanguosha->cloneCard(pattern,originalCard->getSuit(),originalCard->getNumber());
        card->addSubcard(originalCard);
        card->setSkillName(objectName());
        card->setShowSkill(objectName());
        return card;
    }

    bool isEnabledAtPlay(const Player *player) const
    {
        return !player->hasUsed("ViewAsSkill_xuexiCard");
    }

};

class XuexiTri : public TriggerSkill
{
public:
    XuexiTri() : TriggerSkill("#xuexitri")
    {
        frequency = NotFrequent;
        events << CardUsed;
        //view_as_skill = new Xuexivs;
    }

    virtual bool canPreshow() const
    {
        return true;
    }

    virtual TriggerList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
    {
        TriggerList skill_list;
        if (event == CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.from && (use.card->isKindOf("BasicCard")|| use.card->isNDTrick())){
                QList<ServerPlayer *> premieres = room->findPlayersBySkillName(objectName());
                foreach (ServerPlayer *premiere, premieres) {
                    if (premiere->isFriendWith(use.from) && premiere != use.from && !premiere->property("xuexi_card").toStringList().contains(use.card->objectName())&&(!room->getCurrent()||!room->getCurrent()->hasFlag(premiere->objectName()+"xuexi_used"))){
                       skill_list.insert(premiere, QStringList(objectName()));
                    }
                    else if(premiere == use.from && !premiere->property("xuexi_card").toStringList().contains(use.card->objectName())){
                        QStringList list = premiere->property("xuexi_card").toStringList();
                        list << use.card->objectName();
                        premiere->setProperty("xuexi_card", QVariant(list));
                    }
                }
            }
        }
        return skill_list;
    }

    virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *premiere) const
   {
        if (premiere->hasShownSkill("xuexi")||premiere->askForSkillInvoke("xuexi", data)) {
            if (player->askForSkillInvoke("xuexi", data)){
              if (room->getCurrent()){
                room->setPlayerFlag(room->getCurrent(), premiere->objectName()+"xuexi_used");
              }
              return true;
            }
        }
        return false;
    }

    virtual bool effect(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *premiere) const
    {
        premiere->showGeneral(premiere->inHeadSkills("xuexi"));
        CardUseStruct use = data.value<CardUseStruct>();
        room->obtainCard(premiere, use.card);
        QStringList list = premiere->property("xuexi_canusecard").toStringList();
        if (!list.contains(use.card->objectName())&& list.length()<5){
            list<<use.card->objectName();
            room->setPlayerMark(premiere, use.card->objectName()+"xuexi",1);
        }
        premiere->setProperty("xuexi_canusecard", QVariant(list));
        return false;
    }
};

class Qinggan : public TriggerSkill
{
public:
    Qinggan() : TriggerSkill("qinggan")
    {
        frequency = NotFrequent;
        events << CardUsed << DrawNCards;
    }

    virtual TriggerList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
    {
        TriggerList skill_list;
        if (event == CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.from && (use.card->getTypeId()!= Card::TypeSkill)){
                QList<ServerPlayer *> premieres = room->findPlayersBySkillName(objectName());
                foreach (ServerPlayer *premiere, premieres) {
                    if (premiere->isFriendWith(use.from) &&(!room->getCurrent()||!room->getCurrent()->hasFlag(premiere->objectName()+"qinggan_used"))){
                       skill_list.insert(premiere, QStringList(objectName()));
                    }
                }
            }
        }
        else{
            if (player->property("qinggan_card").toInt() != 0){
                skill_list.insert(player, QStringList(objectName()));
            }
        }
        return skill_list;
    }

    virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *premiere) const
   {
        if (event == CardUsed && premiere->askForSkillInvoke(this, data)) {
            if (room->getCurrent()){
              room->setPlayerFlag(room->getCurrent(), premiere->objectName()+"qinggan_used");
            }
            return true;
        }
        else if (event == DrawNCards){
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *premiere) const
    {
        if (event == CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            premiere->drawCards(1);
            if(player!=premiere){
                player->drawCards(1);
            }
            premiere->setProperty("qinggan_card", QVariant(use.card->getTypeId()));
        }
        else{
            int n = data.toInt();
            if (n>0){
                n = n-1;
            }
            data.setValue(n);
            int ini = premiere->property("qinggan_card").toInt();
            premiere->setProperty("qinggan_card", QVariant());

            //Card *card = Sanguosha->cloneCard(pattern);
            QList<int> list;
            foreach(int id, room->getDrawPile()){
                Card *c = Sanguosha->getCard(id);
                if (c->getTypeId()==ini)
                    list<< id;
            }
            if (list.length()>0){
                room->obtainCard(premiere, list.at(rand()%list.length()));
            }
        }
        return false;
    }
};

class WurenRange : public AttackRangeSkill
{
public:
   WurenRange() : AttackRangeSkill("#wurenrange")
    {
    }

    virtual int getFixed(const Player *target, bool include_weapon) const
    {
        if (target->hasSkill("leiqe") || target->hasSkill("huocheqie") || target->hasSkill("tongziqie") || target->hasSkill("paoqie") || target->hasSkill("huocheqie") || target->hasSkill("xiaowuwan")) {
            return 2;
        }
        return -1;
    }
};

class Wuren : public TriggerSkill
{
public:
    Wuren() : TriggerSkill("wuren")
    {
        frequency = NotFrequent;
        events << EventPhaseChanging << EventPhaseStart << Death;
    }

    virtual QStringList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &) const
    {
        if (event==EventPhaseChanging){
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (TriggerSkill::triggerable(player) &&!player->isSkipped(Player::Play)&& change.to==Player::Play) {
                 return QStringList(objectName());
            }
        }
        else if (event==EventPhaseStart){
            if (player->getPhase()==Player::Start && player->getMark("wuren_used")>0){
                room->setPlayerMark(player, "wuren_used", 0);
                QStringList list;
                list << "leiqie" << "huocheqie" << "tongziqie" << "paoqie" << "xiaowuwan";
                foreach(auto p, room->getAlivePlayers()){
                    foreach(auto s, p->getVisibleSkillList()){
                        if (list.contains(s->objectName()) && room->getTag(p->objectName()+"wuren"+s->objectName()).toBool()){
                            room->detachSkillFromPlayer(p, s->objectName(), false, false, p->inHeadSkills(s));
                            room->setTag(p->objectName()+"wuren"+s->objectName(), QVariant(false));
                        }
                    }
                }
            }
        }
        else{
            DeathStruct death = data.value<DeathStruct>();
            if (death.who->getMark("wuren_used")>0){
                QStringList list;
                list << "leiqie" << "huocheqie" << "tongziqie" << "paoqie" << "xiaowuwan";
                foreach(auto p, room->getAlivePlayers()){
                    foreach(auto s, p->getVisibleSkillList()){
                        if (list.contains(s->objectName()) && room->getTag(p->objectName()+"wuren"+s->objectName()).toBool()){
                            room->detachSkillFromPlayer(p, s->objectName(), false, false, p->inHeadSkills(s));
                            room->setTag(p->objectName()+"wuren"+s->objectName(), QVariant(false));
                        }
                    }
                }
            }
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        if (event==EventPhaseChanging&&player->askForSkillInvoke(this, data)) {
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        if (event==EventPhaseChanging){
            QStringList list;
            list << "leiqie" << "huocheqie" << "tongziqie" << "paoqie" << "xiaowuwan";
            QString s1 = room->askForChoice(player, objectName(), list.join("+"), data);
            list.removeOne(s1);
            room->acquireSkill(player, s1, true, player->inHeadSkills(this));
            room->setTag(player->objectName()+"wuren"+s1, QVariant(true));
            QString s2 = room->askForChoice(player, objectName(), list.join("+"), data);
            room->acquireSkill(player, s2, true, player->inHeadSkills(this));
            room->setTag(player->objectName()+"wuren"+s2, QVariant(true));
            list.removeOne(s2);
            room->setPlayerMark(player, "wuren_used", 1);
            QList<ServerPlayer*> plist;
            foreach(auto p, room->getAlivePlayers()){
                if (p->isFriendWith(player) && p!=player){
                    plist<<p;
                }
            }
            if (plist.length()>0){
                ServerPlayer *dest = room->askForPlayerChosen(player, plist, objectName(), QString(), true, true);
                if (dest){
                    QString s3 = room->askForChoice(player, objectName(), list.join("+"), data);
                    room->acquireSkill(dest, s3, true, true);
                    room->setTag(dest->objectName()+"wuren"+s3, QVariant(true));
                }
            }
        }
        return false;
    }
};

class Leiqie : public FilterSkill
{
public:
    Leiqie() : FilterSkill("leiqie")
    {
    }

    virtual bool viewFilter(const Card *to_select, ServerPlayer *player) const
    {
        Room *room = player->getRoom();
        int id = to_select->getEffectiveId();
        if (player->hasShownSkill(objectName()))
            return to_select->objectName() == "slash"
                    && ( room->getCardPlace(id) == Player::PlaceHand);

        return false;
    }

    virtual const Card *viewAs(const Card *originalCard) const
    {
        int id = originalCard->getEffectiveId();
        Card::Suit suit = originalCard->getSuit();
        int point = originalCard->getNumber();
        Card *slash = Sanguosha->cloneCard("thunder_slash", suit, point);
        WrappedCard *new_card = Sanguosha->getWrappedCard(id);
        new_card->takeOver(slash);
        new_card->setModified(true);
        return new_card;
    }

    virtual int getEffectIndex(const ServerPlayer *, const Card *) const
    {
        return -2;
    }
};

class Huocheqie : public FilterSkill
{
public:
    Huocheqie() : FilterSkill("huocheqie")
    {
    }

    virtual bool viewFilter(const Card *to_select, ServerPlayer *player) const
    {
        Room *room = player->getRoom();
        int id = to_select->getEffectiveId();
        if (player->hasShownSkill(objectName()))
            return to_select->objectName() == "slash"
                    && ( room->getCardPlace(id) == Player::PlaceHand);

        return false;
    }

    virtual const Card *viewAs(const Card *originalCard) const
    {
        int id = originalCard->getEffectiveId();
        Card::Suit suit = originalCard->getSuit();
        int point = originalCard->getNumber();
        Card *slash = Sanguosha->cloneCard("fire_slash", suit, point);
        WrappedCard *new_card = Sanguosha->getWrappedCard(id);
        new_card->takeOver(slash);
        new_card->setModified(true);
        return new_card;
    }

    virtual int getEffectIndex(const ServerPlayer *, const Card *) const
    {
        return -2;
    }
};

class HuocheqieTargetMod : public TargetModSkill
{
public:
    HuocheqieTargetMod() : TargetModSkill("#huocheqie-target")
    {
        pattern = "Slash";
    }

    virtual int getExtraTargetNum(const Player *from, const Card *card) const
    {
        if (from->hasShownSkill("huocheqie"))
            return 1;
        else
            return 0;
    }

};

class Tongziqie : public TriggerSkill
{
public:
    Tongziqie() : TriggerSkill("tongziqie")
    {
        frequency = NotFrequent;
        events << DamageCaused;
    }

    virtual QStringList triggerable(TriggerEvent event, Room *, ServerPlayer *player, QVariant &data, ServerPlayer * &) const
    {
        if (event==DamageCaused){
            DamageStruct damage = data.value<DamageStruct>();
            if (TriggerSkill::triggerable(player) && damage.card && damage.card->isKindOf("Slash")) {
                 return QStringList(objectName());
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

    virtual bool effect(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        DamageStruct damage = data.value<DamageStruct>();
        room->broadcastSkillInvoke(objectName(), player);
        room->loseHp(player);
        damage.damage = damage.damage+1;
        data.setValue(damage);
        return false;
    }
};

class Paoqie : public TriggerSkill
{
public:
    Paoqie() : TriggerSkill("paoqie")
    {
        frequency = NotFrequent;
        events << Damage;
    }

    virtual QStringList triggerable(TriggerEvent event, Room *, ServerPlayer *player, QVariant &data, ServerPlayer * &) const
    {
        if (event==Damage){
            DamageStruct damage = data.value<DamageStruct>();
            if (TriggerSkill::triggerable(player) && damage.card && damage.card->isKindOf("Slash") && damage.to->isAlive()) {
                 return QStringList(objectName());
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

    virtual bool effect(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        DamageStruct damage = data.value<DamageStruct>();
        room->broadcastSkillInvoke(objectName(), player);
        if (!damage.to->isKongcheng()){
            int id = room->askForCardChosen(player, damage.to, "h", objectName());
            room->throwCard(id, damage.to, player);
        }
        if (damage.to->getEquips().length()>0){
            int id = room->askForCardChosen(player, damage.to, "e", objectName());
            room->throwCard(id, damage.to, player);
        }
        return false;
    }
};

class Xiaowuwan : public TriggerSkill
{
public:
    Xiaowuwan() : TriggerSkill("xiaowuwan")
    {
        frequency = NotFrequent;
        events << TargetConfirmed;
    }

    virtual QStringList triggerable(TriggerEvent event, Room *, ServerPlayer *player, QVariant &data, ServerPlayer * &) const
    {
        if (event==TargetConfirmed){
            CardUseStruct use = data.value<CardUseStruct>();
            if (TriggerSkill::triggerable(player) && player==use.from && use.card && use.card->isKindOf("Slash")) {
                 return QStringList(objectName());
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

    virtual bool effect(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        bool discard = room->askForDiscard(player,objectName(),1,1,true,true,QString(),true);
        if (!discard)
            return false;
        CardUseStruct use = data.value<CardUseStruct>();
        QVariantList jink_list = player->tag["Jink_" + use.card->toString()].toList();

        room->sendCompulsoryTriggerLog(player, objectName());

        foreach(auto p, use.to){
            int index = use.to.indexOf(p);
            LogMessage log;
            log.type = "#NoJink";
            log.from = p;
            p->getRoom()->sendLog(log);
            jink_list[index] = 0;
        }

        player->tag["Jink_" + use.card->toString()] = jink_list;
        return false;
    }
};

class LeiqieTrigger : public TriggerSkill
{
public:
    LeiqieTrigger() : TriggerSkill("#leiqie")
    {
        frequency = NotFrequent;
        events << CardFinished << TargetConfirmed;
        global=true;
    }

    virtual QStringList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &) const
    {
        if (event == CardFinished) {
             CardUseStruct use = data.value<CardUseStruct>();
             foreach(auto p, use.to){
                 if (p->getMark("leiqie_null")>0){
                     room->setPlayerMark(p, "Armor_Nullified", p->getMark("Armor_Nullified")-1);
                     room->setPlayerMark(p, "leiqie_null", 0);
                 }
             }
        }
        if (event == TargetConfirmed) {
             CardUseStruct use = data.value<CardUseStruct>();
             if (use.from && use.from->hasSkill("leiqie") && player==use.from){
                 foreach(auto p, use.to){
                     if (p != use.from){
                         room->setPlayerMark(p, "Armor_Nullified", p->getMark("Armor_Nullified")+1);
                         room->setPlayerMark(p, "leiqie_null", 1);
                     }
                 }
             }
        }
        return QStringList();
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        return false;
    }
};

QString GaolingPattern = "pattern";
class Gaolingvs : public OneCardViewAsSkill
{
public:
    Gaolingvs() : OneCardViewAsSkill("gaoling")
    {
    }

    virtual bool isEnabledAtPlay(const Player *player) const
    {
        return false;
    }

    bool viewFilter(const Card *card) const
    {
        return true;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const
    {
        if ((pattern=="nullification" || pattern=="jink")&&player->getHandcardNum()%2==0){
            GaolingPattern = pattern;
            return player->hasSkill("gaoling");
        }
        return false;
    }
    virtual bool isEnabledAtNullification(const ServerPlayer *player) const
    {
        if (player->getHandcardNum()%2==0){
            GaolingPattern = "nullification";
            return player->hasSkill("gaoling");
        }
    }

    virtual const Card *viewAs(const Card *originalCard) const
    {
        QString pattern = GaolingPattern;
        Card *card=Sanguosha->cloneCard(pattern,originalCard->getSuit(),originalCard->getNumber());
        card->addSubcard(originalCard);
        card->setSkillName(objectName());
        card->setShowSkill(objectName());
        return card;
    }
};

class Gaoling : public TriggerSkill
{
public:
    Gaoling() : TriggerSkill("gaoling")
    {
        events << CardUsed << CardResponded;
        view_as_skill=new Gaolingvs;
    }

    virtual QStringList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &) const
    {
        if (event==CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            int id = use.card->getSubcards().at(0);
            Card *c = Sanguosha->getCard(id);
            if (use.card->getSkillName()==objectName() && c->isKindOf("EquipCard")){
                return QStringList(objectName());
            }
        }
        else{
            CardResponseStruct resp = data.value<CardResponseStruct>();
            int id = resp.m_card->getSubcards().at(0);
            Card *c = Sanguosha->getCard(id);
            if (resp.m_card->getSkillName()==objectName() && c->isKindOf("EquipCard")) {
                return QStringList(objectName());
            }
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
   {
        if (room->askForChoice(player, objectName(),"draw1card+cancel",data) == "draw1card") {
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        player->drawCards(1);
        return false;
    }
};

class Shengmu : public TriggerSkill
{
public:
    Shengmu() : TriggerSkill("shengmu")
    {
        events << Damaged;
    }

    virtual TriggerList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const
    {
        TriggerList skill_list;
        if (player == NULL) return skill_list;
        if (triggerEvent == Damaged) {
            DamageStruct damage = data.value<DamageStruct>();
            if ( damage.to->isDead())
                return skill_list;

            QList<ServerPlayer *> setsunas = room->findPlayersBySkillName(objectName());
            foreach(ServerPlayer *setsuna, setsunas)
                if (setsuna->canDiscard(setsuna, "he") && setsuna->getHandcardNum()%2 == 1 && (!room->getCurrent() || !room->getCurrent()->hasFlag(setsuna->objectName()+"shengmu_used")))
                    skill_list.insert(setsuna, QStringList(objectName()));
            return skill_list;
        }
        return skill_list;
    }

    virtual bool cost(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who) const
    {
        ServerPlayer *setsuna = ask_who;

        if (setsuna != NULL) {
            setsuna->tag["shengmu_data"] = data;
            const Card *card = room->askForCard(setsuna, ".|.|.", QString("@shengmu:%1:%2").arg(setsuna->getGeneralName(),player->getGeneralName()), data);
            setsuna->tag.remove("shengmu_data");

            if (card) {
                if (room->getCurrent()){
                    room->setPlayerFlag(room->getCurrent(), setsuna->objectName()+"shengmu_used");
                }
                if (card->isRed()){
                    room->setPlayerFlag(setsuna, "shengmu_red");
                }
                room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, setsuna->objectName(), data.value<DamageStruct>().to->objectName());
                room->broadcastSkillInvoke(objectName(), setsuna);
                return true;
            }
        }
        return false;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *ask_who) const
    {
        ServerPlayer *setsuna = ask_who;
        if (setsuna == NULL) return false;
        DamageStruct damage = data.value<DamageStruct>();

        RecoverStruct recover;
        recover.who = setsuna;
        room->recover(player, recover, true);
        if (setsuna->hasFlag("shengmu_red")){
            room->setPlayerFlag(setsuna, "-shengmu_red");
            room->doLightbox("SE_Shengmu$", 1000);
            player->drawCards(1);
        }
        return false;
    }
};

class Fenjie : public TriggerSkill
{
public:
    Fenjie() : TriggerSkill("fenjie")
    {
        frequency = NotFrequent;
        events << CardEffect;
    }

    virtual TriggerList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
    {
        TriggerList skill_list;
        CardEffectStruct effect = data.value<CardEffectStruct>();
        QList<ServerPlayer *> tatsuyas = room->findPlayersBySkillName(objectName());
        ServerPlayer *current = room->getCurrent();
        foreach (ServerPlayer *tatsuya, tatsuyas) {
            if (effect.from && tatsuya == effect.from && effect.to && effect.from != effect.to && (effect.card->isKindOf("BasicCard")||effect.card->isNDTrick()) && (!current || !current->hasFlag(tatsuya->objectName()+"fenjie"))){
                 skill_list.insert(tatsuya, QStringList(objectName()));
            }
        }
        return skill_list;
    }

    virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *tatsuya) const
    {
        if (tatsuya->askForSkillInvoke(this, data)){
            return true;
        }
        return false;
    }

     virtual bool effect(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *tatsuya) const
    {
        CardEffectStruct effect = data.value<CardEffectStruct>();
        ServerPlayer *current = room->getCurrent();
        if (current){
            room->setPlayerFlag(current, tatsuya->objectName()+"fenjie");
        }
        if (effect.to->getEquips().length()>0){
            int id = room->askForCardChosen(tatsuya, effect.to, "e", objectName());
            room->throwCard(id, effect.to, tatsuya);
            DamageStruct da;
            da.from=tatsuya;
            da.to=effect.to;
            da.damage=1;
            room->damage(da);

        }
        return false;
    }
};

ChongzuCard::ChongzuCard()
{
    target_fixed = true;
}

void ChongzuCard::use(Room *room, ServerPlayer *player, QList<ServerPlayer *> &targets) const
{
    QList<int> list;
    foreach(int id, room->getDiscardPile()){
        Card *c = Sanguosha->getCard(id);
        if (c->isKindOf("BasicCard") || c->isKindOf("EquipCard")){
          list << id;
        }
    }
    if (list.length()>0){
        room->fillAG(list, player);
        int id = room->askForAG( player,list, false,"chongzu");
        QList<ServerPlayer*> players;
        foreach(auto p, room->getAlivePlayers()){
            players << p;
        }
        ServerPlayer *dest;
        if (!players.isEmpty()){
          dest = room->askForPlayerChosen(player, players, objectName());
          room->obtainCard(dest, id);
        }
        room->clearAG(player);
        Card *card = Sanguosha->getCard(id);
        if (card->isKindOf("EquipCard") && room->askForChoice(dest, "chongzu", "use_card_chongzu+cancel")!="cancel"){
            CardUseStruct use;
            use.from = dest;
            use.to.append(dest);
            use.card = card;
            room->useCard(use, false);
        }
    }
}

class Chongzu : public ViewAsSkill
{
public:
    Chongzu() : ViewAsSkill("chongzu")
    {
    }

    bool isEnabledAtPlay(const Player *player) const
    {
        return !player->hasUsed("ChongzuCard");
    }

    bool viewFilter(const QList<const Card *> &selected, const Card *) const
    {
        return selected.length()< 2;
    }

    const Card *viewAs(const QList<const Card *> &cards) const
    {
        if (cards.length() != 2)
            return NULL;
        ChongzuCard *vs = new ChongzuCard();
        vs->addSubcards(cards);
        vs->setSkillName(objectName());
        vs->setShowSkill(objectName());
        return vs;
    }
};

//careerist
class CareeristTransform : public TriggerSkill
{
public:
    CareeristTransform() : TriggerSkill("careeristtransform")
    {
        frequency = Compulsory;
        events << GeneralTransformed;
        global = true;
    }

    virtual QStringList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer * &) const
    {
        QString to = data.toString().split(":").last();
        const General *general = Sanguosha->getGeneral(to);
        if (general->getKingdom() == "careerist"){
            return QStringList(objectName());
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *kirei) const
   {
        return true;
    }

    virtual bool effect(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *kirei) const
    {
        QString to = data.toString().split(":").last();
        if (player->getGeneral()->objectName() == to){
            room->transformHeadGeneral(player);
        }
        else if (player->getGeneral2()->objectName() == to){
            room->transformDeputyGeneral(player);
        }
        return false;
    }
};




class Yuyue : public TriggerSkill
{
public:
    Yuyue() : TriggerSkill("yuyue")
    {
        frequency = NotFrequent;
        events << Damage;
    }

    virtual TriggerList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
    {
        TriggerList skill_list;
        if (event==Damage){
            DamageStruct damage = data.value<DamageStruct>();
            QList<ServerPlayer *> kireis = room->findPlayersBySkillName(objectName());
            foreach (ServerPlayer *kirei, kireis) {
                if (damage.damage>0){
                     skill_list.insert(kirei, QStringList(objectName()));
                }
            }
        }
        /*if (event == EventPhaseStart && player->getPhase() == Player::Start){
            //room->removePlayerDisableShow(player, objectName());
        }*/
        return skill_list;
    }

    virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *kirei) const
   {
        if (kirei->askForSkillInvoke(this, data)) {
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *kirei) const
    {
        DamageStruct damage = data.value<DamageStruct>();
        room->broadcastSkillInvoke(objectName(), kirei);
        kirei->drawCards(damage.damage);
        if (damage.damage > 1){
            room->doLightbox("Yuyue$", 800);
        }
        return false;
    }
};

class Xianhai : public TriggerSkill
{
public:
    Xianhai() : TriggerSkill("xianhai")
    {
        frequency = NotFrequent;
        events << DamageCaused;
    }

    virtual TriggerList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
    {
        TriggerList skill_list;
        if (event==DamageCaused){
            DamageStruct damage = data.value<DamageStruct>();
            QList<ServerPlayer *> kireis = room->findPlayersBySkillName(objectName());
            foreach (ServerPlayer *kirei, kireis) {
                if (damage.to->isAlive() && damage.to->getHp()-damage.damage <= 0 && !room->getTag(kirei->objectName()+damage.to->objectName()+"xianhai").toBool()){
                     skill_list.insert(kirei, QStringList(objectName()));
                }
            }
        }
        /*if (event == EventPhaseStart && player->getPhase() == Player::Start){
            //room->removePlayerDisableShow(player, objectName());
        }*/
        return skill_list;
    }

    virtual bool cost(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *kirei) const
   {
        if (kirei->askForSkillInvoke(this, data)) {
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *kirei) const
    {
        DamageStruct damage = data.value<DamageStruct>();
        room->setTag(kirei->objectName()+damage.to->objectName()+"xianhai", QVariant(true));
        room->broadcastSkillInvoke(objectName(), kirei);
        ServerPlayer *dest = room->askForPlayerChosen(kirei, room->getAlivePlayers(), objectName());
        damage.from = dest;
        data.setValue(damage);
        return false;
    }
};

class Heimu : public TriggerSkill
{
public:
    Heimu() : TriggerSkill("heimu")
    {
        events << GeneralShown;
        frequency = Compulsory;
    }

    virtual bool canPreshow() const
    {
        return false;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const
    {
        if (triggerEvent == GeneralShown) {
            if (TriggerSkill::triggerable(player))
                return (data.toBool() == player->inHeadSkills(objectName())) ? QStringList(objectName()) : QStringList();
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        if (triggerEvent == GeneralShown) {
            return true;
        }
        return false;
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const
    {
        if (triggerEvent == GeneralShown){
           room->broadcastSkillInvoke(objectName(), player);
           foreach(auto p, room->getOtherPlayers(player)){
               DamageStruct da;
               da.from=player;
               da.to=p;
               da.damage=1;
               da.nature= DamageStruct::Normal;
               room->damage(da);
           }
        }
        return false;
    }
};

class Juewang : public TriggerSkill
{
public:
    Juewang() : TriggerSkill("juewang")
    {
        events << EventPhaseStart << EventPhaseEnd << Death;
        frequency = Compulsory;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const
    {
        if (triggerEvent == EventPhaseStart) {
            if (TriggerSkill::triggerable(player) && player->getPhase()==Player::Start)
                return QStringList(objectName());
        }
        /*if (triggerEvent == Death) {
            DeathStruct death = data.value<DeathStruct>();
            if (TriggerSkill::triggerable(player) && death.damage->from&& player==death.damage->from)
                return QStringList(objectName());
        }*/
        if (triggerEvent == EventPhaseEnd) {
            if (TriggerSkill::triggerable(player) && player->getPhase()==Player::Finish){
                foreach(auto p, room->getAlivePlayers()){
                    room->removePlayerCardLimitation(p, "use,response", ".|.|.|.");
                }
            }
        }
        return QStringList();
    }

    virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        if (triggerEvent == EventPhaseStart) {
            return player->hasShownSkill(objectName())||player->askForSkillInvoke(this, data);
        }
        return false;
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const
    {
        if (triggerEvent == EventPhaseStart){
           room->broadcastSkillInvoke(objectName(), player);
           foreach(auto p, room->getAlivePlayers()){
               if (!p->isFriendWith(player)){
                   bool has = false;
                   foreach(auto q, room->getOtherPlayers(p)){
                       if (p->isFriendWith(q)){
                           has = true;
                       }
                   }
                   if (p->getHp()<player->getHp()){
                       room->setPlayerCardLimitation(p, "use,response", ".|.|.|.", false);
                   }
               }
           }
        }
        if (triggerEvent == Death) {
            room->broadcastSkillInvoke(objectName(), player);
            foreach(auto p, room->getOtherPlayers(player)){
                if (!p->isFriendWith(player)){
                  DamageStruct da;
                  da.from=player;
                  da.to=p;
                  da.damage=1;
                  da.nature= DamageStruct::Normal;
                  room->damage(da);
                }
            }
        }
        return false;
    }
};

class Bingdu : public TriggerSkill
{
public:
    Bingdu() : TriggerSkill("bingdu")
    {
        events << BuryVictim;
    }

    virtual int getPriority() const
    {
        return -4;
    }

    virtual TriggerList triggerable(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const
    {
        TriggerList skill_list;
        if (event == BuryVictim) {

            DeathStruct death = data.value<DeathStruct>();
            QList<ServerPlayer *> junkos = room->findPlayersBySkillName(objectName());
            foreach (ServerPlayer *junko, junkos) {
              if (TriggerSkill::triggerable(junko) && death.damage && death.damage->from && junko== death.damage->from)
                  skill_list.insert(junko, QStringList(objectName()));
            }
        }

        return skill_list;
    }

    virtual bool cost(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *junko) const
    {
        if (triggerEvent == BuryVictim) {
            return junko->askForSkillInvoke(this, data);
        }
        return false;
    }

    virtual bool effect(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *junko) const
    {
        DeathStruct death = data.value<DeathStruct>();
        QString choice = room->askForChoice(junko, "bingdu", "bingdu_revive+bingdu_use");
        if(choice=="bingdu_revive"){
            room->revivePlayer(death.who);
            room->setPlayerProperty(death.who, "hp", QVariant(1));
            room->setPlayerProperty(death.who, "role", QVariant("careerist"));
            room->setPlayerProperty(death.who, "CareeristFriend", junko->objectName());
        }
        else{
            Card *card = Sanguosha->cloneCard("ruler_card");
            card->setSkillName("juewang");
            CardUseStruct use;
            use.from = junko;
            use.card = card;
            room->useCard(use, false);
        }
        return false;
    }
};

//ice_slash
class IceSlashSkill : public TriggerSkill
{
public:
    IceSlashSkill() : TriggerSkill("IceSlash")
    {
        events << DamageCaused;
        global = true;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer *) const
    {
        DamageStruct damage = data.value<DamageStruct>();

        if (damage.card && damage.card->isKindOf("IceSlash")
            && !damage.to->isNude() && damage.by_user
            && !damage.chain && !damage.transfer && player->askForSkillInvoke(this, data)) {
            if (damage.from->canDiscard(damage.to, "he")) {
                int card_id = room->askForCardChosen(player, damage.to, "he", "IceSword", false, Card::MethodDiscard);
                room->throwCard(Sanguosha->getCard(card_id), damage.to, damage.from);

                if (damage.from->isAlive() && damage.to->isAlive() && damage.from->canDiscard(damage.to, "he")) {
                    card_id = room->askForCardChosen(player, damage.to, "he", "IceSword", false, Card::MethodDiscard);
                    room->throwCard(Sanguosha->getCard(card_id), damage.to, damage.from);
                }
            }
            return true;
        }
        return false;
    }
};

//bug fix

class BugFixSkill1 : public TriggerSkill
{
public:
    BugFixSkill1() : TriggerSkill("bugfixskill1")
    {
        events << GeneralTransformed << GeneralShown << GeneralRevived;
        global = true;
    }

    virtual int getPriority() const
    {
        return 2;
    }

    virtual void record(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &) const
    {
        foreach(auto p, room->getAlivePlayers()){
            foreach (const Skill *skill, p->getVisibleSkillList()) {
                if (skill->getFrequency() == Skill::Club && !skill->getClubName().isEmpty() && p->hasShownSkill(skill)
                    && (!skill->isLordSkill() || p->hasLordSkill(skill->objectName()))){
                    if (!p->hasClub(skill->getClubName())){
                       p->addClub(skill->getClubName());
                    }
                }
            }
        }

        foreach(auto p, room->getAlivePlayers()){
            foreach (const Skill *skill, p->getVisibleSkillList()) {
                if (!p->hasShownSkill(skill->objectName()))
                    continue;
                Package *package;
                const QList<const Package *> packages = Sanguosha->getPackages();
                foreach(const Package *pa, packages){
                    package=const_cast<Package *>(pa);
                    if (package && package->getRelatedAttachSkill(skill->objectName()) != ""){
                        QString s =  package->getRelatedAttachSkill(skill->objectName());
                        foreach(auto q, room->getAlivePlayers()){
                            const Skill *sk = Sanguosha->getSkill(s);
                            if (!q->hasSkill(s) && sk->isAttachedLordSkill()){
                                room->attachSkillToPlayer(q, s);
                            }
                        }
                    }

                }
            }
        }
    }
};

//new cards
GuangyuCard::GuangyuCard(Suit suit, int number, bool is_transferable) : BasicCard(suit, number)
{
    setObjectName("guangyucard");
    target_fixed = false;
    transferable = is_transferable;
}

QString GuangyuCard::getSubtype() const
{
    return "assist_card";
}

bool GuangyuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    int total_num = 1 + Sanguosha->correctCardTarget(TargetModSkill::ExtraTarget, Self, this);

    if (targets.length() >= total_num)
        return false;

    return true;
}

void GuangyuCard::onUse(Room *room, const CardUseStruct &card_use) const
{
    CardUseStruct use = card_use;
    if (use.to.isEmpty())
        use.to << use.from;
    BasicCard::onUse(room, use);
}

void GuangyuCard::onEffect(const CardEffectStruct &effect) const
{
    Room *room = effect.to->getRoom();
    //room->setEmotion(effect.from, "guangyu");

    if (effect.to->hasFlag("Global_Dying") && Sanguosha->getCurrentCardUseReason() != CardUseStruct::CARD_USE_REASON_PLAY) {
        // recover hp
        RecoverStruct recover;
        recover.card = this;
        recover.who = effect.from;
        room->recover(effect.to, recover ,true);
    } else {
        room->setPlayerProperty(effect.to, "chained", QVariant(false));
        if (!effect.to->faceUp()){
            effect.to->turnOver();
        }
    }

    foreach(auto c, effect.to->getJudgingArea()){
        if (c->isKindOf("Key")){
            room->obtainCard(effect.to, c);
        }
    }
}

bool GuangyuCard::isAvailable(const Player *player) const
{
    return !player->isProhibited(player, this) && BasicCard::isAvailable(player);
}

Eireishoukan::Eireishoukan(Card::Suit suit, int number)
    : SingleTargetTrick(suit, number)
{
    setObjectName("eirei_shoukan");
    target_fixed = true;
}

QString Eireishoukan::getSubtype() const
{
    return "eirei_shoukan";
}

void Eireishoukan::onUse(Room *room, const CardUseStruct &card_use) const
{
    CardUseStruct use = card_use;
    if (use.to.isEmpty())
        use.to << use.from;
    SingleTargetTrick::onUse(room, use);
}

void Eireishoukan::onEffect(const CardEffectStruct &effect) const
{
    ServerPlayer *player = effect.to;
    Room *room = effect.to->getRoom();

    if (!player->getGeneral2()) return;

    if (!player->hasShownGeneral2())
        player->showGeneral(false, false, false);

    QStringList names;
    names << player->getActualGeneral1Name() << player->getActualGeneral2Name();
    QStringList available, to_select;
    foreach (QString name, Sanguosha->getLimitedGeneralNames()) {
        if ((player->getKingdom() == "careerist" || Sanguosha->getGeneral(name)->getKingdom().split("|").contains(player->getKingdom()))
                && !name.startsWith("lord_") && !room->getUsedGeneral().contains(name) && Sanguosha->getGeneral(name)->getKingdom() != "careerist")
            available << name;
    }
    if (available.isEmpty()) return;

    QVariant qnum;
    int num = 3;
    qnum = num;
    if (player->getKingdom()=="magic"){
        qnum = num+1;
    }

    //room->getThread()->trigger(GeneralTransforming, this, player, qnum);
    num = qnum.toInt();
    if (num < 1) return;

    qShuffle(available);
    for (int i = 1; i <= num; i++) {
        if (available.isEmpty()) break;
        to_select << available.takeFirst();
    }

    QString general_name = room->askForGeneral(player, to_select.join("+"), QString(), true, "transform");

    room->handleUsedGeneral("-" + player->getActualGeneral2Name());
    room->handleUsedGeneral(general_name);

    player->removeGeneral(false);

    QStringList duanchangList = player->property("Duanchang").toString().split(",");
    if (duanchangList.contains("deputy"))
        duanchangList.removeOne("deputy");
    room->setPlayerProperty(player, "Duanchang", duanchangList.join(","));

    QVariant void_data;
    QList<const TriggerSkill *> game_start;

    foreach (const Skill *skill, Sanguosha->getGeneral(general_name)->getVisibleSkillList(true, false)) {
        if (skill->inherits("TriggerSkill")) {
            const TriggerSkill *tr = qobject_cast<const TriggerSkill *>(skill);
            if (tr != NULL) {
                if (tr->getTriggerEvents().contains(GameStart) && !tr->triggerable(GameStart, room, player, void_data).isEmpty())
                    game_start << tr;
            }
        }
        player->addSkill(skill->objectName(), false);
    }

    room->changePlayerGeneral2(player, "anjiang");
    player->setActualGeneral2Name(general_name);
    room->notifyProperty(player, player, "actual_general2");
    room->notifyProperty(player, player, "general2", general_name);

    QVariant string;
    string = names[1]+":"+general_name;

    names[1] = general_name;
    room->setPlayerProperty(player, "general2_showed", false);

    room->setTag(player->objectName(), names);

    foreach (const Skill *skill, Sanguosha->getGeneral(general_name)->getSkillList(true, false)) {
        if (skill->getFrequency() == Skill::Limited && !skill->getLimitMark().isEmpty()) {
            player->setMark(skill->getLimitMark(), 1);
            JsonArray arg;
            arg << player->objectName();
            arg << skill->getLimitMark();
            arg << 1;
            room->doNotify(player,QSanProtocol::S_COMMAND_SET_MARK, arg);
        }
    }

    foreach (const TriggerSkill *skill, game_start) {
        if (skill->cost(GameStart, room, player, void_data, player))
            skill->effect(GameStart, room, player, void_data, player);
    }

    player->showGeneral(false, false, true);
    room->getThread()->trigger(GeneralTransformed, room, player, string);
}

bool Eireishoukan::isAvailable(const Player *player) const
{
    return player->hasShownOneGeneral() && !player->isProhibited(player, this) && TrickCard::isAvailable(player);
}

Isekai::Isekai(Card::Suit suit, int number)
    : SingleTargetTrick(suit, number)
{
    setObjectName("isekai");
    target_fixed = true;
}

QString Isekai::getSubtype() const
{
    return "isekai";
}

void Isekai::onUse(Room *room, const CardUseStruct &card_use) const
{
    CardUseStruct use = card_use;
    if (use.to.isEmpty())
        use.to << use.from;
    SingleTargetTrick::onUse(room, use);
}

void Isekai::onEffect(const CardEffectStruct &effect) const
{
    ServerPlayer *player = effect.to;
    Room *room = effect.to->getRoom();
    int n = player->getHandcardNum();
    player->throwAllHandCards();
    QString choice = room->askForChoice(player, "isekai", "draw_maxhpcards_recover+draw_throwcards", QVariant::fromValue(n));
    if (choice == "draw_maxhpcards_recover"){
        int m = player->getMaxHp();
        player->drawCards(m);
        if (m<n){
            RecoverStruct recover;
            recover.card = this;
            recover.who = effect.from;
            room->recover(effect.to, recover ,true);
        }
    }
    else{
        player->drawCards(n);
    }
    if (player->getKingdom()=="real"){
        room->setPlayerProperty(effect.to, "chained", QVariant(false));
        if (!effect.to->faceUp()){
            effect.to->turnOver();
        }
    }
}

bool Isekai::isAvailable(const Player *player) const
{
    return !player->isProhibited(player, this) && TrickCard::isAvailable(player);
}

RulerCard::RulerCard(Card::Suit suit, int number, bool is_transferable)
    : AOE(suit, number)
{
    setObjectName("ruler_card");
    transferable = is_transferable;
}

bool RulerCard::isAvailable(const Player *player) const
{
    bool canUse = false;
    QList<const Player *> players = player->getNextAlive()->getFormation();
    foreach (const Player *p, players) {
        if (player->isProhibited(p, this))
            continue;

        canUse = true;
        break;
    }
    bool judge = true;
    if (player->getMark("ruler_card_used")>0 && player->getRole()!="careerist"){
        judge = false;
    }
    if (player->getMark("ruler_card_turn")>0){
        judge = false;
    }
    return canUse && TrickCard::isAvailable(player) && judge;
}

void RulerCard::onUse(Room *room, const CardUseStruct &card_use) const
{
    CardUseStruct new_use = card_use;
    foreach(auto p, room->getAlivePlayers()){
        if (p->getRole()!="careerist"){
            new_use.to << p;
        }
    }

    if (card_use.from){
        room->setPlayerMark(card_use.from, "ruler_card_used", card_use.from->getMark("ruler_card_used")+1);
        room->setTag("ruler_card_turn", QVariant(card_use.from->objectName()));
        foreach(auto p, room->getOtherPlayers(card_use.from)){
           room->setPlayerMark(p, "ruler_card_turn", 1);
        }
    }
    if (!card_use.to.isEmpty()){
       TrickCard::onUse(room, card_use);
    }
    else {
        TrickCard::onUse(room, new_use);
    }
}

void RulerCard::onEffect(const CardEffectStruct &effect) const
{
    Room *room = effect.to->getRoom();
    QList<ServerPlayer *> ruler_targets;
    foreach (auto p, room->getAlivePlayers()) {
        if (effect.to->canSlash(p, NULL, false) && effect.to->inMyAttackRange(p) && effect.to != p)
            ruler_targets << p;
    }
    ServerPlayer *target;
    const Card *card;
    if (!ruler_targets.isEmpty()){
        target = room->askForPlayerChosen(effect.to, ruler_targets, objectName(), QString(), true);
    }
    if (target){
        card = room->askForUseSlashTo(effect.to, target, "@ruler-slash");
    }
    if (ruler_targets.isEmpty() || !target || !card)
    {
        if (effect.to->getHandcardNum()+effect.to->getEquips().length()>2){
           room->askForDiscard(effect.to, objectName(), 2, 2, false, true);
        }
        else{
            effect.to->throwAllCards();
        }
    }
    else if(target && card){
        target->gainMark("@rulers");
    }
}


RevolutionPackage::RevolutionPackage()
    : Package("revolution")
{

  skills << new Xingbao << new Jiyuunotsubasa << new Shiso << new Zahyo << new TrialTrigger<< new LianjiTrigger
        << new LunhuiRecord << new PoxiaoTrigger << new YandanMaxCards << new PoshiDistance << new PoshiTargetMod << new PoshiTrigger << new ZizhengTrigger << new Xintiao << new ZhouliRecord
        << new WurenRange << new Leiqie << new Huocheqie << new HuocheqieTargetMod << new Tongziqie << new Paoqie << new Xiaowuwan << new Wuyoumax << new LeiqieTrigger << new Daokegive << new BugFixSkill1 << new CommandEffect;
  //skills << new CareeristTransform;

  //real
  General *yukino = new General(this, "Yukino", "real", 3, false);
  yukino->addSkill(new Shifeng);
  yukino->addSkill(new Zhiyan);
  yukino->addCompanion("Hikigaya");

  General *yuri = new General(this, "yuri", "real", 3, false);
  yuri->addSkill(new Zuozhan);
  yuri->addSkill(new Nishen);

  General *yuzuru = new General(this, "Yuzuru", "real", 4);
  yuzuru->addSkill(new Jiuzhu);
  yuzuru->addSkill(new Shexin);
  yuzuru->addRelateSkill("xintiao");
  yuzuru->setDeputyMaxHpAdjustedValue(-1);
  yuzuru->addCompanion("Kanade");

  General *chitanda = new General(this, "Chitanda", "real", 3, false);
  chitanda->addSkill(new Shouji);
  chitanda->addSkill(new Haoqi);

  General *sonya = new General(this, "Sonya", "real", 4, false);
  sonya->addSkill(new Shashou);

  //science
  General *kirito = new General(this, "Kirito", "science", 4);
  kirito->addSkill(new Erdao);
  kirito->addSkill(new ErdaoExtra);
  insertRelatedSkills("erdao", "#erdaoextra");
  kirito->addSkill(new Fengbi);
  kirito->addCompanion("SE_Asuna");
  kirito->addRelateSkill("xingbao");

  General *eugeo = new General(this, "Eugeo", "science", 3);
  eugeo->addSkill(new Rennai);
  eugeo->addSkill(new Zhanfang);
  eugeo->addCompanion("Kirito");

  General *yuuki = new General(this, "Yuuki", "science", 4, false);
  yuuki->addSkill(new Yaozhan);
  yuuki->addSkill(new Lianji);
  yuuki->addCompanion("SE_Asuna");

  General *wsaki = new General(this, "WSaki", "science", 3, false);
  wsaki->addSkill(new Zhouli);
  wsaki->addSkill(new Zhenyan);
  wsaki->addSkill(new ZhenyanRecord);
  insertRelatedSkills("zhenyan", "#zhenyan-record");

  General *akame = new General(this, "Akame", "science", 3, false);
  akame->addSkill(new Zhuisha);
  akame->addSkill(new Songzang);

  //magic
  General *rem = new General(this, "Rem", "magic", 3, false);
  rem->addSkill(new Lianchui);
  rem->addSkill(new Xianshu);
  rem->addSkill(new Huanbing);
  General *asagi = new General(this, "Asagi", "magic", 3, false);
  asagi->addSkill(new Guanli);
  asagi->addSkill(new Poyi);
  //General *arc4subaru = new General(this, "Arc4subaru", "magic", 3);
  //arc4subaru->addSkill(new Trial);
  General *fuwaaika = new General(this, "Fuwaaika", "magic", 3, false);
  fuwaaika->addSkill(new Poshi);
  fuwaaika->addSkill(new Liansuo);
  fuwaaika->addSkill(new Yinguo);
  General *enmaai = new General(this, "Enmaai", "magic", 3, false);
  enmaai->addSkill(new Weituo);
  enmaai->addSkill(new Liufang);

  General *tohsaka = new General(this, "Tohsaka", "magic", 3, false);
  tohsaka->addSkill(new Jiaji);
  tohsaka->addSkill(new Modan);
  tohsaka->addCompanion("EmiyaShirou");

  //game
  General *akagi= new General(this, "Akagi", "game", 4, false);
  akagi->addSkill(new Chicheng);
  akagi->addSkill(new Zhikong);
  akagi->addCompanion("Kaga");
  General *asashio = new General(this, "Asashio", "game", 3, false);
  asashio->addSkill(new Fanqian);
  asashio->addSkill(new Buyu);

  General *reisen= new General(this, "Reisen", "game", 3, false);
  reisen->addSkill(new Huanshi);
  reisen->addSkill(new Kuangzao);

  General *noire = new General(this, "Noire", "game", 3, false);
  noire->addSkill(new Wushi);
  noire->addSkill(new Wuyou);

  General *noora = new General(this, "Noora", "game", 3, false);
  noora->addSkill(new Daoke);
  noora->addSkill(new Kaihua);
  insertRelatedAttachSkill("daoke", "daokegive");


  //lord
  /*General *lorderen = new General(this, "lord_SE_Eren$", "science", 4, true, true);
  lorderen->addSkill(new Jinji);
  lorderen->addRelateSkill("jiyuunotsubasa");
  lorderen->addSkill(new Shizu);
  lorderen->addRelateSkill("zahyo");
  insertRelatedSkills("shizu", "zahyo");*/

  //boss
  //General *shiso_no_kyojin = new General(this, "Shiso_no_kyojin", "god", 5, true, true);

  //double kingdom
  General *rika = new General(this, "Rika", "magic|real", 3, false);
  rika->addSkill(new Suipian);
  rika->addSkill(new Lunhui);
  rika->addSkill(new Poxiao);
  General *nmakoto = new General(this, "NMakoto", "science|real", 4);
  nmakoto->addSkill(new Yandan);
  nmakoto->addSkill(new Lunpo);
  nmakoto->addSkill(new LunpoRecord);
  insertRelatedSkills("lunpo", "#lunpo-record");
  nmakoto->addSkill(new Zizheng);
  nmakoto->setHeadMaxHpAdjustedValue(-1);
  General *premiere = new General(this, "Premiere", "science|game", 3 ,false);
  premiere->addCompanion("Kirito");
  premiere->addSkill(new Xuexi);
  premiere->addSkill(new XuexiTri);
  insertRelatedSkills("xuexi", "#xuexitri");
  premiere->addSkill(new Qinggan);
  General *misuzu = new General(this, "Misuzu", "magic|game", 4 ,false);
  misuzu->addSkill(new Wuren);
  General *setsuna = new General(this, "Setsuna", "real|game", 3 ,false);
  setsuna->addSkill(new Gaoling);
  setsuna->addSkill(new Shengmu);
  General *tatsuya = new General(this, "Tatsuya", "magic|science", 4);
  tatsuya->addSkill(new Fenjie);
  tatsuya->addSkill(new Chongzu);

  //General *touma = new General(this, "NTTouma", "magic|science", 4);
  //touma->addCompanion("Mikoto");

  //careerist
  General *kirei = new General(this, "Kirei", "careerist", 3);
  kirei->addSkill(new Yuyue);
  kirei->addSkill(new Xianhai);
  General *junko = new General(this, "Junko", "careerist", 3, false);
  junko->addSkill(new Heimu);
  junko->addSkill(new Juewang);
  junko->addSkill(new Bingdu);

  addMetaObject<ShifengCard>();
  addMetaObject<ZhiyanCard>();
  addMetaObject<XingbaoCard>();
  addMetaObject<ShisoCard>();
  addMetaObject<FanqianCard>();
  addMetaObject<PoyiCard>();
  addMetaObject<ChichengCard>();
  addMetaObject<YaozhanCard>();
  addMetaObject<PoxiaoCard>();
  addMetaObject<PoshiCard>();
  addMetaObject<JiuzhuCard>();
  addMetaObject<ShexinCard>();
  addMetaObject<ZhouliCard>();
  addMetaObject<HuanshiCard>();
  addMetaObject<ShoujiCard>();
  addMetaObject<ModanCard>();
  addMetaObject<ShashouCard>();
  addMetaObject<DaokegiveCard>();
  addMetaObject<ChongzuCard>();
}

RevolutionCardPackage::RevolutionCardPackage() : Package("revolutioncard", CardPack)
{
    QList<Card *> cards;
    cards << new IceSlash(Card::Spade, 7)
        << new IceSlash(Card::Spade, 7)
        << new IceSlash(Card::Spade, 8)
        << new IceSlash(Card::Spade, 8)
        << new IceSlash(Card::Spade, 8)
        << new GuangyuCard(Card::Heart, 5)
        << new GuangyuCard(Card::Heart, 6)
        << new GuangyuCard(Card::Heart, 9)
        << new Eireishoukan(Card::Diamond, 1)
        << new Isekai(Card::Club, 5)
        << new RulerCard(Card::Spade, 6);

    foreach(Card *card, cards)
        card->setParent(this);

}

ADD_PACKAGE(Revolution)
ADD_PACKAGE(RevolutionCard)
