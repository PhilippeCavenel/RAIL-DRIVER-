/****************************************************************************
** Meta object code from reading C++ file 'SimpleGraphModel.hpp'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../SimpleGraphModel.hpp"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SimpleGraphModel.hpp' doesn't include <QObject>."
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
struct qt_meta_tag_ZN16SimpleGraphModelE_t {};
} // unnamed namespace

template <> constexpr inline auto SimpleGraphModel::qt_create_metaobjectdata<qt_meta_tag_ZN16SimpleGraphModelE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "SimpleGraphModel",
        "portUpdated",
        "",
        "NodeId",
        "nodeId",
        "PortType",
        "portType",
        "PortIndex",
        "portIndex"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'portUpdated'
        QtMocHelpers::SignalData<void(NodeId, PortType, PortIndex)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 }, { 0x80000000 | 5, 6 }, { 0x80000000 | 7, 8 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<SimpleGraphModel, qt_meta_tag_ZN16SimpleGraphModelE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject SimpleGraphModel::staticMetaObject = { {
    QMetaObject::SuperData::link<QtNodes::AbstractGraphModel::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16SimpleGraphModelE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16SimpleGraphModelE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN16SimpleGraphModelE_t>.metaTypes,
    nullptr
} };

void SimpleGraphModel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<SimpleGraphModel *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->portUpdated((*reinterpret_cast< std::add_pointer_t<NodeId>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<PortType>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<PortIndex>>(_a[3]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (SimpleGraphModel::*)(NodeId , PortType , PortIndex )>(_a, &SimpleGraphModel::portUpdated, 0))
            return;
    }
}

const QMetaObject *SimpleGraphModel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SimpleGraphModel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16SimpleGraphModelE_t>.strings))
        return static_cast<void*>(this);
    return QtNodes::AbstractGraphModel::qt_metacast(_clname);
}

int SimpleGraphModel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QtNodes::AbstractGraphModel::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void SimpleGraphModel::portUpdated(NodeId _t1, PortType _t2, PortIndex _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1, _t2, _t3);
}
QT_WARNING_POP
