
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
        else if (event == EventPhaseChanging){
            QStringList result = player->tag["zuozhan_tag"].toStringList();
            if (result.count() == 0){
                return skill_list;
            }
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.to == Player::NotActive){
                player->tag["zuozhan_tag"].clear();
                return skill_list;
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
                if (!yuri || !yuri->isAlive() || dying.who->hasClub()){
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
       if (event == Death && player->hasShownSkill(this)||player->askForSkillInvoke(this, data)){
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
        events << GeneralShown << DrawNCards;
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
       if (event == GeneralShown){
           if ( TriggerSkill::triggerable(player) && player->hasShownAllGenerals()){
               bool head = player->inHeadSkills(this);
               room->detachSkillFromPlayer(player, objectName(), false, false, head);
               room->acquireSkill(player, "xingbao", true, head);
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

class Lianchui : public TriggerSkill
{
public:
    Lianchui() : TriggerSkill("lianchui")
    {
        frequency = NotFrequent;
        events << TargetChosen;
        relate_to_place = "head";
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const
   {
        if (triggerEvent==TargetChosen){
            CardUseStruct use = data.value<CardUseStruct>();
            if (TriggerSkill::triggerable(player) && use.card != NULL && (use.card->isKindOf("Slash")||use.card->isKindOf("Duel"))) {
                QStringList targets;
                foreach (ServerPlayer *to, use.to) {

                }
                if (!targets.isEmpty())
                    return QStringList(objectName() + "->" + targets.join("+"));
            }
        }
        return QStringList();
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
        events << DamageCaused << CardUsed;
    }

    virtual QStringList triggerable(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data, ServerPlayer* &) const
   {
        if (triggerEvent == DamageCaused){
            DamageStruct damage = data.value<DamageStruct>();
            if (TriggerSkill::triggerable(player) && damage.card && damage.card->isBlack()){
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
                    room->doLightbox("Erenattack1$", 2000);
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

RevolutionPackage::RevolutionPackage()
    : Package("revolution")
{

  skills << new Xingbao << new Jiyuunotsubasa << new Shiso << new Zahyo;

  //real
  General *yukino = new General(this, "Yukino", "real", 3, false);
  yukino->addSkill(new Shifeng);
  yukino->addSkill(new Zhiyan);
  yukino->addCompanion("Hikigaya");

  General *yuri = new General(this, "yuri", "real", 3, false);
  yuri->addSkill(new Zuozhan);
  yuri->addSkill(new Nishen);

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

  //magic
  General *rem = new General(this, "Rem", "magic", 3, false);
  rem->addSkill(new Lianchui);
  rem->addSkill(new Xianshu);
  rem->addSkill(new Huanbing);
  General *Asagi = new General(this, "Asagi", "magic", 3, false);

  //game
  General *akagi= new General(this, "Akagi", "game", 4, false);
  General *asashio = new General(this, "Asashio", "game", 3, false);

  //lord
  General *lorderen = new General(this, "lord_SE_Eren$", "science", 4, true, true);
  lorderen->addSkill(new Jinji);
  lorderen->addRelateSkill("jiyuunotsubasa");
  lorderen->addSkill(new Shizu);
  lorderen->addRelateSkill("zahyo");
  insertRelatedSkills("shizu", "zahyo");

  //boss
  //General *shiso_no_kyojin = new General(this, "Shiso_no_kyojin", "god", 5, true, true);

  addMetaObject<ShifengCard>();
  addMetaObject<ZhiyanCard>();
  addMetaObject<XingbaoCard>();
  addMetaObject<ShisoCard>();

}

RevolutionCardPackage::RevolutionCardPackage() : Package("revolutioncard", CardPack)
{
    QList<Card *> cards;

    foreach(Card *card, cards)
        card->setParent(this);

}

ADD_PACKAGE(Revolution)
ADD_PACKAGE(RevolutionCard)
