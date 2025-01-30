/* System.h
Copyright (c) 2014 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#ifndef SYSTEM_H_
#define SYSTEM_H_

#include "PeriodicEvent.h"
#include "StellarObject.h"

#include <QVector2D>
#include <QString>

#include <map>
#include <optional>
#include <set>
#include <vector>

class DataNode;
class DataWriter;
class Planet;



// Class representing a star system. This includes characteristics like what
// ships enter that system, what asteroids are present, who owns the system, and
// what prices the trade goods have in that system. It also includes the stellar
// objects in each system, and the hyperspace links between systems.
class System {
public:
    struct Asteroid {
        QString type; int count; double energy;
        Asteroid(const QString &type, int count, double energy) : type(type), count(count), energy(energy) {}
    };
    struct Minable {
        QString type; int count; double energy;
        Minable(const QString &type, int count, double energy) : type(type), count(count), energy(energy) {}
    };
    struct Belt {
        Belt(double radius, int weight = 1) : radius(radius), weight(weight) {}
        Belt() = delete;
        double radius;
        int weight;
    };
    struct RaidFleet {
        RaidFleet(QString fleetName, double minimum, double maximum)
            : fleetName(fleetName), minimumAttraction(minimum), maximumAttraction(maximum)
        {
        }
        QString fleetName;
        double minimumAttraction = 2.;
        double maximumAttraction = 0.;
    };


public:
    void Load(const DataNode &node);
    void Save(DataWriter &file) const;

    const QString &TrueName() const;
    bool HasDisplayName() const;
    const QString &DisplayName() const;
    const QVector2D &Position() const;
    const QString &Government() const;

    bool Hidden() const;
    bool Shrouded() const;
    bool Inaccessible() const;

    const std::set<QString> &Links() const;

    double JumpRange() const;
    double HyperArrival() const;
    double JumpArrival() const;
    double HyperDeparture() const;
    double JumpDepature() const;

    std::vector<StellarObject> &Objects();
    const std::vector<StellarObject> &Objects() const;

    // Get the habitable zone's center.
    double HabitableZone() const;
    // Get the radius of the zone occupied by the given stellar object. This
    // zone includes the object and anything that orbits around it. If this
    // object is in orbit around something else, this function returns 0.
    double OccupiedRadius(const StellarObject &object) const;
    double OccupiedRadius() const;
    double StarRadius() const;

    bool HasRamscoopUniversal() const;
    double RamscoopAddend() const;
    double RamscoopMultiplier() const;

    const std::vector<Asteroid> &Asteroids() const;
    std::vector<Minable> &Minables();
    const std::vector<Minable> &Minables() const;

    int Trade(const QString &commodity) const;

    std::vector<PeriodicEvent> &Fleets();
    const std::vector<PeriodicEvent> &Fleets() const;

    std::vector<PeriodicEvent> &Hazards();
    const std::vector<PeriodicEvent> &Hazards() const;

    bool RaidsDisabled() const;
    std::vector<RaidFleet> &RaidFleets();
    const std::vector<RaidFleet> &RaidFleets() const;

    // Position the planets, etc.
    void SetDay(double day);

    // Modify the system:
    void Init(const QString &name, const QVector2D &position);
    void SetTrueName(const QString &name);
    void SetDisplayName(const QString &name);
    void SetPosition(const QVector2D &pos);
    void SetGovernment(const QString &gov);
    void ToggleHidden();
    void ToggleShrouded();
    void ToggleInaccessible();
    void ToggleLink(System *other);
    void ChangeLink(const QString &from, const QString &to);
    void SetJumpRange(double value);
    void SetHyperArrival(double value);
    void SetJumpArrival(double value);
    void SetHyperDeparture(double value);
    void SetJumpDeparture(double value);
    void SetTrade(const QString &commodity, int value);

    void ToggleRamscoopUniversal();
    void SetRamscoopAddend(double value);
    void SetRamscoopMultiplier(double value);

    void ToggleRaids();

    // Editing the stellar objects and their locations:
    void Move(StellarObject *object, double dDistance, double dAngle = 0.);
    void ChangeAsteroids();
    void ChangeMinables();
    void ChangeStar();
    void ChangeSprite(StellarObject *object);
    void AddPlanet();
    void AddMoon(StellarObject *object, bool isStation = false);
    void Randomize(bool allowHabitable, bool requireHabitable);
    void Delete(StellarObject *object);


private:
    void LoadObject(const DataNode &node, int parent = -1);
    void SaveObject(DataWriter &file, const StellarObject &object) const;
    void Recompute(StellarObject &object, bool updateOffset = true);

    // Get a list of all sprites that are in use already.
    std::set<QString> Used() const;


private:
    QString trueName;
    std::optional<QString> displayName;
    QVector2D position;
    QString government;

    std::set<QString> attributes;

    std::set<QString> links;

    double jumpRange = 0.;
    double hyperspaceArrivalDistance = 0.;
    double hyperspaceDepartureDistance = 0.;
    double jumpArrivalDistance = 0.;
    double jumpDepartureDistance = 0.;

    // Stellar objects, listed in such an order that an object's parents are
    // guaranteed to appear before it (so that if we traverse the vector in
    // order, updating positions, an object's parents will already be at the
    // proper position before that object is updated).
    std::vector<StellarObject> objects;

    double habitable;

    bool ramscoopUniversal = true;
    double ramscoopAddend = 0.;
    double ramscoopMultiplier = 1.;

    QString haze;
    double starfieldDensity = 1.;
    QString music;

    std::vector<Asteroid> asteroids;
    std::vector<Minable> minables;
    std::map<QString, int> trade;
    std::vector<PeriodicEvent> fleets;
    std::vector<PeriodicEvent> hazards;
    std::vector<Belt> belts;;

    bool hidden = false;
    bool shrouded = false;
    bool inaccessible = false;

    double invisibleFenceRadius = 10000.;

    std::vector<RaidFleet> raidFleets;
    bool noRaids = false;

    std::list<DataNode> unparsed;
    std::list<DataNode> ramscoopUnparsed;

    // Keep track of the current time step.
    double timeStep;
};



#endif
