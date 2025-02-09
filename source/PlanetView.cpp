/* PlanetView.cpp
Copyright (c) 2015 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include "PlanetView.h"

#include "LandscapeView.h"
#include "Map.h"
#include "StellarObject.h"

#include <QCheckBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QRegularExpression>
#include <QRegularExpressionValidator>

#include <cmath>
#include <limits>

using namespace std;

namespace {
    double GetOptionalValue(const QString &text)
    {
        return text.isEmpty() ? numeric_limits<double>::quiet_NaN() : text.toDouble();
    }
}



PlanetView::PlanetView(Map &mapData, QWidget *parent) :
    QWidget(parent), mapData(mapData)
{
    trueName = new QLineEdit(this);
    connect(trueName, SIGNAL(textChanged(const QString &)), this, SLOT(TrueNameEdited()));
    connect(trueName, SIGNAL(editingFinished()), this, SLOT(TrueNameChanged()));
    displayName = new QLineEdit(this);
    connect(displayName, SIGNAL(editingFinished()), this, SLOT(DisplayNameChanged()));
    attributes = new QLineEdit(this);
    connect(attributes, SIGNAL(editingFinished()), this, SLOT(AttributesChanged()));

    government = new QLineEdit(this);
    connect(government, SIGNAL(editingFinished()), this, SLOT(GovernmentChanged()));

    landscape = new LandscapeView(mapData, this);
    landscape->setMinimumHeight(360);
    landscape->setMaximumHeight(360);

    description = new QPlainTextEdit(this);
    description->setTabStopDistance(20);
    connect(description, SIGNAL(textChanged()), this, SLOT(DescriptionChanged()));

    spaceport = new QPlainTextEdit(this);
    spaceport->setTabStopDistance(20);
    connect(spaceport, SIGNAL(textChanged()), this, SLOT(SpaceportDescriptionChanged()));

    shipyard = new QLineEdit(this);
    connect(shipyard, SIGNAL(editingFinished()), this, SLOT(ShipyardChanged()));
    outfitter = new QLineEdit(this);
    connect(outfitter, SIGNAL(editingFinished()), this, SLOT(OutfitterChanged()));

    reputation = new QLineEdit(this);
    reputation->setValidator(new QRegularExpressionValidator(QRegularExpression("-?\\d*\\.?\\d*"), reputation));
    connect(reputation, SIGNAL(editingFinished()), this, SLOT(ReputationChanged()));

    bribe = new QLineEdit(this);
    bribe->setValidator(new QRegularExpressionValidator(QRegularExpression("0||0?\\.\\d*"), bribe));
    connect(bribe, SIGNAL(editingFinished()), this, SLOT(BribeChanged()));

    security = new QLineEdit(this);
    security->setValidator(new QRegularExpressionValidator(QRegularExpression("0||0?\\.\\d*"), security));
    connect(security, SIGNAL(editingFinished()), this, SLOT(SecurityChanged()));

    tribute = new QLineEdit(this);
    tribute->setValidator(new QRegularExpressionValidator(QRegularExpression("\\d*"), tribute));
    connect(tribute, SIGNAL(editingFinished()), this, SLOT(TributeChanged()));

    tributeThreshold = new QLineEdit(this);
    tributeThreshold->setValidator(new QRegularExpressionValidator(QRegularExpression("\\d*"), tributeThreshold));
    connect(tributeThreshold, SIGNAL(editingFinished()), this, SLOT(TributeThresholdChanged()));

    tributeFleetNames = new QLineEdit(this);
    connect(tributeFleetNames, SIGNAL(editingFinished()), this, SLOT(TributeFleetNamesChanged()));


    QGridLayout *layout = new QGridLayout(this);
    int row = 1;

    QHBoxLayout *firstRowLayout = new QHBoxLayout(this);
    firstRowLayout->addWidget(new QLabel("Planet:", this));
    firstRowLayout->addWidget(trueName);
    firstRowLayout->addWidget(new QLabel("Display name:", this));
    firstRowLayout->addWidget(displayName);
    firstRowLayout->addWidget(new QLabel("Government:", this));
    firstRowLayout->addWidget(government);
    layout->addLayout(firstRowLayout, 0, 0, 1, -1);

    layout->addWidget(new QLabel("Attributes:", this), row, 0);
    layout->addWidget(attributes, row++, 1);

    layout->addWidget(landscape, row++, 0, 1, 2);

    layout->addWidget(description, row++, 0, 1, 2);
    layout->addWidget(new QLabel("Spaceport description:", this), row++, 0, 1, 2);
    layout->addWidget(spaceport, row++, 0, 1, 2);

    QHBoxLayout *salesAndTribute = new QHBoxLayout(this);
    QVBoxLayout *salesWrapperLayout = new QVBoxLayout(this);
    QGridLayout *salesLayout = new QGridLayout(this);
    salesLayout->addWidget(new QLabel("Shipyard:", this), 0, 0);
    salesLayout->addWidget(shipyard, 1, 0);
    salesLayout->addWidget(new QLabel("Outfitter:", this), 0, 1);
    salesLayout->addWidget(outfitter, 1, 1);
    salesWrapperLayout->addLayout(salesLayout);
    salesWrapperLayout->addStretch();
    salesAndTribute->addLayout(salesWrapperLayout);

    QGridLayout *tributeLayout = new QGridLayout(this);
    QWidget *tributeBox = new QWidget(this);
    QHBoxLayout *tributeNumberLayout = new QHBoxLayout(tributeBox);
    tributeNumberLayout->setContentsMargins(0, 0, 0, 0);
    tributeNumberLayout->addWidget(tribute);
    tributeNumberLayout->addWidget(new QLabel("Threshold:", this));
    tributeNumberLayout->addWidget(tributeThreshold);
    tributeNumberLayout->addStretch();
    tributeLayout->addWidget(new QLabel("Tribute:", this), 0, 0);
    tributeLayout->addWidget(tributeBox, 0, 1, 1, -1);
    tributeLayout->addWidget(new QLabel("Fleets:", this), 1, 0);
    tributeLayout->addWidget(tributeFleetNames, 1, 1, 1, -1);

    QWidget *box = new QWidget(this);
    QHBoxLayout *hLayout = new QHBoxLayout(box);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->addWidget(new QLabel("Required reputation:", this));
    hLayout->addWidget(reputation);
    hLayout->addWidget(new QLabel("Bribe:", this));
    hLayout->addWidget(bribe);
    hLayout->addWidget(new QLabel("Security:", this));
    hLayout->addWidget(security);
    hLayout->addStretch();
    tributeLayout->addWidget(box, 2, 0, 1, -1);

    salesAndTribute->addLayout(tributeLayout);
    layout->addLayout(salesAndTribute, row++, 0, 1, -1);

    setLayout(layout);
}



void PlanetView::SetPlanet(StellarObject *object)
{
    this->object = object;

    auto it = mapData.Planets().end();
    if(object && !object->GetPlanet().isEmpty())
        it = mapData.Planets().find(object->GetPlanet());

    if(it == mapData.Planets().end())
    {
        trueName->clear();
        displayName->clear();
        attributes->clear();
        government->clear();
        landscape->SetPlanet(nullptr);
        description->clear();
        spaceport->clear();
        shipyard->clear();
        outfitter->clear();
        reputation->clear();
        bribe->clear();
        security->clear();
        tribute->clear();
        tributeThreshold->clear();
        tributeFleetNames->clear();
    }
    else
    {
        Planet &planet = it->second;
        trueName->setText(planet.TrueName());
        if(planet.HasDisplayName())
            displayName->setText(planet.DisplayName());
        displayName->setPlaceholderText(planet.TrueName());
        attributes->setText(ToString(planet.Attributes()));
        government->setText(planet.Government());
        if(government->text().isEmpty() && object->GetSystem())
            government->setPlaceholderText(object->GetSystem()->Government());
        landscape->SetPlanet(&planet);

        disconnect(description, SIGNAL(textChanged()), this, SLOT(DescriptionChanged()));
        description->setPlainText(planet.Description());
        connect(description, SIGNAL(textChanged()), this, SLOT(DescriptionChanged()));

        disconnect(spaceport, SIGNAL(textChanged()), this, SLOT(SpaceportDescriptionChanged()));
        spaceport->setPlainText(planet.SpaceportDescription());
        connect(spaceport, SIGNAL(textChanged()), this, SLOT(SpaceportDescriptionChanged()));

        shipyard->setText(ToString(planet.Shipyard()));
        outfitter->setText(ToString(planet.Outfitter()));
        reputation->setText(std::isnan(planet.RequiredReputation()) ?
            QString() : QString::number(planet.RequiredReputation()));
        bribe->setText(std::isnan(planet.Bribe()) ?
            QString() : QString::number(planet.Bribe()));
        security->setText(std::isnan(planet.Security()) ?
            QString() : QString::number(planet.Security()));
        tribute->setText(std::isnan(planet.Tribute()) ?
            QString() : QString::number(planet.Tribute()));
        tributeThreshold->setText(std::isnan(planet.TributeThreshold()) ?
            QString() : QString::number(planet.TributeThreshold()));
        tributeFleetNames->setText(ToString(planet.TributeFleetNames()));

    }
}



void PlanetView::Reinitialize()
{
    SetPlanet(nullptr);
    landscape->Reinitialize();
}



void PlanetView::TrueNameChanged()
{
    if(!object || object->GetPlanet() == trueName->text() || trueName->text().isEmpty())
        return;

    if(mapData.Planets().count(trueName->text()))
    {
        QMessageBox::warning(this, "Duplicate name",
            "A planet named \"" + trueName->text() + "\" already exists.");
    }
    else
    {
        // Copy the planet data from the old name to the new name..
        mapData.RenamePlanet(object, trueName->text());

        landscape->SetPlanet(&(mapData.Planets()[trueName->text()]));

        mapData.SetChanged();
    }
}



void PlanetView::TrueNameEdited()
{
    displayName->setPlaceholderText(trueName->text());
}



void PlanetView::DisplayNameChanged()
{
    if(!object || object->GetPlanet().isEmpty())
        return;

    Planet &planet = mapData.Planets()[object->GetPlanet()];
    if(planet.DisplayName() == displayName->text())
        return;
    if(planet.DisplayName().isEmpty() && displayName->text() == object->GetPlanet())
        return;
    if(displayName->text() == object->GetPlanet())
        planet.SetDisplayName(QString());
    else
        planet.SetDisplayName(displayName->text());
    mapData.SetChanged();
}



void PlanetView::AttributesChanged()
{
    if(object && !object->GetPlanet().isEmpty())
    {
        vector<QString> list = ToList(attributes->text());
        Planet &planet = mapData.Planets()[object->GetPlanet()];
        if(planet.Attributes() != list)
        {
            planet.Attributes() = list;
            mapData.SetChanged();
        }
    }
}



void PlanetView::GovernmentChanged()
{
    if(!object || object->GetPlanet().isEmpty())
        return;

    Planet &planet = mapData.Planets()[object->GetPlanet()];
    const QString &text = government->text();
    if(planet.Government() == text)
        return;
    if(object->GetSystem() && text == object->GetSystem()->Government())
        planet.SetGovernment(QString());
    else
        planet.SetGovernment(text);
    mapData.SetChanged();
}



void PlanetView::DescriptionChanged()
{
    if(object && !object->GetPlanet().isEmpty())
    {
        QString newDescription = description->toPlainText();
        Planet &planet = mapData.Planets()[object->GetPlanet()];
        if(planet.Description() != newDescription)
        {
            planet.SetDescription(newDescription);
            mapData.SetChanged();
        }
    }
}



void PlanetView::SpaceportDescriptionChanged()
{
    if(object && !object->GetPlanet().isEmpty())
    {
        QString newDescription = spaceport->toPlainText();
        Planet &planet = mapData.Planets()[object->GetPlanet()];
        if(planet.SpaceportDescription() != newDescription)
        {
            planet.SetSpaceportDescription(newDescription);
            mapData.SetChanged();
        }
    }
}



void PlanetView::ShipyardChanged()
{
    if(object && !object->GetPlanet().isEmpty())
    {
        vector<QString> list = ToList(shipyard->text());
        Planet &planet = mapData.Planets()[object->GetPlanet()];
        if(planet.Shipyard() != list)
        {
            planet.Shipyard() = list;
            mapData.SetChanged();
        }
    }
}



void PlanetView::OutfitterChanged()
{
    if(object && !object->GetPlanet().isEmpty())
    {
        vector<QString> list = ToList(outfitter->text());
        Planet &planet = mapData.Planets()[object->GetPlanet()];
        if(planet.Outfitter() != list)
        {
            planet.Outfitter() = list;
            mapData.SetChanged();
        }
    }
}



void PlanetView::ReputationChanged()
{
    if(object && !object->GetPlanet().isEmpty())
    {
        double value = GetOptionalValue(reputation->text());
        Planet &planet = mapData.Planets()[object->GetPlanet()];
        if(planet.RequiredReputation() != value || std::isnan(planet.RequiredReputation()) != std::isnan(value))
        {
            planet.SetRequiredReputation(value);
            mapData.SetChanged();
        }
    }
}



void PlanetView::BribeChanged()
{
    if(object && !object->GetPlanet().isEmpty())
    {
        double value = GetOptionalValue(bribe->text());
        Planet &planet = mapData.Planets()[object->GetPlanet()];
        if(planet.Bribe() != value || std::isnan(planet.Bribe()) != std::isnan(value))
        {
            planet.SetBribe(value);
            mapData.SetChanged();
        }
    }
}



void PlanetView::SecurityChanged()
{
    if(object && !object->GetPlanet().isEmpty())
    {
        double value = GetOptionalValue(security->text());
        Planet &planet = mapData.Planets()[object->GetPlanet()];
        if(planet.Security() != value || std::isnan(planet.Security()) != std::isnan(value))
        {
            planet.SetSecurity(value);
            mapData.SetChanged();
        }
    }
}



void PlanetView::TributeChanged()
{
    if(object && !object->GetPlanet().isEmpty())
    {
        double value = GetOptionalValue(tribute->text());
        Planet &planet = mapData.Planets()[object->GetPlanet()];
        if(planet.Tribute() != value || std::isnan(planet.Tribute()) != std::isnan(value))
        {
            planet.SetTribute(value);
            mapData.SetChanged();
        }
    }
}



void PlanetView::TributeThresholdChanged()
{
    if(object && !object->GetPlanet().isEmpty())
    {
        double value = GetOptionalValue(tributeThreshold->text());
        Planet &planet = mapData.Planets()[object->GetPlanet()];
        if(planet.TributeThreshold() != value || std::isnan(planet.TributeThreshold()) != std::isnan(value))
        {
            planet.SetTributeThreshold(value);
            mapData.SetChanged();
        }
    }
}



void PlanetView::TributeFleetNamesChanged()
{
    if(object && !object->GetPlanet().isEmpty())
    {
        vector<QString> newFleetNames = ToList(tributeFleetNames->text());
        Planet &planet = mapData.Planets()[object->GetPlanet()];
        if(planet.TributeFleetNames() != newFleetNames)
        {
            planet.TributeFleetNames() = newFleetNames;
            mapData.SetChanged();
        }
    }
}



QString PlanetView::ToString(const vector<QString> &list)
{
    if(list.empty())
        return "";

    auto it = list.begin();
    QString result = *it;

    for(++it; it != list.end(); ++it)
        result += ", " + *it;

    return result;
}



vector<QString> PlanetView::ToList(const QString &str)
{
    vector<QString> result;

    QStringList strings = str.split(",", Qt::SkipEmptyParts);
    for(const QString &token : strings)
        result.emplace_back(token.trimmed());

    return result;
}
