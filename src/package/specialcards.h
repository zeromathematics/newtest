#ifndef SPECIALCARDS
#define SPECIALCARDS

#include "package.h"
#include "card.h"
#include "wrappedcard.h"
#include "skill.h"
#include "standard.h"
#include "generaloverview.h"


class SpecialCardPackage : public Package
{
    Q_OBJECT

public:
    SpecialCardPackage();
};

class FadingCardPackage : public Package
{
    Q_OBJECT

public:
    FadingCardPackage();
};

class FadingPackage : public Package
{
    Q_OBJECT

public:
    FadingPackage();
};

class MapoTofu : public BasicCard
{
    Q_OBJECT

public:
    Q_INVOKABLE MapoTofu(Card::Suit suit, int number);
    QString getSubtype() const;

    static bool IsAvailable(const Player *player, const Card *analeptic = NULL);

    bool isAvailable(const Player *player) const;
    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onUse(Room *room, const CardUseStruct &card_use) const;
    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class Tacos : public BasicCard
{
    Q_OBJECT

public:
    Q_INVOKABLE Tacos(Card::Suit suit, int number);
    QString getSubtype() const;

    bool isAvailable(const Player *player) const;
    void onUse(Room *room, const CardUseStruct &card_use) const;
    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class PengtiaoCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE PengtiaoCard();
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class XiangsuiCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE XiangsuiCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class PenglaiCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE PenglaiCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

#endif // SPECIALCARDS

