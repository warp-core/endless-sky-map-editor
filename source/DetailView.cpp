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



DetailView::DetailView(Map &mapData, GalaxyView *galaxyView, QWidget *parent) :
    QWidget(parent), mapData(mapData), galaxyView(galaxyView)
{
    // Create the left sidebar, showing details about the selected system.
    QVBoxLayout *layout = new QVBoxLayout(this);

    layout->addWidget(new QLabel("System Name:", this));
    trueName = new QLineEdit(this);
    connect(trueName, SIGNAL(textChanged(const QString &)), this, SLOT(TrueNameEdited()));
    connect(trueName, SIGNAL(editingFinished()), this, SLOT(TrueNameChanged()));
    layout->addWidget(trueName);
    useDisplayName = new QCheckBox("Display name:", this);
    connect(useDisplayName, SIGNAL(clicked()), this, SLOT(UseDisplayNameChanged()));
    layout->addWidget(useDisplayName);
    displayName = new QLineEdit(this);
    connect(displayName, SIGNAL(editingFinished()), this, SLOT(DisplayNameChanged()));
    layout->addWidget(displayName);

    layout->addWidget(new QLabel("Government:", this));
    government = new QLineEdit(this);
    connect(government, SIGNAL(editingFinished()), this, SLOT(GovernmentChanged()));
    layout->addWidget(government);
    // Selecting the government field changes the system and link colors on the Galaxy map.
    government->installEventFilter(this);


    // Add a table to display this system's default trade.
    tradeWidget = new QTreeWidget(this);
    tradeWidget->setMinimumHeight(310);
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
        
        UpdateCommodities();
        UpdateFleets();
        UpdateMinables();
        UpdateHazards();
    }
    else
    {
        trueName->clear();
        displayName->clear();
        government->clear();

        tradeWidget->clear();
        fleets->clear();
        minables->clear();
        hazards->clear();
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
