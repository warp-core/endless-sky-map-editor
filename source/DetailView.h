/* DetailView.h
Copyright (c) 2015 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#ifndef DETAILVIEW_H
#define DETAILVIEW_H

#include <QWidget>

#include <QList>

#include <map>

class GalaxyView;
class Map;
class System;

class QCheckBox;
class QLineEdit;
class QObject;
class QTreeWidget;
class QTreeWidgetItem;



class DetailView : public QWidget
{
    Q_OBJECT
public:
    explicit DetailView(Map &mapData, GalaxyView *galaxyView, QWidget *parent = 0);

    void SetSystem(System *system);
    void UpdateMinables();
    void UpdateCommodities();
    bool eventFilter(QObject* object, QEvent* event);

signals:

public slots:
    void TrueNameEdited();
    void TrueNameChanged();
    void DisplayNameChanged();
    void GovernmentChanged();
    void HiddenClicked();
    void ShroudedClicked();
    void InaccessibleClicked();
    void JumpRangeChanged();
    void HyperArrivalChanged();
    void JumpArrivalChanged();
    void ArrivalFromHabitableClicked();
    void HyperDepartureChanged();
    void JumpDepartureChanged();
    void RamscoopUniversalClicked();
    void RamscoopAddendChanged();
    void RamscoopMultiplierChanged();
    void CommodityClicked(QTreeWidgetItem *item, int column);
    void CommodityChanged(int value);
    void FleetChanged(QTreeWidgetItem *item, int column);
    void MinablesChanged(QTreeWidgetItem *item, int column);
    void HazardChanged(QTreeWidgetItem *item, int column);
    void RaidsDisabledClicked();
    void RaidsCustomClicked();
    void RaidFleetsChanged(QTreeWidgetItem *item, int column);

private:
    void UpdateFleets();
    void UpdateHazards();
    void UpdateRaidFleets();


private:
    Map &mapData;
    GalaxyView *galaxyView = nullptr;
    System *system = nullptr;

    QLineEdit *trueName = nullptr;
    QLineEdit *displayName = nullptr;
    QLineEdit *government = nullptr;

    QCheckBox *hidden = nullptr;
    QCheckBox *shrouded = nullptr;
    QCheckBox *inaccessible = nullptr;

    QCheckBox *arrivalFromHabitable = nullptr;
    QLineEdit *hyperArrival = nullptr;
    QLineEdit *jumpArrival = nullptr;
    QLineEdit *hyperDeparture = nullptr;
    QLineEdit *jumpDeparture = nullptr;
    QLineEdit *jumpRange = nullptr;

    QCheckBox *ramscoopUniversal = nullptr;
    QLineEdit *ramscoopAddend = nullptr;
    QLineEdit *ramscoopMultiplier = nullptr;

    QTreeWidget *tradeWidget = nullptr;
    std::map<QObject *, QTreeWidgetItem *> spinMap;
    QTreeWidget *fleets = nullptr;
    QTreeWidget *minables = nullptr;
    QTreeWidget *hazards = nullptr;
    QCheckBox *raidsDisabled = nullptr;
    QCheckBox *raidsCustom = nullptr;
    QTreeWidget *raidFleets = nullptr;
};



#endif // DETAILVIEW_H
