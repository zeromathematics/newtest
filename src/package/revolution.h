
#ifndef _REVOLUTION_H
#define _REVOLUTION_H

#include "package.h"
#include "card.h"
#include "wrappedcard.h"
#include "skill.h"
#include "standard.h"
#include "generaloverview.h"

class RevolutionPackage : public Package
{
    Q_OBJECT

public:
    RevolutionPackage();
};

class RevolutionCardPackage : public Package
{
    Q_OBJECT

public:
    RevolutionCardPackage();
};

class GuangyuCard : public BasicCard
{
    Q_OBJECT

public:
    Q_INVOKABLE GuangyuCard(Card::Suit suit, int number, bool is_transferable = false);
    virtual QString getSubtype() const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
    virtual bool isAvailable(const Player *player) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
};

class Eireishoukan : public SingleTargetTrick
{
    Q_OBJECT

public:
    Q_INVOKABLE Eireishoukan(Card::Suit suit, int number);

    virtual QString getSubtype() const;

    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    //virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
    virtual bool isAvailable(const Player *player) const;
};

class Isekai : public SingleTargetTrick
{
    Q_OBJECT

public:
    Q_INVOKABLE Isekai(Card::Suit suit, int number);

    virtual QString getSubtype() const;

    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    //virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
    virtual bool isAvailable(const Player *player) const;
};

class RulerCard : public AOE
{
    Q_OBJECT

public:
    Q_INVOKABLE RulerCard(Card::Suit suit, int number, bool is_transferable = false);

    virtual bool isAvailable(const Player *player) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class XingbaoCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE XingbaoCard();
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
};

class ShifengCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE ShifengCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class ZhiyanCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE ZhiyanCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void extraCost(Room *room, const CardUseStruct &card_use) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

int shisoPower(Card* card);

class ShisoCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE ShisoCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class FanqianCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE FanqianCard();

    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class PoyiCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE PoyiCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class ChichengCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE ChichengCard();

    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class YaozhanCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE YaozhanCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class PoxiaoCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE PoxiaoCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class PoshiCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE PoshiCard();

    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class JiuzhuCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE JiuzhuCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class ShexinCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE ShexinCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class ZhouliCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE ZhouliCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class HuanshiCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE HuanshiCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void extraCost(Room *room, const CardUseStruct &card_use) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class ShoujiCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE ShoujiCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class ModanCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE ModanCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class ShashouCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE ShashouCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class DaokegiveCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE DaokegiveCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class ChongzuCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE ChongzuCard();
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

#endif
