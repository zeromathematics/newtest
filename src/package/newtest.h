
#ifndef _TEST_H
#define _TEST_H

#include "package.h"
#include "card.h"
#include "wrappedcard.h"
#include "skill.h"
#include "standard.h"
#include "generaloverview.h"

class NewtestPackage : public Package
{
    Q_OBJECT

public:
    NewtestPackage();
};

class NewtestCardPackage : public Package
{
    Q_OBJECT

public:
    NewtestCardPackage();
};

class Key : public DelayedTrick
{
    Q_OBJECT

public:
    Q_INVOKABLE Key(Card::Suit suit, int number);

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void takeEffect(ServerPlayer *target) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class ZhurenCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE ZhurenCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class GonglueCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE GonglueCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class ShourenCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE ShourenCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class QiyuanCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE QiyuanCard();
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class JizhanCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE JizhanCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class LiaoliCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE LiaoliCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class HeiyanCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE HeiyanCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class TiaojiaoCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE TiaojiaoCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class DuanzuiCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE DuanzuiCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};


class MojuCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE MojuCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class ShengjianCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE ShengjianCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class CaibaoCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE CaibaoCard();
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class KekediCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE KekediCard();

    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class NuequCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE NuequCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class HongzhaCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE HongzhaCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class BoxueCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE BoxueCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class BianhuaCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE BianhuaCard();
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class YuleiCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE YuleiCard();
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class BadanCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE BadanCard();
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class GuailiCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE GuailiCard();
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class LingjieCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE LingjieCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class XuwuCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE XuwuCard();
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
};

class ReimugiveCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE ReimugiveCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class GongfangCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE GongfangCard();
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class AzuyizhiCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE AzuyizhiCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class YizhiresponseCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE YizhiresponseCard();
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class RandongSummon : public ArraySummonCard
{
    Q_OBJECT

public:
    Q_INVOKABLE RandongSummon();
};

class YuehuangCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE YuehuangCard();
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class HuodanSummon : public ArraySummonCard
{
    Q_OBJECT

public:
    Q_INVOKABLE HuodanSummon();
};

class CongyunSummon : public ArraySummonCard
{
    Q_OBJECT

public:
    Q_INVOKABLE CongyunSummon();
};

class LaoyueCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE LaoyueCard();
    virtual bool targetFixed() const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual const Card *validateInResponse(ServerPlayer *user) const;
    virtual const Card *validate(CardUseStruct &cardUse) const;
};

class GaixieCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE GaixieCard();
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class MaihuoCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE MaihuoCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class ShixianCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE ShixianCard();
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class TiaoyueCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE TiaoyueCard();
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class QiequCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE QiequCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class ZuduiSummon : public ArraySummonCard
{
    Q_OBJECT

public:
    Q_INVOKABLE ZuduiSummon();
};

#endif

