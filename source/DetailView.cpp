/* DetailView.cpp
Copyright (c) 2015 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include "DetailView.h"

#include "GalaxyView.h"
#include "Map.h"
#include "System.h"

#include <QCheckBox>
#include <QEvent>
#include <QMessageBox>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QString>
#include <QTreeWidget>
#include <QVBoxLayout>

using namespace std;

namespace {
    double GetOptionalValue(const QString &text)
    {
        return text.isEmpty() ? numeric_limits<double>::quiet_NaN() : text.toDouble();
    }
}



DetailView::DetailView(Map &mapData, GalaxyView *galaxyView, QWidget *parent) :
    QWidget(parent), mapData(mapData), galaxyView(galaxyView)
{
    // Create the left sidebar, showing details about the selected system.
    QVBoxLayout *layout = new QVBoxLayout(this);


    QHBoxLayout *trueNameRow = new QHBoxLayout(this);
    trueNameRow->addWidget(new QLabel("System Name:", this));
    trueName = new QLineEdit(this);
    connect(trueName, SIGNAL(textChanged(const QString &)), this, SLOT(TrueNameEdited()));
    connect(trueName, SIGNAL(editingFinished()), this, SLOT(TrueNameChanged()));
    trueNameRow->addWidget(trueName);
    layout->addLayout(trueNameRow);

    QHBoxLayout *displayNameRow = new QHBoxLayout(this);
    useDisplayName = new QCheckBox("Display name:", this);
    connect(useDisplayName, SIGNAL(clicked()), this, SLOT(UseDisplayNameChanged()));
    displayNameRow->addWidget(useDisplayName);
    displayName = new QLineEdit(this);
    connect(displayName, SIGNAL(editingFinished()), this, SLOT(DisplayNameChanged()));
    displayNameRow->addWidget(displayName);
    layout->addLayout(displayNameRow);

    QHBoxLayout *governmentRow = new QHBoxLayout(this);
    governmentRow->addWidget(new QLabel("Government:", this));
    government = new QLineEdit(this);
    connect(government, SIGNAL(editingFinished()), this, SLOT(GovernmentChanged()));
    governmentRow->addWidget(government);
    // Selecting the government field changes the system and link colors on the Galaxy map.
    government->installEventFilter(this);
    layout->addLayout(governmentRow);

    QHBoxLayout *visibilityLayout = new QHBoxLayout(this);
    hidden = new QCheckBox("Hidden", this);
    connect(hidden, SIGNAL(clicked()), this, SLOT(HiddenClicked()));
    visibilityLayout->addWidget(hidden);
    shrouded = new QCheckBox("Shrouded", this);
    connect(shrouded, SIGNAL(clicked()), this, SLOT(ShroudedClicked()));
    visibilityLayout->addWidget(shrouded);
    inaccessible = new QCheckBox("Inaccessible", this);
    connect(inaccessible, SIGNAL(clicked()), this, SLOT(InaccessibleClicked()));
    visibilityLayout->addWidget(inaccessible);
    layout->addLayout(visibilityLayout);

    QGridLayout *interstellarTravelLayout = new QGridLayout(this);
    interstellarTravelLayout->addWidget(new QLabel("Arrival:", this), 0, 1, 1, 1);
    interstellarTravelLayout->addWidget(new QLabel("Departure:", this), 0, 2, 1, 1);
    interstellarTravelLayout->addWidget(new QLabel("Hyperspace:", this), 1, 0, 1, 1);
    interstellarTravelLayout->addWidget(new QLabel("Jump drive:", this), 2, 0, 1, 1);
    interstellarTravelLayout->addWidget(new QLabel("Jump range:", this), 3, 1, 1, 1);
    hyperArrival = new QLineEdit(this);
    hyperArrival->setValidator(new QRegularExpressionValidator(QRegularExpression("-?\\d*\\.?\\d*"), hyperArrival));
    connect(hyperArrival, SIGNAL(editingFinished()), this, SLOT(HyperArrivalChanged()));
    interstellarTravelLayout->addWidget(hyperArrival, 1, 1, 1, 1);
    jumpArrival = new QLineEdit(this);
    jumpArrival->setValidator(new QRegularExpressionValidator(QRegularExpression("\\d*\\.?\\d*"), jumpArrival));
    connect(jumpArrival, SIGNAL(editingFinished()), this, SLOT(JumpArrivalChanged()));
    interstellarTravelLayout->addWidget(jumpArrival, 2, 1, 1, 1);
    hyperDeparture = new QLineEdit(this);
    hyperDeparture->setValidator(new QRegularExpressionValidator(QRegularExpression("\\d*\\.?\\d*"), hyperDeparture));
    connect(hyperDeparture, SIGNAL(editingFinished()), this, SLOT(HyperDepartureChanged()));
    interstellarTravelLayout->addWidget(hyperDeparture, 1, 2, 1, 1);
    jumpDeparture = new QLineEdit(this);
    jumpDeparture->setValidator(new QRegularExpressionValidator(QRegularExpression("\\d*\\.?\\d*"), jumpDeparture));
    connect(jumpDeparture, SIGNAL(editingFinished()), this, SLOT(JumpDepartureChanged()));
    interstellarTravelLayout->addWidget(jumpDeparture, 2, 2, 1, 1);
    jumpRange = new QLineEdit(this);
    jumpRange->setValidator(new QRegularExpressionValidator(QRegularExpression("\\d*\\.?\\d*"), jumpRange));
    connect(jumpRange, SIGNAL(editingFinished()), this, SLOT(JumpRangeChanged()));
    interstellarTravelLayout->addWidget(jumpRange, 3, 2, 1, 1);
    arrivalFromHabitable = new QCheckBox("Arrival from habitable", this);
    connect(arrivalFromHabitable, SIGNAL(clicked()), this, SLOT(ArrivalFromHabitableClicked()));
    interstellarTravelLayout->addWidget(arrivalFromHabitable, 3, 0, 1, 1);
    layout->addLayout(interstellarTravelLayout);


    QGridLayout *ramscoopLayout = new QGridLayout(this);
    ramscoopLayout->addWidget(new QLabel("Ramscoop:", this), 0, 0);
    ramscoopUniversal = new QCheckBox("Universal", this);
    connect(ramscoopUniversal, SIGNAL(clicked()), this, SLOT(RamscoopUniversalClicked()));
    ramscoopLayout->addWidget(ramscoopUniversal, 1, 0);
    ramscoopLayout->addWidget(new QLabel("Addend:", this), 0, 1);
    ramscoopAddend = new QLineEdit(this);
    ramscoopAddend->setValidator(new QRegularExpressionValidator(QRegularExpression("-?\\d*\\.?\\d*"), ramscoopAddend));
    connect(ramscoopAddend, SIGNAL(editingFinished()), this, SLOT(RamscoopAddendChanged()));
    ramscoopLayout->addWidget(ramscoopAddend, 0, 2);
    ramscoopLayout->addWidget(new QLabel("Multiplier:", this), 1, 1);
    ramscoopMultiplier = new QLineEdit(this);
    ramscoopMultiplier->setValidator(new QRegularExpressionValidator(QRegularExpression("-?\\d*\\.?\\d*"), ramscoopMultiplier));
    connect(ramscoopMultiplier, SIGNAL(editingFinished()), this, SLOT(RamscoopMultiplierChanged()));
    ramscoopLayout->addWidget(ramscoopMultiplier, 1, 2);
    layout->addLayout(ramscoopLayout);


    // Add a table to display this system's default trade.
    tradeWidget = new QTreeWidget(this);
    tradeWidget->setMinimumHeight(250);
    tradeWidget->setIndentation(0);
    tradeWidget->setColumnCount(3);
    tradeWidget->setHeaderLabels({"Commodity", "Price", ""});
    // Selecting a commodity field changes the system and link colors on the Galaxy map.
    connect(tradeWidget, SIGNAL(itemClicked(QTreeWidgetItem *, int)),
        this, SLOT(CommodityClicked(QTreeWidgetItem *, int)));
    layout->addWidget(tradeWidget);


    // Add a table to display this system's fleets.
    fleets = new QTreeWidget(this);
    fleets->setIndentation(0);
    fleets->setColumnCount(2);
    fleets->setHeaderLabels({"Fleet Name", "Period"});
    fleets->setColumnWidth(0, 200);
    fleets->setColumnWidth(1, 80);
    layout->addWidget(fleets);


    // Add a table to display this system's minables (if any).
    minables = new QTreeWidget(this);
    minables->setIndentation(0);
    minables->setColumnCount(3);
    minables->setHeaderLabels({"Minable", "Count", "Energy"});
    minables->setColumnWidth(0, 140);
    minables->setColumnWidth(1, 70);
    minables->setColumnWidth(2, 70);
    layout->addWidget(minables);


    hazards = new QTreeWidget(this);
    hazards->setIndentation(0);
    hazards->setColumnCount(2);
    hazards->setHeaderLabels({"Hazard Name", "Period"});
    hazards->setColumnWidth(0, 200);
    hazards->setColumnWidth(1, 80);
    layout->addWidget(hazards);


    QHBoxLayout *raidsCheckBoxes = new QHBoxLayout(this);
    raidsDisabled = new QCheckBox("Disable raids", this);
    connect(raidsDisabled, SIGNAL(clicked()), this, SLOT(RaidsDisabledClicked()));
    raidsCheckBoxes->addWidget(raidsDisabled);
    raidsCustom = new QCheckBox("Use custom raid fleets", this);
    connect(raidsCustom, SIGNAL(clicked()), this, SIGNAL(RaidsCustomClicked()));
    raidsCheckBoxes->addWidget(raidsCustom);
    layout->addLayout(raidsCheckBoxes);

    raidFleets = new QTreeWidget(this);
    raidFleets->setIndentation(0);
    raidFleets->setColumnCount(3);
    raidFleets->setHeaderLabels({"Raid fleet name", "Minimum attraction", "Maximum attraction"});
    raidFleets->setColumnWidth(0, 160);
    raidFleets->setColumnWidth(1, 60);
    raidFleets->setColumnWidth(2, 60);
    layout->addWidget(raidFleets);


    setLayout(layout);
}



void DetailView::SetSystem(System *system)
{
    if(system == this->system)
        return;

    this->system = system;
    if(system)
    {
        trueName->setText(system->TrueName());
        if(system->HasDisplayName())
        {
            useDisplayName->setCheckState(Qt::CheckState::Checked);
            displayName->setText(system->DisplayName());
            displayName->setReadOnly(false);
        }
        else
        {
            useDisplayName->setCheckState(Qt::CheckState::Unchecked);
            displayName->clear();
            displayName->setReadOnly(true);
        }
        government->setText(system->Government());

        hidden->setChecked(system->Hidden());
        shrouded->setChecked(system->Shrouded());
        inaccessible->setChecked(system->Inaccessible());

        if((system->HyperArrival() == system->JumpArrival()) && (system->HyperArrival() == system->HabitableZone()))
        {
            QString distance = QString::number(system->HabitableZone());
            arrivalFromHabitable->setChecked(true);
            hyperArrival->setReadOnly(true);
            hyperArrival->setText(distance);
            jumpArrival->setReadOnly(true);
            jumpArrival->setText(distance);
        }
        else
        {
            arrivalFromHabitable->setChecked(false);
            hyperArrival->setReadOnly(false);
            hyperArrival->setText(QString::number(system->HyperArrival()));
            jumpArrival->setReadOnly(false);
            jumpArrival->setText(QString::number(system->JumpArrival()));
        }
        hyperDeparture->setText(QString::number(system->HyperDeparture()));
        jumpDeparture->setText(QString::number(system->JumpDepature()));
        jumpRange->setText(QString::number(system->JumpRange()));
        jumpRange->setPlaceholderText("100");

        ramscoopUniversal->setChecked(system->HasRamscoopUniversal());
        ramscoopAddend->setText(QString::number(system->RamscoopAddend()));
        ramscoopMultiplier->setText(QString::number(system->RamscoopMultiplier()));

        UpdateCommodities();
        UpdateFleets();
        UpdateMinables();
        UpdateHazards();
        raidsDisabled->setChecked(system->RaidsDisabled());
        raidsCustom->setChecked(!system->RaidsDisabled() && !system->RaidFleets().empty());
        UpdateRaidFleets();
    }
    else
    {
        trueName->clear();
        displayName->clear();
        government->clear();

        hidden->setChecked(false);
        shrouded->setChecked(false);
        inaccessible->setChecked(false);

        hyperArrival->clear();
        jumpArrival->clear();
        hyperDeparture->clear();
        jumpDeparture->clear();
        jumpRange->clear();
        arrivalFromHabitable->setChecked(false);

        ramscoopUniversal->setChecked(true);
        ramscoopAddend->setText("0");
        ramscoopMultiplier->setText("1");

        tradeWidget->clear();
        fleets->clear();
        minables->clear();
        hazards->clear();
        raidsDisabled->setChecked(false);
        raidsCustom->setChecked(false);
        raidFleets->clear();
    }
    update();
}



void DetailView::UpdateMinables()
{
    if(!system || !minables)
        return;

    disconnect(minables, SIGNAL(itemChanged(QTreeWidgetItem *, int)),
        this, SLOT(MinablesChanged(QTreeWidgetItem *, int)));
    minables->clear();

    for(const System::Minable &minable : system->Minables())
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(minables);
        item->setText(0, minable.type);
        item->setText(1, QString::number(minable.count));
        item->setText(2, QString::number(minable.energy));
        item->setText(3, QString::number(&minable - &system->Minables().front()));

        item->setFlags(item->flags() | Qt::ItemIsEditable);
        minables->addTopLevelItem(item);
    }
    {
        // Add one last item, which is empty, but can be edited to add a row.
        QTreeWidgetItem *item = new QTreeWidgetItem(minables);
        item->setFlags(item->flags() | Qt::ItemIsEditable);

        item->setText(3, QString::number(system->Minables().size()));
        minables->addTopLevelItem(item);
    }
    minables->setColumnWidth(0, 120);
    connect(minables, SIGNAL(itemChanged(QTreeWidgetItem *, int)),
        this, SLOT(MinablesChanged(QTreeWidgetItem *, int)));
}



void DetailView::UpdateCommodities()
{
    QString current;
    if(tradeWidget && tradeWidget->currentItem())
        current = tradeWidget->currentItem()->text(0);

    spinMap.clear();
    tradeWidget->clear();
    tradeWidget->setColumnWidth(1, 70);
    for(const auto &it : mapData.Commodities())
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(tradeWidget);
        int price = system->Trade(it.name);
        item->setText(0, it.name);
        item->setText(2, mapData.PriceLevel(it.name, price));

        QSpinBox *spin = new QSpinBox(tradeWidget);
        spin->setMinimum(0);
        spin->setMaximum(9999);
        spin->setValue(price);
        spin->setSingleStep(10);
        spinMap[spin] = item;
        connect(spin, SIGNAL(valueChanged(int)), this, SLOT(CommodityChanged(int)));
        tradeWidget->insertTopLevelItem(tradeWidget->topLevelItemCount(), item);
        tradeWidget->setItemWidget(item, 1, spin);

        if(it.name == current)
            tradeWidget->setCurrentItem(item, 0);
    }
}



bool DetailView::eventFilter(QObject *object, QEvent *event)
{
    if(object == government && event->type() == QEvent::FocusIn)
        galaxyView->SetGovernment(government->text());
    return false;
}



// Change the name of the selected system.
void DetailView::TrueNameEdited()
{
    displayName->setPlaceholderText(trueName->text());
}



void DetailView::TrueNameChanged()
{
    trueName->blockSignals(true);
    if(galaxyView && system && system->TrueName() != trueName->text())
    {
        // Attempt the name change in GalaxyView, to update both this and SystemView.
        if(!galaxyView->RenameSystem(system->TrueName(), trueName->text()))
            trueName->setText(system->TrueName());
    }
    trueName->blockSignals(false);
}



void DetailView::UseDisplayNameChanged()
{
    if(!system)
        return;

    if(useDisplayName->isChecked())
        displayName->setReadOnly(false);
    else
        displayName->setReadOnly(true);
}



void DetailView::DisplayNameChanged()
{
    if(!system)
        return;
    if(system->DisplayName() == displayName->text())
        return;
    if(system->DisplayName().isEmpty() && displayName->text() == system->TrueName())
        return;
    if(displayName->text() == system->TrueName())
        system->SetDisplayName(QString());
    else
        system->SetDisplayName(displayName->text());
    mapData.SetChanged();
}



void DetailView::GovernmentChanged()
{
    const QString &newGov = government->text();
    if(!system || system->Government() == newGov || newGov.isEmpty())
        return;

    system->SetGovernment(newGov);
    galaxyView->SetGovernment(newGov);
    mapData.SetChanged();

    // Refresh the Galaxy map since it is using a new Government color.
    galaxyView->update();
}



void DetailView::HiddenClicked()
{
    if(!system)
        return;

    system->ToggleHidden();
    mapData.SetChanged();
}



void DetailView::ShroudedClicked()
{
    if(!system)
        return;

    system->ToggleShrouded();
    mapData.SetChanged();
}



void DetailView::InaccessibleClicked()
{
    if(!system)
        return;

    system->ToggleInaccessible();
    mapData.SetChanged();
}



void DetailView::JumpRangeChanged()
{
    if(!system)
        return;

    system->SetJumpRange(jumpRange->text().toDouble());
    mapData.SetChanged();
}



void DetailView::HyperArrivalChanged()
{
    if(!system)
        return;

    system->SetHyperArrival(hyperArrival->text().toDouble());
    mapData.SetChanged();
}



void DetailView::JumpArrivalChanged()
{
    if(!system)
        return;

    system->SetJumpArrival(jumpArrival->text().toDouble());
    mapData.SetChanged();
}



void DetailView::ArrivalFromHabitableClicked()
{
    if(!system)
        return;

    if(arrivalFromHabitable->isChecked())
    {
        hyperArrival->setReadOnly(true);
        hyperArrival->setText(QString::number(system->HabitableZone()));
        system->SetHyperArrival(system->HabitableZone());
        jumpArrival->setReadOnly(true);
        jumpArrival->setText(QString::number(system->HabitableZone()));
        system->SetJumpArrival(system->HabitableZone());
    }
    else
    {
        hyperArrival->setReadOnly(false);
        jumpArrival->setReadOnly(false);
    }

    mapData.SetChanged();
}



void DetailView::HyperDepartureChanged()
{
    if(!system)
        return;

    system->SetHyperDeparture(hyperDeparture->text().toDouble());
    mapData.SetChanged();
}



void DetailView::JumpDepartureChanged()
{
    if(!system)
        return;

    system->SetJumpDeparture(jumpDeparture->text().toDouble());
    mapData.SetChanged();
}






void DetailView::RamscoopUniversalClicked()
{
    if(!system)
        return;

    system->ToggleRamscoopUniversal();
    mapData.SetChanged();
}



void DetailView::RamscoopAddendChanged()
{
    if(!system)
        return;

    system->SetRamscoopAddend(GetOptionalValue(ramscoopAddend->text()));
    mapData.SetChanged();
}



void DetailView::RamscoopMultiplierChanged()
{
    if(!system)
        return;

    system->SetRamscoopMultiplier(GetOptionalValue(ramscoopMultiplier->text()));
    mapData.SetChanged();
}



void DetailView::CommodityClicked(QTreeWidgetItem *item, int /*column*/)
{
    if(galaxyView)
        galaxyView->SetCommodity(item->text(0));
}



void DetailView::CommodityChanged(int value)
{
    auto it = spinMap.find(sender());
    if(it == spinMap.end() || !system)
        return;

    tradeWidget->setCurrentItem(it->second);
    CommodityClicked(it->second, 0);
    system->SetTrade(it->second->text(0), value);
    it->second->setText(2, mapData.PriceLevel(it->second->text(0), value));
    mapData.SetChanged();
    galaxyView->update();
}



void DetailView::FleetChanged(QTreeWidgetItem *item, int column)
{
    if(!system)
        return;

    unsigned row = item->text(2).toInt();
    if(row == system->Fleets().size())
        system->Fleets().emplace_back(item->text(0), item->text(1).toInt());
    else if(item->text(0).isEmpty() && item->text(1).isEmpty())
        system->Fleets().erase(system->Fleets().begin() + row);
    else if(column == 0)
        system->Fleets()[row].name = item->text(0);
    else if(column == 1)
        system->Fleets()[row].period = item->text(1).toInt();
    else
        return;

    mapData.SetChanged();

    UpdateFleets();
}



void DetailView::MinablesChanged(QTreeWidgetItem *item, int column)
{
    if(!system)
        return;

    unsigned row = item->text(2).toInt();
    if(row == system->Minables().size())
        system->Minables().emplace_back(item->text(0), item->text(1).toInt(), item->text(2).toDouble());
    else if(item->text(0).isEmpty() && item->text(1).isEmpty())
        system->Minables().erase(system->Minables().begin() + row);
    else if(column == 0)
        system->Minables()[row].type = item->text(0);
    else if(column == 1)
        system->Minables()[row].count = item->text(1).toInt();
    else if(column == 2)
        system->Minables()[row].energy = item->text(2).toDouble();
    else
        return;

    mapData.SetChanged();

    UpdateMinables();
}



void DetailView::HazardChanged(QTreeWidgetItem *item, int column)
{
    if(!system)
        return;

    unsigned row = item->text(2).toInt();
    if(row == system->Hazards().size())
        system->Hazards().emplace_back(item->text(0), item->text(1).toInt());
    else if(item->text(0).isEmpty() && item->text(1).isEmpty())
        system->Hazards().erase(system->Hazards().begin() + row);
    else if(column == 0)
        system->Hazards()[row].name = item->text(0);
    else if(column == 1)
        system->Hazards()[row].period = item->text(1).toInt();
    else
        return;

    mapData.SetChanged();

    UpdateHazards();
}



void DetailView::RaidsDisabledClicked()
{
    if(!system)
        return;

    system->ToggleRaids();

    bool disableRaids = raidsDisabled->isChecked();
    raidsCustom->setDisabled(disableRaids);
    raidsCustom->setChecked(disableRaids);
    raidsCustom->setCheckable(!disableRaids);

    mapData.SetChanged();

    UpdateRaidFleets();
}



void DetailView::RaidsCustomClicked()
{
    if(!system)
        return;

    if(raidsDisabled->isChecked())
        return;

    UpdateRaidFleets();
}



void DetailView::RaidFleetsChanged(QTreeWidgetItem *item, int column)
{
    if(!system)
        return;

    unsigned row = item->text(3).toInt();
    if(row == system->RaidFleets().size())
        system->RaidFleets().emplace_back(item->text(0), item->text(1).toInt(), item->text(2).toInt());
    else if(item->text(0).isEmpty() && item->text(1).isEmpty() && item->text(2).isEmpty())
        system->RaidFleets().erase(system->RaidFleets().begin() + row);
    else if(column == 0)
        system->RaidFleets()[row].fleetName = item->text(0);
    else if(column == 1)
        system->RaidFleets()[row].minimumAttraction = item->text(1).toInt();
    else if(column == 2)
        system->RaidFleets()[row].maximumAttraction = item->text(2).toInt();
    else
        return;

    mapData.SetChanged();

    UpdateRaidFleets();
}



void DetailView::UpdateFleets()
{
    if(!system || !fleets)
        return;

    disconnect(fleets, SIGNAL(itemChanged(QTreeWidgetItem *, int)),
        this, SLOT(FleetChanged(QTreeWidgetItem *, int)));
    fleets->clear();

    for(const PeriodicEvent &fleet : system->Fleets())
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(fleets);
        item->setText(0, fleet.name);
        item->setText(1, QString::number(fleet.period));
        item->setText(2, QString::number(&fleet - &system->Fleets().front()));
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        fleets->addTopLevelItem(item);
    }
    {
        // Add one last item, which is empty, but can be edited to add a row.
        QTreeWidgetItem *item = new QTreeWidgetItem(fleets);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        item->setText(2, QString::number(system->Fleets().size()));
        fleets->addTopLevelItem(item);
    }
    fleets->setColumnWidth(0, 200);
    connect(fleets, SIGNAL(itemChanged(QTreeWidgetItem *, int)),
        this, SLOT(FleetChanged(QTreeWidgetItem *, int)));
}



void DetailView::UpdateHazards()
{
    if(!system || !hazards)
        return;

    disconnect(hazards, SIGNAL(itemChanged(QTreeWidgetItem *, int)),
        this, SLOT(HazardChanged(QTreeWidgetItem *, int)));
    hazards->clear();

    for(const PeriodicEvent &hazard : system->Hazards())
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(hazards);
        item->setText(0, hazard.name);
        item->setText(1, QString::number(hazard.period));
        item->setText(2, QString::number(&hazard - &system->Hazards().front()));
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        fleets->addTopLevelItem(item);
    }
    {
        // Add one last item, which is empty, but can be edited to add a row.
        QTreeWidgetItem *item = new QTreeWidgetItem(hazards);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        item->setText(2, QString::number(system->Hazards().size()));
        hazards->addTopLevelItem(item);
    }
    hazards->setColumnWidth(0, 200);
    connect(hazards, SIGNAL(itemChanged(QTreeWidgetItem *, int)),
        this, SLOT(HazardChanged(QTreeWidgetItem *, int)));
}



void DetailView::UpdateRaidFleets()
{
    if(!system || !raidFleets)
        return;

    disconnect(raidFleets, SIGNAL(itemChanged(QTreeWidgetItem *, int)),
        this, SLOT(RaidFleetsChanged(QTreeWidgetItem *, int)));
    raidFleets->clear();

    for(const System::RaidFleet &raidFleet : system->RaidFleets())
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(raidFleets);
        item->setText(0, raidFleet.fleetName);
        item->setText(1, QString::number(raidFleet.minimumAttraction));
        item->setText(2, QString::number(raidFleet.maximumAttraction));
        item->setText(3, QString::number(&raidFleet - &system->RaidFleets().front()));
        if(!raidsDisabled->isChecked() && raidsCustom->isChecked())
            item->setFlags(item->flags() | Qt::ItemIsEditable);
        else if(raidsDisabled->isChecked() || !raidsCustom->isChecked())
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        raidFleets->addTopLevelItem(item);
    }
    {
        // Add one last item, which is empty, but can be edited to add a row.
        QTreeWidgetItem *item = new QTreeWidgetItem(raidFleets);
        if(!raidsDisabled->isChecked() && raidsCustom->isChecked())
            item->setFlags(item->flags() | Qt::ItemIsEditable);
        else if(raidsDisabled->isChecked() || !raidsCustom->isChecked())
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        item->setText(3, QString::number(system->RaidFleets().size()));
        raidFleets->addTopLevelItem(item);
    }
    raidFleets->setColumnWidth(0, 160);
    connect(raidFleets, SIGNAL(itemChanged(QTreeWidgetItem *, int)),
        this, SLOT(RaidFleetsChanged(QTreeWidgetItem *, int)));
}
