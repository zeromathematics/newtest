#include "maria-battle-scenario.h"
#include "skill.h"
#include "engine.h"
#include "room.h"
#include "banpair.h"

class MariaBattleScenarioRule : public ScenarioRule
{
public:
    MariaBattleScenarioRule(Scenario *scenario)
        : ScenarioRule(scenario)
    {
        events << GameStart;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const
    {
        room->changeBGM( "maria-battle1");
        if (player == NULL)
            foreach(ServerPlayer *p, room->getPlayers()){
                p->showGeneral(true, true, false);
                p->showGeneral(false, true, false);
            }
        return false;
    }
};

MariaBattleScenario::MariaBattleScenario()
    : Scenario("maria_battle")
{
    rule = new MariaBattleScenarioRule(this);
    random_seat = false;
}

void MariaBattleScenario::assign(QStringList &generals, QStringList &generals2, QStringList &kingdoms, Room *room) const
{
    QMap<QString, QStringList> roles;
    QStringList enemy_roles, friend_roles;
    enemy_roles << "puretitan" << "puretitan" << "powertitan";
    friend_roles << "erwin" << "soldier" << "human" << "human" << "human";
    roles.insert("Titan", enemy_roles);
    roles.insert("WOF", friend_roles);
    qShuffle(kingdoms);
    QStringList enemy_generals, friend_generals;
    foreach (const QString &general, Sanguosha->getLimitedGeneralNames()) {
        if (general.startsWith("lord_")) continue;
        if  (BanPair::isBanned(general)) continue;
        QString kingdom = Sanguosha->getGeneral(general)->getKingdom();
        friend_generals << general;

    }
    qShuffle(enemy_generals);
    qShuffle(friend_generals);
    Q_ASSERT(friend_generals.length() >= 10);
    QMap<ServerPlayer *,QString> human_map;
    QMap<ServerPlayer *,QString> general_map;

    QList<ServerPlayer *> players = room->getPlayers();
    QStringList  list;
    list << "Eren" << "Levi" << "Mikasa" << "Armin" << "Hange";
    for (int i = 0; i < 8; i++) {
        if (players[i]->getState() == "online") {
            QStringList choices;
            foreach(const QString &kingdom, roles.keys())
                if (roles[kingdom].contains("human"))
                    choices << kingdom;
            QString choice = choices.at(qrand() % choices.length());
            QStringList role_list = roles[choice];
            role_list.removeOne("human");
            roles[choice] = role_list;
            /*for (int j = 0; j < 5; j++){
              players[i]->addToSelected(friend_generals.takeFirst());
            }*/
            QString answer = room->askForGeneral(players[i], list, QString(), true);
            list.removeOne(answer);
            general_map.insert(players[i], answer);
            human_map.insert(players[i], "WOF");
        }
    }

    QList<ServerPlayer *> humans = human_map.keys();
    //room->chooseGenerals(humans, true, true);

    for (int i = 0; i < 8; i++) {
        if (human_map.contains(players[i])) {
//            QStringList answer = human_map[players[i]];
//            kingdoms << answer.takeFirst();
            QString kingdom = human_map[players[i]]== "WOF"? "science" : "game";
            kingdoms << human_map[players[i]];
            generals << general_map[players[i]];
            generals2 << "white4";
        } else {
            QStringList kingdom_choices;
            foreach(const QString &kingdom, roles.keys())
                if (!roles[kingdom].isEmpty())
                    kingdom_choices << kingdom;
            QString kingdom = kingdom_choices.at(qrand() % kingdom_choices.length());
            QString kingdom1 = kingdom == "WOF"? "science" : "game";
            kingdoms << kingdom;
            QStringList role_list = roles[kingdom];
            QString role = role_list.at(qrand() % role_list.length());
            role_list.removeOne(role);
            roles[kingdom] = role_list;
            if (role == "puretitan") {
                QString name = "Pure_Titan";
                generals << name;
                generals2 << "white10";
            } else if (role == "powertitan") {
                QString name = "Armored_Titan";
                generals << name;
                generals2 << "white15";
            } else if (role == "erwin") {
                QString name = "Erwin";
                generals << name;
                generals2 << "white5";
            } else if (role == "soldier") {
                QString name = "WOF_Soldier";
                generals << name;
                generals2 << "white4";
            } else if (role == "main_soldier") {
                QString name = getRandomMainSoldiers();
                generals << name;
                generals2 << "white4";
            } else if (role == "human") {
                int n = 5;
                /*QStringList choices;
                for (int j = 0; j < n; j++)
                    choices << (friend_generals.takeFirst());
                QString answer = room->askForGeneral(players[i], choices, QString(), false);*/
                QString answer = room->askForGeneral(players[i], list, QString(), true);
                list.removeOne(answer);
                generals << answer;
                generals2 << "white4";
            }
        }
    }

}

int MariaBattleScenario::getPlayerCount() const
{
    return 8;
}

QString MariaBattleScenario::getRoles() const
{
    return "ZNNNNNNN";
}

QString MariaBattleScenario::getRandomMainSoldiers() const
{
    QStringList heros;
    heros << "Eren" << "Levi" << "Mikasa" << "Armin" << "Hange";
    return heros.at(qrand() % heros.length());
}
