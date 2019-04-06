/****************************************************************************
** Meta object code from reading C++ file 'msmodoWidget.cpp'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'msmodoWidget.cpp' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_msmodoSettingsWidget[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      20,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      24,   22,   21,   21, 0x0a,
      46,   22,   21,   21, 0x0a,
      66,   22,   21,   21, 0x0a,
      93,   22,   21,   21, 0x0a,
     117,   22,   21,   21, 0x0a,
     140,   22,   21,   21, 0x0a,
     169,   22,   21,   21, 0x0a,
     196,   22,   21,   21, 0x0a,
     221,   22,   21,   21, 0x0a,
     247,   22,   21,   21, 0x0a,
     278,   22,   21,   21, 0x0a,
     307,   22,   21,   21, 0x0a,
     332,   22,   21,   21, 0x0a,
     356,   22,   21,   21, 0x0a,
     378,   22,   21,   21, 0x0a,
     402,   22,   21,   21, 0x0a,
     436,   22,   21,   21, 0x0a,
     464,   22,   21,   21, 0x0a,
     495,   22,   21,   21, 0x0a,
     523,   22,   21,   21, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_msmodoSettingsWidget[] = {
    "msmodoSettingsWidget\0\0v\0onEditServer(QString)\0"
    "onEditPort(QString)\0onEditScaleFactor(QString)\0"
    "onToggleSyncMeshes(int)\0onToggleSyncBones(int)\0"
    "onToggleSyncBlendshapes(int)\0"
    "onToggleBakeDeformers(int)\0"
    "onToggleDoubleSided(int)\0"
    "onToggleSyncTextures(int)\0"
    "onToggleSyncMeshInstances(int)\0"
    "onToggleSyncReplicators(int)\0"
    "onToggleSyncCameras(int)\0"
    "onToggleSyncLights(int)\0onToggleAutoSync(int)\0"
    "onClickManualSync(bool)\0"
    "onEditAnimationTimeScale(QString)\0"
    "onEditAnimationSPS(QString)\0"
    "onToggleKeyframeReduction(int)\0"
    "onToggleKeepFlatCurves(int)\0"
    "onClickSyncAnimations(bool)\0"
};

void msmodoSettingsWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        msmodoSettingsWidget *_t = static_cast<msmodoSettingsWidget *>(_o);
        switch (_id) {
        case 0: _t->onEditServer((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->onEditPort((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: _t->onEditScaleFactor((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: _t->onToggleSyncMeshes((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->onToggleSyncBones((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->onToggleSyncBlendshapes((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: _t->onToggleBakeDeformers((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: _t->onToggleDoubleSided((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 8: _t->onToggleSyncTextures((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 9: _t->onToggleSyncMeshInstances((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 10: _t->onToggleSyncReplicators((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 11: _t->onToggleSyncCameras((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 12: _t->onToggleSyncLights((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 13: _t->onToggleAutoSync((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 14: _t->onClickManualSync((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 15: _t->onEditAnimationTimeScale((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 16: _t->onEditAnimationSPS((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 17: _t->onToggleKeyframeReduction((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 18: _t->onToggleKeepFlatCurves((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 19: _t->onClickSyncAnimations((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData msmodoSettingsWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject msmodoSettingsWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_msmodoSettingsWidget,
      qt_meta_data_msmodoSettingsWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &msmodoSettingsWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *msmodoSettingsWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *msmodoSettingsWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_msmodoSettingsWidget))
        return static_cast<void*>(const_cast< msmodoSettingsWidget*>(this));
    return QWidget::qt_metacast(_clname);
}

int msmodoSettingsWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 20)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 20;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
