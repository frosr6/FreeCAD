/******************************************************************************
 *   Copyright (c) 2012 Jan Rheinländer <jrheinlaender@users.sourceforge.net> *
 *                                                                            *
 *   This file is part of the FreeCAD CAx development system.                 *
 *                                                                            *
 *   This library is free software; you can redistribute it and/or            *
 *   modify it under the terms of the GNU Library General Public              *
 *   License as published by the Free Software Foundation; either             *
 *   version 2 of the License, or (at your option) any later version.         *
 *                                                                            *
 *   This library  is distributed in the hope that it will be useful,         *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *   GNU Library General Public License for more details.                     *
 *                                                                            *
 *   You should have received a copy of the GNU Library General Public        *
 *   License along with this library; see the file COPYING.LIB. If not,       *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,            *
 *   Suite 330, Boston, MA  02111-1307, USA                                   *
 *                                                                            *
 ******************************************************************************/


#include "PreCompiled.h"

#ifndef _PreComp_
# include <QMessageBox>
# include <QAction>
# include <QTimer>
#endif

#include <Base/Console.h>
#include <Base/Tools.h>
#include <Base/UnitsApi.h>
#include <App/Application.h>
#include <App/Document.h>
#include <App/Origin.h>
#include <App/OriginFeature.h>
#include <Gui/Application.h>
#include <Gui/Document.h>
#include <Gui/BitmapFactory.h>
#include <Gui/ViewProvider.h>
#include <Gui/WaitCursor.h>
#include <Gui/Selection.h>
#include <Gui/Command.h>
#include <Gui/ViewProviderOrigin.h>

#include <Mod/PartDesign/App/FeatureLinearPattern.h>
#include <Mod/PartDesign/App/DatumPlane.h>
#include <Mod/PartDesign/App/DatumLine.h>
#include <Mod/PartDesign/App/Body.h>
#include <Mod/Sketcher/App/SketchObject.h>

#include "ReferenceSelection.h"
#include "TaskMultiTransformParameters.h"
#include "Utils.h"

#include "ui_TaskLinearPatternParameters.h"
#include "TaskLinearPatternParameters.h"

using namespace PartDesignGui;
using namespace Gui;

/* TRANSLATOR PartDesignGui::TaskLinearPatternParameters */

TaskLinearPatternParameters::TaskLinearPatternParameters(ViewProviderTransformed *TransformedView,QWidget *parent)
    : TaskTransformedParameters(TransformedView, parent)
    , ui(new Ui_TaskLinearPatternParameters)
{
    // we need a separate container widget to add all controls to
    proxy = new QWidget(this);
    ui->setupUi(proxy);
    QMetaObject::connectSlotsByName(this);

    this->groupLayout()->addWidget(proxy);

    ui->buttonOK->hide();
    ui->checkBoxUpdateView->setEnabled(true);

    selectionMode = none;

    blockUpdate = false; // Hack, sometimes it is NOT false although set to false in Transformed::Transformed()!!
    setupUI();
}

TaskLinearPatternParameters::TaskLinearPatternParameters(TaskMultiTransformParameters *parentTask, QLayout *layout)
        : TaskTransformedParameters(parentTask), ui(new Ui_TaskLinearPatternParameters)
{
    proxy = new QWidget(parentTask);
    ui->setupUi(proxy);
    connect(ui->buttonOK, SIGNAL(clicked(bool)),
            parentTask, SLOT(onSubTaskButtonOK()));
    QMetaObject::connectSlotsByName(this);

    layout->addWidget(proxy);

    ui->buttonOK->setEnabled(true);
    ui->checkBoxUpdateView->hide();

    selectionMode = none;

    blockUpdate = false; // Hack, sometimes it is NOT false although set to false in Transformed::Transformed()!!
    setupUI();
}

void TaskLinearPatternParameters::setupUI()
{
    TaskTransformedParameters::setupUI();

    connect(ui->comboDirection, SIGNAL(activated(int)),
            this, SLOT(onDirectionChanged(int)));
    connect(ui->checkReverse, SIGNAL(toggled(bool)),
            this, SLOT(onCheckReverse(bool)));
    connect(ui->spinLength, SIGNAL(valueChanged(double)),
            this, SLOT(onLength(double)));
    connect(ui->spinOccurrences, SIGNAL(valueChanged(uint)),
            this, SLOT(onOccurrences(uint)));
    connect(ui->checkBoxUpdateView, SIGNAL(toggled(bool)),
            this, SLOT(onUpdateView(bool)));

    PartDesign::LinearPattern* pcLinearPattern = static_cast<PartDesign::LinearPattern*>(getObject());
    ui->spinLength->bind(pcLinearPattern->Length);
    ui->spinOccurrences->setMaximum(INT_MAX);
    ui->spinOccurrences->bind(pcLinearPattern->Occurrences);

    ui->comboDirection->setEnabled(true);
    ui->checkReverse->setEnabled(true);
    ui->spinLength->blockSignals(true);
    ui->spinLength->setEnabled(true);
    ui->spinLength->setUnit(Base::Unit::Length);
    ui->spinLength->blockSignals(false);
    ui->spinOccurrences->setEnabled(true);
    
    dirLinks.setCombo(*(ui->comboDirection));
    App::DocumentObject* sketch = getSketchObject();
    if (sketch && sketch->isDerivedFrom(Part::Part2DObject::getClassTypeId())) {
        this->fillAxisCombo(dirLinks, static_cast<Part::Part2DObject*>(sketch));
    }
    else {
        this->fillAxisCombo(dirLinks, nullptr);
    }

    //show the parts coordinate system axis for selection
    PartDesign::Body * body = PartDesign::Body::findBodyOf(getObject());
    if(body) {
        try {
            App::Origin *origin = body->getOrigin();
            ViewProviderOrigin* vpOrigin;
            vpOrigin = static_cast<ViewProviderOrigin*>(Gui::Application::Instance->getViewProvider(origin));
            vpOrigin->setTemporaryVisibility(true, false);
        } catch (const Base::Exception &ex) {
            Base::Console().Error ("%s\n", ex.what () );
        }
    }

    updateUI();
}

void TaskLinearPatternParameters::updateUI()
{
    Base::StateLocker lock(blockUpdate);

    PartDesign::LinearPattern* pcLinearPattern = static_cast<PartDesign::LinearPattern*>(getObject());

    bool reverse = pcLinearPattern->Reversed.getValue();
    double length = pcLinearPattern->Length.getValue();
    unsigned occurrences = pcLinearPattern->Occurrences.getValue();

    if (dirLinks.setCurrentLink(pcLinearPattern->Direction) == -1){
        //failed to set current, because the link isn't in the list yet
        dirLinks.addLink(pcLinearPattern->Direction, getRefStr(pcLinearPattern->Direction.getValue(),
                                                               pcLinearPattern->Direction.getSubValues()));
        dirLinks.setCurrentLink(pcLinearPattern->Direction);
    }

    // Note: These three lines would trigger onLength(), on Occurrences() and another updateUI() if we
    // didn't check for blockUpdate
    ui->checkReverse->setChecked(reverse);
    ui->spinLength->setValue(length);
    ui->spinOccurrences->setValue(occurrences);
}

void TaskLinearPatternParameters::onSelectionChanged(const Gui::SelectionChanges& msg)
{
    if (selectionMode != none && msg.Type == Gui::SelectionChanges::AddSelection) {
        if (selectionMode == reference) {
            // TODO check if this works correctly (2015-09-01, Fat-Zer)
            exitSelectionMode();
            std::vector<std::string> directions;
            App::DocumentObject* selObj = nullptr;
            PartDesign::LinearPattern* pcLinearPattern = static_cast<PartDesign::LinearPattern*>(getObject());
            if (pcLinearPattern) {
                getReferencedSelection(pcLinearPattern, msg, selObj, directions);

                // Note: ReferenceSelection has already checked the selection for validity
                if (selObj && (selectionMode == reference ||
                               selObj->isDerivedFrom(App::Line::getClassTypeId()) ||
                               selObj->isDerivedFrom(Part::Feature::getClassTypeId()) ||
                               selObj->isDerivedFrom(PartDesign::Line::getClassTypeId()) ||
                               selObj->isDerivedFrom(PartDesign::Plane::getClassTypeId()))) {
                    setupTransaction();
                    pcLinearPattern->Direction.setValue(selObj, directions);
                    recomputeFeature();
                    updateUI();
                }
            }
            return;
        }
    }
    TaskTransformedParameters::onSelectionChanged(msg);
}

void TaskLinearPatternParameters::onCheckReverse(const bool on) {
    if (blockUpdate)
        return;
    PartDesign::LinearPattern* pcLinearPattern = static_cast<PartDesign::LinearPattern*>(getObject());
    pcLinearPattern->Reversed.setValue(on);

    exitSelectionMode();
    kickUpdateViewTimer();
}

void TaskLinearPatternParameters::onLength(const double l) {
    if (blockUpdate)
        return;
    PartDesign::LinearPattern* pcLinearPattern = static_cast<PartDesign::LinearPattern*>(getObject());
    pcLinearPattern->Length.setValue(l);

    exitSelectionMode();
    kickUpdateViewTimer();
}

void TaskLinearPatternParameters::onOccurrences(const uint n) {
    if (blockUpdate)
        return;
    PartDesign::LinearPattern* pcLinearPattern = static_cast<PartDesign::LinearPattern*>(getObject());
    pcLinearPattern->Occurrences.setValue(n);

    exitSelectionMode();
    kickUpdateViewTimer();
}

void TaskLinearPatternParameters::onDirectionChanged(int /*num*/)
{
    if (blockUpdate)
        return;
    PartDesign::LinearPattern* pcLinearPattern = static_cast<PartDesign::LinearPattern*>(getObject());
    try{
        if(dirLinks.getCurrentLink().getValue() == 0){
            // enter reference selection mode
            selectionMode = reference;
            Gui::Selection().clearSelection();
            addReferenceSelectionGate(true, true);
        } else {
            exitSelectionMode();
            pcLinearPattern->Direction.Paste(dirLinks.getCurrentLink());
        }
    } catch (Base::Exception &e) {
        QMessageBox::warning(0,tr("Error"),QString::fromLatin1(e.what()));
    }

    kickUpdateViewTimer();
}

void TaskLinearPatternParameters::onUpdateView(bool on)
{
    blockUpdate = !on;
    if (on) {
        // Do the same like in TaskDlgLinearPatternParameters::accept() but without doCommand
        PartDesign::LinearPattern* pcLinearPattern = static_cast<PartDesign::LinearPattern*>(getObject());
        std::vector<std::string> directions;
        App::DocumentObject* obj;

        setupTransaction();
        getDirection(obj, directions);
        pcLinearPattern->Direction.setValue(obj,directions);
        pcLinearPattern->Reversed.setValue(getReverse());
        pcLinearPattern->Length.setValue(getLength());
        pcLinearPattern->Occurrences.setValue(getOccurrences());

        recomputeFeature();
    }
}

void TaskLinearPatternParameters::getDirection(App::DocumentObject*& obj, std::vector<std::string>& sub) const
{
    const App::PropertyLinkSub &lnk = dirLinks.getCurrentLink();
    obj = lnk.getValue();
    sub = lnk.getSubValues();
}

bool TaskLinearPatternParameters::getReverse(void) const
{
    return ui->checkReverse->isChecked();
}

double TaskLinearPatternParameters::getLength(void) const
{
    return ui->spinLength->value().getValue();
}

unsigned TaskLinearPatternParameters::getOccurrences(void) const
{
    return ui->spinOccurrences->value();
}

TaskLinearPatternParameters::~TaskLinearPatternParameters()
{
    try {
        //hide the parts coordinate system axis for selection
        PartDesign::Body * body = PartDesign::Body::findBodyOf(getObject());
        if (body) {
            App::Origin *origin = body->getOrigin();
            ViewProviderOrigin* vpOrigin;
            vpOrigin = static_cast<ViewProviderOrigin*>(Gui::Application::Instance->getViewProvider(origin));
            vpOrigin->resetTemporaryVisibility();
        }
    }
    catch (const Base::Exception &ex) {
        Base::Console().Error ("%s\n", ex.what () );
    }

    if (proxy)
        delete proxy;
}

void TaskLinearPatternParameters::changeEvent(QEvent *e)
{
    TaskBox::changeEvent(e);
    if (e->type() == QEvent::LanguageChange) {
        ui->retranslateUi(proxy);
    }
}

void TaskLinearPatternParameters::apply()
{
    std::vector<std::string> directions;
    App::DocumentObject* obj;
    getDirection(obj, directions);
    std::string direction = buildLinkSingleSubPythonStr(obj, directions);

    auto tobj = TransformedView->getObject();
    FCMD_OBJ_CMD(tobj,"Direction = " << direction);
    FCMD_OBJ_CMD(tobj,"Reversed = " << getReverse());

    ui->spinLength->apply();
    ui->spinOccurrences->apply();
}

//**************************************************************************
//**************************************************************************
// TaskDialog
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TaskDlgLinearPatternParameters::TaskDlgLinearPatternParameters(ViewProviderLinearPattern *LinearPatternView)
    : TaskDlgTransformedParameters(LinearPatternView, new TaskLinearPatternParameters(LinearPatternView))
{
}

#include "moc_TaskLinearPatternParameters.cpp"
