/***************************************************************************
 *   Copyright (c) 2009 Jürgen Riegel <juergen.riegel@web.de>              *
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
# include <boost_bind_bind.hpp>
# include <QAbstractSpinBox>
# include <QActionEvent>
# include <QApplication>
# include <QCursor>
# include <QLineEdit>
# include <QPointer>
# include <QPushButton>
# include <QTimer>
# include <QComboBox>
#endif

#include "TaskView.h"
#include "TaskDialog.h"
#include "TaskAppearance.h"
#include "TaskEditControl.h"
#include <Gui/Document.h>
#include <Gui/Application.h>
#include <Gui/ViewProvider.h>
#include <Gui/Control.h>
#include <Gui/ActionFunction.h>
#include <Gui/MainWindow.h>
#include <Gui/ViewParams.h>
#include <Gui/Widgets.h>

#if defined (QSINT_ACTIONPANEL)
#include <Gui/QSint/actionpanel/taskgroup_p.h>
#include <Gui/QSint/actionpanel/taskheader_p.h>
#include <Gui/QSint/actionpanel/freecadscheme.h>
#endif

using namespace Gui::TaskView;
namespace bp = boost::placeholders;

//**************************************************************************
//**************************************************************************
// TaskContent
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++




//**************************************************************************
//**************************************************************************
// TaskWidget
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TaskWidget::TaskWidget( QWidget *parent)
    : QWidget(parent)
{

}

TaskWidget::~TaskWidget()
{
}

//**************************************************************************
//**************************************************************************
// TaskGroup
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#if !defined (QSINT_ACTIONPANEL)
TaskGroup::TaskGroup(QWidget *parent)
    : iisTaskGroup(parent, false)
{
    setScheme(iisFreeCADTaskPanelScheme::defaultScheme());
}

TaskGroup::~TaskGroup()
{
}

namespace Gui { namespace TaskView {
class TaskIconLabel : public iisIconLabel {
public:
    TaskIconLabel(const QIcon &icon, 
                  const QString &title,
                  QWidget *parent = 0)
      : iisIconLabel(icon, title, parent) {
          // do not allow to get the focus because when hiding the task box
          // it could cause to activate another MDI view.
          setFocusPolicy(Qt::NoFocus);
    }
    void setTitle(const QString &text) {
        myText = text;
        update();
    }
};
}
}

void TaskGroup::actionEvent (QActionEvent* e)
{
    QAction *action = e->action();
    switch (e->type()) {
    case QEvent::ActionAdded:
        {
            TaskIconLabel *label = new TaskIconLabel(
                action->icon(), action->text(), this);
            this->addIconLabel(label);
            connect(label,SIGNAL(clicked()),action,SIGNAL(triggered()),Qt::QueuedConnection);
            break;
        }
    case QEvent::ActionChanged:
        {
            // update label when action changes
            QBoxLayout* bl = this->groupLayout();
            int index = this->actions().indexOf(action);
            if (index < 0) break;
            QWidgetItem* item = static_cast<QWidgetItem*>(bl->itemAt(index));
            TaskIconLabel* label = static_cast<TaskIconLabel*>(item->widget());
            label->setTitle(action->text());
            break;
        }
    case QEvent::ActionRemoved:
        {
            // cannot change anything
            break;
        }
    default:
        break;
    }
}
#else
TaskGroup::TaskGroup(QWidget *parent)
    : QSint::ActionBox(parent)
{
}

TaskGroup::TaskGroup(const QString & headerText, QWidget *parent)
    : QSint::ActionBox(headerText, parent)
{
}

TaskGroup::TaskGroup(const QPixmap & icon, const QString & headerText, QWidget *parent)
    : QSint::ActionBox(icon, headerText, parent)
{
}

TaskGroup::~TaskGroup()
{
}

void TaskGroup::actionEvent (QActionEvent* e)
{
    QAction *action = e->action();
    switch (e->type()) {
    case QEvent::ActionAdded:
        {
            this->createItem(action);
            break;
        }
    case QEvent::ActionChanged:
        {
            break;
        }
    case QEvent::ActionRemoved:
        {
            // cannot change anything
            break;
        }
    default:
        break;
    }
}
#endif
//**************************************************************************
//**************************************************************************
// TaskBox
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#if !defined (QSINT_ACTIONPANEL)
TaskBox::TaskBox(const QPixmap &icon, const QString &title, bool expandable, QWidget *parent)
    : iisTaskBox(icon, title, expandable, parent), wasShown(false)
{
    setScheme(iisFreeCADTaskPanelScheme::defaultScheme());
	connect(myHeader, SIGNAL(activated()), this, SIGNAL(toggledExpansion()));
    m_foldDirection = 1;
}
#else
TaskBox::TaskBox(QWidget *parent)
  : QSint::ActionGroup(parent), wasShown(false)
{
    // override vertical size policy because otherwise task dialogs
    // whose needsFullSpace() returns true won't take full space.
    myGroup->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	connect(myHeader, SIGNAL(activated()), this, SIGNAL(toggledExpansion()));
    m_foldDirection = 1;
}

TaskBox::TaskBox(const QString &title, bool expandable, QWidget *parent)
  : QSint::ActionGroup(title, expandable, parent), wasShown(false)
{
    // override vertical size policy because otherwise task dialogs
    // whose needsFullSpace() returns true won't take full space.
    myGroup->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	connect(myHeader, SIGNAL(activated()), this, SIGNAL(toggledExpansion()));
    m_foldDirection = 1;
}

TaskBox::TaskBox(const QPixmap &icon, const QString &title, bool expandable, QWidget *parent)
    : QSint::ActionGroup(icon, title, expandable, parent), wasShown(false)
{
    // override vertical size policy because otherwise task dialogs
    // whose needsFullSpace() returns true won't take full space.
    myGroup->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	connect(myHeader, SIGNAL(activated()), this, SIGNAL(toggledExpansion()));
    m_foldDirection = 1;
}

QSize TaskBox::minimumSizeHint() const
{
    // ActionGroup returns a size of 200x100 which leads to problems
    // when there are several task groups in a panel and the first
    // one is collapsed. In this case the task panel doesn't expand to
    // the actually required size and all the remaining groups are
    // squeezed into the available space and thus the widgets in there
    // often can't be used any more.
    // To fix this problem minimumSizeHint() is implemented to again
    // respect the layout's minimum size.
    QSize s1 = QSint::ActionGroup::minimumSizeHint();
    QSize s2 = QWidget::minimumSizeHint();
    return QSize(qMax(s1.width(), s2.width()), qMax(s1.height(), s2.height()));
}
#endif

TaskBox::~TaskBox()
{
}

void TaskBox::showEvent(QShowEvent*)
{
    wasShown = true;
}

void TaskBox::hideGroupBox()
{
    if (!wasShown) {
        // get approximate height
        int h=0;
        int ct = groupLayout()->count();
        for (int i=0; i<ct; i++) {
            QLayoutItem* item = groupLayout()->itemAt(i);
            if (item && item->widget()) {
                QWidget* w = item->widget();
                h += w->height();
            }
        }

        m_tempHeight = m_fullHeight = h;
        // For the very first time the group gets shown
        // we cannot do the animation because the layouting
        // is not yet fully done
        m_foldDelta = 0;
    }
    else {
        m_tempHeight = m_fullHeight = myGroup->height();
        m_foldDelta = m_fullHeight / myScheme->groupFoldSteps;
    }

    m_foldStep = 0.0;
    m_foldDirection = -1;

    // make sure to have the correct icon
    bool block = myHeader->blockSignals(true);
    myHeader->fold();
    myHeader->blockSignals(block);

    myDummy->setFixedHeight(0);
    myDummy->hide();
    myGroup->hide();

    m_foldPixmap = QPixmap();
    setFixedHeight(myHeader->height());
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    Q_EMIT toggledExpansion();
}

int TaskBox::foldDirection() const
{
    return m_foldDirection;
}

bool TaskBox::isGroupVisible() const
{
    return myGroup->isVisible();
}

void TaskBox::actionEvent (QActionEvent* e)
{
    QAction *action = e->action();
    switch (e->type()) {
    case QEvent::ActionAdded:
        {
#if !defined (QSINT_ACTIONPANEL)
            TaskIconLabel *label = new TaskIconLabel(
                action->icon(), action->text(), this);
            this->addIconLabel(label);
            connect(label,SIGNAL(clicked()),action,SIGNAL(triggered()));
#else
            QSint::ActionLabel *label = new QSint::ActionLabel(action, this);
            this->addActionLabel(label, true, false);
#endif
            break;
        }
    case QEvent::ActionChanged:
        {
#if !defined (QSINT_ACTIONPANEL)
            // update label when action changes
            QBoxLayout* bl = myGroup->groupLayout();
            int index = this->actions().indexOf(action);
            if (index < 0) break;
            QWidgetItem* item = static_cast<QWidgetItem*>(bl->itemAt(index));
            TaskIconLabel* label = static_cast<TaskIconLabel*>(item->widget());
            label->setTitle(action->text());
#endif
            break;
        }
    case QEvent::ActionRemoved:
        {
            // cannot change anything
            break;
        }
    default:
        break;
    }
}

//**************************************************************************
//**************************************************************************
// TaskPanel
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#if defined (QSINT_ACTIONPANEL)
TaskPanel::TaskPanel(QWidget *parent)
  : QSint::ActionPanel(parent)
{
}

TaskPanel::~TaskPanel()
{
}

QSize TaskPanel::minimumSizeHint() const
{
    // ActionPanel returns a size of 200x150 which leads to problems
    // when there are several task groups in the panel and the first
    // one is collapsed. In this case the task panel doesn't expand to
    // the actually required size and all the remaining groups are
    // squeezed into the available space and thus the widgets in there
    // often can't be used any more.
    // To fix this problem minimumSizeHint() is implemented to again
    // respect the layout's minimum size.
    QSize s1 = QSint::ActionPanel::minimumSizeHint();
    QSize s2 = QWidget::minimumSizeHint();
    return QSize(qMax(s1.width(), s2.width()), qMax(s1.height(), s2.height()));
}
#endif

//**************************************************************************
//**************************************************************************
// TaskView
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TaskView::TaskView(QWidget *parent)
    : QWidget(parent),ActiveDialog(0),ActiveCtrl(0)
{
    //addWidget(new TaskEditControl(this));
    //addWidget(new TaskAppearance(this));
    //addStretch();
    
    this->setAutoFillBackground(true);
    this->layout = new QVBoxLayout(this);
    this->layout->setSpacing(0);
    this->scrollarea = new QScrollArea(this);
    this->layout->addWidget(scrollarea, 1);

#if !defined (QSINT_ACTIONPANEL)
    taskPanel = new iisTaskPanel(this);
    taskPanel->setScheme(iisFreeCADTaskPanelScheme::defaultScheme());
#else
    taskPanel = new TaskPanel(this);
    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(taskPanel->sizePolicy().hasHeightForWidth());
    taskPanel->setSizePolicy(sizePolicy);
    taskPanel->setScheme(QSint::FreeCADPanelScheme::defaultScheme());
#endif
    this->scrollarea->setWidget(taskPanel);
    this->scrollarea->setWidgetResizable(true);
    // this->scrollarea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->scrollarea->setMinimumWidth(200);

    Gui::Selection().Attach(this);

    connectApplicationActiveDocument = 
    App::GetApplication().signalActiveDocument.connect
        (boost::bind(&Gui::TaskView::TaskView::slotActiveDocument, this, bp::_1));
    connectApplicationDeleteDocument = 
    App::GetApplication().signalDeletedDocument.connect
        (boost::bind(&Gui::TaskView::TaskView::slotDeletedDocument, this));
    connectApplicationUndoDocument = 
    App::GetApplication().signalUndoDocument.connect
        (boost::bind(&Gui::TaskView::TaskView::slotUndoDocument, this, bp::_1));
    connectApplicationRedoDocument = 
    App::GetApplication().signalRedoDocument.connect
        (boost::bind(&Gui::TaskView::TaskView::slotRedoDocument, this, bp::_1));

    this->timer = new QTimer(this);
    this->timer->setSingleShot(true);
    connect(this->timer, SIGNAL(timeout()), this, SLOT(onUpdateWatcher()));
}

TaskView::~TaskView()
{
    connectApplicationActiveDocument.disconnect();
    connectApplicationDeleteDocument.disconnect();
    connectApplicationUndoDocument.disconnect();
    connectApplicationRedoDocument.disconnect();
    Gui::Selection().Detach(this);
}

bool TaskView::isEmpty(bool includeWatcher) const
{
    if (ActiveCtrl || ActiveDialog)
        return false;

    if (includeWatcher) {
        for (auto * watcher : ActiveWatcher) {
            if (watcher->shouldShow())
                return false;
        }
    }
    return true;
}

bool TaskView::event(QEvent* event)
{
    // Workaround for a limitation in Qt (#0003794)
    // Line edits and spin boxes don't handle the key combination
    // Shift+Keypad button (if NumLock is activated)
    if (event->type() == QEvent::ShortcutOverride) {
        QWidget* focusWidget = qApp->focusWidget();
        bool isLineEdit = qobject_cast<QLineEdit*>(focusWidget);
        bool isSpinBox = qobject_cast<QAbstractSpinBox*>(focusWidget);

        if (isLineEdit || isSpinBox) {
            QKeyEvent * kevent = static_cast<QKeyEvent*>(event);
            Qt::KeyboardModifiers ShiftKeypadModifier = Qt::ShiftModifier | Qt::KeypadModifier;
            if (kevent->modifiers() == Qt::NoModifier ||
                kevent->modifiers() == Qt::ShiftModifier ||
                kevent->modifiers() == Qt::KeypadModifier ||
                kevent->modifiers() == ShiftKeypadModifier) {
                switch (kevent->key()) {
                case Qt::Key_Delete:
                case Qt::Key_Home:
                case Qt::Key_End:
                case Qt::Key_Backspace:
                case Qt::Key_Left:
                case Qt::Key_Right:
                    kevent->accept();
                default:
                    break;
                }
            }
        }
    }
    return QWidget::event(event);
}

void TaskView::keyPressEvent(QKeyEvent* ke)
{
    if (ActiveCtrl && ActiveDialog) {
        if (ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Enter) {
            // spin box uses Key_Return to signal finish editing. At least for
            // PartDesign, most task panel disable spin box keyboard tracking,
            // therefore it expects the user press enter key to confirm.
            QWidget* focusWidget = qApp->focusWidget();
            if(qobject_cast<QAbstractSpinBox*>(focusWidget))
                return;

            // get all buttons of the complete task dialog
            QList<QPushButton*> list = this->findChildren<QPushButton*>();
            for (int i=0; i<list.size(); ++i) {
                QPushButton *pb = list.at(i);
                if (pb->isDefault() && pb->isVisible()) {
                    if (pb->isEnabled()) {
#if defined(FC_OS_MACOSX)
                        // #0001354: Crash on using Enter-Key for confirmation of chamfer or fillet entries
                        QPoint pos = QCursor::pos();
                        QCursor::setPos(pb->parentWidget()->mapToGlobal(pb->pos()));
#endif
                        pb->click();
#if defined(FC_OS_MACOSX)
                        QCursor::setPos(pos);
#endif
                    }
                    return;
                }
            }
        }
        else if (ke->key() == Qt::Key_Escape && ActiveDialog->isEscapeButtonEnabled()) {
            // get only the buttons of the button box
            QDialogButtonBox* box = ActiveCtrl->standardButtons();
            QList<QAbstractButton*> list = box->buttons();
            for (int i=0; i<list.size(); ++i) {
                QAbstractButton *pb = list.at(i);
                if (box->buttonRole(pb) == QDialogButtonBox::RejectRole) {
                    if (pb->isEnabled()) {
#if defined(FC_OS_MACOSX)
                        // #0001354: Crash on using Enter-Key for confirmation of chamfer or fillet entries
                        QPoint pos = QCursor::pos();
                        QCursor::setPos(pb->parentWidget()->mapToGlobal(pb->pos()));
#endif
                        pb->click();
#if defined(FC_OS_MACOSX)
                        QCursor::setPos(pos);
#endif
                    }
                    return;
                }
            }

            // In case a task panel has no Close or Cancel button
            // then invoke resetEdit() directly
            // See also ViewProvider::eventCallback
            Gui::TimerFunction* func = new Gui::TimerFunction();
            func->setAutoDelete(true);
            Gui::Document* doc = Gui::Application::Instance->getDocument(ActiveDialog->getDocumentName().c_str());
            if (doc) {
                func->setFunction(boost::bind(&Document::resetEdit, doc));
                QTimer::singleShot(0, func, SLOT(timeout()));
            }
        }
    }
    else {
        QWidget::keyPressEvent(ke);
    }
}

void TaskView::slotActiveDocument(const App::Document& doc)
{
    Q_UNUSED(doc); 
    if (!ActiveDialog)
        updateWatcher();
}

void TaskView::slotDeletedDocument()
{
    if (!ActiveDialog)
        updateWatcher();
}

void TaskView::slotUndoDocument(const App::Document&)
{
    if (ActiveDialog && ActiveDialog->isAutoCloseOnTransactionChange()) {
        ActiveDialog->autoClosedOnTransactionChange();
        removeDialog();
    }

    if (!ActiveDialog)
        updateWatcher();
}

void TaskView::slotRedoDocument(const App::Document&)
{
    if (ActiveDialog && ActiveDialog->isAutoCloseOnTransactionChange()) {
        ActiveDialog->autoClosedOnTransactionChange();
        removeDialog();
    }

    if (!ActiveDialog)
        updateWatcher();
}

/// @cond DOXERR
void TaskView::OnChange(Gui::SelectionSingleton::SubjectType &rCaller,
                        Gui::SelectionSingleton::MessageType Reason)
{
    Q_UNUSED(rCaller); 
    std::string temp;

    if (Reason.Type == SelectionChanges::AddSelection ||
        Reason.Type == SelectionChanges::ClrSelection || 
        Reason.Type == SelectionChanges::SetSelection ||
        Reason.Type == SelectionChanges::RmvSelection) {

        if (!ActiveDialog)
            updateWatcher();
    }

}
/// @endcond

void TaskView::showDialog(TaskDialog *dlg)
{
    // if trying to open the same dialog twice nothing needs to be done
    if (ActiveDialog == dlg)
        return;
    assert(!ActiveDialog);
    assert(!ActiveCtrl);

    // remove the TaskWatcher as long as the Dialog is up
    removeTaskWatcher();

    // first create the control element, set it up and wire it:
    ActiveCtrl = new TaskEditControl(this);
    ActiveCtrl->buttonBox->setStandardButtons(dlg->getStandardButtons());

    // make connection to the needed signals
    connect(ActiveCtrl->buttonBox,SIGNAL(accepted()),
            this,SLOT(accept()));
    connect(ActiveCtrl->buttonBox,SIGNAL(rejected()),
            this,SLOT(reject()));
    connect(ActiveCtrl->buttonBox,SIGNAL(helpRequested()),
            this,SLOT(helpRequested()));
    connect(ActiveCtrl->buttonBox,SIGNAL(clicked(QAbstractButton *)),
            this,SLOT(clicked(QAbstractButton *)));

    this->contents = dlg->getDialogContent();

    if (ViewParams::getTaskNoWheelFocus()) {
        // Since task dialog contains mlutiple panels which often require
        // scrolling up and down to access. Using wheel focus in any input
        // field may cause accidental change of value while scrolling.
        for (auto widget : this->contents) {
            for(auto child : widget->findChildren<QWidget*>()) {
                if (child->focusPolicy() == Qt::WheelFocus
                        && !qobject_cast<QAbstractItemView*>(child))
                    child->setFocusPolicy(Qt::StrongFocus);
                // It's not enough for some widget. We must use a eventFilter
                // to actively filter out wheel event if not in focus.
                if (qobject_cast<QComboBox*>(child)
                        || qobject_cast<QAbstractSpinBox*>(child))
                    child->installEventFilter(this);
            }
        }
    }

    Control().signalShowDialog(this, this->contents);

    // give to task dialog to customize the button box
    dlg->modifyStandardButtons(ActiveCtrl->buttonBox);

    if (ViewParams::getStickyTaskControl() && parentWidget()) {
        if (dlg->buttonPosition() == TaskDialog::North)
            this->layout->insertWidget(0, ActiveCtrl);
        else
            this->layout->addWidget(ActiveCtrl);
        for (auto widget : this->contents)
            taskPanel->addWidget(widget);
    }
    else if (dlg->buttonPosition() == TaskDialog::North) {
        taskPanel->addWidget(ActiveCtrl);
        for (auto widget : this->contents)
            taskPanel->addWidget(widget);
    }
    else {
        for (auto widget : this->contents)
            taskPanel->addWidget(widget);
        taskPanel->addWidget(ActiveCtrl);
    }

#if defined (QSINT_ACTIONPANEL)
    taskPanel->setScheme(QSint::FreeCADPanelScheme::defaultScheme());
#endif

    if (!dlg->needsFullSpace())
        taskPanel->addStretch();

    // set as active Dialog
    ActiveDialog = dlg;

    ActiveDialog->open();

    getMainWindow()->updateActions();

    Gui::LineEditStyle::setupChildren(this);

    Q_EMIT taskUpdate();
}

bool TaskView::eventFilter(QObject *o, QEvent *ev)
{
    if (o->isWidgetType()) {
        auto widget = static_cast<QWidget*>(o);
        switch(ev->type()) {
        case QEvent::Wheel:
            if (!widget->hasFocus()) {
                ev->setAccepted(false);
                return true;
            }
            break;
        case QEvent::FocusIn:
            widget->setFocusPolicy(Qt::WheelFocus);
            break;
        case QEvent::FocusOut:
            widget->setFocusPolicy(Qt::StrongFocus);
            break;
        default:
            break;
        }
    }
    return QWidget::eventFilter(o, ev);
}

void TaskView::removeDialog(void)
{
    getMainWindow()->updateActions();

    if (ActiveCtrl) {
        taskPanel->removeWidget(ActiveCtrl);
        delete ActiveCtrl;
        ActiveCtrl = 0;
    }

    TaskDialog* remove = NULL;
    if (ActiveDialog) {
        // See 'accept' and 'reject'
        if (ActiveDialog->property("taskview_accept_or_reject").isNull()) {
            Control().signalRemoveDialog(this, this->contents);
            for (auto widget : contents) 
                taskPanel->removeWidget(widget);
            contents.clear();
            remove = ActiveDialog;
            ActiveDialog = 0;
        }
        else {
            ActiveDialog->setProperty("taskview_remove_dialog", true);
        }
    }

    taskPanel->removeStretch();

    // put the watcher back in control
    addTaskWatcher();
    
    if (remove) {
        remove->closed();
        remove->emitDestructionSignal();
        delete remove;
    }
}

void TaskView::updateWatcher(void)
{
    this->timer->start(200);
}

void TaskView::onUpdateWatcher(void)
{
    if (ActiveCtrl || ActiveDialog)
        return;

    // In case a child of the TaskView has the focus and get hidden we have
    // to make sure to set the focus on a widget that won't be hidden or
    // deleted because otherwise Qt may forward the focus via focusNextPrevChild()
    // to the mdi area which may switch to another mdi view which is not an
    // acceptable behaviour.
    QWidget *fw = QApplication::focusWidget();
    if (!fw)
        this->setFocus();
    QPointer<QWidget> fwp = fw;
    while (fw &&  !fw->isWindow()) {
        if (fw == this) {
            this->setFocus();
            break;
        }
        fw = fw->parentWidget();
    }

    // add all widgets for all watcher to the task view
    for (std::vector<TaskWatcher*>::iterator it=ActiveWatcher.begin();it!=ActiveWatcher.end();++it) {
        bool match = (*it)->shouldShow();
        std::vector<QWidget*> &cont = (*it)->getWatcherContent();
        for (std::vector<QWidget*>::iterator it2=cont.begin();it2!=cont.end();++it2) {
            if (match)
                (*it2)->show();
            else
                (*it2)->hide();
        }
    }

    // In case the previous widget that had the focus is still visible
    // give it the focus back.
    if (fwp && fwp->isVisible())
        fwp->setFocus();

    Q_EMIT taskUpdate();
}

void TaskView::addTaskWatcher(const std::vector<TaskWatcher*> &Watcher)
{
    // remove and delete the old set of TaskWatcher
    for (std::vector<TaskWatcher*>::iterator it=ActiveWatcher.begin();it!=ActiveWatcher.end();++it)
        delete *it;

    ActiveWatcher = Watcher;
    if (!ActiveCtrl && !ActiveDialog)
        addTaskWatcher();
}

void TaskView::clearTaskWatcher(void)
{
    std::vector<TaskWatcher*> watcher;
    removeTaskWatcher();
    // make sure to delete the old watchers
    addTaskWatcher(watcher);
}

void TaskView::addTaskWatcher(void)
{
    // add all widgets for all watcher to the task view
    for (std::vector<TaskWatcher*>::iterator it=ActiveWatcher.begin();it!=ActiveWatcher.end();++it){
        std::vector<QWidget*> &cont = (*it)->getWatcherContent();
        for (std::vector<QWidget*>::iterator it2=cont.begin();it2!=cont.end();++it2){
           taskPanel->addWidget(*it2);
        }
    }

    if (!ActiveWatcher.empty())
        taskPanel->addStretch();
    updateWatcher();

#if defined (QSINT_ACTIONPANEL)
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
    // Workaround to avoid a crash in Qt. See also
    // https://forum.freecadweb.org/viewtopic.php?f=8&t=39187
    //
    // Notify the button box about a style change so that it can
    // safely delete the style animation of its push buttons.
    QDialogButtonBox* box = taskPanel->findChild<QDialogButtonBox*>();
    if (box) {
        QEvent event(QEvent::StyleChange);
        QApplication::sendEvent(box, &event);
    }
#endif

    taskPanel->setScheme(QSint::FreeCADPanelScheme::defaultScheme());
#endif
}

void TaskView::removeTaskWatcher(void)
{
    // In case a child of the TaskView has the focus and get hidden we have
    // to make sure that set the focus on a widget that won't be hidden or
    // deleted because otherwise Qt may forward the focus via focusNextPrevChild()
    // to the mdi area which may switch to another mdi view which is not an
    // acceptable behaviour.
    QWidget *fw = QApplication::focusWidget();
    if (!fw)
        this->setFocus();
    while (fw &&  !fw->isWindow()) {
        if (fw == this) {
            this->setFocus();
            break;
        }
        fw = fw->parentWidget();
    }

    // remove all widgets
    for (std::vector<TaskWatcher*>::iterator it=ActiveWatcher.begin();it!=ActiveWatcher.end();++it) {
        std::vector<QWidget*> &cont = (*it)->getWatcherContent();
        for (std::vector<QWidget*>::iterator it2=cont.begin();it2!=cont.end();++it2) {
            (*it2)->hide();
            taskPanel->removeWidget(*it2);
        }
    }

    taskPanel->removeStretch();
}

void TaskView::accept()
{
    if (!ActiveDialog) { // Protect against segfaults due to out-of-order deletions
        Base::Console().Warning("ActiveDialog was null in call to TaskView::accept()\n");
        return;
    }

    // Make sure that if 'accept' calls 'closeDialog' the deletion is postponed until
    // the dialog leaves the 'accept' method
    ActiveDialog->setProperty("taskview_accept_or_reject", true);
    bool success = ActiveDialog->accept();
    ActiveDialog->setProperty("taskview_accept_or_reject", QVariant());
    if (success || ActiveDialog->property("taskview_remove_dialog").isValid())
        removeDialog();
}

void TaskView::reject()
{
    if (!ActiveDialog) { // Protect against segfaults due to out-of-order deletions
        Base::Console().Warning("ActiveDialog was null in call to TaskView::reject()\n");
        return;
    }

    // Make sure that if 'reject' calls 'closeDialog' the deletion is postponed until
    // the dialog leaves the 'reject' method
    ActiveDialog->setProperty("taskview_accept_or_reject", true);
    bool success = ActiveDialog->reject();
    ActiveDialog->setProperty("taskview_accept_or_reject", QVariant());
    if (success || ActiveDialog->property("taskview_remove_dialog").isValid())
        removeDialog();
}

void TaskView::helpRequested()
{
    ActiveDialog->helpRequested();
}

void TaskView::clicked (QAbstractButton * button)
{
    int id = ActiveCtrl->buttonBox->standardButton(button);
    ActiveDialog->clicked(id);
}

void TaskView::clearActionStyle()
{
#if defined (QSINT_ACTIONPANEL)
    static_cast<QSint::FreeCADPanelScheme*>(QSint::FreeCADPanelScheme::defaultScheme())->clearActionStyle();
    taskPanel->setScheme(QSint::FreeCADPanelScheme::defaultScheme());
#endif
}

void TaskView::restoreActionStyle()
{
#if defined (QSINT_ACTIONPANEL)
    static_cast<QSint::FreeCADPanelScheme*>(QSint::FreeCADPanelScheme::defaultScheme())->restoreActionStyle();
    taskPanel->setScheme(QSint::FreeCADPanelScheme::defaultScheme());
#endif
}


#include "moc_TaskView.cpp"
