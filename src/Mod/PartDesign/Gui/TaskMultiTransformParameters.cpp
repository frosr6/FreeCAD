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
#endif

#include "ui_TaskMultiTransformParameters.h"
#include "TaskMultiTransformParameters.h"
#include "TaskMirroredParameters.h"
#include "TaskLinearPatternParameters.h"
#include "TaskPolarPatternParameters.h"
#include "TaskScaledParameters.h"
#include "Utils.h"
#include <App/Application.h>
#include <App/Document.h>
#include <App/Origin.h>
#include <Gui/Application.h>
#include <Gui/Document.h>
#include <Gui/BitmapFactory.h>
#include <Gui/ViewProvider.h>
#include <Gui/WaitCursor.h>
#include <Base/Console.h>
#include <Gui/Selection.h>
#include <Gui/Command.h>
#include <Gui/Control.h>
#include <Mod/PartDesign/App/FeatureMultiTransform.h>
#include <Mod/PartDesign/App/FeatureMirrored.h>
#include <Mod/PartDesign/App/FeatureLinearPattern.h>
#include <Mod/PartDesign/App/FeaturePolarPattern.h>
#include <Mod/PartDesign/App/FeatureScaled.h>
#include <Mod/PartDesign/App/Body.h>
#include <Mod/Sketcher/App/SketchObject.h>

using namespace PartDesignGui;
using namespace Gui;

/* TRANSLATOR PartDesignGui::TaskMultiTransformParameters */

TaskMultiTransformParameters::TaskMultiTransformParameters(ViewProviderTransformed *TransformedView,QWidget *parent)
    : TaskTransformedParameters(TransformedView, parent)
    , ui(new Ui_TaskMultiTransformParameters)
    , subTask(nullptr)
    , subFeature(nullptr)
{
    // we need a separate container widget to add all controls to
    proxy = new QWidget(this);
    ui->setupUi(proxy);
    QMetaObject::connectSlotsByName(this);
    this->groupLayout()->addWidget(proxy);

    setupUI();

    // Create a context menu for the listview of transformation features
    auto action = new QAction(tr("Edit"), ui->listTransformFeatures);
    action->connect(action, SIGNAL(triggered()),
                    this, SLOT(onTransformEdit()));
    ui->listTransformFeatures->addAction(action);
    action = new QAction(tr("Delete"), ui->listTransformFeatures);
    action->connect(action, SIGNAL(triggered()),
                    this, SLOT(onTransformDelete()));
    ui->listTransformFeatures->addAction(action);
    action = new QAction(tr("Add mirrored transformation"), ui->listTransformFeatures);
    action->connect(action, SIGNAL(triggered()),
                    this, SLOT(onTransformAddMirrored()));
    ui->listTransformFeatures->addAction(action);
    action = new QAction(tr("Add linear pattern"), ui->listTransformFeatures);
    action->connect(action, SIGNAL(triggered()),
                    this, SLOT(onTransformAddLinearPattern()));
    ui->listTransformFeatures->addAction(action);
    action = new QAction(tr("Add polar pattern"), ui->listTransformFeatures);
    action->connect(action, SIGNAL(triggered()),
                    this, SLOT(onTransformAddPolarPattern()));
    ui->listTransformFeatures->addAction(action);
    action = new QAction(tr("Add scaled transformation"), ui->listTransformFeatures);
    action->connect(action, SIGNAL(triggered()),
                    this, SLOT(onTransformAddScaled()));
    ui->listTransformFeatures->addAction(action);
    action = new QAction(tr("Move up"), ui->listTransformFeatures);
    action->connect(action, SIGNAL(triggered()),
                    this, SLOT(onMoveUp()));
    ui->listTransformFeatures->addAction(action);
    action = new QAction(tr("Move down"), ui->listTransformFeatures);
    action->connect(action, SIGNAL(triggered()),
                    this, SLOT(onMoveDown()));
    ui->listTransformFeatures->addAction(action);
    ui->listTransformFeatures->setContextMenuPolicy(Qt::ActionsContextMenu);
    connect(ui->checkBoxUpdateView, SIGNAL(toggled(bool)),
            this, SLOT(onUpdateView(bool)));

    connect(ui->listTransformFeatures, SIGNAL(activated(QModelIndex)),
            this, SLOT(onTransformActivated(QModelIndex)));

    updateUI();
}

void TaskMultiTransformParameters::updateUI()
{
    // Get the transformFeatures data
    PartDesign::MultiTransform* pcMultiTransform = static_cast<PartDesign::MultiTransform*>(TransformedView->getObject());
    std::vector<App::DocumentObject*> transformFeatures = pcMultiTransform->Transformations.getValues();

    // Fill data into dialog elements
    ui->listTransformFeatures->setEnabled(true);
    ui->listTransformFeatures->clear();
    for (std::vector<App::DocumentObject*>::const_iterator i = transformFeatures.begin(); i != transformFeatures.end(); i++)
    {
        if ((*i) != NULL) {
            auto vp = Application::Instance->getViewProvider(*i);
            new QListWidgetItem(vp?vp->getIcon():QIcon(),
                                QString::fromUtf8((*i)->Label.getValue()),
                                ui->listTransformFeatures);
        }
    }
    if (transformFeatures.size() > 0) {
        ui->listTransformFeatures->setCurrentRow(0, QItemSelectionModel::ClearAndSelect);
        editHint = false;
    } else {
        ui->listTransformFeatures->addItem(tr("Right-click to add"));
        editHint = true;
    }
}

void TaskMultiTransformParameters::slotDeletedObject(const Gui::ViewProviderDocumentObject& Obj)
{
    if (Obj.getObject() == this->subFeature)
        this->subFeature = nullptr;
    TaskTransformedParameters::slotDeletedObject(Obj);
}

void TaskMultiTransformParameters::closeSubTask()
{
    if (subTask) {
        exitSelectionMode();
        disconnect(ui->checkBoxUpdateView, 0, subTask, 0);
        delete subTask;
        subTask = NULL;
        this->proxy->setMinimumHeight(this->defaultMinimumHeight);
    }
}

void TaskMultiTransformParameters::onTransformDelete()
{
    if (editHint)
        return; // Can't delete the hint...
    int row = ui->listTransformFeatures->currentIndex().row();
    PartDesign::MultiTransform* pcMultiTransform = static_cast<PartDesign::MultiTransform*>(TransformedView->getObject());
    std::vector<App::DocumentObject*> transformFeatures = pcMultiTransform->Transformations.getValues();

    App::DocumentObject* feature = transformFeatures[row];
    if (feature == this->subFeature)
        this->subFeature = nullptr;

    setupTransaction();
    pcMultiTransform->getDocument()->removeObject(feature->getNameInDocument());
    closeSubTask();

    transformFeatures.erase(transformFeatures.begin() + row);
    pcMultiTransform->Transformations.setValues(transformFeatures);
    // Note: When the last transformation is deleted, recomputeFeature does nothing, because Transformed::execute()
    // says: "No transformations defined, exit silently"
    recomputeFeature();

    ui->listTransformFeatures->model()->removeRow(row);
    ui->listTransformFeatures->setCurrentRow(0, QItemSelectionModel::ClearAndSelect);
}

void TaskMultiTransformParameters::onTransformEdit()
{
    if (editHint)
        return; // Can't edit the hint...
    closeSubTask(); // For example if user is editing one subTask and then double-clicks on another without OK'ing first
    ui->listTransformFeatures->currentItem()->setSelected(true);
    int row = ui->listTransformFeatures->currentIndex().row();
    PartDesign::MultiTransform* pcMultiTransform = static_cast<PartDesign::MultiTransform*>(TransformedView->getObject());
    std::vector<App::DocumentObject*> transformFeatures = pcMultiTransform->Transformations.getValues();

    subFeature = static_cast<PartDesign::Transformed*>(transformFeatures[row]);
    if (transformFeatures[row]->getTypeId() == PartDesign::Mirrored::getClassTypeId())
        subTask = new TaskMirroredParameters(this, ui->verticalLayout);
    else if (transformFeatures[row]->getTypeId() == PartDesign::LinearPattern::getClassTypeId())
        subTask = new TaskLinearPatternParameters(this, ui->verticalLayout);
    else if (transformFeatures[row]->getTypeId() == PartDesign::PolarPattern::getClassTypeId())
        subTask = new TaskPolarPatternParameters(this, ui->verticalLayout);
    else if (transformFeatures[row]->getTypeId() == PartDesign::Scaled::getClassTypeId())
        subTask = new TaskScaledParameters(this, ui->verticalLayout);
    else
        return; // TODO: Show an error?
    
    this->proxy->setMinimumHeight(this->defaultMinimumHeight
            + subTask->getProxyWidget()->minimumHeight());

    connect(ui->checkBoxUpdateView, SIGNAL(toggled(bool)),
            subTask, SLOT(onUpdateView(bool)));
}

void TaskMultiTransformParameters::onTransformActivated(const QModelIndex& index)
{
    Q_UNUSED(index);
    onTransformEdit();
}

void TaskMultiTransformParameters::onTransformAddMirrored()
{
    closeSubTask();
    std::string newFeatName = TransformedView->getObject()->getDocument()->getUniqueObjectName("Mirrored");
    auto pcActiveBody = PartDesignGui::getBody(false);
    if (!pcActiveBody)
        return;

    // Gui::Command::openCommand(QT_TRANSLATE_NOOP("Command", "Mirrored"));
    setupTransaction();

    FCMD_OBJ_CMD(pcActiveBody, "newObject('PartDesign::Mirrored','"<<newFeatName<<"')");
    auto Feat = pcActiveBody->getDocument()->getObject(newFeatName.c_str());
    if (!Feat)
        return;
    //Gui::Command::updateActive();
    App::DocumentObject* sketch = getSketchObject();
    if (sketch)
        FCMD_OBJ_CMD(Feat, "MirrorPlane = ("<<Gui::Command::getObjectCmd(sketch)<<",['V_Axis'])");
    else {
        FCMD_OBJ_CMD(Feat, "MirrorPlane = ("
                << Gui::Command::getObjectCmd(pcActiveBody->getOrigin()->getXY()) << ",[''])");
    }

    finishAdd(newFeatName);
}

void TaskMultiTransformParameters::onTransformAddLinearPattern()
{
    // See CmdPartDesignLinearPattern
    //
    closeSubTask();
    std::string newFeatName = TransformedView->getObject()->getDocument()->getUniqueObjectName("LinearPattern");
    auto pcActiveBody = PartDesignGui::getBody(false);
    if (!pcActiveBody)
        return;

    // Gui::Command::openCommand(QT_TRANSLATE_NOOP("Command", "Make LinearPattern"));
    setupTransaction();

    FCMD_OBJ_CMD(pcActiveBody, "newObject('PartDesign::LinearPattern','"<<newFeatName<<"')");
    auto Feat = pcActiveBody->getDocument()->getObject(newFeatName.c_str());
    if (!Feat)
        return;
    //Gui::Command::updateActive();
    App::DocumentObject* sketch = getSketchObject();
    if (sketch) {
        FCMD_OBJ_CMD(Feat, "Direction = ("<<Gui::Command::getObjectCmd(sketch)<<",['H_Axis'])");
    }
    else {
        // set Direction value before filling up the combo box to avoid creating an empty item
        // inside updateUI()
        PartDesign::Body* body = static_cast<PartDesign::Body*>(Part::BodyBase::findBodyOf(getObject()));
        if (body) {
            FCMD_OBJ_CMD(Feat, "Direction = ("<<Gui::Command::getObjectCmd(body->getOrigin()->getX())<<",[''])");
        }
    }

    FCMD_OBJ_CMD(Feat, "Length = 100");
    FCMD_OBJ_CMD(Feat, "Occurrences = 2");

    finishAdd(newFeatName);
}

void TaskMultiTransformParameters::onTransformAddPolarPattern()
{
    closeSubTask();
    std::string newFeatName = TransformedView->getObject()->getDocument()->getUniqueObjectName("PolarPattern");
    auto pcActiveBody = PartDesignGui::getBody(false);
    if (!pcActiveBody)
        return;

    // Gui::Command::openCommand(QT_TRANSLATE_NOOP("Command", "PolarPattern"));
    setupTransaction();

    FCMD_OBJ_CMD(pcActiveBody, "newObject('PartDesign::PolarPattern','"<<newFeatName<<"')");
    auto Feat = pcActiveBody->getDocument()->getObject(newFeatName.c_str());
    if (!Feat)
        return;
    //Gui::Command::updateActive();
    App::DocumentObject* sketch = getSketchObject();
    if (sketch)
        FCMD_OBJ_CMD(Feat, "Axis = ("<<Gui::Command::getObjectCmd(sketch)<<",['N_Axis'])");
    else {
        FCMD_OBJ_CMD(Feat, "Axis = ("
                << Gui::Command::getObjectCmd(pcActiveBody->getOrigin()->getZ())<<",[''])");
    }
    FCMD_OBJ_CMD(Feat, "Angle = 360");
    FCMD_OBJ_CMD(Feat, "Occurrences = 2");

    finishAdd(newFeatName);
}

void TaskMultiTransformParameters::onTransformAddScaled()
{
    closeSubTask();
    std::string newFeatName = TransformedView->getObject()->getDocument()->getUniqueObjectName("Scaled");
    auto pcActiveBody = PartDesignGui::getBody(false);
    if (!pcActiveBody)
        return;

    // Gui::Command::openCommand(QT_TRANSLATE_NOOP("Command", "Scaled"));
    setupTransaction();

    FCMD_OBJ_CMD(pcActiveBody, "newObject('PartDesign::Scaled','"<<newFeatName<<"')");
    auto Feat = pcActiveBody->getDocument()->getObject(newFeatName.c_str());
    if (!Feat)
        return;
    //Gui::Command::updateActive();
    FCMD_OBJ_CMD(Feat, "Factor = 2");
    FCMD_OBJ_CMD(Feat, "Occurrences = 2");

    finishAdd(newFeatName);
}

void TaskMultiTransformParameters::finishAdd(std::string &newFeatName)
{
    //Gui::Command::updateActive();
    //Gui::Command::copyVisual(newFeatName.c_str(), "ShapeColor", getOriginals().front()->getNameInDocument().c_str());
    //Gui::Command::copyVisual(newFeatName.c_str(), "DisplayMode", getOriginals().front()->getNameInDocument().c_str());

    setupTransaction();
    PartDesign::MultiTransform* pcMultiTransform = static_cast<PartDesign::MultiTransform*>(TransformedView->getObject());
    if (editHint) {
        // Remove hint, first feature is being added
        ui->listTransformFeatures->model()->removeRow(0);
    }
    int row = ui->listTransformFeatures->currentIndex().row();

    // Insert new transformation after the selected row
    // This means that in order to insert at the beginning, the user has to use "Move Up" in the menu

    auto newFeature = static_cast<PartDesign::Transformed*>(
            pcMultiTransform->getDocument()->getObject(newFeatName.c_str()));
    newFeature->BaseFeature.setValue(nullptr);

    auto body = PartDesign::Body::findBodyOf(newFeature);
    if (body && body->Tip.getValue() == newFeature)
        body->setTip(pcMultiTransform);

    std::vector<App::DocumentObject*> transformFeatures = pcMultiTransform->Transformations.getValues();
    if (row == ui->listTransformFeatures->model()->rowCount() - 1) {
        // Note: Inserts always happen before the specified iterator so in order to append at the
        // end we need to use push_back() and append()
        transformFeatures.push_back(newFeature);
        auto vp = Application::Instance->getViewProvider(newFeature);
        new QListWidgetItem(vp?vp->getIcon():QIcon(),
                            QString::fromUtf8(newFeature->Label.getValue()),
                            ui->listTransformFeatures);
        ui->listTransformFeatures->setCurrentRow(row+1, QItemSelectionModel::ClearAndSelect);
    } else {
        // Note: The feature tree always seems to append to the end, no matter what we say here
        transformFeatures.insert(transformFeatures.begin() + row + 1, newFeature);
        ui->listTransformFeatures->insertItem(row + 1, QString::fromLatin1(newFeature->Label.getValue()));
        ui->listTransformFeatures->setCurrentRow(row + 1, QItemSelectionModel::ClearAndSelect);
    }
    pcMultiTransform->Transformations.setValues(transformFeatures);

    recomputeFeature();

    editHint = false;

    onTransformEdit();
}

void TaskMultiTransformParameters::moveTransformFeature(const int increment)
{
    setupTransaction();
    int row = ui->listTransformFeatures->currentIndex().row();
    PartDesign::MultiTransform* pcMultiTransform = static_cast<PartDesign::MultiTransform*>(TransformedView->getObject());
    std::vector<App::DocumentObject*> transformFeatures = pcMultiTransform->Transformations.getValues();

    App::DocumentObject* feature = transformFeatures[row];
    transformFeatures.erase(transformFeatures.begin() + row);
    QListWidgetItem* item = new QListWidgetItem(*(ui->listTransformFeatures->item(row)));
    ui->listTransformFeatures->model()->removeRow(row);
    // After this operation, if we were to insert at index row again, things will remain unchanged

    row += increment;

    if (row < 0)
        row = 0;

    if (row >= ui->listTransformFeatures->model()->rowCount()) {
        // Note: Inserts always happen before the specified iterator so in order to append at the
        // end we need to use push_back() and append()
        transformFeatures.push_back(feature);
        ui->listTransformFeatures->addItem(item);
        ui->listTransformFeatures->setCurrentRow(row, QItemSelectionModel::ClearAndSelect);
    } else {
        transformFeatures.insert(transformFeatures.begin() + row, feature);
        ui->listTransformFeatures->insertItem(row, item);
        ui->listTransformFeatures->setCurrentRow(row, QItemSelectionModel::ClearAndSelect);
    }

    pcMultiTransform->Transformations.setValues(transformFeatures);
    recomputeFeature();
}

void TaskMultiTransformParameters::onMoveUp()
{
    moveTransformFeature(-1);
}

void TaskMultiTransformParameters::onMoveDown()
{
    moveTransformFeature(+1);
}

void TaskMultiTransformParameters::onSubTaskButtonOK() {
    closeSubTask();
}

void TaskMultiTransformParameters::onUpdateView(bool on)
{
    blockUpdate = !on;
    if (on) {
        setupTransaction();
        recomputeFeature();
    }
}

const std::vector<App::DocumentObject*> TaskMultiTransformParameters::getTransformFeatures(void) const
{
    PartDesign::MultiTransform* pcMultiTransform = static_cast<PartDesign::MultiTransform*>(TransformedView->getObject());
    return pcMultiTransform->Transformations.getValues();
}

void TaskMultiTransformParameters::apply()
{
}

TaskMultiTransformParameters::~TaskMultiTransformParameters()
{
    try {
        closeSubTask();
    }
    catch (const Py::Exception&) {
        Base::PyException e; // extract the Python error text
        e.ReportException();
    }
    if (proxy)
        delete proxy;
}

void TaskMultiTransformParameters::changeEvent(QEvent *e)
{
    TaskBox::changeEvent(e);
    if (e->type() == QEvent::LanguageChange) {
        ui->retranslateUi(proxy);
    }
}

//**************************************************************************
//**************************************************************************
// TaskDialog
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TaskDlgMultiTransformParameters::TaskDlgMultiTransformParameters(ViewProviderMultiTransform *MultiTransformView)
    : TaskDlgTransformedParameters(MultiTransformView, new TaskMultiTransformParameters(MultiTransformView))
{
}
//==== calls from the TaskView ===============================================================

bool TaskDlgMultiTransformParameters::accept()
{
    // Set up transformations
    TaskMultiTransformParameters* mtParameter = static_cast<TaskMultiTransformParameters*>(parameter);
    std::vector<App::DocumentObject*> transformFeatures = mtParameter->getTransformFeatures();
    std::stringstream str;
    str << Gui::Command::getObjectCmd(vp->getObject()) << ".Transformations = [";
    for (std::vector<App::DocumentObject*>::const_iterator it = transformFeatures.begin(); it != transformFeatures.end(); it++)
    {
        if ((*it) != NULL)
            str << Gui::Command::getObjectCmd(*it) << ",";
    }
    str << "]";
    Gui::Command::runCommand(Gui::Command::Doc,str.str().c_str());

    return TaskDlgFeatureParameters::accept ();
}

// FIXME: It seems all roll back is finely handled by abortCommand() in parent classes. On the other
//        hand manual removal of objects may lead to segfault in dialog distructer of subtransformation
//        due to TaskMultiTransformParameters::getSubFeature() returns already destroid object. So check
//        that everything is fine and delete the method. (2015-07-31, Fat-Zer)
//bool TaskDlgMultiTransformParameters::reject()
//{
//    // Get objects before view is invalidated
//    // For the same reason we can't delegate showing the originals to TaskDlgTransformedParameters::reject()
//    PartDesign::MultiTransform* pcMultiTransform = static_cast<PartDesign::MultiTransform*>(vp->getObject());
//    std::vector<App::DocumentObject*> transformFeatures = pcMultiTransform->Transformations.getValues();
//
//    // Delete the transformation features - must happen before abortCommand()!
//    for (std::vector<App::DocumentObject*>::const_iterator it = transformFeatures.begin(); it != transformFeatures.end(); ++it)
//    {
//        if ((*it) != NULL)
//            Gui::Command::doCommand(
//                Gui::Command::Doc,"App.ActiveDocument.removeObject(\"%s\")", (*it)->getNameInDocument());
//    }
//
//    return TaskDlgTransformedParameters::reject();
//}

#include "moc_TaskMultiTransformParameters.cpp"
