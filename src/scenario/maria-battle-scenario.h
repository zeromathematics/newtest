#ifndef MARIABATTLESCENARIO
#define MARIABATTLESCENARIO

#include "scenario.h"



class ServerPlayer;

class MariaBattleScenario : public Scenario
{
    Q_OBJECT

public:
    explicit MariaBattleScenario();

    virtual void assign(QStringList &generals, QStringList &generals2, QStringList &kingdoms, Room *room) const;
    virtual int getPlayerCount() const;
    virtual QString getRoles() const;
    QString getRandomMainSoldiers() const;

};

#endif // MARIABATTLESCENARIO

