/***************************************************************************
 *   Copyright (c) 2011 Jürgen Riegel <juergen.riegel@web.de>              *
 *   Copyright (c) 2011 Werner Mayer <wmayer[at]users.sourceforge.net>     *
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
# include <assert.h>
# include <string>
# include <boost_bind_bind.hpp>
# include <QApplication>
# include <QString>
# include <QStatusBar>
# include <QTextStream>
#endif

#include <boost/algorithm/string/predicate.hpp>

/// Here the FreeCAD includes sorted by Base,App,Gui......
#include "Application.h"
#include "Document.h"
#include "Selection.h"
#include "SelectionFilter.h"
#include "View3DInventor.h"
#include <Base/Exception.h>
#include <Base/Console.h>
#include <Base/Tools.h>
#include <Base/Interpreter.h>
#include <Base/UnitsApi.h>
#include <App/Application.h>
#include <App/Document.h>
#include <App/DocumentObject.h>
#include <App/DocumentObjectPy.h>
#include <App/GeoFeature.h>
#include <App/DocumentObserver.h>
#include <Gui/SelectionObjectPy.h>
#include "MainWindow.h"
#include "Tree.h"
#include "ViewParams.h"
#include "ViewProviderDocumentObject.h"
#include "Macro.h"
#include "Command.h"
#include "Widgets.h"

FC_LOG_LEVEL_INIT("Selection",false,true,true)

using namespace Gui;
using namespace std;
namespace bp = boost::placeholders;

SelectionGateFilterExternal::SelectionGateFilterExternal(const char *docName, const char *objName) {
    if(docName) {
        DocName = docName;
        if(objName)
            ObjName = objName;
    }
}

bool SelectionGateFilterExternal::allow(App::Document *doc ,App::DocumentObject *obj, const char*) {
    if(!doc || !obj)
        return true;
    if(DocName.size() && doc->getName()!=DocName)
        notAllowedReason = "Cannot select external object";
    else if(ObjName.size() && ObjName==obj->getNameInDocument())
        notAllowedReason = "Cannot select self";
    else
        return true;
    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////

SelectionObserver::SelectionObserver(bool attach,int resolve)
    :resolve(resolve),blockSelection(false)
{
    if(attach)
        attachSelection();
}

SelectionObserver::SelectionObserver(const ViewProviderDocumentObject *vp,bool attach,int resolve)
    :resolve(resolve),blockSelection(false)
{
    if(vp && vp->getObject() && vp->getObject()->getDocument()) {
        filterDocName = vp->getObject()->getDocument()->getName();
        filterObjName = vp->getObject()->getNameInDocument();
    }
    if(attach)
        attachSelection();
}


SelectionObserver::~SelectionObserver()
{
    detachSelection();
}

bool SelectionObserver::blockConnection(bool block)
{
    bool ok = blockSelection;
    if (block)
        blockSelection = true;
    else
        blockSelection = false;
    return ok;
}

bool SelectionObserver::isConnectionBlocked() const
{
    return blockSelection;
}

bool SelectionObserver::isConnectionAttached() const
{
    return connectSelection.connected();
}

void SelectionObserver::attachSelection()
{
    if (!connectSelection.connected()) {
        auto &signal = resolve > 1 ? Selection().signalSelectionChanged3 :
                       resolve     ? Selection().signalSelectionChanged2 :
                                     Selection().signalSelectionChanged  ;
        connectSelection = signal.connect(boost::bind
            (&SelectionObserver::_onSelectionChanged, this, bp::_1));

        if (filterDocName.size()) {
            Selection().addSelectionGate(
                    new SelectionGateFilterExternal(filterDocName.c_str(),filterObjName.c_str()));
        }
    }
}

void SelectionObserver::_onSelectionChanged(const SelectionChanges& msg) {
    try {
        if (blockSelection)
            return;
        onSelectionChanged(msg);
    } catch (Base::Exception &e) {
        e.ReportException();
        FC_ERR("Unhandled Base::Exception caught in selection observer");
    } catch (Py::Exception &) {
        Base::PyGILStateLocker lock;
        Base::PyException e;
        e.ReportException();
        FC_ERR("Unhandled Python exception caught in selection observer");
    } catch (std::exception &e) {
        FC_ERR("Unhandled std::exception caught in selection observer: " << e.what());
    } catch (...) {
        FC_ERR("Unhandled unknown exception caught in selection observer");
    }
}

void SelectionObserver::detachSelection()
{
    if (connectSelection.connected()) {
        connectSelection.disconnect();
        if(filterDocName.size())
            Selection().rmvSelectionGate();
    }
}

// -------------------------------------------

std::vector<SelectionObserverPython*> SelectionObserverPython::_instances;

SelectionObserverPython::SelectionObserverPython(const Py::Object& obj, int resolve) 
    : SelectionObserver(true,resolve),inst(obj)
{
#undef FC_PY_ELEMENT
#define FC_PY_ELEMENT(_name) FC_PY_GetCallable(obj.ptr(),#_name,py_##_name);
    FC_PY_SEL_OBSERVER
}

SelectionObserverPython::~SelectionObserverPython()
{
}

void SelectionObserverPython::addObserver(const Py::Object& obj, int resolve)
{
    _instances.push_back(new SelectionObserverPython(obj,resolve));
}

void SelectionObserverPython::removeObserver(const Py::Object& obj)
{
    SelectionObserverPython* obs=0;
    for (std::vector<SelectionObserverPython*>::iterator it =
        _instances.begin(); it != _instances.end(); ++it) {
        if ((*it)->inst == obj) {
            obs = *it;
            _instances.erase(it);
            break;
        }
    }

    delete obs;
}

void SelectionObserverPython::onSelectionChanged(const SelectionChanges& msg)
{
    switch (msg.Type)
    {
    case SelectionChanges::AddSelection:
        addSelection(msg);
        break;
    case SelectionChanges::RmvSelection:
        removeSelection(msg);
        break;
    case SelectionChanges::SetSelection:
        setSelection(msg);
        break;
    case SelectionChanges::ClrSelection:
        clearSelection(msg);
        break;
    case SelectionChanges::SetPreselect:
        setPreselection(msg);
        break;
    case SelectionChanges::RmvPreselect:
        removePreselection(msg);
        break;
    case SelectionChanges::PickedListChanged:
        pickedListChanged();
        break;
    default:
        break;
    }
}

void SelectionObserverPython::pickedListChanged()
{
    if(py_pickedListChanged.isNone())
        return;
    Base::PyGILStateLocker lock;
    try {
        Py::Callable(py_pickedListChanged).apply(Py::Tuple());
    }
    catch (Py::Exception&) {
        Base::PyException e; // extract the Python error text
        e.ReportException();
    }
}

void SelectionObserverPython::addSelection(const SelectionChanges& msg)
{
    if(py_addSelection.isNone())
        return;
    Base::PyGILStateLocker lock;
    try {
        Py::Tuple args(4);
        args.setItem(0, Py::String(msg.pDocName ? msg.pDocName : ""));
        args.setItem(1, Py::String(msg.pObjectName ? msg.pObjectName : ""));
        args.setItem(2, Py::String(msg.pSubName ? msg.pSubName : ""));
        Py::Tuple tuple(3);
        tuple[0] = Py::Float(msg.x);
        tuple[1] = Py::Float(msg.y);
        tuple[2] = Py::Float(msg.z);
        args.setItem(3, tuple);
        Base::pyCall(py_addSelection.ptr(),args.ptr());
    }
    catch (Py::Exception&) {
        Base::PyException e; // extract the Python error text
        e.ReportException();
    }
}

void SelectionObserverPython::removeSelection(const SelectionChanges& msg)
{
    if(py_removeSelection.isNone())
        return;
    Base::PyGILStateLocker lock;
    try {
        Py::Tuple args(3);
        args.setItem(0, Py::String(msg.pDocName ? msg.pDocName : ""));
        args.setItem(1, Py::String(msg.pObjectName ? msg.pObjectName : ""));
        args.setItem(2, Py::String(msg.pSubName ? msg.pSubName : ""));
        Base::pyCall(py_removeSelection.ptr(),args.ptr());
    }
    catch (Py::Exception&) {
        Base::PyException e; // extract the Python error text
        e.ReportException();
    }
}

void SelectionObserverPython::setSelection(const SelectionChanges& msg)
{
    if(py_setSelection.isNone())
        return;
    Base::PyGILStateLocker lock;
    try {
        Py::Tuple args(1);
        args.setItem(0, Py::String(msg.pDocName ? msg.pDocName : ""));
        Base::pyCall(py_setSelection.ptr(),args.ptr());
    }
    catch (Py::Exception&) {
        Base::PyException e; // extract the Python error text
        e.ReportException();
    }
}

void SelectionObserverPython::clearSelection(const SelectionChanges& msg)
{
    if(py_clearSelection.isNone())
        return;
    Base::PyGILStateLocker lock;
    try {
        Py::Tuple args(1);
        args.setItem(0, Py::String(msg.pDocName ? msg.pDocName : ""));
        Base::pyCall(py_clearSelection.ptr(),args.ptr());
    }
    catch (Py::Exception&) {
        Base::PyException e; // extract the Python error text
        e.ReportException();
    }
}

void SelectionObserverPython::setPreselection(const SelectionChanges& msg)
{
    if(py_setPreselection.isNone())
        return;
    Base::PyGILStateLocker lock;
    try {
        Py::Tuple args(3);
        args.setItem(0, Py::String(msg.pDocName ? msg.pDocName : ""));
        args.setItem(1, Py::String(msg.pObjectName ? msg.pObjectName : ""));
        args.setItem(2, Py::String(msg.pSubName ? msg.pSubName : ""));
        Base::pyCall(py_setPreselection.ptr(),args.ptr());
    }
    catch (Py::Exception&) {
        Base::PyException e; // extract the Python error text
        e.ReportException();
    }
}

void SelectionObserverPython::removePreselection(const SelectionChanges& msg)
{
    if(py_removePreselection.isNone())
        return;
    Base::PyGILStateLocker lock;
    try {
        Py::Tuple args(3);
        args.setItem(0, Py::String(msg.pDocName ? msg.pDocName : ""));
        args.setItem(1, Py::String(msg.pObjectName ? msg.pObjectName : ""));
        args.setItem(2, Py::String(msg.pSubName ? msg.pSubName : ""));
        Base::pyCall(py_removePreselection.ptr(),args.ptr());
    }
    catch (Py::Exception&) {
        Base::PyException e; // extract the Python error text
        e.ReportException();
    }
}

// -------------------------------------------

static int _DisableTopParentCheck;

SelectionNoTopParentCheck::SelectionNoTopParentCheck() {
    ++_DisableTopParentCheck;
}

SelectionNoTopParentCheck::~SelectionNoTopParentCheck() {
    if(_DisableTopParentCheck>0)
        --_DisableTopParentCheck;
}

bool SelectionNoTopParentCheck::enabled() {
    return _DisableTopParentCheck>0;
}

// -------------------------------------------

SelectionContext::SelectionContext(const App::SubObjectT &sobj)
    :_sobj(Selection().getContext())
{
    Selection().setContext(sobj);
}

SelectionContext::~SelectionContext()
{
}

// -------------------------------------------
bool SelectionSingleton::hasSelection() const
{
    return !_SelList.empty();
}

bool SelectionSingleton::hasPreselection() const {
    return !CurrentPreselection.Object.getObjectName().empty();
}

std::vector<SelectionSingleton::SelObj> SelectionSingleton::getCompleteSelection(int resolve) const
{
    return getSelection("*",resolve);
}

std::vector<App::SubObjectT> SelectionSingleton::getSelectionT(
        const char* pDocName, int resolve, bool single) const
{
    auto sels = getSelection(pDocName,resolve,single);
    std::vector<App::SubObjectT> res;
    res.reserve(sels.size());
    for(auto &sel : sels)
        res.emplace_back(sel.pObject,sel.SubName);
    return res;
}

std::vector<SelectionSingleton::SelObj> SelectionSingleton::getSelection(const char* pDocName, 
        int resolve, bool single) const
{
    std::vector<SelObj> temp;
    if(single) temp.reserve(1);
    SelObj tempSelObj;

    App::Document *pcDoc = 0;
    if(!pDocName || strcmp(pDocName,"*")!=0) {
        pcDoc = getDocument(pDocName);
        if (!pcDoc)
            return temp;
    }

    std::map<App::DocumentObject*,std::set<std::string> > objMap;

    for(auto &sel : _SelList) {
        if(!sel.pDoc) continue;
        const char *subelement = 0;
        auto obj = getObjectOfType(sel,App::DocumentObject::getClassTypeId(),resolve,&subelement);
        if(!obj || (pcDoc && sel.pObject->getDocument()!=pcDoc))
            continue;

        // In case we are resolving objects, make sure no duplicates
        if(resolve && !objMap[obj].insert(std::string(subelement?subelement:"")).second)
            continue;

        if(single && temp.size()) {
            temp.clear();
            break;
        }

        tempSelObj.DocName  = obj->getDocument()->getName();
        tempSelObj.FeatName = obj->getNameInDocument();
        tempSelObj.SubName = subelement;
        tempSelObj.TypeName = obj->getTypeId().getName();
        tempSelObj.pObject  = obj;
        tempSelObj.pResolvedObject  = sel.pResolvedObject;
        tempSelObj.pDoc     = obj->getDocument();
        tempSelObj.x        = sel.x;
        tempSelObj.y        = sel.y;
        tempSelObj.z        = sel.z;

        temp.push_back(tempSelObj);
    }

    return temp;
}

bool SelectionSingleton::hasSelection(const char* doc, bool resolve) const
{
    App::Document *pcDoc = 0;
    if(!doc || strcmp(doc,"*")!=0) {
        pcDoc = getDocument(doc);
        if (!pcDoc)
            return false;
    }
    for(auto &sel : _SelList) {
        if(!sel.pDoc) continue;
        auto obj = getObjectOfType(sel,App::DocumentObject::getClassTypeId(),resolve);
        if(obj && (!pcDoc || sel.pObject->getDocument()==pcDoc)) {
            return true;
        }
    }

    return false;
}

bool SelectionSingleton::hasSubSelection(const char* doc, bool subElement) const
{
    App::Document *pcDoc = 0;
    if(!doc || strcmp(doc,"*")!=0) {
        pcDoc = getDocument(doc);
        if (!pcDoc)
            return false;
    }
    for(auto &sel : _SelList) {
        if(pcDoc && pcDoc != sel.pDoc)
            continue;
        if(sel.SubName.empty())
            continue;
        if(subElement && sel.SubName.back()!='.')
            return true;
        if(sel.pObject != sel.pResolvedObject)
            return true;
    }

    return false;
}

std::vector<App::SubObjectT> SelectionSingleton::getPickedList(const char* pDocName) const
{
    std::vector<App::SubObjectT> res;

    App::Document *pcDoc = 0;
    if(!pDocName || strcmp(pDocName,"*")!=0) {
        pcDoc = getDocument(pDocName);
        if (!pcDoc)
            return res;
    }

    res.reserve(_PickedList.size());
    for(auto &sel : _PickedList) {
        if (!pcDoc || sel.pDoc == pcDoc) {
            res.emplace_back(sel.DocName.c_str(), sel.FeatName.c_str(), sel.SubName.c_str());
        }
    }
    return res;
}

std::vector<SelectionObject> SelectionSingleton::getSelectionEx(
        const char* pDocName, Base::Type typeId, int resolve, bool single) const {
    return getObjectList(pDocName,typeId,_SelList,resolve,single);
}

std::vector<SelectionObject> SelectionSingleton::getPickedListEx(const char* pDocName, Base::Type typeId) const {
    return getObjectList(pDocName,typeId,_PickedList,false);
}

std::vector<SelectionObject> SelectionSingleton::getObjectList(const char* pDocName, Base::Type typeId,
        std::list<_SelObj> &objList, int resolve, bool single) const
{
    std::vector<SelectionObject> temp;
    if(single) temp.reserve(1);
    std::map<App::DocumentObject*,size_t> SortMap;

    // check the type
    if (typeId == Base::Type::badType())
        return temp;

    App::Document *pcDoc = 0;
    if(!pDocName || strcmp(pDocName,"*")!=0) {
        pcDoc = getDocument(pDocName);
        if (!pcDoc)
            return temp;
    }

    for (auto &sel : objList) {
        if(!sel.pDoc) continue;
        const char *subelement = 0;
        auto obj = getObjectOfType(sel,typeId,resolve,&subelement);
        if(!obj || (pcDoc && sel.pObject->getDocument()!=pcDoc))
            continue;
        auto it = SortMap.find(obj);
        if(it!=SortMap.end()) {
            // only add sub-element
            if (subelement && *subelement) {
                if(resolve && !temp[it->second]._SubNameSet.insert(subelement).second)
                    continue;
                temp[it->second].SubNames.push_back(subelement);
                temp[it->second].SelPoses.emplace_back(sel.x,sel.y,sel.z);
            }
        }
        else {
            if(single && temp.size()) {
                temp.clear();
                break;
            }
            // create a new entry
            temp.emplace_back(obj);
            if (subelement && *subelement) {
                temp.back().SubNames.push_back(subelement);
                temp.back().SelPoses.emplace_back(sel.x,sel.y,sel.z);
                if(resolve)
                    temp.back()._SubNameSet.insert(subelement);
            }
            SortMap.insert(std::make_pair(obj,temp.size()-1));
        }
    }

    return temp;
}

bool SelectionSingleton::needPickedList() const {
    return _needPickedList;
}

void SelectionSingleton::enablePickedList(bool enable) {
    if(enable != _needPickedList) {
        _needPickedList = enable;
        _PickedList.clear();
        notify(SelectionChanges(SelectionChanges::PickedListChanged));
    }
}

void SelectionSingleton::notify(SelectionChanges &&Chng) {
    if(Notifying) {
        NotificationQueue.push_back(std::move(Chng));
        return;
    }
    Base::FlagToggler<bool> flag(Notifying);
    NotificationQueue.push_back(std::move(Chng));
    while(NotificationQueue.size()) {
        const auto &msg = NotificationQueue.front();
        bool notify;
        switch(msg.Type) {
        case SelectionChanges::AddSelection:
            notify = isSelected(msg.pDocName,msg.pObjectName,msg.pSubName,0);
            break;
        case SelectionChanges::RmvSelection:
            notify = !isSelected(msg.pDocName,msg.pObjectName,msg.pSubName,0);
            break;
        case SelectionChanges::SetPreselect:
            notify = CurrentPreselection.Type==SelectionChanges::SetPreselect 
                && CurrentPreselection.Object == msg.Object;
            break;
        case SelectionChanges::RmvPreselect:
            notify = CurrentPreselection.Type==SelectionChanges::ClrSelection;
            break;
        default:
            notify = true;
        }
        if(notify) {
            Notify(msg);
            try {
                signalSelectionChanged(msg);
            }
            catch (const boost::exception&) {
                // reported by code analyzers
                Base::Console().Warning("notify: Unexpected boost exception\n");
            }
        }
        NotificationQueue.pop_front();
    }
}

bool SelectionSingleton::hasPickedList() const {
    return _PickedList.size();
}

int SelectionSingleton::getAsPropertyLinkSubList(App::PropertyLinkSubList &prop) const
{
    std::vector<Gui::SelectionObject> sel = this->getSelectionEx();
    std::vector<App::DocumentObject*> objs; objs.reserve(sel.size()*2);
    std::vector<std::string> subs; subs.reserve(sel.size()*2);
    for (std::size_t iobj = 0; iobj < sel.size(); iobj++) {
        Gui::SelectionObject &selitem = sel[iobj];
        App::DocumentObject* obj = selitem.getObject();
        const std::vector<std::string> &subnames = selitem.getSubNames();
        if (subnames.size() == 0){//whole object is selected
            objs.push_back(obj);
            subs.emplace_back();
        } else {
            for (std::size_t isub = 0; isub < subnames.size(); isub++) {
                objs.push_back(obj);
                subs.push_back(subnames[isub]);
            }
        }
    }
    assert(objs.size()==subs.size());
    prop.setValues(objs, subs);
    return objs.size();
}

App::DocumentObject *SelectionSingleton::getObjectOfType(_SelObj &sel, 
        Base::Type typeId, int resolve, const char **subelement)
{
    auto obj = sel.pObject;
    if(!obj || !obj->getNameInDocument())
        return 0;
    const char *subname = sel.SubName.c_str();
    if(resolve) {
        obj = sel.pResolvedObject;
        if(resolve==2 && sel.elementName.first.size())
            subname = sel.elementName.first.c_str();
        else
            subname = sel.elementName.second.c_str();
    }
    if(!obj)
        return 0;
    if(!obj->isDerivedFrom(typeId) &&
       (resolve!=3 || !obj->getLinkedObject(true)->isDerivedFrom(typeId)))
        return 0;
    if(subelement) *subelement = subname;
    return obj;
}

vector<App::DocumentObject*> SelectionSingleton::getObjectsOfType(const Base::Type& typeId, const char* pDocName, int resolve) const
{
    std::vector<App::DocumentObject*> temp;

    App::Document *pcDoc = 0;
    if(!pDocName || strcmp(pDocName,"*")!=0) {
        pcDoc = getDocument(pDocName);
        if (!pcDoc)
            return temp;
    }

    std::set<App::DocumentObject*> objs;
    for(auto &sel : _SelList) {
        if(pcDoc && pcDoc!=sel.pDoc) continue;
        App::DocumentObject *pObject = getObjectOfType(sel,typeId,resolve);
        if (pObject) {
            auto ret = objs.insert(pObject);
            if(ret.second)
                temp.push_back(pObject);
        }
    }

    return temp;
}

std::vector<App::DocumentObject*> SelectionSingleton::getObjectsOfType(const char* typeName, const char* pDocName, int resolve) const
{
    Base::Type typeId = Base::Type::fromName(typeName);
    if (typeId == Base::Type::badType())
        return std::vector<App::DocumentObject*>();
    return getObjectsOfType(typeId, pDocName, resolve);
}

unsigned int SelectionSingleton::countObjectsOfType(const Base::Type& typeId, const char* pDocName, int resolve) const
{
    unsigned int iNbr=0;
    App::Document *pcDoc = 0;
    if(!pDocName || strcmp(pDocName,"*")!=0) {
        pcDoc = getDocument(pDocName);
        if (!pcDoc)
            return 0;
    }

    for (auto &sel : _SelList) {
        if((!pcDoc||pcDoc==sel.pDoc) && getObjectOfType(sel,typeId,resolve))
            iNbr++;
    }

    return iNbr;
}

unsigned int SelectionSingleton::countObjectsOfType(const char* typeName, const char* pDocName, int resolve) const
{
    if (!typeName)
        return size();
    Base::Type typeId = Base::Type::fromName(typeName);
    if (typeId == Base::Type::badType())
        return 0;
    return countObjectsOfType(typeId, pDocName, resolve);
}


void SelectionSingleton::slotSelectionChanged(const SelectionChanges& msg) {
    if(msg.Type == SelectionChanges::ShowSelection ||
       msg.Type == SelectionChanges::HideSelection)
        return;
    
    if(msg.Object.getSubName().size()) {
        auto pParent = msg.Object.getObject();
        if(!pParent) return;
        std::pair<std::string,std::string> elementName;
        auto &newElementName = elementName.first;
        auto &oldElementName = elementName.second;
        auto pObject = App::GeoFeature::resolveElement(pParent,msg.pSubName,elementName);
        if (!pObject) return;
        SelectionChanges msg2(msg.Type,pObject->getDocument()->getName(),
                pObject->getNameInDocument(),
                newElementName.size()?newElementName.c_str():oldElementName.c_str(),
                pObject->getTypeId().getName(), msg.x,msg.y,msg.z);

        try {
            msg2.pOriginalMsg = &msg;
            signalSelectionChanged3(msg2);

            msg2.Object.setSubName(oldElementName.c_str());
            msg2.pSubName = msg2.Object.getSubName().c_str();
            signalSelectionChanged2(msg2);
        }
        catch (const boost::exception&) {
            // reported by code analyzers
            Base::Console().Warning("slotSelectionChanged: Unexpected boost exception\n");
        }
    }
    else {
        try {
            signalSelectionChanged3(msg);
            signalSelectionChanged2(msg);
        }
        catch (const boost::exception&) {
            // reported by code analyzers
            Base::Console().Warning("slotSelectionChanged: Unexpected boost exception\n");
        }
    }
}

void SelectionSingleton::setContext(const App::SubObjectT &sobj)
{
    ContextObject = sobj;
}

const App::SubObjectT &SelectionSingleton::getContext() const
{
    return ContextObject;
}

App::SubObjectT SelectionSingleton::getExtendedContext(App::DocumentObject *obj) const
{
    auto sobj = ContextObject.getSubObject();
    if (!obj || obj == sobj)
        return ContextObject;

    sobj = CurrentPreselection.Object.getSubObject();
    if (!obj || obj == sobj)
        return CurrentPreselection.Object;

    if (!obj) {
        auto sels = getSelectionT(nullptr, 0, true);
        if (sels.size())
            return sels.front();
    } else {
        for (auto &sel : getSelectionT(nullptr, 0)) {
            sobj = sel.getSubObject();
            if (sobj == obj)
                return sel;
        }
    }

    auto gdoc = Application::Instance->editDocument();
    if (gdoc && gdoc == Application::Instance->activeDocument()) {
        auto objT = gdoc->getInEditT();
        sobj = objT.getSubObject();
        if (!obj || sobj == obj)
            return objT;
    }

    return App::SubObjectT();
}

int SelectionSingleton::setPreselect(const char* pDocName, const char* pObjectName, const char* pSubName,
                                     float x, float y, float z, int signal, bool msg)
{
    if(!pDocName || !pObjectName) {
        rmvPreselect();
        return 0;
    }
    if(!pSubName) pSubName = "";

    if(DocName==pDocName && FeatName==pObjectName && SubName==pSubName) {
        if(hx!=x || hy!=y || hz!=z) {
            hx = x;
            hy = y;
            hz = z;
            CurrentPreselection.x = x;
            CurrentPreselection.y = y;
            CurrentPreselection.z = z;

            if (msg)
                format(0,0,0,x,y,z,true);

            // MovePreselect is likely going to slow down large scene rendering.
            // Disable it for now.
#if 0
            SelectionChanges Chng(SelectionChanges::MovePreselect,
                    DocName,FeatName,SubName,std::string(),x,y,z);
            notify(Chng);
#endif
        }
        return -1;
    }

    rmvPreselect();

    if (ActiveGate && signal!=1) {
        App::Document* pDoc = getDocument(pDocName);
        if (!pDoc || !pObjectName) 
            return 0;
        std::pair<std::string,std::string> elementName;
        auto pObject = pDoc->getObject(pObjectName);
        if(!pObject)
            return 0;

        const char *subelement = pSubName;
        if(gateResolve) {
            auto &newElementName = elementName.first;
            auto &oldElementName = elementName.second;
            pObject = App::GeoFeature::resolveElement(pObject,pSubName,elementName);
            if (!pObject)
                return 0;
            if(gateResolve > 1)
                subelement = newElementName.size()?newElementName.c_str():oldElementName.c_str();
            else
                subelement = oldElementName.c_str();
        }
        if (!ActiveGate->allow(pObject->getDocument(),pObject,subelement)) {
            QString msg;
            if (ActiveGate->notAllowedReason.length() > 0){
                msg = QObject::tr(ActiveGate->notAllowedReason.c_str());
            } else {
                msg = QCoreApplication::translate("SelectionFilter","Not allowed:");
            }
            auto sobjT = App::SubObjectT(pDocName, pObjectName, pSubName);
            msg.append(QString::fromLatin1(" %1").arg(
                        QString::fromUtf8(sobjT.getSubObjectFullName().c_str())));

            if (getMainWindow()) {
                getMainWindow()->showMessage(msg);
                Gui::MDIView* mdi = Gui::Application::Instance->activeDocument()->getActiveView();
                mdi->setOverrideCursor(QCursor(Qt::ForbiddenCursor));
            }
            return -2;
        }
        Gui::MDIView* mdi = Gui::Application::Instance->activeDocument()->getActiveView();
        mdi->restoreOverrideCursor();
    }

    DocName = pDocName;
    FeatName= pObjectName;
    SubName = pSubName;
    hx = x;
    hy = y;
    hz = z;

    // set up the change object
    SelectionChanges Chng(SelectionChanges::SetPreselect,
            DocName,FeatName,SubName,std::string(),x,y,z,signal);

    CurrentPreselection = Chng;

    if (msg)
        format(0,0,0,x,y,z,true);

    FC_TRACE("preselect "<<DocName<<'#'<<FeatName<<'.'<<SubName);
    notify(Chng);

    // It is possible the preselect is removed during notification
    return DocName.empty()?0:1;
}

void SelectionSingleton::setPreselectCoord( float x, float y, float z)
{
    // if nothing is in preselect ignore
    if(CurrentPreselection.Object.getObjectName().empty()) return;

    CurrentPreselection.x = x;
    CurrentPreselection.y = y;
    CurrentPreselection.z = z;

    format(0,0,0,x,y,z,true);
}

QString SelectionSingleton::format(App::DocumentObject *obj,
                                   const char *subname, 
                                   float x, float y, float z,
                                   bool show)
{
    if (!obj || !obj->getNameInDocument())
        return QString();
    return format(obj->getDocument()->getName(), obj->getNameInDocument(), subname, x, y, z, show);
}

QString SelectionSingleton::format(const char *docname,
                                   const char *objname,
                                   const char *subname, 
                                   float x, float y, float z,
                                   bool show)
{
    App::SubObjectT objT(docname?docname:DocName.c_str(),
                         objname?objname:FeatName.c_str(),
                         subname?subname:SubName.c_str());

    QString text;
    QTextStream ts(&text);

    auto sobj = objT.getSubObject();
    if (sobj) {
        int index = -1;
        std::string element = objT.getOldElementName(&index);
        ts << sobj->getNameInDocument();
        if (index > 0)
            ts << "." << element.c_str() << index;
        ts << " | ";
        if (sobj->Label.getStrValue() != sobj->getNameInDocument())
            ts << QString::fromUtf8(sobj->Label.getValue()) << " | ";
    }
    if(x != 0. || y != 0. || z != 0.) {
        auto fmt = [this](double v) -> QString {
            Base::Quantity q(v, Base::Quantity::MilliMetre.getUnit());
            double factor;
            QString unit;
            Base::UnitsApi::schemaTranslate(q, factor, unit);
            QLocale Lc;
            const Base::QuantityFormat& format = q.getFormat();
            if (format.option != Base::QuantityFormat::None) {
                uint opt = static_cast<uint>(format.option);
                Lc.setNumberOptions(static_cast<QLocale::NumberOptions>(opt));
            }
            return QString::fromLatin1("%1 %2").arg(
                        Lc.toString(v/factor, format.toFormat(),
                                    fmtDecimal<0 ? format.precision : fmtDecimal),
                        unit);
        };
        ts << fmt(x) << "; " << fmt(y) << "; " << fmt(z) << QLatin1String(" | ");
    }

    ts << objT.getDocumentName().c_str() << "#" 
       << objT.getObjectName().c_str() << "."
       << objT.getSubName().c_str();

    PreselectionText.clear();
    if (show && getMainWindow()) {
        getMainWindow()->showMessage(text);

        std::vector<std::string> cmds;
        const auto &cmdmap = Gui::Application::Instance->commandManager().getCommands();
        for (auto it = cmdmap.upper_bound("Std_Macro_Presel"); it != cmdmap.end(); ++it) {
            if (!boost::starts_with(it->first, "Std_Macro_Presel"))
                break;
            auto cmd = dynamic_cast<MacroCommand*>(it->second);
            if (cmd && cmd->isPreselectionMacro() && cmd->getOption())
                cmds.push_back(it->first);
        }

        for (auto &name : cmds) {
            auto cmd = dynamic_cast<MacroCommand*>(
                    Gui::Application::Instance->commandManager().getCommandByName(name.c_str()));
            if (cmd)
                cmd->invoke(2);
        }
        if (cmds.empty()) 
            ToolTip::hideText();
        else {
            QPoint pt(ViewParams::getPreselectionToolTipOffsetX(),
                      ViewParams::getPreselectionToolTipOffsetY());
            ToolTip::showText(pt,
                              QString::fromUtf8(PreselectionText.c_str()),
                              Application::Instance->activeView(),
                              true,
                              (ToolTip::Corner)ViewParams::getPreselectionToolTipCorner());
        }
    }

    return text;
}

const std::string &SelectionSingleton::getPreselectionText() const
{
    return PreselectionText;
}

void SelectionSingleton::setPreselectionText(const std::string &txt) 
{
    PreselectionText = txt;
}

void SelectionSingleton::setFormatDecimal(int d)
{
    fmtDecimal = d;
}

void SelectionSingleton::rmvPreselect()
{
    if (DocName == "")
        return;

    SelectionChanges Chng(SelectionChanges::RmvPreselect,DocName,FeatName,SubName);

    // reset the current preselection
    CurrentPreselection = SelectionChanges();

    DocName = "";
    FeatName= "";
    SubName = "";
    hx = 0;
    hy = 0;
    hz = 0;

    if (ActiveGate && getMainWindow()) {
        Gui::MDIView* mdi = Gui::Application::Instance->activeDocument()->getActiveView();
        mdi->restoreOverrideCursor();
    }

    FC_TRACE("rmv preselect");

    // notify observing objects
    notify(std::move(Chng));

}

const SelectionChanges &SelectionSingleton::getPreselection(void) const
{
    return CurrentPreselection;
}

// add a SelectionGate to control what is selectable
void SelectionSingleton::addSelectionGate(Gui::SelectionGate *gate, int resolve)
{
    if (ActiveGate)
        rmvSelectionGate();
    
    ActiveGate = gate;
    gateResolve = resolve;
}

// remove the active SelectionGate
void SelectionSingleton::rmvSelectionGate(void)
{
    if (ActiveGate) {
        // Delay deletion to avoid recursion
        std::unique_ptr<Gui::SelectionGate> guard(ActiveGate);
        ActiveGate = nullptr;
        Gui::Document* doc = Gui::Application::Instance->activeDocument();
        if (doc) {
            // if a document is about to be closed it has no MDI view any more
            Gui::MDIView* mdi = doc->getActiveView();
            if (mdi)
                mdi->restoreOverrideCursor();
        }
    }
}


App::Document* SelectionSingleton::getDocument(const char* pDocName) const
{
    if (pDocName && pDocName[0])
        return App::GetApplication().getDocument(pDocName);
    else
        return App::GetApplication().getActiveDocument();
}

int SelectionSingleton::disableCommandLog() {
    if(!logDisabled)
        logHasSelection = hasSelection();
    return ++logDisabled;
}

int SelectionSingleton::enableCommandLog(bool silent) {
    --logDisabled;
    if(!logDisabled && !silent) {
        auto manager = Application::Instance->macroManager();
        if(!hasSelection()) {
            if(logHasSelection) 
                manager->addLine(MacroManager::Cmt, "Gui.Selection.clearSelection()");
        }else{
            for(auto &sel : _SelList)
                sel.log();
        }
    }
    return logDisabled;
}

void SelectionSingleton::_SelObj::log(bool remove, bool clearPreselect) {
    if(logged && !remove) 
        return;
    logged = true;
    std::ostringstream ss;
    ss << "Gui.Selection." << (remove?"removeSelection":"addSelection")
        << "('" << DocName  << "','" << FeatName << "'";
    if(SubName.size()) {
        // Use old style indexed based selection may have ambiguity, e.g in
        // sketch, where the editing geometry and shape geometry may have the
        // same sub element name but refers to different geometry element.
#if 0
        if(elementName.second.size() && elementName.first.size())
            ss << ",'" << SubName.substr(0,SubName.size()-elementName.first.size()) 
                << elementName.second << "'";
        else
#endif
            ss << ",'" << SubName << "'";
    }
    if(!remove && (x || y || z || !clearPreselect)) {
        if(SubName.empty())
            ss << ",''";
        ss << ',' << x << ',' << y << ',' << z;
        if(!clearPreselect)
            ss << ",False";
    }
    ss << ')';
    Application::Instance->macroManager()->addLine(MacroManager::Cmt, ss.str().c_str());
}

bool SelectionSingleton::addSelection(const char* pDocName, const char* pObjectName, 
        const char* pSubName, float x, float y, float z, 
        const std::vector<SelObj> *pickedList, bool clearPreselect)
{
    if(pickedList) {
        _PickedList.clear();
        for(const auto &sel : *pickedList) {
            _PickedList.emplace_back();
            auto &s = _PickedList.back();
            s.DocName = sel.DocName;
            s.FeatName = sel.FeatName;
            s.SubName = sel.SubName;
            s.TypeName = sel.TypeName;
            s.pObject = sel.pObject;
            s.pDoc = sel.pDoc;
            s.x = sel.x;
            s.y = sel.y;
            s.z = sel.z;
        }
        notify(SelectionChanges(SelectionChanges::PickedListChanged));
    }

    _SelObj temp;
    int ret = checkSelection(pDocName,pObjectName,pSubName,0,temp);
    if(ret!=0)
        return false;

    temp.x        = x;
    temp.y        = y;
    temp.z        = z;

    // check for a Selection Gate
    if (ActiveGate) {
        const char *subelement = 0;
        auto pObject = getObjectOfType(temp,App::DocumentObject::getClassTypeId(),gateResolve,&subelement);
        if (!ActiveGate->allow(pObject?pObject->getDocument():temp.pDoc,pObject,subelement)) {
            if (getMainWindow()) {
                QString msg;
                if (ActiveGate->notAllowedReason.length() > 0) {
                    msg = QObject::tr(ActiveGate->notAllowedReason.c_str());
                } else {
                    msg = QCoreApplication::translate("SelectionFilter","Selection not allowed by filter");
                }
                getMainWindow()->showMessage(msg);
                Gui::MDIView* mdi = Gui::Application::Instance->activeDocument()->getActiveView();
                mdi->setOverrideCursor(Qt::ForbiddenCursor);
            }
            ActiveGate->notAllowedReason.clear();
            QApplication::beep();
            return false;
        }
    }

    if(!logDisabled)
        temp.log(false,clearPreselect);

    _SelList.push_back(temp);
    _SelStackForward.clear();

    if(clearPreselect)
        rmvPreselect();

    SelectionChanges Chng(SelectionChanges::AddSelection,
            temp.DocName,temp.FeatName,temp.SubName,temp.TypeName, x,y,z);

    FC_LOG("Add Selection "<<Chng.pDocName<<'#'<<Chng.pObjectName<<'.'<<Chng.pSubName
            << " (" << x << ", " << y << ", " << z << ')');

    notify(std::move(Chng));

    getMainWindow()->updateActions();

    // There is a possibility that some observer removes or clears selection
    // inside signal handler, hence the check here
    return isSelected(temp.DocName.c_str(),temp.FeatName.c_str(), temp.SubName.c_str());
}

void SelectionSingleton::selStackPush(bool clearForward, bool overwrite) {
    static int stackSize;
    if(!stackSize) {
        stackSize = App::GetApplication().GetParameterGroupByPath
                ("User parameter:BaseApp/Preferences/View")->GetInt("SelectionStackSize",100);
    }
    if(clearForward)
        _SelStackForward.clear();
    if(_SelList.empty())
        return;
    if((int)_SelStackBack.size() >= stackSize)
        _SelStackBack.pop_front();
    SelStackItem item;
    for(auto &sel : _SelList)
        item.emplace(sel.DocName.c_str(),sel.FeatName.c_str(),sel.SubName.c_str());
    if(_SelStackBack.size() && _SelStackBack.back()==item)
        return;
    if(!overwrite || _SelStackBack.empty())
        _SelStackBack.emplace_back();
    _SelStackBack.back() = std::move(item);
}

void SelectionSingleton::selStackGoBack(int count) {
    if((int)_SelStackBack.size()<count)
        count = _SelStackBack.size();
    if(count<=0)
        return;
    if(_SelList.size()) {
        selStackPush(false,true);
        clearCompleteSelection();
    } else 
        --count;
    for(int i=0;i<count;++i) {
        _SelStackForward.push_front(std::move(_SelStackBack.back()));
        _SelStackBack.pop_back();
    }
    std::deque<SelStackItem> tmpStack;
    _SelStackForward.swap(tmpStack);
    while(_SelStackBack.size()) {
        bool found = false;
        for(auto &sobjT : _SelStackBack.back()) {
            if(sobjT.getSubObject()) {
                addSelection(sobjT.getDocumentName().c_str(),
                             sobjT.getObjectName().c_str(),
                             sobjT.getSubName().c_str());
                found = true;
            }
        }
        if(found)
            break;
        tmpStack.push_front(std::move(_SelStackBack.back()));
        _SelStackBack.pop_back();
    }
    _SelStackForward = std::move(tmpStack);
    getMainWindow()->updateActions();
}

void SelectionSingleton::selStackGoForward(int count) {
    if((int)_SelStackForward.size()<count)
        count = _SelStackForward.size();
    if(count<=0)
        return;
    if(_SelList.size()) {
        selStackPush(false,true);
        clearCompleteSelection();
    }
    for(int i=0;i<count;++i) {
        _SelStackBack.push_back(_SelStackForward.front());
        _SelStackForward.pop_front();
    }
    std::deque<SelStackItem> tmpStack;
    _SelStackForward.swap(tmpStack);
    while(1) {
        bool found = false;
        for(auto &sobjT : _SelStackBack.back()) {
            if(sobjT.getSubObject()) {
                addSelection(sobjT.getDocumentName().c_str(),
                             sobjT.getObjectName().c_str(),
                             sobjT.getSubName().c_str());
                found = true;
            }
        }
        if(found || tmpStack.empty()) 
            break;
        _SelStackBack.push_back(tmpStack.front());
        tmpStack.pop_front();
    }
    _SelStackForward = std::move(tmpStack);
    getMainWindow()->updateActions();
}

std::vector<SelectionObject> SelectionSingleton::selStackGet(
        const char* pDocName, int resolve, int index) const
{
    const SelStackItem *item = 0;
    if(index>=0) {
        if(index>=(int)_SelStackBack.size())
            return {};
        item = &_SelStackBack[_SelStackBack.size()-1-index];
    }else{
        index = -index-1;
        if(index>=(int)_SelStackForward.size())
            return {};
        item = &_SelStackBack[_SelStackForward.size()-1-index];
    }
    
    std::list<_SelObj> selList;
    for(auto &sobjT : *item) {
        _SelObj sel;
        if(checkSelection(sobjT.getDocumentName().c_str(),
                          sobjT.getObjectName().c_str(),
                          sobjT.getSubName().c_str(),
                          0,
                          sel,
                          &selList)==0)
        {
            selList.push_back(sel);
        }
    }

    return getObjectList(pDocName,App::DocumentObject::getClassTypeId(),selList,resolve);
}

bool SelectionSingleton::addSelections(const char* pDocName, const char* pObjectName, const std::vector<std::string>& pSubNames)
{
    if(_PickedList.size()) {
        _PickedList.clear();
        notify(SelectionChanges(SelectionChanges::PickedListChanged));
    }

    bool update = false;
    for(std::vector<std::string>::const_iterator it = pSubNames.begin(); it != pSubNames.end(); ++it) {
        _SelObj temp;
        int ret = checkSelection(pDocName,pObjectName,it->c_str(),0,temp);
        if(ret!=0)
            continue;

        temp.x        = 0;
        temp.y        = 0;
        temp.z        = 0;

        _SelList.push_back(temp);
        _SelStackForward.clear();

        SelectionChanges Chng(SelectionChanges::AddSelection,
                temp.DocName,temp.FeatName,temp.SubName,temp.TypeName);

        FC_LOG("Add Selection "<<Chng.pDocName<<'#'<<Chng.pObjectName<<'.'<<Chng.pSubName);

        notify(std::move(Chng));
        update = true;
    }

    if(update)
        getMainWindow()->updateActions();
    return true;
}

bool SelectionSingleton::updateSelection(bool show, const char* pDocName, 
                            const char* pObjectName, const char* pSubName)
{
    if(!pDocName || !pObjectName)
        return false;
    if(!pSubName)
        pSubName = "";
    auto pDoc = getDocument(pDocName);
    if(!pDoc) return false;
    auto pObject = pDoc->getObject(pObjectName);
    if(!pObject) return false;
    _SelObj sel;
    if(checkSelection(pDocName,pObjectName,pSubName,0,sel,&_SelList)<=0)
        return false;
    if(DocName==sel.DocName && FeatName==sel.FeatName && SubName==sel.SubName) {
        if(show) {
            FC_TRACE("preselect signal");
            notify(SelectionChanges(SelectionChanges::SetPreselect,DocName,FeatName,SubName));
        }else
            rmvPreselect();
    }
    SelectionChanges Chng(show?SelectionChanges::ShowSelection:SelectionChanges::HideSelection,
            sel.DocName, sel.FeatName, sel.SubName, sel.pObject->getTypeId().getName());

    FC_LOG("Update Selection "<<Chng.pDocName << '#' << Chng.pObjectName << '.' <<Chng.pSubName);

    notify(std::move(Chng));

    return true;
}

bool SelectionSingleton::addSelection(const SelectionObject& obj,bool clearPreselect)
{
    const std::vector<std::string>& subNames = obj.getSubNames();
    const std::vector<Base::Vector3d> points = obj.getPickedPoints();
    if (!subNames.empty() && subNames.size() == points.size()) {
        bool ok = true;
        for (std::size_t i=0; i<subNames.size(); i++) {
            const std::string& name = subNames[i];
            const Base::Vector3d& pnt = points[i];
            ok &= addSelection(obj.getDocName(), obj.getFeatName(), name.c_str(),
                               static_cast<float>(pnt.x),
                               static_cast<float>(pnt.y),
                               static_cast<float>(pnt.z),0,clearPreselect);
        }
        return ok;
    }
    else if (!subNames.empty()) {
        bool ok = true;
        for (std::size_t i=0; i<subNames.size(); i++) {
            const std::string& name = subNames[i];
            ok &= addSelection(obj.getDocName(), obj.getFeatName(), name.c_str());
        }
        return ok;
    }
    else {
        return addSelection(obj.getDocName(), obj.getFeatName());
    }
}


void SelectionSingleton::rmvSelection(const char* pDocName, const char* pObjectName, const char* pSubName, 
        const std::vector<SelObj> *pickedList)
{
    if(pickedList) {
        _PickedList.clear();
        for(const auto &sel : *pickedList) {
            _PickedList.emplace_back();
            auto &s = _PickedList.back();
            s.DocName = sel.DocName;
            s.FeatName = sel.FeatName;
            s.SubName = sel.SubName;
            s.TypeName = sel.TypeName;
            s.pObject = sel.pObject;
            s.pDoc = sel.pDoc;
            s.x = sel.x;
            s.y = sel.y;
            s.z = sel.z;
        }
        notify(SelectionChanges(SelectionChanges::PickedListChanged));
    }

    if(!pDocName) return;

    _SelObj temp;
    int ret = checkSelection(pDocName,pObjectName,pSubName,0,temp);
    if(ret<0)
        return;

    std::vector<SelectionChanges> changes;
    for(auto It=_SelList.begin(),ItNext=It;It!=_SelList.end();It=ItNext) {
        ++ItNext;
        if(It->DocName!=temp.DocName || It->FeatName!=temp.FeatName) 
            continue;
        // if no subname is specified, remove all subobjects of the matching object
        if(temp.SubName.size()) {
            // otherwise, match subojects with common prefix, separated by '.'
            if(!boost::starts_with(It->SubName,temp.SubName) ||
               (It->SubName.length()!=temp.SubName.length() && It->SubName[temp.SubName.length()-1]!='.'))
                continue;
        }

        It->log(true);

        changes.emplace_back(SelectionChanges::RmvSelection,
                It->DocName,It->FeatName,It->SubName,It->TypeName);

        // destroy the _SelObj item
        _SelList.erase(It);
    }

    // NOTE: It can happen that there are nested calls of rmvSelection()
    // so that it's not safe to invoke the notifications inside the loop
    // as this can invalidate the iterators and thus leads to undefined
    // behaviour.
    // So, the notification is done after the loop, see also #0003469
    if(changes.size()) {
        for(auto &Chng : changes) {
            FC_LOG("Rmv Selection "<<Chng.pDocName<<'#'<<Chng.pObjectName<<'.'<<Chng.pSubName);
            notify(std::move(Chng));
        }
        getMainWindow()->updateActions();
    }
}

struct SelInfo {
    std::string DocName;
    std::string FeatName;
    std::string SubName;
    SelInfo(const std::string &docName,
            const std::string &featName,
            const std::string &subName)
        :DocName(docName)
        ,FeatName(featName)
        ,SubName(subName)
    {}
};

void SelectionSingleton::setVisible(VisibleState vis, const std::vector<App::SubObjectT> &_sels) {
    std::set<std::pair<App::DocumentObject*,App::DocumentObject*> > filter;
    int visible;
    switch(vis) {
    case VisShow:
        visible = 1;
        break;
    case VisToggle:
        visible = -1;
        break;
    default:
        visible = 0;
    }

    const auto &sels = _sels.size()?_sels:getSelectionT(0,0);
    for(auto &sel : sels) {
        App::DocumentObject *obj = sel.getObject();
        if(!obj) continue;

        // get parent object
        App::DocumentObject *parent = 0;
        std::string elementName;
        obj = obj->resolve(sel.getSubName().c_str(),&parent,&elementName);
        if (!obj || !obj->getNameInDocument() || (parent && !parent->getNameInDocument()))
            continue;
        // try call parent object's setElementVisible
        if (parent) {
            // prevent setting the same object visibility more than once
            if (!filter.insert(std::make_pair(obj,parent)).second)
                continue;
            int visElement = parent->isElementVisible(elementName.c_str());
            if (visElement >= 0) {
                if (visElement > 0)
                    visElement = 1;
                if (visible >= 0) {
                    if (visElement == visible)
                        continue;
                    visElement = visible;
                }
                else {
                    visElement = !visElement;
                }

                if(!visElement)
                    updateSelection(false,
                                    sel.getDocumentName().c_str(),
                                    sel.getObjectName().c_str(),
                                    sel.getSubName().c_str());

                parent->setElementVisible(elementName.c_str(),visElement?true:false);

                if(visElement && ViewParams::instance()->getUpdateSelectionVisual())
                    updateSelection(true,
                                    sel.getDocumentName().c_str(),
                                    sel.getObjectName().c_str(),
                                    sel.getSubName().c_str());
                continue;
            }

            // Fall back to direct object visibility setting
        }
        if(!filter.insert(std::make_pair(obj,(App::DocumentObject*)0)).second){
            continue;
        }

        auto vp = Application::Instance->getViewProvider(obj);

        if(vp) {
            bool visObject;
            if(visible>=0)
                visObject = visible ? true : false;
            else
                visObject = !vp->isShow();

            SelectionNoTopParentCheck guard;
            if(visObject) {
                vp->show();
                if(ViewParams::instance()->getUpdateSelectionVisual())
                    updateSelection(true,
                                    sel.getDocumentName().c_str(),
                                    sel.getObjectName().c_str(),
                                    sel.getSubName().c_str());
            } else {
                updateSelection(false,
                                sel.getDocumentName().c_str(),
                                sel.getObjectName().c_str(),
                                sel.getSubName().c_str());
                vp->hide();
            }
        }
    }
}

void SelectionSingleton::setSelection(const char* pDocName, const std::vector<App::DocumentObject*>& sel)
{
    App::Document *pcDoc;
    pcDoc = getDocument(pDocName);
    if (!pcDoc)
        return;

    if(_PickedList.size()) {
        _PickedList.clear();
        notify(SelectionChanges(SelectionChanges::PickedListChanged));
    }

    bool touched = false;
    for(auto obj : sel) {
        if(!obj || !obj->getNameInDocument())
            continue;
        _SelObj temp;
        int ret = checkSelection(pDocName,obj->getNameInDocument(),0,0,temp);
        if(ret!=0)
            continue;
        touched = true;
        _SelList.push_back(temp);
    }

    if(touched) {
        _SelStackForward.clear();
        notify(SelectionChanges(SelectionChanges::SetSelection,pDocName));
        getMainWindow()->updateActions();
    }
}

void SelectionSingleton::clearSelection(const char* pDocName, bool clearPreSelect)
{
    // Because the introduction of external editing, it is best to make
    // clearSelection(0) behave as clearCompleteSelection(), which is the same
    // behavior of python Selection.clearSelection(None)
    if (!pDocName || !pDocName[0] || strcmp(pDocName,"*")==0) {
        clearCompleteSelection(clearPreSelect);
        return;
    }

    if (_PickedList.size()) {
        _PickedList.clear();
        notify(SelectionChanges(SelectionChanges::PickedListChanged));
    }

    App::Document* pDoc;
    pDoc = getDocument(pDocName);
    if (pDoc) {
        std::string docName = pDocName;
        if (clearPreSelect && DocName == docName)
            rmvPreselect();

        bool touched = false;
        for (auto it=_SelList.begin();it!=_SelList.end();) {
            if (it->DocName == docName) {
                touched = true;
                it = _SelList.erase(it);
            }
            else {
                ++it;
            }
        }

        if (!touched)
            return;

        if (!logDisabled) {
            std::ostringstream ss;
            ss << "Gui.Selection.clearSelection('" << docName << "'";
            if (!clearPreSelect)
                ss << ", False";
            ss << ')';
            Application::Instance->macroManager()->addLine(MacroManager::Cmt,ss.str().c_str());
        }

        notify(SelectionChanges(SelectionChanges::ClrSelection,docName.c_str()));

        getMainWindow()->updateActions();
    }
}

void SelectionSingleton::clearCompleteSelection(bool clearPreSelect)
{
    if(_PickedList.size()) {
        _PickedList.clear();
        notify(SelectionChanges(SelectionChanges::PickedListChanged));
    }

    if(clearPreSelect)
        rmvPreselect();

    if(_SelList.empty())
        return;

    if(!logDisabled)
        Application::Instance->macroManager()->addLine(MacroManager::Cmt,
                clearPreSelect?"Gui.Selection.clearSelection()"
                              :"Gui.Selection.clearSelection(False)");

    _SelList.clear();

    SelectionChanges Chng(SelectionChanges::ClrSelection);

    FC_LOG("Clear selection");

    notify(std::move(Chng));
    getMainWindow()->updateActions();
}

bool SelectionSingleton::isSelected(const char* pDocName, 
        const char* pObjectName, const char* pSubName, int resolve) const
{
    _SelObj sel;
    return checkSelection(pDocName,pObjectName,pSubName,resolve,sel,&_SelList)>0;
}

bool SelectionSingleton::isSelected(App::DocumentObject* pObject, const char* pSubName, int resolve) const
{
    if(!pObject || !pObject->getNameInDocument() || !pObject->getDocument()) 
        return false;
    _SelObj sel;
    return checkSelection(pObject->getDocument()->getName(),
            pObject->getNameInDocument(),pSubName,resolve,sel,&_SelList)>0;
}

void SelectionSingleton::checkTopParent(App::DocumentObject *&obj, std::string &subname) {
    TreeWidget::checkTopParent(obj,subname);
}

int SelectionSingleton::checkSelection(const char *pDocName, const char *pObjectName, 
        const char *pSubName, int resolve, _SelObj &sel, const std::list<_SelObj> *selList) const
{
    sel.pDoc = getDocument(pDocName);
    if(!sel.pDoc) {
        if(!selList)
            FC_ERR("Cannot find document");
        return -1;
    }
    pDocName = sel.pDoc->getName();
    sel.DocName = pDocName;

    if(pObjectName)
        sel.pObject = sel.pDoc->getObject(pObjectName);
    else
        sel.pObject = 0;
    if (!sel.pObject) {
        if(!selList)
            FC_ERR("Object not found");
        return -1;
    }
    if(sel.pObject->testStatus(App::ObjectStatus::Remove))
        return -1;
    if(pSubName)
       sel.SubName = pSubName;
    if(!resolve && !SelectionNoTopParentCheck::enabled())
        checkTopParent(sel.pObject,sel.SubName);
    pSubName = sel.SubName.size()?sel.SubName.c_str():0;
    sel.FeatName = sel.pObject->getNameInDocument();
    sel.TypeName = sel.pObject->getTypeId().getName();
    const char *element = 0;
    sel.pResolvedObject = App::GeoFeature::resolveElement(sel.pObject,
            pSubName,sel.elementName,false,App::GeoFeature::Normal,0,&element);
    if(!sel.pResolvedObject) {
        if(!selList)
            FC_ERR("Sub-object " << sel.DocName << '#' << sel.FeatName << '.' << sel.SubName << " not found");
        return -1;
    }
    if(sel.pResolvedObject->testStatus(App::ObjectStatus::Remove))
        return -1;
    std::string subname;
    std::string prefix;
    if(pSubName && element) {
        prefix = std::string(pSubName, element-pSubName);
        if(sel.elementName.first.size()) {
            // make sure the selected sub name is a new style if available
            subname = prefix + sel.elementName.first;
            pSubName = subname.c_str();
            sel.SubName = subname;
        }
    }
    if(!selList)
        selList = &_SelList;

    if(!pSubName)
        pSubName = "";

    for (auto &s : *selList) {
        if (s.DocName==pDocName && s.FeatName==sel.FeatName) {
            if(s.SubName==pSubName)
                return 1;
            if(resolve>1 && boost::starts_with(s.SubName,prefix))
                return 1;
        }
    }
    if(resolve==1) {
        for(auto &s : *selList) {
            if(s.pResolvedObject != sel.pResolvedObject)
                continue;
            if(!pSubName[0]) 
                return 1;
            if(s.elementName.first.size()) {
                if(s.elementName.first == sel.elementName.first)
                    return 1;
            }else if(s.SubName == sel.elementName.second)
                return 1;
        }
    }
    return 0;
}

const char *SelectionSingleton::getSelectedElement(App::DocumentObject *obj, const char* pSubName) const 
{
    if (!obj) return 0;

    std::pair<std::string,std::string> elementName;
    if(!App::GeoFeature::resolveElement(obj,pSubName,elementName,true))
        return 0;

    if(elementName.first.size())
        pSubName = elementName.first.c_str();

    for(list<_SelObj>::const_iterator It = _SelList.begin();It != _SelList.end();++It) {
        if (It->pObject == obj) {
            auto len = It->SubName.length();
            if(!len)
                return "";
            if (pSubName && strncmp(pSubName,It->SubName.c_str(),It->SubName.length())==0){
                if(pSubName[len]==0 || pSubName[len-1] == '.')
                    return It->SubName.c_str();
            }
        }
    }
    return 0;
}

void SelectionSingleton::slotDeletedObject(const App::DocumentObject& Obj)
{
    if(!Obj.getNameInDocument()) return;

    // For safety reason, don't bother checking
    rmvPreselect();

    // Remove also from the selection, if selected
    // We don't walk down the hierarchy for each selection, so there may be stray selection
    std::vector<SelectionChanges> changes;
    for(auto it=_SelList.begin(),itNext=it;it!=_SelList.end();it=itNext) {
        ++itNext;
        if(it->pResolvedObject == &Obj || it->pObject==&Obj) {
            changes.emplace_back(SelectionChanges::RmvSelection,
                    it->DocName,it->FeatName,it->SubName,it->TypeName);
            _SelList.erase(it);
        }
    }
    if(changes.size()) {
        for(auto &Chng : changes) {
            FC_LOG("Rmv Selection "<<Chng.pDocName<<'#'<<Chng.pObjectName<<'.'<<Chng.pSubName);
            notify(std::move(Chng));
        }
        getMainWindow()->updateActions();
    }

    if(_PickedList.size()) {
        bool changed = false;
        for(auto it=_PickedList.begin(),itNext=it;it!=_PickedList.end();it=itNext) {
            ++itNext;
            auto &sel = *it;
            if(sel.DocName == Obj.getDocument()->getName() &&
               sel.FeatName == Obj.getNameInDocument())
            {
                changed = true;
                _PickedList.erase(it);
            }
        }
        if(changed)
            notify(SelectionChanges(SelectionChanges::PickedListChanged));
    }
}


//**************************************************************************
// Construction/Destruction

/**
 * A constructor.
 * A more elaborate description of the constructor.
 */
SelectionSingleton::SelectionSingleton()
    :CurrentPreselection(SelectionChanges::ClrSelection)
    ,_needPickedList(false)
{
    hx = 0;
    hy = 0;
    hz = 0;
    ActiveGate = 0;
    gateResolve = 1;
    App::GetApplication().signalDeletedObject.connect(boost::bind(&Gui::SelectionSingleton::slotDeletedObject, this, bp::_1));
    signalSelectionChanged.connect(boost::bind(&Gui::SelectionSingleton::slotSelectionChanged, this, bp::_1));

    auto hGrp = App::GetApplication().GetParameterGroupByPath(
            "User parameter:BaseApp/Preferences/Units");
    fmtDecimal = hGrp->GetInt("DecimalsPreSel",-1);
}

/**
 * A destructor.
 * A more elaborate description of the destructor.
 */
SelectionSingleton::~SelectionSingleton()
{
}

SelectionSingleton* SelectionSingleton::_pcSingleton = NULL;

SelectionSingleton& SelectionSingleton::instance(void)
{
    if (_pcSingleton == NULL)
        _pcSingleton = new SelectionSingleton;
    return *_pcSingleton;
}

void SelectionSingleton::destruct (void)
{
    if (_pcSingleton != NULL)
        delete _pcSingleton;
    _pcSingleton = 0;
}

//**************************************************************************
// Python stuff

// SelectionSingleton Methods  // Methods structure
PyMethodDef SelectionSingleton::Methods[] = {
    {"addSelection",         (PyCFunction) SelectionSingleton::sAddSelection, METH_VARARGS,
     "addSelection(object,[string,float,float,float]) -- Add an object to the selection\n"
     "where string is the sub-element name and the three floats represent a 3d point"},
    {"updateSelection",      (PyCFunction) SelectionSingleton::sUpdateSelection, METH_VARARGS,
     "updateSelection(show,object,[string]) -- update an object in the selection\n"
     "where string is the sub-element name and the three floats represent a 3d point"},
    {"removeSelection",      (PyCFunction) SelectionSingleton::sRemoveSelection, METH_VARARGS,
     "removeSelection(object) -- Remove an object from the selection"},
    {"clearSelection"  ,     (PyCFunction) SelectionSingleton::sClearSelection, METH_VARARGS,
     "clearSelection(docName='',clearPreSelect=True) -- Clear the selection\n"
     "Clear the selection to the given document name. If no document is\n"
     "given the complete selection is cleared."},
    {"isSelected",           (PyCFunction) SelectionSingleton::sIsSelected, METH_VARARGS,
     "isSelected(object,resolve=True) -- Check if a given object is selected"},
    {"setPreselection",      reinterpret_cast<PyCFunction>(reinterpret_cast<void (*) (void)>( SelectionSingleton::sSetPreselection )), METH_VARARGS|METH_KEYWORDS,
     "setPreselection() -- Set preselected object"},
    {"getPreselection",      (PyCFunction) SelectionSingleton::sGetPreselection, METH_VARARGS,
     "getPreselection() -- Get preselected object"},
    {"clearPreselection",   (PyCFunction) SelectionSingleton::sRemPreselection, METH_VARARGS,
     "clearPreselection() -- Clear the preselection"},
    {"setPreselectionText", (PyCFunction) SelectionSingleton::sSetPreselectionText, METH_VARARGS,
     "setPreselectionText() -- Set preselection message text"},
    {"getPreselectionText", (PyCFunction) SelectionSingleton::sGetPreselectionText, METH_VARARGS,
     "getPreselectionText() -- Get preselected message text"},
    {"countObjectsOfType",   (PyCFunction) SelectionSingleton::sCountObjectsOfType, METH_VARARGS,
     "countObjectsOfType([string], [string], [resolve=1]) -- Get the number of selected objects\n"
     "The first argument defines the object type e.g. \"Part::Feature\". If not given, then\n"
     "return the total number of selections. The second argumeht defines the document name.\n"
     "If no document name is given the currently active document is used"},
    {"getSelection",         (PyCFunction) SelectionSingleton::sGetSelection, METH_VARARGS,
     "getSelection(docName='',resolve=1,single=False) -- Return a list of selected objects\n"
     "\ndocName - document name. Empty string means the active document, and '*' means all document"
     "\nresolve - whether to resolve the subname references."
     "\n          0: do not resolve, 1: resolve, 2: resolve with element map"
     "\nsingle - only return if there is only one selection"},
    {"getPickedList",         (PyCFunction) SelectionSingleton::sGetPickedList, 1,
     "getPickedList(docName='') -- Return a list of objects under the last mouse click\n"
     "\ndocName - document name. Empty string means the active document, and '*' means all document"},
    {"enablePickedList",      (PyCFunction) SelectionSingleton::sEnablePickedList, METH_VARARGS,
     "enablePickedList(boolean) -- Enable/disable pick list"},
    {"getCompleteSelection", (PyCFunction) SelectionSingleton::sGetCompleteSelection, METH_VARARGS,
     "getCompleteSelection(resolve=1) -- Return a list of selected objects of all documents."},
    {"getSelectionEx",         (PyCFunction) SelectionSingleton::sGetSelectionEx, METH_VARARGS,
     "getSelectionEx(docName='',resolve=1, single=False) -- Return a list of SelectionObjects\n"
     "\ndocName - document name. Empty string means the active document, and '*' means all document"
     "\nresolve - whether to resolve the subname references."
     "\n          0: do not resolve, 1: resolve, 2: resolve with element map"
     "\nsingle - only return if there is only one selection\n"
     "\nThe SelectionObjects contain a variety of information about the selection, e.g. sub-element names."},
    {"getSelectionObject",  (PyCFunction) SelectionSingleton::sGetSelectionObject, METH_VARARGS,
     "getSelectionObject(doc,obj,sub,(x,y,z)) -- Return a SelectionObject"},
    {"addObserver",         (PyCFunction) SelectionSingleton::sAddSelObserver, METH_VARARGS,
     "addObserver(Object, resolve=1) -- Install an observer\n"},
    {"removeObserver",      (PyCFunction) SelectionSingleton::sRemSelObserver, METH_VARARGS,
     "removeObserver(Object) -- Uninstall an observer\n"},
    {"addSelectionGate",      (PyCFunction) SelectionSingleton::sAddSelectionGate, METH_VARARGS,
     "addSelectionGate(String|Filter|Gate, resolve=1) -- activate the selection gate.\n"
     "The selection gate will prohibit all selections which do not match\n"
     "the given selection filter string.\n"
     " Examples strings are:\n"
     "'SELECT Part::Feature SUBELEMENT Edge',\n"
     "'SELECT Robot::RobotObject'\n"
     "\n"
     "You can also set an instance of SelectionFilter:\n"
     "filter = Gui.Selection.Filter('SELECT Part::Feature SUBELEMENT Edge')\n"
     "Gui.Selection.addSelectionGate(filter)\n"
     "\n"
     "And the most flexible approach is to write your own selection gate class\n"
     "that implements the method 'allow'\n"
     "class Gate:\n"
     "  def allow(self,doc,obj,sub):\n"
     "    return (sub[0:4] == 'Face')\n"
     "Gui.Selection.addSelectionGate(Gate())"},
    {"removeSelectionGate",      (PyCFunction) SelectionSingleton::sRemoveSelectionGate, METH_VARARGS,
     "removeSelectionGate() -- remove the active selection gate\n"},
    {"setVisible",            (PyCFunction) SelectionSingleton::sSetVisible, METH_VARARGS, 
     "setVisible(visible=None) -- set visibility of all selection items\n"
     "If 'visible' is None, then toggle visibility"},
    {"pushSelStack",      (PyCFunction) SelectionSingleton::sPushSelStack, METH_VARARGS,
     "pushSelStack(clearForward=True, overwrite=False) -- push current selection to stack\n\n"
     "clearForward: whether to clear the forward selection stack.\n"
     "overwrite: overwrite the top back selection stack with current selection."},
    {"hasSelection",      (PyCFunction) SelectionSingleton::sHasSelection, METH_VARARGS,
     "hasSelection(docName='', resolve=False) -- check if there is any selection\n"},
    {"hasSubSelection",   (PyCFunction) SelectionSingleton::sHasSubSelection, METH_VARARGS,
     "hasSubSelection(docName='',subElement=False) -- check if there is any selection with subname\n"},
    {"getSelectionFromStack",(PyCFunction) SelectionSingleton::sGetSelectionFromStack, METH_VARARGS,
     "getSelectionFromStack(docName='',resolve=1,index=0) -- Return a list of SelectionObjects from selection stack\n"
     "\ndocName - document name. Empty string means the active document, and '*' means all document"
     "\nresolve - whether to resolve the subname references."
     "\n          0: do not resolve, 1: resolve, 2: resolve with element map"
     "\nindex - select stack index, 0 is the last pushed selection, positive index to trace further back,\n"
     "          and negative for forward stack item"},
    {"checkTopParent",   (PyCFunction) SelectionSingleton::sCheckTopParent, METH_VARARGS,
     "checkTopParent(obj, subname='')\n\n"
     "Check object hierarchy to find the top parent of the given (sub)object.\n"
     "Returns (topParent,subname)\n"},
    {"setContext",   (PyCFunction) SelectionSingleton::sSetContext, METH_VARARGS,
     "setContext(obj, subname='')\n\n"
     "Set the context sub-object when calling ViewObject.doubleClicked/setupContextMenu()."},
    {"getContext",   (PyCFunction) SelectionSingleton::sGetContext, METH_VARARGS,
     "getContext(extended = False) -> (obj, subname)\n\n"
     "Obtain the current context sub-object.\n"
     "If extended is True, then try various options in the following order,\n"
     " * Explicitly set context by calling setContext(),\n"
     " * Current pre-selected object,\n"
     " * If there is only one selection in the active document,\n"
     " * If the active document is in editing, then return the editing object."},
    {NULL, NULL, 0, NULL}  /* Sentinel */
};

PyObject *SelectionSingleton::sAddSelection(PyObject * /*self*/, PyObject *args)
{
    SelectionLogDisabler disabler(true);
    PyObject *clearPreselect = Py_True;
    char *objname;
    char *docname;
    char* subname=0;
    float x=0,y=0,z=0;
    if (PyArg_ParseTuple(args, "ss|sfffO!", &docname, &objname ,
                &subname,&x,&y,&z,&PyBool_Type,&clearPreselect)) 
    {
        Selection().addSelection(docname,objname,subname,x,y,z,0,PyObject_IsTrue(clearPreselect));
        Py_Return;
    }
    PyErr_Clear();

    PyObject *object;
    subname = 0;
    x=0,y=0,z=0;
    if (PyArg_ParseTuple(args, "O!|sfffO!", &(App::DocumentObjectPy::Type),&object,
                &subname,&x,&y,&z,&PyBool_Type,&clearPreselect)) 
    {
        App::DocumentObjectPy* docObjPy = static_cast<App::DocumentObjectPy*>(object);
        App::DocumentObject* docObj = docObjPy->getDocumentObjectPtr();
        if (!docObj || !docObj->getNameInDocument()) {
            PyErr_SetString(Base::BaseExceptionFreeCADError, "Cannot check invalid object");
            return NULL;
        }

        Selection().addSelection(docObj->getDocument()->getName(),
                                 docObj->getNameInDocument(),
                                 subname,x,y,z,0,PyObject_IsTrue(clearPreselect));
        Py_Return;
    }

    PyErr_Clear();
    PyObject *sequence;
    if (PyArg_ParseTuple(args, "O!O|O!", &(App::DocumentObjectPy::Type),&object,
                &sequence,&PyBool_Type,&clearPreselect)) 
    {
        App::DocumentObjectPy* docObjPy = static_cast<App::DocumentObjectPy*>(object);
        App::DocumentObject* docObj = docObjPy->getDocumentObjectPtr();
        if (!docObj || !docObj->getNameInDocument()) {
            PyErr_SetString(Base::BaseExceptionFreeCADError, "Cannot check invalid object");
            return NULL;
        }

        try {
            if (PyTuple_Check(sequence) || PyList_Check(sequence)) {
                Py::Sequence list(sequence);
                for (Py::Sequence::iterator it = list.begin(); it != list.end(); ++it) {
                    std::string subname = static_cast<std::string>(Py::String(*it));
                    Selection().addSelection(docObj->getDocument()->getName(),
                                             docObj->getNameInDocument(),
                                             subname.c_str(),0,0,0,0,PyObject_IsTrue(clearPreselect));
                }

                Py_Return;
            }
        }
        catch (const Py::Exception&) {
            // do nothing here
        }
    }

    PyErr_SetString(PyExc_ValueError, "type must be 'DocumentObject[,subname[,x,y,z]]' or 'DocumentObject, list or tuple of subnames'");
    return 0;
}

PyObject *SelectionSingleton::sUpdateSelection(PyObject * /*self*/, PyObject *args)
{
    PyObject *show;
    PyObject *object;
    char* subname=0;
    if(!PyArg_ParseTuple(args, "OO!|s", &show,&(App::DocumentObjectPy::Type),&object,&subname))
        return 0;
    App::DocumentObjectPy* docObjPy = static_cast<App::DocumentObjectPy*>(object);
    App::DocumentObject* docObj = docObjPy->getDocumentObjectPtr();
    if (!docObj || !docObj->getNameInDocument()) {
        PyErr_SetString(Base::BaseExceptionFreeCADError, "Cannot check invalid object");
        return NULL;
    }

    Selection().updateSelection(PyObject_IsTrue(show),
            docObj->getDocument()->getName(), docObj->getNameInDocument(), subname);
    Py_Return;
}


PyObject *SelectionSingleton::sRemoveSelection(PyObject * /*self*/, PyObject *args)
{
    SelectionLogDisabler disabler(true);
    char *docname,*objname;
    char* subname=0;
    if(PyArg_ParseTuple(args, "ss|s", &docname,&objname,&subname)) {
        Selection().rmvSelection(docname,objname,subname);
        Py_Return;
    }
    PyErr_Clear();

    PyObject *object;
    subname = 0;
    if (!PyArg_ParseTuple(args, "O!|s", &(App::DocumentObjectPy::Type),&object,&subname))
        return NULL;

    App::DocumentObjectPy* docObjPy = static_cast<App::DocumentObjectPy*>(object);
    App::DocumentObject* docObj = docObjPy->getDocumentObjectPtr();
    if (!docObj || !docObj->getNameInDocument()) {
        PyErr_SetString(Base::BaseExceptionFreeCADError, "Cannot check invalid object");
        return NULL;
    }

    Selection().rmvSelection(docObj->getDocument()->getName(),
                             docObj->getNameInDocument(),
                             subname);

    Py_Return;
}

PyObject *SelectionSingleton::sClearSelection(PyObject * /*self*/, PyObject *args)
{
    SelectionLogDisabler disabler(true);
    PyObject *clearPreSelect = Py_True;
    char *documentName=0;
    if (!PyArg_ParseTuple(args, "|O!", &PyBool_Type, &clearPreSelect)) {
        PyErr_Clear();
        if (!PyArg_ParseTuple(args, "|sO!", &documentName, &PyBool_Type, &clearPreSelect))
            return NULL;
    }
    Selection().clearSelection(documentName,PyObject_IsTrue(clearPreSelect));
    Py_Return;
}

PyObject *SelectionSingleton::sIsSelected(PyObject * /*self*/, PyObject *args)
{
    PyObject *object;
    char* subname=0;
    PyObject *resolve = Py_True;
    if (!PyArg_ParseTuple(args, "O!|sO", &(App::DocumentObjectPy::Type), &object, &subname,&resolve))
        return NULL;

    App::DocumentObjectPy* docObj = static_cast<App::DocumentObjectPy*>(object);
    bool ok = Selection().isSelected(docObj->getDocumentObjectPtr(), subname,PyObject_IsTrue(resolve));
    return Py_BuildValue("O", (ok ? Py_True : Py_False));
}

PyObject *SelectionSingleton::sCountObjectsOfType(PyObject * /*self*/, PyObject *args)
{
    char* objecttype=0;
    char* document=0;
    int resolve = 1;
    if (!PyArg_ParseTuple(args, "|ssi", &objecttype, &document,&resolve))
        return NULL;

    unsigned int count = Selection().countObjectsOfType(objecttype, document, resolve);
#if PY_MAJOR_VERSION < 3
    return PyInt_FromLong(count);
#else
    return PyLong_FromLong(count);
#endif
}

PyObject *SelectionSingleton::sGetSelection(PyObject * /*self*/, PyObject *args)
{
    char *documentName=0;
    int resolve = 1;
    PyObject *single=Py_False;
    if (!PyArg_ParseTuple(args, "|siO", &documentName,&resolve,&single))     // convert args: Python->C 
        return NULL;                             // NULL triggers exception

    std::vector<SelectionSingleton::SelObj> sel;
    sel = Selection().getSelection(documentName,resolve,PyObject_IsTrue(single));

    try {
        std::set<App::DocumentObject*> noduplicates;
        std::vector<App::DocumentObject*> selectedObjects; // keep the order of selection
        Py::List list;
        for (std::vector<SelectionSingleton::SelObj>::iterator it = sel.begin(); it != sel.end(); ++it) {
            if (noduplicates.insert(it->pObject).second) {
                selectedObjects.push_back(it->pObject);
            }
        }
        for (std::vector<App::DocumentObject*>::iterator it = selectedObjects.begin(); it != selectedObjects.end(); ++it) {
            list.append(Py::asObject((*it)->getPyObject()));
        }
        return Py::new_reference_to(list);
    }
    catch (Py::Exception&) {
        return 0;
    }
}

PyObject *SelectionSingleton::sEnablePickedList(PyObject * /*self*/, PyObject *args)
{
    PyObject *enable = Py_True;
    if (!PyArg_ParseTuple(args, "|O", &enable))     // convert args: Python->C 
        return NULL;                             // NULL triggers exception

    Selection().enablePickedList(PyObject_IsTrue(enable));
    Py_Return;
}

PyObject *SelectionSingleton::sSetPreselection(PyObject * /*self*/, PyObject *args, PyObject *kwd)
{
    PyObject *object;
    char* subname=0;
    float x=0,y=0,z=0;
    int type=1;
    static char *kwlist[] = {"obj","subname","x","y","z","tp",0};
    if (PyArg_ParseTupleAndKeywords(args, kwd, "O!|sfffi", kwlist,
                &(App::DocumentObjectPy::Type),&object,&subname,&x,&y,&z,&type)) {
        App::DocumentObjectPy* docObjPy = static_cast<App::DocumentObjectPy*>(object);
        App::DocumentObject* docObj = docObjPy->getDocumentObjectPtr();
        if (!docObj || !docObj->getNameInDocument()) {
            PyErr_SetString(Base::BaseExceptionFreeCADError, "Cannot check invalid object");
            return NULL;
        }

        Selection().setPreselect(docObj->getDocument()->getName(),
                                 docObj->getNameInDocument(),
                                 subname,x,y,z,type);
        Py_Return;
    }

    PyErr_SetString(PyExc_ValueError, "type must be 'DocumentObject[,subname[,x,y,z]]'");
    return 0;
}

PyObject *SelectionSingleton::sGetPreselection(PyObject * /*self*/, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ""))
        return NULL;

    const SelectionChanges& sel = Selection().getPreselection();
    SelectionObject obj(sel);
    return obj.getPyObject();
}

PyObject *SelectionSingleton::sRemPreselection(PyObject * /*self*/, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ""))
        return NULL;

    Selection().rmvPreselect();
    Py_Return;
}

PyObject *SelectionSingleton::sGetCompleteSelection(PyObject * /*self*/, PyObject *args)
{
    int resolve = 1;
    if (!PyArg_ParseTuple(args, "|i",&resolve))     // convert args: Python->C 
        return NULL;                             // NULL triggers exception

    std::vector<SelectionSingleton::SelObj> sel;
    sel = Selection().getCompleteSelection(resolve);

    try {
        Py::List list;
        for (std::vector<SelectionSingleton::SelObj>::iterator it = sel.begin(); it != sel.end(); ++it) {
            list.append(Py::asObject(it->pObject->getPyObject()));
        }
        return Py::new_reference_to(list);
    }
    catch (Py::Exception&) {
        return 0;
    }
}

PyObject *SelectionSingleton::sGetSelectionEx(PyObject * /*self*/, PyObject *args)
{
    char *documentName=0;
    int resolve=1;
    PyObject *single = Py_False;
    if (!PyArg_ParseTuple(args, "|siO", &documentName,&resolve,&single))     // convert args: Python->C 
        return NULL;                             // NULL triggers exception

    std::vector<SelectionObject> sel;
    sel = Selection().getSelectionEx(documentName,
            App::DocumentObject::getClassTypeId(),resolve,PyObject_IsTrue(single));

    try {
        Py::List list;
        for (std::vector<SelectionObject>::iterator it = sel.begin(); it != sel.end(); ++it) {
            list.append(Py::asObject(it->getPyObject()));
        }
        return Py::new_reference_to(list);
    }
    catch (Py::Exception&) {
        return 0;
    }
}

PyObject *SelectionSingleton::sGetPickedList(PyObject * /*self*/, PyObject *args)
{
    char *documentName=0;
    if (!PyArg_ParseTuple(args, "|s", &documentName))     // convert args: Python->C 
        return NULL;                             // NULL triggers exception

    std::vector<SelectionObject> sel;
    sel = Selection().getPickedListEx(documentName);

    try {
        Py::List list;
        for (std::vector<SelectionObject>::iterator it = sel.begin(); it != sel.end(); ++it) {
            list.append(Py::asObject(it->getPyObject()));
        }
        return Py::new_reference_to(list);
    }
    catch (Py::Exception&) {
        return 0;
    }
}

PyObject *SelectionSingleton::sGetSelectionObject(PyObject * /*self*/, PyObject *args)
{
    char *docName, *objName, *subName;
    PyObject* tuple=0;
    if (!PyArg_ParseTuple(args, "sss|O!", &docName, &objName, &subName,
                                          &PyTuple_Type, &tuple))
        return NULL;

    try {
        SelectionObject selObj;
        selObj.DocName  = docName;
        selObj.FeatName = objName;
        std::string sub = subName;
        if (!sub.empty()) {
            selObj.SubNames.push_back(sub);
            if (tuple) {
                Py::Tuple t(tuple);
                double x = (double)Py::Float(t.getItem(0));
                double y = (double)Py::Float(t.getItem(1));
                double z = (double)Py::Float(t.getItem(2));
                selObj.SelPoses.emplace_back(x,y,z);
            }
        }

        return selObj.getPyObject();
    }
    catch (const Py::Exception&) {
        return 0;
    }
    catch (const Base::Exception& e) {
        PyErr_SetString(Base::BaseExceptionFreeCADError, e.what());
        return 0;
    }
}

PyObject *SelectionSingleton::sAddSelObserver(PyObject * /*self*/, PyObject *args)
{
    PyObject* o;
    int resolve = 1;
    if (!PyArg_ParseTuple(args, "O|i",&o,&resolve))
        return NULL;
    PY_TRY {
        SelectionObserverPython::addObserver(Py::Object(o),resolve);
        Py_Return;
    } PY_CATCH;
}

PyObject *SelectionSingleton::sRemSelObserver(PyObject * /*self*/, PyObject *args)
{
    PyObject* o;
    if (!PyArg_ParseTuple(args, "O",&o))
        return NULL;
    PY_TRY {
        SelectionObserverPython::removeObserver(Py::Object(o));
        Py_Return;
    } PY_CATCH;
}

PyObject *SelectionSingleton::sAddSelectionGate(PyObject * /*self*/, PyObject *args)
{
    char* filter;
    int resolve = 1;
    if (PyArg_ParseTuple(args, "s|i",&filter,&resolve)) {
        PY_TRY {
            Selection().addSelectionGate(new SelectionFilterGate(filter),resolve);
            Py_Return;
        } PY_CATCH;
    }

    PyErr_Clear();
    PyObject* filterPy;
    if (PyArg_ParseTuple(args, "O!|i",SelectionFilterPy::type_object(),&filterPy,resolve)) {
        PY_TRY {
            Selection().addSelectionGate(new SelectionFilterGatePython(
                        static_cast<SelectionFilterPy*>(filterPy)),resolve);
            Py_Return;
        } PY_CATCH;
    }

    PyErr_Clear();
    PyObject* gate;
    if (PyArg_ParseTuple(args, "O|i",&gate,&resolve)) {
        PY_TRY {
            Selection().addSelectionGate(new SelectionGatePython(Py::Object(gate, false)),resolve);
            Py_Return;
        } PY_CATCH;
    }

    PyErr_SetString(PyExc_ValueError, "Argument is neither string nor SelectionFiler nor SelectionGate");
    return 0;
}

PyObject *SelectionSingleton::sRemoveSelectionGate(PyObject * /*self*/, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ""))
        return NULL;

    PY_TRY {
        Selection().rmvSelectionGate();
    } PY_CATCH;

    Py_Return;
}

PyObject *SelectionSingleton::sSetVisible(PyObject * /*self*/, PyObject *args)
{
    PyObject *visible = Py_None;
    if (!PyArg_ParseTuple(args, "|O",&visible))
        return NULL;                             // NULL triggers exception 

    PY_TRY {
        int vis;
        if(visible == Py_None) {
            vis = -1;
        }
#if PY_MAJOR_VERSION < 3
        else if(PyInt_Check(visible)) {
            vis = PyInt_AsLong(visible);
        }
#else
        else if(PyLong_Check(visible)) {
            vis = PyLong_AsLong(visible);
        }
#endif
        else {
            vis = PyObject_IsTrue(visible)?1:0;
        }
        if(vis<0)
            Selection().setVisible(VisToggle);
        else
            Selection().setVisible(vis==0?VisHide:VisShow);
    } PY_CATCH;

    Py_Return;
}

PyObject *SelectionSingleton::sPushSelStack(PyObject * /*self*/, PyObject *args)
{
    PyObject *clear = Py_True;
    PyObject *overwrite = Py_False;
    if (!PyArg_ParseTuple(args, "|OO",&clear,&overwrite))
        return NULL;                             // NULL triggers exception 

    Selection().selStackPush(PyObject_IsTrue(clear),PyObject_IsTrue(overwrite));
    Py_Return;
}

PyObject *SelectionSingleton::sHasSelection(PyObject * /*self*/, PyObject *args)
{
    const char *doc = 0;
    PyObject *resolve = Py_False;
    if (!PyArg_ParseTuple(args, "|sO",&doc,&resolve))
        return NULL;                             // NULL triggers exception 

    PY_TRY {
        bool ret;
        if(doc || PyObject_IsTrue(resolve))
            ret = Selection().hasSelection(doc,PyObject_IsTrue(resolve));
        else
            ret = Selection().hasSelection();
        return Py::new_reference_to(Py::Boolean(ret));
    } PY_CATCH;
}

PyObject *SelectionSingleton::sHasSubSelection(PyObject * /*self*/, PyObject *args)
{
    const char *doc = 0;
    PyObject *subElement = Py_False;
    if (!PyArg_ParseTuple(args, "|sO!",&doc,&PyBool_Type,&subElement))
        return NULL;                             // NULL triggers exception 

    PY_TRY {
        return Py::new_reference_to(
                Py::Boolean(Selection().hasSubSelection(doc,PyObject_IsTrue(subElement))));
    } PY_CATCH;
}

PyObject *SelectionSingleton::sGetSelectionFromStack(PyObject * /*self*/, PyObject *args)
{
    char *documentName=0;
    int resolve=1;
    int index=0;
    if (!PyArg_ParseTuple(args, "|sii", &documentName,&resolve,&index))     // convert args: Python->C 
        return NULL;                             // NULL triggers exception

    PY_TRY {
        Py::List list;
        for(auto &sel : Selection().selStackGet(documentName, resolve, index))
            list.append(Py::asObject(sel.getPyObject()));
        return Py::new_reference_to(list);
    } PY_CATCH;
}

PyObject* SelectionSingleton::sCheckTopParent(PyObject *, PyObject *args) {
    PyObject *pyObj;
    const char *subname = 0;
    if (!PyArg_ParseTuple(args, "O!|s", &App::DocumentObjectPy::Type,&pyObj,&subname))
        return 0;
    PY_TRY {
        std::string sub;
        if(subname)
            sub = subname;
        App::DocumentObject *obj = static_cast<App::DocumentObjectPy*>(pyObj)->getDocumentObjectPtr();
        checkTopParent(obj,sub);
        return Py::new_reference_to(Py::TupleN(Py::asObject(obj->getPyObject()),Py::String(sub)));
    } PY_CATCH;
}

PyObject *SelectionSingleton::sGetContext(PyObject *, PyObject *args)
{
    PyObject *extended = Py_False;
    if (!PyArg_ParseTuple(args, "|O", &extended))
        return 0;
    App::SubObjectT sobjT;
    if (PyObject_IsTrue(extended))
        sobjT = Selection().getExtendedContext();
    else
        sobjT = Selection().ContextObject;
    auto obj = sobjT.getObject();
    if (!obj)
        Py_Return;
    return Py::new_reference_to(Py::TupleN(Py::asObject(obj->getPyObject()),
                                           Py::String(sobjT.getSubName().c_str())));
}

PyObject *SelectionSingleton::sSetContext(PyObject *, PyObject *args)
{
    PyObject *pyObj;
    char *subname = "";
    if (!PyArg_ParseTuple(args, "O!|s", &App::DocumentObjectPy::Type,&pyObj,&subname))
        return 0;
    Selection().ContextObject = App::SubObjectT(
            static_cast<App::DocumentObjectPy*>(pyObj)->getDocumentObjectPtr(), subname);
    Py_Return;
}

PyObject *SelectionSingleton::sGetPreselectionText(PyObject *, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ""))
        return 0;
    return Py::new_reference_to(Py::String(Selection().getPreselectionText()));
}

PyObject *SelectionSingleton::sSetPreselectionText(PyObject *, PyObject *args)
{
    const char *subname;
    if (!PyArg_ParseTuple(args, "s", &subname))
        return 0;
    Selection().setPreselectionText(subname);
    Py_Return;
}
