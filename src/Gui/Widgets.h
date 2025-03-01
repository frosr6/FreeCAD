/***************************************************************************
 *   Copyright (c) 2004 Werner Mayer <wmayer[at]users.sourceforge.net>     *
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


#ifndef GUI_WIDGETS_H
#define GUI_WIDGETS_H

#include <Gui/ui_DlgTreeWidget.h>
#include <QListWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPointer>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QBasicTimer>
#include <QElapsedTimer>
#include <QToolButton>
#include <QModelIndex>
#include <QProxyStyle>
#include "ExpressionBinding.h"

class QTextEdit;

namespace Gui {
class PrefCheckBox;
class CommandViewItemPrivate;

/**
 * This class allows to drag one or more items which correspond to a Command object.
 * The dragged items can be dropped onto the @ref Gui::PythonConsole.
 * @see CommandViewItem, Command
 * @author Werner Mayer
 */
class CommandIconView : public QListWidget
{
  Q_OBJECT

public:
  CommandIconView (QWidget * parent = 0);
  virtual ~CommandIconView ();

protected:
  void startDrag ( Qt::DropActions supportedActions );

protected Q_SLOTS:
  void onSelectionChanged( QListWidgetItem * item, QListWidgetItem * );

Q_SIGNALS:
  /** Emits this signal if selection has changed. */
  void emitSelectionChanged( const QString& );
};

// ------------------------------------------------------------------------------

class GuiExport ActionSelector : public QWidget
{
    Q_OBJECT

public:
    ActionSelector(QWidget* parent=0);
    ~ActionSelector();

    QTreeWidget* availableTreeWidget() const
    { return availableWidget; }
    QTreeWidget* selectedTreeWidget() const
    { return selectedWidget; }
    void setSelectedLabel(const QString&);
    QString selectedLabel() const;
    void setAvailableLabel(const QString&);
    QString availableLabel() const;

private:
    void keyPressEvent(QKeyEvent *);
    void changeEvent(QEvent*);
    void retranslateUi();
    void setButtonsEnabled();

private Q_SLOTS:
    void on_addButton_clicked();
    void on_removeButton_clicked();
    void on_upButton_clicked();
    void on_downButton_clicked();
    void onCurrentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*);
    void onItemDoubleClicked(QTreeWidgetItem * item, int column);

private:
    QGridLayout *gridLayout;
    QVBoxLayout *vboxLayout;
    QVBoxLayout *vboxLayout1;
    QPushButton *addButton;
    QPushButton *removeButton;
    QPushButton *upButton;
    QPushButton *downButton;
    QLabel      *labelAvailable;
    QLabel      *labelSelected;
    QTreeWidget *availableWidget;
    QTreeWidget *selectedWidget;
    QSpacerItem *spacerItem;
    QSpacerItem *spacerItem1;
};

// ------------------------------------------------------------------------------

/**
 * The AccelLineEdit class provides a lineedit to specfify shortcuts.
 * \author Werner Mayer
 */
class GuiExport AccelLineEdit : public QLineEdit
{
  Q_OBJECT

public:
    AccelLineEdit(QWidget * parent=0);
    bool isNone() const;

    std::string latinText() const {
        return std::string(text().toLatin1().constData());
    }

    void setLatinText(const std::string &txt) {
        setText(QLatin1String(txt.c_str()));
    }

protected:
    void keyPressEvent(QKeyEvent * e);

private:
    QString noneStr;
    int keyPressedCount;
};

// ------------------------------------------------------------------------------
// Style for controlling text cursor width of a QLineEdit
//
// Modified from https://stackoverflow.com/a/54933105

class LineEditStyle: public QProxyStyle
{
    Q_OBJECT
    Q_PROPERTY(int cursorWidth READ cursorWidth WRITE setCursorWidth)

public:
    LineEditStyle(QStyle *style);

    int cursorWidth() const{
        if(cursor_width < 0)
            return baseStyle()->pixelMetric(PM_TextCursorWidth);
        return pixelMetric(PM_TextCursorWidth);
    }

    void setCursorWidth(int cursorWidth){
        cursor_width = cursorWidth;
    }

    int pixelMetric(QStyle::PixelMetric metric,
                    const QStyleOption *option = nullptr,
                    const QWidget *widget = nullptr) const override
    {
        if(metric == PM_TextCursorWidth)
            if(cursor_width > 0)
                return  cursor_width;
        return  QProxyStyle::pixelMetric(metric, option, widget);
    }

    static void setup(QLineEdit *);
    static void setup(QTextEdit *);
    static void setup(QPlainTextEdit *);
    static void setupWidget(QWidget *w);
    static void setupObject(QObject *o);
    static void setupChildren(QObject *o);

private:
    int cursor_width;
};

// ------------------------------------------------------------------------------

/**
 * The ClearLineEdit class adds a clear button at the right side.
 * http://stackoverflow.com/questions/21232224/qlineedit-with-custom-button
 */
class GuiExport ClearLineEdit : public QLineEdit
{
  Q_OBJECT

public:
    ClearLineEdit (QWidget * parent=0);

protected:
    void resizeEvent(QResizeEvent *);

private Q_SLOTS:
    void updateClearButton(const QString &text);

private:
#if QT_VERSION >= 0x050200
    QAction *clearAction;
#else
    QToolButton *clearButton;
#endif
};

// ------------------------------------------------------------------------------

typedef QPair<QString, bool> CheckListItem;

/**
 * The CheckListDialog class provides a dialog with a QListView with
 * checkable items inside.
 * \author Werner Mayer
 */
class GuiExport CheckListDialog : public QDialog
{

  Q_OBJECT

public:
  CheckListDialog( QWidget* parent = 0, Qt::WindowFlags fl = Qt::WindowFlags() );
  ~CheckListDialog();

  void setCheckableItems( const QStringList& items );
  void setCheckableItems( const QList<CheckListItem>& items );
  QStringList getCheckedItems() const;

  void accept ();

private:
  QStringList checked;
  Ui_DlgTreeWidget ui;
};

// ------------------------------------------------------------------------------

/**
 *  Implementation of a color button.
 * \author Werner Mayer
 */
class GuiExport ColorButton : public QPushButton
{
    Q_OBJECT

    Q_PROPERTY( QColor color READ color WRITE setColor )
    Q_PROPERTY( bool allowChangeColor READ allowChangeColor WRITE setAllowChangeColor )
    Q_PROPERTY( bool drawFrame READ drawFrame WRITE setDrawFrame )
    Q_PROPERTY( bool allowChangeAlpha READ allowChangeAlpha WRITE setAllowChangeAlpha )

public:
    ColorButton(QWidget* parent = 0);
    ~ColorButton();

    void setColor(const QColor&);
    QColor color() const;

    void setPackedColor(uint32_t);
    uint32_t packedColor() const;

    void setAllowChangeColor(bool);
    bool allowChangeColor() const;

    void setDrawFrame(bool);
    bool drawFrame() const;

    void setModal(bool);
    bool isModal() const;

    void setAutoChangeColor(bool);
    bool autoChangeColor() const;

    void setAllowChangeAlpha(bool);
    bool allowChangeAlpha() const;

public Q_SLOTS:
    void onChooseColor();

private Q_SLOTS:
    void onColorChosen(const QColor&);
    void onRejected();

Q_SIGNALS:
    /** Emits this signal when color has changed */
    void changed();

protected:
    void paintEvent (QPaintEvent*);

private:
    struct ColorButtonP *d;
};

class GuiExport TransparentColorButton : public ColorButton
{
public:
    TransparentColorButton(QWidget* parent = 0);
};

// ------------------------------------------------------------------------------

/**
 * A text label where a url can specified. When the user clicks on the text label the system browser
 * gets opened with the specified url.
 *
 * This can be used for e.g. in the about dialog where the url of the maintainer of an application
 * can be specified.
 * @author Werner Mayer
 */
class GuiExport UrlLabel : public QLabel
{
  Q_OBJECT
  Q_PROPERTY( QString  url    READ url   WRITE setUrl)

public:
  UrlLabel ( QWidget * parent = 0, Qt::WindowFlags f = Qt::WindowFlags() );
  virtual ~UrlLabel();

  QString url() const;

public Q_SLOTS:
  void setUrl( const QString &u );

protected:
  void enterEvent ( QEvent * );
  void leaveEvent ( QEvent * );
  void mouseReleaseEvent ( QMouseEvent * );

private:
  QString _url;
};

// ----------------------------------------------------------------------

/**
 * The LabelButton class provides a label with a button on the right side.
 * @author Werner Mayer
 */
class GuiExport LabelButton : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(QVariant value  READ value  WRITE setValue)

public:
    LabelButton (QWidget * parent = 0);
    virtual ~LabelButton();

    QVariant value() const;

    QLabel *getLabel() const;
    QPushButton *getButton() const;

public Q_SLOTS:
    void setValue(const QVariant&);

protected:
    virtual void showValue(const QVariant& data);
    void resizeEvent(QResizeEvent*);

protected Q_SLOTS:
    virtual void browse();

Q_SIGNALS:
    void valueChanged(const QVariant &);
    void buttonClicked();

private:
    QLabel *label;
    QPushButton *button;
    QVariant _val;
};

/** Implements a transparent tool tip label that is a child widget of the main
 * window
 */
class GuiExport TipLabel: public QLabel
{
    Q_OBJECT
public:
    TipLabel(QWidget *parent);
    static TipLabel * instance(QWidget *parent);
    static void hideLabel();
    void set(const QString &);
protected:
    void paintEvent(QPaintEvent *e);
    void resizeEvent(QResizeEvent *e);
private:
    int fontSize = 0;
};

// ----------------------------------------------------------------------

/**
 * Qt's tooltip does not work as expected with some classes, e.g. when showing
 * it in the 3d view it immediately receives a timer event to destroy itself.
 * This class is thought to circumvent this behaviour by filtering the internal
 * timer events.
 * @author Werner Mayer
 */
class GuiExport ToolTip : public QObject
{
public:
    enum Corner {
        NoCorner = -1, // pos is treat as global coordinate
        TopLeft = 0,
        TopRight = 1,
        BottomLeft = 2,
        BottomRight = 3,
    };
    static void showText(const QPoint & pos,
                         const QString & text,
                         QWidget * w = 0,
                         bool overlay = false,
                         Corner corner = NoCorner);
    static void hideText();

protected:
    static ToolTip* instance();

    ToolTip();
    virtual ~ToolTip();

    void timerEvent(QTimerEvent *e);
    bool eventFilter(QObject* o, QEvent*e);

    void installEventFilter();
    void removeEventFilter();

private:
    bool installed, hidden;
    static ToolTip* inst;
    bool overlay = false;
    QString text;
    QPoint pos;
    Corner corner;
    QPointer<QWidget> w; // need guard in case widget gets destroyed
    QBasicTimer tooltipTimer;
    QElapsedTimer displayTime;
};

// ----------------------------------------------------------------------

class GuiExport StatusWidget : public QDialog
{
    Q_OBJECT

public:
    StatusWidget(QWidget* parent);
    ~StatusWidget();
    void setStatusText(const QString&);
    QSize sizeHint () const;
    void showText(int ms);

protected:
    void showEvent(QShowEvent*);
    void hideEvent(QHideEvent*);

private:
    QLabel* label;
};

// ----------------------------------------------------------------------

class PropertyListEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    PropertyListEditor(QWidget *parent = 0);

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();

protected:
    void resizeEvent(QResizeEvent *event);

private Q_SLOTS:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &, int);

private:
    QWidget *lineNumberArea;
};

// ----------------------------------------------------------------------

class GuiExport LabelEditor : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(QString  text        READ text        WRITE setText      )
    Q_PROPERTY(QString  buttonText  READ buttonText  WRITE setButtonText)

public:
    enum InputType {String, Float, Integer};

    LabelEditor (QWidget * parent = 0);
    virtual ~LabelEditor();

    /**
    * Returns the text.
    */
    QString text() const;

    /**
    * Returns the button's text.
    */
    QString buttonText() const;

    /**
    * Set the input type.
    */
    void setInputType(InputType);

public Q_SLOTS:
    virtual void setText(const QString &);
    virtual void setButtonText (const QString &);
    virtual void validateText (const QString &);

Q_SIGNALS:
    void textChanged(const QString &);

private Q_SLOTS:
    void changeText();

protected:
    void resizeEvent(QResizeEvent*);

private:
    InputType type;
    QString plainText;
    QLineEdit *lineEdit;
    QPushButton *button;
};

/**
 * The ExpLineEdit class provides a lineedit that support expressing binding.
 * \author realthunder
 */
class GuiExport ExpLineEdit : public QLineEdit, public ExpressionBinding
{
    Q_OBJECT

public:
    ExpLineEdit ( QWidget * parent=0, bool expressionOnly=false );

    void setExpression(boost::shared_ptr<App::Expression> expr);
    void bind(const App::ObjectIdentifier &_path);
    bool apply(const std::string &propName);

    void keyPressEvent(QKeyEvent *event);
    void resizeEvent(QResizeEvent *event);

private Q_SLOTS:
    void finishFormulaDialog();
    void openFormulaDialog();
    virtual void onChange();

private:
    bool autoClose;
};

} // namespace Gui

#endif // GUI_WIDGETS_H
