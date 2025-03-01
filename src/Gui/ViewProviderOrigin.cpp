/***************************************************************************
 *   Copyright (c) 2015 Stefan Tröger <stefantroeger@gmx.net>              *
 *   Copyright (c) 2015 Alexander Golubev (Fat-Zer) <fatzer2@gmail.com>    *
 *                                                                         *
 *   This file is part of the FreeCAD CAx development system.              *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this library; see the file COPYING.LIB. If not,    *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,         *
 *   Suite 330, Boston, MA  02111-1307, USA                                *
 *                                                                         *
 ***************************************************************************/


#include "PreCompiled.h"

#ifndef _PreComp_
# include <QApplication>
# include <QMenu>
# include <QAction>
# include <QPixmap>
# include <Inventor/actions/SoGetBoundingBoxAction.h>
# include <Inventor/nodes/SoSeparator.h>
#endif


#include <Base/Vector3D.h>
#include <App/Origin.h>
#include <App/OriginFeature.h>
#include <App/Document.h>

/// Here the FreeCAD includes sorted by Base,App,Gui......
#include "ViewProviderOrigin.h"
#include "ViewProviderPlane.h"
#include "ViewProviderLine.h"
#include "Application.h"
#include "Command.h"
#include "BitmapFactory.h"
#include "Document.h"
#include "Tree.h"
#include "ViewParams.h"
#include "View3DInventor.h"
#include "View3DInventorViewer.h"
#include "SoFCUnifiedSelection.h"

#include "Base/Console.h"

#include <App/Origin.h>

using namespace Gui;


PROPERTY_SOURCE(Gui::ViewProviderOrigin, Gui::ViewProviderDocumentObject)

/**
 * Creates the view provider for an object group.
 */
ViewProviderOrigin::ViewProviderOrigin()
{
    auto sz = defaultSize();
    ADD_PROPERTY_TYPE ( Size, (Base::Vector3d(sz,sz,sz)), 0, App::Prop_None,
        QT_TRANSLATE_NOOP("App::Property", "The displayed size of the origin"));
    Size.setStatus(App::Property::ReadOnly, true);

    ADD_PROPERTY_TYPE ( Margin, (Base::Vector3d(2,2,2)), 0, App::Prop_None,
        QT_TRANSLATE_NOOP("App::Property", "Margin to be added to the calculated size of the origin"));

    sPixmap = "Std_CoordinateSystem";
    Visibility.setValue(false);

    // Do not override visibility of origin group
    if(pcModeSwitch->isOfType(SoFCSwitch::getClassTypeId())) 
        static_cast<SoFCSwitch*>(pcModeSwitch)->defaultChild = -1;

    pcGroupChildren = new SoGroup();
    pcGroupChildren->ref();
}

ViewProviderOrigin::~ViewProviderOrigin() {
    pcGroupChildren->unref();
    pcGroupChildren = 0;
}

std::vector<App::DocumentObject*> ViewProviderOrigin::claimChildren(void) const {
    return static_cast<App::Origin*>( getObject() )->OriginFeatures.getValues ();
}

std::vector<App::DocumentObject*> ViewProviderOrigin::claimChildren3D(void) const {
    return claimChildren ();
}

void ViewProviderOrigin::attach(App::DocumentObject* pcObject)
{
    Gui::ViewProviderDocumentObject::attach(pcObject);
    addDisplayMaskMode(pcGroupChildren, "Base");
}

std::vector<std::string> ViewProviderOrigin::getDisplayModes(void) const
{
    return { "Base" };
}

void ViewProviderOrigin::setDisplayMode(const char* ModeName)
{
    if (strcmp(ModeName, "Base") == 0)
        setDisplayMaskMode("Base");
    ViewProviderDocumentObject::setDisplayMode(ModeName);
}

void ViewProviderOrigin::setTemporaryVisibility(bool axis, bool plane) {
    App::Origin* origin = static_cast<App::Origin*>( getObject() );

    bool saveState = tempVisMap.empty();

    try {
        // Remember & Set axis visibility
        for(App::DocumentObject* obj : origin->axes()) {
            if (obj) {
                Gui::ViewProvider* vp = Gui::Application::Instance->getViewProvider(obj);
                if(vp) {
                    if (saveState) {
                        tempVisMap.emplace(vp, vp->isVisible());
                    }
                    vp->setVisible(axis);
                }
            }
        }

        // Remember & Set plane visibility
        for(App::DocumentObject* obj : origin->planes()) {
            if (obj) {
                Gui::ViewProvider* vp = Gui::Application::Instance->getViewProvider(obj);
                if(vp) {
                    if (saveState) {
                        tempVisMap.emplace(vp, vp->isVisible());
                    }
                    vp->setVisible(plane);
                }
            }
        }
    } catch (const Base::Exception &ex) {
        Base::Console().Error ("%s\n", ex.what() );
    }

    // Remember & Set self visibility
    tempVisMap.emplace(this, isVisible());
    setVisible(true);

}

void ViewProviderOrigin::resetTemporaryVisibility() {
    for(std::pair<Gui::ViewProvider*, bool> pair : tempVisMap) {
        pair.first->setVisible(pair.second);
    }
    tempVisMap.clear ();
}

double ViewProviderOrigin::defaultSize()
{
    return 0.25 * ViewParams::instance()->getNewDocumentCameraScale();
}

double ViewProviderOrigin::baseSize()
{
    return 10;
}

bool ViewProviderOrigin::isTemporaryVisibility() {
    return !tempVisMap.empty();
}

void ViewProviderOrigin::updateData(const App::Property *prop) {
    App::Origin* origin = static_cast<App::Origin*> ( getObject() );
    if(origin) {
        if(prop == &origin->OriginFeatures && origin->OriginFeatures.getSize()
                && !origin->getDocument()->isPerformingTransaction()
                && !origin->testStatus(App::ObjectStatus::Remove))
        {
            Size.touch();
        }
    }
    ViewProviderDocumentObject::updateData ( prop );
}

bool ViewProviderOrigin::doubleClicked() {
    App::Origin* origin = static_cast<App::Origin*> ( getObject() );
    if(origin)
        origin->initObjects();
    return true;
}

void ViewProviderOrigin::setupContextMenu(QMenu* menu, QObject* receiver, const char* member)
{
    App::Origin* origin = static_cast<App::Origin*> ( getObject() );
    if(origin && !origin->OriginFeatures.getSize()) {
        QAction* act = menu->addAction(QObject::tr("Create origin features"), receiver, member);
        act->setData(QVariant((int)ViewProvider::Default));
    }
}

bool ViewProviderOrigin::setEdit(int ModNum)
{
    if(ModNum == ViewProvider::Default)
        return doubleClicked();
    return false;
}

void ViewProviderOrigin::onChanged(const App::Property* prop) {
    App::Origin* origin = static_cast<App::Origin*> ( getObject() );
    if(!origin) {
        ViewProviderDocumentObject::onChanged ( prop );
        return;
    }

    if ((prop==&Size || prop==&Margin) && origin->OriginFeatures.getSize()) {
        try {
            Gui::Application *app = Gui::Application::Instance;
            Base::Vector3d sz = Size.getValue() + Margin.getValue();

            // Calculate axes and planes sizes
            double szXY = std::max ( sz.x, sz.y );
            double szXZ = std::max ( sz.x, sz.z );
            double szYZ = std::max ( sz.y, sz.z );

            double szX = std::min ( szXY, szXZ );
            double szY = std::min ( szXY, szYZ );
            double szZ = std::min ( szXZ, szYZ );

            // Find view providers
            Gui::ViewProviderPlane* vpPlaneXY, *vpPlaneXZ, *vpPlaneYZ;
            Gui::ViewProviderLine* vpLineX, *vpLineY, *vpLineZ;
            // Planes
            vpPlaneXY = static_cast<Gui::ViewProviderPlane *> ( app->getViewProvider ( origin->getXY () ) );
            vpPlaneXZ = static_cast<Gui::ViewProviderPlane *> ( app->getViewProvider ( origin->getXZ () ) );
            vpPlaneYZ = static_cast<Gui::ViewProviderPlane *> ( app->getViewProvider ( origin->getYZ () ) );
            // Axes
            vpLineX = static_cast<Gui::ViewProviderLine *> ( app->getViewProvider ( origin->getX () ) );
            vpLineY = static_cast<Gui::ViewProviderLine *> ( app->getViewProvider ( origin->getY () ) );
            vpLineZ = static_cast<Gui::ViewProviderLine *> ( app->getViewProvider ( origin->getZ () ) );

            // set their sizes
            if (vpPlaneXY) { vpPlaneXY->Size.setValue ( szXY ); }
            if (vpPlaneXZ) { vpPlaneXZ->Size.setValue ( szXZ ); }
            if (vpPlaneYZ) { vpPlaneYZ->Size.setValue ( szYZ ); }
            if (vpLineX) { vpLineX->Size.setValue ( szX ); }
            if (vpLineY) { vpLineY->Size.setValue ( szY ); }
            if (vpLineZ) { vpLineZ->Size.setValue ( szZ ); }

        } catch (const Base::Exception &ex) {
            // While restoring a document don't report errors if one of the lines or planes
            // cannot be found.
            App::Document* doc = getObject()->getDocument();
            if (!doc->testStatus(App::Document::Restoring))
                Base::Console().Error ("%s\n", ex.what() );
        }
    } else if (prop == &Visibility) {
        if(Visibility.getValue())
            origin->initObjects();
    }

    ViewProviderDocumentObject::onChanged ( prop );
}

bool ViewProviderOrigin::onDelete(const std::vector<std::string> &) {
    App::Origin* origin = static_cast<App::Origin*>( getObject() );

    if ( !origin->getInList().empty() ) {
        return false;
    }

    auto objs = origin->OriginFeatures.getValues();
    origin->OriginFeatures.setValues({});

    for (auto obj: objs ) {
        Gui::Command::doCommand( Gui::Command::Doc, "App.getDocument(\"%s\").removeObject(\"%s\")",
                obj->getDocument()->getName(), obj->getNameInDocument() );
    }

    return true;
}
