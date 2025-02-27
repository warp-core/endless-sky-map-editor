/* System.cpp
Copyright (c) 2014 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include "System.h"

#include "DataNode.h"
#include "DataWriter.h"
#include "pi.h"
#include "Planet.h"

#include <QString>

#include <algorithm>
#include <cmath>
#include <limits>
#include <set>

using namespace std;

namespace {
    static const double MIN_GAP = 50.;
    static const int RANDOM_GAP = 100;
    static const double MIN_MOON_GAP = 10.;
    static const int RANDOM_MOON_GAP = 50;
    static const double STAR_MASS_SCALE = .25;
    //static const double PLANET_MASS_SCALE = .015;
    static const double HABITABLE_SCALE = 6.25;

    static const int RANDOM_STAR_DISTANCE = 40;
    static const double MIN_STAR_DISTANCE = 40.;

    struct StarData {
        StarData(QString name, double habitable, double mass) : name(name), habitable(habitable), mass(mass) {}
        StarData() = delete;
        double Mass() const { return mass * 6.25; }
        QString name;
        double habitable;
    private:
        double mass;
    };

    const vector<StarData> stars = {{
        {"o-supergiant", 33450., 33450.}, // 1.5x giant
        {"o-giant", 22300., 22300.}, // 1.5x above the gap between the next two smallest
        {"o0", 13720., 13720.}, // 70
        {"o3", 11500., 11500.}, // ~66
        {"o5", 10000., 10000.}, // ~63
        {"o8", 8650., 8650.}, // ~60
        {"o-dwarf", 1325., 1325.}, // Proportional with o8

        {"b-supergiant", 17025., 17025.}, // 1.5x giant
        {"b-giant", 11350., 11350.}, // 1.5x above the gap between the next two smallest
        {"b0", 7000., 7000.}, // ~56
        {"b3", 6300., 6300.}, // ~54
        {"b5", 5600., 5600.}, // ~52
        {"b8", 5000., 5000.}, // 50
        {"b-dwarf", 1125., 1125.}, // Proportional with b8

        {"a-supergiant", 11850., 11850.}, // 1.5x giant
        {"a-giant", 7900., 7900.}, // 1.5x above the gap between the next two smallest
        {"a0", 3650., 3650.}, // ~45
        {"a3", 3400., 3400.}, // ~44
        {"a5", 3200., 3200.}, // ~43
        {"a8", 3000., 3000.}, // ~42
        {"a-dwarf", 750., 750.}, //  Proportional with a8
        {"a-eater", 1000., 800.},

        {"f-supergiant", 8400., 8400.}, // 1.5x giant
        {"f-giant", 5600., 5600.}, // 1.5x above the gap between the next two smallest
        {"f0", 2560., 2560.}, // 40
        {"f3", 2200., 2200.}, // 38
        {"f5", 1715., 1715.}, // 35
        {"f5-old", 3430., 3430.}, // 2x f5
        {"f8", 1310., 1310.}, // 32
        {"f-dwarf", 355., 355.}, // Proportional with f8

        {"g-supergiant", 6075., 6075.}, // 1.5x giant
        {"g-giant", 4050., 4050.}, // 1.5x above the gap between k-giant and m-giant
        {"g0", 1080., 1080.}, // 30
        {"g0-old", 2160., 2160.}, // 2x g0
        {"g3", 700., 700.}, // 26
        {"g5", 625., 625.}, // 25
        {"g5-old", 1250., 1250.}, // 2x g5
        {"g8", 550., 550.}, // 24
        {"g-dwarf", 150., 150.}, // Proportional with g8

        {"k-supergiant", 4500., 4500.}, // 1.5x giant
        {"k-giant", 3000., 3000.}, // Proportional to k0
        {"k0", 490., 490.}, // ~23
        {"k0-old", 980., 980.}, // 2x k0
        {"k3", 450., 450.}, // ~22.5
        {"k5", 425., 425.}, // ~22
        {"k5-old", 950., 950.}, // 2x k5
        {"k8", 370., 370.}, // ~21
        {"k-dwarf", 100., 100.}, // Proportional with k8

        {"m-supergiant", 3450., 3450.}, // 1.5x giant
        {"m-giant", 2300., 2300.}, // Proportional to m0
        {"m0", 320., 320.}, // ~20
        {"m3", 230., 230.}, // ~18
        {"m5", 160., 160.}, // ~16
        {"m8", 135., 135.}, // 15
        {"m-dwarf", 35., 35.}, // Proportional with m8

        {"l-dwarf", 30., 30.}, // Less than m-dwarf

        {"carbon", 3000., 3000.},
        {"nova", 100., 5000.},
        {"nova-old", 100., 5000.},
        {"nova-small", 100., 5000.},
        {"neutron", 100., 5000.},
        {"neutron-small", 80., 2000.},
        {"magnetar", 120., 5000.},
        {"wr", 50000., 5000.},
        {"black-hole", 100000., 100000.},
        {"small-black-hole", 10000., 10000.},
        {"coal-black-hole", 10000., 10000.},
        {"twilight-black-hole", 100000., 10000.},

        // Use a higher mass than habitable range so that object periods aren't super high.
        {"browndwarf-l-rogue", 10., 20.},
        {"browndwarf-t-rogue", 10., 20.},
        {"browndwarf-y-rogue", 10., 20.},

        // Chopping block
        {"rogue-radiating", 10., 20.},
        {"protostar-orange", 370., 370.},
        {"protostar-yellow", 135., 135.},
        {"giant", 3450., 3450.},

        {"smoke ring", 0., 0.},
        {"void-scar", 0., 0.},

        // Default value for systems without a known star.
        {"default", 100., 100.}
    }};
}



void System::Load(const DataNode &node)
{
    if(node.Size() < 2)
        return;
    trueName = node.Token(1);

    habitable = numeric_limits<double>::quiet_NaN();

    for(const DataNode &child : node)
    {
        if(child.Token(0) == "display name" && child.Size() >= 2)
            displayName = child.Token(1);
        else if(child.Token(0) == "pos" && child.Size() >= 3)
            position = QVector2D(child.Value(1), child.Value(2));
        else if(child.Token(0) == "attributes")
            for(int i = 1; i < child.Size(); ++i)
                attributes.insert(child.Token(i));
        else if(child.Token(0) == "hidden")
            hidden = true;
        else if(child.Token(0) == "shrouded")
            shrouded = true;
        else if(child.Token(0) == "inaccessible")
            inaccessible = true;
        else if(child.Token(0) == "jump range" && child.Size() >= 2)
        {
            jumpRange = max(0., child.Value(1));
        }
        else if(child.Token(0) == "arrival" && (child.Size() >= 2 || child.HasChildren()))
        {
            if(child.Size() >= 2)
            {
                hyperspaceArrivalDistance = child.Value(1);
                jumpArrivalDistance = fabs(hyperspaceArrivalDistance);
            }
            for(const DataNode &grand : child)
            {
                if(grand.Size() < 2)
                    continue;
                if(grand.Token(0) == "link")
                    hyperspaceArrivalDistance = grand.Value(1);
                else if(grand.Token(0) == "jump")
                    jumpArrivalDistance = fabs(grand.Value(1));
            }
        }
        else if(child.Token(0) == "departure" && (child.Size() >= 2 || child.HasChildren()))
        {
            if(child.Size() >= 2)
            {
                hyperspaceDepartureDistance = fabs(child.Value(1));
                jumpDepartureDistance = hyperspaceDepartureDistance;
            }
            for(const DataNode &grand : child)
            {
                if(grand.Size() < 2)
                    continue;
                if(grand.Token(0) == "link")
                    hyperspaceDepartureDistance = fabs(grand.Value(1));
                else if(grand.Token(0) == "jump")
                    jumpDepartureDistance = fabs(grand.Value(1));
            }
        }
        else if(child.Token(0) == "government" && child.Size() >= 2)
            government = child.Token(1);
        else if(child.Token(0) == "ramscoop" && child.HasChildren())
        {
            for(const auto &grand : child)
            {
                bool hasValue = grand.Size() >= 2;
                if(grand.Token(0) == "addend" && hasValue)
                    ramscoopAddend = grand.Value(1);
                else if(grand.Token(0) == "multiplier" && hasValue)
                    ramscoopMultiplier = grand.Value(1);
                else if(grand.Token(0) == "universal" && hasValue)
                {
                    const QString &value = grand.Token(1);
                    ramscoopUniversal = (value == "true" || value == "1");
                }
                else
                    ramscoopUnparsed.emplace_back(grand);
            }
        }
        else if(child.Token(0) == "habitable" && child.Size() >= 2)
            habitable = child.Value(1);
        else if(child.Token(0) == "belt" && child.Size() >= 2)
        {
            if(child.Size() >= 3)
                belts.emplace_back(child.Value(1), child.Value(2));
            else
                belts.emplace_back(child.Value(1));
        }
        else if(child.Token(0) == "haze" && child.Size() >= 2)
            haze = child.Token(1);
        else if(child.Token(0) == "music" && child.Size() >= 2)
            music = child.Token(1);
        else if(child.Token(0) == "link" && child.Size() >= 2)
            links.emplace(child.Token(1));
        else if(child.Token(0) == "asteroids" && child.Size() >= 4)
            asteroids.emplace_back(child.Token(1), static_cast<int>(child.Value(2)), child.Value(3));
        else if(child.Token(0) == "trade" && child.Size() >= 3)
            trade[child.Token(1)] = child.Value(2);
        else if(child.Token(0) == "fleet" && child.Size() >= 3)
            fleets.emplace_back(child.Token(1), static_cast<int>(child.Value(2)));
        else if(child.Token(0) == "minables" && child.Size() >= 3)
            minables.emplace_back(child.Token(1), static_cast<int>(child.Value(2)), child.Value(3));
        else if(child.Token(0) == "hazard" && child.Size() >= 3)
            hazards.emplace_back(child.Token(1), static_cast<int>(child.Value(2)));
        else if(child.Token(0) == "invisible fence" && child.Size() >= 2)
            invisibleFenceRadius = child.Value(1);
        else if(child.Token(0) == "starfield density" && child.Size() >= 2)
            starfieldDensity = child.Value(1);
        else if(child.Token(0) == "object")
            LoadObject(child);
        else
            unparsed.push_back(child);
    }
}



void System::Save(DataWriter &file) const
{
    file.Write("system", trueName);
    file.BeginChild();
    {
        if(hidden)
            file.Write("hidden");
        if(shrouded)
            file.Write("shrouded");
        if(inaccessible)
            file.Write("inaccessible");
        file.Write("pos", position.x(), position.y());
        if(displayName.has_value())
            file.Write("display name", *displayName);
        if(!government.isEmpty())
            file.Write("government", government);
        if(!attributes.empty())
        {
            file.WriteToken("attributes");
            for(const QString &attribute : attributes)
                file.WriteToken(attribute, '"');
            file.Write();
        }
        if(hyperspaceArrivalDistance || jumpArrivalDistance)
        {
            if(hyperspaceArrivalDistance == jumpArrivalDistance)
                file.Write("arrival", hyperspaceArrivalDistance);
            else
            {
                file.Write("arrival");
                file.BeginChild();
                {
                    if(hyperspaceArrivalDistance)
                        file.Write("link", hyperspaceArrivalDistance);
                    if(jumpArrivalDistance)
                        file.Write("jump", jumpArrivalDistance);
                }
                file.EndChild();
            }
        }
        if(hyperspaceDepartureDistance || jumpDepartureDistance)
        {
            if(hyperspaceDepartureDistance == jumpDepartureDistance)
                file.Write("departure", hyperspaceDepartureDistance);
            else
            {
                file.Write("departure");
                file.BeginChild();
                {
                    if(hyperspaceDepartureDistance)
                        file.Write("link", hyperspaceDepartureDistance);
                    if(jumpDepartureDistance)
                        file.Write("jump", jumpDepartureDistance);
                }
                file.EndChild();
            }
        }
        if(!ramscoopUniversal || ramscoopAddend || ramscoopMultiplier != 1. || !ramscoopUnparsed.empty())
        {
            file.Write("ramscoop");
            file.BeginChild();
            {
                if(!ramscoopUniversal)
                    file.Write("universal", "0");
                if(ramscoopAddend)
                    file.Write("addend", ramscoopAddend);
                if(ramscoopMultiplier != 1.)
                    file.Write("multiplier", ramscoopMultiplier);
                for(const auto &it : ramscoopUnparsed)
                    file.Write(it);
            }
            file.EndChild();
        }
        if(!std::isnan(habitable))
            file.Write("habitable", habitable);
        if(belts.size() == 1)
            file.Write("belt", belts.front().radius);
        else
            for(const Belt &belt : belts)
                file.Write("belt", belt.radius, belt.weight);
        if(invisibleFenceRadius != 10000.)
            file.Write("invisible fence", invisibleFenceRadius);
        if(jumpRange && jumpRange != 100.)
            file.Write("jump range", jumpRange);
        if(!haze.isEmpty())
            file.Write("haze", haze);
        if(starfieldDensity != 1.)
            file.Write("starfield density", starfieldDensity);
        if(!music.isEmpty())
            file.Write("music", music);
        for(const QString &it : links)
            file.Write("link", it);
        for(const Asteroid &it : asteroids)
            file.Write("asteroids", it.type, it.count, it.energy);
        for(const Minable &it : minables)
            file.Write("minables", it.type, it.count, it.energy);
        for(const auto &it : trade)
            file.Write("trade", it.first, it.second);
        for(const PeriodicEvent &it : fleets)
            if(!it.name.isEmpty() && it.period)
                file.Write("fleet", it.name, it.period);
        for(const PeriodicEvent &it : hazards)
            if(!it.name.isEmpty() && it.period)
                file.Write("hazard", it.name, it.period);
        for(const DataNode &node : unparsed)
            file.Write(node);
        for(const StellarObject &object : objects)
            SaveObject(file, object);
    }
    file.EndChild();
}



const QString &System::TrueName() const
{
    return trueName;
}



bool System::HasDisplayName() const
{
    return displayName.has_value();
}



const QString &System::DisplayName() const
{
    return displayName.has_value() ? *displayName : trueName;
}



const QVector2D &System::Position() const
{
    return position;
}



const QString &System::Government() const
{
    return government;
}



bool System::Hidden() const
{
    return hidden;
}



bool System::Shrouded() const
{
    return shrouded;
}



bool System::Inaccessible() const
{
    return inaccessible;
}



const set<QString> &System::Links() const
{
    return links;
}



double System::JumpRange() const
{
    return jumpRange;
}



double System::HyperArrival() const
{
    return hyperspaceArrivalDistance;
}



double System::JumpArrival() const
{
    return jumpArrivalDistance;
}



double System::HyperDeparture() const
{
    return hyperspaceDepartureDistance;
}



double System::JumpDepature() const
{
    return jumpDepartureDistance;
}



vector<StellarObject> &System::Objects()
{
    return objects;
}



const vector<StellarObject> &System::Objects() const
{
    return objects;
}



// Get the habitable zone's center.
double System::HabitableZone() const
{
    return habitable;
}



// Get the radius of the zone occupied by the given stellar object. This
// zone includes the object and anything that orbits around it. If this
// object is in orbit around something else, this function returns 0.
double System::OccupiedRadius(const StellarObject &object) const
{
    // Make sure the object is part of this system and is a primary object.
    if(&object < &objects.front() || &object > &objects.back() || object.Parent() >= 0 || object.IsStar())
        return 0.;

    double radius = object.Radius();
    int index = &object - &objects.front();
    for(const StellarObject &other : objects)
        if(other.Parent() == index)
            radius = max(radius, other.Distance() + other.Radius());

    return radius;
}



double System::OccupiedRadius() const
{
    if(objects.empty())
        return 0.;

    double radius = objects.back().Radius() + objects.back().Distance();
    if(objects.back().Parent() >= 0)
        radius += objects[objects.back().Parent()].Distance();

    return radius;
}



double System::StarRadius() const
{
    double radius = 0.;
    for(const StellarObject &other : objects)
    {
        if(!other.IsStar())
            break;
        radius = max(radius, other.Distance() + other.Radius());
    }
    return radius;
}



bool System::HasRamscoopUniversal() const
{
    return ramscoopUniversal;
}



double System::RamscoopAddend() const
{
    return ramscoopAddend;
}



double System::RamscoopMultiplier() const
{
    return ramscoopMultiplier;
}



const vector<System::Asteroid> &System::Asteroids() const
{
    return asteroids;
}



vector<System::Minable> &System::Minables()
{
    return minables;
}



const vector<System::Minable> &System::Minables() const
{
    return minables;
}



int System::Trade(const QString &commodity) const
{
    auto it = trade.find(commodity);
    return (it == trade.end()) ? 0 : it->second;
}



vector<PeriodicEvent> &System::Fleets()
{
    return fleets;
}



const vector<PeriodicEvent> &System::Fleets() const
{
    return fleets;
}



vector<PeriodicEvent> &System::Hazards()
{
    return hazards;
}



const vector<PeriodicEvent> &System::Hazards() const
{
    return hazards;
}



bool System::RaidsDisabled() const
{
    return noRaids;
}



vector<System::RaidFleet> &System::RaidFleets()
{
    return raidFleets;
}



const vector<System::RaidFleet> &System::RaidFleets() const
{
    return raidFleets;
}



// Position the planets, etc.
void System::SetDay(double day)
{
    timeStep = day;
    for(StellarObject &object : objects)
    {
        double angle = (!object.period ? 0. : day * 360. / object.period) + object.offset;
        angle *= TO_RAD;

        object.position = object.distance * QVector2D(sin(angle), -cos(angle));

        // Because of the order of the vector, the parent's position has always
        // been updated before this loop reaches any of its children, so:
        if(object.parent >= 0)
            object.position += objects[object.parent].position;
    }
}



void System::Init(const QString &name, const QVector2D &position)
{
    trueName = name;
    displayName = name;
    this->position = position;

    Randomize(true, false);
    ChangeAsteroids();
    ChangeMinables();
}



void System::SetTrueName(const QString &name)
{
    trueName = name;
}



void System::SetDisplayName(const QString &name)
{
    displayName = name;
}



void System::SetPosition(const QVector2D &pos)
{
    position = pos;
}



void System::SetGovernment(const QString &gov)
{
    government = gov;
}



void System::ToggleHidden()
{
    hidden = !hidden;
}



void System::ToggleShrouded()
{
    shrouded = !shrouded;
}



void System::ToggleInaccessible()
{
    inaccessible = !inaccessible;
}



void System::ToggleLink(System *other)
{
    if(!other || other == this)
        return;

    if(links.erase(other->trueName))
        other->links.erase(trueName);
    else
    {
        // These systems were not linked, so link them.
        links.emplace(other->trueName);
        other->links.emplace(trueName);
    }
}



// Change the name of a linked System. If the new name is empty, this
// effectively deletes the link.
void System::ChangeLink(const QString &from, const QString &to)
{
    if(links.erase(from) && !to.isEmpty())
        links.emplace(to);
}



void System::SetJumpRange(double value)
{
    jumpRange = value;
}



void System::SetHyperArrival(double value)
{
    hyperspaceArrivalDistance = value;
}



void System::SetJumpArrival(double value)
{
    jumpArrivalDistance = value;
}



void System::SetHyperDeparture(double value)
{
    hyperspaceDepartureDistance = value;
}



void System::SetJumpDeparture(double value)
{
    jumpDepartureDistance = value;
}



void System::SetTrade(const QString &commodity, int value)
{
    trade[commodity] = value;
}



void System::ToggleRamscoopUniversal()
{
    ramscoopUniversal = !ramscoopUniversal;
}



void System::SetRamscoopAddend(double value)
{
    ramscoopAddend = value;
}



void System::SetRamscoopMultiplier(double value)
{
    ramscoopMultiplier = value;
}



void System::ToggleRaids()
{
    noRaids = !noRaids;
}



void System::Move(StellarObject *object, double dDistance, double dAngle)
{
    if(!object || !object->period || object->IsStar())
        return;

    // Find the next object in from this object. Determine what the orbital
    // radius of that object is. Don't allow objects too close together.
    auto it = objects.begin() + (object - &objects.front());
    auto root = (it->Parent() >= 0 ? objects.begin() + it->Parent() : it);
    if(root != objects.begin())
    {
        auto previous = root - 1;
        while(previous != objects.begin() && previous->Parent() >= 0)
            --previous;

        double previousOccupied = previous->IsStar() ?
            StarRadius() : (OccupiedRadius(*previous) + previous->Distance());
        double rootOccupied = root->Distance() - OccupiedRadius(*root);

        double gap = rootOccupied - previousOccupied;
        double sign = (it == root ? 1. : -1.);
        gap += sign * dDistance;
        if(gap < MIN_GAP)
            dDistance += sign * (MIN_GAP - gap);
    }
    // If this is a moon, we also have to make sure it does not collide with
    // whatever planet is in one space from it.
    if(object->Parent() >= 0)
    {
        double previousOccupied = it[-1].Radius();
        if(it[-1].Parent() >= 0)
            previousOccupied += it[-1].Distance();

        double thisOccupied = it->Distance() - it->Radius();
        double gap = thisOccupied + dDistance - previousOccupied;
        if(gap < MIN_MOON_GAP)
            dDistance += MIN_MOON_GAP - gap;
    }

    object->offset -= dAngle;

    // If a root object is moved, every root object beyond it must be moved in
    // or out by whatever amount its radius changed by. If a child object was
    // moved, all the children after it must be moved by that amount and then
    // all root objects after it must be moved by twice that amount.
    for( ; it != objects.end(); ++it)
        if(it->Parent() < 0 || it->Parent() == object->Parent())
        {
            it->distance += dDistance;
            Recompute(*it);
        }
}



void System::ChangeAsteroids()
{
    asteroids.clear();

    // Pick the total number of asteroids. Bias towards small numbers, with
    // a few systems with many more.
    int fullTotal = (rand() % 21) * (rand() % 21) + 1;
    double energy = (rand() % 21 + 10) * (rand() % 21 + 10) * .01;
    const QString suffix[2] = {" rock", " metal"};
    const QString prefix[3] = {"small", "medium", "large"};

    int total[2] = {rand() % fullTotal, 0};
    total[1] = fullTotal - total[0];

    for(int i = 0; i < 2; ++i)
    {
        if(!total[i])
            continue;

        int count[3] = {0, rand() % total[i], 0};
        int remaining = total[i] - count[1];
        if(remaining)
        {
            count[0] = rand() % remaining;
            count[2] = remaining - count[0];
        }

        for(int j = 0; j < 3; ++j)
            if(count[j])
                asteroids.emplace_back(
                    prefix[j] + suffix[i],
                    count[j],
                    energy * (rand() % 101 + 50) * .01);
    }
}



void System::ChangeMinables()
{
    // First, change the belt radius.
    belts.clear();
    belts.emplace_back(rand() % 1000 + 1000);
    minables.clear();

    // Next, figure out the quantity and energy of the ordinary asteroids.
    int totalCount = 0;
    double totalEnergy = 0.;
    for(const Asteroid &asteroid : asteroids)
    {
        totalCount += asteroid.count;
        totalEnergy += asteroid.energy * asteroid.count;
    }

    // Do not auto-create systems with only minable asteroids.
    if(!totalCount)
        return;

    double meanEnergy = totalEnergy / totalCount;
    // Minables are much less common than ordinary asteroids.
    totalCount /= 4;

    map<QString, double> probability = {
        {"aluminum",  12},
        {"copper",  8},
        {"gold",  2},
        {"iron",  13},
        {"lead",  15},
        {"neodymium",  3},
        {"platinum",  1},
        {"silicon",  2},
        {"silver",  5},
        {"titanium",  11},
        {"tungsten",  6},
        {"uranium",  4}
    };
    map<QString, int> choices;
    for(int i = 0; i < 3; ++i)
    {
        // Pick three random minable types, with decreasing quantities.
        totalCount = rand() % (totalCount + 1);
        if(!totalCount)
            break;

        int choice = rand() % 100;
        for(const auto &it : probability)
        {
            choice -= it.second;
            if(choice < 0)
            {
                choices[it.first] += totalCount;
                break;
            }
        }
    }
    for(const auto &it : choices)
    {
        double energy = (rand() % 1000 + 1000) * .001 * meanEnergy;
        minables.emplace_back(it.first, it.second, energy);
    }
}



void System::ChangeStar()
{
    double oldStarRadius = StarRadius();
    unsigned oldStars = 0;
    while(!objects.empty() && objects.front().IsStar())
    {
        objects.erase(objects.begin());
        ++oldStars;
    }

    // If the number of stars is changing, all parent indices change.
    unsigned stars = 1 + !(rand() % 3);
    if(stars != oldStars)
        for(StellarObject &object : objects)
            if(object.parent >= 0)
                object.parent += stars - oldStars;

    double mass = 0.;
    if(stars == 1)
    {
        StellarObject star = StellarObject::Star();
        star.system = this;
        star.period = 10.;
        mass = pow(star.Radius(), 3.) * STAR_MASS_SCALE;

        objects.insert(objects.begin(), star);
    }
    else
    {
        StellarObject first = StellarObject::Star();
        StellarObject second = StellarObject::Star();
        first.system = this;
        second.system = this;
        first.offset = 0.;
        second.offset = 180.;

        double firstR = first.Radius();
        double secondR = second.Radius();
        double firstMass = pow(firstR, 3.) * STAR_MASS_SCALE;
        double secondMass = pow(secondR, 3.) * STAR_MASS_SCALE;
        mass = firstMass + secondMass;

        double distance = firstR + secondR + (rand() % RANDOM_STAR_DISTANCE) + MIN_STAR_DISTANCE;
        // m1 * d1 = m2 * d2
        // d1 + d2 = d;
        // m1 * d1 = m2 * (d - d1)
        // m1 * d1 = m2 * d - m2 * d1
        // (m1 + m2) * d1 = m2 * d
        double firstD = (secondMass * distance) / mass;
        double secondD = (firstMass * distance) / mass;
        first.distance = firstD;
        second.distance = secondD;

        double period = sqrt(pow(distance, 3.) / mass);
        first.period = period;
        second.period = period;

        objects.insert(objects.begin(), (firstD < secondD) ? second : first);
        objects.insert(objects.begin(), (firstD < secondD) ? first : second);
    }
    habitable = mass / HABITABLE_SCALE;

    if(objects.size() > stars)
    {
        double newStarRadius = StarRadius();
        Move(&objects[stars], newStarRadius - oldStarRadius);
    }
}



void System::ChangeSprite(StellarObject *object)
{
    if(!object || object < &objects.front() || object > &objects.back())
        return;

    StellarObject newObject;
    set<QString> used = Used();
    do {
        if(object->IsStation())
            newObject = StellarObject::Station();
        else if(object->IsMoon())
            newObject = StellarObject::Moon();
        else if(object->IsGiant())
            newObject = StellarObject::Giant();
        else
        {
            double distance = (object->Parent() >= 0 ? objects[object->Parent()].Distance() : object->Distance());
            if(distance >= .5 * habitable && distance < 2. * habitable)
                newObject = StellarObject::Planet();
            else
                newObject = StellarObject::Uninhabited();
        }
    } while(used.count(newObject.Sprite()));

    // Check how much the radius will change by, then change the sprite.
    double radiusChange = newObject.Radius() - object->Radius();
    object->sprite = newObject.sprite;

    // If this object has a parent:
    // this distance += dRadius
    // children after distance += 2 * dRadius
    // parent distance += 2 * dRadius
    // after distance += 2 * dRadius
    // Otherwise:
    // this distance += dRadius
    // child distance += dRadius
    // after distance += 2 * dRadius

    // Get the index of this object, for checking if other objects are children.
    //int index = object - &objects.front();

    // Get an iterator to this object.
    auto it = objects.begin() + (object - &*objects.begin());
    // Move this object out by an amount equal to the radius change.
    it->distance += radiusChange;
    Recompute(*it);

    // If this object has a parent, the parent's occupied radius will be
    // expanding by twice the radius change, so it must move out by that amount.
    // In addition, root objects beyond this one's parent will be moving out by
    // four times the radius change, instead of two.
    if(object->Parent() >= 0)
    {
        radiusChange *= 2.;
        objects[object->Parent()].distance += radiusChange;
        Recompute(objects[object->Parent()]);
    }
    // The objects that will be affected are: children of this object, and all
    // "root" objects outside of this one.
    bool isChild = true;
    for(++it; it != objects.end(); ++it)
    {
        if(isChild && it->Parent() < 0)
        {
            isChild = false;
            radiusChange *= 2.;
        }
        if(isChild || it->Parent() < 0)
        {
            it->distance += radiusChange;
            Recompute(*it);
        }
    }
}



void System::AddPlanet()
{
    // The spacing between planets grows exponentially.
    int randomPlanetSpace = RANDOM_GAP;
    for(const StellarObject &object : objects)
        if(!object.IsStar() && object.Parent() < 0)
            randomPlanetSpace += randomPlanetSpace / 2;

    double distance = OccupiedRadius();
    int space = rand() % randomPlanetSpace;
    distance += (space * space) * .01 + MIN_GAP;

    set<QString> used = Used();

    StellarObject root;
    int rootIndex = static_cast<int>(objects.size());

    bool isHabitable = (distance > habitable * .5 && distance < habitable * 2. - 120.);
    bool isSmall = !(rand() % 10);
    bool isTerrestrial = !isSmall && (rand() % 2000 > distance);

    // Occasionally, moon-sized objects can be root objects. Otherwise, pick a
    // giant or a normal planet, with giants more frequent in the outer parts
    // of the solar system.
    do {
        if(isSmall)
            root = StellarObject::Moon();
        else if(isTerrestrial)
            root = isHabitable ? StellarObject::Planet() : StellarObject::Uninhabited();
        else
            root = StellarObject::Giant();
    } while(used.count(root.Sprite()));
    objects.push_back(root);
    objects.back().system = this;
    used.insert(root.Sprite());

    int moonCount = rand() % (isTerrestrial ? (rand() % 2 + 1) : (rand() % 3 + 3));
    if(root.Radius() < 70)
        moonCount = 0;

    double moonDistance = root.Radius();
    int randomMoonSpace = RANDOM_MOON_GAP;
    for(int i = 0; i < moonCount; ++i)
    {
        moonDistance += rand() % randomMoonSpace + MIN_MOON_GAP;
        // Each moon, on average, should be spaced more widely than the one before.
        randomMoonSpace += 20;

        // Use a moon sprite only once per system.
        StellarObject moon;
        do {
            moon = StellarObject::Moon();
        } while(used.count(moon.Sprite()));
        used.insert(moon.Sprite());

        moon.distance = moonDistance + moon.Radius();
        moon.parent = rootIndex;
        Recompute(moon, false);
        objects.push_back(moon);
        objects.back().system = this;
        moonDistance += 2. * moon.Radius();
    }
    objects[rootIndex].distance = distance + moonDistance;
    Recompute(objects[rootIndex], false);
}



void System::AddMoon(StellarObject *object, bool isStation)
{
    if(!object || object < &objects.front() || object > &objects.back())
        return;

    double originalMoonDistance = object->Radius();
    int randomMoonSpace = RANDOM_MOON_GAP;
    int rootIndex = object - &objects.front();
    auto it = objects.begin() + rootIndex + 1;
    while(it != objects.end() && it->Parent() == rootIndex)
    {
        randomMoonSpace += 20;
        originalMoonDistance = it->Distance() + it->Radius();
        ++it;
    }

    double moonDistance = originalMoonDistance + rand() % randomMoonSpace + MIN_MOON_GAP;
    set<QString> used = Used();
    StellarObject moon;
    do {
        moon = isStation ? StellarObject::Station() : StellarObject::Moon();
    } while(used.count(moon.Sprite()));

    moon.distance = moonDistance + moon.Radius();
    moon.parent = rootIndex;
    Recompute(moon, false);

    // Move the next root planet out from this one farther out.
    double distanceIncrease = moonDistance + 2. * moon.Radius() - originalMoonDistance;
    if(it != objects.end())
        Move(&*it, 2. * distanceIncrease);

    // Move this root planet farther out.
    objects[rootIndex].distance += distanceIncrease;
    Recompute(objects[rootIndex]);

    // Insert the new moon, then update the parent indices of all moons farther
    // out than this one (because their parents' indices have changed).
    it = objects.insert(it, moon);
    it->system = this;
    for( ; it != objects.end(); ++it)
        if(it->parent > rootIndex)
            ++it->parent;
}



void System::Randomize(bool allowHabitable, bool requireHabitable)
{
    // Try to create a system satisfying the given parameters.
    for(int i = 0; i < 100; ++i)
    {
        objects.clear();
        ChangeStar();
        while(OccupiedRadius() < 2000.)
            AddPlanet();

        bool isInhabited = false;
        bool isHabitable = false;
        for(const StellarObject &object : objects)
        {
            isInhabited |= object.IsInhabited();
            double d = object.Distance();
            isHabitable |= object.Parent() < 0 && object.IsTerrestrial()
                && d > .5 * habitable && d < 2. * habitable;
        }
        if(isInhabited && !allowHabitable)
            continue;
        if(!isHabitable && requireHabitable)
            continue;
        break;
    }
    UpdateObjectPointers();
}



void System::Delete(StellarObject *object)
{
    if(!object || objects.empty())
        return;

    int index = object - &objects.front();
    if(index < 0 || static_cast<unsigned>(index) >= objects.size())
        return;

    double shrink = object->Radius();

    auto it = objects.begin() + index;
    auto end = it + 1;
    while(end != objects.end() && end->Parent() == index)
    {
        shrink = max(shrink, end->Distance() + end->Radius());
        ++end;
    }
    int parentShift = end - it;
    objects.erase(it, end);

    it = objects.begin() + index;
    if(it == objects.end())
        return;

    Move(&*it, -2. * shrink);
    for( ; it != objects.end(); ++it)
        if(it->parent >= 0)
            it->parent -= parentShift;
}



void System::UpdateObjectPointers()
{
    for(StellarObject &object : objects)
        object.system = this;
}



void System::LoadObject(const DataNode &node, int parent)
{
    int index = static_cast<int>(objects.size());

    objects.emplace_back(parent);
    StellarObject &object = objects.back();
    object.system = this;

    if(node.Size() >= 2)
        object.planet = node.Token(1);

    for(const DataNode &child : node)
    {
        if(child.Token(0) == "sprite" && child.Size() >= 2)
        {
            object.sprite = child.Token(1);
            for(const DataNode &grand : child)
                object.spriteProperties.emplace_back(grand);
        }
        else if(child.Token(0) == "distance" && child.Size() >= 2)
            object.distance = child.Value(1);
        else if(child.Token(0) == "period" && child.Size() >= 2)
            object.period = child.Value(1);
        else if(child.Token(0) == "offset" && child.Size() >= 2)
            object.offset = child.Value(1);
        else if(child.Token(0) == "object")
            LoadObject(child, index);
        else
            object.unparsed.push_back(child);
    }
}



void System::SaveObject(DataWriter &file, const StellarObject &object) const
{
    int level = 0;
    int parent = object.parent;
    while(parent >= 0)
    {
        file.BeginChild();
        ++level;
        parent = objects[parent].parent;
    }
    if(!object.planet.isEmpty())
        file.Write("object", object.planet);
    else
        file.Write("object");
    file.BeginChild();
    {
        if(!object.sprite.isEmpty())
        {
            file.Write("sprite", object.sprite);
            file.BeginChild();
            {
                for(const DataNode &property : object.spriteProperties)
                    file.Write(property);
            }
            file.EndChild();
        }
        if(object.distance)
            file.Write("distance", object.distance);
        if(object.period)
        {
            QString result = QString::number(object.period, 'g', 13);
            if(result.contains('.') && result.size() > 7)
            {
                result.resize(7);
                if(result.back() == '.')
                    result.resize(6);
            }
            file.Write("period", result);
        }
        if(object.offset)
            file.Write("offset", object.offset);
        for(const DataNode &node : object.unparsed)
            file.Write(node);
    }
    file.EndChild();
    while(level--)
        file.EndChild();
}



void System::Recompute(StellarObject &object, bool updateOffset)
{
    if(object.sprite.startsWith("star/") || object.sprite.contains("rogue"))
        return;
    double mass = 0.;
    for(const auto &stellar : objects)
    {
        if((stellar.sprite.startsWith("star/") && !stellar.sprite.contains("-core")) || stellar.sprite.contains("rogue"))
        {
            const QString name = stellar.sprite.section('/', 1);
            const auto it = std::find_if(stars.begin(), stars.end(),
                [&name](const StarData &it) -> bool { return it.name == name; });
            const StarData &star = (it == stars.end() ? stars.back() : *it);
            mass += star.Mass();
        }
    }
    if(object.Parent() >= 0.)
        return; //mass = pow(objects[object.Parent()].Radius(), 3.) * PLANET_MASS_SCALE;

    double d = object.distance;
    if(object.sprite.contains("panel"))
        d = 812.;
    double newPeriod = sqrt(pow(d, 3) / mass);
    if(updateOffset)
    {
        double delta = timeStep / object.period - timeStep / newPeriod;
        object.offset += 360. * (delta - floor(delta));
        object.offset = fmod(object.offset, 360.);
    }
    object.period = newPeriod;
}



set<QString> System::Used() const
{
    set<QString> used;
    for(const StellarObject &object : objects)
        used.insert(object.Sprite());
    return used;
}
