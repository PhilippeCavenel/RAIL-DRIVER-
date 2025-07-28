/****************************************************************************
** Meta object code from reading C++ file 'ScribeMainWindow.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../ScribeMainWindow.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ScribeMainWindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.9.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN16ScribeMainWindowE_t {};
} // unnamed namespace

template <> constexpr inline auto ScribeMainWindow::qt_create_metaobjectdata<qt_meta_tag_ZN16ScribeMainWindowE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "ScribeMainWindow",
        "checkTextSignal",
        "",
        "toggleUndo",
        "undoAvailable",
        "toggleRedo",
        "redoAvailable",
        "toggleCopyAndCut",
        "copyCutAvailable",
        "updateTabAndWindowTitle",
        "closeTab",
        "Editor*",
        "tabToClose",
        "index",
        "closeTabShortcut",
        "informUser",
        "title",
        "message",
        "onTabRegainedFocus",
        "onTabLostFocus",
        "on_currentTabChanged",
        "on_languageSelected",
        "QAction*",
        "languageAction",
        "on_actionNew_triggered",
        "on_actionSaveTriggered",
        "on_actionOpen_triggered",
        "on_actionExit_triggered",
        "on_actionUndo_triggered",
        "on_actionCut_triggered",
        "on_actionCopy_triggered",
        "on_actionPaste_triggered",
        "on_actionFind_triggered",
        "on_actionGo_To_triggered",
        "on_actionSelect_All_triggered",
        "on_actionRedo_triggered",
        "on_actionPrint_triggered",
        "on_actionStatus_Bar_triggered",
        "on_actionTime_Date_triggered",
        "on_actionFont_triggered",
        "on_actionAuto_Indent_triggered",
        "on_actionWord_Wrap_triggered",
        "on_actionTool_Bar_triggered",
        "on_actionUpdate_triggered",
        "on_actionCheck_Program_triggered",
        "on_action_simpleCommand",
        "actionSelect_Port_Com",
        "processError",
        "s",
        "processTimeout",
        "on_progressBar_valueChanged",
        "value"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'checkTextSignal'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'toggleUndo'
        QtMocHelpers::SlotData<void(bool)>(3, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 4 },
        }}),
        // Slot 'toggleRedo'
        QtMocHelpers::SlotData<void(bool)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 6 },
        }}),
        // Slot 'toggleCopyAndCut'
        QtMocHelpers::SlotData<void(bool)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 8 },
        }}),
        // Slot 'updateTabAndWindowTitle'
        QtMocHelpers::SlotData<void()>(9, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'closeTab'
        QtMocHelpers::SlotData<bool(Editor *)>(10, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { 0x80000000 | 11, 12 },
        }}),
        // Slot 'closeTab'
        QtMocHelpers::SlotData<bool(int)>(10, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::Int, 13 },
        }}),
        // Slot 'closeTabShortcut'
        QtMocHelpers::SlotData<void()>(14, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'informUser'
        QtMocHelpers::SlotData<void(QString, QString)>(15, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 16 }, { QMetaType::QString, 17 },
        }}),
        // Slot 'onTabRegainedFocus'
        QtMocHelpers::SlotData<void()>(18, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onTabLostFocus'
        QtMocHelpers::SlotData<void()>(19, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'on_currentTabChanged'
        QtMocHelpers::SlotData<void(int)>(20, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 13 },
        }}),
        // Slot 'on_languageSelected'
        QtMocHelpers::SlotData<void(QAction *)>(21, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 22, 23 },
        }}),
        // Slot 'on_actionNew_triggered'
        QtMocHelpers::SlotData<void()>(24, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_actionSaveTriggered'
        QtMocHelpers::SlotData<bool()>(25, 2, QMC::AccessPrivate, QMetaType::Bool),
        // Slot 'on_actionOpen_triggered'
        QtMocHelpers::SlotData<void()>(26, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_actionExit_triggered'
        QtMocHelpers::SlotData<void()>(27, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_actionUndo_triggered'
        QtMocHelpers::SlotData<void()>(28, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_actionCut_triggered'
        QtMocHelpers::SlotData<void()>(29, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_actionCopy_triggered'
        QtMocHelpers::SlotData<void()>(30, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_actionPaste_triggered'
        QtMocHelpers::SlotData<void()>(31, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_actionFind_triggered'
        QtMocHelpers::SlotData<void()>(32, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_actionGo_To_triggered'
        QtMocHelpers::SlotData<void()>(33, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_actionSelect_All_triggered'
        QtMocHelpers::SlotData<void()>(34, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_actionRedo_triggered'
        QtMocHelpers::SlotData<void()>(35, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_actionPrint_triggered'
        QtMocHelpers::SlotData<void()>(36, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_actionStatus_Bar_triggered'
        QtMocHelpers::SlotData<void()>(37, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_actionTime_Date_triggered'
        QtMocHelpers::SlotData<void()>(38, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_actionFont_triggered'
        QtMocHelpers::SlotData<void()>(39, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_actionAuto_Indent_triggered'
        QtMocHelpers::SlotData<void()>(40, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_actionWord_Wrap_triggered'
        QtMocHelpers::SlotData<void()>(41, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_actionTool_Bar_triggered'
        QtMocHelpers::SlotData<void()>(42, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_actionUpdate_triggered'
        QtMocHelpers::SlotData<void()>(43, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_actionCheck_Program_triggered'
        QtMocHelpers::SlotData<void()>(44, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'on_action_simpleCommand'
        QtMocHelpers::SlotData<void()>(45, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'actionSelect_Port_Com'
        QtMocHelpers::SlotData<void()>(46, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'processError'
        QtMocHelpers::SlotData<void(const QString &)>(47, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 48 },
        }}),
        // Slot 'processTimeout'
        QtMocHelpers::SlotData<void(const QString &)>(49, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 48 },
        }}),
        // Slot 'on_progressBar_valueChanged'
        QtMocHelpers::SlotData<void(int)>(50, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 51 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<ScribeMainWindow, qt_meta_tag_ZN16ScribeMainWindowE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject ScribeMainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16ScribeMainWindowE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16ScribeMainWindowE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN16ScribeMainWindowE_t>.metaTypes,
    nullptr
} };

void ScribeMainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<ScribeMainWindow *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->checkTextSignal(); break;
        case 1: _t->toggleUndo((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 2: _t->toggleRedo((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 3: _t->toggleCopyAndCut((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 4: _t->updateTabAndWindowTitle(); break;
        case 5: { bool _r = _t->closeTab((*reinterpret_cast< std::add_pointer_t<Editor*>>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 6: { bool _r = _t->closeTab((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 7: _t->closeTabShortcut(); break;
        case 8: _t->informUser((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 9: _t->onTabRegainedFocus(); break;
        case 10: _t->onTabLostFocus(); break;
        case 11: _t->on_currentTabChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 12: _t->on_languageSelected((*reinterpret_cast< std::add_pointer_t<QAction*>>(_a[1]))); break;
        case 13: _t->on_actionNew_triggered(); break;
        case 14: { bool _r = _t->on_actionSaveTriggered();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 15: _t->on_actionOpen_triggered(); break;
        case 16: _t->on_actionExit_triggered(); break;
        case 17: _t->on_actionUndo_triggered(); break;
        case 18: _t->on_actionCut_triggered(); break;
        case 19: _t->on_actionCopy_triggered(); break;
        case 20: _t->on_actionPaste_triggered(); break;
        case 21: _t->on_actionFind_triggered(); break;
        case 22: _t->on_actionGo_To_triggered(); break;
        case 23: _t->on_actionSelect_All_triggered(); break;
        case 24: _t->on_actionRedo_triggered(); break;
        case 25: _t->on_actionPrint_triggered(); break;
        case 26: _t->on_actionStatus_Bar_triggered(); break;
        case 27: _t->on_actionTime_Date_triggered(); break;
        case 28: _t->on_actionFont_triggered(); break;
        case 29: _t->on_actionAuto_Indent_triggered(); break;
        case 30: _t->on_actionWord_Wrap_triggered(); break;
        case 31: _t->on_actionTool_Bar_triggered(); break;
        case 32: _t->on_actionUpdate_triggered(); break;
        case 33: _t->on_actionCheck_Program_triggered(); break;
        case 34: _t->on_action_simpleCommand(); break;
        case 35: _t->actionSelect_Port_Com(); break;
        case 36: _t->processError((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 37: _t->processTimeout((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 38: _t->on_progressBar_valueChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 5:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< Editor* >(); break;
            }
            break;
        case 12:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QAction* >(); break;
            }
            break;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (ScribeMainWindow::*)()>(_a, &ScribeMainWindow::checkTextSignal, 0))
            return;
    }
}

const QMetaObject *ScribeMainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ScribeMainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16ScribeMainWindowE_t>.strings))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int ScribeMainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 39)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 39;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 39)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 39;
    }
    return _id;
}

// SIGNAL 0
void ScribeMainWindow::checkTextSignal()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
QT_WARNING_POP
